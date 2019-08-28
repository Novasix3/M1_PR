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
 
        fd = open("/dev/taskmonitor", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }

        int ret_val;
	char message[100];

	ret_val = ioctl(fd, GET_SAMPLE, message);

	if (ret_val < 0) {
		printf("taskmonitor_get_msg failed:%d\n", ret_val);
		exit(-1);
	}

	printf("%s\n", message);

	ret_val = ioctl(fd, TASKMON_STOP, message);

	if (ret_val < 0) {
		printf("taskmonitor_stop_thread failed:%d\n", ret_val);
		exit(-1);
	}

	//printf("%s\n", message);

	ret_val = ioctl(fd, TASKMON_STOP, message);

	if (ret_val < 0) {
		printf("taskmonitor_stop_thread failed:%d\n", ret_val);
		exit(-1);
	}

	//printf("%s\n", message);

	ret_val = ioctl(fd, TASKMON_START, message);

	if (ret_val < 0) {
		printf("taskmonitor_start_thread failed:%d\n", ret_val);
		exit(-1);
	}

	//printf("%s\n", message);

	ret_val = ioctl(fd, TASKMON_START, message);

	if (ret_val < 0) {
		printf("taskmonitor_start_thread failed:%d\n", ret_val);
		exit(-1);
	}

	int newpid = 3;

	ret_val = ioctl(fd, TASKMON_SET_PID, &newpid);

	if (ret_val < 0) {
		printf("taskmonitor_start_thread failed:%d\n", ret_val);
		exit(-1);
	}

	ret_val = ioctl(fd, GET_SAMPLE, message);

	if (ret_val < 0) {
		printf("taskmonitor_get_msg failed:%d\n", ret_val);
		exit(-1);
	}

	printf("%s\n", message);
 
        close(fd);
}