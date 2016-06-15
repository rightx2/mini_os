/********************************************************
 * Filename: core/sync.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 *
 * Description: semaphore, condition variable management.
 ********************************************************/
#include <core/eos.h>

void eos_init_semaphore(eos_semaphore_t *sem, int32u_t initial_count, int8u_t queue_type) {
    /* initialization */
    sem->num_of_resource = initial_count;
    sem->wait_queue = NULL;
    sem->queue_type = queue_type;
}

int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout) {
    /* Write just simple semaphore mechanism without waiting(loop) mechanism*/
    int first_sleep = 1;
    while(1) {
        if (sem->num_of_resource > 0) {
            sem->num_of_resource--;
            return 1;
        } else {
            if (timeout < 0) {
                return -1;
            } else if (timeout == 0) {
                _os_wait(&(sem->wait_queue), sem->queue_type);
                eos_schedule();
                continue;
            } else {
                if (!first_sleep) {
                    _os_wait(&(sem->wait_queue), sem->queue_type);
                    eos_schedule();
                    continue;
                }
                first_sleep = 0;
                eos_sleep(timeout);
            }
        }
    }
}

void eos_release_semaphore(eos_semaphore_t *sem) {
    /* Write just simple semaphore mechanism without waiting(loop) mechanism*/
    sem->num_of_resource++;
    if (sem->wait_queue) {
        _os_wakeup_single(&(sem->wait_queue), sem->queue_type);
    }
    eos_schedule();
}

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type) {
    /* initialization */
    cond->wait_queue = NULL;
    cond->queue_type = queue_type;
}

void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex) {
    /* release acquired semaphore */
    eos_release_semaphore(mutex);
    /* wait on condition's wait_queue */
    // _os_wait(&cond->wait_queue);
    /* acquire semaphore before return */
    eos_acquire_semaphore(mutex, 0);
}

void eos_notify_condition(eos_condition_t *cond) {
    /* select a task that is waiting on this wait_queue */
    _os_wakeup_single(&cond->wait_queue, cond->queue_type);
}
