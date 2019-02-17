#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#define SIZE_HN 50

int main(int argc, char* argv[]){
	int my_rank, p, source, dest, tag = 0;
	char message[100];
	char hostname[SIZE_HN];
	MPI_Status status;

	gethostname(hostname, SIZE_HN);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	sprintf(message, "coucou du proc #%d depuis %s !", my_rank, hostname);
	if(my_rank == 0){
		MPI_Ssend(message, strlen(message)+1, MPI_CHAR, (my_rank + 1)%p, tag, MPI_COMM_WORLD);
		printf("sur %s, le proc #%d a envoyÃ© le message : \"%s\"\n", hostname, my_rank, message);
	}else if(my_rank == p-1){
		MPI_Recv(message, 100, MPI_CHAR, (my_rank - 1 + p)%p, tag, MPI_COMM_WORLD, &status);
		printf("sur %s, le proc #%d a recu le message : \"%s\"\n", hostname, my_rank, message);
	}else{
		MPI_Recv(message, 100, MPI_CHAR, (my_rank - 1 + p)%p, tag, MPI_COMM_WORLD, &status);
		printf("sur %s, le proc #%d a recu le message : \"%s\"\n", hostname, my_rank, message);
		MPI_Ssend(message, strlen(message)+1, MPI_CHAR, (my_rank + 1)%p, tag, MPI_COMM_WORLD);
		printf("sur %s, le proc #%d a envoyÃ© le message : \"%s\"\n", hostname, my_rank, message);
	}

	MPI_Finalize();
}