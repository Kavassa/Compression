#include "matrice.h"
#include "dct.h"
#include "jpg.h"
#include "image.h"

/*
 * Calcul de la DCT ou de l'inverse DCT sur un petit carré de l'image.
 * On fait la transformation de l'image ``sur place'' c.a.d.
 * que le paramètre "image" est utilisé pour l'entrée et la sortie.
 *
 * DCT de l'image :  DCT * IMAGE * DCT transposée
 * Inverse        :  DCT transposée * I' * DCT
 */
void dct_image(int inverse, int nbe, float **image)
{
  static float** DCT_inverse;
  static float** DCT;
  float** image_temp = allocation_matrice_carree_float(nbe);

  if(DCT == NULL)
  {
    //Allocation de la matrice DCT
    DCT = allocation_matrice_carree_float(nbe);
    //On la rempli avec les coeff de la DCT
    coef_dct(nbe, DCT);
    //DCT inverse
    DCT_inverse = allocation_matrice_carree_float(nbe);
    //On transpose
    transposition_matrice_carree(nbe, DCT, DCT_inverse);
  }

  if(inverse)
  {
    produit_matrices_carrees_float(nbe, DCT_inverse, image, image_temp);
    produit_matrices_carrees_float(nbe, image_temp, DCT, image);
  }
  else
  {
    produit_matrices_carrees_float(nbe, DCT, image, image_temp);
    produit_matrices_carrees_float(nbe, image_temp, DCT_inverse, image);
  }
  //liberation_matrice_carree_float(DCT_inverse, nbe);
  //liberation_matrice_carree_float(DCT, nbe);
  liberation_matrice_carree_float(image_temp, nbe);
}

/*
 * Quantification/Déquantification des coefficients de la DCT
 * Si inverse est vrai, on déquantifie.
 * Attention, on reste en calculs flottant (en sortie aussi).
 */
void quantification(int nbe, int qualite, float **extrait, int inverse)
{
  int i;
  int j;
  for(i = 0; i < nbe; i++)
  {
    for(j = 0; j < nbe; j++)
    {
      if(inverse)
        extrait[i][j] = extrait[i][j] * ((1+(i + j +1)) * qualite);
      else
        extrait[i][j] = extrait[i][j] / ((1+(i + j +1)) * qualite);
    }
  }
}

/*
 * ZIGZAG.
 * On fournit à cette fonction les coordonnées d'un point
 * et elle nous donne le suivant (Toujours YX comme d'habitude)
 *
 * +---+---+---+---+     +---+---+---+
 * |00 |01 |   |   |     |   |   |   |
 * | ----/ | /---/ |     | ----/ | | |
 * |   |/  |/  |/  |     |   |/  |/| |
 * +---/---/---/---+     +---/---/-|-+
 * |10/|  /|  /|   |     |  /|  /| | |
 * | / | / | / | | |     | / | / | | |
 * | | |/  |/  |/| |     | | |/  |/  |
 * +-|-/---/---/-|-+     +-|-/---/---+
 * | |/|  /|  /| | |     | |/|  /|   |
 * | / | / | / | | |     | / | ----- |
 * |   |/  |/  |/  |     |   |   |   |
 * +---/---/---/---+     +---+---+---+
 * |  /|  /|  /|   |    
 * | /---/ | /---- |    
 * |   |   |   |   |    
 * +---+---+---+---+    
 */
void zigzag(int nbe, int *y, int *x)
{
  int * x_temp = x;
  int * y_temp = y;
  if( (*y + *x) % 2)
  {
    x_temp = y;
    y_temp = x;
  }
  if(*x_temp+1 >= nbe) //Si on sort de la matrice
    (*y_temp)++;
  else
  { 
    if(*y_temp != 0)
      (*y_temp)--;
    (*x_temp)++;
  }

  /*
  //Si on monte ou on va a droite
  if( (*y + *x) % 2 == 0)
  {
    if(*x+1 >= nbe) //Si on sort de la matrice
      (*y)++;
    else
    { 
      if(*y != 0)
        (*y)--;
      (*x)++;
    }
  }
  //Si on descend ou on va a gauche
  else
  {
    if(*y+1 >= nbe) //Si on sort de la matrice
      (*x)++;
    else
    { 
      if(*x != 0)
        (*x)--;
      (*y)++;
    }
  }*/
}



/*
 * Extraction d'une matrice de l'image (le résultat est déjà alloué).
 * La sous-image carrée à la position et de la taille indiquée
 * est stockée dans matrice "extrait"
 */

static void extrait_matrice(int y, int x, int nbe
			    , const struct image *entree
			    , float **extrait
			    )
 {
  int i, j ;

  for(j=0;j<nbe;j++)
    for(i=0;i<nbe;i++)
      if ( j+y < entree->hauteur && i+x < entree->largeur )
	extrait[j][i] = entree->pixels[j+y][i+x] ;
      else
	extrait[j][i] = 0 ;
 }

/*
 * Insertion d'une matrice de l'image.
 * C'est l'opération inverse de la précédente.
 */

static void insert_matrice(int y, int x, int nbe
			   , float **extrait
			   , struct image *sortie
			   )
 {
  int i, j ;

  for(j=0;j<nbe;j++)
    for(i=0;i<nbe;i++)
      if ( j+y < sortie->hauteur && i+x < sortie->largeur )
	{
	  if ( extrait[j][i] < 0 )
	    sortie->pixels[j+y][i+x] = 0 ;
	  else
	    {
	      if ( extrait[j][i] > 255 )
		sortie->pixels[j+y][i+x] = 255 ;
	      else
		sortie->pixels[j+y][i+x] = rint(extrait[j][i]) ;
	    }
	}
 }


/*
 * Compression d'une l'image :
 * Pour chaque petit carré on fait la dct et l'on stocke dans un fichier
 */
void compresse_image(int nbe, const struct image *entree, FILE *f)
 {
  static float **tmp = NULL ;
  int i, j, k ;

  if ( tmp == NULL )
    {
      tmp = allocation_matrice_carree_float(nbe) ;
    }

  for(j=0;j<entree->hauteur;j+=nbe)
    for(i=0;i<entree->largeur;i+=nbe)
      {
	extrait_matrice(j, i, nbe, entree, tmp) ;
	dct_image(0, nbe, tmp) ;
	for(k=0; k<nbe; k++)
	  assert(fwrite(tmp[k], sizeof(**tmp), nbe, f) == nbe) ;
      }
 }

/*
 * Décompression image
 * On récupère la DCT de chaque fichier, on fait l'inverse et
 * on insère dans l'image qui est déjà allouée
 */
void decompresse_image(int nbe, struct image *entree, FILE *f)
 {
  static float **tmp = NULL ;
  int i, j, k ;

  if ( tmp == NULL )
    {
      tmp = allocation_matrice_carree_float(nbe) ;
    }

  for(j=0;j<entree->hauteur;j+=nbe)
    for(i=0;i<entree->largeur;i+=nbe)
      {
	for(k=0; k<nbe; k++)
	  assert(fread(tmp[k], sizeof(**tmp), nbe, f) == nbe) ;
	dct_image(1, nbe, tmp) ;
	insert_matrice(j, i, nbe, tmp, entree) ;
      }
 }
