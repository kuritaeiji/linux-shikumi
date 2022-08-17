#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

static void child()
{
    printf("I'm child! my pid is %d.\n", getpid());
}

static void parent(pid_t pid_c)
{
    printf("I'm parent! my pid is %d and the pid of my child is %d.\n", getpid(), pid_c);
}

int main(void)
{
    pid_t ret;
    ret = fork();

    if (ret == -1) {
        err(EXIT_FAILURE, "fork() failed");
    }
    if (ret == 0) {
        child();
    } else {
        parent(ret);
    }

    exit(EXIT_SUCCESS);
}