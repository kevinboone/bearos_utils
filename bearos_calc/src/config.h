/*=========================================================================

  bearcalc

  config.h 

  Various limits that affect the operation of Bearcalc.
 
  Copyright (c)2022 Kevin Boone, GPL v3.0 

=========================================================================*/
#pragma once

// Version string displayed in -v output
#define CALC_VERSION "0.1a"

// Number of decimal places to which rat (rationalize) mode will try to
//   approximate an answer. Rational answers are _always_ approximate, 
//   because the floating-port support is in binary. However, sometimes
//   the approximations can be quite useful.
#define CALC_RAT_ORDER 3

// Longest line accepted in interactive mode.
#define CALC_MAX_LINE 1024 

// Longest name of a function or variable
#define CALC_MAX_VARNAME 30

// Maximum number of user-defined functions. They don't use a lot of
//   memory, so this value can be large-ish
#define CALC_MAX_FUNCTIONS 500

