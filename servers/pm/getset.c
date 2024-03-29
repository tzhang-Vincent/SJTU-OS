/* This file handles the 6 system calls that get and set uids and gids.
 * It also handles getpid(), setsid(), and getpgrp().  The code for each
 * one is so tiny that it hardly seemed worthwhile to make each a separate
 * function.
 */

#include "pm.h"
#include <minix/callnr.h>
#include <minix/endpoint.h>
#include <limits.h>
#include <minix/com.h>
#include <signal.h>
#include "mproc.h"
#include "param.h"

/*===========================================================================*
 *				do_get					     *
 *===========================================================================*/
PUBLIC int do_get()
{
/* Handle GETUID, GETGID, GETPID, GETPGRP.
 */

  register struct mproc *rmp = mp;
  int r, proc;
  int ngroups;

  switch(call_nr) {
  	case GETGROUPS:
  		ngroups = m_in.grp_no;
  		if (ngroups > NGROUPS_MAX || ngroups < 0)
  			return(EINVAL);

  		if (ngroups == 0) {
  			r = rmp->mp_ngroups;
  			break;
  		}

		if (ngroups < rmp->mp_ngroups) 
			/* Asking for less groups than available */
			return(EINVAL);
	

  		r = sys_datacopy(SELF, (vir_bytes) rmp->mp_sgroups, who_e,
  			(vir_bytes) m_in.groupsp, ngroups * sizeof(gid_t));

  		if (r != OK) 
  			return(r);
  		
  		r = rmp->mp_ngroups;
  		break;
	case GETUID:
		r = rmp->mp_realuid;
		rmp->mp_reply.reply_res2 = rmp->mp_effuid;
		break;

	case GETGID:
		r = rmp->mp_realgid;
		rmp->mp_reply.reply_res2 = rmp->mp_effgid;
		break;

	case MINIX_GETPID:
		r = mproc[who_p].mp_pid;
		rmp->mp_reply.reply_res2 = mproc[rmp->mp_parent].mp_pid;
		break;

	case GETPGRP:
		r = rmp->mp_procgrp;
		break;

	default:
		r = EINVAL;
		break;	
  }
  return(r);
}

/*===========================================================================*
 *				do_set					     *
 *===========================================================================*/
PUBLIC int do_set()
{
/* Handle SETUID, SETEUID, SETGID, SETEGID, SETSID. These calls have in common
 * that, if successful, they will be forwarded to VFS as well.
 */
  register struct mproc *rmp = mp;
  message m;
  int r, i;
  int ngroups;

  switch(call_nr) {
	case SETUID:
	case SETEUID:
		if (rmp->mp_realuid != (uid_t) m_in.usr_id && 
				rmp->mp_effuid != SUPER_USER)
			return(EPERM);
		if(call_nr == SETUID) rmp->mp_realuid = (uid_t) m_in.usr_id;
		rmp->mp_effuid = (uid_t) m_in.usr_id;

		m.m_type = PM_SETUID;
		m.PM_PROC = rmp->mp_endpoint;
		m.PM_EID = rmp->mp_effuid;
		m.PM_RID = rmp->mp_realuid;

		break;

	case SETGID:
	case SETEGID:
		if (rmp->mp_realgid != (gid_t) m_in.grp_id && 
				rmp->mp_effuid != SUPER_USER)
			return(EPERM);
		if(call_nr == SETGID) rmp->mp_realgid = (gid_t) m_in.grp_id;
		rmp->mp_effgid = (gid_t) m_in.grp_id;

		m.m_type = PM_SETGID;
		m.PM_PROC = rmp->mp_endpoint;
		m.PM_EID = rmp->mp_effgid;
		m.PM_RID = rmp->mp_realgid;

		break;
	case SETGROUPS:
		if (rmp->mp_effuid != SUPER_USER)
			return(EPERM);

		ngroups = m_in.grp_no;

		if (ngroups > NGROUPS_MAX || ngroups < 0) 
			return(EINVAL);

		if (m_in.groupsp == NULL) 
			return(EFAULT);

		r = sys_datacopy(who_e, (vir_bytes) m_in.groupsp, SELF,
			     (vir_bytes) rmp->mp_sgroups,
			     ngroups * sizeof(gid_t));
		if (r != OK) 
			return(r);

		for (i = ngroups; i < NGROUPS_MAX; i++)
			rmp->mp_sgroups[i] = 0;
		rmp->mp_ngroups = ngroups;

		m.m_type = PM_SETGROUPS;
		m.PM_PROC = rmp->mp_endpoint;
		m.PM_GROUP_NO = rmp->mp_ngroups;
		m.PM_GROUP_ADDR = rmp->mp_sgroups;

		break;
	case SETSID:
		if (rmp->mp_procgrp == rmp->mp_pid) return(EPERM);
		rmp->mp_procgrp = rmp->mp_pid;

		m.m_type = PM_SETSID;
		m.PM_PROC = rmp->mp_endpoint;

		break;

	default:
		return(EINVAL);
  }

  /* Send the request to FS */
  tell_fs(rmp, &m);

  /* Do not reply until FS has processed the request */
  return(SUSPEND);
}
