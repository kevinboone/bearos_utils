/*==========================================================================

  bearcalc

  term.c

  Functions for interacting with ANSI-like terminals

  Note that most of the code in this file is for the Linux version of
  Bearcalc. BearOS does not use the same API for controlling terminal
  behaviour.

  (c)2021-2 Kevin Boone, GPLv3.0

==========================================================================*/
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h> 
#include <ctype.h> 
#include <errno.h> 
#include <unistd.h> 
#include "term.h"
#include "mystring.h"

#if BEAROS
#include <bearos/compat.h>
#include <bearos/devctl.h>
#include <bearos/printf.h>
extern int _puts (const char *s);
#else
#include <termios.h>
static struct termios orig_termios;
#endif

#include <bearos/terminal.h> // For key codes

#define TERM_ROWS 23
#define TERM_COLS 80

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

/*============================================================================
 * term_add_line_to_history 
 * ==========================================================================*/
static void term_add_line_to_history (List *history, int max_history, 
        const char *buff)
  {
  BOOL should_add = TRUE;
  int l = list_length (history);
  if (l < max_history)
    {
    for (int i = 0; i < l && should_add; i++)
      {
      if (strcmp (buff, list_get (history, i)) == 0)
        should_add = FALSE;
      }
    }

  if (should_add)
    {
    if (l >= max_history)
      {
      const char *s = list_get (history, 0);
      list_remove_object (history, s);
      }
    list_append (history, strdup (buff));
    }
  }

