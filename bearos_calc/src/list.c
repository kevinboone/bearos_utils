/*============================================================================

  bearcalc

  list.c

  Methods for maintaining a single-linked list.

  Copyright (c)2000-2022 Kevin Boone, GPL v3.0

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include "list.h"
#include "string.h" 

#define LOG_IN
#define LOG_OUT

typedef struct _ListItem
  {
  struct _ListItem *next;
  void *data;
  } ListItem;

struct _List
  {
  ListItemFreeFn free_fn; 
  ListItem *head;
  };

/*==========================================================================
list_create
*==========================================================================*/
List *list_create (ListItemFreeFn free_fn)
  {
  LOG_IN
  List *list = malloc (sizeof (List));
  memset (list, 0, sizeof (List));
  list->free_fn = free_fn;
  LOG_OUT
  return list;
  }

/*==========================================================================
  list_create_strings
 
  This is a helper function for creating a list of C strings -- not
  string objects
*==========================================================================*/
List *list_create_strings (void)
  {
  return list_create (free);
  }

/*==========================================================================
  list_destroy
*==========================================================================*/
void list_destroy (List *self)
  {
  LOG_IN
  if (self) 
    {
    ListItem *l = self->head;
    while (l)
      {
      if (self->free_fn)
        self->free_fn (l->data);
      ListItem *temp = l;
      l = l->next;
      free (temp);
      }

    free (self);
    }
  LOG_OUT
  }


/*==========================================================================
list_prepend
Note that the caller must not modify or free the item added to the list. It
will remain on the list until free'd by the list itself, by calling
the supplied free function
*==========================================================================*/
void list_prepend (List *self, void *item)
  {
  LOG_IN
  ListItem *i = malloc (sizeof (ListItem));
  i->data = item;
  i->next = NULL;

  if (self->head)
    {
    i->next = self->head;
    self->head = i;
    }
  else
    {
    self->head = i;
    }
  LOG_OUT
  }


/*==========================================================================
  list_append
  Note that the caller must not modify or free the item added to the list. 
  It will remain on the list until free'd by the list itself, by calling
    the supplied free function
*==========================================================================*/
void list_append (List *self, void *item)
  {
  LOG_IN
  ListItem *i = malloc (sizeof (ListItem));
  i->data = item;
  i->next = NULL;

  if (self->head)
    {
    ListItem *l = self->head;
    while (l->next)
      l = l->next;
    l->next = i;
    }
  else
    {
    self->head = i;
    }
  LOG_OUT
  }


/*==========================================================================
  list_length
*==========================================================================*/
int list_length (const List *self)
  {
  LOG_IN
  ListItem *l = self->head;

  int i = 0;
  while (l != NULL)
    {
    l = l->next;
    i++;
    }

  LOG_OUT
  return i;
  }

/*==========================================================================
  list_get
*==========================================================================*/
void *list_get (const List *self, int index)
  {
  LOG_IN
  ListItem *l = self->head;
  int i = 0;
  while (l != NULL && i != index)
    {
    l = l->next;
    i++;
    }
  LOG_OUT
  return l->data;
  }

/*==========================================================================
  list_contains
*==========================================================================*/
BOOL list_contains (List *self, const void *item, ListCompareFn fn)
  {
  LOG_IN
  ListItem *l = self->head;
  BOOL found = FALSE;
  while (l != NULL && !found)
    {
    if (fn (l->data, item, NULL) == 0) found = TRUE; 
    l = l->next;
    }
  LOG_OUT
  return found; 
  }

/*==========================================================================
list_remove_object
Remove the specific item from the list, if it is present. The object's
remove function will be called. This method can't be used to remove an
object by value -- that is, you can't pass "dog" to the method to remove
all strings whose value is "dog". Use list_remove() for that.
*==========================================================================*/
void list_remove_object (List *self, const void *item)
  {
  LOG_IN
  ListItem *l = self->head;
  ListItem *last_good = NULL;
  while (l != NULL)
    {
    if (l->data == item)
      {
      if (l == self->head)
        {
        self->head = l->next; // l-> next might be null
        }
      else
        {
        if (last_good) last_good->next = l->next;
        }
      self->free_fn (l->data);  
      ListItem *temp = l->next;
      free (l);
      l = temp;
      } 
    else
      {
      last_good = l;
      l = l->next;
      }
    }
  LOG_OUT
  }


/*==========================================================================
list_remove
Remove all items from the last that are a match for 'item', as
determined by a comparison function.

IMPORTANT -- The "item" argument cannot be a direct reference to an
item already in the list. If that item is removed from the list its
memory will be freed. The "item" argument will thus be an invalid
memory reference, and the program will crash when it is next used. 
It is necessary to provide a comparison function, and items will be
removed (and freed) that satisfy the comparison function. 

To remove one specific, known, item from the list, uselist_remove_object()
*==========================================================================*/
void list_remove (List *self, const void *item, ListCompareFn fn)
  {
  LOG_IN
  ListItem *l = self->head;
  ListItem *last_good = NULL;
  while (l != NULL)
    {
    if (fn (l->data, item, NULL) == 0)
      {
      if (l == self->head)
        {
        self->head = l->next; // l-> next might be null
        }
      else
        {
        if (last_good) last_good->next = l->next;
        }
      self->free_fn (l->data);  
      ListItem *temp = l->next;
      free (l);
      l = temp;
      } 
    else
      {
      last_good = l;
      l = l->next;
      }
    }
  LOG_OUT
  }

