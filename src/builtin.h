#ifndef _BUILTIN_H_
#define _BUILTIN_H_

#include <stdbool.h>
#include "command.h"

bool builtin_is_exit(pipeline pipe);
/*
 * Indica si un "exit"
 *
 * REQUIRES: pipe != NULL
 *
 */

bool builtin_is_cd(pipeline pipe);
/*
 * Indica si el comando es un "cd"
 *
 * REQUIRES: pipe != NULL
 *
 */

bool builtin_is_internal(pipeline pipe);
/*
 * Indica si el comando es interno 
 *
 * REQUIRES: pipe != NULL
 *
 */

void builtin_exec(pipeline pipe);
/*
 * Ejecuta un comando interno
 *
 * REQUIRES: {builtin_is_internal(pipe)}
 *
 */

#endif
