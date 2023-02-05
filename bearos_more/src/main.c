#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef BEAROS
#include <bearos/compat.h>
#include <bearos/terminal.h>
#include <bearos/printf.h>
#endif
#include "term.h"

int screen_cols = 80; // TODO
int screen_rows = 25; // TODO
int rows = 0; // Rows printed in this page

#define TRUE 1
#define FALSE 0
typedef int BOOL;

#ifdef BEAROS
#define printf printf_
#endif

typedef enum
  {
  ACTION_LINE, ACTION_PAGE, ACTION_QUIT
  } Action;

/*==========================================================================
  prompt 
==========================================================================*/
static Action prompt (int fd_ctrl)
  {
  printf ("more....");
#ifndef BEAROS
  fflush (stdout);
#endif
  // TODO: we must get the character from the terminal, not stdin,
  //   which might be the input source
  int c = term_get_key (fd_ctrl);
  printf ("\x0D");
  printf ("        ");
  printf ("\x0D");
#ifdef BEAROS
  if (c == 'q' || c == 3 /* ctrl-c */ || c == VK_INTR)
#else
  if (c == 'q' || c == 3 /* ctrl-c */)
#endif
    return ACTION_QUIT;
  if (c == 10)
    return ACTION_LINE;
  return ACTION_PAGE;
  }

/*==========================================================================
  do_fragment
==========================================================================*/
static int do_fragment (int fd_ctrl, const char *s)
  {
  printf ("%s\n", s);
  rows++;
  if (rows == screen_rows - 1)
    {
    Action a = prompt (fd_ctrl);
    switch (a)
      {
      case ACTION_QUIT:
        return -1; 
        break;
      case ACTION_LINE:
        rows --;
        break;
      case ACTION_PAGE:
        rows = 0; 
        break;
      }
    }
  return 0;
  }

/*==========================================================================
  do_line
==========================================================================*/
static int do_line (int fd_ctrl, char *line, int n)
  {
  int remain = n; 
  int pos = 0;
  while (remain >= 0)
    {
    if (remain < screen_cols)
      {
      if (do_fragment (fd_ctrl, line + pos) != 0) return -1;
      }
    else
      {
      char orig = line[pos + screen_cols];
      line[pos + screen_cols] = 0;
      if (do_fragment (fd_ctrl, line + pos) != 0) return -1;
      line[pos + screen_cols] = orig;
      }
    remain -= screen_cols;
    pos += screen_cols;
    }
  return 0;
  }

/*==========================================================================
  do_fd
==========================================================================*/
static int do_fp (int fd_ctrl, FILE *fp)
  {
  char *line = NULL;
  size_t len = 0;
  int n = 0;
  BOOL stop = FALSE;

  while (((n = getline (&line, &len, fp)) > 0) && !stop)
    {
    if (line[n - 1] == 10) 
      {
      line[n - 1] = 0;
      n--;
      }
    if (do_line (fd_ctrl, line, n) != 0) stop = TRUE;
    }
  free (line);

  return 0;
  }

/*==========================================================================
  do_file
==========================================================================*/
static int do_file (int fd_ctrl, const char *argv0, const char *filename)
  {
  int ret;
  FILE *fp = fopen (filename, "r");
  if (fp)
    {
    ret = do_fp (fd_ctrl, fp);
    fclose (fp);
    }
  else
    {
#ifdef BEAROS
    printf_stderr_ ("%s: %s: %s\n", argv0, filename, strerror (errno));
#else
    fprintf (stderr, "%s: %s: %s\n", argv0, filename, strerror (errno));
#endif
    ret = errno;
    }
  return ret;
  }

/*=========================================================================
  show_usage 
=========================================================================*/
static void show_usage (const char *argv0)
  {
  printf ("Usage: %s [options] [files...]\n", argv0); 
  printf ("   -v: show version\n");
  printf ("   -h: show this message\n");
  printf ("   -r: rows in display\n");
  printf ("   -c: columns in display\n");
  }

/*=========================================================================
  show_version
=========================================================================*/
static void show_version (const char *argv0)
  {
  printf ("%s for BearOS version 0.1\n", argv0); 
  printf ("Copyright (c)2022 Kevin Boone, GPL3\n");
  }

/*==========================================================================
  main
==========================================================================*/
int main (int argc, char **argv)
  {
  int opt;
  int ret = 0;
  optind = 0;
  BOOL usage = FALSE;
  BOOL version = FALSE;
  int set_screen_rows = -1; 
  int set_screen_cols = -1; 
  rows = 0;

#ifdef BEAROS
  char *ctrl_dev = BEAROS_CONTROLLING_TTY; // typically p:/con 
#else
  char *ctrl_dev = "/dev/tty"; 
#endif

  if (!isatty (1))
    {
#ifdef BEAROS
    printf_stderr_ ("%s: stdout is not a tty\n", argv[0]);
#else
    fprintf (stderr, "%s: stdout is not a tty\n", argv[0]);
#endif
    return EINVAL;
    }

  while (((opt = getopt (argc, argv, "hvr:c:")) != -1) && (ret == 0))
    {
    switch (opt)
      {
      case 'v':
	version = TRUE;
	break;
      case 'h':
	usage = TRUE;
	break;
      case 'r':
	set_screen_rows = atoi (optarg); 
	break;
      case 'c':
	set_screen_cols = atoi (optarg); 
	break;
      default:
	ret = -1; 
      }
    }

  if (usage)
    {
    ret = -1;
    show_usage (argv[0]);
    }  

  if (version)
    {
    ret = -1;
    show_version (argv[0]);
    }  

  if (ret == 0)
    {
    if (set_screen_rows == 0 || set_screen_cols == 0) // atoi ret zero on fail
      {
#ifdef BEAROS
      printf_stderr_ ("%s: Invalid screen size\n", argv[0]);
#else
      fprintf (stderr, "%s: Invalid screen size\n", argv[0]);
#endif
      ret = EINVAL;
      }
    }

  if (ret == 0)
    {
    int r, c;
    term_get_size (STDOUT_FILENO, &r, &c); 
    if (set_screen_rows > 2) 
      screen_rows = set_screen_rows;
    else
      screen_rows = r;
    if (set_screen_cols > 2) 
      screen_cols = set_screen_cols;
    else
      screen_cols = c;

    int f_control = open (ctrl_dev, O_RDONLY);
    if (f_control >= 0)
      {
      ret = term_init (f_control); 
      if (ret == 0)
        {
	term_set_raw (f_control);
	if ((argc - optind == 0))
	  {
	  FILE *fp = fdopen (0, "r");
	  ret = do_fp (f_control, fp);  
	  //fclose (fp);
	  }
	else
	  {
	  for (int i = optind; i < argc && ret == 0; i++)
	    {
	    ret = do_file (f_control, argv[0], argv[i]);
	    }
	  }
	term_reset (f_control, f_control); 
        }
      else
        {
#ifdef BEAROS
        printf_stderr_ ("%s: Can't initalize controlling terminal %s\n", 
#else
        fprintf (stderr, "%s: Can't initalize controlling terminal %s\n", 
#endif
	   argv[0], ctrl_dev);
        }
      close (f_control);
      }
    else
      {
#ifdef BEAROS
      printf_stderr_ ("%s: Can't open controlling terminal %s\n", 
#else
      fprintf (stderr, "%s: Can't open controlling terminal %s\n", 
#endif
	 argv[0], ctrl_dev);
      }
    }

  if (usage) ret = 0;
  return ret;
  }

