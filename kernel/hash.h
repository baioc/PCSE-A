/*
 * Damien Dejean - Gaëtan Morin
 * Ensimag, Projet Système 2010
 *
 * XUNIL
 * Table de hachage générique
 */

#ifndef ___HASH_H___
#define ___HASH_H___

typedef long (*hash_func_t) (void *key);
typedef int (*hash_compare_t) (void *key1, void *key2);

/* Slots de la table */
typedef struct {
        void *key;
        void *value;
        long hash;
        int mode;
} hash_slot_t;

/* Structure de table de hachage */
typedef struct {
        long fill;              /* Nombre de slots actifs + supprimés */
        long count;             /* Nombre de slots actifs */
        unsigned long mask;     /* Masque d'index (nombre de slots - 1) */
        hash_slot_t *table;     /* Tableau dynamique des slots */
        hash_func_t hfunc;      /* Fonction de hachage */
        hash_compare_t cmp;     /* Comparateur de clés */
} hash_t;




/* ===== Initialisation / Destruction ===== */ 

/*
 * Initialise une table de hachage
 * map   : table de hachage à initialiser
 * hfunc : fonction calculant le hashcode à partir d'une clé
 * cmp   : fonction comparant deux clés (renvoie 0 si les 2 clés
 *         sont différentes)
 */
int hash_init(hash_t *map, hash_func_t hfunc, hash_compare_t cmp);

/*
 * Initialise une table de hachage en mode 'direct'.
 * Le hashcode est la valeur du pointeur lui-même et deux clés
 * sont considérées comme égales si les pointeurs sont égaux.
 */
int hash_init_direct(hash_t *map);

/*
 * Initialise une table de hachage pour une indexation sur des chaînes de
 * caractère.
 * NB : Pour une table de hachage initialisée avec cette fonction, les
 * clés doivent être des 'char *'.
 */
int hash_init_string(hash_t *map);

/*
 * Libère la mémoire allouée par les fonctions d'initialisation.
 * Après appel à cette fonction, <map> ne peut plus être utilisée
 * jusqu'au prochain appel à hash_init...().
 */
void hash_destroy(hash_t *map);



/* ===== Utilisation de la table ===== */ 

/*
 * Renvoie une valeur non nulle si la clé <key> est présente dans la
 * table de hachage <map>.
 */
int hash_isset(hash_t *map, void *key);

/*
 * Retourne la valeur associée à la clé <key> dans la table de hachage
 * <map>. Si la clé n'est pas présente, retourne <default_value>.
 */
void *hash_get(hash_t *map, void *key, void *default_value);

/*
 * Associe la valeur <value> à la clé <key> dans la table de hachage
 * <map>. Si la clé n'est pas présente, elle est insérée, sinon sa
 * valeur associée est mise à jour.
 * Retourne 0 si l'affectation a réussit.
 */
int hash_set(hash_t *map, void *key, void *value);

/*
 * Supprime la clé <key> et sa valeur associée de la table de hachage
 * <map>.
 * Retourne 0 si la suppression a réussit.
 */
int hash_del(hash_t *map, void *key);

#endif

