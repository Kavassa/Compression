#include "image.h"


/*
 * Lecture d'une ligne du fichier.
 * On saute les lignes commençant par un "#" (commentaire)
 * On simplifie en considérant que les lignes ne dépassent pas MAXLIGNE
 */

void lire_ligne(FILE *f, char *ligne)
{
	do
	//Si le premier caractère est un # on lit la ligne suivante
	{
		fgets(ligne, MAXLIGNE, f);
	}
	while(ligne[0] == '#');
}

/*
 * Allocation d'une image
 */

struct image* allocation_image(int hauteur, int largeur)
{
	int indice_hauteur;
	struct image *img;
	
	//Allocation de l'image
	ALLOUER(img, 1);
	
	//Remplissage des champs
	img->hauteur = hauteur;
	img->largeur = largeur;
	ALLOUER(img->pixels, hauteur);
	for(indice_hauteur = 0; indice_hauteur < hauteur; indice_hauteur++)
	{
		ALLOUER(img->pixels[indice_hauteur], largeur);
	}

	return img;
}

/*
 * Libération image
 */

void liberation_image(struct image* image)
{
	int indice_hauteur;
	for(indice_hauteur = 0; indice_hauteur < image->hauteur; indice_hauteur++)
	{
		free(image->pixels[indice_hauteur]);
	}
	free(image->pixels);
	free(image);
}

/*
 * Allocation et lecture d'un image au format PGM.
 * (L'entête commence par "P5\nLargeur Hauteur\n255\n"
 * Avec des lignes de commentaire possibles avant la dernière.
 */

struct image* lecture_image(FILE *f)
{
	int indice_hauteur;
	int indice_largeur;
	/*char * ligne;
	ALLOUER(ligne, MAXLIGNE);*/
	char ligne[MAXLIGNE];
	int hauteur;
	int largeur;
	struct image* img = NULL;

	//On lit la ligne avec le P5
	lire_ligne(f, ligne);
	
	/*if(strcmp(ligne, "P5\n"))
	{*/

	//Recuperation hauteut largeur
	//Possiblité de fgets pour stoker nombre dans une chaine et recup avec atoi
	fscanf(f, "%d%d%*c", &largeur, &hauteur);
	//Allocation de l'image
	img = allocation_image(hauteur, largeur);

	//On lit d'eventuelles lignes de commentaires et le 255
	lire_ligne(f, ligne);

	for(indice_hauteur = 0; indice_hauteur < img->hauteur; indice_hauteur++)
	{
		for(indice_largeur = 0; indice_largeur < img->largeur; indice_largeur++)
		{
			img->pixels[indice_hauteur][indice_largeur] = fgetc(f);
		}
	}

	//free(ligne);

	/*}
	else
		fprintf(stderr, "*** ERREUR: lecture_image\n");*/

	return img; /* pour enlever un warning du compilateur */
}

/*
 * Écriture de l'image (toujours au format PGM)
 */

void ecriture_image(FILE *f, const struct image *image)
{
	int indice_hauteur;
	int indice_largeur;

	//On remet l'entete au debut de l'image
	fprintf(f, "P5\n");
	fprintf(f, "%d %d\n", image->largeur, image->hauteur);
	fprintf(f, "255\n");

	//On remet les valeurs de pixels
	for(indice_hauteur = 0; indice_hauteur < image->hauteur; indice_hauteur++)
	{
		for(indice_largeur = 0; indice_largeur < image->largeur; indice_largeur++)
		{
			fprintf(f, "%c", image->pixels[indice_hauteur][indice_largeur]);
			//fputc(image->pixels[indice_hauteur][indice_largeur], f);
		}
	}
}
