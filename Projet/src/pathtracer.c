/* 
 * Algorithme
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <mpi.h>
#include "path_tracing.h"
#include "debug.h"
#include "tasks.h"

#define TAG_DEMANDE 1
#define TAG_WORK 2
#define TAG_NBTASK 3
#define TAG_TASK 4
#define TAG_GPR 5
#define TAG_END 6

#define TAG_PIXELS 7
#define TAG_STEAL 8
#define TAG_ANSWER 9

#define ROOTPROC 0

/************************************************
* Name:		getArgument
*
* Description:	Get programme parameters from command line argument	
*
* Argument:	- int argc	: number of argument
*		- char **argv	: array of argument
*		- int *w	: width
*		- int *h	: height
*		- int *samples	: number of samples
************************************************/
void getArgument(int argc, char **argv, int *w, int *h, int *samples){
	if (argc != 4) 
		exit(1);

	*w = atoi(argv[1]);
	*h = atoi(argv[2]);
	*samples = atoi(argv[3]);
}


/************************************************
* Name:		nb_pixel_task
*
* Description:	Number of pixel computed in 1 second	
*
* Argument:	- int samples	: number of samples
*
* Return:	number of pixels 
************************************************/
int nb_pixel_task(int samples){
	int res;

	res = 200 * 320 / samples;
	return res > 0 ? res : 1;
}


/************************************************
* Name:		fine_task_repartition
*
* Description:	Give separate task to every proc	
*
* Argument:	- int my_rank	: rank of processu
*		- int nbproc	: number of processus
*		- job *job	: my job
*		- int w	: width
*		- int h	: height
*		- int nbPixels	: number of pixels per task
*
* Return:	number of task given 
************************************************/
int fine_task_repartition(int my_rank, int nbproc, job *job, int w, int h, int nbPixels){
	int i, nbTask, end;

	nbTask = 0;
	for ( i = my_rank * nbPixels ; i < w * h ; i += nbproc * nbPixels ) {
		end = i + nbPixels < w * h ? i + nbPixels : w * h;
		job_add_task(job, create_task(i, end - 1));
		nbTask++;
	}
	return nbTask;
}

/************************************************
* Name:		task_repartition
*
* Description:	Give separate task to every proc	
*
* Argument:	- int my_rank	: rank of processus
*		- job *job	: job to add task
*		- int w	: width of image
*		- int h : height
************************************************/
void task_repartition(int my_rank, int nbproc, job *job, int w, int h){
	int i;

	for ( i = my_rank ; i < h ; i += nbproc ) {
		job_add_task(job, create_task_line(i, w));
	}
}

/************************************************
* Name:		init_get_computed_child
*
* Description:	Prepare a buffer to receive computed pixels
* 		of child
*
* Argument:	- MPI_Request *requestChild	: request
*		- int child	: rank of child
*		- int level	: level in the tree
*		- int nbTotalTask	: total number of task (approximate)
*		- int w	: width
*		- int h	: height
*
* Return:	array receiving computed values 
************************************************/
int *init_get_computed_child(MPI_Request *requestChild, int child, int level, int nbTotalTask, int w, int h){
	int *sent;

	sent = malloc(sizeof(int) * 3 * w * h);
	if ( sent == NULL ){
		printf("Couldn't allocate buffer to receive child\n");
		return NULL;
	}
	
	MPI_Irecv(sent, 3 * w * h, MPI_INT, child, TAG_PIXELS, MPI_COMM_WORLD, requestChild);
	return sent;
}

/************************************************
* Name:		nb_to_steal
*
* Description:	Give the number of task to steal	
*
* Argument:	- proc_info *stealer	: stealer's info
*		- proc_info *victim	: victim's info
*
* Return:	number of task to steal 
************************************************/
int nb_to_steal(proc_info *stealer, proc_info *victim){
	int toSteal;

	if ( victim->nbAssigned < 3 || victim->nbAssigned + 1 < stealer->nbAssigned )
		return 0;
	toSteal = (victim->nbAssigned - stealer->nbAssigned) / 2;
	return toSteal < 5 ? toSteal : 5;
}

/************************************************
* Name:		proc_to_steal
*
* Description:	Get the number of processus to steal from	
*
* Argument:	- job *job	: my job
*		- int nbproc	: number of processus
*
* Return:	number of processus to steal, -1 if not found 
************************************************/
int proc_to_steal(job *job, int nbproc){
	int i, max, index, steal;
	proc_info *cpy;

	max = 0;
	index = -1;
	cpy = job->info;
	for ( i = 0 ; i < nbproc - 1 ; i++ ) {
		if(cpy->id == (nbproc-1))
			cpy = job->info - (job->info->id);
		else
			cpy++;

		steal = nb_to_steal(job->info, cpy);
		if( steal > max ){
			max = steal;
			index = cpy->id;
		}
	}
	return index;
}

