#include <core/eos.h>

static eos_tcb_t tcb1;
static eos_tcb_t tcb2;
static int8u_t stack1[8096];
static int8u_t stack2[8096];

/* task1 function -print number 1 to 20 repeatedly */
static void print_number(void *arg) {
	int i= 0;
    while(++i) {
        printf("%d", i);
        eos_schedule();// 태스크1 수행중단, 태스크2 수행재개
        if (i== 20) {
            i= 0;
        }
    }
}

static void print_alphabet(void *arg) {
	int i= 96;
    while(++i) {
        printf("%c", i);
        eos_schedule(); // 태스크2 수행중단, 태스크1 수행재개
        if (i== 122) {
            i= 96;
        }
    }
}

void eos_user_main() {
	eos_create_task(&tcb1, (addr_t)stack1, 8096, print_number, NULL, 0);
	eos_create_task(&tcb2, (addr_t)stack2, 8096, print_alphabet, NULL, 0);
}