/*=========================================================================

  term_get_line

=========================================================================*/
int term_get_line (int fd_in, char *buff, int len, 
        int max_history, List *history)
  {
  int pos = 0;
  bool done = 0;
  bool got_line = true;
  buff[0] = 0; // Make sure that something is written as the output
  // The main input buffer
  String *sbuff = string_create_empty ();
  // A copy of the main input buffer, taken when we up-arrow back
  //  into the history. We might need to restore this on a down-arrow
  char *tempstr = NULL;

  bool interrupt = false;

  int histpos = -1;

  term_init (STDIN_FILENO);
  term_set_raw (STDIN_FILENO);

  while (!done)
    {
    int c = term_get_key (fd_in);
    if (c == VK_INTR)
      {
      got_line = true; // This isn't the end of input (necessarily).
      done = true;
      interrupt = true;
      }
    else if (c == VK_EOI) 
      {
      got_line = false;
      done = true;
      }
    else if (c == VK_DEL || c == VK_BACK)
      {
      if (pos > 0) 
        {
        pos--;
        string_delete_c_at (sbuff, pos);
        putchar (O_BACKSPACE);
        const char *s = string_cstr (sbuff);
        int l = string_length (sbuff);
        for (int i = pos; i < l; i++)
          {
          putchar (s[i]);
          }
        putchar (' ');
        for (int i = pos; i <= l; i++)
          {
          putchar (O_BACKSPACE);
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
        putchar (O_BACKSPACE);
        }
      }
    else if (c == VK_CTRLLEFT)
      {
      if (pos == 1)
        {
        pos = 0;
        putchar (O_BACKSPACE);
        }
      else
        {
        const char *s = string_cstr (sbuff);
        while (pos > 0 && isspace ((int)s[(pos - 1)]))
          {
          pos--;
          putchar (O_BACKSPACE);
          }
        while (pos > 0 && !isspace ((int)s[pos - 1]))
          {
          pos--;
          putchar (O_BACKSPACE);
          }
        }
      }
    else if (c == VK_CTRLRIGHT)
      {
      const char *s = string_cstr (sbuff);

      while (s[pos] != 0 && !isspace ((int)s[pos]))
        {
        putchar (s[pos]);
        pos++;
        }
      while (s[pos] != 0 && isspace ((int)s[pos]))
        {
        putchar (s[pos]);
        pos++;
        }
      }
    else if (c == VK_RIGHT)
      {
      const char *s = string_cstr (sbuff);
      int l = string_length (sbuff);
      if (pos < l)
        {
        putchar (s[pos]);
        pos++;
        }
      }
    else if (c == VK_UP)
      {
      if (!history) continue;
      if (histpos == 0) continue;
      int histlen = list_length (history);
      if (histlen == 0) continue;
      //printf ("histlen=%d histpos=%d\n", histlen, histpos);

      if (histpos == -1)
        {
        // We are stepping out of the main line, into the
        //  top of the history
        if (!tempstr)
            tempstr = strdup (string_cstr (sbuff));
        histpos = histlen - 1;
         }
      else
        {
        histpos --;
        }

      int oldlen = string_length (sbuff);
      const char *newline = list_get (history, histpos); 
      int newlen = (int)strlen (newline);
      // Move to the start of the line 
       for (int i = 0; i < pos; i++)
        putchar (O_BACKSPACE);
      // Write the new line
      for (int i = 0; i < newlen; i++)
        putchar (newline[i]);
      // Erase from the end of the new line to the end of the old 
      for (int i = newlen; i < oldlen; i++)
        putchar (' ');
      for (int i = newlen; i < oldlen; i++)
        putchar (O_BACKSPACE);
      pos = newlen;
      string_destroy (sbuff);
      sbuff = string_create (newline);
      }
    else if (c == VK_DOWN)
      {
      if (!history) continue;
      int histlen = list_length (history);
      if (histpos < 0) continue; 
      char *newline = "";
      bool restored_temp = false;
      if (histpos == histlen - 1)
        {
        // We're about to move off the end of the history, back to 
        //   the main line. So restore the main line, if there is 
        //   one, or just set it to "" if there is not
        histpos = -1;
        if (tempstr)
          {
          newline = tempstr;
          restored_temp = true;
          }
        }
      else
        {
        restored_temp = false;
        histpos++;
        newline = list_get (history, histpos); 
        }

      int oldlen = string_length (sbuff);
      int newlen = (int)strlen (newline);
      // Move to the start of the line 
       for (int i = 0; i < pos; i++)
          putchar (O_BACKSPACE);
      // Write the new line
      for (int i = 0; i < newlen; i++)
          putchar (newline[i]);
        // Erase from the end of the new line to the end of the old 
      for (int i = newlen; i < oldlen; i++)
          putchar (' ');
      for (int i = newlen; i < oldlen; i++)
          putchar (O_BACKSPACE);
      pos = newlen;
      string_destroy (sbuff);
      sbuff = string_create (newline);
      if (restored_temp)
        {
        free (tempstr); 
        tempstr = NULL;
        }
      }
    else if (c == VK_HOME)
      {
      if (pos > 0)
        {
        for (int i = 0; i < pos; i++)
          putchar (O_BACKSPACE);
        pos = 0;
        }
      }
    else if (c == VK_END)
      {
      const char *s = string_cstr (sbuff);
      int l = string_length (sbuff);
      for (int i = pos; i < l; i++)
          putchar (s[i]);
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
          //if (pos >= l - 1)
          if (pos >= l)
            putchar ((char)c); 
          else
            {
            const char *s = string_cstr (sbuff);
            for (int i = pos - 1; i < l; i++)
              putchar (s[i]); 
            for (int i = pos; i < l; i++)
              putchar (O_BACKSPACE); 
            }
          }
        }
      }
    fflush (stdout);
    }

  strncpy (buff, string_cstr(sbuff), (long unsigned int)len);
  string_destroy (sbuff);

  if (tempstr) free (tempstr);

  printf (O_ENDL_STR); 
  histpos = -1; 

  term_reset (STDIN_FILENO);

  if (got_line)
    {
    if (history && !interrupt && buff[0])
      {
      term_add_line_to_history (history, max_history, buff);
      }
    if (interrupt)
      {
      buff[0] = 0;
      return TGL_INTR;
      }
    else
      return TGL_OK;
    }
  else
    return TGL_EOI;
  }

/*=========================================================================

  term_init

=========================================================================*/
int term_init (int fd)
  {
#ifdef BEAROS
  terminal_set_raw (fd);
#else
  if (!isatty (fd)) return ENOTTY;
  //atexit(editorAtExit);
  if (tcgetattr (fd, &orig_termios) == -1) return ENOTTY; 
#endif
  return 0;
  }

/*=========================================================================

  term_set_raw

=========================================================================*/
void term_set_raw (int fd)
  {
#ifdef BEAROS
  terminal_set_raw (fd);
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
  tcsetattr (fd, TCSAFLUSH,&raw);
#endif
  }

/*=========================================================================

  term_reset

=========================================================================*/
void term_reset (int fd)
  {
#ifdef BEAROS
  terminal_reset (fd, STDOUT_FILENO);
#else
  tcsetattr (fd, TCSAFLUSH, &orig_termios);
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



