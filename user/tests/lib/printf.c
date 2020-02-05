/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 */

#include "sysapi.h"

/*******************************************************************************
 * Gestion de liste d'arguments de taille variable (printf)
 ******************************************************************************/
#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
  *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))
#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#define va_start(AP, LASTARG)                                           \
 (AP = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))
#define va_end(AP)      ((void)0)

/*******************************************************************************
 * Printf macros
 ******************************************************************************/
#define PRINTF_LEFT_JUSTIFY 1
#define PRINTF_SHOW_SIGN 2
#define PRINTF_SPACE_PLUS 4
#define PRINTF_ALTERNATE 8
#define PRINTF_PAD0 16
#define PRINTF_CAPITAL_X 32

#define PRINTF_BUF_LEN 512

        static void
cons_puts(const char *s)
{
        cons_write(s, strlen(s));
}

/*******************************************************************************
  static int _printf( char *s, unsigned long n, const char *format, va_list ap )
  La grosse fonction du module. Une version interne du printf, qui imprime
  soit sur la sortie standard, soit dans un buffer.
  Si s == 0 : on imprime sur la sortie standard.
  Si s != 0 : on imprime dans la chaine s sur une taille maximale n.
 *******************************************************************************/
struct printf_st {
        /* Flags obtenus a l'interpretation de la chaine de format. */
        int flags;
        int width;
        int precision;
        char modifier;
        int count;

        /* Ce buffer permet de contenir le texte correspondant a l'affichage
           d'un nombre. */
        char buffer_nombre[20];

        /* Buffer pour l'ecriture a la console. */
        char buffer_ecr[PRINTF_BUF_LEN];
        int pos_ecr;

        char *str;
        unsigned long strl;
};

void print(struct printf_st *pf, char c)
{
        while (1) {
                if (pf->str == 0) {
                        /* Cas de l'ecriture sur un fichier. */
                        if (c == 0)
                                return;

                        if (pf->pos_ecr < PRINTF_BUF_LEN - 1) {
                                pf->count++;
                                pf->buffer_ecr[pf->pos_ecr++] = c;
                        } else {
                                pf->buffer_ecr[PRINTF_BUF_LEN - 1] = 0;
                                cons_puts(pf->buffer_ecr);
                                pf->buffer_ecr[0] = c;
                                pf->pos_ecr = 1;
                        }
                } else {
                        /* Cas de l'ecriture dans un buffer. */
                        if ((c != 0) && (pf->strl != 0)) {
                                pf->count++;
                                *pf->str++ = c;
                                pf->strl--;
                        }
                }
                if (c != '\n') return;
                c = '\r';
        }
}

/****************************************************************************
 * Pour afficher les "%s".
 ***************************************************************************/
