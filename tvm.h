/*
 * tvm.h
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

#ifndef __TRIPEL__VM__H
#define __TRIPEL__VM__H

#include <jit/jit.h>
#include <jit/jit-type.h>
#include <jit/jit-dynamic.h>

#define GC_THREADS
#include <gc.h>

#define TYPEID_SBYTE                0x0
#define TYPEID_UBYTE                0x1
#define TYPEID_SHORT                0x2
#define TYPEID_USHORT               0x3
#define TYPEID_INT                  0x4
#define TYPEID_UINT                 0x5
#define TYPEID_LONG                 0x6
#define TYPEID_ULONG                0x7
#define TYPEID_FLOAT                0x8
#define TYPEID_DOUBLE               0x9
#define TYPEID_VOID                 0xa
#define TYPEID_VOID_PTR             0xb

#define TYPEID_NATIVE_CHAR          0xc
#define TYPEID_NATIVE_UCHAR         0xd
#define TYPEID_NATIVE_SHORT         0xe
#define TYPEID_NATIVE_USHORT        0xf
#define TYPEID_NATIVE_INT           0x10
#define TYPEID_NATIVE_UINT          0x11
#define TYPEID_NATIVE_LONG          0x12
#define TYPEID_NATIVE_ULONG         0x13
#define TYPEID_NATIVE_LONGLONG      0x14
#define TYPEID_NATIVE_ULONGLONG     0x15
#define TYPEID_NATIVE_FLOAT         0x16
#define TYPEID_NATIVE_DOUBLE        0x17
#define TYPEID_NATIVE_LONGDOUBLE    0x18

#define TYPEID_POINTER              0x19
#define TYPEID_STRUCT               0x1a
#define TYPEID_LIB_STRUCT           0x1b

/*
 * Tripel Bytecode Opcodes
 */
