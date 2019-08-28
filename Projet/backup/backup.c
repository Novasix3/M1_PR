/* basé sur on smallpt, a Path Tracer by Kevin Beason, 2008
 *  	http://www.kevinbeason.com/smallpt/ 
 *
 * Converti en C et modifié par Charles Bouillaguet, 2019
 *
 * Pour des détails sur le processus de rendu, lire :
 * 	https://docs.google.com/open?id=0B8g97JkuSSBwUENiWTJXeGtTOHFmSm51UC01YWtCZw
 */

#define _XOPEN_SOURCE
#include <math.h>   
#include <stdlib.h> 
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/stat.h>  /* pour mkdir    */ 
#include <unistd.h>    /* pour getuid   */
#include <sys/types.h> /* pour getpwuid */
#include <pwd.h>       /* pour getpwuid */


enum Refl_t {DIFF, SPEC, REFR};   /* types de matériaux (DIFFuse, SPECular, REFRactive) */

struct Sphere { 
	double radius; 
	double position[3];
	double emission[3];     /* couleur émise (=source de lumière) */
	double color[3];        /* couleur de l'objet RGB (diffusion, refraction, ...) */
	enum Refl_t refl;       /* type de reflection */
	double max_reflexivity;
};

static const int KILL_DEPTH = 7;
static const int SPLIT_DEPTH = 4;

/* la scène est composée uniquement de spheres */
struct Sphere spheres[] = { 
// radius position,                         emission,     color,              material 
   {1e5,  { 1e5+1,  40.8,       81.6},      {},           {.75,  .25,  .25},  DIFF, -1}, // Left 
   {1e5,  {-1e5+99, 40.8,       81.6},      {},           {.25,  .25,  .75},  DIFF, -1}, // Right 
   {1e5,  {50,      40.8,       1e5},       {},           {.75,  .75,  .75},  DIFF, -1}, // Back 
   {1e5,  {50,      40.8,      -1e5 + 170}, {},           {},                 DIFF, -1}, // Front 
   {1e5,  {50,      1e5,        81.6},      {},           {0.75, .75,  .75},  DIFF, -1}, // Bottom 
   {1e5,  {50,     -1e5 + 81.6, 81.6},      {},           {0.75, .75,  .75},  DIFF, -1}, // Top 
   {16.5, {40,      16.5,       47},        {},           {.999, .999, .999}, SPEC, -1}, // Mirror 
   {16.5, {73,      46.5,       88},        {},           {.999, .999, .999}, REFR, -1}, // Glass 
   {10,   {15,      45,         112},       {},           {.999, .999, .999}, DIFF, -1}, // white ball
   {15,   {16,      16,         130},       {},           {.999, .999, 0},    REFR, -1}, // big yellow glass
   {7.5,  {40,      8,          120},        {},           {.999, .999, 0   }, REFR, -1}, // small yellow glass middle
   {8.5,  {60,      9,          110},        {},           {.999, .999, 0   }, REFR, -1}, // small yellow glass right
   {10,   {80,      12,         92},        {},           {0, .999, 0},       DIFF, -1}, // green ball
   {600,  {50,      681.33,     81.6},      {12, 12, 12}, {},                 DIFF, -1},  // Light 
   {5,    {50,      75,         81.6},      {},           {0, .682, .999}, DIFF, -1}, // occlusion, mirror
}; 


/********** micro BLAS LEVEL-1 + quelques fonctions non-standard **************/
static inline void copy(const double *x, double *y)
{
	for (int i = 0; i < 3; i++)
		y[i] = x[i];
} 

static inline void zero(double *x)
{
	for (int i = 0; i < 3; i++)
		x[i] = 0;
} 

static inline void axpy(double alpha, const double *x, double *y)
{
	for (int i = 0; i < 3; i++)
		y[i] += alpha * x[i];
} 

static inline void scal(double alpha, double *x)
{
	for (int i = 0; i < 3; i++)
		x[i] *= alpha;
} 

static inline double dot(const double *a, const double *b)
{ 
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
} 

static inline double nrm2(const double *a)
{
	return sqrt(dot(a, a));
}

