#include "bases.h"
#include "matrice.h"
#include "dct.h"

/*
 * La fonction calculant les coefficients de la DCT (et donc de l'inverse)
 * car la matrice de l'inverse DCT est la transposée de la matrice DCT
 *
 * Cette fonction prend beaucoup de temps.
 * il faut que VOUS l'utilisiez le moins possible (UNE SEULE FOIS)
 *
 * FAITES LES CALCULS EN "double"
 *
 * La valeur de Pi est : M_PI
 *
 * Pour ne pas avoir de problèmes dans la suite du TP, indice vos tableau
 * avec [j][i] et non [i][j].
 */
void coef_dct(int nbe, float **table)
{
	int i;
	int j;
	double racine_deux_sur_racine_nbe = sqrt(2) / sqrt(nbe);
	double un_sur_racine_nbe = 1/sqrt(nbe);

	for(i = 0 ; i < nbe ; i++)
	{
		for(j = 0 ; j < nbe ; j++)
		{
			if(j == 0) 
			 	table[j][i] = un_sur_racine_nbe;
			else
			 	table[j][i] = racine_deux_sur_racine_nbe * 
			 					cos( j * (2 * i + 1) * M_PI / (2*nbe) ); 
		}
	}
}



/*
 * La fonction calculant la DCT ou son inverse.
 *
 * Cette fonction va être appelée très souvent pour faire
 * la DCT du son ou de l'image (nombreux paquets).
 */
void dct(int   inverse,		/* ==0: DCT, !=0 DCT inverse */
	 int nbe,		/* Nombre d'échantillons  */
	 const float *entree,	/* Le son avant transformation (DCT/INVDCT) */
	 float *sortie		/* Le son après transformation */
	 )
{


	float** coeff_DCT;
	float** DCT;
	//Allocation de la matrice DCT
	coeff_DCT = allocation_matrice_carree_float(nbe);
	//On la rempli avec les coeff de la DCT
	coef_dct(nbe, coeff_DCT);

	if(inverse)
	{
		DCT = allocation_matrice_carree_float(nbe);
		//Transposé de DCT
		transposition_matrice_carree(nbe, coeff_DCT, DCT);
		//Libération de  la matrice de coeff
		liberation_matrice_carree_float(coeff_DCT, nbe);
	}
	else
	{
		//On veut la DCT et non son inverse
		DCT = coeff_DCT;
	}

	//Produit du vecteur son par la matrice DCT
	produit_matrice_carree_vecteur(nbe, DCT, entree, sortie);
	//Liberation de la matrice DCT
	liberation_matrice_carree_float(DCT, nbe);
}
