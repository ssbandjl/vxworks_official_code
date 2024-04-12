/* v_vxdemo.c - VxWorks Demo MIB interface to SNMP Agent */

/* Copyright 1984-1993 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
modification history
--------------------
01a,12oct93,jag  written
*/


/*
DESCRIPTION

This module contains the necessary routines to interface the SNMP agent with
the WRS Demo MIB extension definition.  The MIB is defined in concise MIB 
format in the file vxdemo.my.  This file is the input to the MIB compiler 
which generates the following:  The data structures that need to be added
to the SNMP agent to support the new MIB extension; The necessary header files; 
The functions headers (or stubs optional) that need be written to support the
MIB extension.

The software that needs to be written to support the MIB extension is divided
in two layers.  The first layer is called "v_" and the second layer is called
"k_".  The corresponding layers in the vxWorks demo MIB are v_vxdemo.c and 
k_vxdemo.c.  The functionality of each layer is described in the corresponding
module.

The SNMP agent is responsible for the communication protocol, proper 
encapsulation of the MIB variables, validation of OIDs and the invokation of
the "v_" routines.  The "v_" layer validates the value and operation of the
MIB variable and calls the respective "k_" routine.  The "k_" routine does the
actual operation on the variable.

The groups in the vxWorks Demo MIB are sysconfig, systasks, sysmemory and 
sysnfs.  Each have a set of "v_" and "k_" routines that support the group.  
On a per group and tables within a group the following "v_" routines need to 
be provided.

groupID_get (): 
Validate the request and retrive the requested variable from the k_ routine.

groupID_test():
Validate the request and the value to which a variable or table entry is to be
set.  This routine dynamically allocates a structure used to save variables
values.  After all the related variables have been collected this routine sets
the state of the allocated structure to ADD_MODIFY or DELETE. The state of this
structure informs the agent that the groupID_set() routine should be invoked.

groupID_cleanup():
This routine is invoked after the goupID_set routine to free any resources
that have allocated or locked in groupID_test.

groupID_undo():
This routine stub is only provided for compatability in this release.

groupID_set():
This routine is invoked to set the MIB variable(s).  The "k_" set routine
is invoked by this routine.
*/

/* includes */

#include <vxWorks.h>
#include <taskLib.h>

#include <stdio.h>
#include <string.h>
#include "snmpd.h"
#include "snmp.h"
#include "diag.h"


/*******************************************************************************
*
* sysconfig_get -  Read the value of a variable in the sysconfig group.
*
* This routine returns the value of the requested MIB variable that corresponds
* to its group.   All the values from the group are obtain from the "k_"
* logic but only the requested value is returned by this function.
*
* RETURNS: A pointer to a VarBind struct, or NULL if the request is invalid.
*
* SEE ALSO:
*/
VarBind * sysconfig_get 
    (
    OID         * incoming,     /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo  * object,       /* Ptr to MIB var in the SNMP MIB tree */
    int           searchType,   /* EXACT = GetRequest,NEXT = Get[Next|Bulk] */
    ContextInfo * contextInfo,  /* Reserved for future used */
    int           serialNum     /* Used for caching */
    )
    {
    static sysconfig_t  
		 * sysVars;     /* Ptr to struct returned by the k_ routine */
    static int     lastSerialNum;
    void         * dp;          /* Generic ptr used for returned variable */
    int            reqVar = -1; /* Variable Requested by the Agent */
    int            instLength;  /* Length of the incoming OID */
    static OID   * last_incoming;

    if ((lastSerialNum != serialNum) || (sysVars == NULL) ||
	(CmpOIDInst(incoming, last_incoming, object->oid.length) != 0))
	{
    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.
     */

	instLength = incoming->length - object->oid.length;

	switch (searchType) 
	    {
	    case EXACT:
		if (instLength == 1 && 
		    incoming->oid_ptr [incoming->length - 1] == 0) 
		{
		reqVar = object->nominator;
		}
		break;

	    case NEXT:
		if (instLength <= 0) 
		{
		reqVar = object->nominator;
		}
		break;

	    default:
		DPRINTF ((0, "snmpd: Internal error. (invalid search type in \
			  sysconfig_get -- %d)\n", searchType));
	    }				/* switch */

	/* Retrieve the data from the kernel-specific routine. */

	if (reqVar == -1 || 
	    ((sysVars = k_sysconfig_get (serialNum, contextInfo, 
					 reqVar)) == NULL))
	    {
	    return ((VarBind *) NULL);
	    }

	/* Get new catche information */

     	lastSerialNum = serialNum;

	if (last_incoming != NULL)
	    FreeOID(last_incoming);
		     
	last_incoming = CloneOID(incoming);
	}
    else
	{
	reqVar = object->nominator;
	}

    /*
     * Set up the variable requested by the agent.
     */

    switch (reqVar) 
	{
	case I_sysState:
	    dp = (void *) &sysVars->sysState;
	    break;

	case I_sysUserName:
	    dp = (void *) MakeOctetString (sysVars->sysUserName->octet_ptr,
					    sysVars->sysUserName->length);
	    break;

	case I_sysUserPassw:
	    dp = (void *) MakeOctetString (sysVars->sysUserPassw->octet_ptr,
					    sysVars->sysUserPassw->length);
	    break;

	default:
	    return ((VarBind *) NULL);
	}

    /*
     *  Construct the response for the SNMP agent.  The structure returned
     *  will be strung onto the response PDU.  The global strucutre ZeroOid
     *  is used for leaf variables.
     */

    return (MakeVarBind (object, &ZeroOid, dp));
    }


/*******************************************************************************
*
* system_free - Free the systems data structure.
*
* This routine is invoked indirectly by the SNMP agent to free the system
* configuration structure that was allocated during the call to test.  It
* gets call after the call to set, or after the test fails.
*
* RETURNS: N/A
* 
* SEE ALSO:
*/
LOCAL void sysconfig_free 
    (
    sysconfig_t * sysVars
    )
    {
    if (sysVars != NULL) 
	{
	if (sysVars->sysUserName != NULL) 
	    {
	    FreeOctetString (sysVars->sysUserName);
	    }
	if (sysVars->sysUserPassw != NULL) 
	    {
	    FreeOctetString (sysVars->sysUserPassw);
	    }
	free ((char *) sysVars);
	}
    }

