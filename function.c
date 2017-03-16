#include "tvm.h"
#include "from_bytes.h"

tvm_func_data_t tvm_func_data_create
    (tvm_module_t module, unsigned char* begin, unsigned char* end, jit_ushort stack_len, jit_ushort locals_num, jit_ushort labels_num)
{
    tvm_func_data_t data = malloc(sizeof(struct _tvm_func_data));
    data->module = module;
    data->begin = begin;
    data->end = end;
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
	
	//set function data
	jit_function_set_meta(func, 0, data, &jit_free, 1);
	
	jit_function_set_on_demand_compiler(func, tvm_function_build);
	
	return func;
}


int tvm_function_build
	(jit_function_t function)
{
	
	
	
	return JIT_RESULT_OK;
}


