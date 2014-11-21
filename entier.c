#include "bits.h"
#include "entier.h"

/*
 * Les fonctions de ce fichier permette d'encoder et de décoder
 * des entiers en utilisant des codes statiques.
 */

/*
 * Codage d'un entier (entre 0 et 32767 inclus) en une chaine de bits
 * qui est écrite dans le bitstream.
 *
 * Le nombre est codé par la concaténation du PREFIXE et SUFFIXE
 * Le suffixe est en fait le nombre entier sauf le premier bit a 1
 * 
 * Nombre de bits |    PRÉFIXE     | nombres codés | SUFFIXE
 *       0        |       00       |      0        |
 *     	 1        |       010      |  1 (pas 0)    |
 *     	 2        |       011      |     2-3       | 2=0 3=1
 *     	 3        |      1000      |     4-7       | 4=00 5=01 6=10 7=11
 *     	 4        |      1001      |     8-15      | 8=000 ... 15=111
 *     	 5        |      1010      |    16-31      | 16=0000 ... 31=1111
 *     	 6        |      1011      |    32-63      |
 *     	 7        |      11000     |    64-127     |
 *     	 8        |      11001     |   128-255     |
 *     	 9        |      11010     |   256-511     |
 *     	 10       |      11011     |   512-1023    |
 *     	 11       |      11100     |  1024-2047    |
 *     	 12       |      11101     |  2048-4097    |
 *     	 13       |      11110     |  4096-8191    |
 *     	 14       |      111110    |  8192-16383   |
 *     	 15       |      111111    | 16384-32767   |
 *
 * Je vous conseille de faire EXIT si l'entier est trop grand.
 *
 */

static char *prefixes[] = { "00", "010", "011", "1000", "1001", "1010", "1011",
			    "11000", "11001", "11010", "11011", "11100",
			    "11101", "11110", "111110", "111111" } ;

void put_entier(struct bitstream *b, unsigned int f)
{
  int i;
  int nb_bits = nb_bits_utile(f);

  //Je put le prefixe
  if(nb_bits > TAILLE(prefixes))
    EXIT;

  put_bit_string(b, prefixes[nb_bits]);
  for(i = 0; i<nb_bits-1; i++)
    {
      //Je put le reste en enlevant le premier bit à 1
      put_bit(b, prend_bit(f, nb_bits-2-i));
    }
}

/*
 * Cette fonction fait l'inverse de la précédente.
 *
 * Un implémentation propre, extensible serait d'utiliser
 * un arbre binaire comme pour le décodage d'Huffman.
 * Ou bien parcourir l'arbre des états 8 bits par 8 bits (voir le cours)
 * Mais je ne vous le demande pas
 */

unsigned int get_entier(struct bitstream *b)
{
  //Faire un tableau pour stocker le prefixe
  char tab[50];
  int indice_tab = 0;
  int indice_prefixe;
  int res = 0;
  int parcour_bits;
  Booleen lu;
  Booleen prefixe_trouve = 0;

  while(prefixe_trouve == 0)
    {
      //Je lis mon bit et le stocke dans le tab
      lu = get_bit(b);
      if(lu)
	tab[indice_tab] = '1';
      else
	tab[indice_tab] = '0';
		
      tab[indice_tab+1] = '\0';	

      //Pour tous les prefixes
      for(indice_prefixe = 0; indice_prefixe < 16; indice_prefixe++)
	{
	  //On test si c'est égal à ce qu'on à déjà récupéré
	  if(strcmp(prefixes[indice_prefixe],tab) == 0)
	    {
	      prefixe_trouve = 1;
	      break;
	    }
	}
      indice_tab++;
    }

  //Les deux premier cas spéciaux 0 et 1
  if(indice_prefixe == 0)	
    return 0;
  if(indice_prefixe == 1)
    return 1;

  //Pour les autres cas
  res = pose_bit(res, indice_prefixe-1, 1);
  parcour_bits = indice_prefixe-2;
  while(parcour_bits>=0)
    {
      res = pose_bit(res, parcour_bits, get_bit(b));
      parcour_bits--;		
    }
  return res ; /* pour enlever un warning du compilateur */
}

/*
 * Operation sur des entiers signés
 *
 * Si l'entier est signé, il est précédé d'un bit à 1:negatif et 0:positif
 * On considère que l'entier 0 est positif donc on pourra ajouter
 * 1 aux nombres négatif pour récupérer la place du zero négatif.
 *    2 --> 0 2
 *    1 --> 0 1
 *    0 --> 0 0
 *   -1 --> 1 0
 *   -2 --> 1 1
 *   -3 --> 1 2
 *
 */

void put_entier_signe(struct bitstream *b, int i)
{
  if(i < 0)
    { put_bit(b, 1) ; put_entier(b, -(i+1)); }
  else
    { put_bit(b, 0) ; put_entier(b, i)     ; }
}

/*
 *
 */
int get_entier_signe(struct bitstream *b)
{
  //Si on recupère un 1 en premier bit
  if(get_bit(b))
    //On retourne l'entier en ajoutant 1 et inversant son signe
    return -(get_entier(b)+1);
  return get_entier(b); 
}









