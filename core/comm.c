/********************************************************
 * Filename: core/comm.c
 *
 * Author: jtlim, RTOSLab. SNU.
 *
 * Description: message queue management.
 ********************************************************/
#include <core/eos.h>

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

    eos_init_semaphore(&(mq->getsem), 0, queue_type);
    eos_init_semaphore(&(mq->putsem), queue_size, queue_type);
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
    PRINT("***Sender is acquiring putsem(Sem for put)\n");
    if(eos_acquire_semaphore(&mq->putsem, timeout)) {
        PRINT("Got semaphore!\n");
        PRINT("Putting message.......\n");
        memcpy(mq->rear, message, mq->msg_size);
        mq->rear = mq->rear + mq->msg_size;
        void *queue_end = mq->queue_start + (mq->queue_size)*(mq->msg_size);
        if (mq->rear + mq->msg_size > queue_end) {
            mq->rear = mq->queue_start;
        }
        // "I complete putting messages in this space,
        //   so you(receiver) are allowed to read message in this space"
        PRINT("Putting message done!!\n");
        PRINT("***Sender is increasing getsem(Sem for read)\n");
        eos_release_semaphore(&(mq->getsem));

        /* 이 printf구문은 timeout 구현전 단순히 semaphore가 제대로 작동하는지 확인하기 위한 비안전한 디버깅구문임
           왜냐하면 release를 했는데, 해당 세마포어 내부 변수에 다시 접근하는 구문인데, 도중에 갑자기 context switching 일어나면 race condition발생 */
        printf("send_message() : <new msg, empty space> =  <%d, %d>\n", mq->getsem.num_of_resource, mq->queue_size - mq->getsem.num_of_resource);
    }
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
    PRINT("***Receiver is acquiring getsem(Sem for read)\n");
    if(eos_acquire_semaphore(&mq->getsem, timeout)) {
        PRINT("Reading message!!\n");
        memcpy(message, mq->front, mq->msg_size);
        mq->front = mq->front + mq->msg_size;
        void *queue_end = mq->queue_start + (mq->queue_size)*(mq->msg_size);
        if (mq->front + mq->msg_size > queue_end) {
            mq->front = mq->queue_start;
        }
        // "I complete reading messages from this space,
        //   so you(sender) are allowed to put message in this space"
        PRINT("Reading message done!!\n");
        PRINT("***Receiver is increasing putsem(Sem for put)\n");
        eos_release_semaphore(&(mq->putsem));

        /* 이 printf구문은 timeout 구현전 단순히 semaphore가 제대로 작동하는지 확인하기 위한 비안전한 디버깅구문임
           왜냐하면 release를 했는데, 해당 세마포어 내부 변수에 다시 접근하는 구문인데, 도중에 갑자기 context switching 일어나면 race condition발생 */
        printf("receive_message() : <new msg, empty space> =  <%d, %d>\n", mq->getsem.num_of_resource, mq->putsem.num_of_resource);
    }
}
