#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "anytimer.h"

void f1(void *s)
{
    printf("%s\n", s);
}

void f2(void *s)
{
    printf("%s\n", s);
}


void f3(void *s)
{
    printf("%s\n", s);
}


int main(){
    int id = at_addjob(5, f1, "f1()");
    if(id < 0 || id >= JOBMAX)
    {
        perror("at_addjob(f1)");
        return -ENOSPC;
    }

    id = at_addjob(2, f2, "f2()");
    if(id < 0 || id >= JOBMAX)
    {
        perror("at_addjob(f2)");
        return -ENOSPC;
    }

    id = at_addjob(7, f3, "f3()");
    if(id < 0 || id >= JOBMAX)
    {
        perror("at_addjob(f3)");
        return -ENOSPC;
    }


    while(1);

    exit(1);
}
