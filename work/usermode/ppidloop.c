#include <unistd.h>

int main(void)
{
    while (1) {
        getppid();
    }
    return 0;
}