void print_string(struct printf_st *pf, char *s) {
        int size = 0;
        char *ptr = s;

        /* Calcule la taille de la partie de la chaine a afficher. */
        if (pf->precision >= 0)
                while ((size < pf->precision) && (*ptr++ != 0))
                        size++;
        else
                /* Pas besoin d'aller trop loin dans le calcul de la taille de la
                   partie a afficher. Si la valeur est superieure a width, elle ne nous
                   sert a rien, bien qu'on va tout afficher. */
                while ((size < pf->width) && (*ptr++ != 0))
                        size++;

        if (!(pf->flags & PRINTF_LEFT_JUSTIFY))
                while (pf->width-- > size)
                        print(pf, ' ');
        while ((pf->precision-- != 0) && (*s != 0))
                print(pf, *s++);
        while (pf->width-- > size)
                print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%c".
 ******************************************************************************/
void print_char(struct printf_st *pf, char c) {
        if (!(pf->flags & PRINTF_LEFT_JUSTIFY))
                while (pf->width-- > 1)
                        print(pf, ' ');
        print(pf, c);
        while (pf->width-- > 1)
                print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%x", "%X".
 ******************************************************************************/
void print_hexa(struct printf_st *pf, unsigned long i) {
        int pos = 0;
        int n;

        /* On ne met pas le "0x" si le nombre est nul. */
        if (i == 0)
                pf->flags &= ~PRINTF_ALTERNATE;

        /* La pf->precision par defaut pour un entier est 1. */
        if (pf->precision == -1)
                pf->precision = 1;
        else
                pf->flags &= ~PRINTF_PAD0;

        /* On ecrit l'entier dans le buffer. */
        while (i != 0) {
                n = i % 16;
                i = i / 16;

                /* On calcule le chiffre de poids faible. */
                if (n < 10)
                        n += '0';
                else if (pf->flags & PRINTF_CAPITAL_X)
                        n += 'A' - 10;
                else
                        n += 'a' - 10;

                /* On le met en buffer. */
                pf->buffer_nombre[pos++] = n;
        }

        /* On met a jour la precision avec celle que demande le nombre affiche. */
        pf->precision = (pos > pf->precision) ? pos : pf->precision;

        /* Si on doit remplir avec des 0, on modifie la precision en consequence. */
        if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
                n = pf->width;

                if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
                        n--;
                if (pf->flags & PRINTF_ALTERNATE)
                        n -= 2;
                pf->precision = (pf->precision > n) ? pf->precision : n;
                n = pf->width;
        } else {
                n = pf->precision;
                if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
                        n++;
                if (pf->flags & PRINTF_ALTERNATE)
                        n += 2;
        }
        /* Ici n = nombre de caracteres != ' ' affiches. */

        /* Doit-on mettre des espaces de remplissage avant le nombre ? */
        if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
                while (pf->width-- > n)
                        print(pf, ' ');
        }

        /* On place eventuellement le signe. */
        if (pf->flags & PRINTF_SHOW_SIGN)
                print(pf, '+');
        else if (pf->flags & PRINTF_SPACE_PLUS)
                print(pf, ' ');

        /* On ecrit l'eventuel "0x" ou "0X". */
        if (pf->flags & PRINTF_ALTERNATE) {
                print(pf, '0');
                if (pf->flags & PRINTF_CAPITAL_X)
                        print(pf, 'X');
                else
                        print(pf, 'x');
        }

        /* On met les eventuels 0 de remplissage. */
        while (pf->precision-- > pos)
                print(pf, '0');

        /* On copie le reste du nombre. */
        while (pos-- != 0)
                print(pf, pf->buffer_nombre[pos]);

        /* On met enfin les eventuels espaces de fin. */
        while (pf->width-- > n)
                print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%d", "%i" et "%u". Le signe doit etre '+' ou '-'.
 ******************************************************************************/
void print_dec(struct printf_st *pf, unsigned long i, char sign) {
        int pos = 0;
        int n;

        /* La precision par defaut pour un entier est 1. */
        if (pf->precision == -1)
                pf->precision = 1;
        else
                pf->flags &= ~PRINTF_PAD0;

        /* On determine le signe a afficher. */
        if ((sign == '+') && (!(pf->flags & PRINTF_SHOW_SIGN))) {
                if (pf->flags & PRINTF_SPACE_PLUS)
                        sign = ' ';
                else
                        sign = 0;
        }

        /* On ecrit l'entier dans le buffer. */
        while (i != 0) {
                /* On le met en buffer. */
                pf->buffer_nombre[pos++] = (i % 10) + '0';
                i = i / 10;
        }

        /* On met a jour la precision avec celle que demande le nombre affiche. */
        pf->precision = (pos > pf->precision) ? pos : pf->precision;

        /* Si on doit remplir avec des 0, on modifie la precision en consequence. */
        if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
                n = pf->width;

                if (sign != 0)
                        n--;
                pf->precision = (pf->precision > n) ? pf->precision : n;
                n = pf->width;
        } else {
                n = pf->precision;
                if (sign != 0)
                        n++;
        }
        /* Ici n = nombre de caracteres != ' ' affiches. */

        /* Doit-on mettre des espaces de remplissage avant le nombre ? */
        if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
                while (pf->width-- > n)
                        print(pf, ' ');
        }

        /* On place eventuellement le signe. */
        if (sign != 0)
                print(pf, sign);

        /* On met les eventuels 0 de remplissage. */
        while (pf->precision-- > pos)
                print(pf, '0');

        /* On copie le reste du nombre. */
        while (pos-- != 0)
                print(pf, pf->buffer_nombre[pos]);

        /* On met enfin les eventuels espaces de fin. */
        while (pf->width-- > n)
                print(pf, ' ');
}

/*******************************************************************************
 *   Pour afficher les "%x", "%X".
 ******************************************************************************/
