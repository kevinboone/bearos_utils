/*=========================================================================

  bearcalc

  main.c 
 
  General handy definitions

  Copyright (c)2022 Kevin Boone, GPL v3.0 

=========================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h" 
#include "defs.h" 
#include "list.h"
#include "term.h"
#include "calc.h"

#ifdef BEAROS
#include <bearos/printf.h> 
#endif

#include <bearos/terminal.h> // For key codes

/*=========================================================================
  show_usage 
=========================================================================*/
static void show_usage (const char *argv0)
  {
  printf ("Usage: %s [options] [expression]\n", argv0); 
  printf ("   -f: process a file\n");
  printf ("   -v: show version\n");
  }

/*=========================================================================
  show_version
=========================================================================*/
static void show_version (const char *argv0)
  {
  printf ("%s for BearOS version " CALC_VERSION "\n", argv0); 
  printf ("Copyright (c)2022 Kevin Boone, GPL3\n");
  }

/*=========================================================================
  do_interactive 
=========================================================================*/
static void do_interactive (void)
  {
  BOOL stop = FALSE;
  List *history = list_create (free);
  printf ("Bearcalc version " CALC_VERSION ".\n");
  printf ("Enter 'help' for brief instructions.\n"); 
  while (!stop)
    {
    char s[CALC_MAX_LINE]; 
    printf ("> ");
    fflush (stdout);
    if (term_get_line (STDIN_FILENO, s, sizeof (s), 100, history) == TGL_OK)
      {
      char *ss = s;
      while (isspace (*ss) && *ss) ss++;
      BOOL error;
      if (ss[0])
        if (calc_process_line (NULL, ss, 0, &error) == FALSE) stop = TRUE;
      }
    else
      stop = TRUE;
    }
  list_destroy (history);
  }

/*=========================================================================
  main 
=========================================================================*/
int main (int argc, char **argv) 
  {
  int opt;
  int ret = 0;
  optind = 0;
  BOOL usage = FALSE;
  BOOL version = FALSE;
  BOOL stop = FALSE;

  calc_init_vars();
  while (((opt = getopt (argc, argv, "hvf:")) != -1) && (ret == 0))
    {
    switch (opt)
      {
      case 'v':
        version = TRUE;
        break;
      case 'h':
        usage = TRUE;
        break;
      case 'f':
        if (calc_do_file (optarg) == FALSE) stop = true;
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

  if (ret == 0 && !stop)
    {
    if ((argc - optind >= 1))
      {
      char *line = malloc(1);
      line[0] = 0;
      int len = 1;
      for (int i = optind; i < argc; i++)
        {
        len = len + strlen (argv[1]) + 1; 
        line = realloc (line, len);
        strcat (line, argv[i]);
        if (i != argc - 1)
          strcat (line, " "); 
        }
      BOOL error;
      calc_process_line (NULL, line, 0, &error);
      free (line);
      }  
    else
      {
      do_interactive ();
      }
    }
 
  calc_free_vars();
  if (usage) ret = 0;
  return ret;
  }


