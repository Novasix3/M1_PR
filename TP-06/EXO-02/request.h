#ifndef __HEADER_REQUEST__
#define __HEADER_REQUEST__

#include <linux/ioctl.h>

#define HELLO _IOR('N', 0, char*)
#define WHO _IOW('N', 1, char*)

#endif