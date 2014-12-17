#include "bases.h"
#include "bitstream.h"
#include "sf.h"
#include "intstream.h"
#include "image.h"
#include "rle.h"
#include "exception.h"
#include "ondelette.h"

/*
 * Les premières fonction vous sont fournies et vous permettent
 * d'écrire facilement la suite.
 */




/*
 * Allocation d'une matrice de float.
 * (tableau de pointeur sur tableau de flottants)
 */

float** allocation_matrice_float(int hauteur, int largeur)
 {
  int i ;
  float **table ;
  
  ALLOUER(table, hauteur) ;
  for(i=0; i<hauteur; i++)
    ALLOUER(table[i], largeur) ;
  return(table) ;
 }

/*
 * Libération
 */

void liberation_matrice_float(float **table, int hauteur)
 {
  int i ;

  for(i=0; i<hauteur; i++)
    free(table[i]) ;

  free(table) ;
 }

/*
 * Elle fonctionne même pour des images rectangulaires.
 * Elle n'alloue pas la mémoire
 * La taille indiquée est celle de la matrice de départ.
 */

void transposition_matrice(float **entree, float **sortie,
			   int hauteur, int largeur)
 {
  int j, i ;

  for(j=0; j<hauteur; j++)
    for(i=0; i<largeur; i++)
      sortie[i][j] = entree[j][i] ;
 }

/*
 * Cela vous permettra d'écrire plus facilement
 * la matrice de flottant dans un fichier image
 */
struct image* creation_image_a_partir_de_matrice_float(float **im
						       , int hauteur
						       , int largeur)
 {
  int j, i ;
  struct image *image ;
  
  image = allocation_image(hauteur, largeur) ;

  for(j=0; j<image->hauteur; j++)
    for(i=0; i<image->largeur; i++)
      {
	if ( im[j][i] > 255 )
	  image->pixels[j][i] = 255 ;
	else if ( im[j][i] < 0 )
	  image->pixels[j][i] = 0 ;
	else
	  image->pixels[j][i] = im[j][i] ;
      }

  return(image) ;
 }
/*
 * Affichage directe de la matrice de flottant sur l'écran
 * Attention, si vous affichez l'image après transformation ondelette
 * le résultat semblera très bruité car les valeurs négatives
 * vont être transformées en blancs.
 */
void affiche_matrice_float(float **im, int hauteur, int largeur)
 {
  FILE *f ;
  struct image *image ;
  
  image = creation_image_a_partir_de_matrice_float(im, hauteur, largeur) ;
  f = popen("xv -", "w") ;
  ecriture_image(f, image) ;
  pclose(f) ;
  liberation_image(image) ;
 }





/*
 * Cette fonction effectue UNE SEULE itération d'une ondelette 1D
 * Voici quelques exemples de calculs
 *
 * Entree            Sortie
 * A                   A
 * A B               (A+B)/2 (A-B)/2
 * A B C             (A+B)/2    C    (A-B)/2
 * A B C D           (A+B)/2 (C+D)/2 (A-B)/2 (C-D)/2
 * A B C D E         (A+B)/2 (C+D)/2   E     (A-B)/2 (C-D)/2
 * A B C D E F       (A+B)/2 (C+D)/2 (E+F)/2 (A-B)/2 (C-D)/2 (E-F)/2
 */

void ondelette_1d(const float *entree, float *sortie, int nbe)
{
  int indice_des_adds;
  int indice_des_diff;
  int indice_sortie = 0;
  int indice_entree = 0;
  int indice_mid = nbe/2;

  //On stocke les +
  for(indice_des_adds = 0; indice_des_adds < indice_mid; indice_des_adds++)
  {
    sortie[indice_sortie] = (entree[indice_entree] + entree[indice_entree+1])/2; //(X+(X+1))/2
    indice_sortie++;
    indice_entree += 2;
  }

  if(nbe%2) //Si nbe est impair
  {
    sortie[indice_sortie] = entree[nbe-1];
    indice_sortie++;
  }
  indice_entree = 0;

  //On stocke les -
  for(indice_des_diff = 0; indice_des_diff < indice_mid; indice_des_diff++)
  {
    sortie[indice_sortie] = (entree[indice_entree]-entree[indice_entree+1])/2; //(X-(X+1))/2
    indice_sortie++;
    indice_entree += 2;
  }
}

