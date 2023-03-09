#include <stdlib.h>		/* Callocs y free's */
#include <stdio.h>		/* I/O */
#include <unistd.h>		/* Constantes y tipos simbólicos estándar */
#include <assert.h>		/* Chequeo de precondiciones */
#include <string.h>		/* Operaciones del tipo String*/

#include "builtin.h"	/* Firmas de funciones para éste archivo*/
#include "tests/syscall_mock.h"     /* Mock para syscalls en los tests */

bool builtin_is_exit(pipeline pipe){
	assert(pipe != NULL);
	bool result = 0 == strcmp("exit" ,scommand_front((pipeline_front(pipe)))); 
	return result;
}
bool builtin_is_cd(pipeline pipe){
	assert(pipe != NULL);
	bool result = 0 == strcmp("cd" ,scommand_front((pipeline_front(pipe))));
	return result;
}

bool builtin_is_internal(pipeline pipe){
	assert(pipe != NULL);
	bool result = builtin_is_exit(pipe) || builtin_is_cd(pipe);
	return result;
}

void builtin_exec(pipeline pipe){
	assert(builtin_is_internal(pipe));
	assert(!pipeline_is_empty(pipe));
	if (builtin_is_cd(pipe)){
		scommand_pop_front(pipeline_front(pipe));
		if(!scommand_is_empty(pipeline_front(pipe))){
			int ch = chdir(scommand_front(pipeline_front(pipe)));
			if (ch != 0){
				printf("cd: %s: %s\n",strerror(2),scommand_front(pipeline_front(pipe)));
			}
		}else{
			/*
				Si se ejecuta sólo cd, entonces te lleva a home.
			*/
			chdir(getenv("HOME"));
		}	
	}else if(builtin_is_exit(pipe)){
		exit(EXIT_SUCCESS);
	}
}