#define OP_NOP                      0x0
#define OP_LD_I8                    0x1
#define OP_LD_U8                    0x2
#define OP_LD_I16                   0x3
#define OP_LD_U16                   0x4
#define OP_LD_I32                   0x5
#define OP_LD_U32                   0x6
#define OP_LD_I64                   0x7
#define OP_LD_U64                   0x8
#define OP_LD_F32                   0x9
#define OP_LD_F64                   0xa
#define OP_LD_NULL                  0xb
#define OP_LD_STR                   0xc
#define OP_ADDR                     0xd
#define OP_VAL                      0xe
#define OP_AT                       0xf
#define OP_AT_C                     0x10
#define OP_AT_1                     0x11
#define OP_AT_2                     0x12
#define OP_AT_3                     0x13
#define OP_AD_AT                    0x14
#define OP_AD_AT_C                  0x15
#define OP_AD_AT_1                  0x16
#define OP_AD_AT_2                  0x17
#define OP_AD_AT_3                  0x18
#define OP_FIELD                    0x19
#define OP_FIELD_0                  0x1a
#define OP_FIELD_1                  0x1b
#define OP_FIELD_2                  0x1c
#define OP_FIELD_3                  0x1d
#define OP_PT_FIELD                 0x1e
#define OP_PT_FIELD_0               0x1f
#define OP_PT_FIELD_1               0x20
#define OP_PT_FIELD_2               0x21
#define OP_PT_FIELD_3               0x22
#define OP_AD_FIELD                 0x23
#define OP_AD_FIELD_0               0x24
#define OP_AD_FIELD_1               0x25
#define OP_AD_FIELD_2               0x26
#define OP_AD_FIELD_3               0x27
#define OP_AD_PT_FIELD              0x28
#define OP_AD_PT_FIELD_0            0x29
#define OP_AD_PT_FIELD_1            0x2a
#define OP_AD_PT_FIELD_2            0x2b
#define OP_AD_PT_FIELD_3            0x2c
#define OP_PUSH                     0x2d
#define OP_PUSH_0                   0x2e
#define OP_PUSH_1                   0x2f
#define OP_PUSH_2                   0x30
#define OP_PUSH_3                   0x31
#define OP_PUSH_AD                  0x32
#define OP_PUSH_AD_0                0x33
#define OP_PUSH_AD_1                0x34
#define OP_PUSH_AD_2                0x35
#define OP_PUSH_AD_3                0x36
#define OP_PUSH_ARG                 0x37
#define OP_PUSH_ARG_0               0x38
#define OP_PUSH_ARG_1               0x39
#define OP_PUSH_ARG_2               0x3a
#define OP_PUSH_ARG_3               0x3b
#define OP_PUSH_GBL                 0x3c
#define OP_PUSH_E_GBL               0x3d
#define OP_POP                      0x3e
#define OP_DUP                      0x3f
#define OP_CLEAR                    0x40
#define OP_DECL_I8                  0x41
#define OP_DECL_U8                  0x42
#define OP_DECL_I16                 0x43
#define OP_DECL_U16                 0x44
#define OP_DECL_I32                 0x45
#define OP_DECL_U32                 0x46
#define OP_DECL_I64                 0x47
#define OP_DECL_U64                 0x48
#define OP_DECL_F32                 0x49
#define OP_DECL_F64                 0x4a
#define OP_DECL_VP                  0x4b
#define OP_DECL_PT                  0x4c
#define OP_DECL_ST                  0x4d
#define OP_DECL_E_ST                0x4e
#define OP_DECL_T                   0x4f
#define OP_STORE                    0x50
#define OP_STORE_0                  0x51
#define OP_STORE_1                  0x52
#define OP_STORE_2                  0x53
#define OP_STORE_3                  0x54
#define OP_STORE_VAL                0x55
#define OP_STORE_VAL_0              0x56
#define OP_STORE_VAL_1              0x57
#define OP_STORE_VAL_2              0x58
#define OP_STORE_VAL_3              0x59
#define OP_STORE_GBL                0x5a
#define OP_STORE_E_GBL              0x5b
#define OP_SET_AT                   0x5c
#define OP_SET_AT_0                 0x5d
#define OP_SET_AT_1                 0x5e
#define OP_SET_AT_2                 0x5f
#define OP_SET_AT_3                 0x60
#define OP_SET_AT_C                 0x61
#define OP_SET_FIELD                0x62
#define OP_SET_FIELD_0              0x63
#define OP_SET_FIELD_1              0x64
#define OP_SET_FIELD_2              0x65
#define OP_SET_FIELD_3              0x66
#define OP_SET_PT_FIELD             0x67
#define OP_SET_PT_FIELD_0           0x68
#define OP_SET_PT_FIELD_1           0x69
#define OP_SET_PT_FIELD_2           0x6a
#define OP_SET_PT_FIELD_3           0x6b
#define OP_S_ALLOC                  0x6c
#define OP_S_ALLOC_C                0x6d
#define OP_GC_ALLOC                 0x6e
#define OP_GC_ALLOC_C               0x6f
#define OP_GC_ATOM_ALLOC            0x70
#define OP_GC_ATOM_ALLOC_C          0x71
#define OP_CALL                     0x72
#define OP_N_CALL                   0x73
#define OP_E_CALL                   0x74
#define OP_EN_CALL                  0x75
#define OP_RET                      0x76
#define OP_RET_STD                  0x77
#define OP_FUNC_AD                  0x78
#define OP_E_FUNC_AD                0x79
#define OP_N_FUNC_AD                0x7a
#define OP_EN_FUNC_AD               0x7b
#define OP_CALL_PT                  0x7c
#define OP_VAL_ASSIGN               0x7d
#define OP_SIZEOF                   0x7e
#define OP_SIZEOF_T                 0x7f
#define OP_SIZEOF_T_MUL             0x80
#define OP_MINUM                    0x81
#define OP_ADD                      0x82
#define OP_SUB                      0x83
#define OP_MUL                      0x84
#define OP_DIV                      0x85
#define OP_REM                      0x86
#define OP_INC                      0x87
#define OP_DEC                      0x88
#define OP_NEG                      0x89
#define OP_AND                      0x8a
#define OP_OR                       0x8b
#define OP_XOR                      0x8c
#define OP_NOT                      0x8d
#define OP_SHL                      0x8e
#define OP_SHR                      0x8f
#define OP_EQ                       0x90
#define OP_NEQ                      0x91
#define OP_LT                       0x92
#define OP_LE                       0x93
#define OP_GT                       0x94
#define OP_GE                       0x95
#define OP_IS_NULL                  0x96
#define OP_TO_BOOL                  0x97
#define OP_TO_BOOL_N                0x98
#define OP_JMP                      0x99
#define OP_JMP_IF                   0x9a
#define OP_JMP_IF_N                 0x9b
#define OP_LABEL                    0x9c
#define OP_CAST_I8                  0x9d
#define OP_CAST_U8                  0x9e
#define OP_CAST_I16                 0x9f
#define OP_CAST_U16                 0xa0
#define OP_CAST_I32                 0xa1
#define OP_CAST_U32                 0xa2
#define OP_CAST_I64                 0xa3
#define OP_CAST_U64                 0xa4
#define OP_CAST_F32                 0xa5
#define OP_CAST_F64                 0xa6
#define OP_CAST_VP                  0xa7
#define OP_CAST_PT                  0xa8
#define OP_CAST_ST                  0xa9
#define OP_CAST_E_ST                0xaa
#define OP_CAST_T                   0xab
#define OP_ABORT                    0xac


