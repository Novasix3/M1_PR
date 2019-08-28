#ifndef HPC_TASKS_H
#define HPC_TASKS_H

#include "path_tracing.h"

/*
 * Structure task qui contient une tache
 * avec un pointeur vers la partie calcule
 */
struct task_s{
	int beg; /* indice premier pixel a calculer */
	int end; /* indice dernier pixel */
	struct picturePixel *computed; /* si calcule pointe vers valeur */
};
typedef struct task_s task;

task create_task(int beg, int end);
task create_task_line(int number, int w);

int do_task(task *task, int w, int h, int samples);

int do_task_mt(task *task, int w, int h, int samples);

int do_task_simd(task *task, int w, int h, int samples);

int do_task_avx(task *task, int w, int h, int samples);

int do_task_all(task *task, int w, int h, int samples);

/*
 * Structure proc_info qui va remplacer notre procTask
 * pour simplification
 */
struct proc_info_s{
	int id; /* rang du processus */
	int nbAssigned; /* nombre de taches supposes assignees */
	int nbDone; /* nombre de taches faites */
	int parent; /* rang du pere */
	int child[2]; /* rang des enfants */
	int level; /* level of proc in the tree */
	int victim; /* boolean if proc is good to steal */
};
typedef struct proc_info_s proc_info;

proc_info *create_proc_info(int nbproc, int nbTask);

void free_proc_info(proc_info *p);

int *proc_info_sendable(proc_info *infos, int nbproc);

int proc_info_update(proc_info *infos, int *update, int nbproc);

/*
 * Structure job qui contient les taches faites et
 * a faire du processus courant
 */
struct job_s{
	int nbAssigned; /* nombre de taches assignees */
	int sizeMallocAssigned; /* taille du malloc des taches assignees */
	task *assigned; /* tableau des taches assignees */
	int nbDone; /* nombre de taches faites */
	int sizeMallocDone; /* taille du malloc des taches faites */
	task *done; /* tableau des taches faites */
	int nbComputed; /* number of pixel computed */
	int sizeMallocComputed; /* taille du malloc des pixels calculees */
	struct picturePixel *computed; /* tableau de toutes les valeurs calculees */
	proc_info *info; /* info du processus de proc_info */
};
typedef struct job_s job;

job *create_job(int maxAssigned, int maxDone, int maxComputed, proc_info *my_info);

void free_job(job* steve);

int job_add_task(job *job, task task);

int job_do_last(job *job, int w, int h, int samples);

int job_do_last_mt(job *job, int w, int h, int samples);

int job_do_last_simd(job *job, int w, int h, int samples);

int job_do_last_avx(job *job, int w, int h, int samples);

int job_do_last_all(job *job, int w, int h, int samples);

int *sendable_tasks(proc_info *infos, job *job, int nbproc, int nbTask);

int receive_tasks(proc_info *infos, job *job, int *sent, int nbproc);

/*
 * Structure computed qui contient les pixels calcules
 * et transformer en int
 */
struct computed_s{
	int beg;
	int end;
};
typedef struct computed_s task_computed;

/*
 * Structure qui contient les pixels calcules 
 * pas encore mis en ordre
 */
struct pixels_int_s{
	int nbDone;
	int sizeMallocDone;
	task_computed *done;
	int nbComputed;
	int sizeMallocComputed;
	int *computed;
};
typedef struct pixels_int_s pixels_int;

pixels_int *job_compute(job *job, int maxDone, int maxComputed);

void free_pixels(pixels_int *pixel);

int computed_size_sendable(pixels_int *pixel);

int *computed_sendable(pixels_int *pixels);

int computed_add(pixels_int *pixel, int *sent);

/*
 * Structure de l'image final
 */
struct image_s{
	int w;
	int h;
	int *computed;
};
typedef struct image_s image;

image *assemble_image(pixels_int *computed, int w, int h);

void free_image(image *i);

int store_image(image *image);

#endif
