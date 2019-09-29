/*
setddl.c
*/

#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>
#include <kernel/clock.h>

int setddl(deadline)
long deadline;
{
	message m;
  	memset(&m, 0, sizeof(m));
  	clock_t t = get_uptime() + deadline;
  	m->m2_l1 = t;
	return  (_syscall(MM, SETDDL, &m));
}
