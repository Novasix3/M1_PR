#ifndef PATH_TRACING_H
#define PATH_TRACING_H

struct picturePixel{
	double R;
	double G;
	double B;
};

double wtime();

int toInt(double x);

void computePixel(int w, int h, int samples, int i, int j, struct picturePixel *pix);

void computePixel_mt(int w, int h, int samples, int i, int j, struct picturePixel *pix);

void computePixel_simd(int w, int h, int samples, int i, int j, struct picturePixel *pix);

void computePixel_avx(int w, int h, int samples, int i, int j, struct picturePixel *pix);

double double_random(unsigned int *state);

void initRayVariables(int w, int h);

void initRayVariables_avx(int w, int h);

#endif