/********* fonction non-standard *************/
static inline void mul(const double *x, const double *y, double *z)
{
	for (int i = 0; i < 3; i++)
		z[i] = x[i] * y[i];
} 

static inline void normalize(double *x)
{
	scal(1 / nrm2(x), x);
}

/* produit vectoriel */
static inline void cross(const double *a, const double *b, double *c)
{
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

/****** tronque *************/
static inline void clamp(double *x)
{
	for (int i = 0; i < 3; i++) {
		if (x[i] < 0)
			x[i] = 0;
		if (x[i] > 1)
			x[i] = 1;
	}
} 

/******************************* calcul des intersections rayon / sphere *************************************/
   
// returns distance, 0 if nohit 
double sphere_intersect(const struct Sphere *s, const double *ray_origin, const double *ray_direction)
{ 
	double op[3];
	// Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0 
	copy(s->position, op);
	axpy(-1, ray_origin, op);
	double eps = 1e-4;
	double b = dot(op, ray_direction);
	double discriminant = b * b - dot(op, op) + s->radius * s->radius; 
	if (discriminant < 0)
		return 0;   /* pas d'intersection */
	else 
		discriminant = sqrt(discriminant);
	/* détermine la plus petite solution positive (i.e. point d'intersection le plus proche, mais devant nous) */
	double t = b - discriminant;
	if (t > eps) {
		return t;
	} else {
		t = b + discriminant;
		if (t > eps)
			return t;
		else
			return 0;  /* cas bizarre, racine double, etc. */
	}
}

/* détermine si le rayon intersecte l'une des spere; si oui renvoie true et fixe t, id */
bool intersect(const double *ray_origin, const double *ray_direction, double *t, int *id)
{ 
	int n = sizeof(spheres) / sizeof(struct Sphere);
	double inf = 1e20; 
	*t = inf;
	for (int i = 0; i < n; i++) {
		double d = sphere_intersect(&spheres[i], ray_origin, ray_direction);
		if ((d > 0) && (d < *t)) {
			*t = d;
			*id = i;
		} 
	}
	return *t < inf;
} 

/* calcule (dans out) la lumiance reçue par la camera sur le rayon donné */
void radiance(const double *ray_origin, const double *ray_direction, int depth, unsigned short *PRNG_state, double *out)
{ 
	int id = 0;                             // id de la sphère intersectée par le rayon
	double t;                               // distance à l'intersection
	if (!intersect(ray_origin, ray_direction, &t, &id)) {
		zero(out);    // if miss, return black 
		return; 
	}
	const struct Sphere *obj = &spheres[id];
	
	/* point d'intersection du rayon et de la sphère */
	double x[3];
	copy(ray_origin, x);
	axpy(t, ray_direction, x);
	
	/* vecteur normal à la sphere, au point d'intersection */
	double n[3];  
	copy(x, n);
	axpy(-1, obj->position, n);
	normalize(n);
	
	/* vecteur normal, orienté dans le sens opposé au rayon 
	   (vers l'extérieur si le rayon entre, vers l'intérieur s'il sort) */
	double nl[3];
	copy(n, nl);
	if (dot(n, ray_direction) > 0)
		scal(-1, nl);
	
	/* couleur de la sphere */
	double f[3];
	copy(obj->color, f);
	double p = obj->max_reflexivity;

	/* processus aléatoire : au-delà d'une certaine profondeur,
	   décide aléatoirement d'arrêter la récusion. Plus l'objet est
	   clair, plus le processus a de chance de continuer. */
	depth++;
	if (depth > KILL_DEPTH) {
		if (erand48(PRNG_state) < p) {
			scal(1 / p, f); 
		} else {
			copy(obj->emission, out);
			return;
		}
	}

	/* Cas de la réflection DIFFuse (= non-brillante). 
	   On récupère la luminance en provenance de l'ensemble de l'univers. 
	   Pour cela : (processus de monte-carlo) on choisit une direction
	   aléatoire dans un certain cone, et on récupère la luminance en 
	   provenance de cette direction. */
	if (obj->refl == DIFF) {
		double r1 = 2 * M_PI * erand48(PRNG_state);  /* angle aléatoire */
		double r2 = erand48(PRNG_state);             /* distance au centre aléatoire */
		double r2s = sqrt(r2); 
		
		double w[3];   /* vecteur normal */
		copy(nl, w);
		
		double u[3];   /* u est orthogonal à w */
		double uw[3] = {0, 0, 0};
		if (fabs(w[0]) > .1)
			uw[1] = 1;
		else
			uw[0] = 1;
		cross(uw, w, u);
		normalize(u);
		
		double v[3];   /* v est orthogonal à u et w */
		cross(w, u, v);
		
		double d[3];   /* d est le vecteur incident aléatoire, selon la bonne distribution */
		zero(d);
		axpy(cos(r1) * r2s, u, d);
		axpy(sin(r1) * r2s, v, d);
		axpy(sqrt(1 - r2), w, d);
		normalize(d);
		
		/* calcule récursivement la luminance du rayon incident */
		double rec[3];
		radiance(x, d, depth, PRNG_state, rec);
		
		/* pondère par la couleur de la sphère, prend en compte l'emissivité */
		mul(f, rec, out);
		axpy(1, obj->emission, out);
		return;
	}

	/* dans les deux autres cas (réflection parfaite / refraction), on considère le rayon
	   réfléchi par la spère */

	double reflected_dir[3];
	copy(ray_direction, reflected_dir);
	axpy(-2 * dot(n, ray_direction), n, reflected_dir);

	/* cas de la reflection SPEculaire parfaire (==mirroir) */
	if (obj->refl == SPEC) { 
		double rec[3];
		/* calcule récursivement la luminance du rayon réflechi */
		radiance(x, reflected_dir, depth, PRNG_state, rec);
		/* pondère par la couleur de la sphère, prend en compte l'emissivité */
		mul(f, rec, out);
		axpy(1, obj->emission, out);
		return;
	}

	/* cas des surfaces diélectriques (==verre). Combinaison de réflection et de réfraction. */
	bool into = dot(n, nl) > 0;      /* vient-il de l'extérieur ? */
	double nc = 1;                   /* indice de réfraction de l'air */
	double nt = 1.5;                 /* indice de réfraction du verre */
	double nnt = into ? (nc / nt) : (nt / nc);
	double ddn = dot(ray_direction, nl);
	
	/* si le rayon essaye de sortir de l'objet en verre avec un angle incident trop faible,
	   il rebondit entièrement */
	double cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
	if (cos2t < 0) {
		double rec[3];
		/* calcule seulement le rayon réfléchi */
		radiance(x, reflected_dir, depth, PRNG_state, rec);
		mul(f, rec, out);
		axpy(1, obj->emission, out);
		return;
	}
	
	/* calcule la direction du rayon réfracté */
	double tdir[3];
	zero(tdir);
	axpy(nnt, ray_direction, tdir);
	axpy(-(into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)), n, tdir);

	/* calcul de la réflectance (==fraction de la lumière réfléchie) */
	double a = nt - nc;
	double b = nt + nc;
	double R0 = a * a / (b * b);
	double c = 1 - (into ? -ddn : dot(tdir, n));
	double Re = R0 + (1 - R0) * c * c * c * c * c;   /* réflectance */
	double Tr = 1 - Re;                              /* transmittance */
	
	/* au-dela d'une certaine profondeur, on choisit aléatoirement si
	   on calcule le rayon réfléchi ou bien le rayon réfracté. En dessous du
	   seuil, on calcule les deux. */
	double rec[3];
	if (depth > SPLIT_DEPTH) {
		double P = .25 + .5 * Re;             /* probabilité de réflection */
		if (erand48(PRNG_state) < P) {
			radiance(x, reflected_dir, depth, PRNG_state, rec);
			double RP = Re / P;
			scal(RP, rec);
		} else {
			radiance(x, tdir, depth, PRNG_state, rec);
			double TP = Tr / (1 - P); 
			scal(TP, rec);
		}
	} else {
		double rec_re[3], rec_tr[3];
		radiance(x, reflected_dir, depth, PRNG_state, rec_re);
		radiance(x, tdir, depth, PRNG_state, rec_tr);
		zero(rec);
		axpy(Re, rec_re, rec);
		axpy(Tr, rec_tr, rec);
	}
	/* pondère, prend en compte la luminance */
	mul(f, rec, out);
	axpy(1, obj->emission, out);
	return;
}

