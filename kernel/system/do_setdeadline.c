/* The kernel call implemented in this file:
 *   m_type:	SYS_SETDDL
 *
 * The parameters for this kernel call are:
 *    m2_i1:	PR_ENDPT   	process number to change deadline
 *    m2_l1:	deadline
 */
#include "../system.h"
#include <sys/resource.h>

#if USE_SETDEADLINE
/*===========================================================================*
 *				  do_setdeadline				     *
 *===========================================================================*/
PUBLIC int do_setdeadline(message *m_ptr)
{
	rp = proc_addr(m_ptr->m2_i1);
	long deadline = m_ptr->m2_l1;
	RTS_LOCK_SET(rp, RTS_SYS_LOCK);
	rp->p_deadline.tmr_exp_time = m_ptr->m2_l1;
  	RTS_LOCK_UNSET(rp, RTS_SYS_LOCK);
	
	return(OK);
}

#endif