/*******************************************************************************
*
* sysconfig_cleanup -  Free all the resources allocated in sysconfig_test.
*
* This routine is invoked by the SNMP agent after the set operation has taken
* placed or if the test routine failed.
*
* RETURNS: NO_ERROR always successful.
*
* SEE ALSO:
*/
LOCAL int sysconfig_cleanup
    (
    doList_t *trash
    )
    {
    sysconfig_free (trash->data);
    sysconfig_free (trash->undodata);
    return NO_ERROR;
    }

/*******************************************************************************
*
* sysconfig_undo -  provided for future use
*
* RETURNS: ERROR
*
* SEE ALSO:
*/
LOCAL int sysconfig_undo
    (
    )
    {
    return UNDO_FAILED_ERROR;
    }

/*******************************************************************************
*
* sysconfig_test -  Validate new values for the MIB variables.
*
* This routine ensures that the value to which a MIB variable is to be set is
* in accordance to its MIB definition in the file vxdemo.my.  After all the
* variables required to update the an entry or variable are collected, the state
* of the doList structure is set to ADD_MODIFY or DELETE, based on the operaion
* requested.
*
* RETURNS: NO_ERROR for successful validation, otherwise an error is returned.
* 
* SEE ALSO:
*/
int sysconfig_test 
    (
    OID           * incoming,    /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo    * object,      /* Ptr to MIB var in the SNMP MIB tree */
    ObjectSyntax  * value,       /* Ptr the value to set the MIB variable */
    doList_t      * doHead,      /* Ptr to the list of SNMP outstanding reqs */
    doList_t      * doCur,       /* Ptr to free doList element */
    ContextInfo   * contextInfo  /* Reserved for future used. */
    )
    {
    sysconfig_t   * sysVars;
    int             instLength = incoming->length - object->oid.length;

    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.  In the case of a set operation the variable
     * needs to be specified exactly and therefore the length must be 1.
     */

    if (instLength != 1 || incoming->oid_ptr [object->oid.length] != 0) 
	return (NO_ACCESS_ERROR);

    /*
     * Create the data portion of the do-list element and initialize it. This
     * needs to use "calloc" so all the pointers in this structure are
     * initialized to NULL.
     */

    if ((doCur->data = (void *) calloc (1, sizeof(sysconfig_t))) == NULL) 
	{
	return (GEN_ERROR);
	}

    doCur->setMethod = sysconfig_set;
    doCur->cleanupMethod = sysconfig_cleanup;
    doCur->undoMethod = sysconfig_undo;
    doCur->state = UNKNOWN;

    sysVars = (sysconfig_t *) (doCur->data);

    /*
     * Validate the value and store it in the do-list.
     */

    switch (object->nominator) 
	{
	case I_sysState:
	
	    /* The system state can only be set to running = 1 or reboot = 2 */

	    if (value->sl_value != D_sysState_system_running && 
		value->sl_value != D_sysState_system_reboot)
		return (WRONG_VALUE_ERROR);

	    SET_VALID(I_sysState, sysVars->valid);  /* Set valid vector */
	    sysVars->sysState = value->sl_value;    /* Cpy to alloc struct */
	    break;

	case I_sysUserName:

	    if (value->os_value->length <= 0)
		return (WRONG_LENGTH_ERROR);

	    SET_VALID(I_sysUserName, sysVars->valid);
	    sysVars->sysUserName = MakeOctetString (value->os_value->octet_ptr,
						value->os_value->length);
	    break;

	case I_sysUserPassw:

	    if (value->os_value->length <= 0) 
		return (WRONG_LENGTH_ERROR);

	    SET_VALID(I_sysUserPassw, sysVars->valid);
	    sysVars->sysUserPassw = MakeOctetString (value->os_value->octet_ptr,
						value->os_value->length);
	    break;

	default:
	    DPRINTF((0, "snmpd: Internal error (invalid nominator \
		    in sysconfig_test)\n"));
	    return (GEN_ERROR);
    }				/* switch */

    doCur->state = ADD_MODIFY;

    return (NO_ERROR);
    }

/*******************************************************************************
*
* sysconfig_set -  Invokes k_ logic to set the variable.
*
* This set routine assumes that the variables to be set are correct.  This
* assumption is safe because the test routine has already validated the
* variable(s) and set the state of the operation.
*
* RETURNS: Success or Error to the SNMP agent.
* 
* SEE ALSO:
*/
int sysconfig_set 
    (
    doList_t    * doHead,      /* Ptr to the list of SNMP outstanding reqs */
    doList_t    * doCur,       /* Ptr to free doList element */
    ContextInfo * contextInfo  /* Reserved for future use */
    )
    {
    return (k_sysconfig_set ((sysconfig_t *) (doCur->data), contextInfo, 
			      doCur->state));
    }



