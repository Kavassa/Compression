/*
 * Le but du shannon-fano dynamique est de ne pas transmettre la table
 * des occurrences. Pour ce faire, on ajoute à la table un symbole ESCAPE
 * qui permet l'ajout d'éléments à la table.
 * Le décompresseur sait qu'après un événement ESCAPE il trouvera
 * la valeur (et non le code) d'un événement à ajouter à la table.
 */


#include "bits.h"
#include "sf.h"

#define VALEUR_ESCAPE 0x7fffffff /* Plus grand entier positif */

struct evenement
{
  int valeur ;
  int nb_occurrences ;
} ;

struct shannon_fano
{
  int nb_evenements ;
  struct evenement evenements[200000] ;
} ;



/*
 * Allocation des la structure et remplissage des champs pour initialiser
 * le tableau des événements avec l'événement ESCAPE (avec une occurrence).
 */
struct shannon_fano* open_shannon_fano()
{
  struct shannon_fano* sf_retourne;
  ALLOUER(sf_retourne, 1);

  sf_retourne->nb_evenements = 1;
  sf_retourne->evenements[0].nb_occurrences = 1;
  sf_retourne->evenements[0].valeur = VALEUR_ESCAPE;

  return sf_retourne ;
}



/*
 * Fermeture (libération mémoire)
 */
void close_shannon_fano(struct shannon_fano *sf)
{
  free(sf);
}



/*
 * En entrée l'événement (sa valeur, pas son code shannon-fano).
 * En sortie la position de l'événement dans le tableau "evenements"
 * Si l'événement n'est pas trouvé, on retourne la position
 * de l'événement ESCAPE.
 */
static int trouve_position(const struct shannon_fano *sf, int evenement)
{
  int indice_evenement;
  for(indice_evenement = 0; indice_evenement < sf->nb_evenements; indice_evenement++)
  {
    if(sf->evenements[indice_evenement].valeur == evenement)
      return indice_evenement;
  }
  //Sinon on return la position de l'evenement ESCAPE
  return trouve_position(sf, VALEUR_ESCAPE);
}



/*
 * Soit le sous-tableau "evenements[position_min..position_max]"
 * Les bornes sont incluses.
 *
 * LES FORTES OCCURRENCES SONT LES PETITS INDICES DU TABLEAU
 *
 * Il faut trouver la séparation.
 * La séparation sera codée comme l'indice le plus fort
 * des fortes occurrences.
 *
 * La séparation minimise la valeur absolue de la différence
 * de la somme des occurrences supérieures et inférieures.
 *
 * L'algorithme (trivial) n'est pas facile à trouver, réfléchissez bien.
 */
static int trouve_separation(const struct shannon_fano *sf
			     , int position_min
			     , int position_max)
{
  int somme_totale = 0;
  int somme_sup = 0;
  int somme_inf ;
  int i;

  //On calcule la somme des valeurs evenements
  for(i = position_min; i<= position_max; i++)
  {
    somme_totale += sf->evenements[i].nb_occurrences;
  }
  somme_inf = somme_totale;

  while(position_min <= position_max)
  {
    somme_inf -= sf->evenements[position_min].nb_occurrences;
    if(somme_inf <= (somme_totale/2))
    {
      somme_sup = somme_inf + sf->evenements[position_min].nb_occurrences;
      break;
    }
    position_min++;
  }
  
  //On calcul la difference de chaque coté de la frontiere puis on prend la diff la plus petite
  if(somme_sup - (somme_totale/2) < (somme_totale/2) - somme_inf)
    return position_min-1;
  return position_min;
}



/*
 * Cette fonction (simplement itérative)
 * utilise "trouve_separation" pour générer les bons bit dans "bs"
 * le code de l'événement "sf->evenements[position]".
 */
static void encode_position(struct bitstream *bs,struct shannon_fano *sf,
			    int position)
{
  int position_min = 0;
  int position_max = sf->nb_evenements-1;
  int position_separation; 

  while(position_max != position_min)
  {
    position_separation = trouve_separation(sf, position_min, position_max);
    //Si on est la partie gauche (fortes occurences) on met 0 et on décale le max
    if(position <= position_separation)
    {
      put_bit(bs, 0);
      position_max = position_separation;
    }
    else//Sinon partie droite on met 1 et on decale le min
    {
      put_bit(bs, 1);
      position_min = position_separation+1;
    }
  }
}



/*
 * Cette fonction incrémente le nombre d'occurrence de
 * "sf->evenements[position]"
 * Puis elle modifie le tableau pour qu'il reste trié par nombre
 * d'occurrence (un simple échange d'événement suffit)
 *
 * Les faibles indices correspondent aux grand nombres d'occurrences
 */

