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
#include "interrupts.h"
#include "stdbool.h"
#include "stdio.h"

void mouse_interrupt_handler(void);

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

#define PS2_CMND_PORT 0x64
#define PS2_DATA_PORT 0x60

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static uint16_t *ptr_mem(uint32_t lig, uint32_t col);
static void      ecrit_car(uint32_t lig, uint32_t col, char c, char cl_bg,
                           char cl_text);
static void      place_curseur(uint32_t lig, uint32_t col, bool move);
static void      traite_car(char c, char cl_bg, char cl_text);
static void      defilement(void);
static void      efface_ecran(void);

static void do_mouse(uint8_t flags, uint8_t dx, uint8_t dy);

/*******************************************************************************
 * Variables
 ******************************************************************************/

// position the next character will be written to
static uint32_t current_pos_lig = 0;
static uint32_t current_pos_col = 0;

// purely visual cursor indication
static uint8_t mouse_packet[3];
static int     mouse_byte;
static struct {
  int x;
  int y;
} mouse_position;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void console_init(void)
{
  efface_ecran();
  printf(":: reached kernel start \n");

  // PS2 reference: https://wiki.osdev.org/%228042%22_PS/2_Controller
  printf(":: configuring PS/2 mouse driver \n");
  // check controller configuration byte
  outb(0x20, PS2_CMND_PORT);
  uint8_t ccb = inb(PS2_DATA_PORT);
  printf("initial PS/2 controller configuration: %#x\n", ccb);
  // enable second device (mouse)
  outb(0xA8, PS2_CMND_PORT);
  ccb |= 0b00000010;
  // and activate its IRQ
  outb(0x60, PS2_CMND_PORT);
  outb(ccb, PS2_DATA_PORT);
  // test config succeeded
  outb(0x20, PS2_CMND_PORT);
  ccb = inb(PS2_DATA_PORT);
  if (!(ccb & 0b00100010)) goto GIVE_UP;
  printf("successfully initialized second PS/2 device\n");

  // Mouse ref: https://wiki.osdev.org/Mouse_Input
  // default setup
  outb(0xD4, PS2_CMND_PORT);
  outb(0xF6, PS2_DATA_PORT);
  if (inb(PS2_DATA_PORT) != 0xFA) goto GIVE_UP;
  // change sample rate
  outb(0xD4, PS2_CMND_PORT);
  outb(0xF3, PS2_DATA_PORT);
  if (inb(PS2_DATA_PORT) != 0xFA) goto GIVE_UP;
  outb(0xD4, PS2_CMND_PORT);
  outb(40, PS2_DATA_PORT);
  if (inb(PS2_DATA_PORT) != 0xFA) goto GIVE_UP;
  // enables streaming
  outb(0xD4, PS2_CMND_PORT);
  outb(0xF4, PS2_DATA_PORT);
  if (inb(PS2_DATA_PORT) != 0xFA) goto GIVE_UP;
  printf("mouse ok\n");
  mouse_byte = -1; // signal first drop

  // cursor interrupt setup
  // TODO: use INTR_VECTOR_OFFSET instead of 32
  set_interrupt_handler(32 + 12, mouse_interrupt_handler, PL_USER);
  mask_irq(12, false);

  return;
GIVE_UP:
  printf("Error: mouse driver initialization failed\n");
  return;
}

void mouse_interrupt(void)
{
  // XXX: the first byte we get is dirty, drop it so as to not mess the buffer
  const uint8_t byte = inb(PS2_DATA_PORT);
  const bool    drop = mouse_byte < 0;

  if (!drop) {
    mouse_packet[mouse_byte] = byte;
    mouse_byte = (mouse_byte + 1) % sizeof(mouse_packet);
    // process mouse input when it is complete
    if (mouse_byte == 0)
      do_mouse(mouse_packet[0], mouse_packet[1], mouse_packet[2]);
  } else {
    mouse_byte = 0;
  }

  acknowledge_interrupt(12);
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
static inline void place_curseur(uint32_t lig, uint32_t col, bool move)
{
  const uint16_t pos = col + lig * CONSOLE_L;
  outb(0x0f, ECRAN_CMD_PORT);
  outb((pos & 0xFF), ECRAN_DAT_PORT);
  outb(0x0e, ECRAN_CMD_PORT);
  outb((pos >> 8), ECRAN_DAT_PORT);
  if (move) {
    current_pos_lig = lig;
    current_pos_col = col;
    mouse_position.x = current_pos_col;
    mouse_position.y = current_pos_lig;
  }
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
      traite_car('U', cl_bg, cl_text);
      return;
    }
  }
  place_curseur(current_pos_lig, current_pos_col, true);
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

static void do_mouse(uint8_t flags, uint8_t dx, uint8_t dy)
{
  if (flags & 0b11000000) return; // drop "overflow" packets

  // sign-extend movement bytes in case they're negative
  int sdx = flags & 0b00010000 ? 0xFFFFFF00 | (unsigned)dx : (unsigned)dx;
  int sdy = flags & 0b00100000 ? 0xFFFFFF00 | (unsigned)dy : (unsigned)dy;

  // these can't overflow, but result must respect screen bounds
  int x = mouse_position.x + sdx;
  x = x < 0 ? 0 : (x >= CONSOLE_L ? CONSOLE_L - 1 : x);
  // mouse's vertical axis is the usual one, but cursor's in screen coords.
  int y = mouse_position.y - sdy;
  y = y < 0 ? 0 : (y >= CONSOLE_H ? CONSOLE_H - 1 : y);

  // update VISUAL cursor position
  place_curseur(y, x, false);
  mouse_position.x = x;
  mouse_position.y = y;
}
