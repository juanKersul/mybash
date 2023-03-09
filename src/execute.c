#include <stdlib.h>     /* Callocs y free's */
#include <stdio.h>      /* I/O */
#include <assert.h>     /* Chequeo de precondiciones */
#include <errno.h>      /* Seteo de variable errno / manejo de errores */
#include <unistd.h>     /* Constantes y tipos simbólicos estándar */
#include <sys/types.h>  /* Constantes y tipos simbólicos de sistema, pid_t */
#include <sys/wait.h>   /* Constantes y tipos simbólicos para wait()/waitpid() */
#include <fcntl.h>      /* Manejo de FileDescriptors */
#include <string.h>     /* Operaciones del tipo String*/

#include "command.h"    /* Comandos */
#include "builtin.h"    /* Manejo de comandos internos */
#include "execute.h"    /* Firmas de funciones para éste archivo*/
#include "tests/syscall_mock.h"     /* Mock para syscalls en los tests */

/*
    Retorna los argumentos del command pasado, pidiendo memoria para cada argumento, sin liberarlo.
    Luego de usarla aplicar free().
*/
static char ** get_args(pipeline apipe){
    unsigned int size_command = scommand_length(pipeline_front(apipe));
    scommand command = pipeline_front(apipe);
    char ** argv = calloc(size_command+1,sizeof(char *));
    for (unsigned int i = 0u; i < size_command; i++){
        argv[i] = strdup(scommand_front(command));
        scommand_pop_front(command);
    }    
    argv[size_command] = NULL;
    return argv;
}

/*
    La función redirect redirige la salida estándar a los archivos outfilename y la entrada estándar al archivo infilename.
    Si cualquiera de los parámetros es NULL, no ocurre el redireccionamiento.
    Retorna 0 si fue exitoso ó -1 en caso contrario.
*/
static int redirect (pipeline apipe){
    assert(apipe != NULL);
    char * infilename, * outfilename;
    int infd, outfd;
    infilename = scommand_get_redir_in(pipeline_front(apipe));
    outfilename = scommand_get_redir_out(pipeline_front(apipe));
    /* 
        Caso redireccion de una entrada estándar hacia infilename.
    */
    if (infilename != NULL){
        infd = open(infilename, O_RDONLY,S_IRWXU);  
                                /* O_RDONLY = sólo lectura. S_IRWXU = Lectura, escritura y ejecución para el propietario */
        if (infd == -1){
            return -1;
        }
        if (dup2(infd,STDIN_FILENO) == -1){
            close(infd);
            return -1;
        }
        close(infd);
    }
    /* 
        Caso redireccion de una salida estándar hacia outfilename.
    */
    if (outfilename != NULL){
        outfd = open(outfilename,O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                                /* Permiso de escritura ó creación en caso de que no exista, más permisos de lectura y escritura para usr = propietario, grp = grupo o oth = other*/
        if (outfd == -1){
            return -1;
        }
        if (dup2(outfd,STDOUT_FILENO) == -1){
            close(outfd);
            return -1;
        }
        close(outfd);
    }
    return 0;
}

/*
    Ejecuta n commandos, forkeando y redirigiendo la entrada y salida.
    Modifica a apipe en el proceso.
    apipe pipeline a ejecutar.
    Requires: apipe != NULL
*/
static void execute_external_commands(pipeline apipe) {
    assert(apipe != NULL);
    int num_pipes = pipeline_length(apipe)-1;
    pid_t pid;    
    /*
        arreglo dinamico para las puntas de las pipes.
    */
    int * pipefds = calloc(num_pipes*2,sizeof(int));
    
    /*
        Creo pipes con casos de error.
    */
    for(int i = 0; i < (num_pipes); i++){
        if(pipe(pipefds + i*2) < 0) {
            perror("File Descriptor error");
            exit(EXIT_FAILURE);
        }
    }
    int pipe_counter = 0;
    while(!pipeline_is_empty(apipe)) {
        pid = fork();
        /*
            Código del Hijo. 
        */
        if(pid == 0) {
            if (redirect(apipe) == -1){   
                perror("Redirection failed");
                exit(EXIT_FAILURE);
            }                   
            /*
                No es el ultimo comando.
            */
            if(pipeline_length(apipe)!=1){
                if(dup2(pipefds[pipe_counter + 1], STDOUT_FILENO) < 0){
                    close(pipefds[pipe_counter + 1]);
                    perror("File Descriptor error");
                    exit(EXIT_FAILURE);
                }
            }
            /* 
                No es el primer comando.
            */
            if(pipe_counter != 0 ){ 
                if(dup2(pipefds[pipe_counter-2], STDIN_FILENO) < 0){
                    close(pipefds[pipe_counter-2]);
                    perror("File Descriptor error");
                    exit(EXIT_FAILURE);
                }
            }
            /*
                Hijo cierra pipes
            */
            for(int i = 0; i < 2*num_pipes; i++){
                close(pipefds[i]);
            }
            char ** argv = get_args(apipe);
            if(execvp(argv[0],argv) < 0 ){
                perror("No es un comando válido");
                for (unsigned int i = 0u; i < scommand_length(pipeline_front(apipe)); i++){
                    free(argv[i]);
                }
                exit(EXIT_FAILURE);
            }
        }else if(pid < 0){
            /*
                Caso error fork  
            */
            perror("error");
            exit(EXIT_FAILURE);
        }
        /*
            Primer comando ya ejecutado, se saca del pipeline
        */
        pipeline_pop_front(apipe);
        pipe_counter += 2;
    }
    /*
        Padre cierra los pipes y espera al hijo, si es necesario, fuera del while.
    */
    for(int i = 0; i < 2 * num_pipes; i++){
        close(pipefds[i]);
    }
    if(pipeline_get_wait(apipe)){
        for (int i = 0; i < (num_pipes+1); i++){        
            wait(NULL);
        }
    }
    free(pipefds);
}   


void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);
    /*
        Evitar la señal del hijo, para evitar procesos zombies.
    */
    signal(SIGCHLD,SIG_IGN);
    /*
        No se puede ejecutar comandos tales como (internos) pipe (externos).
    */
    if(!pipeline_is_empty(apipe)){
        if(builtin_is_internal(apipe)){
            /*
                builtin_exec no maneja los casos de wait.
            */
            builtin_exec(apipe);
        }else{
            execute_external_commands(apipe);
        }
    }
}