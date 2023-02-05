/*==========================================================================

  bearcalc

  calc.h

  Functions for evaluating expressions and commands, and displaying 
  results.

  (c)2022 Kevin Boone, GPLv3.0

==========================================================================*/
#pragma once

#include <stdint.h> 
#include "defs.h"

// Number display modes
typedef enum
  {
  CALC_DISPLAY_DEC = 0,
  CALC_DISPLAY_HEX,
  CALC_DISPLAY_RAT,
  CALC_DISPLAY_ENG
  } CalcDisplayMode;

BEGIN_DECLS

/** Process a line, and display the result, if any. The line may be
    an expression, a command, or an assignment. If there is a 
    syntax error, the function displays an error message, and sets
    *error to TRUE. Return value is TRUE unless the line contains
    the command 'quit'. The 'file' and 'linenum' arguments are only 
    used when display error messages, and file can be NULL. */
extern BOOL calc_process_line (const char *file, const char *line, 
              int linenum, BOOL *error);

/** Set the display mode to one of the CALC_DISPLAY_XXX constants. */
extern void calc_set_display_mode (CalcDisplayMode mode);

/** Initialize the variable table, which is also used for user-defined
    functions. */
extern void calc_init_vars (void);

/** Clean up memory associated with the variable table. */
extern void calc_free_vars (void);

/** Process a file, line-by-line. Returns TRUE unless the file contains
    a 'quit' command. */ 
extern BOOL calc_do_file (const char *filename);


END_DECLS


