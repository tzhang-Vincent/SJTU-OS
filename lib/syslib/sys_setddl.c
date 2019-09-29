#include "syslib.h"
PUBLIC int sys_setddl(endpoint_t proc_ep, long deadline)
{
  message m;

  m.m2_i1 = proc_ep;
  m.m2_l1 = deadline;
  return(_taskcall(SYSTASK, SYS_SETDDL, &m));
}