/*
 * Comme pour la DCT, on applique dans un sens puis dans l'autre.
 *
 * La fonction reçoit "image" et la modifie directement.
 *
 * Vous pouvez allouer plusieurs images intermédiaires pour
 * simplifier la fonction.
 *
 * Par exemple pour une image  3x6
 *    3x6 ondelette horizontale
 *    On transpose, on a donc une image 6x3
 *    6x3 ondelette horizontale
 *    On transpose à nouveau, on a une image 3x6
 *    On ne travaille plus que sur les basses fréquences (moyennes)
 *    On ne travaille donc que sur le haut gauche de l'image de taille 2x3
 *
 * On recommence :
 *    2x3 horizontal   
 *    transposee => 3x2
 *    3x2 horizontal
 *    transposee => 2x3
 *    basse fréquences => 1x2
 *
 * On recommence :
 *    1x2 horizontal
 *    transposee => 2x1
 *    2x1 horizontal (ne fait rien)
 *    transposee => 1x2
 *    basse fréquences => 1x1
 *
 * L'image finale ne contient qu'un seul pixel de basse fréquence.
 * Les autres sont des blocs de plus ou moins haute fréquence.
 * Sur une image 8x8 :
 *
 * M   	F1H  F2H  F2H  F3H  F3H  F3H  F3H
 * F1V 	F1HV F2H  F2H  F3H  F3H  F3H  F3H
 * F2V 	F2V  F2HV F2HV F3H  F3H  F3H  F3H
 * F2V 	F2V  F2HV F2HV F3H  F3H  F3H  F3H
 * F3V 	F3V  F3V  F3V  F3HV F3HV F3HV F3HV
 * F3V 	F3V  F3V  F3V  F3HV F3HV F3HV F3HV
 * F3V 	F3V  F3V  F3V  F3HV F3HV F3HV F3HV
 * F3V 	F3V  F3V  F3V  F3HV F3HV F3HV F3HV
 *
 * La fréquence F2 est plus petite (moins haute) que la fréquence F3
 * F1H  Indique que c'est une fréquence horizontale
 * F1V  Indique que c'est une fréquence verticale
 * F1HV Indique que c'est une fréquence calculée dans les 2 directions
 * 
 */



void ondelette_2d(float **image, int hauteur, int largeur)
{
  int indice_hauteur;
  int indice_largeur; 
  float** image_transposee;
  float* sortie_largeur;
  float* sortie_hauteur;
  ALLOUER(sortie_largeur, largeur);
  ALLOUER(sortie_hauteur, hauteur);

  while(hauteur > 1 || largeur > 1)
  {
    for(indice_hauteur = 0; indice_hauteur < hauteur; indice_hauteur++)
    {
      //Ondelette sur la ligne
      ondelette_1d( image[indice_hauteur], sortie_largeur, largeur);
      for(indice_largeur = 0; indice_largeur < largeur; indice_largeur++)
      {
        image[indice_hauteur][indice_largeur] = sortie_largeur[indice_largeur];
      }
    }
    //Transpose
    image_transposee = allocation_matrice_float(largeur, hauteur);
    transposition_matrice(image, image_transposee, hauteur, largeur);

    for(indice_largeur = 0; indice_largeur < largeur; indice_largeur++)
    {
      //Ondelette sur la ligne
      ondelette_1d( image_transposee[indice_largeur], sortie_hauteur, hauteur);
      for(indice_hauteur = 0; indice_hauteur < hauteur; indice_hauteur++)
      {
        image_transposee[indice_largeur][indice_hauteur] = sortie_hauteur[indice_hauteur];
      }
    }
    //Re transpose
    transposition_matrice(image_transposee, image, largeur, hauteur);
    hauteur = hauteur%2 + hauteur/2;
    largeur = largeur%2 + largeur/2;

    liberation_matrice_float(image_transposee, largeur);
  }

  free(sortie_largeur);
  free(sortie_hauteur);
}



/*
 * Quantification de l'ondelette.
 * La facteur de qualité initial s'applique à la fréquence la plus haute.
 * Quand on divise la fréquence par 2 on divise qualité par 8
 * tout en restant supérieur à 1.
 * Une qualité de 1 indique que l'on a pas de pertes.
 */

void quantif_ondelette(float **image, int hauteur, int largeur, float qualite)
{

  /*while( qualite >= 1 )
  {
    //Quantification
    for(i = 0; i < hauteur; i++)
    {
      for(j = 0; j < largeur; j++)
      {
        image[i][j] = image[i][j] / ((1+(i + j +1)) * qualite);
      }
    }
    hauteur /= 2 //modulo;
    largeur /= 2;
    qualite /= 8;
  
  }*/









}

