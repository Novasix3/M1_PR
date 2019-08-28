#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
 
int main(int argc, char** argv)
{
	char *name = "yoyo";
	char *buffer = malloc(200);
        if( !syscall(335, name, 4, buffer, 200) ) {
        	printf("success \"%s\"\n", buffer);
        }else{
        	printf("failed\n");
        }

        return 0;
}