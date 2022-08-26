#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define BUFFER_SIZE 1000
#define ALLOC_SIZE (100 * 1024 * 1024)

static char command[BUFFER_SIZE];
static char file_contents[BUFFER_SIZE];
static char overwrite_date[] = "HELLO";

int main(void)
{
    pid_t pid;

    pid = getpid();
    snprintf(command, BUFFER_SIZE, "cat /proc/%d/maps", pid);

    puts("*** memory map before mapping file ***");
    fflush(stdout);
    system(command);

    // ファイルディスクリプタ（ファイル識別子）
    int fd;
    fd = open("testfile", O_RDWR);
    if (fd == -1) {
        err(EXIT_FAILURE, "open() failed");
    }

    char *file_contents;
    file_contents = mmap(NULL, ALLOC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_contents == (void *) -1) {
        warn("mmap() failed");
        if (close(fd) == -1) {
            warn("close() failed");
            exit(EXIT_SUCCESS);
        }
    }

    puts("");
    printf("*** succeeded to map file: address = %p; size = %x ***\n", file_contents, ALLOC_SIZE);

    puts("");
    puts("*** memory map after mapping file ***");
    fflush(stdout);
    system(command);

    printf("*** file contents before overwrite mapped region: %s", file_contents);

    memcpy(file_contents, overwrite_date, strlen(overwrite_date));

    puts("");
    printf("*** overwritten mapped region with: %s\n", file_contents);

    if (munmap(file_contents, ALLOC_SIZE) == -1) {
        warn("munmap() failed");
    }
}