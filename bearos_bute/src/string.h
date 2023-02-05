/*============================================================================

  boilerplate 
  string.h
  Copyright (c)2017 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"

struct _String;
typedef struct _String String;

String      *string_create_empty (void);
String      *string_create (const char *s);
String      *string_clone (const String *self);
int          string_find (const String *self, const char *search);
int          string_find_last (const String *self, const char *search);
void         string_destroy (String *self);
const char  *string_cstr (const String *self);
const char  *string_cstr_safe (const String *self);
void         string_append_printf (String *self, const char *fmt,...);
void         string_append (String *self, const char *s);
void         string_append_c (String *self, const uint32_t c);
void         string_prepend (String *self, const char *s);
int          string_length (const String *self);
String      *string_substitute_all (const String *self, 
                const char *search, const char *replace);
void        string_delete (String *self, const int pos, 
                const int len);
void        string_insert (String *self, const int pos, 
                const char *replace);
void        string_append_byte (String *self, const BYTE byte);
void        string_trim_left (String *self);
void        string_trim_right (String *self);
BOOL        string_ends_with (const String *self, const char *test);
void        string_delete_last (String *self);
void        string_insert_c_at (String *self, int pos, char c);
void        string_delete_c_at (String *self, int pos);

