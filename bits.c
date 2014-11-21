#include "bitstream.h"
#include "bits.h"

/*
 * Oon écrit les "nb" bits de droite de "v"
 * dans le fichier (toujours du poids fort au faible).
 *
 * Pour v=11 nb=8 on va écrire les bits : 00001011 dans le fichier
 */

void put_bits(struct bitstream *b, unsigned int nb, unsigned long v)
{
  int i;
  //Revoir le cas nb=0
  for(i = nb-1; i>=0; i--)
    {
      put_bit(b, prend_bit(v, i));
    }
}


/*
 * Lecture de "nb" bits venant du fichier.
 * et on retourne un entier contenant ces bits cadrés à droite (poids faibles)
 * Par exemple pour nb=2 on peut retourner des valeurs de 0 à 3 inclu.
 * Suivant les 2 bits dans le fichier on obtiendra :
 * 00->0 01->1 10->2 11->3
 */

unsigned int get_bits(struct bitstream *b, unsigned int nb)
{
  int i;
  int res = 0;
  for(i = 0; i < nb ; i++)
    {
      res = pose_bit(res, nb-1-i, get_bit(b));
      // res = 2*res + get_bit(b)
    }
    return res;
}

/*
 * Pour vous simplifier la programmation.
 * Cette fonction stocke une chaine de la forme "0011010101010111010101001"
 * dans le flot de bit sous la forme d'une suite de bit 0 et 1.
 *
 * Comme d'habitude le caractère '0' c'est Faux les autres sont vrai
 */

void put_bit_string(struct bitstream *b, const char *bits)
{
  while(*bits != '\0') // On peut utiliser bit[i]
    { 
      put_bit(b, *bits++ != '0');
      /*if(*bits=='0')
    	put_bit(b, Faux);
    else
 	put_bit(b, Vrai);
	bits++;*/
  }
}