/*
 * Sortie des coefficients dans le bonne ordre afin
 * d'être bien compressé par la RLE.
 * Cette fonction n'est pas optimale, elle devrait faire
 * un zigzag en plus dans chacun des blocs.
 */

void codage_ondelette(float **image, int hauteur, int largeur, FILE *f)
 {
  int j, i ;
  float *t, *pt ;
  struct intstream *entier, *entier_signe ;
  struct bitstream *bs ;
  struct shannon_fano *sf ;
  int hau, lar ;

  /*
   * Conversion de la matrice en une table linéaire
   * Pour pouvoir utiliser la fonction "compresse"
   */
  ALLOUER(t, hauteur*largeur) ;
  pt = t ;
  hau = hauteur ;
  lar = largeur ;

  while( hau != 1 || lar != 1 )
    {
      for(j=0; j<hau; j++)
	for(i=0; i<lar; i++)
	  if ( j>=(hau+1)/2 || i>=(lar+1)/2 )
	    *pt++ = image[j][i] ;

      hau = (hau+1)/2 ;
      lar = (lar+1)/2 ;
    }
  *pt = image[0][0] ;
  /*
   * Compression RLE avec Shannon-Fano
   */
  bs = open_bitstream("-", "w") ;
  sf = open_shannon_fano() ;
  entier = open_intstream(bs, Shannon_fano, sf) ;
  entier_signe = open_intstream(bs, Shannon_fano, sf) ;

  compresse(entier, entier_signe, hauteur*largeur, t) ;

  close_intstream(entier) ;
  close_intstream(entier_signe) ;
  close_bitstream(bs) ;
  free(t) ;
 }
  


/*
*******************************************************************************
* Fonctions inverses
*******************************************************************************
*/

void ondelette_1d_inverse(const float *entree, float *sortie, int nbe)
{
  int i;
  int k=0;
  int j=0;
  int nbe_impaire = nbe%2 + nbe/2.0;
  int retour;

  for(i=0;i<nbe_impaire;i++)
  {
      retour = nbe_impaire+k;
      sortie[j] = (entree[i] + entree[retour]);
      sortie[j+1] = (entree[i] - entree[retour]);
      j += 2;
      k++;
  }
  if(nbe%2)
      sortie[j] = entree[nbe_impaire];
}


