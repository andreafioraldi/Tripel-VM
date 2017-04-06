/*
 * module.c
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

/*Read an ushort and get a struct by index from a module*/
#define tvm_module_get_struct_type(module, buf) \
    jit_type_copy(((module)->structs+(int)tvm_ushort_from_bytes(buf))->type)

/*Read an ushort and get an external struct by index from a module*/
#define tvm_module_get_ext_struct_type(module, buf) \
    jit_type_copy((*((module)->ext_structs+(int)tvm_ushort_from_bytes(buf)))->type)

jit_type_t tvm_module_get_type
    (tvm_module_t module, unsigned char** buf)
{
    int id = *((*buf)++);
    switch(id)
    {
        case TYPEID_POINTER:
        return tvm_module_get_pointer_type(module, buf);

        case TYPEID_STRUCT:
        return tvm_module_get_struct_type(module, buf);

        case TYPEID_LIB_STRUCT:
        return tvm_module_get_ext_struct_type(module, buf);

        default:
        return tvm_types_table[id];
    }
}

/*Define path separator*/
#ifdef _WIN32
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

void tvm_module_build
    (tvm_module_t module)
{
    unsigned char* buf = module->bytecode;

    //remove header if present
    if(module->bytecode_end >= buf + 22 && jit_strncmp(buf, "#!/usr/bin/env tripel\n", 22) == 0)
        buf += 22;

    char* name;
    size_t name_len;
    int i, j;

    //read the number of string constants
    jit_ushort num = tvm_ushort_from_bytes(buf);

    //alloc space for strings
    module->strings = jit_malloc(sizeof(jit_sbyte*) * num);
    module->strings_len = num;

    if(num != 0)
    {
        *module->strings = buf;

        //assign pointers to code for each string
        for(i = 0; i < num -1; ++i)
        {
            //increment buffer until NUL
            while(*(buf++)) ;

            module->strings[i] = buf;
        }

        //increment buffer until NUL for the last string
        while(*(buf++)) ;
    }

    //read the number of structure definitions
    num = tvm_ushort_from_bytes(buf);

    //alloc space for structs
    module->structs = jit_malloc(sizeof(tvm_struct_t) * num);
    module->structs_len = num;
    module->structs_map = tvm_map_create(num);

    for(i = 0; i < num; ++i)
    {
        name = buf;

        while(*(buf++)) ;

        //read struct fields number
        jit_ushort fields_num = tvm_ushort_from_bytes(buf);

        char** fields_names = jit_malloc(sizeof(char*) * fields_num);

        jit_type_t* fields_types = jit_malloc(sizeof(jit_type_t) * fields_num);

        //read fields names and types
        for(j = 0; j < fields_num; ++j)
        {
            fields_names[j] = buf;

            while(*(buf++)) ;

            fields_types[j] = tvm_module_get_type(module, &buf);
        }

        //create struct type
        module->structs[i].type = jit_type_create_struct(fields_types, fields_num, 0);
        module->structs[i].fields_names = fields_names;

        tvm_map_add(module->structs_map, name, module->structs+i);

        //free types array
        jit_free(fields_types);
    }
printf(" globals %X\n", buf-module->bytecode);
    //read the number of global vars
    num = tvm_ushort_from_bytes(buf);

    //alloc globals vars container and add it to garbage collector roots
    module->globals = jit_malloc(sizeof(tvm_global_var_t) * num);
    //GC_add_roots(module->globals_pool, sizeof(struct _tvm_global_var) * num);
    module->globals_len = num;
    module->globals_map = tvm_map_create(num);

    for(i = 0; i < num; ++i)
    {
        name = buf;

        while(*(buf++)) ;

        jit_type_t type = tvm_module_get_type(module, &buf);

        //gc alloc global var space and set to 0
        size_t type_size = jit_type_get_size(type);
        module->globals[i].data = GC_MALLOC(type_size);
        jit_memset(module->globals[i].data, 0, type_size);

        //set pointer type
        module->globals[i].type = jit_type_create_pointer(type, 0);

        tvm_map_add(module->globals_map, name, module->globals+i);
    }

    //read the number of native function pointers
    num = tvm_ushort_from_bytes(buf);

    module->c_funcs = jit_malloc(sizeof(tvm_funcptr_t) * num);
    module->c_funcs_len = num;
    module->c_funcs_map = tvm_map_create(num);

    //read the number of native libraries
    jit_ushort libs_num = tvm_ushort_from_bytes(buf);

    for(i = 0; i < libs_num; ++i)
    {
        name = buf;
        while(*(buf++)) ;

        jit_dynlib_handle_t handle;

        //load natiive library
        handle = jit_dynlib_open(name);
        if(handle == NULL)
        {
            fprintf(stderr, "fatal VM error! native library %s not found.\n", name);
            exit(EXIT_FAILURE);
        }

        //get the number of functions to import
        jit_ushort l_num = tvm_ushort_from_bytes(buf);

        char * fname;
        jit_ushort params_num;
        jit_type_t * params;

        for(j = 0; j < l_num; ++j)
        {
            //get imported function name
            fname = buf;
            while(*(buf++)) ;

            //get function pointer from lib
            void * functor = jit_dynlib_get_symbol(handle, fname);
            if(functor == NULL)
            {
                fprintf(stderr, "fatal VM error! native symbol %s in %s not found.\n", fname, name);
                exit(EXIT_FAILURE);
            }

            jit_type_t ret_type = tvm_module_get_type(module, &buf);

            //read parameters number
            params_num = tvm_ushort_from_bytes(buf);

            params = jit_malloc(sizeof(jit_type_t) * params_num);

            //read parameters types
            int k;
            for(k = 0; k < params_num; ++k)
                params[k] = tvm_module_get_type(module, &buf);

            //fill funcptr record at index i in the module
            module->c_funcs[i].signature = jit_type_create_signature(jit_abi_cdecl, ret_type, params, params_num, 0);
            module->c_funcs[i].functor = functor;

            tvm_map_add(module->c_funcs_map, fname, module->c_funcs+i);

            jit_free(params);
        }
    }

    //start
    jit_ushort stack_len, locals_num, labels_num;

    //get properties
    stack_len = tvm_ushort_from_bytes(buf);
    locals_num = tvm_ushort_from_bytes(buf);
    labels_num = tvm_ushort_from_bytes(buf);

    //get function code length
    jit_uint len = tvm_uint_from_bytes(buf);
    tvm_func_data_t func_data = tvm_func_data_create(module, buf, buf + len, "<start>", stack_len, locals_num, labels_num);

    module->start = tvm_function_create(tvm_start_signature, func_data);

    buf += len;

    //read the number of functions
    num = tvm_ushort_from_bytes(buf);

    module->funcs = jit_malloc(sizeof(jit_function_t) * num);
    module->funcs_len = num;
    module->funcs_map = tvm_map_create(num);

    jit_ushort params_num;
    jit_type_t * params;

    for(i = 0; i < num; ++i)
    {
        name = buf;
        while(*(buf++)) ;

        //get return type
        jit_type_t ret_type = tvm_module_get_type(module, &buf);

        //read the number of parameters
        params_num = tvm_ushort_from_bytes(buf);

        params = jit_malloc(sizeof(jit_type_t) * params_num);

        //read parameters types
        int k;
        for(k = 0; k < params_num; ++k)
            params[k] = tvm_module_get_type(module, &buf);

        //create signature
        jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, ret_type, params, params_num, 0);

        //get properties
        stack_len = tvm_ushort_from_bytes(buf);
        locals_num = tvm_ushort_from_bytes(buf);
        labels_num = tvm_ushort_from_bytes(buf);

        //get function code length
        len = tvm_uint_from_bytes(buf);

        //create function
        func_data = tvm_func_data_create(module, buf, buf + len, name, stack_len, locals_num, labels_num);
        module->funcs[i] = tvm_function_create(signature, func_data);

        tvm_map_add(module->funcs_map, name, module->funcs+i);

        buf += len;
    }

    //external symbols counters
    module->ext_structs_len = tvm_ushort_from_bytes(buf);
    module->ext_globals_len = tvm_ushort_from_bytes(buf);
    module->ext_c_funcs_len = tvm_ushort_from_bytes(buf);
    module->ext_funcs_len = tvm_ushort_from_bytes(buf);

    //alloc external symbols
    module->ext_structs = jit_malloc(sizeof(void*) * module->ext_structs_len);
    module->ext_globals = jit_malloc(sizeof(void*) * module->ext_globals_len);
    module->ext_c_funcs = jit_malloc(sizeof(void*) * module->ext_c_funcs_len);
    module->ext_funcs = jit_malloc(sizeof(void*) * module->ext_funcs_len);

    //iterators
    tvm_struct_t** ext_structs_it = module->ext_structs;
    tvm_global_var_t** ext_globals_it = module->ext_globals;
    tvm_funcptr_t** ext_c_funcs_it = module->ext_c_funcs;
    jit_function_t** ext_funcs_it = module->ext_funcs;

    //read the number of libraries
    num = tvm_ushort_from_bytes(buf);

    for(i = 0; i < num; ++i)
    {
        name = buf;

        while(*buf)
            ++buf;
        ++buf;

        name_len = (char*)buf - name;

        tvm_module_t lib = tvm_program_find_module(module->program, name);

        //if the module is not found load it
        if(lib == NULL)
        {
            char * path = jit_malloc(tvm_libpath_len + name_len + 5);

            jit_memcpy(path, name, name_len);
            jit_memcpy(path + name_len, ".tripel", 8);

            FILE * fp = fopen(path, "r");

            if(fp == NULL)
            {
                jit_memmove(path + tvm_libpath_len, path, name_len + 5);
                jit_memcpy(path, tvm_libpath, tvm_libpath_len);

                fp = fopen(path, "r");

                if(fp == NULL)
                {
                    fprintf(stderr, "fatal VM error! library %s not found.\n", name);
                    exit(EXIT_FAILURE);
                }
            }

            fseek(fp, 0, SEEK_END);
            long file_size = ftell(fp);
            rewind(fp);

            unsigned char* file_content = jit_malloc(file_size);

            long k;
            for(k = 0; k < file_size; ++k)
                file_content[k] = fgetc(fp);

            jit_free(path);

            lib = tvm_module_create_build(module->program, file_content, file_content + file_size);

            //add the module to the program
            tvm_map_add(module->program->modules, name, lib);

            //call the start entry of the module
            ((int (*)())jit_function_to_closure(lib->start))();
        }

        //read the number of structure definitions to import from the lib
        jit_ushort l_num = tvm_ushort_from_bytes(buf);

        for(j = 0; j < l_num; ++j)
        {
            name = buf;

            while(*(buf++)) ;

            *ext_structs_it = tvm_map_get(lib->structs_map, name);
            //TODO error check
            ++ext_structs_it;
        }

        //read the number of global vars to import from the lib
        l_num = tvm_ushort_from_bytes(buf);

        for(j = 0; j < l_num; ++j)
        {
            name = buf;

            while(*(buf++)) ;

            *ext_globals_it = tvm_map_get(lib->globals_map, name);
            ++ext_globals_it;
        }

        //read the number of native functions to import from the lib
        l_num = tvm_ushort_from_bytes(buf);

        for(j = 0; j < l_num; ++j)
        {
            name = buf;

            while(*(buf++)) ;

            *ext_c_funcs_it = tvm_map_get(lib->c_funcs_map, name);
            ++ext_c_funcs_it;
        }

        //read the number of functions to import from the lib
        l_num = tvm_ushort_from_bytes(buf);

        for(j = 0; j < l_num; ++j)
        {
            name = buf;

            while(*(buf++)) ;

            *ext_funcs_it = tvm_map_get(lib->funcs_map, name);
            ++ext_funcs_it;
        }

    }

}

