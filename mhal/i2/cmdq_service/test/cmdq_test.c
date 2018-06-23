#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "drvcmdq_io.h"

#define DEV_NAME "/dev/cmdq"

int main(int argc, char** argv)
{
    int testarg ;
    int nCmdqFd = -1;

    if(argc < 2)
    {
        printf("test argc need 1 parameter\n");
        return 0;
    }

    if(0 > (nCmdqFd = open(DEV_NAME, O_RDWR | O_SYNC)))
    {
        printf("%s open /dev/drvcmdq failed!!\n", __FUNCTION__);
        return 0;
    }
#if 1
    testarg = atoi(argv[1]);

    if(ioctl(nCmdqFd, IOCTL_CMDQ_TEST_1, &testarg))
    {
        printf("fail test (%d) fail!!\n", testarg);

    }
#endif
    close(nCmdqFd);
    return 0;
}
