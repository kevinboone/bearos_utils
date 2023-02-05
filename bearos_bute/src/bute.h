
#pragma once

#include "defs.h"

struct _ButeEnv;
typedef struct _ButeEnv ButeEnv;

extern void buteenv_destroy (ButeEnv *self);

extern void buteenv_run (ButeEnv *self);

extern ErrCode buteenv_add_editor (ButeEnv *self, const char *filename);

extern ButeEnv *buteenv_create (BOOL noindent, int rows, int cols); 