double wtime()
{
	struct timeval ts;
	gettimeofday(&ts, NULL);
	return (double)ts.tv_sec + ts.tv_usec / 1E6;
}

int toInt(double x)
{
	return pow(x, 1 / 2.2) * 255 + .5;   /* gamma correction = 2.2 */
} 

/************************************************
* Name:		computePixel
*
* Description:	Calcul la valeur d'un pixel et la copie en memoire	
*
* Argument:	- int w	: longueur
*		- int h	: hauteur
*		- int samples	: nombre de rayon
*		- int i	: coordonne y du pixel
*		- int j	: coordonne x du pixel
*		- unsigned short *PRNG_state	: Je ne sais pas
*		- double *camera_position : position de la camera
*		- double *camera_direction : direction de la camera
*		- double *cx : increment en x
*		- double *cy : increment en y
*		- double *image	: adresse de stockage du pixel
************************************************/
void computePixel(int w, int h, int samples, int i, int j, unsigned short *PRNG_state, 
		double *camera_position, double *camera_direction, double *cx, double *cy,
		double *image) 
{
	/* calcule la luminance d'un pixel, avec sur-échantillonnage 2x2 */
	double pixel_radiance[3] = {0, 0, 0};
	for (int sub_i = 0; sub_i < 2; sub_i++) {
		for (int sub_j = 0; sub_j < 2; sub_j++) {
			double subpixel_radiance[3] = {0, 0, 0};
			/* simulation de monte-carlo : on effectue plein de lancers de rayons et on moyenne */
			for (int s = 0; s < samples; s++) { 
				/* tire un rayon aléatoire dans une zone de la caméra qui correspond à peu près au pixel à calculer */
				double r1 = 2 * erand48(PRNG_state);
				double dx = (r1 < 1) ? sqrt(r1) - 1 : 1 - sqrt(2 - r1); 
				double r2 = 2 * erand48(PRNG_state);
				double dy = (r2 < 1) ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
				double ray_direction[3];
				copy(camera_direction, ray_direction);
				axpy(((sub_i + .5 + dy) / 2 + i) / h - .5, cy, ray_direction);
				axpy(((sub_j + .5 + dx) / 2 + j) / w - .5, cx, ray_direction);
				normalize(ray_direction);

				double ray_origin[3];
				copy(camera_position, ray_origin);
				axpy(140, ray_direction, ray_origin);
				
				/* estime la lumiance qui arrive sur la caméra par ce rayon */
				double sample_radiance[3];
				radiance(ray_origin, ray_direction, 0, PRNG_state, sample_radiance);
				/* fait la moyenne sur tous les rayons */
				axpy(1. / samples, sample_radiance, subpixel_radiance);
			}
			clamp(subpixel_radiance);
			/* fait la moyenne sur les 4 sous-pixels */
			axpy(0.25, subpixel_radiance, pixel_radiance);
		}
	}
	copy(pixel_radiance, image); // <-- retournement vertical
}