/*******************************************************************************
*
* taskEntry_get -  Read the value of a variable in the requested task entry.
*
* This routine returns the value of the requested MIB variable that corresponds
* to its group.   All the values from the group are obtain from the "k_"
* logic but only the requested value is returned by this function.
*
* RETURNS: A pointer to a VarBind struct, or NULL if the request is invalid.
*
* SEE ALSO:
*/
VarBind * taskEntry_get 
    (
    OID          * incoming,    /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo   * object,      /* Ptr to MIB var in the SNMP MIB tree */
    int            searchType,  /* EXACT = GetRequest,NEXT = Get[Next|Bulk] */
    ContextInfo  * contextInfo, /* Reserved for future used */
    int            serialNum    /* Used for caching */
    )
    {
    static taskEntry_t    
		   * pTaskEntry;
    void           * dp;         /* Generic ptr used for returned variable */
    static OID       inst;	 /* Table entry index to return to agent */
    static unsigned long    
		     oidbuff;    /* OID buffer for inst variable */
    long             taskId;     /* Task ID is used for table index */
    int              instLength; /* Length of the incoming OID */
    static int       lastSerialNum;
    static OID     * last_incoming;
    int              reqVar = -1;

    if ((lastSerialNum != serialNum) || (pTaskEntry == NULL) ||
	(CmpOIDInst(incoming, last_incoming, object->oid.length) != 0))
	{
    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.
     */
	instLength = incoming->length - object->oid.length;

	/* 
	 * The task ID is set to zero in case the INDEX is not specified and the
	 * first table entry is requested.
	 */
	taskId = 0;

	switch (searchType) 
	    {
	    case EXACT:
		if (instLength == 1) 
		    {
		    reqVar = object->nominator;   /* Field in the table req */
		    taskId = (long) incoming->oid_ptr [object->oid.length];
		    }
		break;

	    case NEXT:
		reqVar = object->nominator;       /* Field in the table req */

		/*
		 * If the taskId was not specify, use task ID of 0 to access 
		 * the first entry, otherwise pick the taskId from the 
		 * requested OID.
		 */

		taskId = (instLength < 1) ? 0 :
			 (unsigned long) incoming->oid_ptr [object->oid.length];
		break;

	    default:
		DPRINTF((0, "snmpd: Internal error. (invalid search type in \
			 taskEntry_get -- %d)\n", searchType));
	    }

	/* Retrieve the data from the kernel-specific routine. */

	if (reqVar == -1 || 
	    (pTaskEntry = k_taskEntry_get (serialNum, contextInfo,
				       reqVar, searchType, taskId)) == NULL) 
	    {
	    return ((VarBind *) NULL); /* Entry not Found or End of the Table */
	    }

	/* 
	 * Return to SNMP NM the INDEX of the entry whose OID was requested
	 * this INDEX will be used to request more OIDs of the same entry 
	 * entry or to request the NEXT entry in the table.
	 */

	inst.length = 1;
	inst.oid_ptr = &oidbuff;
	inst.oid_ptr [0] = pTaskEntry->taskId;

	/* Get new catche information */

     	lastSerialNum = serialNum;

	if (last_incoming != NULL)
	    FreeOID(last_incoming);
		     
	last_incoming = CloneOID(incoming);
	}
    else
	{
	reqVar = object->nominator;       /* Field in the table req */
	}

    switch (reqVar) 
	{
	case I_taskName:
	    dp = (void *) MakeOctetString (pTaskEntry->taskName->octet_ptr,
					   pTaskEntry->taskName->length);
	    break;

	case I_taskMain:
	    dp = (void *) MakeOctetString (pTaskEntry->taskMain->octet_ptr,
					   pTaskEntry->taskMain->length);
	    break;

	default:
	    
	    /*
	     *  Any other variable requested can be access using the S_OFFSET
	     *  macro, which computes the offset of the variable in the 
	     *  structure and returns a pointer to the variable.
	     */

	    dp = S_OFFSET(pTaskEntry, reqVar);
	}

    /*
     *  Construct the response for the SNMP agent.  The structure returned
     *  will be strung onto the response PDU.  The inst parameter contains the
     *  index in the table for the requested variable.
     */

    return (MakeVarBind (object, &inst, dp));
    }

/*******************************************************************************
*
* taskEntry_free -  Free the taskEntry data structure.
*
* This routine is invoked indirectly by the SNMP agent to free the taskEntry
* structure that was allocated during the call to test, and after the call to
* set, or after the test fails.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
void taskEntry_free
    (
    taskEntry_t      * ptaskEntry
    )
    {
    if (ptaskEntry != NULL) 
	{
	if (ptaskEntry->taskName != NULL) 
	    FreeOctetString (ptaskEntry->taskName);

	if (ptaskEntry->taskMain != NULL) 
	    FreeOctetString (ptaskEntry->taskMain);

	free ((char *) ptaskEntry);
	}
    }

/*******************************************************************************
*
* taskEntry_cleanup -  Free all the resources allocated in taskEntry_test.
*
* This routine is invoked by the SNMP agent after the set operation has taken
* placed or if the test routine failed.
*
* RETURNS: NO_ERROR always successful.
*
* SEE ALSO:
*/
LOCAL int taskEntry_cleanup
    (
    doList_t *trash
    )
    {
    taskEntry_free (trash->data);
    taskEntry_free (trash->undodata);
    return NO_ERROR;
    }

/*******************************************************************************
*
* taskEntry_undo -  provided for future use
*
* RETURNS: ERROR
*
* SEE ALSO:
*/
LOCAL int taskEntry_undo
    (
    )
    {
    return UNDO_FAILED_ERROR;
    }

