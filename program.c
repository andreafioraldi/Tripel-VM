/*
 * program.c
 *
 * Copyright 2017 Andrea Fioraldi <andreafioraldi@gmail.com>
 *
 * This file is part of Tripel Virtual Machine.
 *
 * Tripel Virtual Machine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tripel Virtual Machine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

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


