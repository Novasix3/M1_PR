#ifndef HPC_DEBUG_H
#define HPC_DEBUG_H

#define DEBUG_LVL 3

#define MINIMAL_DEBUG 1
#define PARTIAL_DEBUG 2
#define FULL_DEBUG 3

#include "tasks.h"

void start_log(int my_rank);

double diff_time();

void print_debug_int(char *format, int var, int lvl);

void print_debug_task(char *format, task* task, int lvl);

void print_debug_proc_info(char *format, proc_info* info, int lvl);
void print_debug_proc_info_all(char *format, proc_info* info, int size, int lvl);

void print_debug_job(char *format, job* steve, int lvl);

void print_debug_pixel_int(char *format, pixels_int* pix, int lvl);

void write_log();

void close_log();

#endif
