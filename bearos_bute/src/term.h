/*==========================================================================

  term.h

  Functions for interacting with ANSI-like terminals

  (c)2021 Kevin Boone, GPLv3.0

==========================================================================*/
#pragma once

#include <stdint.h> 
#include "defs.h"

/** Clear screen, without homing cursor. */
extern void term_clear (void);

/** Clear from cursor to end of line. */
void term_clear_eol (void);

/** Clear screen and move cursor to (0,0) */
extern void term_clear_and_home (void);

extern void term_get_size (int *rows, int *cols);

/** Write a line, truncating to fit width if required. Top line is zero. */
extern void term_write_line (uint8_t row, const char *line, BOOL truncate);

/** Set the cursor position, from top-left (which is 0, 0). */
void term_set_cursor (uint8_t row, uint8_t col);

/** Erase the whole line containing the cursor. */
extern void term_erase_current_line (void);

/** Work out how long a string will take on the display, if placed at
  the specified column, and allowing for tabs. */
extern uint8_t term_get_displayed_length (const char *line, uint8_t col);

void term_enable (BOOL enable);

void term_show_cursor (void);

void term_hide_cursor (void);

int term_init (void);
void term_set_raw (void);
void term_reset (void);
int term_get_key (int fd_in);

BOOL term_get_line (int fd_in, char *buff, int len, BOOL *interrupt);
void term_write_string (const char *s);
void term_write_buff (const char *from, int l);