/*
!!!ATTENTION!!!
This library doesn't provide any kind of information hiding
due to the need of strong optimization (like macro for inlinig).
This choice is justified by the high importance of performance
in this kind of applications.
Be careful or you will break all!
*/

/*Recurrent functions signature*/
extern jit_type_t tvm_start_signature;
extern jit_type_t tvm_gc_malloc_signature;

/*String type*/
extern jit_type_t tvm_type_string;

/*Directory where libs are stored*/
extern char* tvm_libpath;
extern size_t tvm_libpath_len;

/*
Setup macro, it must be the first instruction in main()
or the library will not works.
It also initialize the garbage collector.
*/
#define TVM_INIT \
do { \
    GC_INIT(); \
    tvm_libpath = getenv("TRIPEL_LIBPATH"); \
    if(tvm_libpath) \
        tvm_libpath_len = jit_strlen(tvm_libpath); \
    else tvm_libpath_len = 0; \
    \
    jit_type_t param[] = { jit_type_ulong }; \
    \
    tvm_gc_malloc_signature = jit_type_create_signature( \
        jit_abi_cdecl, \
        jit_type_void_ptr, \
        param, 1, 0 \
    ); \
    \
    jit_type_t params[] = { jit_type_int, jit_type_void_ptr }; \
    \
    tvm_start_signature = jit_type_create_signature( \
        jit_abi_cdecl, \
        jit_type_int, \
        params, 2, 0 \
    ); \
    \
    tvm_type_string = jit_type_create_pointer(jit_type_sbyte, 0); \
    jit_type_t types_table[] = { \
        jit_type_sbyte, \
        jit_type_ubyte, \
        jit_type_short, \
        jit_type_ushort, \
        jit_type_int, \
        jit_type_uint, \
        jit_type_long, \
        jit_type_ulong, \
        jit_type_float32, \
        jit_type_float64, \
        jit_type_void, \
        jit_type_void_ptr, \
        \
        jit_type_sys_schar, \
        jit_type_sys_uchar, \
        jit_type_sys_short, \
        jit_type_sys_ushort, \
        jit_type_sys_int, \
        jit_type_sys_uint, \
        jit_type_sys_long, \
        jit_type_sys_ulong, \
        jit_type_sys_longlong, \
        jit_type_sys_ulonglong, \
        jit_type_sys_float, \
        jit_type_sys_double, \
        jit_type_sys_long_double \
    }; \
    tvm_types_table = types_table; \
    \
    tvm_type_string = jit_type_create_pointer(jit_type_sbyte, 0); \
} while(0)

/*Primitive types table*/
extern jit_type_t* tvm_types_table;

struct _tvm_module;
struct _tvm_program;

typedef struct _tvm_program* tvm_program_t;
typedef struct _tvm_module* tvm_module_t;

/*
Record used to store all info nedded by a function to be build.
*/
struct _tvm_func_data
{
    tvm_module_t module;
    unsigned char* begin;
    unsigned char* end;
    char* name;
    jit_ushort stack_len;
    jit_ushort locals_num;
    jit_ushort labels_num;
};

typedef struct _tvm_func_data* tvm_func_data_t;

/*Alloc a tvm_func_data_t and set the fields*/
tvm_func_data_t tvm_func_data_create
    (tvm_module_t module, unsigned char* begin, unsigned char* end, char* name, jit_ushort stack_len, jit_ushort locals_num, jit_ushort labels_num);

/*Create a jit_function and insert data*/
jit_function_t tvm_function_create
    (jit_type_t signature, tvm_func_data_t data);

/*Get data stored in a jit_function*/
#define tvm_function_get_data(func) \
    (tvm_func_data_t) jit_function_get_meta(func, 0)

/*Build process, called on demand*/
int tvm_function_build
    (jit_function_t function);

