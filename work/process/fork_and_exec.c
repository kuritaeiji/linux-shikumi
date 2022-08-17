#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

static void child()
{
    // charのポインタの配列 = stringの配列
    char* args[] = {"/bin/echo", "Hello", NULL};
    printf("I'm child! my pid is %d\n", getpid());
    fflush(stdout);
    execve("/bin/echo", args, NULL);
    // execveは現在のプロセスを新たなプロセスで置き換える（メモリ上の話）
    // この場合/bin/echoプログラムをメモリにコピーしている
    err(EXIT_FAILURE, "exec() failure");
    // ここに辿り着いたということは/bin/echoプログラムのプロセス生成ができなかったということ
}

static void parent(pid_t pid_c)
{
    printf("I'm child! my pid id %d and the pid of my child is %d.\n", getpid(), pid_c);
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