/*=========================================================================

  bearcalc

  calc.c 
 
  Copyright (c)2022 Kevin Boone, GPL v3.0 

=========================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "config.h" 
#include "defs.h" 
#include "list.h"
#include "term.h"
#include "calc.h"
#include "tinyexpr.h"
#include "rationalize.h"

#ifdef BEAROS
#include <bearos/printf.h>
#endif

// User-defined variable/function list
te_variable vars[CALC_MAX_FUNCTIONS];
int nvars = 0;

// We must have at least x1, x2,... in the variable list, for the parser
//   not to complain when they are used as paramters. But they don't have
//   to reference any meaningful value.
double dummy1 = 0, dummy2 = 0, dummy3 = 0;

int sigfigs = 5;

CalcDisplayMode display_mode = CALC_DISPLAY_DEC;

/*=========================================================================
  calc_display_mode
=========================================================================*/
void calc_set_display_mode (CalcDisplayMode mode)
  {
  display_mode = mode;
  }

/*=========================================================================
  calc_result_err
=========================================================================*/
static const char *calc_result_err (double result)
  {
  if (result != result) return "nan";
  if (result < -DBL_MAX) return "inf";
  if (result > DBL_MAX) return "inf";
  return 0;
  }

/*=========================================================================
  calc_print_eng
=========================================================================*/
static void calc_print_eng (double result)
  {
  if (result == 0) { printf ("0\n"); return; }
  if (result < 0) { result = -result; printf ("-"); }

  int exp = (int) (floor (log10 (result	) / 3.0) * 3.0);
  double divisor = pow (10, exp);

  printf ("%g", result / divisor); 
  if (exp == 3)
    printf ("k\n");
  else if (exp == 6) 
    printf ("M\n");
  else if (exp == 9) 
    printf ("G\n");
  else if (exp == 12) 
    printf ("T\n");
  else if (exp == -3) 
    printf ("m\n");
  else if (exp == -6) 
    printf ("u\n");
  else if (exp == -9) 
    printf ("n\n");
  else if (exp == -12) 
    printf ("p\n");
  else
   if (exp == 0) { printf ("\n");} else { printf ("E%d\n", (int)exp); }
  }

/*=========================================================================
  calc_print_number
=========================================================================*/
static void calc_print_number (double result)
  {
  const char *err = calc_result_err (result);
  if (!err)
    {
    if (display_mode == CALC_DISPLAY_HEX)
      printf ("#%lX\n", (long)result);
    else if (display_mode == CALC_DISPLAY_RAT)
      {
      int whole, numerator, denominator;
      BOOL negative;
      rationalize (result, CALC_RAT_ORDER, FALSE, &whole, &numerator, 
	&denominator, &negative);
      if (negative) printf ("-");
      if (whole != 0)
	printf ("%d ", whole);
      if (numerator != 0)
	printf ("%d/%d", numerator, denominator);
      else
	printf ("0");
      printf ("\n");
      }
    else if (display_mode == CALC_DISPLAY_ENG)
      {
      calc_print_eng (result);
      }
    else
      {
      printf ("%.*g\n", sigfigs, result);
      }
    }
  else
    {
    printf ("%s\n", err);
    }
  }

/*=========================================================================
  calc_print_number
  Find the entry in the user variable/function table with the 
    specified name. If not found, return -1
=========================================================================*/
static int calc_find_var (const char *name)
  {
  for (int i = 0; i < nvars; i++)
    {
    char *testname = vars[i].name;
    if (testname && strcmp (testname, name) == 0) return i;
    }
  return -1;
  }

/*=========================================================================
  calc_delete_closure
=========================================================================*/
static void calc_delete_closure (const char *name)
  {
  int i = calc_find_var (name);
  if (i >= 0)
    {
    free (vars[i].name);
    te_free (vars[i].context);
    vars[i].name = NULL;
    }
  }

/*=========================================================================
  calc_do_closure0
  Execute a user-defined function. On entry, context is a pointer to the
    te_expr object that represent's the compiled function.
=========================================================================*/
static double calc_do_closure0 (void *context)
  {
  te_expr *n = (te_expr *)context;
  double result = te_eval (n);
  return result;
  }

/*=========================================================================
  calc_do_closure1
  Execute a user-defined function. On entry, context is a pointer to the
    te_expr object that represent's the compiled function.
=========================================================================*/
static double calc_do_closure1 (double x, void *context)
  {
  te_expr *n = (te_expr *)context;
  int i = calc_find_var ("x");
  double *addr_x = vars[i].address;
  double old_x = *addr_x;
  *addr_x = x; 
  double result = te_eval (n);
  *addr_x = old_x;
  return result;
  }

/*=========================================================================
  calc_do_closure2
  Execute a user-defined function. On entry, context is a pointer to the
    te_expr object that represent's the compiled function.
=========================================================================*/
static double calc_do_closure2 (double x, double y, void *context)
  {
  te_expr *n = (te_expr *)context;
  int i = calc_find_var ("x");
  double *addr_x = vars[i].address;
  double old_x = *addr_x;
  *addr_x = x; 
  i = calc_find_var ("y");
  double *addr_y = vars[i].address;
  double old_y = *addr_y;
  *addr_y = y; 
  double result = te_eval (n);
  *addr_x = old_x;
  *addr_y = old_y;
  return result;
  }


