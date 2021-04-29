/*
 * console.c
 *
 *  Created on: 05/02/2021
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "console.h"
#include "stdint.h"
#include "string.h"
#include "cpu.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define CONSOLE_H 25
#define CONSOLE_L 80

#define CL_NOIR    0
#define CL_BLEU    1
#define CL_VERT    2
#define CL_CYAN    3
#define CL_ROUGE   4
#define CL_MAGENTA 5
#define CL_MARRON  6
#define CL_GRIS    7
#define CL_LIGHT   0b1000

#define ECRAN_BASE_ADDR (uint16_t *)0xB8000
#define ECRAN_BASE_SIZE (CONSOLE_H * CONSOLE_L * 2)

#define ECRAN_CMD_PORT 0x3D4
#define ECRAN_DAT_PORT 0x3D5

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static uint16_t *ptr_mem(uint32_t lig, uint32_t col);
static void      ecrit_car(uint32_t lig, uint32_t col, char c, char cl_bg,
                           char cl_text);
static void      place_curseur(uint32_t lig, uint32_t col);
static void      traite_car(char c, char cl_bg, char cl_text);
static void      defilement(void);
static void      efface_ecran(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint32_t current_pos_lig = 0;
uint32_t current_pos_col = 0;

/*******************************************************************************
 * Public function
 ******************************************************************************/

/* initialisation de la console */
void console_init(void)
{
  efface_ecran();
}

int cons_write(const char *str, long size)
{
  for (long i = 0; i < size; i++) {
    traite_car(str[i], CL_NOIR, CL_CYAN | CL_LIGHT);
  }
  return size;
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*
 * renvoie un pointeur sur la case mémoire correspondant aux coordonnées
 * fournies (cette fonction peut aussi s'écrire efficacement sous la forme
 * d'une macro
 */
static inline uint16_t *ptr_mem(uint32_t lig, uint32_t col)
{
  return ECRAN_BASE_ADDR + (lig * CONSOLE_L + col);
}

/*  écrit le caractère c aux coordonnées spécifiées (vous pouvez aussi ajouter
 * des paramètres pour permettre de préciser la couleur du caractère, celle du
 * fond ou le bit de clignotement) */
static inline void ecrit_car(uint32_t lig, uint32_t col, char c, char cl_bg,
                             char cl_text)
{
  uint16_t couleur = (cl_bg & 0b111) << 4 | (cl_text & 0b1111);
  *ptr_mem(lig, col) = (uint16_t)c | (couleur << 8);
}

/* place le curseur à la position donnée */
static inline void place_curseur(uint32_t lig, uint32_t col)
{
  uint16_t pos = col + lig * CONSOLE_L;
  outb(0x0f, ECRAN_CMD_PORT);
  outb((pos & 0xFF), ECRAN_DAT_PORT);
  outb(0x0e, ECRAN_CMD_PORT);
  outb((pos >> 8), ECRAN_DAT_PORT);
  current_pos_lig = lig;
  current_pos_col = col;
}

/* traite un caractère donné (c'est à dire qui l'affiche si c'est un caractère
 * normal ou qui implante l'effet voulu si c'est un caractère de contrôle) */
static void traite_car(char c, char cl_bg, char cl_text)
{
  if (c >= 32 && c <= 126) {
    ecrit_car(current_pos_lig, current_pos_col, c, cl_bg, cl_text);
    // update pos
    current_pos_col++;
    if (current_pos_col == CONSOLE_L) {
      current_pos_col = 0;
      current_pos_lig++;
      if (current_pos_lig >= CONSOLE_H) {
        current_pos_lig = CONSOLE_H - 1;
        defilement();
      }
    }
  } else {
    switch (c) {
    case '\b':
      current_pos_col = current_pos_col == 0 ? 0 : current_pos_col - 1;
      break;
    case '\t':
      current_pos_col += 8 - current_pos_col % 8;
      if (current_pos_col >= CONSOLE_L) {
        current_pos_col = 0;
        current_pos_lig++;
        if (current_pos_lig >= CONSOLE_H) {
          current_pos_lig = CONSOLE_H - 1;
          defilement();
        }
      }
      break;
    case '\n':
      current_pos_col = 0;
      current_pos_lig++;
      if (current_pos_lig >= CONSOLE_H) {
        current_pos_lig = CONSOLE_H - 1;
        defilement();
      }
      break;
    case '\f':
      efface_ecran();
      break;
    case '\r':
      current_pos_col = 0;
      break;
    default:
      return;
    }
  }
  place_curseur(current_pos_lig, current_pos_col);
}

/* fait remonter d'une ligne l'affichage à l'écran */
static void defilement(void)
{
  memmove(ECRAN_BASE_ADDR,
          ECRAN_BASE_ADDR + CONSOLE_L,
          ECRAN_BASE_SIZE - CONSOLE_L * 2);
  for (uint16_t x = 0; x < CONSOLE_L; x++)
    *ptr_mem(CONSOLE_H - 1, x) = (uint16_t)' ' | ((uint16_t)0b00001111 << 8);
}

/* parcourir les lignes et les colonnes de l'écran pour écrire dans chaque case
 * un espace en blanc sur fond noir (afin d'initialiser les formats dans la
 * mémoire) */
static void efface_ecran(void)
{
  current_pos_col = 0;
  current_pos_lig = 0;
  for (uint16_t y = 0; y < CONSOLE_H; y++) {
    for (uint16_t x = 0; x < CONSOLE_L; x++)
      *ptr_mem(y, x) = (uint16_t)' ' | ((uint16_t)0b00001111 << 8);
  }
}
