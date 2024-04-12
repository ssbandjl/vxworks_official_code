/* snmpTask.h */

#include <taskLib.h>


#define TASK_TBL_INDEX_LEN      1       /* Length of the index */
#define MAXTASKS                500     /* Max # of tasks in the system */
#define DISPLAY_STR_SZ          255     /* Max Length for display string */
#define TSK_PRIOR		1	/* Flag for undo routine */
#define TSK_STAT_SUS		2	/* Flag for undo routine */
#define TSK_STAT_DEL		4
#define TSK_STAT_RDY		8
#define TSK_OPTN		16	/* Flag for undo routine */

typedef struct {
        INT_32_T        taskId;
        char            taskName[DISPLAY_STR_SZ];
        INT_32_T        taskPriority;
        INT_32_T        taskStatus;
        INT_32_T        taskOptions;
        char            taskMain[DISPLAY_STR_SZ];
        UINT_32_T       taskStackPtr;
        UINT_32_T       taskStackBase;
        UINT_32_T       taskStackPos;
        UINT_32_T       taskStackEnd;
        UINT_32_T       taskStackSize;
        UINT_32_T       taskStackSizeUsage;
        UINT_32_T       taskStackMaxUsed;
        UINT_32_T       taskStackFree;
        INT_32_T        taskErrorStatus;
} STRUCT_taskEntry;


