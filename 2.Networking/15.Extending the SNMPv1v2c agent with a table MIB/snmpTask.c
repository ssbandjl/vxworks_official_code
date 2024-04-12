/******************************************************************************
 ******************************************************************************
 **** snmpTask.c Last build date: Fri Sep 12 11:58:57 1997
 **** from files:
 ****  ../mibs/rfc1155.smi, ../mibs/rfc1213.mib, snmpMib2.mib, snmpMy.mib
 **** starting from node: wrs
 ******************************************************************************
 ******************************************************************************
 */

#include <asn1conf.h>
#include <asn1.h>
#include <buffer.h>
#include <mib.h>
#include <snmpdLib.h>
#include <snmpdefs.h>
#include <snmp.h>
#include <auxfuncs.h>
#include <symLib.h>
#include <sysSymTbl.h>
#include <a_out.h>
#include "snmpTask.h"
#include <private/taskLibP.h>
#include "mibleaf.h"


/*
 * Method routines for the taskTable:
 *
 *   taskId -- read-only
 *  This is the task ID assigned by VxWorks to a task in the
 * system. A taskId of zero specifies a new task.
 *
 *   taskName -- read-write
 *  This is the name of the VxWorks task.  This value can
 * only be specified (set) at task creation.
 *
 *   taskPriority -- read-write
 * The priority of the VxWorks task.  This value can be
 * in the range from 0, the highest priority,
 * to 255, the lowest priority.
 *
 *   taskStatus -- read-write
 * This field specifies the current task status.  It
 * can be used to change the current task state.
 * For example, to suspend a task, the value of
 * taskStatus is changed to task-suspended,
 * to delete a task the value is changed to
 * task-deleted, etc...
 *
 *   taskOptions -- read-write
 * This value represents the sum of the following
 * options:
 * 
 * value      option
 *  1        VX_SUPERVISOR_MODE(read-only)
 *  2        VX_UNBREAKABLE    (break points ignored)
 *  4        VX_DEALLOC_STACK  (deallocate stack)
 *  8        VX_FP_TASK        (floating point support)
 * 16        VX_STDIO          (read-only)
 * 128        VX_PRIVATE_ENV    (private env. variables)
 * 256        VX_NO_STACK_FILL  (don't fill stack)
 * 
 * All the options above can be set at task creation time.
 * However, once the task is executing the only option
 * that can be changed is VX_UNBREAKABLE.  The option is
 * toggled based on the current setting.
 *
 *   taskMain -- read-write
 * This is the name of the entry function for the VxWorks
 * task.  This name can only be specified when a task
 * is created (entry added in the table).  The symbol
 * must exist in the VxWorks target.
 *
 *   taskStackPtr -- read-only
 * This is the saved stack pointer for the task.
 *
 *   taskStackBase -- read-only
 * This is the address of the bottom of the stack of
 * the VxWorks task.
 *
 *   taskStackPos -- read-only
 * This is the effective top of the stack in the current
 * task state.
 *
 *   taskStackEnd -- read-only
 * This is the address of the top of the stack of the
 * VxWorks task.
 *
 *   taskStackSize -- read-write
 * This is the actual size of the stack in bytes.  The
 * size of the stack can only be specified at task
 * creation (adding an entry to the table).
 *
 *   taskStackSizeUsage -- read-only
 * The number of bytes currently in use by the task from 
 * the stack.
 *
 *   taskStackMaxUsed -- read-only
 * This is the maximum number of bytes that have been used
 * by the task from the stack.
 *
 *   taskStackFree -- read-only
 * This is the number of bytes that are free currently in 
 * the task stack.
 *
 *   taskErrorStatus -- read-only
 * This is the most recent error status for this task.
 */



/* Locals */

LOCAL TASK_DESC sysTaskVars;	/* Used as an argument in taskInfoGet */
LOCAL int taskIdList[MAXTASKS];
LOCAL int numTasks;
LOCAL STRUCT_taskEntry data;	/* Used in taskInfoFill */
LOCAL int varSet = 0;		/* Used in the undo process */

/* Forward Declarations */

