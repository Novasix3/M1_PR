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