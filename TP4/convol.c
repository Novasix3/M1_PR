/*
 * Université Pierre et Marie Curie
 * Calcul de convolution sur une image.
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>   /* pour le rint */
#include <string.h> /* pour le memcpy */
#include <time.h>   /* chronometrage */
#include <mpi.h>   /* mpi */

#include "rasterfile.h"

#define MAX(a,b) ((a>b) ? a : b)

#define ROOTPROC 0

#define TAG_END 0
#define TAG_REQ 1
/** 
 * \struct Raster
 * Structure décrivant une image au format Sun Raster
 */

typedef struct {
  struct rasterfile file;  ///< Entête image Sun Raster
  unsigned char rouge[256],vert[256],bleu[256];  ///< Palette de couleur
  unsigned char *data;    ///< Pointeur vers l'image
} Raster;




double my_gettimeofday(){
  struct timeval tmp_time;
  gettimeofday(&tmp_time, NULL);
  return tmp_time.tv_sec + (tmp_time.tv_usec * 1.0e-6L);
}




/**
 * Cette procedure convertit un entier LINUX en un entier SUN 
 *
 * \param i pointeur vers l'entier à convertir
 */

void swap(int *i) {
  unsigned char s[4],*n;
  memcpy(s,i,4);
  n=(unsigned char *)i;
  n[0]=s[3];
  n[1]=s[2];
  n[2]=s[1];
  n[3]=s[0];
}

/**
 * \brief Lecture d'une image au format Sun RASTERFILE.
 *
 * Au retour de cette fonction, la structure r est remplie
 * avec les données liée à l'image. Le champ r.file contient
 * les informations de l'entete de l'image (dimension, codage, etc).
 * Le champ r.data est un pointeur, alloué par la fonction
 * lire_rasterfile() et qui contient l'image. Cette espace doit
 * être libéré après usage.
 *
 * \param nom nom du fichier image
 * \param r structure Raster qui contient l'image
 *  chargée en mémoire
 */

void lire_rasterfile(char *nom, Raster *r) {
  FILE *f;
  int i;
    
  if( (f=fopen( nom, "r"))==NULL) {
    fprintf(stderr,"erreur a la lecture du fichier %s\n", nom);
    exit(1);
  }
  fread( &(r->file), sizeof(struct rasterfile), 1, f);    
  swap(&(r->file.ras_magic));
  swap(&(r->file.ras_width));
  swap(&(r->file.ras_height));
  swap(&(r->file.ras_depth));
  swap(&(r->file.ras_length));
  swap(&(r->file.ras_type));
  swap(&(r->file.ras_maptype));
  swap(&(r->file.ras_maplength));
    
  if ((r->file.ras_depth != 8) ||  (r->file.ras_type != RT_STANDARD) ||
      (r->file.ras_maptype != RMT_EQUAL_RGB)) {
    fprintf(stderr,"palette non adaptee\n");
    exit(1);
  }
    
  /* composante de la palette */
  fread(&(r->rouge),r->file.ras_maplength/3,1,f);
  fread(&(r->vert), r->file.ras_maplength/3,1,f);
  fread(&(r->bleu), r->file.ras_maplength/3,1,f);
    
  if ((r->data=malloc(r->file.ras_width*r->file.ras_height))==NULL){
    fprintf(stderr,"erreur allocation memoire\n");
    exit(1);
  }
  fread(r->data,r->file.ras_width*r->file.ras_height,1,f);
  fclose(f);
}

/**
 * Sauve une image au format Sun Rasterfile
 */

void sauve_rasterfile(char *nom, Raster *r)     {
  FILE *f;
  int i;
  
  if( (f=fopen( nom, "w"))==NULL) {
    fprintf(stderr,"erreur a l'ecriture du fichier %s\n", nom);
    exit(1);
  }
    
  swap(&(r->file.ras_magic));
  swap(&(r->file.ras_width));
  swap(&(r->file.ras_height));
  swap(&(r->file.ras_depth));
  swap(&(r->file.ras_length));
  swap(&(r->file.ras_type));
  swap(&(r->file.ras_maptype));
  swap(&(r->file.ras_maplength));
    
  fwrite(&(r->file),sizeof(struct rasterfile),1,f);
  /* composante de la palette */
  fwrite(&(r->rouge),256,1,f);
  fwrite(&(r->vert),256,1,f);
  fwrite(&(r->bleu),256,1,f);
  /* pour le reconvertir pour la taille de l'image */
  swap(&(r->file.ras_width));
  swap(&(r->file.ras_height));
  fwrite(r->data,r->file.ras_width*r->file.ras_height,1,f); 
  fclose(f);
}

/**
 * Réalise une division d'entiers plus précise que
 * l'opérateur '/'.
 * Remarque : la fonction rint provient de la librairie 
 * mathématique.
 */

unsigned char division(int numerateur,int denominateur) {
  
  if (denominateur != 0)
    return (unsigned char) rint((double)numerateur/(double)denominateur); 
  else 
    return 0;
}