int taskEntCmp (const void *, const void *);
void taskInfoFill (TASK_DESC *);
void taskEntryUndo (OIDC_T, int, OIDC_T *, SNMP_PKT_T *, VB_T *);
int taskSet (STRUCT_taskEntry *);
int optionsSet (STRUCT_taskEntry *);



/* An internal routine to retrieve the values of the variables, used
 * by the method routines taskEntry_get and taskEntry_next.
 */


static int taskEntry_get_value
  (
  OIDC_T      lastmatch,
  SNMP_PKT_T *pktp,
  VB_T       *vbp,
  STRUCT_taskEntry *pData
  )     
{
  switch(lastmatch) {

  case LEAF_taskId:
    getproc_got_int32(pktp, vbp, pData->taskId);    
    break;

  case LEAF_taskName:
    getproc_got_string(pktp, vbp, strlen(pData->taskName), pData->taskName, 0, VT_STRING);
    break;

  case LEAF_taskPriority:
    getproc_got_int32(pktp, vbp, pData->taskPriority);
    break;

  case LEAF_taskStatus:
    /* Values:
     *  task-ready(1)     = VAL_taskStatus_task_ready
     *  task-suspended(2) = VAL_taskStatus_task_suspended
     *  task-delay(3)     = VAL_taskStatus_task_delay
     *  task-deleted(4)   = VAL_taskStatus_task_deleted
     */
    getproc_got_int32(pktp, vbp, pData->taskStatus);
    break;

  case LEAF_taskOptions:
    getproc_got_int32(pktp, vbp, pData->taskOptions);
    break;

  case LEAF_taskMain:
    getproc_got_string(pktp, vbp, strlen(pData->taskMain), pData->taskMain, 0, VT_STRING);
    break;

  case LEAF_taskStackPtr:
    getproc_got_uint32(pktp, vbp, pData->taskStackPtr, VT_GAUGE);
    break;

  case LEAF_taskStackBase:
    getproc_got_uint32(pktp, vbp, pData->taskStackBase, VT_GAUGE);
    break;

  case LEAF_taskStackPos:
    getproc_got_uint32(pktp, vbp, pData->taskStackPos, VT_GAUGE);
    break;

  case LEAF_taskStackEnd:
    getproc_got_uint32(pktp, vbp, pData->taskStackEnd, VT_GAUGE);
    break;

  case LEAF_taskStackSize:
    getproc_got_uint32(pktp, vbp, pData->taskStackSize, VT_GAUGE);
    break;

  case LEAF_taskStackSizeUsage:
    getproc_got_uint32(pktp, vbp, pData->taskStackSizeUsage, VT_GAUGE);
    break;

  case LEAF_taskStackMaxUsed:
    getproc_got_uint32(pktp, vbp, pData->taskStackMaxUsed, VT_GAUGE);
    break;

  case LEAF_taskStackFree:
    getproc_got_uint32(pktp, vbp, pData->taskStackFree, VT_GAUGE);
    break;

  case LEAF_taskErrorStatus:
    getproc_got_int32(pktp, vbp, pData->taskErrorStatus);
    break;

  default:
    return GEN_ERR;
  }
  return NO_ERROR;
}



/********************************************************************
 ** This function copies the entries from the TASC_DESC structure
 ** into the STRUCT_taskEntry structure.
 **
 */

