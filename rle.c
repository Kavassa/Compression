#include "bases.h"
#include "intstream.h"
#include "rle.h"

/*
 * Avant propos sur les "intstream"
 *
 * Un "intstream" permet de stocker des entiers dans un fichier.
 * A son ouverture (faite par les programmes de test) une méthode
 * de codage de l'entier lui est associée.
 * Dans notre cas, cela sera les algorithmes développés
 * dans "entier.c" ou "sf.c"
 *
 * Il faut remarquer que deux "intstream" peuvent permettre
 * d'écrire dans le même "bitstream". Les données sont intercalées
 * dans le fichier, pour les récupérer il faut les relire en faisant
 * les lectures dans le même ordre que les écritures.
 */

/*
 * RLE : Run Length Encoding
 *
 * Version spécifique pour stocker les valeurs de la DCT.
 *
 * Au lieu de coder un entier par valeur
 * on va coder le nombre d'entiers qui sont nuls (entier positif)
 * suivi de la valeur du premier entier non nul.
 *
 * Par exemple pour coder le tableau :
 * 	5 8 0 0 4 0 0 0 0 2 1 0 0 0
 *    On stocke dans les deux "intstream"
 *      0 0     2         4 0       3      Nombres de 0
 *      5 8     4         2 1              La valeur différentes de 0
 * Comme les deux "intstream" sont stockés dans le même fichier
 * il faut absolument lire et écrire les valeurs dans le même ordre.
 *     (0,5) (0,8) (2,4) (4,2) (0,1) (3)
 */

/*
 * Stocker le tableau de flottant dans les deux "instream"
 * En perdant le moins d'information possible.
 */

void compresse(struct intstream *entier, struct intstream *entier_signe
	       , int nbe, const float *dct)
{

	int indice_dct;
	int nb_zero = 0;
	int valeur_dct;
	for(indice_dct = 0; indice_dct < nbe; indice_dct++)
	{
		valeur_dct = rint(dct[indice_dct]);
		//Si l'on trouve un 0 on incrément le nombre de zero et on poursuit
		if( valeur_dct )
		{

			//On stocke le nombre de zero que l'on a vu avant
			put_entier_intstream(entier, nb_zero);
			//On stocke cette valeur
			put_entier_intstream(entier_signe, valeur_dct);
			//On remet a 0
			nb_zero = 0;
		}
		else
			nb_zero++;		

	}
	//Si on finit par des 0 ne pas oublier d'ajouter le nombre de 0
	if( nb_zero )
		put_entier_intstream(entier, nb_zero);
}

/*
 * Lit le tableau de flottant qui est dans les deux "instream"
 */

void decompresse(struct intstream *entier, struct intstream *entier_signe
		 , int nbe, float *dct)
{
	int nb_zero;
	int indice_dct;
	for(indice_dct = 0; indice_dct < nbe; indice_dct++)
	{
		//On commence par ajouter des zeros si on en a
		nb_zero = get_entier_intstream(entier);
		//Boucle sur le nombre de zero avec decalage de l'indice
		while(nb_zero--)
		{
			dct[indice_dct] = 0;
			indice_dct++;
		}
		//Faire attentionau cas ou la chaine se finissait avec des 0, à ne pas
		//aller dans une case du tableau qui n'existe pas vu que l'on incremente
		//l'indice en dehors de la condition du for 
		if(indice_dct < nbe)
			dct[indice_dct] = get_entier_intstream(entier_signe);
	}
}