static int ordre( unsigned char *a, unsigned char *b) {
  return (*a-*b);
}


typedef enum {
  CONVOL_MOYENNE1, ///< Filtre moyenneur
  CONVOL_MOYENNE2, ///< Filtre moyenneur central
  CONVOL_CONTOUR1, ///< Laplacien
  CONVOL_CONTOUR2, ///< Max gradient
  CONVOL_MEDIAN    ///< Filtre médian
} filtre_t;

/**
 * Réalise une opération de convolution avec un noyau prédéfini sur
 * un point.
 *
 * \param choix type de noyau pour la convolution :
 *  - CONVOL_MOYENNE1 : filtre moyenneur
 *  - CONVOL_MOYENNE2 : filtre moyenneur avec un poid central plus fort
 *  - CONVOL_CONTOUR1 : filtre extracteur de contours (laplacien)
 *  - CONVOL_CONTOUR2 : filtre extracteur de contours (max des composantes du gradient)
 *  - CONVOL_MEDIAN : filtre médian (les 9 valeurs sont triées et la valeur
 *     médiane est retournée).
 * \param NO,N,NE,O,CO,E,SO,S,SE: les valeurs des 9 points
 *  concernés pour le calcul de la convolution (cette dernière est
 *  formellement une combinaison linéaire de ces 9 valeurs).
 * \return la valeur de convolution.
 */

unsigned char filtre( filtre_t choix, 
		      unsigned char NO, unsigned char N,unsigned char NE, 
		      unsigned char O,unsigned char CO, unsigned char E, 
		      unsigned char SO,unsigned char S,unsigned char SE) {
  int numerateur,denominateur;

  switch (choix)
    {
    case CONVOL_MOYENNE1:
	  /* filtre moyenneur */
	  numerateur = (int)NO + (int)N + (int)NE + (int)O + (int)CO + 
	    (int)E + (int)SO + (int)S + (int)SE;
	  denominateur = 9;
	  return division(numerateur,denominateur); 

    case CONVOL_MOYENNE2:
	  /* filtre moyenneur */
	  numerateur = (int)NO + (int)N + (int)NE + (int)O + 4*(int)CO +
	    (int)E + (int)SO + (int)S + (int)SE;
	  denominateur = 12;
	  return division(numerateur,denominateur);	

    case CONVOL_CONTOUR1:
	  /* extraction de contours */
	  numerateur = -(int)N - (int)O + 4*(int)CO - (int)E - (int)S;
	  /* numerateur = -(int)NO -(int)N - (int)NE - (int)O + 8*(int)CO -
		 (int)E - (int)SO - (int)S - (int)SE;
	  */
	  return ((4*abs(numerateur) > 255) ? 255 :  4*abs(numerateur));

    case CONVOL_CONTOUR2:
	  /* extraction de contours */
	  numerateur = MAX(abs(CO-E),abs(CO-S));
	  return ((4*numerateur > 255) ? 255 :  4*numerateur);

    case CONVOL_MEDIAN:{
          unsigned char tab[] = {NO,N,NE,O,CO,E,SO,S,SE};
	  /* filtre non lineaire : tri rapide sur la brillance */
	  qsort( tab, 9, sizeof(unsigned char), (int (*) (const void *,const void *))ordre);
	  return tab[4];
    }

    default:
	  printf("\nERREUR : Filtre inconnu !\n\n");
	  exit(1);
    }
}

/**
 * Convolution d'une image par un filtre prédéfini
 * \param choix choix du filtre (voir la fonction filtre())
 * \param tab pointeur vers l'image
 * \param nbl, nbc dimension de l'image
 *
 * \sa filtre()
 */

int convolution( filtre_t choix, unsigned char tab[],int nbl,int nbc) {
  int i,j;
  unsigned char *tmp;
  
  /* Allocation memoire du tampon intermediaire : */
  tmp = (unsigned char*) malloc(sizeof(unsigned char) *nbc*nbl);
  if (tmp == NULL) {
    printf("Erreur dans l'allocation de tmp dans convolution \n");
    return 1;
  }
  
  /* on laisse tomber les bords */
  for(i=1 ; i<nbl-1 ; i++){
    for(j=1 ; j<nbc-1 ; j++){
      tmp[i*nbc+j] = filtre(
			    choix,
			    tab[(i+1)*nbc+j-1],tab[(i+1)*nbc+j],tab[(i+1)*nbc+j+1],
			    tab[(i  )*nbc+j-1],tab[(i)*nbc+j],tab[(i)*nbc+j+1],
			    tab[(i-1)*nbc+j-1],tab[(i-1)*nbc+j],tab[(i-1)*nbc+j+1]);
    } /* for j */
  } /* for i */
  
  /* Recopie de l'image apres traitement dans l'image initiale,
   * On remarquera que la premiere, la derniere ligne, la premiere
   * et la derniere colonne ne sont pas copiées (ce qui force a faire
   * la copie ligne par ligne). */
  for( i=1; i<nbl-1; i++){
    memcpy( tab+nbc*i+1, tmp+nbc*i+1, (nbc-2)*sizeof(unsigned char));
  } /* for i */
  
  /* Liberation memoire du tampon intermediaire : */
  free(tmp);   
}


