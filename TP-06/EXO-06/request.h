#ifndef __HEADER_REQUEST__
#define __HEADER_REQUEST__

#include <linux/ioctl.h>

#define GET_SAMPLE _IOR('N', 0, char*)
#define TASKMON_STOP _IOR('N', 1, void*)
#define TASKMON_START _IOR('N', 2, void*)
#define TASKMON_SET_PID _IOR('N', 3, int*)

#endif