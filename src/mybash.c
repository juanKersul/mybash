#include <stdio.h>      /* I/O */
#include <stdbool.h>    /* Tipo y valores Booleanos */

#include "command.h"    /* Frimas de funciones para los comandos */
#include "execute.h"    /* Frimas de funciones para los comandos pipelines */
#include "parser.h"     /* Módulo de parseo de pipelines */
#include "builtin.h"    /* Frimas de funciones para los comandos internos */
#include "prompt.h"     /* Frimas de funciones para el prompt */


int main(int argc, char *argv[]){
    Parser parser;
    pipeline pipe;
    bool quit = false;

    parser = parser_new(stdin);
    while (!quit) {
        show_prompt();
        pipe = parse_pipeline(parser);
        quit = parser_at_eof(parser); /* Chequeo si hay que salir luego de ejecutar el comando */
        if (pipe != NULL) {
            quit = quit || builtin_is_exit(pipe);
            execute_pipeline(pipe);
            pipeline_destroy(pipe);
        }
    }
    parser_destroy(parser); 
    parser = NULL;
    return 0;	
}
