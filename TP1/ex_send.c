#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#define SIZE_HN 50
#define SIZE_MESSAGE 4500

int main(int argc, char* argv[]){
	int my_rank, p, source, dest, tag = 0;
	char message[SIZE_MESSAGE];
	char hostname[SIZE_HN];
	MPI_Status status;

	gethostname(hostname, SIZE_HN);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


	sprintf(message, "coucou du proc #%d depuis %s !", my_rank, hostname);
	int i;
	for(i = 0; i < SIZE_MESSAGE-2; ++i)
		*(message+i) = 'e';
	*(message+i) = '\0';
	printf("original message = %s\n", message);
	MPI_Send(message, strlen(message)+1, MPI_CHAR, (my_rank + 1)%p, tag, MPI_COMM_WORLD);
	MPI_Recv(message, strlen(message)+1, MPI_CHAR, (my_rank - 1 + p)%p, tag, MPI_COMM_WORLD, &status);
	printf("sur %s, le proc #%d a recu le message : \"%s\"\n", hostname, my_rank, message);

	MPI_Finalize();
}