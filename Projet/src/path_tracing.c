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

#include "path_tracing.h"

#include <immintrin.h>

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

static const double CST = 0.5135;  /* ceci défini l'angle de vue */
double camera_position[3], camera_direction[3], cx[3], cy[3];
__m256d camera_position_avx, camera_direction_avx, cx_avx, cy_avx;

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

#pragma omp declare simd aligned(x,y:32)
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

static inline __m256d dot_avx(__m256d a, __m256d b){
	__m256d c, d;
	c = _mm256_mul_pd(a, b);
	d = _mm256_hadd_pd(c, c);
	return _mm256_add_pd(d, _mm256_permute4x64_pd(d, 0x1B));
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

static inline __m256d normalize_avx(__m256d a){
	return _mm256_mul_pd(_mm256_div_pd( _mm256_set1_pd(1.0) , _mm256_sqrt_pd( dot_avx(a, a) )), a);
}

/* produit vectoriel */
static inline void cross(const double *a, const double *b, double *c)
{
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

static inline __m256d cross_avx(__m256d a, __m256d b){
	__m256d a1, a2, b1, b2;
	a1 = _mm256_permute4x64_pd(a, 0x09);
	a2 = _mm256_permute4x64_pd(a, 0x12);
	b1 = _mm256_permute4x64_pd(b, 0x12);
	b2 = _mm256_permute4x64_pd(b, 0x09);
	return _mm256_sub_pd(_mm256_mul_pd(a1, b1), _mm256_mul_pd(a2, b2));
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

static inline double tiny_clamp(double a){
	return a > 1. ? 1. : a < 0. ? 0. : a;
}

static inline __m256d clamp_avx(__m256d a){
	return _mm256_set_pd(0., tiny_clamp(a[2]), tiny_clamp(a[1]), tiny_clamp(a[0]));
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
void radiance(const double *ray_origin, const double *ray_direction, int depth, unsigned int *random_state, double *out)
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
		if (double_random(random_state) < p) {
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
		double r1 = 2 * M_PI * double_random(random_state);  /* angle aléatoire */
		double r2 = double_random(random_state);             /* distance au centre aléatoire */
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
		radiance(x, d, depth, random_state, rec);
		
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
		radiance(x, reflected_dir, depth, random_state, rec);
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
		radiance(x, reflected_dir, depth, random_state, rec);
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
		if (double_random(random_state) < P) {
			radiance(x, reflected_dir, depth, random_state, rec);
			double RP = Re / P;
			scal(RP, rec);
		} else {
			radiance(x, tdir, depth, random_state, rec);
			double TP = Tr / (1 - P); 
			scal(TP, rec);
		}
	} else {
		double rec_re[3], rec_tr[3];
		radiance(x, reflected_dir, depth, random_state, rec_re);
		radiance(x, tdir, depth, random_state, rec_tr);
		zero(rec);
		axpy(Re, rec_re, rec);
		axpy(Tr, rec_tr, rec);
	}
	/* pondère, prend en compte la luminance */
	mul(f, rec, out);
	axpy(1, obj->emission, out);
	return;
}

/* calcule (dans out) la lumiance reçue par la camera sur le rayon donné */
__m256d radiance_avx(__m256d ray_origin_avx, __m256d ray_direction_avx, 
		int depth, unsigned short *random_state)
{ 
	int id = 0;                             // id de la sphère intersectée par le rayon
	double t;                               // distance à l'intersection
	double ray_origin[4], ray_direction[4];
	_mm256_store_pd(ray_origin, ray_origin_avx);
	_mm256_store_pd(ray_direction, ray_direction_avx);
	if (!intersect(ray_origin, ray_direction, &t, &id)) {
		return _mm256_setzero_pd(); 
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
		if (erand48(random_state) < p) {
			scal(1 / p, f); 
		} else {
			return _mm256_set_pd(0., obj->emission[2], obj->emission[1], obj->emission[0]);
		}
	}

	/* Cas de la réflection DIFFuse (= non-brillante). 
	   On récupère la luminance en provenance de l'ensemble de l'univers. 
	   Pour cela : (processus de monte-carlo) on choisit une direction
	   aléatoire dans un certain cone, et on récupère la luminance en 
	   provenance de cette direction. */
	if (obj->refl == DIFF) {
		double r1 = 2 * M_PI * erand48(random_state);  /* angle aléatoire */
		double r2 = erand48(random_state);             /* distance au centre aléatoire */
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
		radiance(x, d, depth, random_state, rec);
		
		/* pondère par la couleur de la sphère, prend en compte l'emissivité */
		return _mm256_set_pd(0., f[2] * rec[2] + obj->emission[2], 
				f[1] * rec[1] + obj->emission[1], f[0] * rec[0] + obj->emission[0]);
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
		radiance(x, reflected_dir, depth, random_state, rec);
		/* pondère par la couleur de la sphère, prend en compte l'emissivité */
		return _mm256_set_pd(0., f[2] * rec[2] + obj->emission[2], 
				f[1] * rec[1] + obj->emission[1], f[0] * rec[0] + obj->emission[0]);
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
		radiance(x, reflected_dir, depth, random_state, rec);
		return _mm256_set_pd(0., f[2] * rec[2] + obj->emission[2], 
				f[1] * rec[1] + obj->emission[1], f[0] * rec[0] + obj->emission[0]);
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
		if (erand48(random_state) < P) {
			radiance(x, reflected_dir, depth, random_state, rec);
			double RP = Re / P;
			scal(RP, rec);
		} else {
			radiance(x, tdir, depth, random_state, rec);
			double TP = Tr / (1 - P); 
			scal(TP, rec);
		}
	} else {
		double rec_re[3], rec_tr[3];
		radiance(x, reflected_dir, depth, random_state, rec_re);
		radiance(x, tdir, depth, random_state, rec_tr);
		zero(rec);
		axpy(Re, rec_re, rec);
		axpy(Tr, rec_tr, rec);
	}
	/* pondère, prend en compte la luminance */
	return _mm256_set_pd(0., f[2] * rec[2] + obj->emission[2], 
			f[1] * rec[1] + obj->emission[1], f[0] * rec[0] + obj->emission[0]);
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

void copyPixel(double* pixel_radiance, struct picturePixel *pix){
	pix->R = pixel_radiance[0];
	pix->G = pixel_radiance[1];
	pix->B = pixel_radiance[2];
	printf("final : R = %.7f, G = %.7f, B = %.7f\n\n\n", 
		pixel_radiance[0], pixel_radiance[1], pixel_radiance[2]);
}

void copyPixel_avx(__m256d pixel_radiance, struct picturePixel *pix){
	pix->R = pixel_radiance[0];
	pix->G = pixel_radiance[1];
	pix->B = pixel_radiance[2];
	printf("final : R = %.7f, G = %.7f, B = %.7f\n\n\n", 
		pixel_radiance[0], pixel_radiance[1], pixel_radiance[2]);
	
}

double inline double_random(unsigned int *state){
	return (double) rand_r(state) / RAND_MAX;
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
*		- struct picturePixel *pix	: adresse de stockage du pixel
************************************************/
inline void computePixel(int w, int h, int samples, int i, int j, struct picturePixel *pix) 
{
	/* calcule la luminance d'un pixel, avec sur-échantillonnage 2x2 */
	double pixel_radiance[3] = {0, 0, 0};
	int s;
	
	for (int sub = 0; sub < 4; sub++) {
		unsigned int random_state = i*i + ( sub & 1 ) + ( sub >> 1 ) * 3;
		double subpixel_radiance[3] = {0, 0, 0};
		double p1 = 0.0, p2 = 0.0, p3 = 0.0;
		/* simulation de monte-carlo : on effectue plein de lancers de rayons et on moyenne */

		for (s = 0; s < samples; s++) {
			/* tire un rayon aléatoire dans une zone de la caméra qui correspond à peu près au pixel à calculer */
			double r1 = 2 * double_random(&random_state);
			double dx =  (r1 < 1) ? sqrt(r1) - 1 : 1 - sqrt(2 - r1); 
			double r2 = 2 * double_random(&random_state);
			double dy = (r2 < 1) ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
			double ray_direction[3];
			copy(camera_direction, ray_direction);
			axpy((((sub >> 1) + .5 + dy) / 2 + i) / h - .5, cy, ray_direction);
			axpy((((sub & 1) + .5 + dx) / 2 + j) / w - .5, cx, ray_direction);
			normalize(ray_direction);

			double ray_origin[3];
			copy(camera_position, ray_origin);
			axpy(140, ray_direction, ray_origin);
			
			/* estime la lumiance qui arrive sur la caméra par ce rayon */
			double sample_radiance[3];
			radiance(ray_origin, ray_direction, 0, &random_state, sample_radiance);
			/* fait la moyenne sur tous les rayons */
			p1 += sample_radiance[0];
			p2 += sample_radiance[1];
			p3 += sample_radiance[2];
		}
		subpixel_radiance[0] = p1 / samples;
		subpixel_radiance[1] = p2 / samples;
		subpixel_radiance[2] = p3 / samples;
		clamp(subpixel_radiance);
		/* fait la moyenne sur les 4 sous-pixels */
		axpy(0.25, subpixel_radiance, pixel_radiance);
	}
	copyPixel(pixel_radiance, pix);
}

/************************************************
* Name:		computePixel_mt
*
* Description:	Calcul la valeur d'un pixel et la copie en memoire avec plusieurs threads
*
* Argument:	- int w	: longueur
*		- int h	: hauteur
*		- int samples	: nombre de rayon
*		- int i	: coordonne y du pixel
*		- int j	: coordonne x du pixel
*		- struct picturePixel *pix	: adresse de stockage du pixel
************************************************/
inline void computePixel_mt(int w, int h, int samples, int i, int j, struct picturePixel *pix) 
{
	/* calcule la luminance d'un pixel, avec sur-échantillonnage 2x2 */
	double pixel_radiance[3] = {0, 0, 0};
	
	//#pragma omp parallel for schedule(dynamic) num_threads(2)
	for (int sub = 0; sub < 4; sub++) {
		unsigned int random_state = i*i + ( sub & 1 ) + ( sub >> 1 ) * 3;
		double subpixel_radiance[3] = {0, 0, 0};
		/* simulation de monte-carlo : on effectue plein de lancers de rayons et on moyenne */

		//#pragma omp parallel for schedule(dynamic) num_threads(2) reduction(+:p1,p2,p3) 
		for (int s = 0; s < samples; s++) {
			/* tire un rayon aléatoire dans une zone de la caméra qui correspond à peu près au pixel à calculer */
			double r1 = 2 * double_random(&random_state);
			double dx =  (r1 < 1) ? sqrt(r1) - 1 : 1 - sqrt(2 - r1); 
			double r2 = 2 * double_random(&random_state);
			double dy = (r2 < 1) ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
			double ray_direction[3];
			copy(camera_direction, ray_direction);
			axpy((((sub >> 1) + .5 + dy) / 2 + i) / h - .5, cy, ray_direction);
			axpy((((sub & 1) + .5 + dx) / 2 + j) / w - .5, cx, ray_direction);
			normalize(ray_direction);

			double ray_origin[3];
			copy(camera_position, ray_origin);
			axpy(140, ray_direction, ray_origin);
			
			/* estime la lumiance qui arrive sur la caméra par ce rayon */
			double sample_radiance[3];
			radiance(ray_origin, ray_direction, 0, &random_state, sample_radiance);
			/* fait la moyenne sur tous les rayons */
			subpixel_radiance[0] += sample_radiance[0];
			subpixel_radiance[1] += sample_radiance[1];
			subpixel_radiance[2] += sample_radiance[2];
		}
		subpixel_radiance[0] *= 1. / samples;
		subpixel_radiance[1] *= 1. / samples;
		subpixel_radiance[2] *= 1. / samples;
		clamp(subpixel_radiance);
		/* fait la moyenne sur les 4 sous-pixels */
//#pragma omp critical
		axpy(0.25, subpixel_radiance, pixel_radiance);
	}
	copyPixel(pixel_radiance, pix);
}

/************************************************
* Name:		computePixel_simd
*
* Description:	Calcul la valeur d'un pixel et la copie en memoire en SIMD
*
* Argument:	- int w	: longueur
*		- int h	: hauteur
*		- int samples	: nombre de rayon
*		- int i	: coordonne y du pixel
*		- int j	: coordonne x du pixel
*		- struct picturePixel *pix	: adresse de stockage du pixel
************************************************/
#pragma omp declare simd
inline void computePixel_simd(int w, int h, int samples, int i, int j, struct picturePixel *pix) 
{
	/* calcule la luminance d'un pixel, avec sur-échantillonnage 2x2 */
	double pixel_radiance[3] __attribute__((aligned(32))) = {0, 0, 0};
	
	//#pragma omp for simd aligned(pixel_radiance:32)
	for (int sub = 0; sub < 4; sub++) {
		unsigned int random_state = i*i + ( sub & 1 ) + ( sub >> 1 ) * 3;
		double subpixel_radiance[3] __attribute__((aligned(32))) = {0, 0, 0};
		/* simulation de monte-carlo : on effectue plein de lancers de rayons et on moyenne */
		double p1 = 0.0, p2 = 0.0, p3 = 0.0;

		#pragma omp simd 
		for (int s = 0; s < samples; s++) {
			/* tire un rayon aléatoire dans une zone de la caméra qui correspond à peu près au pixel à calculer */
			double r1 = 2 * double_random(&random_state);
			double dx =  (r1 < 1) ? sqrt(r1) - 1 : 1 - sqrt(2 - r1); 
			double r2 = 2 * double_random(&random_state);
			double dy = (r2 < 1) ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
			double ray_direction[3] __attribute__((aligned(32)));
			copy(camera_direction, ray_direction);
			axpy((((sub >> 1) + .5 + dy) / 2 + i) / h - .5, cy, ray_direction);
			//printf("après cy : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", ray_direction[0], ray_direction[1], ray_direction[2]);
			axpy((((sub & 1) + .5 + dx) / 2 + j) / w - .5, cx, ray_direction);

			//printf("après cx cy : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", ray_direction[0], ray_direction[1], ray_direction[2]);
			normalize(ray_direction);
			//printf("après normalize : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", ray_direction[0], ray_direction[1], ray_direction[2]);

			double ray_origin[3] __attribute__((aligned(32)));
			copy(camera_position, ray_origin);
			axpy(140, ray_direction, ray_origin);
			printf("après axpy : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", ray_direction[0], ray_direction[1], ray_direction[2]);
			
			/* estime la lumiance qui arrive sur la caméra par ce rayon */
			double sample_radiance[3] __attribute__((aligned(32)));
			radiance(ray_origin, ray_direction, 0, &random_state, sample_radiance);
			/* fait la moyenne sur tous les rayons */
			p1 += sample_radiance[0];
			p2 += sample_radiance[1];
			p3 += sample_radiance[2];
		}
		subpixel_radiance[0] = p1 / samples;
		subpixel_radiance[1] = p2 / samples;
		subpixel_radiance[2] = p3 / samples;
		//printf("div : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", subpixel_radiance[0], subpixel_radiance[1], subpixel_radiance[2]);
		clamp(subpixel_radiance);
		//printf("clamp : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", subpixel_radiance[0], subpixel_radiance[1], subpixel_radiance[2]);
		/* fait la moyenne sur les 4 sous-pixels */
		//#pragma omp critical
		axpy(0.25, subpixel_radiance, pixel_radiance);
		//printf("div : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", pixel_radiance[0], pixel_radiance[1], pixel_radiance[2]);
	}
	copyPixel(pixel_radiance, pix);
}

/************************************************
* Name:		computePixel_avx
*
* Description:	Calcul la valeur d'un pixel et la copie en memoire en avx
*
* Argument:	- int w	: longueur
*		- int h	: hauteur
*		- int samples	: nombre de rayon
*		- int i	: coordonne y du pixel
*		- int j	: coordonne x du pixel
*		- struct picturePixel *pix	: adresse de stockage du pixel
************************************************/
inline void computePixel_avx(int w, int h, int samples, int i, int j, struct picturePixel *pix) 
{
	/* calcule la luminance d'un pixel, avec sur-échantillonnage 2x2 */
	__m256d pixel_radiance_avx __attribute__((aligned(32))) = _mm256_set1_pd(0.);
	
	for (int sub = 0; sub < 4; sub++) {
		printf("début d'un pixel\n" );
		unsigned int random_state = i*i + ( sub & 1 ) + ( sub >> 1 ) * 3;
 		unsigned short PRNG_state[3] = {0, 0, i*i*i};
		__m256d subpixel_radiance_avx __attribute__((aligned(32))) = _mm256_set1_pd(0.);
		//printf("start : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", subpixel_radiance_avx[0], subpixel_radiance_avx[1], subpixel_radiance_avx[2]);
		/* simulation de monte-carlo : on effectue plein de lancers de rayons et on moyenne */

		for (int s = 0; s < samples; s++) {
			/* tire un rayon aléatoire dans une zone de la caméra qui correspond à peu près au pixel à calculer */
			/*double r1 = 2 * double_random(&random_state);
			double dx =  (r1 < 1) ? sqrt(r1) - 1 : 1 - sqrt(2 - r1); 
			double r2 = 2 * double_random(&random_state);
			double dy = (r2 < 1) ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);*/
			double r1 = 2 * erand48(PRNG_state);
			double dx = (r1 < 1) ? sqrt(r1) - 1 : 1 - sqrt(2 - r1); 
			double r2 = 2 * erand48(PRNG_state);
			double dy = (r2 < 1) ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);

			__m256d tmp1 __attribute__((aligned(32))), tmp2 __attribute__((aligned(32)));
			tmp1 = _mm256_set1_pd((((sub >> 1) + .5 + dy) / 2 + i) / h - .5);
			tmp2 = _mm256_set1_pd((((sub & 1) + .5 + dx) / 2 + j) / w - .5);

			__m256d ray_direction_avx __attribute__((aligned(32))) = 
				_mm256_fmadd_pd(tmp1, cy_avx, camera_direction_avx);
			
			ray_direction_avx = _mm256_fmadd_pd(tmp2, cx_avx, ray_direction_avx);
			//printf("après cx : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", ray_direction_avx[0], ray_direction_avx[1], ray_direction_avx[2]);
		
			ray_direction_avx = normalize_avx(ray_direction_avx);
			//printf("normalize : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", ray_direction_avx[0], ray_direction_avx[1], ray_direction_avx[2]);

			__m256d ray_origin_avx __attribute__((aligned(32))) = 
				_mm256_fmadd_pd(_mm256_set1_pd(140), ray_direction_avx, camera_position_avx);
			printf("second fma : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", ray_origin_avx[0], ray_origin_avx[1], ray_origin_avx[2]);
			
			/* estime la lumiance qui arrive sur la caméra par ce rayon */
			__m256d sample_radiance_avx __attribute__((aligned(32))) = 
				radiance_avx(ray_origin_avx, ray_direction_avx, 0, &PRNG_state);
			//printf("radiance : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", sample_radiance_avx[0], sample_radiance_avx[1], sample_radiance_avx[2]);
			/* fait la moyenne sur tous les rayons */
			subpixel_radiance_avx = _mm256_add_pd(subpixel_radiance_avx, sample_radiance_avx);
			//printf("add : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", subpixel_radiance_avx[0], subpixel_radiance_avx[1], subpixel_radiance_avx[2]);
		}
		subpixel_radiance_avx = _mm256_div_pd( subpixel_radiance_avx, _mm256_set1_pd(1.));
		printf("div : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", subpixel_radiance_avx[0], subpixel_radiance_avx[1], subpixel_radiance_avx[2]);
		clamp_avx(subpixel_radiance_avx);
		printf("clamp : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", subpixel_radiance_avx[0], subpixel_radiance_avx[1], subpixel_radiance_avx[2]);
		/* fait la moyenne sur les 4 sous-pixels */
		pixel_radiance_avx = _mm256_add_pd(pixel_radiance_avx, subpixel_radiance_avx);
		printf("second add : [0] = %.7f, [1] = %.7f, [2] = %.7f\n", pixel_radiance_avx[0], pixel_radiance_avx[1], pixel_radiance_avx[2]);
	}
	pixel_radiance_avx = _mm256_div_pd( pixel_radiance_avx, _mm256_set1_pd(4.));
	//printf("final : a[0] = %.7f, a[1] = %.7f, a[2] = %.7f\n\n\n", pixel_radiance_avx[0], pixel_radiance_avx[1], pixel_radiance_avx[2]);
	copyPixel_avx(pixel_radiance_avx, pix);

}

/************************************************
* Name:		initRayVariables
*
* Description:	Initiate variable require to compute ray-tracing	
*
* Argument:	- int w	: width
*		- int h	: height
************************************************/
void initRayVariables(int w, int h){
	int i, n;

	camera_position[0] = 50;
	camera_position[1] = 52;
	camera_position[2] = 295.6;
	camera_direction[0] = 0;
	camera_direction[1] = -0.042612;
	camera_direction[2] = -1;
	normalize(camera_direction);

	/* incréments pour passer d'un pixel à l'autre */
	cx[0] = w * CST / h;
	cx[1] = 0;
	cx[2] = 0;
	cross(cx, camera_direction, cy);  /* cy est orthogonal à cx ET à la direction dans laquelle regarde la caméra */
	normalize(cy);
	scal(CST, cy);

	/* précalcule la norme infinie des couleurs */
	n = sizeof(spheres) / sizeof(struct Sphere);
	for (i = 0; i < n; i++) {
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
}

/************************************************
* Name:		initRayVariables_avx
*
* Description:	Initiate variable require to compute ray-tracing in avx
*
* Argument:	- int w	: width
*		- int h	: height
************************************************/
void initRayVariables_avx(int w, int h){
	camera_position_avx = _mm256_set_pd(0., 295.6, 52., 50.);
	camera_direction_avx = normalize_avx(_mm256_set_pd(0., -1., -0.042612, 0.));

	/* incréments pour passer d'un pixel à l'autre */
	cx_avx = _mm256_set_pd(0., 0., 0.,  w * CST / h);
	cy_avx = cross_avx(cx_avx, camera_direction_avx);  /* cy est orthogonal à cx ET à la direction dans laquelle regarde la caméra */
	cy_avx = _mm256_mul_pd(_mm256_set1_pd(CST), normalize_avx(cy_avx));

	/* précalcule la norme infinie des couleurs */
	/*
	n = sizeof(spheres) / sizeof(struct Sphere);
	for (i = 0; i < n; i++) {
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
	*/
}

