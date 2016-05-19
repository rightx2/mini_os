#include <core/eos.h>
#include <core/eos_internal.h>

typedef struct _os_context {
    /* low address */
    addr_t edi_t;
    addr_t esi_t;
    addr_t ebp_t;
    addr_t ebx_t;
    addr_t edx_t;
    addr_t ecx_t;
    addr_t eax_t;
    addr_t eflags;
    addr_t eip_t;
    /* high address */
} _os_context_t;

void print_context(addr_t context) {
    if(context == NULL) return;
    _os_context_t *ctx = (_os_context_t *)context;
    //PRINT("reg1  =0x%x\n", ctx->reg1);
    //PRINT("reg2  =0x%x\n", ctx->reg2);
    //PRINT("reg3  =0x%x\n", ctx->reg3);
    //...
}

addr_t _os_create_context(addr_t stack_base,
                          size_t stack_size,
                          void (*entry)(void *),
                          void *arg)
{
    /*
        Initialize default context's register values
    */
    _os_context_t context;
    context.eip_t = entry;
    context.eflags = NULL;
    context.eax_t = NULL;
    context.ecx_t = NULL;
    context.edx_t = NULL;
    context.ebx_t = NULL;
    context.ebp_t = NULL;
    context.esi_t = NULL;
    context.edi_t = NULL;

    /*
        Go to top of the stack and move stack pointer
    */
    addr_t sp = stack_base + stack_size;
    sp -= sizeof(_os_context_t);

    /*
        Push context register values
    */
    *(_os_context_t*)sp = context;

    return sp;
}

void _os_restore_context(addr_t sp) {
    /*
        Store sp(argument) into esp and pop pushed register values to corresponding registers and restart from where it stopped
    */
    __asm__ __volatile__ ("\
        movl %0, %%esp; \
        popl %%edi;\
        popl %%esi;\
        popl %%ebp;\
        popl %%ebx;\
        popl %%edx;\
        popl %%ecx;\
        popl %%eax;\
        popfl;\
        ret"
        :                  /*no output*/
        :"m"(sp)          /*input*/
    );
}

addr_t _os_save_context() {
    /*
        Store zero in eax before push it
    */
    __asm__ __volatile__("movl %0, %%eax;"::"n"(0));

    /*
        Push 'resume_point' which will resume from stop point of the task when context is restored.
        And then, push eflags and registers
    */
    __asm__ __volatile__ ("\
        push $resume_point;\
        pushfl;\
        push %%eax;\
        push %%ecx;\
        push %%edx;\
        push %%ebx;\
        push %%ebp;\
        push %%esi;\
        push %%edi;"
        :                 /*no output*/
        :                 /*no input*/
    );

    /*
        Store current stack pointer in %esp for returning
    */
    __asm__ __volatile__("movl %%esp, %%eax;"::);


    /*
        Push old %ebp and %eip then store value of %esp in %ebp so looks like starting new function.
        (Because everytime funcion stack is created, below procedures should be done)
            1. push return address & old ebp
            2. movl %%esp, %%ebp
        By doing so, it protects all pushed register values and return safely to eos_schedule()
    */
    __asm__ __volatile__("\
        push 4(%%ebp);\
        push 0(%%ebp);\
        movl %%esp, %%ebp;"
        :                 /*no output*/
        :                 /*no input*/
    );

    /*
        Define 'resume_point ' so that restored task restarts from where it stopped
    */
    __asm__ __volatile__("\
        resume_point:\
            leave;\
            ret;"
        :                 /*no output*/
        :                 /*no input*/
    );

}