void taskInfoFill 
  (
  TASK_DESC *pSysTemp
  ) 
{
  int 	     value = 0;
  SYM_TYPE   stype;

  data.taskId = pSysTemp->td_id;
  strcpy (data.taskName, pSysTemp->td_name);
  data.taskPriority = pSysTemp->td_priority;

  switch (pSysTemp->td_status) {

  case WIND_READY:
                data.taskStatus = VAL_taskStatus_task_ready;
                break;
  case WIND_SUSPEND:
                data.taskStatus = VAL_taskStatus_task_suspended;
                break;
  case WIND_PEND:
  case WIND_DELAY:
                data.taskStatus = VAL_taskStatus_task_delay;
                break; 
  case WIND_DEAD:
                data.taskStatus = VAL_taskStatus_task_deleted;
                break;
  }

  data.taskOptions = pSysTemp->td_options;

  if ((_func_symFindByValueAndType != (FUNCPTR) NULL) && (sysSymTbl != NULL))	
        (* _func_symFindByValueAndType) (sysSymTbl, pSysTemp->td_entry, data.taskMain, &value, &stype, N_EXT | N_TEXT, N_EXT | N_TEXT);

  data.taskStackPtr = (UINT_32_T) pSysTemp->td_sp;
  data.taskStackBase = (UINT_32_T) pSysTemp->td_pStackBase;
  data.taskStackPos = (UINT_32_T) pSysTemp->td_pStackLimit;
  data.taskStackEnd = (UINT_32_T) pSysTemp->td_pStackEnd;
  data.taskStackSize = pSysTemp->td_stackSize;
  data.taskStackSizeUsage = pSysTemp->td_stackCurrent;
  data.taskStackMaxUsed = pSysTemp->td_stackHigh;
  data.taskStackFree = pSysTemp->td_stackMargin;
  data.taskErrorStatus = pSysTemp->td_errorStatus;

}


/*******************************************************************
 ** This is the get method routine for the task table entries
 **
 */

void taskEntry_get
  (
  OIDC_T      lastmatch,
  int         compc,
  OIDC_T     *compl,
  SNMP_PKT_T *pktp,
  VB_T       *vbp
  )
{
  int error;
  SYM_TYPE stype;

  /* find all the varbinds that share the same getproc and instance */

  snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);

  if (compc != TASK_TBL_INDEX_LEN) { 
    for (; vbp;vbp = vbp->vb_link)
	getproc_nosuchins (pktp, vbp);
    return;
  }
  
  if (taskInfoGet (*compl, &sysTaskVars) != OK) {
	for ( ; vbp; vbp = vbp->vb_link)
		getproc_error(pktp, vbp, GEN_ERR);
	return;
  }

  /* Assign sysTaskVars values to data */

  taskInfoFill (&sysTaskVars);

    /* retrieve all the values from the same data structure */

  for ( ; vbp; vbp = vbp->vb_link) 
	if ((error = taskEntry_get_value(vbp->vb_ml.ml_last_match, pktp, vbp, &data)) != NO_ERROR)
		getproc_error(pktp, vbp, GEN_ERR);
}



/*********************************************************************
 ** This is the next method routine for the task table entries.
 **
 */


void taskEntry_next
  (
  OIDC_T      lastmatch,
  int         compc,
  OIDC_T     *compl,
  SNMP_PKT_T *pktp,
  VB_T       *vbp)
{
  int		i = 0;
  OIDC_T	taskIndex[TASK_TBL_INDEX_LEN];

  /* find all the varbinds that share the same getproc and instance */

  snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);

  /* Get the tasklist and sort it */

  numTasks = taskIdListGet (taskIdList, MAXTASKS);
  qsort (taskIdList, numTasks, sizeof (UINT_32_T), taskEntCmp);

  if (compc != 0) {

  /* See if the task id given exists */
  	for (i=0; i<numTasks; i++) {
		if (taskIdList[i] > (*compl) && taskInfoGet (taskIdList[i], &sysTaskVars) == OK) {
			taskInfoFill (&sysTaskVars);
			break;
		}
	}
  	if (i >= numTasks) {
		for (; vbp; vbp=vbp->vb_link)
			nextproc_no_next(pktp, vbp);
		return;
	}
  } 


  /* We have to fetch the first entry */
  else {
	taskInfoGet (taskIdList[0], &sysTaskVars);
	taskInfoFill (&sysTaskVars);
  }


  memset (taskIndex, 0, TASK_TBL_INDEX_LEN * sizeof (OIDC_T));
  memcpy (taskIndex, &taskIdList[i], TASK_TBL_INDEX_LEN * sizeof (OIDC_T));

    /* we found a next so get the values and set the instance for all
     * the varbinds for this row of this table.
     */

  for ( ; vbp ; vbp = vbp->vb_link) {
	nextproc_next_instance(pktp, vbp, TASK_TBL_INDEX_LEN, taskIndex);
	taskEntry_get_value(vbp->vb_ml.ml_last_match, pktp, vbp, &data);
  }
}