void print_oct(struct printf_st *pf, unsigned int i) {
        int pos = 0;
        int n;

        /* La precision par defaut pour un entier est 1. */
        if (pf->precision == -1)
                pf->precision = 1;
        else
                pf->flags &= ~PRINTF_PAD0;

        /* On ecrit l'entier dans le buffer. */
        while (i != 0) {
                pf->buffer_nombre[pos++] = (i % 8) + '0';
                i = i / 8;
        }

        /* On verifie si on doit mettre un zero en tete. */
        if (pf->flags & PRINTF_ALTERNATE)
                pf->buffer_nombre[pos++] = '0';

        /* On met a jour la precision avec celle que demande le nombre affiche. */
        pf->precision = (pos > pf->precision) ? pos : pf->precision;

        /* Si on doit remplir avec des 0, on modifie la precision en consequence. */
        if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
                n = pf->width;

                if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
                        n--;
                pf->precision = (pf->precision > n) ? pf->precision : n;
                n = pf->width;
        } else {
                n = pf->precision;
                if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
                        n++;
        }
        /* Ici n = nombre de caracteres != ' ' affiches. */

        /* Doit-on mettre des espaces de remplissage avant le nombre ? */
        if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
                while (pf->width-- > n)
                        print(pf, ' ');
        }

        /* On place eventuellement le signe. */
        if (pf->flags & PRINTF_SHOW_SIGN)
                print(pf, '+');
        else if (pf->flags & PRINTF_SPACE_PLUS)
                print(pf, ' ');

        /* On met les eventuels 0 de remplissage. */
        while (pf->precision-- > pos)
                print(pf, '0');

        /* On copie le reste du nombre. */
        while (pos-- != 0)
                print(pf, pf->buffer_nombre[pos]);

        /* On met enfin les eventuels espaces de fin. */
        while (pf->width-- > n)
                print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%p".
 ******************************************************************************/
void print_pointer(struct printf_st *pf, void *p) {
        if (p == 0) {
                print_string(pf, "(nil)");
        } else {
                pf->flags |= PRINTF_ALTERNATE;
                print_hexa(pf, (unsigned long) p);
        }
}

/*******************************************************************************
 * Voici la fonction "principale".
 ******************************************************************************/
int __printf(struct printf_st *pf, const char *format, va_list ap) {
        pf->count = 0;
        while (*format != 0) {
                if (*format == '%') {
                        const char *ptr = format + 1;

                        /* On lit le champ optionnel flags. */
                        pf->flags = 0;
flags_l:
                        switch (*ptr) {
                                case '-':
                                        pf->flags |= PRINTF_LEFT_JUSTIFY;
                                        ptr++;
                                        goto flags_l;

                                case '+':
                                        pf->flags |= PRINTF_SHOW_SIGN;
                                        ptr++;
                                        goto flags_l;

                                case ' ':
                                        pf->flags |= PRINTF_SPACE_PLUS;
                                        ptr++;
                                        goto flags_l;

                                case '#':
                                        pf->flags |= PRINTF_ALTERNATE;
                                        ptr++;
                                        goto flags_l;

                                case '0':
                                        pf->flags |= PRINTF_PAD0;
                                        ptr++;
                                        goto flags_l;
                        }

                        /* On lit le champ optionnel width. */
                        if (*ptr == '*') {
                                pf->width = va_arg(ap, int);
                                ptr++;
                        } else {
                                pf->width = 0;
                                while ((*ptr >= '0') && (*ptr <= '9'))
                                        pf->width =
                                                pf->width * 10 + (*ptr++) - '0';
                        }

                        /* On lit le champ optionnel de precision. */
                        if (*ptr == '.') {
                                ptr++;
                                if (*ptr == '*') {
                                        pf->precision = va_arg(ap, int);
                                        ptr++;
                                } else {
                                        pf->precision = 0;
                                        while ((*ptr >= '0')
                                                        && (*ptr <= '9'))
                                                pf->precision +=
                                                        pf->precision * 10 +
                                                        (*ptr++) - '0';
                                }
                        } else
                                pf->precision = -1;

                        /* On lit le champ optionnel modifier. */
                        pf->modifier = 0;
                        if ((*ptr == 'h') || (*ptr == 'l')
                                        || (*ptr == 'L'))
                                pf->modifier = *ptr++;

                        /* On lit enfin le champ obligatoire. */
                        switch (*ptr) {
                                case 'p':
                                        print_pointer(pf, va_arg(ap, void *));
                                        break;

                                case 'X':
                                        pf->flags |= PRINTF_CAPITAL_X;
					/* FALLTHRU */
                                case 'x':
                                        if (pf->modifier == 'h')
                                                print_hexa(pf, va_arg(ap, int));
                                        else if (pf->modifier == 'l')
                                                print_hexa(pf, va_arg
                                                                (ap, unsigned long));
                                        else
                                                print_hexa(pf, va_arg
                                                                (ap, unsigned int));
                                        break;

                                case 'd':
                                case 'i':
                                        {
                                                int i;

                                                if (pf->modifier == 'h')
                                                        i = va_arg(ap, int);
                                                else if (pf->modifier == 'l')
                                                        i = va_arg(ap, long);
                                                else
                                                        i = va_arg(ap, int);
                                                if (i < 0)
                                                        print_dec(pf, -i, '-');
                                                else
                                                        print_dec(pf, i, '+');
                                                break;
                                        }

                                case 'u':
                                        {
                                                int i;

                                                if (pf->modifier == 'h')
                                                        i = va_arg(ap, int);
                                                else if (pf->modifier == 'l')
                                                        i = va_arg(ap, long);
                                                else
                                                        i = va_arg(ap, int);
                                                if (i < 0)
                                                        print_dec(pf, -i, '-');
                                                else
                                                        print_dec(pf, i, '+');

                                                break;
                                        }

                                case 's':
                                        print_string(pf, va_arg(ap, char *));
                                        break;

                                case 'c':
                                        print_char(pf, va_arg(ap, int));
                                        break;

                                case '%':
                                        print(pf, '%');
                                        break;

                                case 'o':
                                        if (pf->modifier == 'h')
                                                print_oct(pf, va_arg(ap, int));
                                        else if (pf->modifier == 'l')
                                                print_oct(pf, va_arg
                                                                (ap, unsigned long));
                                        else
                                                print_oct(pf, va_arg
                                                                (ap, unsigned int));
                                        break;

                                case 'n':
                                        *va_arg(ap, int *) = pf->count;
                                        break;

                                case 'f':
                                case 'e':
                                case 'E':
                                case 'g':
                                case 'G':
                                        pf->flags = 0;
                                        pf->width = 0;
                                        pf->precision = -1;
                                        print_string
                                                (pf, "<float format not implemented>");
                                        break;

                                default:
                                        /* Comme le format n'est pas valide, on l'affiche ! */
                                        while (format < ptr)
                                                print(pf, *format++);
                                        ptr--;
                        }
                        format = ptr + 1;
                } else
                        print(pf, *format++);
        }
        return pf->count;
}

