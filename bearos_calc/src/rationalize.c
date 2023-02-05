/*============================================================================

  rationalize

  rationalize.c

  Turn a floating-point decimal number into a fraction, with specified
  precision.

  Version 2 -- scale original number by a power of ten, so all calculations
  are done as integers.

  Copyright (c)2022 Kevin Boone, GPL 3.0

============================================================================*/
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "rationalize.h" 

/*============================================================================

  iabs

  Return the absolute value of a 64-bit number 

============================================================================*/

int64_t iabs (int64_t x)
  {
  return x > 0 ? x : -x;
  }

/*============================================================================

  rationalize 

============================================================================*/

void rationalize (double num, int order, BOOL improper, 
         int *whole, int *numerator, 
         int *denominator, BOOL *negative)
  {
  *negative = FALSE;
  if (num < 0) 
    {
    *negative = TRUE;
    num = -num;
    }

  if (!improper)
    {
    double frac = num - floor (num);
    *whole = (int) floor (num);
    num = frac;
    }
  else
    *whole = 0;

  // The largest possible value for "order" is 9, because we have
  //   to be able to fit "10^(order*2)" into the 64-bit integer type
  //   we are using for computation. 
  if (order > 9) order = 9;

  // In this version, we scale the original number so that it has at
  //  least as many non-fraction digits as "order". "scale" is the 
  //  scaling factor, that will be used throughout the computation.
  int64_t scale = pow (10, order);

  // Take "x" to be the scaled version of the number to be converted
  int64_t x = num * scale;
  int64_t orig_x = x;

  // Calculate the first continued fraction coefficient, but bear in mind
  //   that it's derived from a number that is "scale" times too large.
  int64_t a = orig_x / scale; 

  // Previous versions of the numerator and denominator, as in version 1.
  // (No scaling here)
  int64_t n = a;
  int64_t n1 = 1;
  int64_t n2;
  
  int64_t d = 1;
  int64_t d1 = 0;
  int64_t d2;

  // We expand the continued fraction as in version 1, but we have placed
  //   a limit "order" on the number of significant figures in the results.
  // Therefore, we never need to loop more times than order.
  int loops = 0;
  while (loops++ < order && x - a * scale)
    {
    // In version 1, we had "x = 1 / (x-a)", in floating-point
    //  numbers. Of course, we can't do this calculation in integers,
    //  but we can multiplay by the scaling factor "scale". A first
    //  guess at the logic might be "x = scale / (x / scale - a)", but 
    //  this fails because "x / scale" is probably zero in integers. So
    // We need to multiple top and bottom by "scale" to get a version that
    //  will always succeed in integers. Of course, this limits the
    //  value of "order", as scale=10^order, and an int64 is only so big.
    x = scale * scale / (x - a * scale);

    // Calculate the next continued fraction coefficient, again bearing
    // in mind that it comes from a scaled number. Dividing by "scale"
    //  is the equivalent in this integer version as taking floor(x) in
    // version 1.
    a = x / scale; 

    // The expansion of the continued fraction into numerator and
    //   denominator is the same as in version 1.
    n2 = n1; 
    n1 = n;
    n = n2 + a * n1;

    d2 = d1; 
    d1 = d;
    d = d2 + a * d1;

    // If the value of n/d is _exact_ compared to the original number,
    //   stop here -- no point looping further, as all the following
    //   continued fraction coefficients will be zero.
    if (!d) break;
    if (iabs (scale * n / d - orig_x) <= 1) break; 
    } 

  *numerator = n;
  *denominator = d;
  }

