#include "tvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <jit/jit-dump.h>

int main
    (int argc, char**  argv)
{
    TVM_INIT;
    
    if(argc < 2)
        return EXIT_FAILURE;
    
    FILE* input = fopen(argv[1], "rb");
    
    fseek(input, 0, SEEK_END);
    long input_size = ftell(input);
    rewind(input);
    
    char* input_content = jit_malloc(input_size);
    
    long k;
    for(k = 0; k < input_size; ++k)
        input_content[k] = fgetc(input);
    
    tvm_program_t prog = tvm_program_create(input_content, input_content+input_size);
    
    jit_context_build_start(prog->context);
    tvm_program_build(prog);
    jit_context_build_end(prog->context);
    
    tvm_program_run(prog, argc -1, argv +1);
    
    jit_dump_function(stdout, prog->start->start, "<start>");
}