/*******************************************************************************
*
* taskEntry_test -  Validate new values for the MIB variables.
*
* This routine ensures that the value to which a MIB variable is to be set is
* in accordance to its MIB definition in the file vxdemo.my.  After all the
* variables required to update the an entry or variable is collected, the state
* of the doList structure is set to ADD_MODIFY or DELETE, based on the operaion
* requested.
*
* RETURNS: NO_ERROR for successful validation, otherwise an error is returned.
*
* SEE ALSO:
*/
int taskEntry_test 
    (
    OID           * incoming,    /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo    * object,      /* Ptr to MIB var in the SNMP MIB tree */
    ObjectSyntax  * value,       /* Ptr the value to set the MIB variable */
    doList_t      * doHead,      /* Ptr to the list of SNMP outstanding reqs */
    doList_t      * doCur,       /* Ptr to free doList element */
    ContextInfo   * contextInfo  /* Reserved for future used */
    )
    {
    long            taskId;     /* Task ID is used for table index */
    doList_t      * dp;
    taskEntry_t   * ptaskEntry;
    int             instLength;

    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.  In the case of a set operation the variable
     * needs to be specified exactly and therefore the length must be 1.
     */

    instLength = incoming->length - object->oid.length;

    if (instLength != 1)
	return (NO_ACCESS_ERROR);

    /* Get the index of the entry that is to be modified */

    taskId = (long) incoming->oid_ptr [incoming->length - 1];

    /*
     *  There is a single list of SNMP outstanding requests.  The list
     *  is shared by all "v_" routines in the agent.  Each time a variable
     *  is SET the list must be search for the corresponding data structure.
     *  if the structure does not exits it must be allocated.  This space
     *  is freed by the "*free" routine.
     */

    for (dp = doHead; dp != NULL; dp = dp->next) 
	{
	if ((dp->setMethod == taskEntry_set) &&
	    ((void *) (dp->data) != (void *) NULL) &&
	    (((taskEntry_t *) (dp->data))->taskId == taskId))
	    {
	    break;       /* Structure found */
	    }
	}

    if (dp == NULL)
	{
	dp = doCur;
	if ((dp->data = (void *) calloc (1, sizeof(taskEntry_t))) == NULL) 
	    {
	    DPRINTF((0, "snmpd: Can't alloc memory in taskEntry_test\n"));
	    return (GEN_ERROR);
	    }


	dp->setMethod     = taskEntry_set;
        dp->cleanupMethod = taskEntry_cleanup;
        dp->undoMethod    = taskEntry_undo;
	dp->state         = UNKNOWN;

        SET_VALID(I_taskId, ((taskEntry_t *) (dp->data))->valid);
        ((taskEntry_t *) (dp->data))->taskId = taskId;
	}

    /* 
     * Zero is not a valid task ID in vxWorks and is used in this 
     * implementation for the creation of new tasks.  If the ID is
     * not zero then the ID is validated.
     */

    if ((taskId != 0) && (k_taskEntry_get (-1, contextInfo, object->nominator, 
					   EXACT, taskId) == NULL))
	return (NO_ACCESS_ERROR);  /* taskId does not exist */
	
    /*
     * Add the SET request to the do-list element. Test the validity of the
     * value requested to be set.
     */

     ptaskEntry = (taskEntry_t *) dp->data;

     switch (object->nominator)
	 {
	 case I_taskId:

	     /* The task ID of an executing task can't be changed. */

	     if ((taskId != 0) && (taskId != value->sl_value))
		    return (NOT_WRITABLE_ERROR);

	     /* ELSE: Not used for anything else in this code */
	     break;

	 case I_taskName:

	    /* The name of the task can be only changed at creation time */

	    if (taskId != 0)
		return (NOT_WRITABLE_ERROR);

	     ptaskEntry->taskName = MakeOctetString (value->os_value->octet_ptr,
						     value->os_value->length);
	     SET_VALID(I_taskName, ptaskEntry->valid);
	     break;

	 case I_taskPriority:

	    /* Task Priority must be in the range 1 - 255 */

	    if (value->sl_value < 0 || value->sl_value > 255)
		return (WRONG_VALUE_ERROR);

	    ptaskEntry->taskPriority = value->sl_value;
	    SET_VALID(I_taskPriority, ptaskEntry->valid);
	    break;

	 case I_taskStatus:

	    /* The status of a task can only if the task has been created */

	    if (taskId == 0)
		return (NOT_WRITABLE_ERROR);

	    if (value->sl_value != D_taskStatus_task_ready && 
		value->sl_value != D_taskStatus_task_suspended &&
		value->sl_value != D_taskStatus_task_delay &&
		value->sl_value != D_taskStatus_task_deleted)
	    return (WRONG_LENGTH_ERROR);

	    ptaskEntry->taskStatus = value->sl_value;
	    SET_VALID(I_taskStatus, ptaskEntry->valid);
	    break;

	 case I_taskOptions:

	    /*
	     * If a task is being created any options can be changed. Otherwise
	     * the only option that can be changed is VX_UNBREAKABLE.
	     */

	    if (taskId == 0)
		{
		if ((value->sl_value & (VX_UNBREAKABLE | VX_DEALLOC_STACK |
		    VX_FP_TASK | VX_PRIVATE_ENV | VX_NO_STACK_FILL |
                    VX_SUPERVISOR_MODE | VX_STDIO)) == 0 )
		    return (WRONG_VALUE_ERROR);
		}
	    else
		{
		/* Task is running and only VX_UNBREAKABLE can be changed */

		if (value->sl_value != VX_UNBREAKABLE)
		    return (WRONG_VALUE_ERROR);
		}

	    ptaskEntry->taskOptions = value->sl_value;
	    SET_VALID(I_taskOptions, ptaskEntry->valid);
	    break;

	 case I_taskMain:

	    /* Task entry point routine, can be only set at creation time */

	    if (taskId != 0)
		return (NOT_WRITABLE_ERROR);

	    ptaskEntry->taskMain = MakeOctetString (value->os_value->octet_ptr,
						     value->os_value->length);
	    SET_VALID(I_taskMain, ptaskEntry->valid);
	    break;

	 case I_taskStackSize:

	    /* The stack size can only set at the time the task is created */

	    if (taskId != 0)
		return (NOT_WRITABLE_ERROR);

	    ptaskEntry->taskStackSize = value->ul_value;
	    SET_VALID(I_taskStackSize, ptaskEntry->valid);
	    break;

	default:
		DPRINTF((0, "snmpd: Internal error (invalid nominator \
			 in atEntry_test)\n"));
		return (GEN_ERROR);
	 }

    /* 
     *  Existing table entries that are being modified are moved to the
     *  ADD_MODIFY state.  If a task is being created all the required
     *  parameters must be set before the request is moved to the ADD_MODIFY
     *  state.
     */
    if (taskId != 0)
	{
	if (ptaskEntry->taskStatus == D_taskStatus_task_deleted)
	    dp->state = DELETE;
	else
	    dp->state = ADD_MODIFY;
	}
    else
	if (VALID(I_taskName, ptaskEntry->valid) &&
	    VALID(I_taskMain, ptaskEntry->valid) &&
	    VALID(I_taskPriority, ptaskEntry->valid) &&
	    VALID(I_taskStackSize, ptaskEntry->valid) &&
	    VALID(I_taskOptions, ptaskEntry->valid))
	    dp->state = ADD_MODIFY;

    return (NO_ERROR);
    }

/*******************************************************************************
*
* taskEntry_set -  Invokes k_ logic to set the task entry.
*
*
* This set routine assumes that the variables to be set are correct.  This
* assumption is safe because the test routine has already validated the
* variable(s) and set the state of the operation.
*
* RETURNS: Success or Error to the SNMP agent.

* SEE ALSO:
*/
int taskEntry_set 
    (
    doList_t      * doHead,      /* Ptr to the list of SNMP outstanding reqs */
    doList_t      * doCur,       /* Ptr to free doList element */
    ContextInfo   * contextInfo  /* Reserved for future used */
    )
    {
    return (k_taskEntry_set ((taskEntry_t *) (doCur->data), contextInfo, 
			     doCur->state));
    }