tvm_module_t tvm_module_create
    (tvm_program_t program, unsigned char *bytecode, unsigned char *bytecode_end)
{
    tvm_module_t module = jit_malloc(sizeof(struct _tvm_module));

    module->program = program;
    module->bytecode = bytecode;
    module->bytecode_end = bytecode_end;

    return module;
}

tvm_module_t tvm_module_create_build
    (tvm_program_t program, unsigned char *bytecode, unsigned char *bytecode_end)
{
    tvm_module_t module = jit_malloc(sizeof(struct _tvm_module));

    module->program = program;
    module->bytecode = bytecode;
    module->bytecode_end = bytecode_end;

    tvm_module_build(module);

    return module;
}

void tvm_module_free
    (tvm_module_t module)
{
    jit_free(module->c_funcs);

    jit_free(module->funcs);

    int i;
    for(i = 0; i < module->structs_len; ++i)
        jit_type_free(module->structs[i].type);

    jit_free(module->structs);

    for(i = 0; i < module->globals_len; ++i)
    {
        //tells GC to free global data
        GC_free(module->globals[i].data);

        jit_type_free(module->globals[i].type);
    }

    //GC_remove_roots(module->globals, sizeof(struct _tvm_global_var) * num);

    jit_free(module->globals);

    jit_free(module->strings);

    //free arrays of lib elements

    jit_free(module->bytecode);

    tvm_map_free(module->structs_map);
    tvm_map_free(module->globals_map);
    tvm_map_free(module->c_funcs_map);
    tvm_map_free(module->funcs_map);

    jit_free(module);
}