/****************************************************************
 ** This is the compare routine used when a qsort is 
 ** performed in the next method routine.
 **
 */


int taskEntCmp
  (
  const void * pValue1,
  const void * pValue2
  )
{
    return (*((int *)pValue1) - *((int *)pValue2));
}



/****************************************************************
 ** This is the test routine that validates all the entries in
 ** the packet prior to the set.
 **
 */

void taskEntry_test
  (
  OIDC_T      lastmatch,
  int         compc,
  OIDC_T     *compl,
  SNMP_PKT_T *pktp,
  VB_T       *vbp
  )
{
  int taskid, errorStatus = 0;
  STRUCT_taskEntry * pCheckData;
  VB_T *pGroupVbp;

  taskid = *compl;

  /* Allocate memory for vb_priv structure and setup the free routine */

  vbp->vb_priv = snmpdMemoryAlloc (sizeof (STRUCT_taskEntry));

  if (vbp->vb_priv == NULL) {
	testproc_error(pktp, vbp, RESOURCE_UNAVAILABLE);
	return;
  }

  bzero ((void *)vbp->vb_priv, sizeof (STRUCT_taskEntry));
  pCheckData = (STRUCT_taskEntry *) vbp->vb_priv;
  vbp->vb_free_priv = (VBPRIVPROC_T *) snmpVbPrivFree;

  /*  Check if instance is ok  */

  if (compc != TASK_TBL_INDEX_LEN) {
	testproc_error (pktp, vbp, NO_SUCH_NAME);
	return;
  }

  /*  Check if such an entry exists in the table  */

  pCheckData->taskId = taskid;		/* A taskId of zero should also be copied */

  if ((taskid != 0) && (taskInfoGet(taskid, &sysTaskVars) != OK)) { 
	testproc_error(pktp, vbp, NO_SUCH_NAME);
	return;
  }

  if (taskid) {
  	taskInfoFill (&sysTaskVars);
	memcpy ((char *)pCheckData, (char *)&data, sizeof (STRUCT_taskEntry));
  }

  /* find all the varbinds that share the same getproc and instance and
   * group them together. */

  snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);

  /* now check each varbind */

  for (pGroupVbp = vbp; pGroupVbp; pGroupVbp=pGroupVbp->vb_link) {

    /* Each value has to be checked */

  switch (pGroupVbp->vb_ml.ml_last_match) {

  case LEAF_taskName:
		if (taskid != 0) {
			errorStatus = NOT_WRITABLE;
			goto errorReturn;
		}
      		break;

  case LEAF_taskPriority:
		if ((VB_GET_INT32(pGroupVbp) < 0) | (VB_GET_INT32(pGroupVbp) > 255)) {
			errorStatus = WRONG_VALUE;
			goto errorReturn;
		}
      		break;

  case LEAF_taskStatus:
		if (taskid == 0) {
			errorStatus = GEN_ERR;
			goto errorReturn;
		}
      		switch (VB_GET_INT32(pGroupVbp)) {
      		case VAL_taskStatus_task_ready:
      		case VAL_taskStatus_task_suspended:
      		case VAL_taskStatus_task_delay:
      		case VAL_taskStatus_task_deleted:
        		break;
      		default:
        		errorStatus = WRONG_VALUE;
			goto errorReturn;
        		continue;
      		}
      		break;

  case LEAF_taskOptions:
	/* If a task is being created, any options can be changed, otherwise 
	   only the VX_UNBREAKABLE option can be changed */

		if ((taskid == 0) && (VB_GET_INT32(pGroupVbp) & (VX_UNBREAKABLE | VX_DEALLOC_STACK | VX_FP_TASK | VX_PRIVATE_ENV | VX_NO_STACK_FILL | VX_SUPERVISOR_MODE | VX_STDIO)) == 0) {
			errorStatus = GEN_ERR;
			goto errorReturn;
		}
		else  
			if (VB_GET_INT32(pGroupVbp) != VX_UNBREAKABLE) {
				errorStatus = WRONG_VALUE;
				goto errorReturn;
			}
      		break;

  case LEAF_taskMain:
		if (taskid != 0) {
			errorStatus = GEN_ERR;
			goto errorReturn;
		}
      		break;

  case LEAF_taskStackSize:
		if (taskid != 0) {
			errorStatus = GEN_ERR;
			goto errorReturn;
		}
      		break;

  default:
      		errorStatus = GEN_ERR;
		goto errorReturn;
      		return;
    }
  }

  for (; vbp; vbp = vbp->vb_link) 
  	testproc_good (pktp, vbp);
  return;

  errorReturn:
	testproc_error(pktp, vbp, errorStatus);

}


