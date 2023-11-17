#ifndef __ANYTIMER_H
#define __ANYTIMER_H

typedef void (*at_job_func)(void *arg);

int at_addjob(int sec, at_job_func jobp, void *arg);
int at_canceljob(int id);
int at_waitjob(int id);
 
#endif