/*=========================================================================
  calc_do_closure3
  Execute a user-defined function. On entry, context is a pointer to the
    te_expr object that represent's the compiled function.
=========================================================================*/
static double calc_do_closure3 (double x, double y, double z, void *context)
  {
  te_expr *n = (te_expr *)context;
  int i = calc_find_var ("x");
  double *addr_x = vars[i].address;
  double old_x = *addr_x;
  *addr_x = x; 
  i = calc_find_var ("y");
  double *addr_y = vars[i].address;
  double old_y = *addr_y;
  *addr_y = y; 
  i = calc_find_var ("z");
  double *addr_z = vars[i].address;
  double old_z = *addr_z;
  *addr_z = z; 
  double result = te_eval (n);
  *addr_x = old_x;
  *addr_y = old_y;
  *addr_z = old_z;
  return result;
  }

/*=========================================================================
  calc_warn_too_many_functions
=========================================================================*/
static void calc_warn_too_many_funtions (void)
  {
  printf ("Warning: too many functions defined\n");
  }

/*=========================================================================
  calc_add_closure0
=========================================================================*/
static void calc_add_closure0 (const char *name, te_expr *n)
  {
  if (nvars >= CALC_MAX_FUNCTIONS) 
    {
    calc_warn_too_many_funtions(); 
    return;
    }
  calc_delete_closure (name);
  vars[nvars].name = strdup (name);
  vars[nvars].type = TE_CLOSURE0;
  vars[nvars].address = calc_do_closure0;
  vars[nvars].context = n;
  nvars++;
  }

/*=========================================================================
  calc_add_closure1
=========================================================================*/
static void calc_add_closure1 (const char *name, te_expr *n)
  {
  if (nvars >= CALC_MAX_FUNCTIONS) 
    {
    calc_warn_too_many_funtions(); 
    return;
    }
  calc_delete_closure (name);
  vars[nvars].name = strdup (name);
  vars[nvars].type = TE_CLOSURE1;
  vars[nvars].address = calc_do_closure1;
  vars[nvars].context = n;
  nvars++;
  }

/*=========================================================================
  calc_add_closure2
=========================================================================*/
static void calc_add_closure2 (const char *name, te_expr *n)
  {
  if (nvars >= CALC_MAX_FUNCTIONS) 
    {
    calc_warn_too_many_funtions(); 
    return;
    }
  calc_delete_closure (name);
  vars[nvars].name = strdup (name);
  vars[nvars].type = TE_CLOSURE2;
  vars[nvars].address = calc_do_closure2;
  vars[nvars].context = n;
  nvars++;
  }

/*=========================================================================
  calc_add_closure3
=========================================================================*/
static void calc_add_closure3 (const char *name, te_expr *n)
  {
  if (nvars >= CALC_MAX_FUNCTIONS) 
    {
    calc_warn_too_many_funtions(); 
    return;
    }
  calc_delete_closure (name);
  vars[nvars].name = strdup (name);
  vars[nvars].type = TE_CLOSURE3;
  vars[nvars].address = calc_do_closure3;
  vars[nvars].context = n;
  nvars++;
  }

/*=========================================================================
  calc_init_vars
  We need to have at least x, y, and z defined at all times, as these are
    used as the arguments to user-defined functions. The TE compiler won't
    compile an expression with an undefined variable, even if we plan
    to assign a value before evalation.
=========================================================================*/
void calc_init_vars (void)
  {
  vars[0].name = strdup ("x");
  vars[0].type = TE_VARIABLE;
  vars[0].address = &dummy1;
  vars[0].context = NULL;
  nvars++;
  vars[1].name = strdup ("y");
  vars[1].type = TE_VARIABLE;
  vars[1].address = &dummy2;
  vars[1].context = NULL;
  nvars++;
  vars[2].name = strdup ("z");
  vars[2].type = TE_VARIABLE;
  vars[2].address = &dummy3;
  vars[2].context = NULL;
  nvars++;
  }

/*=========================================================================
  calc_free_vars
=========================================================================*/
void calc_free_vars (void)
  {
  for (int i = 0; i < nvars; i++)
    {
    if (vars[i].name)
      {
      free (vars[i].name);
      te_free (vars[i].context);
      }
    }
  }

/*=========================================================================
  calc_show_status
=========================================================================*/
void calc_show_status (void)
  {
  printf ("Angle mode is %s\n", 
    te_get_angle_mode() == TE_ANGLE_DEG ? "[deg]rees" : "[rad]ians");
  printf ("Display mode is "); 
  switch (display_mode)
    {
    case CALC_DISPLAY_ENG: printf ("[eng]ineering"); break;
    case CALC_DISPLAY_HEX: printf ("[hex]adecimal"); break;
    case CALC_DISPLAY_RAT: printf ("[rat]io"); break;
    default: printf ("[dec]imal"); break;
    }
  printf ("\n");
  printf ("Scale is %d significant figures\n", sigfigs);
  // When counting variables, remove the x, y, z that are always present.
  printf ("%d user funtions/variables defined\n", nvars - 3);
  }

