#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
 
#include "request.h"
 
int main()
{
        int fd;
 
        fd = open("/dev/hello", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }

        int ret_val;
	char message[100];

	ret_val = ioctl(fd, HELLO, message);

	if (ret_val < 0) {
		printf("ioctl_get_msg failed:%d\n", ret_val);
		exit(-1);
	}

	printf("%s\n", message);

	strcpy(message, "beer");

	ret_val = ioctl(fd, WHO, message);

	if (ret_val < 0) {
		printf("ioctl_set_msg failed:%d\n", ret_val);
		exit(-1);
	}

	ret_val = ioctl(fd, HELLO, message);

	if (ret_val < 0) {
		printf("ioctl_get_msg failed:%d\n", ret_val);
		exit(-1);
	}

	printf("%s\n", message);
 
        close(fd);
}