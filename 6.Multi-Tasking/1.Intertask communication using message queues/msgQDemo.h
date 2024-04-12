/* msgQDemo.h - Header for the msgQDemo */

/* Copyright 1984-1997 Wind River Systems, Inc. */
 
/*
modification history
--------------------
01b,06nov97,mm  added copyright.
01a,14dec93,ms   written.
*/

#define  CONSUMER_TASK_PRI          99   /* Priority of the consumer task */
#define  PRODUCER_TASK_PRI          98   /* Priority of the producer task */
#define  TASK_STACK_SIZE          5000   /* stack size for spawned tasks */

struct msg {                             /* data structure for msg passing */
        int tid;                         /* task id */         
        int value;                       /* msg value */
        };

LOCAL MSG_Q_ID msgQId;                   /* message queue id */
LOCAL int numMsg = 8;                    /* number of messages */
LOCAL BOOL notDone;                      /* Flag to indicate the completion 
                                            of this demo */
