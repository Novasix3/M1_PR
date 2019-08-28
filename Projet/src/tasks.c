#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#include "path_tracing.h"
#include "tasks.h"
#include "debug.h"


/************************************************
* Name:		create_task
*
* Description:	Create a basic task	
*
* Argument:	- int beg	: indice of first pixel
*		- int end	: indice of last pixel
*
* Return:	task created 
************************************************/
task create_task(int beg, int end){
	task res;
	res.beg = beg;
	res.end = end;
	res.computed = NULL;
	return res;
}

/************************************************
* Name:		create_task_line
*
* Description:	Create a line task	
*
* Argument:	- int number	: indice of line
*		- int w	: width of picture
*
* Return:	task created 
************************************************/
task create_task_line(int number, int w){
	return create_task(number * w, (number + 1) * w - 1);
}

/************************************************
* Name:		do_task
*
* Description:	Compute the task selected
*
* Argument:	- task *task	: task to compute
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed 
************************************************/
int do_task(task *task, int w, int h, int samples){
	int i;

	for ( i = task->beg ; i < task->end + 1 ; i++ ){
		computePixel(w, h, samples, i / w, i % w, task->computed + (i - task->beg));
	}
	
	return task->end - task->beg + 1;
}

/************************************************
* Name:		do_task_mt
*
* Description:	Compute the task selected with multiple threads
*
* Argument:	- task *task	: task to compute
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed 
************************************************/
int do_task_mt(task *task, int w, int h, int samples){
	int i;

	/*
	Mettre du omp ici est le mieux selon moi car calculer un pixel est rapide, mais calculer une ligne
	est long en fonction du nombre de pixels, donc c'est plus optimal selon moi de multi thread ici.
	*/
	#pragma omp parallel for schedule(dynamic) num_threads(4)
	for ( i = task->beg ; i < task->end + 1 ; i++ ){
		computePixel_mt(w, h, samples, i / w, i % w, task->computed + (i - task->beg));
	}
	return task->end - task->beg + 1;
}

/************************************************
* Name:		do_task_simd
*
* Description:	Compute the task selected in simd
*
* Argument:	- task *task	: task to compute
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed 
************************************************/
int do_task_simd(task *task, int w, int h, int samples){
	int i;

	#pragma omp for simd
	for ( i = task->beg ; i < task->end + 1 ; i++ ){
		computePixel_simd(w, h, samples, i / w, i % w, task->computed + (i - task->beg));
	}
	return task->end - task->beg + 1;
}

/************************************************
* Name:		do_task_avx
*
* Description:	Compute the task selected in avx
*
* Argument:	- task *task	: task to compute
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed 
************************************************/
int do_task_avx(task *task, int w, int h, int samples){
	int i;

	for ( i = task->beg ; i < task->end + 1 ; i++ ){
		computePixel_avx(w, h, samples, i / w, i % w, task->computed + (i - task->beg));
	}
	return task->end - task->beg + 1;
}

/************************************************
* Name:		do_task_all
*
* Description:	Compute the task selected with the best
*
* Argument:	- task *task	: task to compute
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed 
************************************************/
int do_task_all(task *task, int w, int h, int samples){
	int i;

	#pragma omp parallel for schedule(dynamic) num_threads(4)
	for ( i = task->beg ; i < task->end + 1 ; i++ ){
		computePixel_avx(w, h, samples, i / w, i % w, task->computed + (i - task->beg));
	}
	return task->end - task->beg + 1;
}

/************************************************
* Name:		create_proc_info
*
* Description:	Create the proc info structure
*
* Argument:	- int nbproc	: number of processus
*		- int nbTask	: number of task per processus
*
* Return:	structure proc_info created 
************************************************/
proc_info *create_proc_info(int nbproc, int nbTask){
	proc_info *res;
	int i, parent;

	res = malloc(sizeof(proc_info) * nbproc);
	if ( res == NULL ){
		printf("Couldn't allocate proc_info\n");
		return res;
	}
	parent = -1;
	for ( i = 0 ; i < nbproc ; i++ ) {
		res[i].id = i;
		res[i].nbAssigned = nbTask;
		res[i].nbDone = 0;
		res[i].parent = parent;
		if ( i % 2 == 0 )
			parent++;
		if ( nbproc > 2 * i + 1 ) {
			res[i].child[0] = 2 * i + 1;
			if ( nbproc > 2 * i + 2 ) {
				res[i].child[1] = 2 * i + 2;
			} else {
				res[i].child[1] = -1;
			}
		} else {
			res[i].child[0] = -1;
			res[i].child[1] = -1;
		}
		if ( i == 0 ){
			res[0].level = 0;
		} else {
			res[i].level = res[res[i].parent].level + 1;
		}
	}
	return res;
}


