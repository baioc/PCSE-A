/* Copyright 2003   R. Civalero   L.G.P.L.
   Modifié par Franck Rousseau et Simon Nieuviarts
   Inspiré du fichier list.h de Linux 

	queue.h : Gestion de files génériques avec priorité
	          A priorité égale, comportement FIFO.
	
	Elle est implémentée par une liste circulaire doublement
	chainée. La file est triée par ordre croissant de priorité 
	lors de l'ajout. Donc l'élément prioritaire qui doit sortir 
	en premier est le dernier de la liste, c'est à dire l'élément 
	précédent la tête de liste.
	
	Pour créer une file :
	 1) Créer une tête de liste de type 'link',
	 2) L'initialiser avec LIST_HEAD_INIT,
	 3) APRES les avoir crées, ajouter des élements dans la file 
	    avec la macro queue_add, 
	Les éléments doivent être des structures contenant au moins un 
	champ de type 'link' et un champ de type 'int' pour la priorité.
	 	
	Pour utiliser la file :
	 - Supprimer des éléments particulier avec la macro queue_del,
	 - Récuperer et enlever de la file l'élement prioritaire avec 
	   la macro queue_out,
	 - Ajouter d'autre éléments avec la macro queue_add,
	 - Tester si la file est vide avec la fonction queue_empty
	 
	Attention : certains pointeurs pointent vers des éléments des
	            files, alors que d'autres pointent vers le champ
		    du lien de chainage de ces éléments.
		    Les têtes de file/liste sont des liens de chainage
		    de type 'link' et non des éléments.
*/

#ifndef QUEUE_H
#define QUEUE_H

#include "debug.h"

/**
 * Type structure pour faire les liens de chainage
 */
typedef struct list_link {
	struct list_link *prev;
	struct list_link *next;
} link;

/**
 * Initialisation d'une tête de liste (tête de file).
 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_link name = LIST_HEAD_INIT(name)
#define INIT_LIST_HEAD(ptr) do { struct list_link *__l = (ptr); __l->next = __l; __l->prev = __l; } while (0)

/**
 * Initialisation d'un maillon de liste.
 *
 * Si vous pensez en avoir besoin, il est fort probable que ce soit une erreur.
 * Les zones mémoires données par l'allocateur ou allouées à la compilation
 * sont initialisées à 0 (respectivement par l'allocateur et par le crt0).
 */
#define INIT_LINK(ptr) do { struct list_link *__l = (ptr); __l->next = 0; __l->prev = 0; } while (0)

/**
 * Ajout d'un élément dans la file avec tri par priorité
 * ptr_elem  : pointeur vers l'élément à chainer
 * head      : pointeur vers la tête de liste
 * type      : type de l'élément à ajouter
 * listfield : nom du champ du lien de chainage
 * priofield : nom du champ de la priorité
 */
#define queue_add(ptr_elem, head, type, listfield, priofield)                 \
	do {                                                                  \
		link *__cur_link=head;                                        \
		type *__elem = (ptr_elem);                                    \
		link *__elem_link=&((__elem)->listfield);                     \
                assert((__elem_link->prev == 0) && (__elem_link->next == 0)); \
		do  __cur_link=__cur_link->next;                              \
	   	while ( (__cur_link != head) &&                               \
		        (((queue_entry(__cur_link,type,listfield))->priofield)\
	     	               < ((__elem)->priofield)) );                    \
	   	__elem_link->next=__cur_link;                                 \
	   	__elem_link->prev=__cur_link->prev;                           \
	   	__cur_link->prev->next=__elem_link;                           \
	   	__cur_link->prev=__elem_link;                                 \
	} while (0)

/**
 * Macro à usage interne utilisée par la macro queue_add
 * Récupération du pointeur vers l'objet correspondant
 *   (On calcule la différence entre l'adresse d'un élément et l'adresse
 *   de son champ de type 'link' contenant les liens de chainage)
 * ptr_link  : pointeur vers le maillon
 * type      : type de l'élément à récupérer 
 * listfield : nom du champ du lien de chainage
 */
#define queue_entry(ptr_link, type, listfield) \
	((type *)((char *)(ptr_link)-(unsigned long)(&((type *)0)->listfield)))


/**
 * Tester si une file est vide
 * head : pointeur vers la tête de liste
 * retourne un entier (0 si pas vide)
 */
static __inline__ int queue_empty(link *head)
{
	return (head->next == head);
}


/**
 * Retrait de l'élément prioritaire de la file
 * head      : pointeur vers la tête de liste
 * type      : type de l'élément à retourner par référence
 * listfield : nom du champ du lien de chainage
 * retourne un pointeur de type 'type' vers l'élément sortant
 */
#define queue_out(head, type, listfield) \
	(type *)__queue_out(head,(unsigned long)(&((type *)0)->listfield))

/**
 * Fonction à usage interne utilisée par la macro ci-dessus
 * head : pointeur vers la tête de liste
 * diff : différence entre l'adresse d'un élément et son champ de 
 *        type 'link' (cf macro list_entry)
 */
