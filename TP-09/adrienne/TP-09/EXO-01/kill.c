#include <sys/syscall.h>
#include <sys/signal.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
	int fid = 0;
	if (fid = fork()) {
		sleep(4);
		syscall(__NR_kill, fid, SIGKILL);
	} else {
		printf("FILS DEBUT\n");
		sleep(10);
		printf("FILS Au revoir\n");
	}
}
