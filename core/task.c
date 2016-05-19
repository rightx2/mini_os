/********************************************************
 * Filename: core/task.c
 *
 * Author: parkjy, RTOSLab. SNU.
 *
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

int32u_t eos_create_task(eos_tcb_t *task,
						 addr_t sblock_start,
						 size_t sblock_size,
						 void (*entry)(void *arg),
						 void *arg,
						 int32u_t priority)
{
    PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);
    task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);
    task->status = READY;       // not currently used
    task->priority = priority;  // task's priority

    task->ready_q_node.ptr_data = task;
    task->ready_q_node.priority = 0; // priority among ready_q_node.
                                     // In ready_queue, it is zero as a Default, which means FIFO
    _os_set_ready(task->priority);

    // Add task(node) in READY queue
    _os_add_node_tail(&_os_ready_queue[task->priority], &(task->ready_q_node));
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {
    // eos_tcb_t* current_task = eos_get_current_task();
    if (_os_current_task){
        addr_t saved_sp = _os_save_context();

        /*
            _os_saved_context() returns stack_pointer, which is not NULL.
            But when all pushed values are restored to its corresponding registers in _os_restore_context,
            eax is set to zero, which is NULL
        */
        if (saved_sp == NULL){
            // when returned from _os_restore_context, this section will be executed
            return;
        }

        // Save saved_sp in tcb
        _os_current_task->sp = saved_sp;

        // Select next task
        _os_current_task = _os_current_task->ready_q_node.next->ptr_data;
    } else {
        int8u_t highest_priority = _os_get_highest_priority();
        _os_node_t *next_node = (_os_node_t *)_os_ready_queue[highest_priority];
        if (next_node == NULL) {
            return;
        }
        _os_current_task = next_node->ptr_data;
    }
    _os_restore_context(_os_current_task->sp);
}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
}

int32u_t eos_get_priority(eos_tcb_t *task) {
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
    task->period = period;
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
}

void _os_init_task() {
	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
}