/************************************************
* Name:		free_proc_info
*
* Description:	Free structure proc	
*
* Argument:	- proc_info *p	: structure to free
************************************************/
void free_proc_info(proc_info *p) {
	free(p);
}


/************************************************
* Name:		proc_info_fill
*
* Description:	Fill an array of int, with infos to send	
*
* Argument:	- int *send	: array to fill
* 		- proc_info *infos	: proc_info to send
*		- int nbproc	: number of processus
************************************************/
static void proc_info_fill(int *send, proc_info *infos, int nbproc){
	int i;

	for ( i = 0 ; i < nbproc ; i++ ) {
		send[2*i] = infos[i].nbAssigned;
		send[2*i+1] = infos[i].nbDone;
	}
}

/************************************************
* Name:		proc_info_sendable
*
* Description:	Create a int array to send	
*
* Argument:	- proc_info *info	: struct to send
*		- int nbproc	: number of processus
*
* Return:	sendable structure 
************************************************/
int *proc_info_sendable(proc_info *info, int nbproc){
	int *send;

	send = malloc(sizeof(int) * 2 * nbproc);
	if ( send == NULL ){
		printf("Couldn't allocate sendable proc_info\n");
		return send;
	}
	proc_info_fill(send, info, nbproc);
	return send;
}


/************************************************
* Name:		proc_info_update
*
* Description:	Update the proc_info structure	
*
* Argument:	- proc_info *info	: proc_info structure to update
*		- int *update	: update array
*		- int nbproc	: number of processus
*
* Return:	number of proc_info element updated 
************************************************/
int proc_info_update(proc_info *info, int *update, int nbproc){
	int i, nbUpdate;

	nbUpdate = 0;
	for ( i = 0 ; i < nbproc ; i++ ) {
		if ( info[i].nbDone < update[2*i+1] ){
			nbUpdate++;
			info[i].nbAssigned = update[2*i];
			info[i].nbDone = update[2*i+1];
		}
	}
	return nbUpdate;
}


/************************************************
* Name:		create_job
*
* Description:	Create a structure job	
*
* Argument:	- int maxAssigned	: max number of task to assign
*		- int maxDone	: max number of task done
*		- int maxComputed	: max number of pixel computed
*		- proc_info *my_info	: info of my processus
*
* Return:	processus created 
************************************************/
job *create_job(int maxAssigned, int maxDone, int maxComputed, proc_info *my_info){
	job *res;

	res = malloc(sizeof(job));
	if ( res == NULL ){
		printf("Couldn't create job struct\n");
		return NULL;
	}
	res->nbAssigned = 0;
	res->sizeMallocAssigned = maxAssigned;
	res->assigned = malloc(sizeof(task) * maxAssigned);
	if ( res->assigned == NULL ){
		printf("Couldn't create job assigned\n");
		goto err1;
	}
	res->nbDone = 0;
	res->sizeMallocDone = maxDone;
	res->done = malloc(sizeof(task) * maxDone);
	if ( res->done == NULL ){
		printf("Couldn't create job done\n");
		goto err2;
	}
	res->nbComputed = 0;
	res->sizeMallocComputed = maxComputed;
	res->computed = malloc(sizeof(struct picturePixel) * maxComputed);
	if ( res->computed == NULL ){
		printf("Couldn't create job computed\n");
		goto err3;
	}
	res->info = my_info;
	res->info->nbAssigned = 0;
	return res;
err3:
	free(res->done);
err2:
	free(res->assigned);
err1:
	free(res);
	return NULL;
}


/************************************************
* Name:		free_job
*
* Description:	Free structure job	
*
* Argument:	- job* steve	: structure to free
************************************************/
void free_job(job* steve) {
	free(steve->assigned);
	free(steve->done);
	free(steve->computed);
	free(steve);
}