/**********************************************************************
 ** This is the set method routine that is invoked after all the
 ** values in the received packet have been validated for a set
 ** operation.
 **
 */

void taskEntry_set
  (
  OIDC_T      lastmatch,
  int         compc,
  OIDC_T     *compl,
  SNMP_PKT_T *pktp,
  VB_T       *vbp
  )
{
  int errorStatus = 0;
  STRUCT_taskEntry newVals;
  VB_T *pVbpSaved = vbp;

  STRUCT_taskEntry *pCheckData = vbp->vb_priv;     
  bzero ((char *) &newVals, sizeof (STRUCT_taskEntry));
  newVals.taskId = pCheckData->taskId;

  /* Obtain all the values because you would need all of them if
     a task is to be spawned
  */

  for (; vbp; vbp = vbp->vb_link) {
	switch (vbp->vb_ml.ml_last_match) {
	
	case LEAF_taskName:
			memcpy (newVals.taskName, EBufferStart(VB_GET_STRING(vbp)), EBufferUsed (VB_GET_STRING(vbp)));
			break;

	case LEAF_taskPriority:
			newVals.taskPriority = VB_GET_INT32(vbp);
			break;

	case LEAF_taskStatus:
			newVals.taskStatus = VB_GET_INT32(vbp);
			break;

	case LEAF_taskOptions:
			newVals.taskOptions = VB_GET_INT32(vbp);
			break;

	case LEAF_taskMain:
			memcpy (newVals.taskMain, EBufferStart (VB_GET_STRING(vbp)), EBufferUsed (VB_GET_STRING(vbp)));
			break;

	case LEAF_taskStackSize:
			newVals.taskStackSize = VB_GET_INT32(vbp);
			break;

	default:
			continue;
	}
  }

  /* If taskId = 0, then create a new task, but first check if the 
     task entrypoint is listed in the symbol table */

  if (newVals.taskId == 0) {
	if ((errorStatus = taskSet (&newVals)) == COMMIT_FAILED)
		goto errorReturn;

	else {
		for (vbp = pVbpSaved; vbp; vbp = vbp->vb_link) 
			setproc_good (pktp, vbp);
		return;
	}
  } 


  /* A new row entry is not required. The existing entries are
     being modified
  */

  else {
	for (vbp = pVbpSaved; vbp; vbp = vbp->vb_link) {

    		switch (vbp->vb_ml.ml_last_match) {

    		case LEAF_taskPriority:
			if (taskPrioritySet (newVals.taskId, newVals.taskPriority) == ERROR) {
				errorStatus = COMMIT_FAILED;
				goto errorReturn;
			}
			varSet |= TSK_PRIOR; 
      			break;

    		case LEAF_taskStatus:
			if ((newVals.taskStatus == VAL_taskStatus_task_suspended) && (taskSuspend (newVals.taskId) != ERROR)) {
				varSet |= TSK_STAT_SUS;
				break;
			}
			if ((newVals.taskStatus == VAL_taskStatus_task_ready) && taskResume(newVals.taskId) != ERROR) {
				varSet |= TSK_STAT_RDY;
				break;
			}
			if ((newVals.taskStatus == VAL_taskStatus_task_deleted) && taskDelete(newVals.taskId) != ERROR) { 
				varSet |= TSK_STAT_DEL;
				break;
			}
			errorStatus = COMMIT_FAILED;
			goto errorReturn;
      			break;

    		case LEAF_taskOptions:
			if ((errorStatus = optionsSet (&newVals)) == COMMIT_FAILED) {
				goto errorReturn;
			}
			varSet |= TSK_OPTN;
      			break;

    		default:
			errorStatus = COMMIT_FAILED;
			goto errorReturn;
      			return;
    		}
  	}

	for (vbp = pVbpSaved; vbp; vbp = vbp->vb_link)
		setproc_good (pktp, vbp);
	return;


	errorReturn:
		pVbpSaved->undoproc = (UNDOPROC_T *) taskEntryUndo;
		snmpVbPrivFree (pVbpSaved);
		setproc_error (pktp, pVbpSaved, errorStatus);
  }
}