void ondelette_2d_inverse(float **image, int hauteur, int largeur)
{
  // int indice_tab = 0;
  // int tab_hauteur[hauteur]; // STOCKE HAUTEUR ET LARGEUR BOULET PAS LES 1 ou 0
  // int tab_largeur[largeur];
  // int hauteur_temp = hauteur;
  // int largeur_temp = largeur;

  // tab_hauteur[indice_tab] = hauteur_temp;
  // tab_largeur[indice_tab] = largeur_temp;
  // indice_tab++;

  // fprintf(stderr, "\nhauteurMAX: %d\n", hauteur);
  // fprintf(stderr, "largeurMAX: %d\n", largeur);

  // while(hauteur_temp > 1 || largeur_temp > 1)
  // {
  //   hauteur_temp = hauteur_temp%2 + hauteur_temp/2;
  //   largeur_temp = largeur_temp%2 + largeur_temp/2;
  //   tab_hauteur[indice_tab] = hauteur_temp;
  //   tab_largeur[indice_tab] = largeur_temp;
  //   fprintf(stderr, "hauteur: %d\n", tab_hauteur[indice_tab]);
  //   fprintf(stderr, "largeur: %d\n", tab_largeur[indice_tab]);
  //   indice_tab++;
  // }
  // indice_tab--;

  // fprintf(stderr, "Fin du while: indice_tab = %d \n", indice_tab);

  // int i;
  // int j;
  // int indice_hauteur;
  // int indice_largeur; 
  // float** image_transposee;
  // float* sortie_largeur;
  // float* sortie_hauteur;
  // ALLOUER(sortie_largeur, largeur);
  // ALLOUER(sortie_hauteur, hauteur);

  // do
  // {
  //   //On multiplie
  //   hauteur_temp = tab_hauteur[indice_tab];
  //   largeur_temp = tab_largeur[indice_tab];
  //   indice_tab--;

  //   fprintf(stderr, "h_temp: %d   l_temp: %d \n", hauteur_temp, largeur_temp);

  //   //On transpose
  //   image_transposee = allocation_matrice_float(largeur_temp, hauteur_temp);
  //   transposition_matrice(image, image_transposee, hauteur_temp, largeur_temp);
    
  //   //########################################
  //   //for(i = 0; i < largeur_temp; i++)
  //   //{
  //   //  for(j = 0; j < hauteur_temp; j++)
  //   //  {
  //   //    fprintf(stderr, "  %f  ", image[i][j]);
  //   //  }
  //   //  fprintf(stderr, "\n");
  //   //}
  //   //fprintf(stderr, "\n");
  //   //for(i = 0; i < hauteur_temp; i++)
  //   //{
  //   //  for(j = 0; j < largeur_temp; j++)
  //   //  {
  //   //    fprintf(stderr, "  %f  ", image_transposee[i][j]);
  //   //  }
  //   //  fprintf(stderr, "\n");
  //   //}
  //   //fprintf(stderr, "\n\n");
  //   //##########################################

  //   for(indice_largeur = 0; indice_largeur < largeur_temp; indice_largeur++)
  //   {
  //     //Ondelette sur la ligne
  //     ondelette_1d_inverse( image_transposee[indice_largeur], sortie_largeur, largeur_temp);
  //     for(indice_hauteur = 0; indice_hauteur < hauteur_temp; indice_hauteur++)
  //     {
  //       image_transposee[indice_hauteur][indice_largeur] = sortie_largeur[indice_largeur];
  //     }
  //   }

  //   // for(i = 0; i < largeur_temp; i++)
  //   // {
  //   //   for(j = 0; j < hauteur_temp; j++)
  //   //   {
  //   //     fprintf(stderr, " %f \n", image[i][j]);
  //   //   }
  //   //   fprintf(stderr, "\n\n");
  //   // }

  //   //Transpose
  //   transposition_matrice(image_transposee, image, largeur_temp, hauteur_temp);

  //   //########################################
  //   // for(i = 0; i < largeur_temp; i++)
  //   // {
  //   //   for(j = 0; j < hauteur_temp; j++)
  //   //   {
  //   //     fprintf(stderr, "  %f  ", image[i][j]);
  //   //   }
  //   //   fprintf(stderr, "\n");
  //   // }
  //   // fprintf(stderr, "\n");
  //   // for(i = 0; i < hauteur_temp; i++)
  //   // {
  //   //   for(j = 0; j < largeur_temp; j++)
  //   //   {
  //   //     fprintf(stderr, "  %f  ", image_transposee[i][j]);
  //   //   }
  //   //   fprintf(stderr, "\n");
  //   // }
  //   // fprintf(stderr, "\n\n");
  //   //##########################################

  //   for(indice_hauteur = 0; indice_hauteur < hauteur_temp; indice_hauteur++)
  //   {
  //     //Ondelette sur la ligne
  //     ondelette_1d_inverse( image[indice_hauteur], sortie_hauteur, hauteur_temp);
  //     for(indice_largeur = 0; indice_largeur < largeur_temp; indice_largeur++)
  //     {
  //       image[indice_hauteur][indice_largeur] = sortie_hauteur[indice_largeur];
  //     }
  //   }

  //   liberation_matrice_float(image_transposee, largeur);
  // }
  // while(indice_tab >= 0);

  // free(sortie_largeur);
  // free(sortie_hauteur);

  fprintf(stderr, "\nNOUVEAU\n");
  int hauteurFenetre = 2, largeurFenetre = 2;
  int hauteurTransposee = largeurFenetre, largeurTransposee = hauteurFenetre;
  int i;
  while(hauteurFenetre <= hauteur && largeurFenetre <= largeur)
  {
    float** transposee = allocation_matrice_float(hauteurTransposee, largeurTransposee);
    transposition_matrice(image, transposee, hauteurFenetre, largeurFenetre);

    float** mat_ondelette = allocation_matrice_float(hauteurTransposee, largeurTransposee);
    for(i = 0; i < hauteurTransposee; i++)
      ondelette_1d_inverse(transposee[i], mat_ondelette[i], largeurTransposee);

    float** mat = allocation_matrice_float(hauteurFenetre, largeurFenetre);
    transposition_matrice(mat_ondelette, mat, hauteurTransposee, largeurTransposee);

    for(i = 0; i < hauteurFenetre; i++)
      ondelette_1d_inverse(mat[i], image[i], largeurFenetre);

    hauteurFenetre *= 2;
    largeurFenetre *= 2;
    hauteurTransposee = largeurFenetre;
    largeurTransposee = hauteurFenetre;
  }

}


