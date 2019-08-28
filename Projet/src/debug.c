#include <stdio.h>

#include "debug.h"
#include "path_tracing.h"

static FILE *log;
static double beg_time;
static int file_id;

/************************************************
* Name:		start_log
*
* Description:	Start timer for beginning of log line
* 		and open the file
*
* Argument:	- int my_rank	: rank of processus
************************************************/
void start_log(int my_rank){
	char logName[20];

	beg_time = wtime();
	if ( DEBUG_LVL < MINIMAL_DEBUG )
		return;
	sprintf(logName, "log/proc%d.log", my_rank);
	log = fopen(logName, "w");
	if ( log == NULL ) {
		printf("Couldn't open log file\n");
	}
	file_id = my_rank;
}


/************************************************
* Name:		diff_time
*
* Description:	Give time since init	
*
* Return:	difference time 
************************************************/
double diff_time(){
	return wtime() - beg_time;
}

/************************************************
* Name:		print_debug
*
* Description:	Print a string in the log file	
*
* Argument:	- char *format	: string format
*		- int lvl	: debug level
************************************************/
static int print_debug(char *format, int lvl){
	if ( lvl > DEBUG_LVL )
		return 1;

	double timer = diff_time();

	fprintf(log, "%f  #%d : %s\n", timer, file_id, format);
	fprintf(log, "%f  #%d : ", timer, file_id);

	return 0;
}


/************************************************
* Name:		print_proc_info
*
* Description:	print a struct proc_info in the log file	
*
* Argument:	- proc_info* info	: struct to print
************************************************/
static void print_proc_info(proc_info* info){
	fprintf(log, "(%d, %d, %d, %d, %d, %d, %d)", 
		info->id, info->nbAssigned, info->nbDone, info->parent, 
		info->child[0], info->child[1], info->level);
}


/************************************************
* Name:		print_task
*
* Description:	print a struct task in the log file	
*
* Argument:	- task* task	: task to print
*		- int lvl	: level of debug
************************************************/
static void print_task(task* task, int lvl){
	
	fprintf(log, "(%d, %d, ", task->beg, task->end);

	if(lvl >= FULL_DEBUG && task->computed != NULL)
		fprintf(log, "%.2f %.2f %.2f)", 
			task->computed->R, task->computed->G, task->computed->B);
	else
		fprintf(log, "%p)", task->computed);
}


/************************************************
* Name:		print_task_all
*
* Description:	print n tasks in the log file	
*
* Argument:	- task* task	: tasks to print
*		- int lvl	: level of debug
*		- int size	: number of task
************************************************/
static void print_task_all(task* task, int lvl, int size){
	int i = 0;
	fprintf(log, "{");

	while(i < size){
		fprintf(log, "[%d]", i);
		print_task(task, lvl);

		task++;

		if(i++ < size-1)
			fprintf(log, ", ");
	}
	fprintf(log, "}");
}


/************************************************
* Name:		print_computed_all
*
* Description:	print all computed value	
*
* Argument:	- task_computed* task	: task to print
*		- int lvl	: level of debug
*		- int size	: number of value to print
************************************************/
static void print_computed_all(task_computed* task, int lvl, int size){
	int i = 0;
	fprintf(log, "{");

	while(i < size){
		fprintf(log, "[%d]", i);
		fprintf(log, "(%d, %d)", task->beg, task->end);

		task++;

		if(i++ < size-1)
			fprintf(log, ", ");
	}
	fprintf(log, "}");
}


/************************************************
* Name:		print_array
*
* Description:	print an array in the log file	
*
* Argument:	- int *array	: array to print
*		- int size	: size of array
************************************************/
static void print_array(int *array, int size){

	fprintf(log, "[");
	for (int i = 0 ; i < size - 1; i++ ) {
		fprintf(log, "%d, ", array[i]);
	}
	fprintf(log, "%d]", array[size - 1]);
}



/************************************************
* Name:		print_debug_int
*
* Description:	print an int in the log file	
*
* Argument:	- char *format	: format to print
*		- int var	: variable to print
*		- int lvl	: debug level
************************************************/
void print_debug_int(char *format, int var, int lvl){
	if ( lvl > DEBUG_LVL )
		return;

	double timer = diff_time();

	fprintf(log, "%f  #%d : ", timer, file_id);
	fprintf(log, format, var);
	fprintf(log, "\n");
}


