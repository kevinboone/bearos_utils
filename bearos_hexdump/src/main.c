
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef BEAROS
#include <bearos/printf.h> 
#else
#define printf_ printf
#endif

typedef int BOOL;
#define TRUE 1;
#define FALSE 0;

/*=========================================================================
  show_usage 
=========================================================================*/
static void show_usage (const char *argv0)
  {
  printf_ ("Usage: %s [options] [path]\n", argv0); 
  printf_ ("   -C: show ASCII characters and hex\n");
  printf_ ("   -m: dump memory (use -s to set address)\n");
  printf_ ("   -n: don't suppress duplicate rows\n");
  printf_ ("   -s: start position\n");
  printf_ ("   -l: length to dump\n");
  printf_ ("   -V: show version\n");
  }

/*=========================================================================
  show_version
=========================================================================*/
static void show_version (const char *argv0)
  {
  printf_ ("%s for BearOS version 0.1\n", argv0); 
  printf_ ("Copyright (c)2022 Kevin Boone, GPL3\n");
  }

/*=========================================================================
  print_row 
=========================================================================*/
static void print_row (int addr, const char *row, BOOL canonical, 
     int width, int n)
  {
  printf_ ("%08x ", addr);
  
  for (int i = 0; i < n; i++)
    {
    char b = row[i];
    printf_ ("%02x ", b & 0xFF);
    } 

  if (canonical)
    {
    for (int i = n; i < width; i++)
       printf_ ("   ");

    for (int i = 0; i < n; i++)
      {
      int b = row[i];
      if (b > 32 && b < 127) // 127 is probably a backspace/delete
	printf_ ("%c", b); 
      else
	printf_ (".");
      } 
    }
  printf_ ("\n");
  }

/*=========================================================================
  do_file 
=========================================================================*/
static BOOL cpcmp (const char *buff, char *lastbuff, int width)
  {
  BOOL diff = FALSE;
  for (int i = 0; i < width; i++)
    {
    if (buff[i] != lastbuff[i]) diff = TRUE;
    lastbuff[i] = buff[i];
    }
  return !diff; 
  }

/*=========================================================================
  do_file 
=========================================================================*/
static int do_file_fd (int fd, BOOL canonical, int skip, int length, 
        BOOL nosuppress)
  {
  int width = 16; // TODO
  char buff[16]; // TODO
  char lastbuff[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
  
  int n;
  int count = 0;
  BOOL last_was_same = FALSE;
  while ((n = read (fd, buff, width)) > 0 && (count < length || !length))
    {
    if (cpcmp (buff, lastbuff, width))
      {
      if (!nosuppress)
        {
        if (!last_was_same) printf_ ("****\n");
        last_was_same = TRUE;
        }
      }
    else
      { 
      last_was_same = FALSE;
      print_row (count + skip, buff, canonical, width, n);
      }
    count += n;
    };

  return 0;
  }

/*=========================================================================
  do_memory
=========================================================================*/
static int do_memory (BOOL canonical, int skip, 
             int length, BOOL nosuppress)
  {
  int width = 16; // TODO
  char buff[16]; // TODO
  char lastbuff[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
  
  int count = 0;
  BOOL last_was_same = FALSE;
  while ((count < length || !length))
    {
    for (int i = 0; i < width; i++)
      {
      char *p = (char *) (count + skip + i);
      buff[i] = *p; 
      }
    if (cpcmp (buff, lastbuff, width))
      {
      if (!nosuppress)
        {
        if (!last_was_same) printf_ ("****\n");
        last_was_same = TRUE;
        }
      }
    else
      { 
      last_was_same = FALSE;
      print_row (count + skip, buff, canonical, width, width);
      }
    count += width;
    };

  return 0;
  }

/*=========================================================================
  do_file_path 
=========================================================================*/
static int do_file_path (const char *pathname, BOOL canonical, 
      int skip, int length, BOOL nosuppress)
  {
  int fd = open (pathname, O_RDONLY);
  if (fd >= 0)
    {
    if (skip)
      {
      off_t pos = lseek (fd, skip, SEEK_SET);
      if (pos == -1)
        {
        int e = errno;
        printf_ ("%s: seek failed: %s", pathname, strerror (errno));
        return e;
        }
      if (pos != skip)
        {
        int e = errno;
        printf_ ("%s: can't set position", pathname);
        return e;
        }
      }
    do_file_fd (fd, canonical, skip, length, nosuppress);
    close (fd); 
    }
  else
    {
    printf_ ("%s: %s\n", pathname, strerror (errno));
    }
  return 0; 
  }

/*=========================================================================
   convert_num 
=========================================================================*/
int convert_num (const char *s, BOOL *ok)
  {
  char *dummy;
  int ret;
  if (strlen (s) > 2 && strncmp (s, "0x", 2) == 0)
    {
    ret = strtol (s + 2, &dummy, 16);
    if (errno || (*dummy))
      {
      printf_ ("Bad number: %s\n", s);
      *ok = FALSE;
      return 0;
      }
    }
  else
    {
    ret = strtol (s, &dummy, 10);
    if (errno || (*dummy))
      {
      printf_ ("Bad number: %s\n", s);
      *ok = FALSE;
      return 0;
      }
    }
  *ok = TRUE;
  return ret;
  }

/*=========================================================================
  usage 
=========================================================================*/
int main (int argc, char **argv) 
  {
  int opt;
  int ret = 0;
  int skip = 0; 
  int length = 0; 
  optind = 0;
  BOOL usage = FALSE;
  BOOL canonical = FALSE;
  BOOL version = FALSE;
  BOOL nosuppress = FALSE;
  BOOL memory = FALSE;

  BOOL ok = TRUE;

  while (((opt = getopt (argc, argv, "+nl:s:ChVm")) != -1) && (ret == 0))
    {
    switch (opt)
      {
      case 'C':
        canonical = TRUE;
        break;
      case 'V':
        version = TRUE;
        break;
      case 'h':
        usage = TRUE;
        break;
      case 'm':
        memory = TRUE;
        break;
      case 's':
        skip = convert_num (optarg, &ok);
        if (!ok)
          {
          ret = EINVAL;
          break;
          }
        break;
      case 'l':
        length = convert_num (optarg, &ok);
        if (!ok)
          {
          ret = EINVAL;
          break;
          }
        break;
      case 'n':
        nosuppress = TRUE;
        break;
      default:
        ret = -1; // TODO
      }
    }

  if (usage)
    {
    ret = -1;
    show_usage (argv[0]);
    exit(0);
    }  

  if (version)
    {
    ret = -1;
    show_version (argv[0]);
    }  

  if (ret == 0)
    {
    if ((argc - optind >= 1) || memory)
      {
      if (memory)
        do_memory (canonical, skip, length, nosuppress);
      else
        do_file_path (argv[optind], canonical, skip, length, nosuppress);
      }  
    else
      {
      show_usage (argv[0]);
      ret = EINVAL; 
      }
    }
 
  if (usage) ret = 0;
  return ret;
  }


