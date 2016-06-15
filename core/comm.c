/********************************************************
 * Filename: core/comm.c
 *
 * Author: jtlim, RTOSLab. SNU.
 *
 * Description: message queue management.
 ********************************************************/
#include <core/eos.h>
#include <string.h>

/* Initialize message queue and its semaphores with given parameters */
void eos_init_mqueue(eos_mqueue_t *mq,
                     void *queue_start,
                     int16u_t queue_size,
                     int8u_t msg_size,
                     int8u_t queue_type)
{
    mq->queue_size = queue_size;
    mq->msg_size = msg_size;
    mq->queue_start = queue_start;
    mq->front = queue_start;
    mq->rear = queue_start;
    mq->queue_type = queue_type;

    /* Initialize semaphore */
    eos_init_semaphore(&(mq->getsem), 0, queue_type);   // Size of getsem start as 0
    eos_init_semaphore(&(mq->putsem), queue_size, queue_type); // Size of getsem start as 5
}


int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
    /* Before sending message, it should get semaphore first */
    if(eos_acquire_semaphore(&mq->putsem, timeout)) {
        // Copy messages and move pointer 'rear'
        memcpy(mq->rear, message, mq->msg_size);
        mq->rear = mq->rear + mq->msg_size;

        // Circular queue mechanism
        void *queue_end = mq->queue_start + (mq->queue_size)*(mq->msg_size);
        if (mq->rear + mq->msg_size > queue_end) {
            mq->rear = mq->queue_start;
        }
        // Release semaphore
        // "I complete putting messages in this space,
        //   so you(receiver) are allowed to read message in this space"
        eos_release_semaphore(&(mq->getsem));
        return 1;
    } else {
        return 0;
    }
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
    /* Before receiving message, it should get semaphore first */
    if(eos_acquire_semaphore(&mq->getsem, timeout)) {
        // Copy messages and move pointer 'first'
        memcpy(message, mq->front, mq->msg_size);
        mq->front = mq->front + mq->msg_size;

        // Circular queue mechanism
        void *queue_end = mq->queue_start + (mq->queue_size)*(mq->msg_size);
        if (mq->front + mq->msg_size > queue_end) {
            mq->front = mq->queue_start;
        }
        // Release semaphore
        // "I complete reading messages from this space,
        //   so you(sender) are allowed to put message in this space"
        eos_release_semaphore(&(mq->putsem));
        return 1;
    } else {
        return 0;
    }
}