/*******************************************************************
 ** This routine is called when a set fails. All the previous sets
 ** would have to be cancelled. If a set has resulted in an existing
 ** task's deletion, that task is not restarted in this routine.
 **
 */

void taskEntryUndo 
    (
    OIDC_T              lastmatch,
    int                 compc,
    OIDC_T *            compl,
    SNMP_PKT_T *        pktp,
    VB_T *              vbp
    )
{
  int errorStatus = 0;
  STRUCT_taskEntry * pCheckData = vbp->vb_priv;

	if (varSet & TSK_PRIOR) {
		if (taskPrioritySet (pCheckData->taskId, pCheckData->taskPriority) == ERROR)
			errorStatus = UNDO_FAILED;
	}  

	if ((varSet & TSK_STAT_RDY) && (taskSuspend (pCheckData->taskId) == ERROR))  {
		errorStatus = UNDO_FAILED;
	}
	if ((varSet & TSK_STAT_SUS) && (taskResume (pCheckData->taskId) == ERROR))  {
		errorStatus = UNDO_FAILED;
	}

	if ((varSet & TSK_OPTN) && (optionsSet (pCheckData))) {
		errorStatus = UNDO_FAILED;
	}

 	if (errorStatus) { 
       		undoproc_error (pktp, vbp, UNDO_FAILED);
       		return;
	}

    	else 
		goto goodReturn;

    goodReturn:
   	undoproc_good (pktp, vbp);
    	return;
}



/***********************************************************************
 ** This is invoked from the set method routine when a new task is
 ** to be spawned. A new table entry is created as a result of this
 **
 */

int taskSet
    (
    STRUCT_taskEntry * pData
    )
{
    int errorStatus = 0;
    SYM_TYPE stype;
    char * tEntryRoutine;
    char tstring[DISPLAY_STR_SZ];

    strcat (tstring, pData->taskMain);
    if (symFindByCName (sysSymTbl, tstring, &tEntryRoutine, &stype) == ERROR) {
	errorStatus = NO_SUCH_NAME;
	return errorStatus;
    }

    strcpy (tstring, pData->taskName);
    if (taskSpawn (tstring, pData->taskPriority, pData->taskOptions, pData->taskStackSize, (FUNCPTR) tEntryRoutine, 0,0,0,0,0,0,0,0,0,0) == ERROR) {
                errorStatus = COMMIT_FAILED;
		return errorStatus;
    }
    return errorStatus;
}
      


/**********************************************************************
 ** This is called when the options for an existing task has to be 
 ** changed. This is also called from the undo routine.
 */


int optionsSet
    (
    STRUCT_taskEntry * pData
    )
{
    int toptions;
    int errorStatus = 0;

    if (taskOptionsGet (pData->taskId, &toptions) == ERROR) {
	errorStatus = COMMIT_FAILED;
	return errorStatus;
    }
    if ((toptions & VX_UNBREAKABLE) == 0) {
	if (taskOptionsSet (pData->taskId, ~toptions, (toptions | VX_UNBREAKABLE)) == ERROR) {
		errorStatus = COMMIT_FAILED;
		return errorStatus;
        }
    }
    else {
	toptions &= ~VX_UNBREAKABLE;
		if (taskOptionsSet (pData->taskId, VX_UNBREAKABLE, toptions) == ERROR) {
			errorStatus = COMMIT_FAILED;
			return errorStatus;
                }
    }
    return errorStatus;
}





