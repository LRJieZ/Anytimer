#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "anytimer.h"

enum{
    JOB_RUNNING = 1,
    JOB_CANCELED,
    JOB_OVER
};

typedef struct{
    int job_state;
    int sec;
    int job_remain;
    at_job_func job_func;
    void *arg;
}at_job_st, *at_job_stp;

static int inited = 0;
static struct sigaction sa_saved;
static at_job_stp at_job_queue[JOBMAX];

static void alrm_action(int n, siginfo_t *infop, void *arg)
{
    int i;
    if(infop->si_code != SI_KERNEL)
        return ;

    write(1, ".", 1);
    
    for(i = 0; i < JOBMAX; i++)
    {
        if(at_job_queue[i] != NULL && (at_job_queue[i]->job_state == JOB_RUNNING))
        {
            at_job_queue[i]->job_remain --;
            if(at_job_queue[i]->job_remain == 0)
            {
                at_job_queue[i]->job_func(at_job_queue[i]->arg);
                at_job_queue[i]->job_state = JOB_OVER;
            }
        }
    }
}

static void module_unload()
{
    struct sigaction sa;
    struct itimerval itv;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 0;
    setitimer(SIGALRM, &itv, NULL);

    sigaction(SIGALRM, &sa_saved, NULL);
}

//挂载定时器，并定义ALRM信号
static void module_load()
{
    struct sigaction sa;
    struct itimerval itv;

    sa.sa_sigaction = alrm_action;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGALRM, &sa, &sa_saved);

    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, NULL);

    atexit(module_unload);
}

static int get_free_pos()
{
    int id;
    int i;
    for(i = 0; i < JOBMAX; i++)
    {
        if(at_job_queue[i] == NULL)
            return i;
    }
    return -ENOSPC;
}

int at_addjob(int sec, at_job_func jobp, void *arg)
{
    if(!inited)
    {
        module_load();  //挂载定时器
        inited = 1;
    }
    
    int id = get_free_pos();
    if(id < 0)
    {
        perror("get_free_pos()");
        return -ENOSPC;
    }

    at_job_stp me = malloc(sizeof(at_job_st));
    if(me == NULL)
    {
        perror("malloc()");
        return -ENOMEM;
    }

    me->job_state = JOB_RUNNING;
    me->sec = sec;
    me->job_remain = sec;
    me->job_func = jobp;
    me->arg = arg;

    

    at_job_queue[id] = me;

    return id;
}
int at_canceljob(int id)
{
    if(id < 0 || id >JOBMAX || at_job_queue[id] == NULL)
    {
        fprintf(stdout, "invalid <id>\n");
        return -ENODEV;
    }

    if(at_job_queue[id]->job_state == JOB_CANCELED)
    {
        fprintf(stdout, "The job was cancelen\n");
        return -ECANCELED;
    }

    if(at_job_queue[id]->job_state == JOB_OVER)
    {
        fprintf(stdout, "The job was finished\n");
        return -EBUSY;
    }

    at_job_queue[id]->job_state = JOB_CANCELED;


    return 0;
}

int at_waitjob(int id)
{
    if(id < 0 || id >JOBMAX || at_job_queue[id] == NULL)
    {
        fprintf(stdout, "invalid <id>\n");
        return -ENODEV;
    }

    if(at_job_queue[id]->job_state == JOB_RUNNING)
    {
        pause();
    }

    if((at_job_queue[id]->job_state == JOB_CANCELED) || (at_job_queue[id]->job_state == JOB_OVER))
    {
        free(at_job_queue[id]);
        at_job_queue[id] = NULL;
    }

    return 0;
}

