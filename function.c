/*
 * function.c
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
    
    //alloc vm stack
    jit_value_t* stack = jit_malloc(data->stack_len * sizeof(jit_value_t));
    jit_value_t* stack_base = stack;
    
    unsigned char* buf;
    for(buf = data->begin; buf < data->end; ++buf)
    {
        switch(*buf)
        {
            case OP_NOP:
            {
                jit_insn_nop(function);
                break;
            }
            case OP_LD_I8: //imm
            {
                *stack = jit_value_create_nint_constant(function, jit_type_sbyte, *(++buf));
                ++stack;
                break;
            }
            case OP_LD_U8: //imm
            {
                *stack = jit_value_create_nint_constant(function, jit_type_ubyte, *(++buf));
                ++stack;
                break;
            }
            case OP_LD_I16: //imm
            {
                jit_short tmp = tvm_short_from_bytes(buf);
                *stack = jit_value_create_nint_constant(function, jit_type_short, tmp);
                ++stack;
                break;
            }
            case OP_LD_U16: //imm
            {
                jit_ushort tmp = tvm_ushort_from_bytes(buf);
                *stack = jit_value_create_nint_constant(function, jit_type_ushort, tmp);
                ++stack;
                break;
            }
            case OP_LD_I32: //imm
            {
                jit_int tmp = tvm_int_from_bytes(buf);
                *stack = jit_value_create_nint_constant(function, jit_type_int, tmp);
                ++stack;
                break;
            }
            case OP_LD_U32: //imm
            {
                jit_uint tmp = tvm_uint_from_bytes(buf);
                *stack = jit_value_create_nint_constant(function, jit_type_uint, tmp);
                ++stack;
                break;
            }
            case OP_LD_I64: //imm
            {
                jit_long tmp = tvm_long_from_bytes(buf);
                *stack = jit_value_create_long_constant(function, jit_type_long, tmp);
                ++stack;
                break;
            }
            case OP_LD_U64: //imm
            {
                jit_ulong tmp = tvm_ulong_from_bytes(buf);
                *stack = jit_value_create_long_constant(function, jit_type_ulong, tmp);
                ++stack;
                break;
            }
            case OP_LD_F32: //imm
            {
                jit_float32 tmp = tvm_float32_from_bytes(buf);
                *stack = jit_value_create_float32_constant(function, jit_type_float32, tmp);
                ++stack;
                break;
            }
            case OP_LD_F64: //imm
            {
                jit_float64 tmp = tvm_float64_from_bytes(buf);
                *stack = jit_value_create_float64_constant(function, jit_type_float64, tmp);
                ++stack;
                break;
            }
            case OP_LD_NULL:
            {
                *stack = jit_value_create_nint_constant(function, jit_type_void_ptr, NULL);
                ++stack;
                break;
            }
            case OP_LD_STR: //imm
            {
                jit_ushort tmp = tvm_ushort_from_bytes(buf);
                *stack = jit_value_create_nint_constant(function, tvm_type_string, data->module->strings[tmp]);
                ++stack;
                break;
            }
            case OP_ADDR:
            {
                --stack;
                *stack = jit_insn_address_of(function, *stack);
                break;
            }
            case OP_VAL:
            {
                --stack;
                *stack = jit_insn_load_relative(function, *stack, 0, jit_type_get_ref(jit_value_get_type(*stack)));
                break;
            }
            case OP_AT:
            {
                --stack;
                jit_value_t idx = *stack;
                --stack;
                jit_type_t type = jit_type_get_ref(jit_value_get_type(*stack));
                *stack = jit_insn_load_elem(function, *stack, idx, type);
                break;
            }
            case OP_AT_C: //imm
            {
                jit_uint tmp = tvm_uint_from_bytes(buf);
                --stack;
                jit_type_t type = jit_type_get_ref(jit_value_get_type(*stack));
                *stack = jit_insn_add_relative(function, *stack, tmp*jit_type_get_size(type), type);
                break;
            }
            case OP_AT_1:
            {
                --stack;
                jit_type_t type = jit_type_get_ref(jit_value_get_type(*stack));
                *stack = jit_insn_add_relative(function, *stack, jit_type_get_size(type), type);
                break;
            }
            case OP_AT_2:
            {
                --stack;
                jit_type_t type = jit_type_get_ref(jit_value_get_type(*stack));
                *stack = jit_insn_add_relative(function, *stack, 2*jit_type_get_size(type), type);
                break;
            }
            case OP_AT_3:
            {
                --stack;
                jit_type_t type = jit_type_get_ref(jit_value_get_type(*stack));
                *stack = jit_insn_add_relative(function, *stack, 3*jit_type_get_size(type), type);
                break;
            }
            case OP_AD_AT:
            {
                --stack;
                jit_value_t idx = *stack;
                --stack;
                jit_type_t type = jit_type_get_ref(jit_value_get_type(*stack));
                *stack = jit_insn_load_elem_address(function, *stack, idx, type);
                break;
            }
            case OP_AD_AT_C:
            {
                jit_uint tmp = tvm_uint_from_bytes(buf);
                --stack;
                *stack = jit_insn_add_relative(function, *stack, tmp*jit_type_get_size(jit_value_get_type(*stack)));
                break;
            }
            case OP_AD_AT_1:
            {
                --stack;
                *stack = jit_insn_add_relative(function, *stack, jit_type_get_size(jit_value_get_type(*stack)));
                break;
            }
            case OP_AD_AT_2:
            {
                --stack;
                *stack = jit_insn_add_relative(function, *stack, 2*jit_type_get_size(jit_value_get_type(*stack)));
                break;
            }
            case OP_AD_AT_3:
            {
                --stack;
                *stack = jit_insn_add_relative(function, *stack, 3*jit_type_get_size(jit_value_get_type(*stack)));
                break;
            }
            case OP_FIELD:
            {
                jit_ushort tmp = tvm_ushort_from_bytes(buf);
                --stack;
                jit_type_t type = jit_value_get_type(*stack);
                *stack = jit_insn_load_relative(function, *stack, jit_type_get_offset(type, tmp), jit_type_get_field(type, tmp));
                break;
            }
            case OP_FIELD_0:
            {
                jit_ushort tmp = tvm_ushort_from_bytes(buf);
                --stack;
                jit_type_t type = jit_value_get_type(*stack);
                *stack = jit_insn_load_relative(function, *stack, 0, jit_type_get_field(type, 0));
                break;
            }
            case OP_FIELD_1:
            {
                jit_ushort tmp = tvm_ushort_from_bytes(buf);
                --stack;
                *stack = jit_insn_load_relative(function, *stack, jit_type_get_offset(type, 1), jit_type_get_field(type, 1));
                break;
            }
            case OP_FIELD_2:
            {
                jit_ushort tmp = tvm_ushort_from_bytes(buf);
                --stack;
                *stack = jit_insn_load_relative(function, *stack, jit_type_get_offset(type, 2), jit_type_get_field(type, 2));
                break;
            }
            case OP_FIELD_3:
            {
                jit_ushort tmp = tvm_ushort_from_bytes(buf);
                --stack;
                *stack = jit_insn_load_relative(function, *stack, jit_type_get_offset(type, 3), jit_type_get_field(type, 3));
                break;
            }
            case OP_PT_FIELD:
            {
                jit_ushort tmp = tvm_ushort_from_bytes(buf);
                --stack;
                jit_type_t type = jit_value_get_type(*stack);
                *stack = jit_insn_add_relative(function, *stack, jit_type_get_offset(type, tmp));
                break;
            }
            case OP_PT_FIELD_0:
            {
                
                break;
            }
            case OP_PT_FIELD_1:
            {
                
                break;
            }
            case OP_PT_FIELD_2:
            {
                
                break;
            }
            case OP_PT_FIELD_3:
            {
                
                break;
            }
            case OP_AD_FIELD:
            {
                
                break;
            }
            case OP_AD_FIELD_0:
            {
                
                break;
            }
            case OP_AD_FIELD_1:
            {
                
                break;
            }
            case OP_AD_FIELD_2:
            {
                
                break;
            }
            case OP_AD_FIELD_3:
            {
                
                break;
            }
            case OP_AD_PT_FIELD:
            {
                
                break;
            }
            case OP_AD_PT_FIELD_0:
            {
                
                break;
            }
            case OP_AD_PT_FIELD_1:
            {
                
                break;
            }
            case OP_AD_PT_FIELD_2:
            {
                
                break;
            }
            case OP_AD_PT_FIELD_3:
            {
                
                break;
            }
            case OP_PUSH:
            {
                
                break;
            }
            case OP_PUSH_0:
            {
                
                break;
            }
            case OP_PUSH_1:
            {
                
                break;
            }
            case OP_PUSH_2:
            {
                
                break;
            }
            case OP_PUSH_3:
            {
                
                break;
            }
            case OP_PUSH_AD:
            {
                
                break;
            }
            case OP_PUSH_AD_0:
            {
                
                break;
            }
            case OP_PUSH_AD_1:
            {
                
                break;
            }
            case OP_PUSH_AD_2:
            {
                
                break;
            }
            case OP_PUSH_AD_3:
            {
                
                break;
            }
            case OP_PUSH_ARG:
            {
                
                break;
            }
            case OP_PUSH_ARG_0:
            {
                
                break;
            }
            case OP_PUSH_ARG_1:
            {
                
                break;
            }
            case OP_PUSH_ARG_2:
            {
                
                break;
            }
            case OP_PUSH_ARG_3:
            {
                
                break;
            }
            case OP_PUSH_GBL:
            {
                
                break;
            }
            case OP_PUSH_E_GBL:
            {
                
                break;
            }
            case OP_POP:
            {
                
                break;
            }
            case OP_DUP:
            {
                
                break;
            }
            case OP_CLEAR:
            {
                
                break;
            }
            case OP_DECL_I8:
            {
                
                break;
            }
            case OP_DECL_U8:
            {
                
                break;
            }
            case OP_DECL_I16:
            {
                
                break;
            }
            case OP_DECL_U16:
            {
                
                break;
            }
            case OP_DECL_I32:
            {
                
                break;
            }
            case OP_DECL_U32:
            {
                
                break;
            }
            case OP_DECL_I64:
            {
                
                break;
            }
            case OP_DECL_U64:
            {
                
                break;
            }
            case OP_DECL_F32:
            {
                
                break;
            }
            case OP_DECL_F64:
            {
                
                break;
            }
            case OP_DECL_VP:
            {
                
                break;
            }
            case OP_DECL_PT:
            {
                
                break;
            }
            case OP_DECL_ST:
            {
                
                break;
            }
            case OP_DECL_E_ST:
            {
                
                break;
            }
            case OP_DECL_T:
            {
                
                break;
            }
            case OP_STORE:
            {
                
                break;
            }
            case OP_STORE_0:
            {
                
                break;
            }
            case OP_STORE_1:
            {
                
                break;
            }
            case OP_STORE_2:
            {
                
                break;
            }
            case OP_STORE_3:
            {
                
                break;
            }
            case OP_STORE_VAL:
            {
                
                break;
            }
            case OP_STORE_VAL_0:
            {
                
                break;
            }
            case OP_STORE_VAL_1:
            {
                
                break;
            }
            case OP_STORE_VAL_2:
            {
                
                break;
            }
            case OP_STORE_VAL_3:
            {
                
                break;
            }
            case OP_STORE_GBL:
            {
                
                break;
            }
            case OP_STORE_E_GBL:
            {
                
                break;
            }
            case OP_SET_AT:
            {
                
                break;
            }
            case OP_SET_AT_0:
            {
                
                break;
            }
            case OP_SET_AT_1:
            {
                
                break;
            }
            case OP_SET_AT_2:
            {
                
                break;
            }
            case OP_SET_AT_3:
            {
                
                break;
            }
            case OP_SET_AT_C:
            {
                
                break;
            }
            case OP_SET_FIELD:
            {
                
                break;
            }
            case OP_SET_FIELD_0:
            {
                
                break;
            }
            case OP_SET_FIELD_1:
            {
                
                break;
            }
            case OP_SET_FIELD_2:
            {
                
                break;
            }
            case OP_SET_FIELD_3:
            {
                
                break;
            }
            case OP_SET_PT_FIELD:
            {
                
                break;
            }
            case OP_SET_PT_FIELD_0:
            {
                
                break;
            }
            case OP_SET_PT_FIELD_1:
            {
                
                break;
            }
            case OP_SET_PT_FIELD_2:
            {
                
                break;
            }
            case OP_SET_PT_FIELD_3:
            {
                
                break;
            }
            case OP_S_ALLOC:
            {
                
                break;
            }
            case OP_S_ALLOC_C:
            {
                
                break;
            }
            case OP_GC_ALLOC:
            {
                
                break;
            }
            case OP_GC_ALLOC_C:
            {
                
                break;
            }
            case OP_GC_ATOM_ALLOC:
            {
                
                break;
            }
            case OP_GC_ATOM_ALLOC_C:
            {
                
                break;
            }
            case OP_CALL:
            {
                
                break;
            }
            case OP_N_CALL:
            {
                
                break;
            }
            case OP_E_CALL:
            {
                
                break;
            }
            case OP_EN_CALL:
            {
                
                break;
            }
            case OP_RET:
            {
                
                break;
            }
            case OP_RET_STD:
            {
                
                break;
            }
            case OP_FUNC_AD:
            {
                
                break;
            }
            case OP_E_FUNC_AD:
            {
                
                break;
            }
            case OP_N_FUNC_AD:
            {
                
                break;
            }
            case OP_EN_FUNC_AD:
            {
                
                break;
            }
            case OP_CALL_PT:
            {
                
                break;
            }
            case OP_VAL_ASSIGN:
            {
                
                break;
            }
            case OP_SIZEOF:
            {
                
                break;
            }
            case OP_SIZEOF_T:
            {
                
                break;
            }
            case OP_SIZEOF_T_MUL:
            {
                
                break;
            }
            case OP_MINUM:
            {
                
                break;
            }
            case OP_ADD:
            {
                
                break;
            }
            case OP_SUB:
            {
                
                break;
            }
            case OP_MUL:
            {
                
                break;
            }
            case OP_DIV:
            {
                
                break;
            }
            case OP_REM:
            {
                
                break;
            }
            case OP_INC:
            {
                
                break;
            }
            case OP_DEC:
            {
                
                break;
            }
            case OP_NEG:
            {
                
                break;
            }
            case OP_AND:
            {
                
                break;
            }
            case OP_OR:
            {
                
                break;
            }
            case OP_XOR:
            {
                
                break;
            }
            case OP_NOT:
            {
                
                break;
            }
            case OP_SHL:
            {
                
                break;
            }
            case OP_SHR:
            {
                
                break;
            }
            case OP_EQ:
            {
                
                break;
            }
            case OP_NEQ:
            {
                
                break;
            }
            case OP_LT:
            {
                
                break;
            }
            case OP_LE:
            {
                
                break;
            }
            case OP_GT:
            {
                
                break;
            }
            case OP_GE:
            {
                
                break;
            }
            case OP_IS_NULL:
            {
                
                break;
            }
            case OP_TO_BOOL:
            {
                
                break;
            }
            case OP_TO_BOOL_N:
            {
                
                break;
            }
            case OP_JMP:
            {
                
                break;
            }
            case OP_JMP_IF:
            {
                
                break;
            }
            case OP_JMP_IF_N:
            {
                
                break;
            }
            case OP_LABEL:
            {
                
                break;
            }
            case OP_CAST_I8:
            {
                
                break;
            }
            case OP_CAST_U8:
            {
                
                break;
            }
            case OP_CAST_I16:
            {
                
                break;
            }
            case OP_CAST_U16:
            {
                
                break;
            }
            case OP_CAST_I32:
            {
                
                break;
            }
            case OP_CAST_U32:
            {
                
                break;
            }
            case OP_CAST_I64:
            {
                
                break;
            }
            case OP_CAST_U64:
            {
                
                break;
            }
            case OP_CAST_F32:
            {
                
                break;
            }
            case OP_CAST_F64:
            {
                
                break;
            }
            case OP_CAST_VP:
            {
                
                break;
            }
            case OP_CAST_PT:
            {
                
                break;
            }
            case OP_CAST_ST:
            {
                
                break;
            }
            case OP_CAST_E_ST:
            {
                
                break;
            }
            case OP_CAST_T:
            {
                
                break;
            }
            case OP_ABORT:
            {
                jit_type_t params_types[] = { jit_type_int };
                jit_type_t sign = jit_type_create_signature(jit_abi_cdecl, jit_type_int, params_types, 1, 0);
                --stack;
                *stack = jit_insn_call_native(function, "abort", exit, sign, stack, EXIT_FAILURE, 0);
                break;
            }
            default:
            fprintf(stderr, "VIRTUAL MACHINE FATAL ERROR!!! unrecognized opcode %x\n", *buf);
            exit(EXIT_FAILURE);
        }
    }
    
    jit_free(stack_base);
    
    return JIT_RESULT_OK;
}


