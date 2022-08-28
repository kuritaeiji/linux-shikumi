#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

// セクションサイズ（ファイルシステムのサイズ）
#define PART_SIZE (1024 * 1024 * 1024)
// アクセスする合計サイズ
#define ACCESS_SIZE (64 * 1024 * 1024)

int main(int argc, char* argv[]) {
    progname = argv[0];
    char* filename = argv[1];

    bool help;
    if (!strcmp(argv[2], "on")) {
        help = true;
    } else {
        help = false;
    }

    bool write_flag;
    if (!strcmp(argv[3], "r")) {
        write_flag = false;
    } else {
        write_flag = true;
    }

    bool random;
    if (!strcmp(argv[4], "seq")) {
        random = false;
    } else {
        random = true;
    }

    int part_size = PART_SIZE;
    int access_size = ACCESS_SIZE;

    // 1回でアクセスするサイズ
    int block_size = atoi(argv[5]) * 1024;

    // セクションでアクセスできる最大の回数
    int maxcount = part_size / block_size;
    // 実際にアクセスする回数
    int count = access_size / block_size;

    int flag = O_RDWR | O_EXCL;
    if (!help) {
        flag |= O_DIRECTORY;
    }

    int fd;
    fd = open(filename, flag);

    int* offset = malloc(sizeof(int) * maxcount);
    int i;
    for (i = 0; i < maxcount; i++) {
        offset[i] = i;
    }
    if (random) {
        for (i = 0; i < maxcount; i++) {
            int j = rand() % maxcount;
            int tmp = offset[i];
            offset[i] = offset[j];
            offset[j] = tmp;
        }
    }

    int sector_size;
    ioctl(fd, BLKSSXGET, &sector_size);

    char* buf;
    int e;
    e = posix_memalign((void**)&buf, sector_size, blocksize);

    for (i = 0; i < count; i++) {
        ssize_t ret;
        lseek(fd, offset[i] * block_size, SEEK_SET);
        if (write_flag) {
            ret = write(fd, buf, block_size);
        } else {
            ret = read(fd,buf, block_size);
        }
    }
}