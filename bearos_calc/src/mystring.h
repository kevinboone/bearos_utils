/*============================================================================

  bearcalc
 
  mystring.h

  A variable-length string.

  Copyright (c)2017 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"
#include "list.h"

struct _String;
typedef struct _String String;

BEGIN_DECLS

String      *string_create_empty (void);
String      *string_create (const char *s);
String      *string_clone (const String *self);
void         string_destroy (String *self);
const char  *string_cstr (const String *self);
void         string_append (String *self, const char *s);
void         string_append_c (String *self, const uint32_t c);
int32_t      string_length (const String *self);
void        string_delete (String *self, const int pos, 
                const int32_t len);
void        string_insert (String *self, const int pos, 
                const char *replace);
void        string_delete_last (String *self);
void        string_insert_c_at (String *self, int pos, char c);
void        string_delete_c_at (String *self, int pos);

END_DECLS

