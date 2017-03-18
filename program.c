#include "tvm.h"

/**** exported globals var ****/
jit_type_t tvm_start_signature;
jit_type_t tvm_gc_malloc_signature;

jit_type_t tvm_type_string;

char* tvm_libpath;
size_t tvm_libpath_len;

jit_type_t* tvm_types_table;
/******************************/

tvm_module_t tvm_program_find_module
	(tvm_program_t program, char* name)
{
	tvm_module_node_t node = program->modules;
	
	while(node != NULL)
	{
		if(jit_strcmp(node->module->name, name) == 0)
			return node->module;
		
		node = node->next;
	}
	
	return NULL;
}


tvm_program_t tvm_program_create
	(unsigned char* bytecode, unsigned char* bytecode_end)
{
	tvm_program_t program = jit_malloc(sizeof(struct _tvm_program));
	
	program->context = jit_context_create();
	
	program->start = tvm_module_create(program, bytecode, bytecode_end);
	
	return program;
}


tvm_program_t tvm_program_create_build
	(unsigned char* bytecode, unsigned char* bytecode_end)
{
	tvm_program_t program = jit_malloc(sizeof(struct _tvm_program));
	
	program->context = jit_context_create();
	
	program->start = tvm_module_create_build(program, bytecode, bytecode_end);
	
	return program;
}


void tvm_program_free
	(tvm_program_t program)
{
    //free modules linked list
	tvm_module_node_t node = program->modules;
	
	tvm_module_node_t temp;

	while(node != NULL)
	{
		tvm_module_free(node->module);
		
		temp = node->next;
		
		jit_free(node);
		
		node = temp;
	}
	
	//free start module
	tvm_module_free(program->start);
	
	//destroy jit context
	jit_context_destroy(program->context);
	
	jit_free(program);
}


