#include <stdio.h>      /* I/O */
#include <string.h>     /* Operaciones del tipo String*/
#include <unistd.h>     /* Constantes y tipos simbólicos estándar, y la función getcwd() */

#include "prompt.h"     /* Firmas de funciones para éste archivo*/

void show_prompt(void){
    char buffer[100];
    /*
        The strrchr() function returns a pointer to the last occurrence of c. The null character terminating a string is considered to be part of the string.
    */
    char * dir = strrchr(getcwd(buffer, 100), '/');
    printf ("\x1b[31m""➜  ""\x1b[32m""%s ""\x1b[34m",dir+1);
    fflush (stdout);
}