/************************************************
* Name:		job_add_task
*
* Description:	Ajoute une tache au job	
*
* Argument:	- job *job	: job
*		- task task	: tache a ajouter
*
* Return:	0 si tache ajoute, 1 sinon
************************************************/
int job_add_task(job *job, task task){
	if ( job->nbAssigned == job->sizeMallocAssigned ){
		printf("Cannot add any more job\n");
		return 1;
	}
	job->assigned[job->nbAssigned] = task;
	job->nbAssigned++;
	job->info->nbAssigned++;
	return 0;
}


/************************************************
* Name:		job_do_last
*
* Description:	Do the last task assigned
*
* Argument:	- job *job	: job
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed, -1 if error
************************************************/
int job_do_last(job *job, int w, int h, int samples){
	task task;
	int nbComputed;

	if ( job->nbDone == job->sizeMallocDone ){
		printf("Already full in task done\n");
		return -1;
	}
	task = job->assigned[job->nbAssigned-1];
	if ( job->nbComputed + task.end - task.beg > job->sizeMallocComputed ) {
		printf("Full in computed value\n");
		return -1;
	}
	task.computed = job->computed + job->nbComputed;
	nbComputed = do_task(&task, w, h, samples);

	job->nbComputed += nbComputed;
	job->nbAssigned--;
	job->info->nbAssigned--;
	job->done[job->nbDone] = task;
	job->nbDone++;
	job->info->nbDone++;
	return nbComputed;
}

/************************************************
* Name:		job_do_last_mt
*
* Description:	Do the last task assigned with multiple threads
*
* Argument:	- job *job	: job
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed, -1 if error
************************************************/
int job_do_last_mt(job *job, int w, int h, int samples){
	task task;
	int nbComputed;

	if ( job->nbDone == job->sizeMallocDone ){
		printf("Already full in task done\n");
		return -1;
	}
	task = job->assigned[job->nbAssigned-1];
	if ( job->nbComputed + task.end - task.beg > job->sizeMallocComputed ) {
		printf("Full in computed value\n");
		return -1;
	}
	task.computed = job->computed + job->nbComputed;
	nbComputed = do_task_mt(&task, w, h, samples);

	job->nbComputed += nbComputed;
	job->nbAssigned--;
	job->info->nbAssigned--;
	job->done[job->nbDone] = task;
	job->nbDone++;
	job->info->nbDone++;
	return nbComputed;
}

/************************************************
* Name:		job_do_last_simd
*
* Description:	Do the last task assigned in simd
*
* Argument:	- job *job	: job
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed, -1 if error
************************************************/
int job_do_last_simd(job *job, int w, int h, int samples){
	task task;
	int nbComputed;

	if ( job->nbDone == job->sizeMallocDone ){
		printf("Already full in task done\n");
		return -1;
	}
	task = job->assigned[job->nbAssigned-1];
	if ( job->nbComputed + task.end - task.beg > job->sizeMallocComputed ) {
		printf("Full in computed value\n");
		return -1;
	}
	task.computed = job->computed + job->nbComputed;
	nbComputed = do_task_simd(&task, w, h, samples);

	job->nbComputed += nbComputed;
	job->nbAssigned--;
	job->info->nbAssigned--;
	job->done[job->nbDone] = task;
	job->nbDone++;
	job->info->nbDone++;
	return nbComputed;
}

/************************************************
* Name:		job_do_last_avx
*
* Description:	Do the last task assigned in avx
*
* Argument:	- job *job	: job
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed, -1 if error
************************************************/
int job_do_last_avx(job *job, int w, int h, int samples){
	task task;
	int nbComputed;

	if ( job->nbDone == job->sizeMallocDone ){
		printf("Already full in task done\n");
		return -1;
	}
	task = job->assigned[job->nbAssigned-1];
	if ( job->nbComputed + task.end - task.beg > job->sizeMallocComputed ) {
		printf("Full in computed value\n");
		return -1;
	}
	task.computed = job->computed + job->nbComputed;
	nbComputed = do_task_avx(&task, w, h, samples);

	job->nbComputed += nbComputed;
	job->nbAssigned--;
	job->info->nbAssigned--;
	job->done[job->nbDone] = task;
	job->nbDone++;
	job->info->nbDone++;
	return nbComputed;
}

