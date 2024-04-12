/* mutexSemDemo.h - Header for the mutexSemDemo */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,06nov97,mm  added copyright.    
01a,14jan94,ms   written.
*/

#define  CONSUMER_TASK_PRI           98  /* Priority of the consumerTask task*/
#define  PRODUCER_TASK_PRI           99  /* Priority of the producerTask task*/
#define  TASK_STACK_SIZE          5000   /* Stack size for spawned tasks */
#define PRODUCED  1                      /* Flag to indicate produced status*/ 
#define CONSUMED  0                      /* Flag to indicate consumed status*/
#define NUM_ITEMS 5                      /* Number of items */
struct shMem                             /* Shared Memory data structure */
    {
    int tid;                             /* task id */  
    int count;                           /* count  number of item produced */
    int status;                          /* 0 if consumed or 1 if produced*/
    };