void dequantif_ondelette(float **image, int hauteur, int largeur, float qualite)
{














}

void decodage_ondelette(float **image, int hauteur, int largeur, FILE *f)
 {
  int j, i ;
  float *t, *pt ;
  struct intstream *entier, *entier_signe ;
  struct bitstream *bs ;
  struct shannon_fano *sf ;

  /*
   * Decompression RLE avec Shannon-Fano
   */
  ALLOUER(t, hauteur*largeur) ;
  bs = open_bitstream("-", "r") ;
  sf = open_shannon_fano() ;
  entier = open_intstream(bs, Shannon_fano, sf) ;
  entier_signe = open_intstream(bs, Shannon_fano, sf) ;

  decompresse(entier, entier_signe, hauteur*largeur, t) ;

  close_intstream(entier) ;
  close_intstream(entier_signe) ;
  close_bitstream(bs) ;

  /*
   * Met dans la matrice
   */
  pt = t ;
  while( hauteur != 1 || largeur != 1 )
    {
      for(j=0; j<hauteur; j++)
	for(i=0; i<largeur; i++)
	  if ( j>=(hauteur+1)/2 || i>=(largeur+1)/2 )
	      image[j][i] = *pt++ ;

      hauteur = (hauteur+1)/2 ;
      largeur = (largeur+1)/2 ;
    }
  image[0][0] = *pt++ ;

  free(t) ;
 }
  
/*
 * Programme de test.
 * La ligne suivante compile, compresse et décompresse l'image
 * et affiche la taille compressée.

export QUALITE=1  # Qualité de "quantification"
export SHANNON=1  # Si 1, utilise shannon-fano dynamique
ondelette <DONNEES/bat710.pgm 1 >xxx && ls -ls xxx && ondelette_inv <xxx | xv -

 */

void ondelette_encode_image(float qualite)
 {
  struct image *image ;
  float **im ;
  int i, j ;

  image = lecture_image(stdin) ;
  assert(fwrite(&image->hauteur, 1, sizeof(image->hauteur), stdout)
	 == sizeof(image->hauteur)) ;
  assert(fwrite(&image->largeur, 1, sizeof(image->largeur), stdout)
	 == sizeof(image->largeur));
  assert(fwrite(&qualite       , 1, sizeof(qualite)       , stdout)
	 == sizeof(qualite));

  im = allocation_matrice_float(image->hauteur, image->largeur) ;
  for(j=0; j<image->hauteur; j++)
    for(i=0; i<image->largeur; i++)
      im[j][i] = image->pixels[j][i] ;

  fprintf(stderr, "Compression ondelette, image %dx%d\n"
	  , image->largeur, image->hauteur) ;
  ondelette_2d     (im, image->hauteur, image->largeur         ) ;
  fprintf(stderr, "Quantification qualité = %g\n", qualite) ;
  quantif_ondelette(im, image->hauteur, image->largeur, qualite) ;
  fprintf(stderr, "Codage\n") ;
  codage_ondelette (im, image->hauteur, image->largeur, stdout ) ;

  //  affiche_matrice_float(im, image->hauteur, image->largeur) ;
 }

void ondelette_decode_image()
 {
  int hauteur, largeur ;
  float qualite ;
  struct image *image ;
  float **im ;

  assert(fread(&hauteur, 1, sizeof(hauteur), stdin) == sizeof(hauteur)) ;
  assert(fread(&largeur, 1, sizeof(largeur), stdin) == sizeof(largeur)) ;
  assert(fread(&qualite, 1, sizeof(qualite), stdin) == sizeof(qualite)) ;

  im = allocation_matrice_float(hauteur, largeur) ;

  fprintf(stderr, "Décodage\n") ;
  decodage_ondelette   (im, hauteur, largeur, stdin ) ;

  fprintf(stderr, "Déquantification qualité = %g\n", qualite) ;
  dequantif_ondelette  (im, hauteur, largeur, qualite) ;

  fprintf(stderr, "Décompression ondelette, image %dx%d\n"
	  , largeur, hauteur) ;
  ondelette_2d_inverse (im, hauteur, largeur         ) ;

  //  affiche_matrice_float(im, hauteur, largeur) ;
  image = creation_image_a_partir_de_matrice_float(im, hauteur, largeur) ;
  ecriture_image(stdout, image) ;
 }