/*=========================================================================
  calc_help
=========================================================================*/
void calc_help (void)
  {
  printf 
("Enter math expressions. Bearcal supports the usual symbols and function\n");
  printf 
("names. To enter hexadecimal numbers, use '#' as a prefix. 'rad' and deg'\n");
  printf 
("set the angle mode for trig functions. `dec`, `hex`, and `rat` (ratio)\n");
  printf 
("set the display mode. Enter 'quit' to exit.\n");
  }

/*=========================================================================
  calc_process_line
  Note: the caller should have trimmed whitespace from the left. We 
    won't check it again here. Returns TRUE if the program should
    continue (no fatal error or user quit).
=========================================================================*/
BOOL calc_process_line (const char *file, const char *line, 
       int linenum, BOOL *error)
  {
  char *scpos = strchr (line, ';');
  if (scpos) *scpos = 0;
  if (!line[0]) return TRUE; 
  BOOL ret = TRUE;
  *error = FALSE;
  if (strcmp (line, "hex") == 0)
    {
    calc_set_display_mode (CALC_DISPLAY_HEX);
    }
  else if (strcmp (line, "dec") == 0)
    {
    calc_set_display_mode (CALC_DISPLAY_DEC);
    }
  else if (strcmp (line, "eng") == 0)
    {
    calc_set_display_mode (CALC_DISPLAY_ENG);
    }
  else if (strcmp (line, "rat") == 0)
    {
    calc_set_display_mode (CALC_DISPLAY_RAT);
    }
  else if (strcmp (line, "deg") == 0)
    {
    te_set_angle_mode (TE_ANGLE_DEG);
    }
  else if (strcmp (line, "rad") == 0)
    {
    te_set_angle_mode (TE_ANGLE_RAD);
    }
  else if (strcmp (line, "help") == 0)
    {
    calc_help();
    }
  else if (strncmp (line, "scale ", 6) == 0)
    {
    sigfigs = atoi (line + 6);
    }
  else if (strcmp (line, "quit") == 0)
    {
    ret = FALSE;
    }
  else if (strcmp (line, "stat") == 0)
    {
    calc_show_status();
    }
  else
    {
    char *colpos = strrchr (line, ':');
    const char *expr_start;
    if (colpos)
      {
      expr_start = colpos + 1;
      }
    else
      expr_start = line;

    int err;
    te_expr *n = te_compile (expr_start, vars, nvars, &err);
    if (n)
      {
      if (!colpos)
        {
        double result = te_eval_and_save (n);
        calc_print_number (result);
        te_free (n);
        }
      else
        {
        // The line has a colon in it. But does it have two colons?
        //   foo:2:x+2...
        char name[CALC_MAX_VARNAME]; 
        int namelen = colpos - line;
        if (namelen >= (int)sizeof (name) - 1) namelen = sizeof(name) - 1;
        strncpy (name, line, namelen);
        name[namelen] = 0;
        // name is now foo:2
        
        const char *fn_name;
        char *colpos2 = strchr (name, ':');
        int nparams;
        if (colpos2)
          {
          *colpos2 = 0;
          fn_name = name;
          nparams = atoi (colpos2 + 1);
          }
        else
          {
          fn_name = name;
          nparams = 0;
          }

        if (nparams > 3) nparams = 3; 
        switch (nparams)
          {
          case 1: calc_add_closure1 (fn_name, n); break;
          case 2: calc_add_closure2 (fn_name, n); break;
          case 3: calc_add_closure3 (fn_name, n); break;
          default: calc_add_closure0 (fn_name, n); 
          }
        }
      }    
    else
      {
      if (file)
        printf ("Syntax error, file '%s', line %d:\n%s\n", file, 
          linenum, expr_start);
      else
        printf ("Syntax error:\n%s\n", expr_start);
      for (int i = 0; i < err; i++) printf (" ");
      printf ("^\n");
      *error = TRUE;
      }
    }
  return ret;
  }

/*=========================================================================
  calc_do_file
=========================================================================*/
BOOL calc_do_file (const char *filename)
  {
  BOOL ret = TRUE;
  FILE *f = fopen (filename, "r");
  if (f)
    {
    char *line = malloc (100);
    size_t n = 100;
    int linenum = 1;
    while (getline (&line, &n, f) > 0 && ret == TRUE)
      {
      BOOL error;
      int l = strlen (line);
      if (l > 0)
        {
        if (line[l - 1] == '\n') line[l - 1] = 0;
        if (calc_process_line (filename, line, linenum, &error) == FALSE)
          ret = FALSE;
        }
      linenum++;
      }
    free (line);
    fclose (f);
    }
  else
    printf ("Can't open file '%s' for reading: %s\n", filename, 
      strerror (errno));
  return ret;
  }


