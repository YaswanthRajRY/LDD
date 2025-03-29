#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#define MAGIC_NO 'M'
#define IOCGET _IOR(MAGIC_NO, 1, int)
#define IOCSET _IOR(MAGIC_NO, 2, int)
#define IOCTOGGLE _IOR(MAGIC_NO, 3, int)

int main()
{
    int fd;
    int user_var = 10;
    int err = 0;

    fd = open("/dev/my_ioctl_device", O_RDWR);
    if (fd < 0)
    {
        perror("failed to open device");
        return EXIT_FAILURE;
    }
    printf("device opened\n");

    ioctl(fd, IOCGET, &user_var);
    printf("get value: %d\n", user_var);

    user_var = 9;

    ioctl(fd, IOCSET, &user_var);
    printf("set value: %d\n", user_var);

    ioctl(fd, IOCTOGGLE, &user_var);
    ioctl(fd, IOCGET, &user_var);
    printf("toggle value: %d\n", user_var);

    close(fd);
    printf("device closed\n");

    return EXIT_SUCCESS;
}