static int _printf(char *s, unsigned long n, const char *format, va_list ap)
{
        struct printf_st pf;
        pf.pos_ecr = 0;
        pf.str = s;
        pf.strl = n;

        if (s != 0) {
                /* Cas du print dans un buffer. */
                if (n) {
                        /* On reserve un caractere pour le 0 terminal. */
                        n--;
                        __printf(&pf, format, ap);
                        *s = 0;

                        /* On renvoie la taille de la chaine ecrite. */
                        return pf.count;
                }

                return 0;
        } else {
                /* Cas du print dans sur la console. */
                __printf(&pf, format, ap);
                pf.buffer_ecr[pf.pos_ecr] = 0;
                cons_puts(pf.buffer_ecr);

                /* On renvoie la taille de la chaine ecrite. */
                return pf.count;
        }
}

/*******************************************************************************
  int printf( const char *format, ... )
 *******************************************************************************/
int safe_printf(const char *format, ...)
{
        int ret;
        va_list ap;

        va_start(ap, format);
        ret = _printf(0, 0, format, ap);

        va_end(ap);
        return ret;
}

void cons_gets(char *s, unsigned long length)
{
#if defined CONS_READ_LINE
        unsigned long n;
        if (length == 0) return;
        n = cons_read(s, length - 1);
        s[n] = 0;
        return;
#elif defined CONS_READ_CHAR
        unsigned long n = 0;
        cons_echo(0);
        while (n < (length-1)) {
                int c = cons_read();
                if ((c <= 126) && (c >= 32)) {
                        s[n] = c;
                        cons_write(s + n, 1);
                        n++;
                } else if ((c == '\n') || (c == 13)) {
                        s[n] = 0;
                        cons_write("\n", 1);
                        cons_echo(1);
                        return;
                } else if ((c == 8) || (c == 127)) {
                        if (n > 0) {
                                n--;
                                safe_printf("%c %c", 8, 8);
                        }
                }
        }
        s[n] = 0;
        cons_echo(1);
#endif
}


