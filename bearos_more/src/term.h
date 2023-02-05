/*==========================================================================

  term.h

  Functions for interacting with ANSI-like terminals

  (c)2021 Kevin Boone, GPLv3.0

==========================================================================*/
#pragma once

#include <stdint.h> 

extern void term_get_size (int fd_in, int *rows, int *cols);
int term_init (int fd_in);
void term_set_raw (int fd_in);
void term_reset (int fd_in, int fd_out);
int term_get_key (int fd_in);