static __inline__ void *__queue_out(link *head, unsigned long diff)
{
	//On récupère un pointeur vers le maillon
	//du dernier élément de la file.
	unsigned long ptr_link_ret=(unsigned long)(head->prev);
	
	//Si la file est vide, on retourne le pointeur NULL.
	if (queue_empty(head)) return ((void *)0);

	//Sinon on retire l'élément de la liste,
	head->prev=head->prev->prev;
	head->prev->next=head;

	((link *)ptr_link_ret)->prev = 0;
	((link *)ptr_link_ret)->next = 0;

	//Et on retourne un pointeur vers cet élément.	
	return ((void *)(ptr_link_ret-diff));
}


/**
 * Suppression d'un élément dans la file
 * ptr_elem  : pointeur vers l'élément à supprimer
 * listfield : nom du champ du lien de chainage
 */
#define queue_del(ptr_elem, listfield)                                       \
	do {                                                                 \
		link *__elem_link=&((ptr_elem)->listfield);                  \
                assert((__elem_link->prev != 0) && (__elem_link->next != 0)); \
		__elem_link->prev->next=__elem_link->next;                   \
	   	__elem_link->next->prev=__elem_link->prev;                   \
                __elem_link->next = 0;                                       \
                __elem_link->prev = 0;                                       \
	} while (0)


/**
 * Parcours d'une file
 * ptr_elem  : pointeur vers un élément utilisé comme itérateur de boucle
 * head      : pointeur vers la tête de liste
 * type      : type des éléments de la liste
 * listfield : nom du champ du lien de chainage
 */
#define queue_for_each(ptr_elem, head, type, listfield)                      \
	for (ptr_elem = queue_entry((head)->next,type,listfield);            \
             &ptr_elem->listfield != (head);                                 \
             ptr_elem = queue_entry(ptr_elem->listfield.next,type,listfield))


/**
 * Parcours d'une file en sens inverse
 * ptr_elem  : pointeur vers un élément utilisé comme itérateur de boucle
 * head      : pointeur vers la tête de liste
 * type      : type des éléments de la liste
 * listfield : nom du champ du lien de chainage
 */
#define queue_for_each_prev(ptr_elem, head, type, listfield)                 \
	for (ptr_elem = queue_entry((head)->prev,type,listfield);            \
             &ptr_elem->listfield != (head);                                 \
             ptr_elem = queue_entry(ptr_elem->listfield.prev,type,listfield))


/**
 * Recuperer un pointeur vers l'element prioritaire de la file
 * sans l'enlever de la file.
 * head : pointeur vers la tête de liste
 * type : type de l'élément à retourner par référence
 * listfield : nom du champ du lien de chainage
 * retourne un pointeur de type 'type' vers l'élément prioritaire
 */
#define queue_top(head, type, listfield) \
(type *)__queue_top(head,(unsigned long)(&((type *)0)->listfield))
 
/**
 * Fonction à usage interne utilisée par la macro ci-dessus
 * head : pointeur vers la tête de liste
 * diff : différence entre l'adresse d'un élément et son champ de
 * type 'link' (cf macro list_entry)
 */
static __inline__ void *__queue_top(link *head, unsigned long diff)
{
	//On récupère un pointeur vers le maillon 
	//du dernier élément de la file. 
	unsigned long ptr_link_ret=(unsigned long)(head->prev); 
 
	//Si la file est vide, on retourne le pointeur NULL. 
	if (queue_empty(head)) return ((void *)0); 
 
	//Sinon retourne un pointeur vers cet élément. 
	return ((void *)(ptr_link_ret-diff)); 
}


/**
 * Recuperer un pointeur vers l'element le moins prioritaire de la file
 * sans l'enlever de la file.
 * head : pointeur vers la tête de liste
 * type : type de l'élément à retourner par référence
 * listfield : nom du champ du lien de chainage
 * retourne un pointeur de type 'type' vers l'élément prioritaire
 */
#define queue_bottom(head, type, listfield) \
(type *)__queue_bottom(head,(unsigned long)(&((type *)0)->listfield))

/**
 * Fonction à usage interne utilisée par la macro ci-dessus
 * head : pointeur vers la tête de liste
 * diff : différence entre l'adresse d'un élément et son champ de
 * type 'link' (cf macro list_entry)
 */
static __inline__ void *__queue_bottom(link *head, unsigned long diff)
{
	//On recupere un pointeur vers le maillon
	//du premier element de la file.
	unsigned long ptr_link_ret=(unsigned long)(head->next);

	//Si la file est vide, on retourne le pointeur NULL.
	if (queue_empty(head)) return ((void *)0);

	//Sinon retourne un pointeur vers cet element.
	return ((void *)(ptr_link_ret-diff));
}



#define LIST_HEAD_INIT_PROC(x) assert(!"utiliser INIT_LIST_HEAD au lieu de LIST_HEAD_INIT_PROC")

#endif
