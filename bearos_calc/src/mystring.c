/*============================================================================

  bearcalc


  string.c

  Copyright (c)2017 Kevin Boone, GPL v3.0

  Methods for handling ASCII/UTF-8 strings. Be aware that these methods
  are just thin wrappers around standard, old-fashioned C library functions,
  and some will misbehave if the string actually contains multi-byte
  characters. In particular, the length() method returns the number of
  bytes, not the number of characters. Methods that search the string may
  potentially match the second or later byte of a multi-byte character.
  Any use of these methods for handling 'real' multibyte UTF-8 needs to
  be tested very carefully.

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "string.h" 
#include "defs.h"
#include "list.h"
#include "mystring.h"

struct _String
  {
  char *str;
  }; 

/*==========================================================================
string_create_empty 
*==========================================================================*/
String *string_create_empty (void)
  {
  return string_create ("");
  }

/*==========================================================================
string_create
*==========================================================================*/
String *string_create (const char *s)
  {
  String *self = malloc (sizeof (String));
  self->str = strdup (s);
  return self;
  }

/*==========================================================================
string_destroy
*==========================================================================*/
void string_destroy (String *self)
  {
  if (self)
    {
    if (self->str) free (self->str);
    free (self);
    }
  }


/*==========================================================================
string_cstr
*==========================================================================*/
const char *string_cstr (const String *self)
  {
  return self->str;
  }


/*==========================================================================
string_append
*==========================================================================*/
void string_append (String *self, const char *s) 
  {
  if (!s) return;
  if (self->str == NULL) self->str = strdup ("");
  int newlen = (int)strlen (self->str) + (int)strlen (s) + 2;
  self->str = realloc (self->str, (size_t)newlen);
  strcat (self->str, s);
  }

/*==========================================================================
string_length
*==========================================================================*/
int32_t string_length (const String *self)
  {
  if (self == NULL) return 0;
  if (self->str == NULL) return 0;
  return (int32_t)strlen (self->str);
  }

/*==========================================================================
string_clone
*==========================================================================*/
String *string_clone (const String *self)
  {
  if (!self->str) return string_create_empty();
  return string_create (string_cstr (self));
  }

/*==========================================================================
string_delete
*==========================================================================*/
void string_delete (String *self, const int pos, const int32_t len)
  {
  char *str = self->str;
  if (pos + len > (int)strlen (str))
    string_delete (self, pos, (int)strlen(str) - len);
  else
    {
    char *buff = malloc (strlen (str) - (size_t)len + 2);
    strncpy (buff, str, (size_t)pos); 
    strcpy (buff + pos, str + pos + len);
    free (self->str);
    self->str = buff;
    }
  }


/*==========================================================================
string_insert
*==========================================================================*/
void string_insert (String *self, const int pos, 
    const char *replace)
  {
  char *buff = malloc (strlen (self->str) + strlen (replace) + 2);
  char *str = self->str;
  strncpy (buff, str, (size_t)pos);
  buff[pos] = 0;
  strcat (buff, replace);
  strcat (buff, str + pos); 
  free (self->str);
  self->str = buff;
  }

/*==========================================================================
  string_insert_c_at
*==========================================================================*/
void string_insert_c_at (String *self, int pos, char c)
  {
  static char s[2] = " ";
  s[0] = c;
  string_insert (self, pos, s);
  }

/*==========================================================================
  string_delete_c_at
*==========================================================================*/
void string_delete_c_at (String *self, int pos)
  {
  string_delete (self, pos, 1); 
  }