/*
Record used to store a c function pointer and its signature.
*/
struct _tvm_funcptr
{
    void* functor;
    jit_type_t signature;
};

typedef struct _tvm_funcptr tvm_funcptr_t;

/*Free the signature*/
#define tvm_funcptr_free(funcptr) \
    jit_type_free((funcptr).signature)

/*
Record used to store a struct type representation (inside jit) and its fields names.
*/
struct _tvm_struct
{
    char** fields_names;//pointers to bytecode, must not freed
    jit_type_t type;
};

typedef struct _tvm_struct tvm_struct_t;

/*
Record that contains a global var allocated by the GC and its type.
*/
struct _tvm_global_var
{
    void* data;//use GC_malloc
    jit_type_t type;//pointer type
};

typedef struct _tvm_global_var tvm_global_var_t;

/*
Record that represents a module with all of its proprties.
*/
struct _tvm_module
{
    tvm_program_t program;
    char* name;//must not freed
    
    unsigned char* bytecode;
    unsigned char* bytecode_end;
    
    char** structs_names;//pointers in bytecode 
    char** globals_names;
    char** c_funcs_names;
    char** funcs_names;
    
    jit_sbyte** strings;
    tvm_struct_t* structs;
    tvm_global_var_t* globals;
    tvm_funcptr_t* c_funcs;
    
    jit_function_t start;
    jit_function_t* funcs;
    
    tvm_struct_t** ext_structs;
    tvm_global_var_t** ext_globals;
    tvm_funcptr_t** ext_c_funcs;
    jit_function_t** ext_funcs;
    
    jit_ushort strings_len;
    jit_ushort structs_len;
    jit_ushort globals_len;
    jit_ushort c_funcs_len;
    jit_ushort funcs_len;
    
    jit_ushort ext_structs_len;
    jit_ushort ext_globals_len;
    jit_ushort ext_c_funcs_len;
    jit_ushort ext_funcs_len;
};

/*Read an ushort and get a struct by index from a module*/
#define tvm_module_get_struct_type(module, buf) \
    jit_type_copy((module)->structs[tvm_ushort_from_bytes(buf)].type)

/*Read an ushort and get an external struct by index from a module*/
#define tvm_module_get_lib_struct_type(module, buf) \
    jit_type_copy((module)->ext_structs[tvm_ushort_from_bytes(buf)]->type)

/*Read and get a type associated with a module*/
jit_type_t tvm_module_get_type
    (tvm_module_t module, unsigned char** buf); //must freed with jit_type_free

/*Read a type associated with a module and get its pointer type*/
#define tvm_module_get_pointer_type(module, buf) \
    jit_type_create_pointer(tvm_module_get_type(module, buf), 0)

/*Parse bytecode and fill module fields*/
void tvm_module_build
    (tvm_module_t module);

/*Alloc a module and set bytecode pointers fields*/
tvm_module_t tvm_module_create
    (tvm_program_t program, unsigned char* bytecode, unsigned char* bytecode_end);

/*Create and build a module*/
tvm_module_t tvm_module_create_build
    (tvm_program_t program, unsigned char* bytecode, unsigned char* bytecode_end);

/*Free a module and all of its fields*/
void tvm_module_free
    (tvm_module_t module);

//linked list
struct _tvm_module_node;
typedef struct _tvm_module_node* tvm_module_node_t;

/*Node of a linked list of modules*/
struct _tvm_module_node
{
    tvm_module_t module;
    tvm_module_node_t next;
};


/*
Record that represents a Tripel program.
*/
struct _tvm_program
{
    tvm_module_t start;
    tvm_module_node_t modules;
    jit_context_t context;
};

/*Alloc a program and set bytecode pointers in start module*/
tvm_program_t tvm_program_create
    (unsigned char* bytecode, unsigned char* bytecode_end);

/*Search a module in a program*/
tvm_module_t tvm_program_find_module
    (tvm_program_t program, char* name);

/*Build start module*/
#define tvm_program_build(program) \
    tvm_module_build((program)->start)

/*Create and build a program*/
tvm_program_t tvm_program_create_build
    (unsigned char* bytecode, unsigned char* bytecode_end);

/*Free a program, destroy the jit_context and free all modules*/
void tvm_program_free
    (tvm_program_t program);

/*Run start function of the start module*/
#define tvm_program_run(program, argc, argv) \
    ((int (*)(int, char**)) jit_function_to_closure((program)->start->start))(argc, argv)


#endif
