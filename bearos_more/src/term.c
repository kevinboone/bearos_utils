/*==========================================================================

  term.c

  Functions for interacting with ANSI-like terminals

  (c)2021 Kevin Boone, GPLv3.0

==========================================================================*/
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h> 
#include <ctype.h> 
#include <errno.h> 
#include <unistd.h> 
#include "term.h"
#include "string.h"

#if BEAROS
#include <bearos/printf.h>
#include <bearos/compat.h>
#include <bearos/devctl.h>
#include <bearos/terminal.h>
#else
#include <termios.h>
static struct termios orig_termios;
#endif

// Defaults
#define TERM_ROWS 23
#define TERM_COLS 80

/*==========================================================================

  term_get_size

==========================================================================*/
void term_get_size (int fd_out, int *rows, int *cols)
  {
#ifdef BEAROS
  DevCtlTermProps props;
  if (terminal_get_props (fd_out, &props) == 0)
    {
    *rows = props.rows;
    *cols = props.cols;
    }
#else
  // TODO: get the real size using an ioctl() call
  (void)fd_out;
  *rows = TERM_ROWS;
  *cols = TERM_COLS;
#endif
  }

/*=========================================================================

  term_init

=========================================================================*/
int term_init (int fd_in)
  {
#ifdef BEAROS
  terminal_set_raw (fd_in);
#else
  if (!isatty (fd_in)) return ENOTTY;
  int ret = tcgetattr (fd_in, &orig_termios);
  if (ret < 0) return errno; 
#endif
  return 0;
  }

/*=========================================================================

  term_set_raw

=========================================================================*/
void term_set_raw (int fd_in)
  {
#ifdef BEAROS
  terminal_set_raw (fd_in);
#else
  struct termios raw;
  raw = orig_termios;  
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  tcsetattr (fd_in, TCSAFLUSH, &raw);
#endif
  }

/*=========================================================================

  term_reset

=========================================================================*/
void term_reset (int fd_in, int fd_out)
  {
#ifdef BEAROS
  terminal_reset (fd_in, fd_out);
#else
  (void)fd_out;
  tcsetattr (fd_in, TCSAFLUSH, &orig_termios);
#endif
  }

/*=========================================================================

  term_get_key

=========================================================================*/
int term_get_key (int fd_in)
  {
#ifdef BEAROS
  return terminal_get_key (fd_in);
#else
  char c;
  read (fd_in, &c, 1);
  return c;
#endif
  }

