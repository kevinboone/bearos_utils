/*=========================================================================

  cal.c

  cal utility for BearOS

  Kevin Boone, December 2022

  Much of the complexity in this code results from the change to Gregorian
  calendar, which is taken to have occurred in 1752. 

  This implementation is largely derived from the one in BusyBox, which is
  itself largely derived from the UC Berkley version, whose copyright
  notice is appended.

  TODO: allow week to start on Sunday if requested
   
=========================================================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#ifdef BEAROS
#include <bearos/printf.h>
#define printf printf_
#endif

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define	SATURDAY 6                /* 1 Jan 1 was a Saturday */

#define	FIRST_MISSING_DAY 639787  /* 3 Sep 1752 */
#define	NUMBER_MISSING_DAYS 11    /* 11 day correction */

#define MAXDAYS 42                /* max slots in a month array */
#define SPACE -1                  /* used in day array */

#define DAY_LEN  3                /* 3 spaces per day */
#define J_DAY_LEN (DAY_LEN + 1)
#define WEEK_LEN 20               /* 7 * 3 - one space at the end */
#define J_WEEK_LEN (WEEK_LEN + 7)
#define HEAD_SEP 2                /* spaces between day headings */

static const unsigned char days_in_month[]  = 
  { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static const unsigned char sep1752[]  = 
  {1, 2, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };

static const char *month_names[12] = 
   {"January", "February", "March", "April", "May", "June", "July", 
    "August", "September", "October", "November", "December"};

BOOL julian = FALSE; // Set by the -j switch

/* =======================================================================
 * leap_year
 * =====================================================================*/
static int leap_year(unsigned yr)
  {
  /* Account for Gregorian reformation in 1752 */
  if (yr <= 1752)
    return !(yr % 4);
  return (!(yr % 4) && (yr % 100)) || !(yr % 400);
  }

/* =======================================================================
 * centuries_since_1700
 *  number of centuries since 1700, not inclusive
 * =====================================================================*/
#define centuries_since_1700(yr) \
        ((yr) > 1700 ? (yr) / 100 - 17 : 0)

/* =======================================================================
 * quad_centuries_since_1700
 * number of centuries since 1700 whose modulo of 400 is 0 
 * =====================================================================*/
#define quad_centuries_since_1700(yr) \
        ((yr) > 1600 ? ((yr) - 1600) / 400 : 0)

/* =======================================================================
 * leap_years_since_year_1 
 * number of leap years between year 1 and this year, not inclusive 
 * =====================================================================*/
#define leap_years_since_year_1(yr) \
        ((yr) / 4 - centuries_since_1700(yr) + quad_centuries_since_1700(yr))

/* =======================================================================
 * day_array
 *   Fill in an array of 42 integers with a calendar.  Assume for a moment
 *   that you took the (maximum) 6 rows in a calendar and stretched them
 *   out end to end.  You would have 42 numbers or spaces.  This routine
 *   builds that array for any month from Jan. 1 through Dec. 9999.
 * =====================================================================*/
static void day_array (unsigned month, unsigned year, unsigned *days)
  {
  unsigned long temp;
  unsigned i;
  unsigned day, dw, dm;

  memset (days, SPACE, MAXDAYS * sizeof(int));

  if ((month == 9) && (year == 1752)) 
    {
    /* Assumes the Gregorian reformation eliminates
    * 3 Sep. 1752 through 13 Sep. 1752.  */
    unsigned j_offset = julian * 244;
    size_t oday = 0;

    do 
      {
      days [oday+2] = sep1752 [oday] + j_offset;
      } while (++oday < sizeof(sep1752));

    return;
    }

        /* day_in_year
         * return the 1 based day number within the year
         */
   day = 1;
   if ((month > 2) && leap_year(year)) {
                ++day;
        }

   i = month;
   while (i) {
                day += days_in_month[--i];
        }

   /* day_in_week
   * return the 0 based day number for any date from 1 Jan. 1 to
   * 31 Dec. 9999.  Assumes the Gregorian reformation eliminates
   * 3 Sep. 1752 through 13 Sep. 1752.  Returns Thursday for all
   * missing days. */

   temp = (long)(year - 1) * 365 + leap_years_since_year_1(year - 1) + day;
   if (temp < FIRST_MISSING_DAY) 
     {
     dw = ((temp - 1 + SATURDAY) % 7);
     } 
   else 
     {
     dw = (((temp - 1 + SATURDAY) - NUMBER_MISSING_DAYS) % 7);
     }

   if (!julian) 
     {
     day = 1;
     }

   dm = days_in_month[month];
   if ((month == 2) && leap_year(year)) 
     {
     ++dm;
     }

   do 
     {
     days[dw++] = day++;
     } while (--dm);
  }

/* =======================================================================
 * build_row
 * =====================================================================*/
static char *build_row (char *p, unsigned *dp)
  {
  unsigned col, val, day;

  memset (p, ' ', (julian + DAY_LEN) * 7);

  col = 0;
  do 
    {
    day = *dp++;
    if ((int)day != SPACE) 
      {
      if (julian) 
        {
        ++p;
        if (day >= 100) 
          {
          *p = '0';
          p[-1] = (day / 100) + '0';
          day %= 100;
          }
        }
      val = day / 10;
      if (val > 0) 
        {
        *p = val + '0';
        }
      *++p = day % 10 + '0';
      p += 2;
      } 
     else 
      {
      p += DAY_LEN + julian;
      }
    } while (++col < 7);

  return p;
  }

/* =======================================================================
 * trim_trailing_spaces_and_print
 * =====================================================================*/
static void trim_trailing_spaces_and_print (char *s)
  {
  char *p = s;

  while (*p) 
    {
    ++p;
    }
  while (p != s) 
    {
    --p;
    if (!isspace(*p)) 
       {
       p[1] = '\0';
       break;
       }
     }
  puts(s);
  }

/* =======================================================================
 * blank_string 
 * =====================================================================*/
static void blank_string(char *buf, size_t buflen)
  {
  memset (buf, ' ', buflen);
  buf[buflen-1] = '\0';
  }

/* =======================================================================
 * center 
 * =====================================================================*/
static void center (const char *str, unsigned len, unsigned separate)
  {
  unsigned n = strlen(str);
  len -= n;
  printf("%*s%*s", (len/2) + n, str, (len/2) + (len % 2) + separate, "");
  }

/*=========================================================================
  show_usage 
=========================================================================*/
static void show_usage (const char *argv0)
  {
  printf ("Usage: %s [options] [month] {year}\n", argv0); 
  printf ("   -v: show version\n");
  printf ("   -h: show this message\n");
  printf ("   -j: show julian day-of-year numbers\n");
  printf ("If no momth is given, a whole year is shown.\n");
  }

/*=========================================================================
  show_version
=========================================================================*/
static void show_version (const char *argv0)
  {
  printf ("%s for BearOS version 0.1\n", argv0); 
  printf ("Copyright (c)2022 Kevin Boone, GPL3\n");
  }

/*=========================================================================
  main 
=========================================================================*/
int main (int argc, char **argv)
  {
  int month = 0;
  int year = 0;
  julian = FALSE;
  int opt;
  int ret = 0;
  optind = 0;
  BOOL usage = FALSE;
  BOOL version = FALSE;

  while (((opt = getopt (argc, argv, "hvj")) != -1) && (ret == 0))
    {
    switch (opt)
      {
      case 'V':
        version = TRUE;
        break;
      case 'h':
        usage = TRUE;
        break;
      case 'j':
        julian = TRUE;
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
    if ((argc - optind == 0))
      {
      time_t now = time (NULL);
      struct tm *tm = localtime (&now);
      year = tm->tm_year + 1900;
      month = tm->tm_mon + 1; 
      }  
    else if ((argc - optind == 1))
      {
      year = atoi (argv[optind]);
      }  
    else if ((argc - optind == 2))
      {
      month = atoi (argv[optind]);
      year = atoi (argv[optind + 1]);
      if (month < 1 || month > 12)
        {
        printf ("Month out of range (1-12)\n");
        ret = ERANGE;
        }
      }  
    else
      {
      show_usage (argv[0]);
      ret = EINVAL; 
      }
    }
 
  if (ret == 0)
    {
    unsigned int days [MAXDAYS];
    const char *day_headings;
    if (julian)
      day_headings = "Su  Mo  Tu  We  Th  Fr  Sa ";
    else
      day_headings = "Su Mo Tu We Th Fr Sa";

    if (month)
      {
      unsigned *dp = days;
      day_array (month, year, days);

      char lineout[30];
      int len = sprintf (lineout, "%s %u", month_names[month - 1], year);
      printf("%*s%s\n%s\n",
                                    ((7*julian + WEEK_LEN) - len) / 2, "",
                                    lineout, day_headings);
      for (int row = 0; row < 6; row++) 
        {
        build_row (lineout, dp)[0] = '\0';
        dp += 7;
        trim_trailing_spaces_and_print (lineout);
        }
      }
    else
      {
      unsigned week_len, days[12][MAXDAYS];
      unsigned *dp;
      char lineout[80];

      sprintf(lineout, "%u", year);
      center(lineout,
        (WEEK_LEN * 3 + HEAD_SEP * 2)
        + julian * (J_WEEK_LEN * 2 + HEAD_SEP
        - (WEEK_LEN * 3 + HEAD_SEP * 2)), 0);

      puts("\n");                /* two blank lines */
      for (int i = 0; i < 12; i++) 
        {
        day_array (i + 1, year, days[i]);
        }
  
      blank_string (lineout, sizeof(lineout));
      week_len = WEEK_LEN + julian * (J_WEEK_LEN - WEEK_LEN);
      for (month = 0; month < 12; month += 3-julian) 
        {
        center (month_names[month], week_len, HEAD_SEP);
        if (!julian) 
          {
          center (month_names[month + 1], week_len, HEAD_SEP);
          }
        center (month_names[month + 2 - julian], week_len, 0);
        printf ("\n%s%*s%s", day_headings, HEAD_SEP, "", day_headings);
        if (!julian) 
          {
          printf ("%*s%s", HEAD_SEP, "", day_headings);
          }
        putchar ('\n');
        for (int row = 0; row < (6*7); row += 7) 
          {
          for (int which_cal = 0; which_cal < 3-julian; which_cal++) 
            {
            dp = days [month + which_cal] + row;
            build_row (lineout + which_cal * (week_len + 2), dp);
            }
          trim_trailing_spaces_and_print(lineout);
          }
        }
      }
    }
  if (usage) ret = 0;
  return ret;
  }

/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kim Letkeman.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

