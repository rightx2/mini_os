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
 * Pointer to TCB of running task.
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
    task->status = READY;       // Initialize state of newly created task as READY
    task->priority = priority;  // Task's priority

    task->ready_q_node.ptr_data = task;
    task->ready_q_node.priority = 0; // Priority among nodes in specific task's priority.
                                     // In ready_queue, it is zero as a default, which means FIFO
    // Set prioirity bit
    _os_set_ready(task->priority);

    // Add task(node) in READY queue
    _os_add_node_tail(&_os_ready_queue[task->priority], &(task->ready_q_node));
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}



/***
    Getting highest priority task (which is READY state) from the READY_Queue
    and start that task.
    IPORTANT : In this function, all task should be ensured in READY state, 100%!!
    If task were in other state, it couldn't be in the READY_queue.
    (Just think about why the name of this queue is "READY"queue)
    So, transistion from READY to other state occurs other fuction,
    and if that happens, the task should be removed from READY_queue in that function, too,
    so that eos_schedule() doesn't have to care about all task's other state in 'Ready' Queue.
    This is sort of abstraction.
***/
void eos_schedule() {
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
    }
    // Select next highest-priority task
    int8u_t highest_task_priority = _os_get_highest_priority();
    _os_current_task = (eos_tcb_t *)(_os_ready_queue[highest_task_priority]->ptr_data);
    if (_os_current_task == NULL) {
        return;
    }

    _os_current_task->status = RUNNING;
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

/***
    Change state of task from RUNNING to WAITING and set alarm
    to be woken after given time. In order to do that, locate
    alarm_node in the alarm queue of counter.
***/
void eos_sleep(int32u_t tick) {
    eos_counter_t *system_timer = eos_get_system_timer();
    eos_alarm_t *cur_task_alarm = &_os_current_task->alarm;

    int32u_t timeout = 0;
    if (tick != 0){
        timeout = (system_timer->tick) + tick;   // period is set to be 'tick'
    } else {
        timeout = (system_timer->tick) + (_os_current_task->period);
    }

    // Change status from RUNNING to WAITING
    _os_current_task->status = WAITING;

    // Set alarm (At the same time, counter의 alarm_queue에 들어간다)
    eos_set_alarm(system_timer, cur_task_alarm, timeout, _os_wakeup_sleeping_task, _os_current_task);

    // Remove the task from the READY queue.
    // 'Removing node from ready queue' also could happen inside of 'eos_set_alarm()'
    // but, one function should do only one task.( = 'eos_set_alarm' should do things only about 'alarm', not about ready_queue)
    _os_remove_node(&_os_ready_queue[_os_current_task->priority], &_os_current_task->ready_q_node);
     // If not even one node left, turn off the priority bit.
    if (_os_ready_queue[_os_current_task->priority] == NULL) {
        _os_unset_ready(_os_current_task->priority);
    }
    eos_schedule();
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

void _os_wait(_os_node_t **wait_queue, int32u_t queue_type) {
    _os_current_task->status = WAITING;
    _os_remove_node(&_os_ready_queue[_os_current_task->priority], &_os_current_task->ready_q_node);
     // If not even one node left, turn off the priority bit.
    if (_os_ready_queue[_os_current_task->priority] == NULL) {
        _os_unset_ready(_os_current_task->priority);
    }

    if (queue_type == FIFO) {
        _os_add_node_tail(wait_queue, &(_os_current_task->ready_q_node));
    } else if (queue_type == PRIORITY) {
        _os_add_node_priority(wait_queue, &(_os_current_task->ready_q_node));
    }
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
    // Remove from semaphore-wait queue
    eos_tcb_t *woken_task = (*wait_queue)->ptr_data;
    _os_remove_node(wait_queue, *wait_queue);

    // set task's status as READY
    woken_task->status = READY;

    // After woken, the task should be added in READY queue
    _os_add_node_tail(&_os_ready_queue[woken_task->priority], &(woken_task->ready_q_node));

    // Set bitmap of task's priority
    _os_set_ready(woken_task->priority);
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}


/**
    After woken up, change state from WAITING to READY and locate task in READY_queue.
**/
void _os_wakeup_sleeping_task(void *arg) {
    eos_tcb_t *woken_task = (eos_tcb_t *)arg;

    // State should be set to be READY after woken.
    woken_task->status = READY;

    // After woken, the task should be added in READY queue
    _os_add_node_tail(&_os_ready_queue[woken_task->priority], &(woken_task->ready_q_node));

    // Set bitmap of task's priority
    _os_set_ready(woken_task->priority);
}

