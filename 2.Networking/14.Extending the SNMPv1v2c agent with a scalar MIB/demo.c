/* demo.c - Demo MIB for SNMPv1/v2c in Tornado 1.0 */

/* Copyright 1992-1996 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,14apr97,rv  fixed SPR 8373
*/
 
/* 
 * This example is from the SNMPv1/v2c Component Release Supplement 1.0
 * for Tornado 1.0.1
 * Section 6.3 Compile-time MIB Extensions
 * Example 6-4 Augmented mibcomp Output
 *
 * The following example shows demo.c after it has been augmented by a MIB 
 * designer, complete with method routines.
 */

#include <remLib.h>
#include <taskLib.h>
#include <asn1.h>
#include <buffer.h>
#include <mib.h>
#include <snmpdefs.h>
#include <snmp.h>
#include <auxfuncs.h>
#include "mibleaf.h"

/*
 * Method routines for the sysconfig variables:
 *
 *   sysState -- read-write
 * The VxWorks target status is reported through this
 * field.  The only valid state that this field can be
 * set to is system-reboot.  The system will be rebooted
 * five seconds after changing the state.
 *
 *   sysUserName -- read-write
 * The user name the VxWorks target uses for remote
 * machine access.
 *
 *   sysUserPassw -- read-write
 * The user password the VxWorks target uses for remote
 * machine access.
 */

/* An internal routine to retrieve the values of the variables, used
 * by the method routines sysconfig_get and sysconfig_next.
 * You need to replace the type STRUCT_sysconfig with something
 * appropriate to your system. */


/* ############################################### 

   This is a structure we define to set and retrieve the prameters
   of thie sysconfig group
*/

typedef  struct
    {
    int     sysState;
    char    sysUserName [MAXSIZE_sysUserName + 1];
    char    sysUserPassw [MAXSIZE_sysUserPassw + 1];
    }  STRUCT_sysconfig;

/* ###############################################
   Define an instance of this structure. 
*/




static  STRUCT_sysconfig sysVars;

/* This routine is a utility routine which will be used by both the 
   the get and the next method routine. It takes the STRUCT_sysconfig
   structure we defined above as input and extracts the appropriate value 
   from it for installaltion into the response packet with the relevant
   getproc procedure. This STRUCT_sysconfig structure would have been
   filled in with the required value from within the get or next method
   routine as appropriate.
*/

static int
  sysconfig_get_value(OIDC_T      lastmatch,
                      SNMP_PKT_T *pktp,
                      VB_T       *vbp,
                      STRUCT_sysconfig *data)     /* !!! */
{
  switch(lastmatch) {

  case LEAF_sysState:
    /* Values:
     *  system-running(1) = VAL_sysState_system_running
     *  system-reboot(2)  = VAL_sysState_system_reboot
     */
    getproc_got_int32(pktp, vbp, data->sysState);    /* !!! */
    break;

  case LEAF_sysUserName:
    /* if the data being returned is in dynamic storage and needs
     * to be free'd, change the 0 argument to a 1. */

    getproc_got_string(pktp, vbp, strlen (data->sysUserName),
						data->sysUserName, 0, VT_STRING);    /* !!! */
    break;

  case LEAF_sysUserPassw:
    /* if the data being returned is in dynamic storage and needs
     * to be free'd, change the 0 argument to a 1. */
    getproc_got_string(pktp, vbp, strlen(data->sysUserPassw),
						data->sysUserPassw, 0, VT_STRING);    /* !!! */
    break;

  default:
    return GEN_ERR;
  }
  return NO_ERROR;
}

/* This is the get method routine. After doing some checks on the
   input varbind we retreive the required values with the appropriate 
   vxworks calls aand call the utility rtn defined above.
*/

void
  sysconfig_get(OIDC_T      lastmatch,
                int         compc,
                OIDC_T     *compl,
                SNMP_PKT_T *pktp,
                VB_T       *vbp)
{
  int              error;

  /* find all the varbinds that share the same getproc and instance */

  snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);

  /* check that the instance is exactly .0 */
  if (!((compc == 1) && (*compl == 0)))
    {
    for ( ; vbp; vbp = vbp->vb_link)
      getproc_nosuchins(pktp, vbp);
    return;
    }

  sysVars.sysState = VAL_sysState_system_running;  /* System is Running */

  /* Get data from vxWorks */

  remCurIdGet (sysVars.sysUserName, sysVars.sysUserPassw);   


  /* retrieve all the values from the same data structure */

  for ( ; vbp; vbp = vbp->vb_link) {
    if ((error = sysconfig_get_value(vbp->vb_ml.ml_last_match, pktp, 
		vbp, &sysVars)) != NO_ERROR)
      getproc_error(pktp, vbp, error);
    }
}


/* The next method routine. It is similar to the get routine except the
   check done is different 
 */

