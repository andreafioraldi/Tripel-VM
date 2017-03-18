#include "tvm.h"
#include "from_bytes.h"
#include <stdlib.h>
#include <stdio.h>

tvm_func_data_t tvm_func_data_create
    (tvm_module_t module, unsigned char* begin, unsigned char* end, char* name, jit_ushort stack_len, jit_ushort locals_num, jit_ushort labels_num)
{
    tvm_func_data_t data = jit_malloc(sizeof(struct _tvm_func_data));
    data->module = module;
    data->begin = begin;
    data->end = end;
    data->name = name;
    data->stack_len = stack_len;
    data->locals_num = locals_num;
    data->labels_num = labels_num;
    return data;
}

jit_function_t tvm_function_create
	(jit_type_t signature, tvm_func_data_t data)
{
	//create the jit function
	jit_function_t func = jit_function_create(data->module->program->context, signature);
	/*
	printf("module = %p\n", data->module);
	printf("begin = %p\n", data->begin);
	printf("end = %p\n", data->end);
	printf("name = %s\n", data->name);
	printf("stack_len = %d\n", data->stack_len);
	printf("locals_num = %d\n", data->locals_num);
	printf("labels_num = %d\n", data->labels_num);
	*/
	//set function data
	jit_function_set_meta(func, 0, data, &jit_free, 1);
	
	//set build function
	jit_function_set_on_demand_compiler(func, &tvm_function_build);
	
	return func;
}


int tvm_function_build
	(jit_function_t function)
{
	tvm_func_data_t data = tvm_function_get_data(function);
	
	register unsigned char* ip;
	for(ip = data->begin; ip < data->end; ++ip)
	{
	    printf("<%p>  %d %x\n", ip, *ip, *ip);
	}
	
	return JIT_RESULT_OK;
}


