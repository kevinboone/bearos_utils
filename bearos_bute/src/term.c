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

#include <bearos/terminal.h>

// ANSI/VT100 control codes
#define TERM_CLEAR "\033[2J"
#define TERM_CLEAREOL "\033[K"
#define TERM_HOME "\033[1;1H"
#define TERM_CUR_BLOCK "\033[?6c"
#define TERM_ERASE_LINE "\033[K"
#define TERM_SET_CURSOR "\033[%d;%dH"
#define TERM_SHOW_CURSOR "\033[?25h"
#define TERM_HIDE_CURSOR "\033[?25l"

#define TERM_ROWS 23
#define TERM_COLS 80

#define TAB_SIZE 8

static BOOL enabled = TRUE;

/*==========================================================================

  term_write_char

==========================================================================*/
void term_write_char (char c)
  {
  putchar (c);
  fflush (stdout);
  }

/*==========================================================================

  term_write_string

==========================================================================*/
void term_write_string (const char *s)
  {
  fputs (s, stdout);
  fflush (stdout);
  }

/*==========================================================================

  term_enable

==========================================================================*/
void term_enable (BOOL enable)
  {
  enabled = enable;
  }

/*==========================================================================

  term_clear

==========================================================================*/
void term_clear (void)
  {
  term_write_string (TERM_CLEAR);
  }

/*==========================================================================

  term_show_cursor

==========================================================================*/
void term_show_cursor (void)
  {
  term_write_string (TERM_SHOW_CURSOR);
  }

/*==========================================================================

  term_hide_cursor

==========================================================================*/
void term_hide_cursor (void)
  {
  term_write_string (TERM_HIDE_CURSOR);
  }

/*==========================================================================

  term_clear

==========================================================================*/
void term_clear_and_home (void)
  {
  term_write_string (TERM_CLEAR);
  term_write_string (TERM_HOME);
  }

/*==========================================================================

  term_clear_eol

==========================================================================*/
void term_clear_eol (void)
  {
  term_write_string (TERM_CLEAREOL);
  }

/*==========================================================================

  term_get_size

==========================================================================*/
void term_get_size (int *rows, int *cols)
  {
#ifdef BEAROS
  DevCtlTermProps props;
  if (terminal_get_props (STDIN_FILENO, &props) == 0)
    {
    *rows = props.rows;
    *cols = props.cols;
    }
#else
  *rows = TERM_ROWS;
  *cols = TERM_COLS;
#endif
  }

/*==========================================================================

  term_set_cursor

==========================================================================*/
void term_set_cursor (uint8_t row, uint8_t col)
  {
  char buff[16];
  sprintf (buff, TERM_SET_CURSOR, row + 1, col + 1);
  term_write_string (buff);
  }

/*==========================================================================

  term_erase_current_line

==========================================================================*/
void term_erase_current_line (void)
  {
  term_write_string (TERM_ERASE_LINE);
  }

/*===========================================================================

  term_get_displayed_length

===========================================================================*/
uint8_t term_get_displayed_length (const char *line, uint8_t col)
  {
  uint8_t dlen = 0;
  int pos = 0;
  BOOL eol = FALSE;

  while (pos < col)
   {
   if (line[pos] == 0) eol = TRUE;
   if (eol)
     dlen++;
   else
     {
     if (line[pos] == '\t')
        {
        // TODO -- this logic only works with 8-space tabs
        dlen = (uint8_t) (dlen + TAB_SIZE);
        dlen &= 0xF8;
        }
     else
        dlen++;
     }
   pos++;
   }

  return dlen;
  }

