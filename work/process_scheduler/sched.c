#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define NLOOP_FOR_ESTIMATION 1000000000UL
#define NSECS_PER_MSEC 1000000UL
#define NSECS_PER_SEC 1000000000UL

static inline unsigned long diff_nsec(struct timespec before, struct timespec after)
{
        return ((after.tv_sec * NSECS_PER_SEC + after.tv_nsec)
                - (before.tv_sec * NSECS_PER_SEC + before.tv_nsec));
	
}

static unsigned long loops_per_msec()
{
    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);
    unsigned long i;
    for (i = 0; i < NLOOP_FOR_ESTIMATION; i++) {
        ;
    }
    clock_gettime(CLOCK_MONOTONIC, &after);
    return NLOOP_FOR_ESTIMATION * NSECS_PER_MSEC / diff_nsec(before, after);
}

static inline void load(unsigned long nloop)
{
    unsigned long i;
    for (i = 0; i < nloop; i++) {
        ;
    }
}

static void child_fn(int id, struct timespec *buf, int nrecord, 
unsigned long nloop_per_resol, struct timespec start)
{
    int i;
    for (i = 0; i < nrecord; i++) {
        struct timespec ts;
        load(nloop_per_resol);
        clock_gettime(CLOCK_MONOTONIC, &ts);
        buf[i] = ts;
    }

    for (i = 0; i < nrecord; i++) {
        // プロセス番号(0,1,2...), 
        printf("%d\t%ld\t%d\n", id, diff_nsec(start, buf[i]) / NSECS_PER_MSEC, (i + 1) * 100 / nrecord);
    }

    exit(EXIT_SUCCESS);
}

// 子プロセス数だけプロセスが終了するまでwaitする
static void parent_fn(int nproc)
{
    int i;
    for (i = 0; i < nproc; i++) {
        wait(NULL);
    }
}

// 動的配列を作成するためのpidのポインタ
pid_t *pids;

int main(int argc, char* argv[])
{
    // linuxの終了ステータスコード(failureは1,successは0)
    int ret = EXIT_FAILURE;

    if (argc < 4) {
        fprintf(stderr, "usage: %s <nproc> <total[ms]> <resolution[ms]>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nproc = atoi(argv[1]);
    int total = atoi(argv[2]);
    int resol = atoi(argv[3]);

    if (nproc < 1) {
        fprintf(stderr, "<nproc> (%d) should be >= 1\n", nproc);
        exit(EXIT_FAILURE);
    }

    if (total < 1) {
        fprintf(stderr, "<total> (%d) should be >= 1\n", total);
        exit(EXIT_FAILURE);
    }

    if (resol < 1) {
        fprintf(stderr, "<resol> (%d) should be >= 1\n", resol);
        exit(EXIT_FAILURE);
    }

    if (total % resol) {
        fprintf(stderr, "<total> (%d) should be multiple of <resolution> (%d)\n", total, resol);
        exit(EXIT_FAILURE);
    }

    // ループを繰り返す回数
    int nrecord = total / resol;

    // ループ分の時刻記録を入れられる動的配列を作成する
    struct timespec *logbuf = malloc(sizeof(struct timespec) * nrecord);

    if (!logbuf) {
        err(EXIT_FAILURE, "malloc(logbuf) failed");
    }

    puts("estimating workload which takes just one milisecond");
    // 1回のresolutionループの間に何回のループをする必要があるか
    unsigned long nloop_per_resol = loops_per_msec() * resol;
    puts("end estimation");
    fflush(stdout);

    // プロセス分だけ動的配列を作成する
    pids = malloc(sizeof(pid_t) * nproc);
    if (pids == NULL) {
        warn("malloc(pids) failed");
        free(logbuf);
        exit(ret);
    }

    // 開始時刻を記録する
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int i, ncreated;
    for (i = 0, ncreated = 0; i < nproc; i++, ncreated++) {
        pids[i] = fork();
        // pids[i] == -1の場合子プロセスの生成に失敗。
        // プロセスの生成に失敗した場合は全ての子プロセスをkillする
        if (pids[i] < 0) {
            int j;
            if (ret == EXIT_FAILURE) {
                for (j = 0; j < ncreated; j++) {
                    // killに失敗した場合-1が返る
                    if (kill(pids[i], SIGINT)) {
                        warn("kill (%d) failed", pids[i]);
                    }
                }
            }

            for (j = 0; j < ncreated; j++) {
                if (wait(NULL) < 0) {
                    warn("wait() failed");
                }
            }
        }

        // 子プロセスの場合
        if (pids[i] == 0) {
            child_fn(i, logbuf, nrecord, nloop_per_resol, start);
        }
    }

    ret = EXIT_SUCCESS;
    exit(ret);
}