#include <core/eos.h>
#include <core/eos_internal.h>

typedef struct _os_context {
    /* low address */
    addr_t edi_t;
    addr_t esi_t;
    addr_t ebp_t;
    addr_t esp_t;
    addr_t ebx_t;
    addr_t edx_t;
    addr_t ecx_t;
    addr_t eax_t;
    addr_t eflags;
    addr_t eip;
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
    // 4bytes
    // addr_t *sp = (addr_t *)(stack_base + stack_size);
    // printf("%p\n",sp--);
    // printf("%p\n",sp--);
    // printf("%p\n",(int8u_t*)stack_base);

    addr_t sp = stack_base + stack_size;
    sp -= sizeof(int32u_t);

    *(int32u_t*)sp = entry;           // eip
    sp -= sizeof(int32u_t);

    *(int32u_t*)sp = NULL;            // eflags
    sp -= sizeof(int32u_t);

    *(int32u_t*)sp = NULL;            // eax
    sp -= sizeof(int32u_t);

    *(int32u_t*)sp = NULL;            // ecx
    sp -= sizeof(int32u_t);

    *(int32u_t*)sp = NULL;            // edx
    sp -= sizeof(int32u_t);

    *(int32u_t*)sp = NULL;            // ebx
    sp -= sizeof(int32u_t);

    // *(int32u_t*)sp = NULL;            // esp --> 굳이 저장할 필요없긴함
    // sp -= sizeof(int32u_t);

    *(int32u_t*)sp = NULL;            // ebp
    sp -= sizeof(int32u_t);

    *(int32u_t*)sp = NULL;            // esi
    sp -= sizeof(int32u_t);

    *(int32u_t*)sp = NULL;            // edi

    return sp;
}

void _os_restore_context(addr_t sp) {
    /*
        Pop pushed register values to corresponding registers and restart from where it stopped
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
        popl _eflags;\
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
        Push resume point which will resume from stop point of the task when context is restored
        Push eflags and registers
    */
    __asm__ __volatile__ ("\
        push $resume_point;\
        push _eflags;\
        push %%eax;\
        push %%ecx;\
        push %%edx;\
        push %%ebx;\
        push %%ebp;\
        push %%esi;\
        push %%edi;"
        :                 /*no output*/
        :                 /*input*/
    );

    /*
        Store currnet stack pointer in %esp for return value
    */
    __asm__ __volatile__("movl %%esp, %%eax;"::);


    /*
        Push old %ebp and %eip then store %esp in %ebp so that it make
        current stack looked like starting new function.
        (Because every time funcion stack is created, below procedures should be done)
            1. push return address & old ebp
            2. movl %%esp, %%ebp
        By doing so, it protects all pushed register values and return safely to eos_schedule()
    */
    __asm__ __volatile__("\
        push 4(%%ebp);\
        push 0(%%ebp);\
        movl %%esp, %%ebp;"
        :                 /*no output*/
        :                 /*input*/
    );

    /*
        Define resume_point so that restored task restarts from where it stopped
    */
    __asm__ __volatile__("\
        resume_point:\
            leave;\
            ret;"
        :                 /*no output*/
        :                 /*input*/
    );

}