/*=========================================================================

  term_get_line

=========================================================================*/
BOOL term_get_line (int fd_in, char *buff, int len, BOOL *interrupt)
  {
#ifdef BEAROS
  *interrupt = FALSE;
  int ret = terminal_get_line (fd_in, buff, len);
  if (ret == TGL_OK) return TRUE;
  if (ret == TGL_INTR) 
    {
    *interrupt = TRUE;
    return TRUE;
    }
  return FALSE;
#else
  int pos = 0;
  BOOL done = 0;
  BOOL got_line = TRUE;
  // The main input buffer
  String *sbuff = string_create_empty ();
  // A copy of the main input buffer, taken when we up-arrow back
  //  into the history. We might need to restore this on a down-arrow
  char *tempstr = NULL;

  while (!done)
    {
    int c = term_get_key (fd_in);
    if (c == VK_INTR)
      {
      got_line = TRUE;
      done = TRUE;
      *interrupt = TRUE;
      }
    else if (c == VK_EOI) 
      {
      got_line = FALSE;
      done = TRUE;
      }
    else if (c == VK_DEL || c == VK_BACK)
      {
      if (pos > 0) 
        {
        pos--;
        string_delete_c_at (sbuff, pos);
        term_write_char (O_BACKSPACE);
        const char *s = string_cstr (sbuff);
        int l = string_length (sbuff);
        for (int i = pos; i < l; i++)
          {
          term_write_char (s[i]);
          }
        term_write_char (' ');
        for (int i = pos; i <= l; i++)
          {
          term_write_char (O_BACKSPACE);
          }
        }
      }
    else if (c == VK_ENTER)
      {
      //buff[pos] = 0;
      done = 1;
      }
    else if (c == VK_LEFT)
      {
      if (pos > 0)
        {
        pos--;
        term_write_char (O_BACKSPACE);
        }
      }
    else if (c == VK_CTRLLEFT)
      {
      if (pos == 1)
        {
        pos = 0;
        term_write_char (O_BACKSPACE);
        }
      else
        {
        const char *s = string_cstr (sbuff);
        while (pos > 0 && isspace (s[(pos - 1)]))
          {
          pos--;
          term_write_char (O_BACKSPACE);
          }
        while (pos > 0 && !isspace (s[pos - 1]))
          {
          pos--;
          term_write_char (O_BACKSPACE);
          }
        }
      }
    else if (c == VK_CTRLRIGHT)
      {
      const char *s = string_cstr (sbuff);

      while (s[pos] != 0 && !isspace (s[pos]))
        {
        term_write_char (s[pos]);
        pos++;
        }
      while (s[pos] != 0 && isspace (s[pos]))
        {
        term_write_char (s[pos]);
        pos++;
        }
      }
    else if (c == VK_RIGHT)
      {
      const char *s = string_cstr (sbuff);
      int l = string_length (sbuff);
      if (pos < l)
        {
        term_write_char (s[pos]);
        pos++;
        }
      }
    else if (c == VK_UP)
      {
      continue;
      }
    else if (c == VK_DOWN)
      {
      continue;
      }
    else if (c == VK_HOME)
      {
      if (pos > 0)
        {
        for (int i = 0; i < pos; i++)
          term_write_char (O_BACKSPACE);
        pos = 0;
        }
      }
    else if (c == VK_END)
      {
      const char *s = string_cstr (sbuff);
      int l = string_length (sbuff);
      for (int i = pos; i < l; i++)
          term_write_char (s[i]);
      pos = l;
      }
    else
      {
      int l = string_length (sbuff);

      if (l < len - 1) // Leave room for null
        {
        if ((c < 256 && c >= 32) || (c == 8))
          {
          string_insert_c_at (sbuff, pos, (char)c);
          pos++;
          int l = string_length (sbuff);
          if (pos >= l - 1)
            term_write_char ((char)c); 
          else
            {
            const char *s = string_cstr (sbuff);
            for (int i = pos - 1; i < l; i++)
              term_write_char (s[i]); 
            for (int i = pos; i < l; i++)
              term_write_char (O_BACKSPACE); 
            }
          }
        }
      }
    }

  strncpy (buff, string_cstr(sbuff), len);
  string_destroy (sbuff);
   //printf ("buff='%s'\n", buff);

  if (tempstr) free (tempstr);

  puts (""); 

  if (got_line)
    return TRUE;
  else
    return FALSE;
#endif
  }


/*=========================================================================

  term_init

=========================================================================*/
int term_init (void)
  {
#ifdef BEAROS
  terminal_set_raw (STDIN_FILENO);
#else
 int fd = STDIN_FILENO;
  if (!isatty (fd)) return ENOTTY;
  //atexit(editorAtExit);
  if (tcgetattr (fd, &orig_termios) == -1) return ENOTTY; 
#endif
  return 0;
  }

/*==========================================================================

  term_write_buff

==========================================================================*/
void term_write_buff (const char *from, int l)
  {
  fwrite (from, l, 1, stdout);
  fflush (stdout);
  }

