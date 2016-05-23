#include <core/eos.h>
#include <unistd.h>

int32u_t stack1[8096]; // stack for task1
int32u_t stack2[8096]; // stack for task2
int32u_t stack3[8096]; // stack for task3

eos_tcb_t tcb1; // tcbfor task1
eos_tcb_t tcb2; // tcbfor task2
eos_tcb_t tcb3; // tcbfor task3

void task1() {
    while(1) {
        PRINT("A\n");
        eos_sleep(0);
    }// ‘A’ 출력 후 다음 주기까지 기다림
}
void task2() {
    while(1) {
        PRINT("B\n");
        eos_sleep(0);
    }
    // ‘B’ 출력 후 다음 주기까지 기다림
}
void task3() {
    while(1) {
        PRINT("C\n");
        eos_sleep(0);
    }
}// ‘C’ 출력 후 다음 주기까지 기다림

void eos_user_main() {
    eos_create_task(&tcb1, stack1, 8096, task1, NULL, 1);// 태스크1 생성
    eos_set_period(&tcb1, 2);

    eos_create_task(&tcb2, stack2, 8096, task2, NULL, 10);// 태스크2 생성
    eos_set_period(&tcb2, 4);

    eos_create_task(&tcb3, stack3, 8096, task3, NULL, 50);// 태스크3 생성
    eos_set_period(&tcb3, 8);
}
