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

    // 똑같다.
    // PRINT("reg1  =0x%x\n", stack_base);
    // PRINT("reg1  =0x%p\n", stack_base);

    // 4 bytes chois
    // printf("stack_base : %p\n", stack_base);
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
    // printf("%p\n",sp1);
    // printf("%p\n",sp1-sizeof(int32u_t));
    // printf("%p\n",sp1--);
    // printf("%p\n",sp1--);

    // sp--;
    // printf("%p\n",sp);
    // printf("\n",sp);
}

void _os_restore_context(addr_t sp) {
}

addr_t _os_save_context() {
}