/**
 * Interface utilisateur
 */

static char usage [] = "Usage : %s <nom image SunRaster> [0|1|2|3|4] <nbiter>\n";

/*
 * Partie principale
 */

int main(int argc, char *argv[]) {

  /* Variables se rapportant a l'image elle-meme */
  Raster r;
  int    w, h;	/* nombre de lignes et de colonnes de l'image */

  /* Variables liees au traitement de l'image */
  int 	 filtre;		/* numero du filtre */
  int 	 nbiter;		/* nombre d'iterations */

  /* Variables liees au chronometrage */
  double debut, fin;

  /* Variables de boucle */
  int 	i,j;


  if (argc != 4) {
    fprintf( stderr, usage, argv[0]);
    return 1;
  }
      
  /* Saisie des paramètres */
  filtre = atoi(argv[2]);
  nbiter = atoi(argv[3]);


  
  int N, P, source, dest, tag = 0;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &N);
  MPI_Comm_rank(MPI_COMM_WORLD, &P);

  int local_h = 0, all_h = 0;

  /* debut du chronometrage */
  debut = my_gettimeofday();   
        
  /* Lecture du fichier Raster */
  if(P == ROOTPROC){
    lire_rasterfile( argv[1], &r);
    h = r.file.ras_height;
    w = r.file.ras_width;

    if(h%N != 0){
      for(int i = 0; i < N; ++i)
        if(i != ROOTPROC)
          MPI_Send(&local_h, 1, MPI_INT, i, TAG_END, MPI_COMM_WORLD);
        

      return 1;
    }

    all_h = h/N;
    local_h = all_h + 1;

    printf("%d : aucune erreur ici, local_h = %d\n", P, local_h);

    for(int i = 0; i < N; ++i)
      if(i != ROOTPROC){
        MPI_Send(&local_h, 1, MPI_INT, i, TAG_REQ, MPI_COMM_WORLD);
        MPI_Send(&w, 1, MPI_INT, i, TAG_REQ, MPI_COMM_WORLD);
      }

  }else{
    MPI_Recv(&local_h, 1, MPI_INT, ROOTPROC, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_TAG == TAG_END)
      return 1;
    all_h = local_h - 1;
    if(P < N-1)
      local_h += 1;
    printf("%d : aucune erreur la, local_h = %d\n", P, local_h);
    MPI_Recv(&w, 1, MPI_INT, ROOTPROC, TAG_REQ, MPI_COMM_WORLD, &status);
  }
    
  unsigned char* local_data = (unsigned char *) malloc(local_h*w);

  printf("%d : malloc valide = %p\n", P, local_data);

  MPI_Scatter(r.data, local_h*w, MPI_UNSIGNED_CHAR, 
    local_data + ((P == ROOTPROC)?0:w), local_h*w, MPI_UNSIGNED_CHAR, 
    ROOTPROC, MPI_COMM_WORLD);


  printf("%d : scatter fait = %p\n", P, local_data);

  /* La convolution a proprement parler */
  for(i=0 ; i < nbiter ; i++){
    printf("%d : iter = %d\n", P, i);
    convolution( filtre, local_data, local_h, w);

    if(P+1 < N){
      MPI_Sendrecv(local_data + (local_h - 2)*w, w, MPI_UNSIGNED_CHAR, P+1, TAG_REQ,
        local_data + (local_h - 1)*w, w, MPI_UNSIGNED_CHAR, P+1, TAG_REQ, MPI_COMM_WORLD, &status);
    }

    if(P-1 >= 0){
      MPI_Sendrecv(local_data + w, w, MPI_UNSIGNED_CHAR, P-1, TAG_REQ,
        local_data, w, MPI_UNSIGNED_CHAR, P-1, TAG_REQ, MPI_COMM_WORLD, &status);
    }
  } /* for i */

  printf("%d : avant gather \n", P);

  MPI_Gather(local_data + ((P == ROOTPROC)?0:w), w*all_h, MPI_UNSIGNED_CHAR, 
    r.data, w*all_h, MPI_UNSIGNED_CHAR, ROOTPROC, MPI_COMM_WORLD);

  printf("%d : gather fait \n", P);

  /* fin du chronometrage */
  fin = my_gettimeofday();
  printf("Temps total de calcul : %g seconde(s) \n", fin - debut);
    
    /* Sauvegarde du fichier Raster */
  if(P == ROOTPROC){ 
    char nom_sortie[100] = "";
    sprintf(nom_sortie, "post-convolution_filtre%d_nbIter%d.ras", filtre, nbiter);
    sauve_rasterfile(nom_sortie, &r);
  }


  free(local_data);
  MPI_Finalize();

  return 0;
}