/*******************************************************************************
*
* sysmemory_get -  Read the value of a variable in the memory group.
*
* This routine returns the value of the requested MIB variable that corresponds
* to its group.   All the values from the group are obtain from the "k_"
* logic but only the requested value is returned by this function.
*
* RETURNS: A pointer to a VarBind struct, or NULL if the request is invalid.
*
* SEE ALSO:
*/
VarBind * sysmemory_get 
    (
    OID          * incoming,    /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo   * object,      /* Ptr to MIB var in the SNMP MIB tree */
    int            searchType,  /* EXACT = GetRequest,NEXT = Get[Next|Bulk] */
    ContextInfo  * contextInfo, /* Reserved for future used */
    int            serialNum    /* Used for caching */
    )
    {
    static sysmemory_t  
		 * memVars;	/* Struct Generated by the MIB Comp. */
    static int     lastSerialNum;
    void         * dp;          /* Generic ptr used for returned variable */
    int            reqVar = -1; /* Variable Requested by the Agent */
    int            instLength;  /* Length of the incoming OID */
    static OID   * last_incoming;

    if ((lastSerialNum != serialNum) || (memVars == NULL) ||
	(CmpOIDInst(incoming, last_incoming, object->oid.length) != 0))
	{
    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.
     */
	instLength = incoming->length - object->oid.length;


	switch (searchType) 
	    {
	    case EXACT:
		if (instLength == 1 && 
		    incoming->oid_ptr[incoming->length - 1] == 0) 
		{
		    reqVar = object->nominator;
		}
		break;

	    case NEXT:
		if (instLength <= 0) 
		{
		    reqVar = object->nominator;
		}
		break;

	    default:
		DPRINTF ((0, "snmpd: Internal error. (invalid search type in \
			  sysmemory_get -- %d)\n", searchType));
	    }				/* switch */

	/* Retrieve the data from the kernel-specific routine.  */

	if (reqVar == -1 || (memVars = k_sysmemory_get (serialNum, contextInfo,
							reqVar)) == NULL) 
	    {
	    return ((VarBind *) NULL);
	    }

	/* Get new catche information */

     	lastSerialNum = serialNum;

	if (last_incoming != NULL)
	    FreeOID(last_incoming);
		     
	last_incoming = CloneOID(incoming);
	}
    else
	{
	reqVar = object->nominator;
	}

    /*
     *  Construct the response for the SNMP agent.  The structure returned
     *  will be strung onto the response PDU.  The global strucutre ZeroOid
     *  is used for leaf variables.  The S_OFFSET macro computes the offset
     *  of the variable in the memVars structure and passes a pointer to the
     *  variable to the MakeVarBind function.
     */

    return (MakeVarBind (object, &ZeroOid, S_OFFSET(memVars, reqVar)));
    }


/*******************************************************************************
*
* sysnfs_get -  Read the value of a variable in the nfs group.
*
* This routine returns the value of the requested MIB variable that corresponds
* to its group.   All the values from the group are obtain from the "k_"
* logic but only the requested value is returned by this function.
*
* RETURNS: A pointer to a VarBind struct, or NULL if the request is invalid.
*
* SEE ALSO:
*/
VarBind * sysnfs_get 
    (
    OID          * incoming,    /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo   * object,      /* Ptr to MIB var in the SNMP MIB tree */
    int            searchType,  /* EXACT = GetRequest,NEXT = Get[Next|Bulk] */
    ContextInfo  * contextInfo, /* Reserved for future used */
    int            serialNum    /* Used for caching */
    )
    {
    static sysnfs_t     
		 * nfsVars;     /* Ptr to struct than contains the MIB vars */
    static int     lastSerialNum;
    void         * dp;          /* Generic ptr used for returned variable */
    int            reqVar = -1; /* Variable Requested by the Agent */
    int            instLength;  /* Length of the incoming OID */
    static OID   * last_incoming;

    if ((lastSerialNum != serialNum) || (nfsVars == NULL) ||
	(CmpOIDInst(incoming, last_incoming, object->oid.length) != 0))
	{
    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.
     */
	instLength = incoming->length - object->oid.length;

	switch (searchType) 
	    {
	    case EXACT:
		if (instLength == 1 && 
		    incoming->oid_ptr [incoming->length - 1] == 0) 
		    {
		reqVar = object->nominator;
		    }
		break;

	    case NEXT:
		if (instLength <= 0) 
		    {
		    reqVar = object->nominator;
		    }
		break;

	    default:
		DPRINTF ((0, "snmpd: Internal error. (invalid search type in \
			  sysnfsy_get -- %d)\n", searchType));
	    }				/* switch */

	/* Retrieve the data from the kernel-specific routine.  */

	if (reqVar == -1 || (nfsVars = k_sysnfs_get (serialNum, contextInfo, 
						     reqVar)) == NULL) 
	    {
	    return ((VarBind *) NULL);
	    }

	/* Get new catche information */

     	lastSerialNum = serialNum;		/* Get new catch */

	if (last_incoming != NULL)
	    FreeOID(last_incoming);
		     
	last_incoming = CloneOID(incoming);
	}
    else
	{
	reqVar = object->nominator;
	}

    /* Build the variable binding.  */

    switch (reqVar) 
	{
	case I_nfsUserId:
	    dp = (void *) &nfsVars->nfsUserId;
	    break;

	case I_nfsGroupId:
	    dp = (void *) &nfsVars->nfsGroupId;
	    break;

	default:
	    return ((VarBind *) NULL);
	}

    /*
     *  Construct the response for the SNMP agent.  The structure returned
     *  will be strung onto the response PDU.  The global strucutre ZeroOid
     *  is used for leaf variables.
     */

    return (MakeVarBind (object, &ZeroOid, dp));
    }