/* Same as computePixel but for a line */
void computeLine(int w, int h, int samples, int i, double *camera_position, 
		double *camera_direction, double *cx, double *cy, double *image) {
 		unsigned short PRNG_state[3] = {0, 0, i*i*i};
		for (unsigned short j = 0; j < w; j++) {
			computePixel(w, h, samples, i, j, PRNG_state, camera_position, camera_direction, cx, cy, 
					image + 3 * j); // <-- retournement vertical
		}
}

#include <mpi.h>

#define TAG_DEMANDE 1
#define TAG_WORK 2
#define TAG_NBTASK 3
#define TAG_TASK 4
#define TAG_GPR 5
#define TAG_END 6

#define ROOTPROC 0


/************************************************
* Name:		initTaskTab
*
* Description:	Remplit les tableaux de liste des taches a faire et
* 		nombre de taches a faire pour chaque processus
*
* Argument:	- int *tasks	: liste des taches
*		- int *procTask	: nombre de tache par proc
*		- int h	: hauteur de l'image
*		- int nbproc	: nombre de processus
*		- int my_rank	: rang du processus
************************************************/
void initTaskTab(int *tasks, int *procTask, int h, int nbproc, int my_rank){
	int i, nbLine;

	nbLine = h / nbproc;
	for ( i = 0 ; i < nbproc ; i++ ) {
		procTask[i] = nbLine;
		procTask[nbproc+i] = 0;
	}
	procTask[my_rank] = 0;
	for ( i = my_rank ; i < h ; i += nbproc ) {
		tasks[procTask[my_rank]++] = i;
	}

}