static void incremente_et_ordonne(struct shannon_fano *sf, int position)
{
  struct evenement evenement_temp;
  sf->evenements[position].nb_occurrences++;

  //Revoir le for car cas inutiles
  /*for(position = position; position > 0; position--)
  {
    if(sf->evenements[position-1].nb_occurrences < sf->evenements[position].nb_occurrences)
    {
      evenement_temp = sf->evenements[position-1];
      sf->evenements[position-1] = sf->evenements[position];
      sf->evenements[position] = evenement_temp;
    }		
  } */

  while( (sf->evenements[position-1].nb_occurrences < 
          sf->evenements[position].nb_occurrences) &&
          position > 0) 
  {
    evenement_temp = sf->evenements[position-1];
    sf->evenements[position-1] = sf->evenements[position];
    sf->evenements[position] = evenement_temp;
    position--;
  }  
}



/*
 * Cette fonction trouve la position de l'événement puis l'encode.
 * Si la position envoyée est celle de ESCAPE, elle fait un "put_bits"
 * de "evenement" pour envoyer le code du nouvel l'événement.
 * Elle termine en appelant "incremente_et_ordonne" pour l'événement envoyé.
 */
void put_entier_shannon_fano(struct bitstream *bs
			     ,struct shannon_fano *sf, int evenement)
{
  int position_evenement = trouve_position(sf ,evenement);
  encode_position(bs, sf, position_evenement);

  if(sf->evenements[position_evenement].valeur == VALEUR_ESCAPE)
  {
    put_bits(bs, sizeof(evenement)*8, evenement);

    sf->evenements[sf->nb_evenements].valeur = evenement;
    sf->evenements[sf->nb_evenements].nb_occurrences = 1;
    sf->nb_evenements++;
  }
  
  incremente_et_ordonne(sf, position_evenement);
}



/*
 * Fonction inverse de "encode_position"
 */
static int decode_position(struct bitstream *bs,struct shannon_fano *sf)
{
  int position_min = 0;
  int position_max = sf->nb_evenements-1;

  int separation;

  while(position_max != position_min)
  {
    separation = trouve_separation(sf, position_min, position_max);
    
    //Si le bit est a 0 on travaille sur la partie 'gauche' du tableau (forte occurrences)
    if(get_bit(bs) == 0)
      position_max = separation;
    else //Sinon sur la partie droite
      position_min = separation + 1;
  }

  return position_min;
}



/*
 * Fonction inverse de "put_entier_shannon_fano"
 *
 * Attention au piège : "incremente_et_ordonne" change le tableau
 * donc l'événement trouvé peut changer de position.
 */
int get_entier_shannon_fano(struct bitstream *bs, struct shannon_fano *sf)
{

  //Je decode pour trouver la position

  //Si l'evenement à cette position est escape
    //x = getbit(sizeof(int)*8)
    //incrementeEtOrdonne
    //j'ajoute le nouvel evenement à la fin avec une valeur de x occurences a 1 et j'incremente le nombre devenements
  //sinon
    //x = valeur de levenement a la position
  //incremente et ordonne

  //retourne x

  int position_element_decode = decode_position(bs, sf);
  int valeur_recuperee;

  if(sf->evenements[position_element_decode].valeur == VALEUR_ESCAPE)
  {
    valeur_recuperee = get_bits(bs, sizeof(valeur_recuperee)*8);

    sf->evenements[sf->nb_evenements].valeur = valeur_recuperee;
    sf->evenements[sf->nb_evenements].nb_occurrences = 1;
    sf->nb_evenements++;
  }
  else
    valeur_recuperee = sf->evenements[position_element_decode].valeur;
  
  incremente_et_ordonne(sf, position_element_decode);

  return valeur_recuperee;
}



/*
 * Fonctions pour les tests, NE PAS MODIFIER, NE PAS UTILISER.
 */
int sf_get_nb_evenements(struct shannon_fano *sf)
{
  return sf->nb_evenements ;
}
void sf_get_evenement(struct shannon_fano *sf, int i, int *valeur, int *nb_occ)
{
  *valeur = sf->evenements[i].valeur ;
  *nb_occ = sf->evenements[i].nb_occurrences ;
}
int sf_table_ok(const struct shannon_fano *sf)
{
  int i, escape ;

  escape = 0 ;
  for(i=0;i<sf->nb_evenements;i++)
    {
      if ( i != 0
	   && sf->evenements[i-1].nb_occurrences<sf->evenements[i].nb_occurrences)
	{
	  fprintf(stderr, "La table des événements n'est pas triée\n") ;
	  return(0) ;
	}
      if ( sf->evenements[i].valeur == VALEUR_ESCAPE )
	escape = 1 ;
    }
  if ( escape == 0 )
    {
      fprintf(stderr, "Pas de ESCAPE dans la table !\n") ;
      return(0) ;
    }
  return 1 ;
}
