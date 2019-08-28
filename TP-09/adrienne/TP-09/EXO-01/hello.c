#include <sys/syscall.h>
#include <sys/signal.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
	syscall(335);
}