void updateTaskTab(int *T, int *buf, int nbproc){
	int i;

	for ( i = 0 ; i < nbproc ; i++ ) {
		if (T[nbproc+i] < buf[nbproc+i]) {
			T[i] = buf[i];
			T[nbproc+i] = buf[nbproc+i];
		}
	}
}

int proc_to_steal(int *procTask, int nbproc, int my_rank){
	int indMax, max, i;

	indMax = -1;
	max = -1;
	for ( i = 0 ; i < nbproc ; i++ ) {
		if (i != my_rank && procTask[i] > nbproc && procTask[i] > max){
			indMax = i;
			max = procTask[i];
		}
	}
	return indMax;
}

int update_task_list(int *my_list, int my_list_length, int *other_list, int other_list_length){
	for(int i = 0; i < other_list_length; ++i)
		*(my_list + (my_list_length + 1) + i) = *(other_list + i);
	return my_list_length + other_list_length;
}

int main(int argc, char **argv)
{ 
	/* Les variables sont initialiser dans le Makefile */
	int w, h, samples;

	/* Variable ajoute */
	int nbproc, my_rank, *tasks, *completed;
	int task_length;
	int *procTask, *procTaskDem, *procTaskRecv;
	MPI_Request requestDemande, requestFull, requestTask, requestSteal;
	MPI_Status statusDemande;
	int flag;
	int nbTaskSend, *taskRecv, aim;

	if (argc != 4) 
		exit(1);

	w = atoi(argv[1]);
	h = atoi(argv[2]);
	samples = atoi(argv[3]);

	static const double CST = 0.5135;  /* ceci défini l'angle de vue */
	double camera_position[3] = {50, 52, 295.6};
	double camera_direction[3] = {0, -0.042612, -1};
	normalize(camera_direction);

	/* incréments pour passer d'un pixel à l'autre */
	double cx[3] = {w * CST / h, 0, 0};    
	double cy[3];
	cross(cx, camera_direction, cy);  /* cy est orthogonal à cx ET à la direction dans laquelle regarde la caméra */
	normalize(cy);
	scal(CST, cy);

	/* précalcule la norme infinie des couleurs */
	int n = sizeof(spheres) / sizeof(struct Sphere);
	for (int i = 0; i < n; i++) {
		double *f = spheres[i].color;
		if ((f[0] > f[1]) && (f[0] > f[2]))
			spheres[i].max_reflexivity = f[0]; 
		else {
			if (f[1] > f[2])
				spheres[i].max_reflexivity = f[1];
			else
				spheres[i].max_reflexivity = f[2]; 
		}
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nbproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	tasks = malloc(sizeof(int) * ( h / nbproc + 1 ));
	taskRecv = malloc(sizeof(int) * ( h / nbproc + 1));
	procTask = malloc(sizeof(int) * 2 * nbproc); 		/* tableau des taches a faire et calcule */
	procTaskDem = malloc(sizeof(int) * 2 * nbproc);
	procTaskRecv = malloc(sizeof(int) * 2 * nbproc);
	initTaskTab(tasks, procTask, h, nbproc, my_rank);

	if(my_rank == ROOTPROC){
		procTask[my_rank] = procTask[my_rank] * 2 - 5;
		procTask[my_rank+1] = 5;
	}else{
		procTask[my_rank-1] = procTask[my_rank-1] * 2 - 5;
		procTask[my_rank] = 5;
	}

	/* boucle principale */
	double *image = malloc(3 * w * h * sizeof(*image));
	if (image == NULL) {
		perror("Impossible d'allouer l'image");
		exit(1);
	}
	
	int steal_signal_sent = 0;
	int steal_from = 0;

	while ( procTask[my_rank] > 0 ){
		if(procTask[my_rank] < nbproc){
			printf("#%d veut voler\n", my_rank);
			steal_from = proc_to_steal(procTask, nbproc, my_rank);
			if(steal_from != -1){
				printf("#%d va voler %d\n", my_rank, steal_from);
				MPI_Isend(procTask, 2 * nbproc, MPI_INT, steal_from, TAG_DEMANDE, MPI_COMM_WORLD, &requestDemande);
			}else{
				printf("#%d ne peut voler personne\n", my_rank);
			}
			steal_signal_sent = 1;
		}
		computeLine(w, h, samples, tasks[procTask[my_rank]-1], camera_position, camera_direction, cx, cy, 
				image + 3 * procTask[my_rank+nbproc] * w);
		procTask[my_rank]--;
		procTask[my_rank+nbproc]++;
		printf("#%d nb tache : %d\n", my_rank, procTask[my_rank]);
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_DEMANDE, MPI_COMM_WORLD, &flag, &statusDemande); 
		if(flag){
			printf("#%d se fait voler\n", my_rank);
			MPI_Recv(procTaskRecv, 2*nbproc, MPI_INT, statusDemande.MPI_SOURCE, TAG_DEMANDE, MPI_COMM_WORLD, &statusDemande);
			updateTaskTab(procTask, procTaskRecv, nbproc);
			if ( procTask[my_rank] > nbproc) {
				printf("#%d va envoyer son tableau et envoyer des lignes\n", my_rank);
				MPI_Send(procTask, 2*nbproc, MPI_INT, statusDemande.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD);
				printf("#%d pas de bug après premier send\n", my_rank);
				nbTaskSend =  ( procTask[my_rank] - procTask[statusDemande.MPI_SOURCE] ) / 2;
				for ( int i =  nbTaskSend ; i >= 0 ; i-- ) {
					taskRecv[i] = tasks[procTask[my_rank]];
					procTask[my_rank]--;
				}
				procTask[statusDemande.MPI_SOURCE] += nbTaskSend;
				procTask[nbproc+statusDemande.MPI_SOURCE]++;
				procTask[my_rank]++;
				printf("#%d pas de bug avant second send\n", my_rank);
				MPI_Send(&nbTaskSend, 1, MPI_INT, statusDemande.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD);
				printf("#%d pas de bug après second send\n", my_rank);
				MPI_Send(taskRecv, nbTaskSend, MPI_INT, statusDemande.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD);
				printf("#%d pas de bug après troisième send\n", my_rank);
			} else {
				printf("#%d va envoyer son tableau sans envoyer de ligne\n", my_rank);
				MPI_Send(procTask, 2*nbproc, MPI_INT, statusDemande.MPI_SOURCE, TAG_GPR, MPI_COMM_WORLD);
				if(statusDemande.MPI_SOURCE == steal_from)
					steal_signal_sent = 0;
			}
		}
		if(steal_signal_sent){
			printf("#%d va récup des lignes de %d\n", my_rank, steal_from);
			MPI_Recv(procTaskRecv, 2 * nbproc, MPI_INT, steal_from, MPI_ANY_TAG, MPI_COMM_WORLD, &statusDemande);
			updateTaskTab(procTask, procTaskRecv, nbproc);
			printf("#%d a récup le tableau de %d\n", my_rank, steal_from);
			if(statusDemande.MPI_TAG == TAG_TASK){
				MPI_Recv(&nbTaskSend, 1, MPI_INT, steal_from, TAG_TASK, MPI_COMM_WORLD, &statusDemande);
				printf("#%d va récup %d lignes\n", my_rank, nbTaskSend);
				MPI_Recv(taskRecv, nbTaskSend, MPI_INT, steal_from, TAG_TASK, MPI_COMM_WORLD, &statusDemande);
				procTask[my_rank] = update_task_list(tasks, procTask[my_rank], taskRecv, nbTaskSend);
			}
			steal_signal_sent = 0;
		}
	}

	/*while ( procTask[my_rank] > 0 ) {
		// Calcul d'une ligne 
		if (my_rank != 0 || procTask[my_rank] < 92){
			computeLine(w, h, samples, tasks[procTask[my_rank]-1], camera_position, camera_direction, cx, cy, 
				image + 3 * procTask[my_rank+nbproc] * w); // <-- retournement vertical
			procTask[my_rank]--;
			procTask[my_rank+nbproc]++;
			printf("#%d nb tache : %d\n", my_rank, procTask[my_rank]);
		}
		// Regarde si on vole des taches 
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_DEMANDE, MPI_COMM_WORLD, &flag, &statusDemande); 
		if ( flag ) {
			printf("#%d se fait voler\n", my_rank);
			MPI_Recv(procTaskBuf, 2*nbproc, MPI_INT, MPI_ANY_SOURCE, TAG_DEMANDE, MPI_COMM_WORLD, &statusDemande);
			updateTaskTab(procTask, procTaskBuf, nbproc);
			if ( procTask[my_rank] > 10 && procTask[statusDemande.MPI_SOURCE] < procTask[my_rank]) {
				nbTaskSend =  ( procTask[my_rank] - procTask[statusDemande.MPI_SOURCE] ) / 2;
				for ( int i =  nbTaskSend ; i >= 0 ; i-- ) {
					taskSend[i] = tasks[procTask[my_rank]];
					procTask[my_rank]--;
				}
				procTask[statusDemande.MPI_SOURCE] += nbTaskSend;
				procTask[nbproc+statusDemande.MPI_SOURCE]++;
				procTask[my_rank]++;
				MPI_Send(procTask, 2*nbproc, MPI_INT, statusDemande.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);
				MPI_Send(&nbTaskSend, 1, MPI_INT, statusDemande.MPI_SOURCE, TAG_NBTASK, MPI_COMM_WORLD);
				MPI_Send(taskSend, nbTaskSend, MPI_INT, statusDemande.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD);			
			} else {
				MPI_Send(procTask, 2*nbproc, MPI_INT, statusDemande.MPI_SOURCE, TAG_GPR, MPI_COMM_WORLD);
			}
		}
		// vole une tache 
		if ( procTask[my_rank] < 90 ) {
			aim = proc_to_steal(procTask, nbproc);
			printf("#%d vole a %d\n", my_rank, aim);
			if ( aim != my_rank ) {
				MPI_Send(procTask, 2*nbproc, MPI_INT, aim, TAG_DEMANDE, MPI_COMM_WORLD);
			}
		}
		// Regarde si une tache a ete recu 
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_WORK, MPI_COMM_WORLD, &flag, &status); 
		if ( flag ) {
			printf("#%d a recu des taches\n", my_rank);
			MPI_Recv(procTaskBuf, 2*nbproc, MPI_INT, MPI_ANY_SOURCE, TAG_WORK, MPI_COMM_WORLD, &status);
			updateTaskTab(procTask, procTaskBuf, nbproc);
			MPI_Recv(&nbTaskSend, 1, MPI_INT, status.MPI_SOURCE, TAG_NBTASK, MPI_COMM_WORLD, &status);
			MPI_Recv(taskSend, nbTaskSend, MPI_INT, status.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD, &status);
			for ( int i = 0 ; i < nbTaskSend ; i++ ) {
				tasks[procTask[my_rank]] = taskSend[i];
				procTask[my_rank]++;
			}
		}
		// Regarde si un full tache a ete recu 
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_GPR, MPI_COMM_WORLD, &flag, &status);
		while ( flag ) {
			MPI_Recv(procTaskBuf, 2*nbproc, MPI_INT, MPI_ANY_SOURCE, TAG_GPR, MPI_COMM_WORLD, &status);
			updateTaskTab(procTask, procTaskBuf, nbproc);
		}
	}*/



	printf("#%d a calculé %d\n", my_rank, procTask[my_rank+nbproc]);


	/* stocke l'image dans un fichier au format NetPbm */
	{
		char fileName[20];

		sprintf(fileName, "image%d.ppm", my_rank);
		FILE *f = fopen(fileName, "w");
		fprintf(f, "P3\n%d %d\n%d\n", w, h, 255); 
		for (int i = 0; i < w * h; i++) 
	  		fprintf(f,"%d %d %d ", toInt(image[3 * i]), toInt(image[3 * i + 1]), toInt(image[3 * i + 2])); 
		fclose(f); 
	}

	free(image);

	MPI_Finalize();
}