/*******************************************************************************
*
* sysnfs_free -  Free the nfs structure.
*
* This routine is invoked indirectly by the SNMP agent to free the system
* nfs structure that was allocated during the call to test, and after the call
* to set, or after the test fails.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
LOCAL void sysnfs_free 
    (
    sysnfs_t * nfsVars
    )
    {
    if (nfsVars != NULL) 
	{
	free ((char *) nfsVars);
	}
    }

/*******************************************************************************
*
* sysnfs_cleanup -  Free all the resources allocated in sysnfs_test.
*
* This routine is invoked by the SNMP agent after the set operation has taken
* placed or if the test routine failed.
*
* RETURNS: NO_ERROR always successful.
*
* SEE ALSO:
*/
LOCAL int sysnfs_cleanup
    (
    doList_t *trash
    )
    {
    sysnfs_free (trash->data);
    sysnfs_free (trash->undodata);
    return NO_ERROR;
    }

/*******************************************************************************
*
* sysnfs_undo -  provided for future use
*
* RETURNS: ERROR
*
* SEE ALSO:
*/
LOCAL int sysnfs_undo
    (
    )
    {
    return UNDO_FAILED_ERROR;
    }

/*******************************************************************************
*
* sysnfs_test -  Validate new values for the MIB variables.
*
* This routine ensures that the value to which a MIB variable is to be set is
* in accordance to its MIB definition in the file vxdemo.my.  After all the
* variables required to update the an entry or variable is collected, the state
* of the doList structure is set to ADD_MODIFY or DELETE, based on the operaion
* requested.
*
* RETURNS: NO_ERROR for successful validation, otherwise an error is returned.
*
* SEE ALSO:
*/
int sysnfs_test 
    (
    OID           * incoming,    /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo    * object,      /* Ptr to MIB var in the SNMP MIB tree */
    ObjectSyntax  * value,       /* Ptr the value to set the MIB variable */
    doList_t      * doHead,      /* Ptr to the list of SNMP outstanding reqs */
    doList_t      * doCur,       /* Ptr to free doList element */
    ContextInfo   * contextInfo  /* Reserved for Future Use */
    )
    {
    sysnfs_t      * nfsVars;
    int             instLength;

    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.  In the case of a set operation the variable
     * needs to be specified exactly and therefore the length must be 1.
     */

    instLength = incoming->length - object->oid.length;

    if (instLength != 1 || incoming->oid_ptr [object->oid.length] != 0)
	return (NO_ACCESS_ERROR);

    /*
     * Create the data portion of the do-list element and initialize it. This
     * needs to use "calloc" so all the pointers in this structure are
     * initialized to NULL. The state is set to READY since the element will
     * be deleted if invalid.
     */

    if ((doCur->data = (void *) calloc (1, sizeof(sysnfs_t))) == NULL) 
	{
	return (GEN_ERROR);
	}

    doCur->setMethod     = sysnfs_set;
    doCur->cleanupMethod = sysnfs_cleanup;
    doCur->undoMethod   = sysnfs_undo;
    doCur->state         = UNKNOWN;

    nfsVars = (sysnfs_t *) (doCur->data);

    /* Validate the value and store it in the do-list.  */

    switch (object->nominator) 
	{
	case I_nfsUserId:
	
	    /* The User ID must be a positive number */

	    if (value->sl_value < 0)
		return (WRONG_VALUE_ERROR);

	    SET_VALID(I_nfsUserId, nfsVars->valid);  /* Set valid vector */
	    nfsVars->nfsUserId = value->sl_value;  /* Cpy to alloc struct */
	    break;

	case I_nfsGroupId:

	    /* The Group ID must be a positive number */

	    if (value->sl_value < 0)
		return (WRONG_VALUE_ERROR);

	    SET_VALID(I_nfsGroupId, nfsVars->valid);  /* Set valid vector */
	    nfsVars->nfsGroupId = value->sl_value;  /* Cpy to alloc struct */
	    break;

	default:
	    DPRINTF((0, "snmpd: Internal error (invalid nominator \
		    in sysnfs_test)\n"));
	    return (GEN_ERROR);
    }				/* switch */

    doCur->state = ADD_MODIFY;

    return (NO_ERROR);
    }

/*******************************************************************************
*
* sysnfs_set -  Invokes k_ logic to set the sysnfs variable.
*
* This set routine assumes that the variables to be set are correct.  This
* assumption is safe because the test routine has already validated the
* variable(s) and set the state of the operation.
*
* RETURNS: Success or Error to the SNMP agent.
*
* SEE ALSO:
*/
int sysnfs_set 
    (
    doList_t       * doHead,      /* Ptr to the list of SNMP outstanding reqs */
    doList_t       * doCur,       /* Ptr to free doList element */
    ContextInfo    * contextInfo  /* Reserved for future used */
    )
    {
    return (k_sysnfs_set ((sysnfs_t *) (doCur->data), contextInfo,
			   doCur->state));
    }


