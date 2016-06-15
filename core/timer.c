/********************************************************
 * Filename: core/timer.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 *
 * Description:
 ********************************************************/
#include <core/eos.h>

static eos_counter_t system_timer;

int8u_t eos_init_counter(eos_counter_t *counter, int32u_t init_value) {
	counter->tick = init_value;
	counter->alarm_queue = NULL;
	return 0;
}

/***
	Initialize 'alarm' and 'alarm_node' in the task
	and add it in alarm_queue of counter in priority order.
***/
void eos_set_alarm(eos_counter_t* counter,
				   eos_alarm_t* alarm,
				   int32u_t timeout,
				   void (*entry)(void *arg),
				   void *arg)
{
	if ((timeout == 0) || (entry == NULL)) {
		return;
	}

	alarm->handler = entry;
	alarm->arg = arg;
	alarm->timeout = timeout;

	alarm->alarm_queue_node.ptr_data = alarm;
	alarm->alarm_queue_node.priority = timeout; // If timeout is small, then higher priority

	// After setting alarm, locate alarm_node in counter's alarm_queue
	_os_add_node_priority(&(counter->alarm_queue), &(alarm->alarm_queue_node));
}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

/***
	Everytime the timer interrupt occurs (1sec),
	look for tasks which should be woken up and call handler of them.
	Then, remove that node from alarm_queue.
***/
void eos_trigger_counter(eos_counter_t* counter) {
	counter->tick += 1;
	PRINT("current time : %d\n", counter->tick);

	// If counter's alarm_queue is empty, nothing to do.
	if (counter->alarm_queue == NULL) {
		return;
	}
	// Define 'checker' for checking whether time-to-wake-up task exists
	_os_node_t *checker = counter->alarm_queue;

	do {
		eos_alarm_t *task_alarm = (eos_alarm_t *)(checker->ptr_data);

		// If it's time to wake up
		if (task_alarm->timeout <= counter->tick) {
			// new variable 'node_remover' for removing that node from alarm_queue
			_os_node_t* node_remover = checker;
			checker = checker->next;

			// Remove that node from alarm queue.
			_os_remove_node(&counter->alarm_queue, node_remover);
			// After removed from alarm_queue, call handler
			task_alarm->handler(task_alarm->arg);
		} else {
			break;
		}
	} while (counter->alarm_queue != NULL);

    eos_schedule();
}

/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg) {
	/* trigger alarms */
	eos_trigger_counter(&system_timer);
}

void _os_init_timer() {
	eos_init_counter(&system_timer, 0);

	/* register timer interrupt handler */
	eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
}
