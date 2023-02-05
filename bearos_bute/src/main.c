#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "bute.h"

/*=========================================================================
  show_usage 
=========================================================================*/
static void show_usage (const char *argv0)
  {
  printf ("Usage: %s [options] [files...]}\n", argv0); 
  printf ("   -c N: set screen columns\n");
  printf ("   -h:   show this message\n");
  printf ("   -i:   disable auto-indent\n");
  printf ("   -r N: set screen rows\n");
  printf ("   -v:   show version\n");
  }

/*=========================================================================
  show_version
=========================================================================*/
static void show_version (const char *argv0)
  {
  printf ("%s for BearOS version " VERSION "\n", argv0); 
  printf ("Copyright (c)2022 Kevin Boone, GPL3\n");
  }

/*==========================================================================

  main 

==========================================================================*/
int main (int argc, char **argv)
  {
  int ret = 0;
  int opt;
  optind = 0;
  BOOL usage = FALSE;
  BOOL version = FALSE;
  BOOL noindent = FALSE;
  int fix_rows = -1;
  int fix_cols = -1;

  while (((opt = getopt (argc, argv, "hvir:c:")) != -1) && (ret == 0))
    {
    switch (opt)
      {
      case 'v':
        version = TRUE;
        break;
      case 'h':
        usage = TRUE;
        break;
      case 'i':
        noindent = TRUE;
        break;
      case 'r':
        fix_rows = atoi (optarg);
        break;
      case 'c':
        fix_cols = atoi (optarg);
        break;
      default:
        ret = -1; // TODO
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
    if (isatty (0))
      {
      ButeEnv *env = buteenv_create (noindent, fix_rows, fix_cols);

      if ((argc - optind == 0))
        {
	buteenv_add_editor (env, NULL);
	}
      else
	{
        for (int i = optind; i < argc; i++)
          {
	  buteenv_add_editor (env, argv[i]);
          }
	}
      buteenv_run (env);
      buteenv_destroy (env);
      }
    else
      {
      fprintf (stderr, "%s: Input is not a terminal\n", argv[0]);
      ret = ENOTTY;
      }
    }
  if (usage) ret = 0;
  return ret;
  }