/*******************************************************************************
*
* nfsEntry_get -  Read the value of a variable in the requested NFS entry.
*
* This routine returns the value of the requested MIB variable that corresponds
* to its group.   All the values from the group are obtain from the "k_"
* logic but only the requested value is returned by this function.
*
* RETURNS: A pointer to a VarBind struct, or NULL if the request is invalid.
*
* SEE ALSO:
*/
VarBind * nfsEntry_get 
    (
    OID          * incoming,    /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo   * object,      /* Ptr to MIB var in the SNMP MIB tree */
    int            searchType,  /* EXACT = GetRequest,NEXT = Get[Next|Bulk] */
    ContextInfo  * contextInfo, /* Reserved for future used */
    int            serialNum    /* Used for caching */
    )
    {
    static nfsEntry_t   
		 * pNfsEntry;   /* Ptr to struct with MIB variables */
    static OID     inst;	/* Table entry index to return to agent */
    static unsigned long  
		   oidbuff;     /* OID buffer for inst variable */
    void         * dp;          /* Generic ptr used for returned variable */
    long           entryIndex;  /* Table entry index */
    int            instLength;  /* Length of the incoming OID */
    static int     lastSerialNum;
    static OID   * last_incoming;
    int            reqVar = -1;


    if ((lastSerialNum != serialNum) || (pNfsEntry == NULL) ||
	(CmpOIDInst(incoming, last_incoming, object->oid.length) != 0))
	{
    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.
     */
	instLength = incoming->length - object->oid.length;

	switch (searchType) 
	    {
	    case EXACT:
		if (instLength == 1) 
		    {
		    reqVar = object->nominator;   /* Field in the table req */
		    entryIndex = (long) incoming->oid_ptr [object->oid.length];
		    }
		break;

	    case NEXT:
		reqVar = object->nominator;       /* Field in the table req */

		/*
		 * If the Index was not specified in the request, pick zero as
		 * the first entry.  This lets the k_ logic start at the first
		 * entry. Otherwise take the table index passed by the agent.
		 */

		entryIndex = (instLength < 1) ? 0 :
			     (long) incoming->oid_ptr [object->oid.length];
		break;

	    default:
		DPRINTF((0, "snmpd: Internal error. (invalid search type in \
			 nfsEntry_get -- %d)\n", searchType));
	    }

	if (reqVar == -1 ||
	    (pNfsEntry = k_nfsEntry_get (serialNum, contextInfo, reqVar, 
					 searchType, entryIndex)) == NULL) 
	    {
	    return ((VarBind *) NULL); /* Entry not Found or End of the Table */
	    }

       /* Return to SNMP NM the INDEX of the entry whose OID was requested
	* this INDEX will be used to request more OIDs of the same entry 
	* entry or to request the NEXT entry in the table.
	*/
	inst.length = 1;
	inst.oid_ptr = &oidbuff;
	inst.oid_ptr [0] = pNfsEntry->nfsIndex;

	/* Get new catche information */

     	lastSerialNum = serialNum;

	if (last_incoming != NULL)
	    FreeOID(last_incoming);
		     
	last_incoming = CloneOID(incoming);
	}
    else
	{
	reqVar = object->nominator;       /* Field in the table req */
	}

    switch (reqVar) 
	{
	case I_nfsIndex:
	    dp = (void *) &pNfsEntry->nfsIndex;
	    break;

	case I_nfsState:
	    dp = (void *) &pNfsEntry->nfsState;
	    break;

	case I_nfsHostName:
	    dp = (void *) MakeOctetString (pNfsEntry->nfsHostName->octet_ptr,
					   pNfsEntry->nfsHostName->length);
	    break;

	case I_nfsHostIpAddr:
	    dp = (void *) MakeOctetString ((unsigned char *) 
					    &pNfsEntry->nfsHostIpAddr, 4L);
	    break;

	case I_nfsHostFileSysName:
	    dp = (void *) 
		  MakeOctetString (pNfsEntry->nfsHostFileSysName->octet_ptr,
				   pNfsEntry->nfsHostFileSysName->length);
	    break;

	case I_nfsLocalFileSysName:
	    dp = (void *) 
		  MakeOctetString (pNfsEntry->nfsLocalFileSysName->octet_ptr,
				   pNfsEntry->nfsLocalFileSysName->length);
	    break;

	default:
	    return ((VarBind *) NULL);
	}

    /*
     *  Construct the response for the SNMP agent.  The structure returned
     *  will be strung onto the response PDU.  The inst parameter contains the
     *  index in the table for the requested variable.
     */

    return (MakeVarBind (object, &inst, dp));
    }

/*******************************************************************************
*
* nfsEntry_free -  Free the nfsEntry data structure.
*
* This routine is invoked indirectly by the SNMP agent to free the nfsEntry
* structure that was allocated during the call to test, and after the call to
* set, or after the test fails.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
LOCAL void nfsEntry_free
    (
    nfsEntry_t      * pNfsEntry
    )
    {
	if (pNfsEntry != NULL) 
	    {
	    if (pNfsEntry->nfsHostName != NULL) 
		FreeOctetString (pNfsEntry->nfsHostName);

	    if (pNfsEntry->nfsHostFileSysName != NULL) 
		FreeOctetString (pNfsEntry->nfsHostFileSysName);

	    if (pNfsEntry->nfsLocalFileSysName != NULL) 
		FreeOctetString (pNfsEntry->nfsLocalFileSysName);

	    free( (char *) pNfsEntry);
	    }
    }

/*******************************************************************************
*
* nfsEntry_cleanup -  Free all the resources allocated in nfsEntry_test.
*
* This routine is invoked by the SNMP agent after the set operation has taken
* placed or if the test routine failed.
*
* RETURNS: NO_ERROR always successful.
*
* SEE ALSO:
*/
LOCAL int nfsEntry_cleanup
    (
    doList_t *trash
    )
    {
    nfsEntry_free(trash->data);
    nfsEntry_free(trash->undodata);
    return NO_ERROR;
    }

/*******************************************************************************
*
* nfsEntry_undo -  provided for future use
*
* RETURNS: ERROR
*
* SEE ALSO:
*/
LOCAL int nfsEntry_undo
    (
    )
    {
    return UNDO_FAILED_ERROR;
    }