/*=========================================================================

  term_set_raw

=========================================================================*/
void term_set_raw (void)
  {
#ifdef BEAROS
  terminal_set_raw (STDIN_FILENO);
#else
  struct termios raw;
  raw = orig_termios;  /* modify the original mode */
  /* input modes: no break, no CR to NL, no parity check, no strip char,
   * no start/stop output control. */
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  /* output modes - disable post processing */
  //raw.c_oflag &= ~(OPOST);
  /* control modes - set 8 bit chars */
  raw.c_cflag |= (CS8);
  /* local modes - choing off, canonical off, no extended functions,
   * no signal chars (^Z,^C) */
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  /* control chars - set return condition: min number of bytes and timer. */
  //raw.c_cc[VMIN] = 0; /* Return each byte, or zero for timeout. */
  //raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */
  tcsetattr (0,TCSAFLUSH,&raw);
#endif
  }

/*=========================================================================

  term_reset

=========================================================================*/
void term_reset (void)
  {
#ifdef BEAROS
  terminal_reset (STDIN_FILENO, STDOUT_FILENO);
#else
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &orig_termios);
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
  (void)fd_in;
  char c = (char)getchar ();
  if (c == '\x1b')
    {
    char c1 = (char)getchar(); // TODO (I_ESC_TIMEOUT);
    if (c1 == (char)0xFF) return VK_ESC;
    //printf ("c1 a =%d %c\n", c1, c1);
    if (c1 == '[')
      {
      char c2 = (char) getchar();
      //printf ("c2 a =%d %c\n", c2, c2);
      if (c2 >= '0' && c2 <= '9')
        {
        char c3 = (char) getchar();
        //printf ("c3 a =%d %c\n", c3, c3);
        if (c3 == '~')
          {
          switch (c2)
            {
            case '0': return VK_END;
            case '1': return VK_HOME;
            case '2': return VK_INS;
            case '3': return VK_DEL; // Usually the key marked "del"
            case '5': return VK_PGUP;
            case '6': return VK_PGDN;
            }
          }
        else if (c3 == ';')
          {
          if (c2 == '1')
            {
            char c4 = (char) getchar (); // Modifier
            char c5 = (char) getchar (); // Direction
            //printf ("c4 b =%d %c\n", c4, c4);
            //printf ("c5 b =%d %c\n", c5, c5);
            if (c4 == '5') // ctrl
              {
              switch (c5)
                {
                case 'A': return VK_CTRLUP;
                case 'B': return VK_CTRLDOWN;
                case 'C': return VK_CTRLRIGHT;
                case 'D': return VK_CTRLLEFT;
                case 'H': return VK_CTRLHOME;
                case 'F': return VK_CTRLEND;
                }
              }
            else if (c4 == '2') // shift 
       {
              switch (c5)
                {
                case 'A': return VK_SHIFTUP;
                case 'B': return VK_SHIFTDOWN;
                case 'C': return VK_SHIFTRIGHT;
                case 'D': return VK_SHIFTLEFT;
                case 'H': return VK_SHIFTHOME;
                case 'F': return VK_SHIFTEND;
                }
              }
            else if (c4 == '6') // shift-ctrl
              {
              switch (c5)
                {
                case 'A': return VK_CTRLSHIFTUP;
                case 'B': return VK_CTRLSHIFTDOWN;
                case 'C': return VK_CTRLSHIFTRIGHT;
                case 'D': return VK_CTRLSHIFTLEFT;
                case 'H': return VK_CTRLSHIFTHOME;
                case 'F': return VK_CTRLSHIFTEND;
                }
              }
            else
              {
              // Any other modifier, we don't support. Just return
              //   the raw direction code
              switch (c5)
                {
                case 'A': return VK_UP;
                case 'B': return VK_DOWN;
                case 'C': return VK_RIGHT;
                case 'D': return VK_LEFT;
                case 'H': return VK_HOME;
                case 'F': return VK_END;
                }
              }
            }
          else return VK_ESC;
          }
        }
      else
        {
        //printf ("c2 b =%d %c\n", c2, c2);
        switch (c2)
          {
          case 'A': return VK_UP;
          case 'B': return VK_DOWN;
          case 'C': return VK_RIGHT;
          case 'D': return VK_LEFT;
          case 'H': return VK_HOME;
          case 'F': return VK_END;
          case 'Z': return VK_SHIFTTAB;
          }
        }
      }
    return '\x1b';
    }
 else
    {
    if (c == I_BACKSPACE) return VK_BACK;
    if (c == I_DEL) return VK_DEL;
    if (c == I_INTR) return VK_INTR;
    if (c == I_EOI) return VK_EOI;
    if (c == 13) return VK_ENTER;
    return c;
    }
#endif
  }