/************************************************
* Name:		job_do_last_all
*
* Description:	Do the last task assigned
*
* Argument:	- job *job	: job
*		- int w	: width of image
*		- int h	: height of image
*		- int samples	: number of samples
*
* Return:	number of pixel computed, -1 if error
************************************************/
int job_do_last_all(job *job, int w, int h, int samples){
	task task;
	int nbComputed;

	if ( job->nbDone == job->sizeMallocDone ){
		printf("Already full in task done\n");
		return -1;
	}
	task = job->assigned[job->nbAssigned-1];
	if ( job->nbComputed + task.end - task.beg > job->sizeMallocComputed ) {
		printf("Full in computed value\n");
		return -1;
	}
	task.computed = job->computed + job->nbComputed;
	nbComputed = do_task_all(&task, w, h, samples);

	job->nbComputed += nbComputed;
	job->nbAssigned--;
	job->info->nbAssigned--;
	job->done[job->nbDone] = task;
	job->nbDone++;
	job->info->nbDone++;
	return nbComputed;
}


/************************************************
* Name:		sendable_tasks
*
* Description:	Create a sendable array to answer 
* 		task stealing
*
* Argument:	- proc_info *infos	: all proc_info
*		- job *job	: my job
*		- int nbproc	: number of processus
*		- int nbTask	: number of task
*
* Return:	array to send 
************************************************/
int *sendable_tasks(proc_info *infos, job *job, int nbproc, int nbTask){
	int *send, pos, i;

	send = malloc(sizeof(int) * ( 2 * nbproc + 1 + 2 * nbTask ));
	if (send == NULL){
		printf("Couldn't allocation to answer stealing\n");
		return NULL;
	}
	proc_info_fill(send, infos, nbproc);
	pos = 2 * nbproc;
	send[pos++] = nbTask;
	for ( i = 0 ; i < nbTask ; i++ ) {
		send[pos++] = job->assigned[job->nbAssigned - 1].beg;
		send[pos++] = job->assigned[job->nbAssigned - 1].end;
		job->nbAssigned--;
		job->info->nbAssigned--;
	}
	return send;
}

/************************************************
* Name:		receive_tasks
*
* Description:	Proceed an answer given 	
*
* Argument:	- proc_info *infos	: current proc_info
*		- job *job	: my job
*		- int *sent	: answer given
*		- int nbproc	: number of processus
*
* Return:	number of task added 
************************************************/
int receive_tasks(proc_info *infos, job *job, int *sent, int nbproc){
	int pos, i, nbTask;
	task tmptask;

	proc_info_update(infos, sent, nbproc);
	pos = 2 * nbproc;
	nbTask = sent[pos++];
	for ( i = 0 ; i < nbTask ; i++ ) {
		tmptask = create_task(sent[pos], sent[pos+1]);
		pos += 2;
		job_add_task(job, tmptask);
	}
	return nbTask;
}

/************************************************
* Name:		job_compute
*
* Description:	Transform all pixel compute of format double
* 		to int
*
* Argument:	- job *job	: job
*		- int maxDone	: size of malloc done
*		- int maxComputed	: size of malloc computed
*
* Return:	 Structure pixels_int created
************************************************/
pixels_int *job_compute(job *job, int maxDone, int maxComputed){
	pixels_int *res;
	int i;

	if ( job->nbAssigned > 0 ){
		printf("Il lui reste des taches a faire\n");
		return NULL;
	}

	res = malloc(sizeof(pixels_int));

	if ( job->nbDone > maxDone || job->nbComputed > maxComputed ) {
		printf("Cannot store all computed\n");
		return res;
	}

	res->nbDone = 0;
	res->sizeMallocDone = maxDone;
	res->done = malloc(sizeof(task_computed) * res->sizeMallocDone);

	for ( i = 0 ; i < job->nbDone ; i++ ) {
		res->done[i].beg = job->done[i].beg;
		res->done[i].end = job->done[i].end;
		res->nbDone++;
	}

	res->nbComputed = 0;
	res->sizeMallocComputed = maxComputed;
	res->computed = malloc(sizeof(int) * 3 * res->sizeMallocComputed);

	for ( i = 0 ; i < job->nbComputed ; i++ ) {
		res->computed[3*i] = toInt(job->computed[i].R);
		res->computed[3*i+1] = toInt(job->computed[i].G);
		res->computed[3*i+2] = toInt(job->computed[i].B);
		res->nbComputed++;
	}
	return res;
}


