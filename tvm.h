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

extern jit_type_t tvm_gc_malloc_signature;
extern jit_type_t tvm_type_string;

extern char const* tvm_libpath;
extern const size_t tvm_libpath_len;

#define TVM_INIT \
do { \
    GC_INIT(); \
    _tvm_libpath = getenv("HOP_LIBPATH"); \
    if(_tvm_libpath) \
        _tvm_libpath_len = jit_strlen(_tvm_libpath); \
    else _tvm_libpath_len = 0; \
    \
    jit_type_t param[] = { jit_type_ulong }; \
    \
    tvm_gc_malloc_signature = jit_type_create_signature( \
        jit_abi_cdecl, \
        jit_type_void_ptr, \
        param, 1, 0 \
    ); \
    \
    tvm_type_string = jit_type_create_pointer(jit_type_sbyte, 0); \
    jit_type_t types_table = { \
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
} while(0)

extern jit_type_t* tvm_types_table;

struct _tvm_module;
struct _tvm_program;

typedef struct _tvm_program* tvm_program_t;
typedef struct _tvm_module* tvm_module_t;


struct _tvm_func_data
{
    tvm_module_t module;
    unsigned char* begin;
    unsigned char* end;
    jit_ushort stack_len;
    jit_ushort locals_num;
    jit_ushort labels_num;
};

typedef struct _tvm_func_data* tvm_func_data_t;

tvm_func_data_t tvm_func_data_create
    (tvm_module_t module, unsigned char* begin, unsigned char* end, jit_ushort stack_len, jit_ushort locals_num, jit_ushort labels_num);

jit_function_t tvm_function_create
    (jit_type_t signature, tvm_func_data_t data);

#define tvm_function_get_data(func) \
    (tvm_func_data_t) jit_function_get_meta(func, 0)

//called on demand
int tvm_function_build
    (jit_function_t function);




struct _tvm_funcptr
{
    void* functor;
    jit_type_t signature;
};

typedef struct _tvm_funcptr tvm_funcptr_t;

#define tvm_funcptr_free(funcptr) \
    jit_type_free((funcptr).signature)



struct _tvm_struct
{
    char** fields_names;//pointers to bytecode, must not freed
    jit_type_t type;
};

typedef struct _tvm_struct tvm_struct_t;



struct _tvm_global_var
{
    void* data;//use GC_malloc
    jit_type_t type;//pointer type
};

typedef struct _tvm_global_var tvm_global_var_t;



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

#define tvm_module_get_struct_type(module, buf) \
    jit_type_copy((module)->structs[tvm_ushort_from_bytes(buf)].type)

#define tvm_module_get_lib_struct_type(module, buf) \
    jit_type_copy((module)->ext_structs[tvm_ushort_from_bytes(buf)]->type)

jit_type_t tvm_module_get_type
    (tvm_module_t module, unsigned char** buf); //must freed with jit_type_free

jit_type_t tvm_module_get_pointer_type
    (tvm_module_t module, unsigned char** buf); //must freed with jit_type_free

void tvm_module_build
    (tvm_module_t module);

tvm_module_t tvm_module_create
    (tvm_program_t program, unsigned char* bytecode, unsigned char* bytecode_end);

tvm_module_t tvm_module_create_build
    (tvm_program_t program, unsigned char* bytecode, unsigned char* bytecode_end);

void tvm_module_free
    (tvm_module_t module);



//linked list
struct _tvm_module_node;
typedef struct _tvm_module_node* tvm_module_node_t;

struct _tvm_module_node
{
    tvm_module_t module;
    tvm_module_node_t next;
};

struct _tvm_program
{
    tvm_module_t start;
    tvm_module_node_t modules;
    jit_context_t context;
};

tvm_program_t tvm_program_create
    (unsigned char* bytecode, unsigned char* bytecode_end);

tvm_module_t tvm_program_find_module
    (tvm_program_t program, char* name);

#define tvm_program_build(program) \
    tvm_module_build((program)->start)

tvm_program_t tvm_program_create
    (unsigned char* bytecode, unsigned char* bytecode_end);

tvm_program_t tvm_program_create_build
    (unsigned char* bytecode, unsigned char* bytecode_end);

void tvm_program_free
    (tvm_program_t program);

#define tvm_program_run(program, argc, argv) \
    ((int (*)(int, char**)) jit_function_to_closure((program)->start->start))(argc, argv)

#endif