/************************************************
* Name:		print_debug_task
*
* Description:	print a task in the log file	
*
* Argument:	- char *format	: format to print
*		- task* task	: task to print
*		- int lvl	: debug level
************************************************/
void print_debug_task(char *format, task* task, int lvl){
	if(print_debug(format, lvl))
		return;

	fprintf(log, "task_s : {");
	
	fprintf(log, "B : %d, E : %d, C : ", task->beg, task->end);

	if(lvl >= FULL_DEBUG && task->computed != NULL)
		fprintf(log, "%.2f %.2f %.2f", 
			task->computed->R, task->computed->G, task->computed->B);
	else
		fprintf(log, "%p", task->computed);

	fprintf(log, "}\n");
}

/************************************************
* Name:		print_debug_proc_info
*
* Description:	print a proc info structure in the log file	
*
* Argument:	- char *format	: format to print
*		- proc_info* info	: proc_info struct to print
*		- int lvl	: debug level
************************************************/
void print_debug_proc_info(char *format, proc_info* info, int lvl){
	if(print_debug(format, lvl))
		return;

	fprintf(log, "proc_info_s : { ");

	print_proc_info(info);

	fprintf(log, " }\n");
}

/************************************************
* Name:		print_debug_proc_info_all
*
* Description:	print all proc_info struct	
*
* Argument:	- char *format	: format to print
*		- proc_info* info	: proc_infos to print
*		- int size	: number of proc_info
*		- int lvl	: debug level
************************************************/
void print_debug_proc_info_all(char *format, proc_info* info, int size, int lvl){
	if(print_debug(format, lvl))
		return;

	int i = 0;

	fprintf(log, "proc_info_s* : { ");

	while(i < size){
		print_proc_info(info);

		info++;

		if(i++ < size-1)
			fprintf(log, ", ");
	}

	fprintf(log, " }\n");
}


/************************************************
* Name:		print_debug_job
*
* Description:	print a job in the log file	
*
* Argument:	- char *format	: format to print
*		- job* steve	: job to print
*		- int lvl	: debug level
************************************************/
void print_debug_job(char *format, job* steve, int lvl){
	if(print_debug(format, lvl))
		return;

	fprintf(log, "job_s : { ");

	fprintf(log, "nbA : %d, nbD : %d, sD : %d, nbC : %d, sC : %d, info : ", 
		steve->nbAssigned, steve->nbDone, steve->sizeMallocDone, 
				steve->nbComputed, steve->sizeMallocComputed);

	print_proc_info(steve->info);

	if(lvl == MINIMAL_DEBUG){
		fprintf(log, ", A : %p, D : %p, C : %p", 
			steve->assigned, steve->done, steve->computed);
	}else{
		fprintf(log, ", A : ");
		print_task_all(steve->assigned, lvl, steve->nbAssigned);
		fprintf(log, ", D : ");
		print_task_all(steve->done, lvl, steve->nbDone);
		fprintf(log, ", C : ");

		//afficher assigned et done comme des tbaleaux de tailles appropriées

		if(lvl >= FULL_DEBUG){
			int i = 0;
			struct picturePixel *temp = steve->computed;
			
			fprintf(log, "{ ");

			while(i < steve->nbComputed){
				fprintf(log, "(%.2f %.2f %.2f)", 
					temp->R, temp->G, temp->B);

				temp++;

				if(i++ < steve->nbComputed-1)
					fprintf(log, ", ");
			}
			
			fprintf(log, "}");

		}else{
			fprintf(log, "%p", steve->computed);
		}
	}

	fprintf(log, " }\n");
}


/************************************************
* Name:		print_debug_pixel_int
*
* Description:	print a pixels_int struct in the log file	
*
* Argument:	- char *format	: format to print
*		- pixels_int* pix	: pixels to print
*		- int lvl	: debug level
************************************************/
void print_debug_pixel_int(char *format, pixels_int* pix, int lvl){
	if(print_debug(format, lvl))
		return;

	fprintf(log, "pixels_int_s : { ");

	fprintf(log, "nbD : %d, sD : %d, nbC : %d, sC : %d, D : ", 
		pix->nbDone, pix->sizeMallocDone, 
		pix->nbComputed, pix->sizeMallocComputed);

	if(lvl >= FULL_DEBUG){
		print_computed_all(pix->done, lvl, pix->nbDone);
		fprintf(log, ", C : ");
		print_array(pix->computed, pix->nbComputed);
	}else{
		fprintf(log, "%p, C : %p", 
			pix->done, pix->computed);
	}
	
	fprintf(log, " }\n");
}


/************************************************
* Name:		write_log
*
* Description:	write buffer saved to file	
************************************************/
void write_log(){
	if ( DEBUG_LVL < MINIMAL_DEBUG )
		return;
	fflush(log);
}

/************************************************
* Name:		close_log
*
* Description:	close the log file	
************************************************/
void close_log(){
	if ( DEBUG_LVL < MINIMAL_DEBUG )
		return;
	fclose(log);
}