/************************************************
* Name:		free_pixels
*
* Description:	Free structure pixels_int	
*
* Argument:	- pixels_int *pixels	: structure to free
************************************************/
void free_pixels(pixels_int *pixels) {
	free(pixels->done);
	free(pixels->computed);
	free(pixels);
}


/************************************************
* Name:		computed_size_sendable
*
* Description:	Get the size of data send in number of int
*
* Argument:	- pixels_int *pixel	: pixels computed
*
* Return:	size of data 
************************************************/
int computed_size_sendable(pixels_int *pixel){
	int size;

	/* First part is the number of task */
	size = 1;
	/* Second part is the task list */
	size += 2 * pixel->nbDone;
	/* Third part is values of pixels */
	size += 3 * pixel->nbComputed;
	return size;
}

/************************************************
* Name:		computed_sendable
*
* Description:	Get an array of int contaning
* 		pixels computed
*
* Argument:	- pixels_int *pixels	: pixels computed
*
* Return:	Array to send 
************************************************/
int *computed_sendable(pixels_int *pixels){
	int *send, pos, i;

	send = malloc(sizeof(int) * computed_size_sendable(pixels));
	pos = 0;
	send[pos++] = pixels->nbDone;
	for ( i = 0 ; i < pixels->nbDone ; i++ ) {
		send[pos++] = pixels->done[i].beg;
		send[pos++] = pixels->done[i].end;
	}
	for ( i = 0 ; i < pixels->nbComputed ; i++ ) {
		send[pos++] = pixels->computed[3*i];
		send[pos++] = pixels->computed[3*i+1];
		send[pos++] = pixels->computed[3*i+2];
	}
	return send;
}

/************************************************
* Name:		computed_add
*
* Description:	Add pixels receive to the struct	
*
* Argument:	- pixels_int *pixels	: pixels computed
*		- int *sent	: pixels received
*
* Return:	number of pixel received 
************************************************/
int computed_add(pixels_int *pixels, int *sent){
	int nbPixel, nbTask, pos, i;

	nbTask = sent[0];
	if ( pixels->nbDone + nbTask > pixels->sizeMallocDone ){
		printf("Over\n");
	}
	nbPixel = 0;
	pos = 1;
	for ( i = 0 ; i < nbTask ; i++ ) {
		pixels->done[pixels->nbDone].beg = sent[pos++];
		pixels->done[pixels->nbDone].end = sent[pos++];
		nbPixel += pixels->done[pixels->nbDone].end - pixels->done[pixels->nbDone].beg + 1;
		pixels->nbDone++;
	}

	if ( pixels->nbComputed + nbPixel > pixels->sizeMallocComputed ) {
		printf("Over\n");
	}
	for ( i = 0 ; i < nbPixel ; i++ ) {
		pixels->computed[3 * pixels->nbComputed] = sent[pos++];
		pixels->computed[3 * pixels->nbComputed + 1] = sent[pos++];
		pixels->computed[3 * pixels->nbComputed + 2] = sent[pos++];
		pixels->nbComputed++;
	}
	return nbPixel;
}

/************************************************
* Name:		assemble_image
*
* Description:	Assemble all pixels computed from
* 		the structure
*
* Argument:	- pixels_int *pixels	: assemble from
*		- int w	: width of picture
*		- int h	: height of picture
*
* Return:	structure image created 
************************************************/
image *assemble_image(pixels_int *pixels, int w, int h){
	image *res;
	int i, j, pos, posw, posh;

	res = malloc(sizeof(image));
	res->w = w;
	res->h = h;
	res->computed = malloc(sizeof(int) * res->w * res->h * 3);
	pos = 0;
	for ( i = 0 ; i < pixels->nbDone ; i++ ) {
		posh = h - pixels->done[i].beg / w - 1;
		posw = pixels->done[i].beg % w;
		for ( j = pixels->done[i].beg ; j < pixels->done[i].end + 1 ; j++ ) {
			res->computed[3*(posh*w+posw)] = pixels->computed[3*pos];
			res->computed[3*(posh*w+posw)+1] = pixels->computed[3*pos+1];
			res->computed[3*(posh*w+posw)+2] = pixels->computed[3*pos+2];
			pos++;
			posw++;
			if ( posw == w ) {
				posw = 0;
				posh--;
			}
		}
	}
	return res;
}