void
  sysconfig_next(OIDC_T      lastmatch,
                 int         compc,
                 OIDC_T     *compl,
                SNMP_PKT_T *pktp,
                VB_T       *vbp)
{
  OIDC_T instance = 0;

  /* the only time there's a next for a scalar is if we're given
   * no instance at all. */

  if (compc != 0)
    {
    nextproc_no_next(pktp, vbp);
    return;
    }

  sysVars.sysState = VAL_sysState_system_running;  /* System is Running */

  /* Get data from VxWorks */
  remCurIdGet (sysVars.sysUserName, sysVars.sysUserPassw); 

  for (snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);
     vbp; vbp = vbp->vb_link) {
     nextproc_next_instance(pktp, vbp, 1, &instance);
     sysconfig_get_value(vbp->vb_ml.ml_last_match, pktp, vbp, &sysVars);
  }
}


/* The test method routine. We perform a number fo checks on the 
* input varbinds to decide if the set should proceed or not
*/

void
  sysconfig_test(OIDC_T      lastmatch,
                 int         compc,
                 OIDC_T     *compl,
                SNMP_PKT_T *pktp,
                VB_T       *vbp)
{
  VB_T *group_vbp;

  /* Only scalar variables here, check for .0 */
  if (!((compc == 1) && (*compl == 0))) {
    testproc_error(pktp, vbp, NO_SUCH_NAME);
    return;
  }

  /* find all the varbinds that share the same getproc and instance and
   * group them together. */

  snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);

  /* now check each varbind */

  for (group_vbp = vbp;
       group_vbp;
       group_vbp = group_vbp->vb_link) {

    switch (group_vbp->vb_ml.ml_last_match) {

    case LEAF_sysState:

      switch (VB_GET_INT32(group_vbp)) {
      case VAL_sysState_system_reboot:
        break;

      case VAL_sysState_system_running:
      default:
        testproc_error(pktp, group_vbp, WRONG_VALUE);
        continue;
      }

      testproc_good(pktp, group_vbp);
      break;

    case LEAF_sysUserName:

      { long length = EBufferUsed(VB_GET_STRING(group_vbp));

        if (!((length >= MINSIZE_sysUserName) &&
              (length <= MAXSIZE_sysUserName))) {
          testproc_error(pktp, group_vbp, WRONG_VALUE);
          break;
        }
      }
      testproc_good(pktp, group_vbp);
      break;

    case LEAF_sysUserPassw:

      { long length = EBufferUsed(VB_GET_STRING(group_vbp));

        if (!((length >= MINSIZE_sysUserPassw) &&
              (length <= MAXSIZE_sysUserPassw))) {
          testproc_error(pktp, group_vbp, WRONG_VALUE);
          break;
        }
      }

      testproc_good(pktp, group_vbp);
      break;

    default:
      testproc_error(pktp, group_vbp, GEN_ERR);
      return;
    }
  }
}

/* This routine reboots the VxWorks target. */
void snmpReboot ()
    {
    printf("\007\007SNMP System Reboot Command in progress\n");
    taskDelay (200);    /* Wait 200 ticks before Reboot */
    reboot (0);         /* Reboot System */
    }
 

void
  sysconfig_set(OIDC_T      lastmatch,
                int         compc,
                OIDC_T     *compl,
                SNMP_PKT_T *pktp,
                VB_T       *vbp)
{

  for ( ; vbp; vbp = vbp->vb_link) {

    switch (vbp->vb_ml.ml_last_match) {

    case LEAF_sysState:

      sysVars.sysState = VB_GET_INT32(vbp);           

      if (taskSpawn ((char*)NULL, 150, VX_NO_STACK_FILL, 1024,
                (FUNCPTR) snmpReboot, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
          {
          setproc_error (pktp, vbp, COMMIT_FAILED);
          return;
          }
      else
          {
          setproc_good(pktp, vbp);
          }
      break;

    case LEAF_sysUserName:

      /* Get data from VxWorks */
      remCurIdGet (sysVars.sysUserName, sysVars.sysUserPassw);

      (void) memcpy (sysVars.sysUserName, EBufferStart (VB_GET_STRING(vbp)),
                     EBufferUsed (VB_GET_STRING(vbp))) ;
      sysVars.sysUserName [EBufferUsed (VB_GET_STRING(vbp))] = '\0';

      /* Set data from VxWorks */
      remCurIdSet (sysVars.sysUserName, sysVars.sysUserPassw);

      setproc_good(pktp, vbp);
      break;

    case LEAF_sysUserPassw:

      /* Get data from VxWorks */
      remCurIdGet (sysVars.sysUserName, sysVars.sysUserPassw); 

      (void) memcpy (sysVars.sysUserPassw, EBufferStart (VB_GET_STRING(vbp)),
                     EBufferUsed (VB_GET_STRING(vbp)));

      sysVars.sysUserPassw [EBufferUsed (VB_GET_STRING(vbp))] = '\0';

      /* Set data from VxWorks */
      remCurIdSet (sysVars.sysUserName, sysVars.sysUserPassw);

      setproc_good(pktp, vbp);
      break;

    default:
      setproc_error(pktp, vbp, COMMIT_FAILED);
      return;
    }
  }
}