/*******************************************************************************
*
* nfsEntry_test -  Validate new values for the MIB variables.
*
* This routine ensures that the value to which a MIB variable is to be set is
* in accordance to its MIB definition in the file vxdemo.my.  After all the
* variables required to update the an entry or variable is collected, the state
* of the doList structure is set to ADD_MODIFY or DELETE, based on the operaion
* requested.
*
* RETURNS: NO_ERROR for successful validation, otherwise an error is returned.
*
* SEE ALSO:
*/
int nfsEntry_test 
    (
    OID           * incoming,    /* Ptr to the OID received in the SNMP PDU */
    ObjectInfo    * object,      /* Ptr to MIB var in the SNMP MIB tree */
    ObjectSyntax  * value,       /* Ptr the value to set the MIB variable */
    doList_t      * doHead,      /* Ptr to the list of SNMP outstanding reqs */
    doList_t      * doCur,       /* Ptr to free doList element */
    ContextInfo   * contextInfo  /* Reserved for future used */
    )
    {
    long            entryIndex;  /* Table Index to be set */
    int             i;		 /* Generic index var */
    doList_t      * dp;
    nfsEntry_t    * pNfsEntry;
    int             instLength;


    /* 
     * instLength represents the length of the (incoming) requested OID
     * beyond the Object Identifier specified in the MIB tree.  This value
     * is used to validate the request on an EXACT or NEXT search.  Ex.
     * The MIB variable fakeMibVar = OID 1.3.6.1.4.1.731.1.1.2 and is 
     * specified this way in the object variable.  The incoming would
     * have fakeMibVar.0 = OID 1.3.6.1.4.1.731.1.1.2.0, the length would
     * be 1 and the last value 0.  This information is used to valiate the
     * EXACT search.  In the case of a NEXT search the length must be 0
     * for leaf variables.  For table entries with an integer index the 
     * incoming OID would be 1.3.6.1.4.1.731.1.1.2.INDEX and the length
     * would represent the index.  In the case of a set operation the variable
     * needs to be specified exactly and therefore the length must be 1.
     */

    instLength = incoming->length - object->oid.length;

    if (instLength != 1) 
	{
	DPRINTF((APWARN, "snmpd: Invalid instance length (%d)\n", instLength));
	return (NO_ACCESS_ERROR);
	}

    /* Get the index of the entry that is to be modified or created */

    entryIndex = (long) incoming->oid_ptr [incoming->length - 1];

    /*
     *  There is a single list of SNMP outstanding requests.  The list
     *  is shared by all "v_" routines in the agent.  Each time a variable
     *  is SET the list must be search for the corresponding data structure.
     *  if the structure does not exits it must be allocated.  This space
     *  is freed by the "*free" routine.
     */

    for (dp = doHead; dp != NULL; dp = dp->next) 
	{
	if ((dp->setMethod == nfsEntry_set) &&
	    ((void *) (dp->data) != (void *) NULL) &&
	    (((nfsEntry_t *) (dp->data))->nfsIndex == entryIndex))
	    {
	    break;       /* Structure found */
	    }
	}

    if (dp == NULL)
	{
	dp = doCur;
	if ((dp->data = (void *) calloc (1, sizeof(nfsEntry_t))) == NULL) 
	    {
	    DPRINTF((0, "snmpd: Can't alloc memory in nfsEntry_test\n"));
	    return (GEN_ERROR);
	    }

	dp->setMethod = nfsEntry_set;
        dp->cleanupMethod = nfsEntry_cleanup;
        dp->undoMethod = nfsEntry_undo;
	dp->state = UNKNOWN;
	((nfsEntry_t *) dp->data)->nfsIndex = entryIndex;
	SET_VALID(I_nfsIndex, ((nfsEntry_t *) dp->data)->valid);
	}


     pNfsEntry = (nfsEntry_t *) dp->data;

     switch (object->nominator)
	 {
	 case I_nfsIndex:

	    /*
	     * The index specified in the instance must match the index
	     * value specified. The value was already saved above.
	     */

	    if (value->sl_value != entryIndex) 
		{
		return (WRONG_VALUE_ERROR);
		}
	    SET_VALID(I_nfsIndex, pNfsEntry->valid);
	    break;

	 case I_nfsState:

	    /* The entry state must be nfs-mounted(1) or nfs-unmount(2) */

	    if (value->sl_value != D_nfsState_nfs_mounted && 
		value->sl_value != D_nfsState_nfs_unmount)
		{
		return (WRONG_VALUE_ERROR);
		}
	    pNfsEntry->nfsState = value->sl_value;
	    SET_VALID(I_nfsState, pNfsEntry->valid);
	    break;

	 case I_nfsHostName:

	    pNfsEntry->nfsHostName = 
				MakeOctetString (value->os_value->octet_ptr,
						 value->os_value->length);
	    SET_VALID(I_nfsHostName, pNfsEntry->valid);
	    break;

	 case I_nfsHostIpAddr:
	
	    if (value->os_value->length != 4)
		return (WRONG_LENGTH_ERROR);

	    for (i = 3; i >= 0; i--) 
		{
		if (value->os_value->octet_ptr[i] > 255)
		    return (WRONG_VALUE_ERROR);

		((unsigned char *) (&pNfsEntry->nfsHostIpAddr))[i] = 
						value->os_value->octet_ptr[i];
		}

	    SET_VALID(I_nfsHostIpAddr, pNfsEntry->valid);
	    break;


	 case I_nfsHostFileSysName:

	    pNfsEntry->nfsHostFileSysName = 
				MakeOctetString (value->os_value->octet_ptr,
						 value->os_value->length);
	    SET_VALID(I_nfsHostFileSysName, pNfsEntry->valid);
	    break;

	 case I_nfsLocalFileSysName:

	    pNfsEntry->nfsLocalFileSysName = 
				MakeOctetString (value->os_value->octet_ptr,
						 value->os_value->length);
	    SET_VALID(I_nfsLocalFileSysName, pNfsEntry->valid);
	    break;

	default:
		DPRINTF((0, "snmpd: Internal error (invalid nominator \
			 in atEntry_test)\n"));
		return (GEN_ERROR);
	 }

	/*
	 * Two operations are allowed in NFS table entries.  To delete an
	 * entry the index and state must be specified.  To add an entry
	 * all the parameters must be specified.
	 */

	if (VALID(I_nfsIndex, pNfsEntry->valid) &&
	    VALID(I_nfsState, pNfsEntry->valid))
	    {
	    if (pNfsEntry->nfsState == D_nfsState_nfs_unmount)
		{
		dp->state = DELETE;
		}
	    else
		{
		if (VALID(I_nfsHostName, pNfsEntry->valid) &&
		    VALID(I_nfsHostIpAddr, pNfsEntry->valid) &&
		    VALID(I_nfsHostFileSysName, pNfsEntry->valid) &&
		    VALID(I_nfsLocalFileSysName, pNfsEntry->valid))
		    {
		    dp->state = ADD_MODIFY;
		    }
		}
	    }
	
	return (NO_ERROR);
    }

/*******************************************************************************
*
* nfsEntry_set -  Invokes k_ logic to set the NFS Entry.
*
* This set routine assumes that the variables to be set are correct.  This
* assumption is safe because the test routine has already validated the
* variable(s) and set the state of the operation.
*
* RETURNS: Success or Error to the SNMP agent.
*
* SEE ALSO:
*/
int nfsEntry_set 
    (
    doList_t    * doHead,      /* Ptr to the list of SNMP outstanding reqs */
    doList_t    * doCur,       /* Ptr to free doList element */
    ContextInfo	* contextInfo  /* Reserved for future used */
    )
    {
    return (k_nfsEntry_set ((nfsEntry_t *) (doCur->data), contextInfo, 
			    doCur->state));
    }