/************************************************
* Name:		free_image
*
* Description:	Free structure image	
*
* Argument:	- image *i	: structure to free
************************************************/
void free_image(image *i) {
	free(i->computed);
	free(i);
}

/************************************************
* Name:		store_image
*
* Description:	Save image to a file	
*
* Argument:	- image *image	: structure image to save
*
* Return:	0 
************************************************/
int store_image(image *image){
	char fileName[20];

	sprintf(fileName, "image/image.ppm");
	FILE *f = fopen(fileName, "w");
	fprintf(f, "P3\n%d %d\n%d\n", image->w, image->h, 255); 
	for (int i = 0; i < image->w * image->h; i++) 
		fprintf(f,"%d %d %d ", image->computed[3 * i], image->computed[3 * i + 1], image->computed[3 * i + 2]); 
	fclose(f);
	return 0;
}

#define TESTSIZE 32

#ifdef TASK_TEST
int main( int argc, char *argv[] ) {
	int w, h, samples;
	int i, j;
	int **sends;
	proc_info **infos;
	job **jobs;
	pixels_int **pixels;
	image *image;

	start_log(0);
	int n = 0;
	int working;
	int steal;
	int sig[TESTSIZE];

	w = 320; h = 200; samples = 200;
	int nbTotalTask = h;
	initRayVariables(w, h); 

	infos = malloc(sizeof(proc_info *) * TESTSIZE);
	sends = malloc(sizeof(int *) * TESTSIZE);
	jobs = malloc(sizeof(job *) * TESTSIZE);
	pixels = malloc(sizeof(pixels_int *) * TESTSIZE);

	for ( i = 0 ; i < TESTSIZE ; i++ ) {
		infos[i] = create_proc_info(TESTSIZE, nbTotalTask);
		jobs[i] = create_job(h, h, w * h, infos[i]);
		for ( j = i ; j < h ; j += TESTSIZE ) {
			job_add_task(jobs[i], create_task_line(j, w));
		}
	}
	do {
		working = TESTSIZE;
		for ( i = 0 ; i < TESTSIZE ; i++ ) {
			if ( jobs[i]->nbAssigned > 0 ){
				printf("%d : doing ", i);
				for ( j = 0 ; j < jobs[i]->nbAssigned ; j++ ) {
					printf("%d, ", jobs[i]->assigned[j].beg);
				}
				printf("\n");
				job_do_last(jobs[i], w, h, samples);
				if ( i < TESTSIZE - 1 ){
					steal = jobs[i]->nbAssigned > 5 ? 5 : jobs[i]->nbAssigned;
					sends[i] = sendable_tasks(infos[i], jobs[i], TESTSIZE, steal);
					sig[i] = 1;
				}
				if ( i > 0 && sig[i-1] == 1){
					receive_tasks(infos[i], jobs[i], sends[i-1], TESTSIZE);
					sig[i-1] = 0;
					free(sends[i-1]);
				}
			} else {
				working--;
			}
		}
	}while ( working > 0 );

	for ( i = 0 ; i < TESTSIZE ; i++ ) {
		pixels[i] = job_compute(jobs[i], h, w*h);
		free_job(jobs[i]);
	}
	free(jobs);

	for ( i = 1 ; i < TESTSIZE ; i++ ) {
		sends[i] = computed_sendable(pixels[i]);
		computed_add(pixels[0], sends[i]);
		free(sends[i]);
		free_pixels(pixels[i]);
	}

	image = assemble_image(pixels[0], w, h);
	store_image(image);

	for ( i = 0 ; i < TESTSIZE ; i++ ) {
		free_proc_info(infos[i]);
	}
	free(infos);
	free(sends);
	free_image(image);
	free_pixels(pixels[0]);
	close_log();
	return 0;
}
#endif
