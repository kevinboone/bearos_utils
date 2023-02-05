/*==========================================================================

  bearcalc

  term.h

  Functions for interacting with ANSI-like terminals

  (c)2021 Kevin Boone, GPLv3.0

==========================================================================*/
#pragma once

#include <stdint.h> 
#include "defs.h"
#include "list.h"

BEGIN_DECLS

extern void term_get_size (int *rows, int *cols);

extern int term_init (int fd);

extern void term_set_raw (int fd);

extern void term_reset (int fd);

extern int term_get_key (int fd_in);

extern int term_get_line (int fd_in, char *buff, int len, 
        int max_history, List *history);

END_DECLS

