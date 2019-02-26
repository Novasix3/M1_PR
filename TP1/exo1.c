#include <stdio.h>
#include <mpi.h>

int main(int argc, char* argv[]){
	int rang, p, valeur, tag = 10;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);

	if(rang){
		valeur = 18;
		MPI_Send(&valeur, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
	}else{
		MPI_Recv(&valeur, 1, MPI_INT, 1, tag, MPI_COMM_WORLD, &status);
		printf("received %d from 1.\n", valeur);
	}

	MPI_Finalize();
}