/************************************************
* Name:		steal
*
* Description:	Steal task from an other processus	
*
* Argument:	- proc_info *infos	: processus infos
*		- job *job	: my job
*		- int nbproc	: number of processus
*		- int *answer	: address to put answer
*
* Return:	-1 if cannot steal, -2 if just stole, -3 if waiting, victim otherwise 
************************************************/
int steal(proc_info *infos, job *job, int nbproc, int *answer){
	static int isStealing = 0, isSent;
	static MPI_Request requestAnswer, requestAsk;
	static int *send;
	int victim, flagAnswer, flagAsk, answerSize;

	if ( isStealing != 1 ){
		victim = proc_to_steal(job, nbproc);
		if ( victim == -1 )
			return -1;
		send = proc_info_sendable(infos, nbproc);
		MPI_Isend(send, 2 * nbproc, MPI_INT, victim, TAG_STEAL, MPI_COMM_WORLD, &requestAsk);
		isSent = 0;
		isStealing = 1;
		answerSize = 2 * nbproc + 1 + 2 * 5;
		MPI_Irecv(answer, answerSize, MPI_INT, victim, TAG_ANSWER, 
				MPI_COMM_WORLD, &requestAnswer);
		return victim;
	} else {
		if ( ! isSent ) {
			MPI_Test(&requestAsk, &flagAsk, MPI_STATUS_IGNORE);
			if ( flagAsk ){
				free(send);
				isSent = 1;
			}
		}
		if ( isSent ){
			MPI_Test(&requestAnswer, &flagAnswer, MPI_STATUS_IGNORE);
			if ( flagAnswer ){
				receive_tasks(infos, job, answer, nbproc);
				isStealing = 0;
				return -2;
			} 
		}
		return -3;
	}
}


/************************************************
* Name:		isVictim
*
* Description:	Check if someone want to steal you	
*
* Argument:	- proc_info *info	: processus infos
*		- job *job	: my job
*		- int nbproc	: number of processus
*		- int *ask	: address to store stealing demande
*
* Return:	1 if someone asked, 0 otherwise
************************************************/
int isVictim(proc_info *info, job *job, int nbproc, int *ask){
	static int isAvailable = 0, waitSend = 0;
	static int *send;
	static MPI_Request requestSteal, requestAnswer;
	int flagSteal, flagAnswer, toSteal, sendSize;
	MPI_Status status;
	
	if ( isAvailable != 1 ){
		MPI_Irecv(ask, 2 * nbproc, MPI_INT, MPI_ANY_SOURCE, TAG_STEAL, 
				MPI_COMM_WORLD, &requestSteal);
		isAvailable = 1;
	}
	MPI_Test(&requestSteal, &flagSteal, &status);
	if ( flagSteal && !waitSend ) {
		proc_info_update(info, ask, nbproc);
		toSteal = nb_to_steal(info + status.MPI_SOURCE, job->info);
		send = sendable_tasks(info, job, nbproc, toSteal);
		sendSize = 2 * nbproc + 1 + 2 * toSteal;
		MPI_Isend(send, sendSize, MPI_INT, status.MPI_SOURCE, TAG_ANSWER, MPI_COMM_WORLD, &requestAnswer);
		waitSend = 1;
	}
	if ( waitSend ){
		MPI_Test(&requestAnswer, &flagAnswer, MPI_STATUS_IGNORE);
		if ( flagAnswer ) {
			waitSend = 0;
			free(send);
			isAvailable = 0;
			return 1;
		} 
	}
	return 0;
}

