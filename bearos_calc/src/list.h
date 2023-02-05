/*============================================================================

  bearcalc

  list.h

  Methods for maintaining a singly-linked list. This is used in Bearcalc
  only for maintaing the command-line history.

  Copyright (c)2000-2022 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h" 

struct _List;
typedef struct _List List;

// The comparison function should return -1, 0, +1, like strcmp. In practice
//   however, the functions that use this only care whether too things 
//   are equal -- ordering is not important. The i1,i2 arguments are 
//   pointers to the actual objects in the list. user_data is not used
//   at present
typedef int (*ListCompareFn) (const void *i1, const void *i2, 
          void *user_data);

typedef void* (*ListCopyFn) (const void *orig);
typedef void (*ListItemFreeFn) (void *);

List   *list_create (ListItemFreeFn free_fn);
void    list_destroy (List *);
void    list_append (List *self, void *item);
void    list_prepend (List *self, void *item);
void   *list_get (const List *self, int index);
int     list_length (const List *self);
BOOL    list_contains (List *self, const void *item, ListCompareFn fn);
void    list_remove (List *self, const void *item, ListCompareFn fn);
List   *list_clone (List *self, ListCopyFn copyFn);
List   *list_create_strings (void);
void    list_remove_object (List *self, const void *item);

