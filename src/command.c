#include <stdio.h> 
#include <glib.h>
#include <assert.h>
#include <string.h>
#include "strextra.h"

#include "command.h"


/********** COMANDO SIMPLE **********/

/* Estructura correspondiente a un comando simple.
 * Es una 3-upla del tipo ([char*], char* , char*).
 */

struct scommand_s {
    GSList *args;
    char * redir_in;
    char * redir_out;
};


scommand scommand_new(void){
   	scommand result = calloc(1,sizeof(*result));
	result->args = NULL;
	result->redir_in = NULL;
	result->redir_out = NULL;
	assert(result != NULL && (result->args == NULL) && scommand_get_redir_in (result) == NULL && scommand_get_redir_out (result) == NULL);
	return result;   
}

scommand scommand_destroy(scommand self){
	assert(self!= NULL);
	while (self->args != NULL){
		scommand_pop_front(self);
	}
	if (self->redir_in) free(self->redir_in);
	if (self->redir_out) free(self->redir_out);
	free (self);
	self = NULL;
	return self;	
}

void scommand_push_back(scommand self, char * argument){
	assert(self != NULL && argument != NULL);
	self->args = g_slist_append(self->args, argument);
	assert(!(self->args == NULL));
}

void scommand_pop_front(scommand self){
	assert(self!=NULL && !(self->args == NULL));
	free(g_slist_nth_data(self->args,(guint)0));
	self->args = g_slist_delete_link(self->args,self->args);
}

void scommand_set_redir_in(scommand self, char * filename){
	assert(self!= NULL);
	if(self->redir_in){
		free(self->redir_in);
	}
	self->redir_in = filename;
}

void scommand_set_redir_out(scommand self, char * filename){
	assert(self!= NULL);
	if(self->redir_out){
		free(self->redir_out);
	}
	self->redir_out = filename;
}

bool scommand_is_empty(const scommand self){
	bool result = false;
	assert(self!=NULL);
	result = g_slist_length(self->args)==(guint)0;
	return result;
}

unsigned int scommand_length(const scommand self){
	unsigned int result = 0u;
	assert(self!=NULL);
	result = g_slist_length(self->args);
	assert((self->args==NULL) == (result == 0u));  //--> ésto da error entra en caso recursivo si se llama a length.
	return(result);
}

char * scommand_front(const scommand self){
	char * result = NULL;
	assert(self!=NULL && !(self->args == NULL));
	result = g_slist_nth_data(self->args,(guint)0);
	assert(result != NULL);
	return result;
}

char * scommand_get_redir_in(const scommand self){
	char * result = NULL;
	assert(self!=NULL);
	result = self->redir_in;
	return result;
}

char * scommand_get_redir_out(const scommand self){
	char * result = NULL;
	assert(self!=NULL);
	result = self->redir_out;
	return result;
}

char * scommand_to_string(const scommand self){
	assert(self!= NULL);
	char *result = strdup("");
	unsigned int length_argument = scommand_length(self);

	for(unsigned int i = 0u; i < length_argument; ++i){
		result = merge_and_free(result,g_slist_nth_data(self->args, i));
		result = merge_and_free(result," ");
	}
	if (self->redir_in!=NULL){
		result = merge_and_free(result," ");
		result = merge_and_free(result,"<");
		result = merge_and_free(result," ");
		result = merge_and_free(result,self->redir_in);
	}
	if (self->redir_out!=NULL){
		result = merge_and_free(result," ");
		result = merge_and_free(result,">");
		result = merge_and_free(result," ");
		result = merge_and_free(result,self->redir_out);
	}
	assert((self->args == NULL) || scommand_get_redir_in(self)==NULL || scommand_get_redir_out(self)==NULL || strlen(result)>0);
	return result;
}





/********** COMANDO PIPELINE **********/

/* Estructura correspondiente a un comando pipeline.
 * Es un 2-upla del tipo ([scommand], bool)
 */

struct pipeline_s {
    GSList *scmds;
    bool wait;
};



pipeline pipeline_new(void){
	pipeline result = calloc(1,sizeof(*result));
	result->scmds=NULL;
	result->wait=true;
	assert(result != NULL && pipeline_is_empty(result) && pipeline_get_wait(result));
	return result;
}

pipeline pipeline_destroy(pipeline self){
	assert(self != NULL);
	while (self->scmds != NULL){
		pipeline_pop_front(self);
	}
	free(self);
	self = NULL;
	assert(self == NULL);
	return self;
}

void pipeline_push_back(pipeline self, scommand sc){
	assert(self != NULL && sc != NULL);
	self->scmds = g_slist_append(self->scmds, sc);
	assert(!(self->scmds==NULL));
}

void pipeline_pop_front(pipeline self){
	assert(self != NULL && !pipeline_is_empty(self));
	scommand_destroy(pipeline_front(self));
	self->scmds = g_slist_delete_link(self->scmds, self->scmds);
}

void pipeline_set_wait(pipeline self, const bool w){
	assert(self != NULL);
	self->wait = w;
}

bool pipeline_is_empty(const pipeline self){
	bool result = false;
	assert(self != NULL);
	result = g_slist_length(self->scmds) == (guint)0;
	return result;
}

unsigned int pipeline_length(const pipeline self){
	unsigned int result = 0u;
	assert(self != NULL);
	result = g_slist_length(self->scmds);
	assert((result==0u) == pipeline_is_empty(self)); //--> pipeline_is_empty es lo mismo que preguntar si self->args == NULL
	return result;
}

scommand pipeline_front(const pipeline self){
	scommand result = NULL;
	assert(self != NULL && !pipeline_is_empty(self));
	result = g_slist_nth_data(self->scmds,(guint)0);
	assert(result != NULL);
	return result;
}

bool pipeline_get_wait(const pipeline self){
	bool result;
	assert(self != NULL);
	result = self-> wait;
	return result;
}

char * pipeline_to_string(const pipeline self){
    assert(self != NULL);
	char *result =strdup("");
	char *str = NULL;

	unsigned int length_scmds = pipeline_length(self);

    for(unsigned int i = 1u; i < length_scmds; i++) {
		result = merge_and_free(result, str=scommand_to_string(g_slist_nth_data(self->scmds,i)));
		free(str);
		result = merge_and_free(result," ");
		result = merge_and_free(result,"|"); 
		result = merge_and_free(result, " ");

    }
	if(!pipeline_get_wait(self)){
		result = merge_and_free(result," ");
		result = merge_and_free(result,"&");
	}
	assert(pipeline_is_empty(self) || pipeline_get_wait(self) || strlen(result)>0);
    return result;
}