int main(int argc, char **argv)
{ 
	/* Les variables sont initialiser dans le Makefile */
	int w, h, samples;

	/* Communication environnement variables */
	int nbproc, my_rank;

	/* Time variables */
	double computed_time = 0.0, tmp;
	int task_done = 0;

	/* Work variables */
	int nbTotalTask, nbTaskPerProc, nbMaxTaskPerProc, nbPixelTask;
	proc_info *infos;
	job *my_job;

	/* Stealing variables */
	int stealStatus, victim, *answerSteal, *ask;

	/* End variables */
	pixels_int *comp;
	int nbMaxPixelsReceive;
	image *image;
	MPI_Request requestChild0, requestChild1, requestEndSend, requestEnd;
	int *computedChild0, *computedChild1;
	int flagChild0, flagChild1, flagEnd;
	int *endSend, endtmp;

	getArgument(argc, argv, &w, &h, &samples);
	initRayVariables(w, h);
	initRayVariables_avx(w, h);

	/* MPI Initialisation */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nbproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


	/* Work initialisation */
	start_log(my_rank);
	nbTotalTask = w * h / nb_pixel_task(samples) + 1;
	nbTaskPerProc = nbTotalTask / nbproc + 1;
	nbMaxTaskPerProc = nbproc > 2 ? 3 * nbTaskPerProc : nbTotalTask;
	nbPixelTask = w * h / nbTotalTask + 1;
	infos = create_proc_info(nbproc, nbTaskPerProc);
	my_job = create_job(nbTaskPerProc, nbMaxTaskPerProc, nbMaxTaskPerProc * nbPixelTask, infos + my_rank);

	print_debug_int("There are %d processus", nbproc, MINIMAL_DEBUG);
	print_debug_int("Your are rank %d", my_rank, MINIMAL_DEBUG);
	print_debug_int("There are %d tasks", nbTotalTask, MINIMAL_DEBUG);
	print_debug_int("There are %d tasks per processus", nbTaskPerProc, MINIMAL_DEBUG);

	if ( infos == NULL || my_job == NULL ){
		printf("Init problem stop execution\n");
		return -1;
	}
	fine_task_repartition(my_rank, nbproc, my_job, w, h, nbPixelTask);
	print_debug_proc_info_all("After initialisation", infos, nbproc, PARTIAL_DEBUG);
	print_debug_job("After initialisation", my_job, PARTIAL_DEBUG);
	printf("#%d start\n", my_rank);
	write_log();

	/* Allocating memory for the end */
	if ( my_job->info->child[0] != -1 ) {
		computedChild0 = init_get_computed_child(&requestChild0, 
				my_job->info->child[0], 0, nbTotalTask, w, h);
		print_debug_int("is ready to receive from %d ", 
				my_job->info->child[0], MINIMAL_DEBUG);
		if ( my_job->info->child[1] != -1 ) {
			computedChild1 = init_get_computed_child(&requestChild1, 
					my_job->info->child[1], 0, nbTotalTask, w, h);
			print_debug_int("is ready to receive from %d ", 
					my_job->info->child[1], MINIMAL_DEBUG);
		} else {
			computedChild1 = NULL;
			print_debug_int("doesn't have a second child", 
					my_job->info->child[1], MINIMAL_DEBUG);
		}
	} else {
		print_debug_int("doesn't have any child", my_job->info->child[0], 
				MINIMAL_DEBUG);
		computedChild0 = NULL;
		computedChild1 = NULL;
	}
	write_log();

	stealStatus = 0;
	answerSteal = malloc(sizeof(int) * ( 2 * nbproc + 1 + 2 * 5));
	ask = malloc(sizeof(int) * 2 * nbproc);
	/* Computing time */
	while ( my_job->nbAssigned > 0 || stealStatus == 1 ) {
		if ( my_job->nbAssigned > 0 ){
			print_debug_job("avant taches", my_job, PARTIAL_DEBUG);
#ifdef SOLO_MPI
			tmp = diff_time();
			job_do_last(my_job, w, h, samples);
			computed_time += diff_time() - tmp;
			task_done++;
#endif
#ifdef HYBRIDE
			tmp = diff_time();
			job_do_last_mt(my_job, w, h, samples);
			computed_time += diff_time() - tmp;
			task_done++;
#endif
#ifdef MPI_SIMD
			tmp = diff_time();
			job_do_last_simd(my_job, w, h, samples);
			computed_time += diff_time() - tmp;
			task_done++;
#endif
#ifdef MPI_AVX
			tmp = diff_time();
			job_do_last_avx(my_job, w, h, samples);
			computed_time += diff_time() - tmp;
			task_done++;
#endif
			print_debug_job("affichage apres taches", my_job, PARTIAL_DEBUG);
		}
		while ( isVictim(infos, my_job, nbproc, ask) == 1 ) {
			print_debug_proc_info_all("victim", infos, nbproc, PARTIAL_DEBUG);
		}
		if ( my_job->nbAssigned < nbproc && stealStatus != -1) {
			//printf("#%d vol\n", my_rank);
			victim = steal(infos, my_job, nbproc, answerSteal);
			if ( victim == -1 ){
				print_debug_proc_info_all("Cannot steal", 
						infos, nbproc, PARTIAL_DEBUG);
				stealStatus = -1;
			}else if (victim == -2){
				print_debug_proc_info_all("Just stole someone", 
						infos, nbproc, PARTIAL_DEBUG);
			}else if (victim == -3){
				print_debug_int("wait in answer", 27, MINIMAL_DEBUG);
				stealStatus = 1;
			}else{
				print_debug_int("vol %d", victim, MINIMAL_DEBUG);
			}
		}
		write_log();
	}
	while ( isVictim(infos, my_job, nbproc, ask) == 1 ) {
		print_debug_proc_info_all("too late victim", infos, nbproc, PARTIAL_DEBUG);
	}
	print_debug_job("arrete de faire des taches", my_job, PARTIAL_DEBUG);
	nbMaxPixelsReceive = my_job->info->level > 1 ? w * h >> (my_job->info->level - 1) : w * h;
	comp = job_compute(my_job, nbTotalTask, nbMaxPixelsReceive);
	print_debug_pixel_int("a calcule ses taches", comp, PARTIAL_DEBUG);

	printf("%f #%d arrete de recevoir des taches\n", diff_time(), my_rank);
	print_debug_proc_info_all("a la fin des receptions de nouvelles taches", infos, nbproc, PARTIAL_DEBUG);

	flagChild0 = (my_job->info->child[0] != -1)?0:1;
	flagChild1 = (my_job->info->child[1] != -1)?0:1;

	while ( ! (flagChild0 && flagChild1) ){

		if ( !flagChild0 ){
			MPI_Test(&requestChild0, &flagChild0, MPI_STATUS_IGNORE);
			if ( flagChild0 ){
				print_debug_int("%d", my_job->info->child[0], PARTIAL_DEBUG);
				print_debug_pixel_int("Avant de récupérer du fils gauche qui est", comp, PARTIAL_DEBUG);
				computed_add(comp, computedChild0);
				print_debug_pixel_int("Après avoir récupéré du fils gauche", comp, PARTIAL_DEBUG);
			}
		}

		if ( !flagChild1 ){
			MPI_Test(&requestChild1, &flagChild1, MPI_STATUS_IGNORE);
			if ( flagChild1 ){
				print_debug_int("%d", my_job->info->child[1], PARTIAL_DEBUG);
				print_debug_pixel_int("Avant de récupérer du fils droit qui est", comp, PARTIAL_DEBUG);
				computed_add(comp, computedChild1);
				print_debug_pixel_int("Après avoir récupéré du fils droit", comp, PARTIAL_DEBUG);
			}
		}

		while ( isVictim(infos, my_job, nbproc, ask) == 1 ) {
			print_debug_proc_info_all("while waiting child", infos, nbproc, PARTIAL_DEBUG);
		}
	}

	print_debug_pixel_int("contient toutes les valeurs", comp, PARTIAL_DEBUG);

	/* Send computed value to parent */
	if ( my_rank != ROOTPROC ){
		endSend = computed_sendable(comp);
		print_debug_int("va envoyer %d int", 
				computed_size_sendable(comp), MINIMAL_DEBUG);
		MPI_Isend(endSend, computed_size_sendable(comp), MPI_INT, my_job->info->parent, TAG_PIXELS, MPI_COMM_WORLD, &requestEndSend);
		flagEnd = 0;
		MPI_Ibcast(&endtmp, 1, MPI_INT, ROOTPROC, MPI_COMM_WORLD, &requestEnd);
		while ( !flagEnd ) {
			while ( isVictim(infos, my_job, nbproc, ask) == 1 ) {
				print_debug_proc_info_all("end victime", infos, nbproc, PARTIAL_DEBUG);
			}
			MPI_Test(&requestEnd, &flagEnd, MPI_STATUS_IGNORE);
		}
		free(endSend);
	} else {
		image = assemble_image(comp, w, h);
		store_image(image);
		free_image(image);
		MPI_Ibcast(&endtmp, 1, MPI_INT, ROOTPROC, MPI_COMM_WORLD, &requestEnd);
	}

	print_debug_pixel_int("fin", comp, MINIMAL_DEBUG);

	free_job(my_job);
	free_pixels(comp);
	free_proc_info(infos);
	free(ask);
	free(answerSteal);

	printf("%f #%d fini, time : %f / %d tasks\n", diff_time(), my_rank, computed_time, task_done);

	close_log();
	
	MPI_Finalize();
}
