#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commit.h"

int main(int argc, char const *argv[]){
	struct version v = {.major = 3, .minor = 5, .flags = 0};
	char* com = (char*) malloc(20);
	strcpy(com, "hello worlddddddddd");

	printf("%lu\n", sizeof(com));

	struct commit c = {.id = 0, .version = v, .comment = com, .next = NULL, .prev = NULL};

	printf("c = %p\n", &c);
	printf("c.id = %p\n", &c.id);
	printf("c.version = %p\n", &c.version);
	printf("c.comment = %p\n", &c.comment);
	printf("c.next = %p\n", &c.next);
	printf("c.prev = %p\n", &c.prev);

	struct commit* c2 = commitOf(&v, &c);

    printf("c5 = %p\n", c2);
    printf("c.id = %p\n", &c2->id);
    printf("c.version = %p\n", &c2->version);
    printf("c.comment = %p\n", &c2->comment);
    printf("c.next = %p\n", &c2->next);
    printf("c.prev = %p\n", &c2->prev);

	return 0;
}
