/*
 * Damien Dejean - Gaëtan Morin
 * Ensimag, Projet Système 2010
 *
 * XUNIL
 * Table de hachage générique
 */

#include "hash.h"
#include "stddef.h"
#include "string.h"
#include "mem.h"


#define HASH_MINSIZE 8

/* Modes des slots */
enum slot_mode {
        SL_EMPTY,       /* N'a jamais contenu de couple clé/valeur */
        SL_ACTIVE,      /* Couple clé/valeur valide */
        SL_DELETED      /* Couple supprimé */
};


/*
 * Réinitialise une table avec un tableau interne de taille <size>.
 * Si l'allocation échoue, retourne -1 et laisse <map> inchangée.
 */
static int hash_initialize(hash_t *map, long size)
{
        long i;
        hash_slot_t *table = mem_alloc(size * sizeof(hash_slot_t));
        if (table == NULL)
                return -1;

        map->fill  = 0;
        map->count = 0;
        map->mask  = size-1;
        map->table = table;
        for (i = 0; i < size; ++i) {
                table[i].key   = NULL;
                table[i].value = NULL;
                table[i].hash  = 0;
                table[i].mode  = SL_EMPTY;
        }
        return 0;
}


/*
 * Algorithme de recherche inspiré de l'implémentation de
 * l'objet Python 'dict' de CPython 2.6
 */
static hash_slot_t *hash_lookup(hash_t *map, void *key, long hash)
{
        unsigned long mask = map->mask;
        unsigned long index = hash & mask;
        unsigned long perturb = hash;
        hash_slot_t *table = map->table;
        hash_slot_t *slot = &table[index];
        hash_slot_t *freeslot = NULL;

        while (1) {
                if (slot->mode == SL_EMPTY)
                        /* Si on a déjà un slot libre on le retourne */
                        return (freeslot == NULL) ? slot : freeslot;
                if (slot->mode == SL_ACTIVE &&
                                (slot->key == key
                                 || (slot->hash == hash && map->cmp(slot->key, key))
                                ))
                        return slot;
                if (slot->mode == SL_DELETED && freeslot == NULL)
                        freeslot = slot;

                /* Parcoure la table dans un ordre pseudo-aléatoire
                 * en fonction du hashcode */
                index = 5 * index + perturb + 1;
                slot =  &(map->table[index & mask]);
                perturb >>= 5;
        }
}


/*
 * Copie les slots actifs contenus dans <oldtable> de taille <oldsize>
 * dans la table <map>.
 */
static void hash_copy(hash_t *map, hash_slot_t *oldtable, long oldsize)
{
        int i;
        hash_slot_t *old, *new;

        for (i = 0; i < oldsize; ++i) {
                old = &oldtable[i];
                if (old->mode != SL_ACTIVE)
                        continue;
                new = hash_lookup(map, old->key, map->hfunc(old->key));
                new->key   = old->key;
                new->value = old->value;
                new->hash  = old->hash;
                new->mode  = SL_ACTIVE;
                ++(map->fill);
                ++(map->count);
        }
}

/*
 * Redimensionne la table si besoin
 */
static int hash_update(hash_t *map)
{
        long old_size = map->mask+1;
        long new_size = old_size;
        hash_slot_t *old_table;

        if (3 * map->fill >= 2 * old_size) {
                /* fill >= (2/3)*size */
                new_size <<= 1;
                if (new_size < old_size)
                        return -1;  /* Dépassement des entiers... */
        } else if (old_size > HASH_MINSIZE && (4 * map->count <= old_size)) {
                /* count <= (1/2)*(size/2) */
                new_size >>= 1;
        } else {
                return 0;
        }

        old_table = map->table;
        if (hash_initialize(map, new_size) == -1)
                return -1;

        hash_copy(map, old_table, old_size);
        mem_free(old_table, old_size * sizeof(hash_slot_t));
        return 0;
}


int hash_init(hash_t *map, hash_func_t hfunc, hash_compare_t cmp)
{
        if (hash_initialize(map, HASH_MINSIZE) == -1)
                return -1;

        map->hfunc = hfunc;
        map->cmp   = cmp;

        return 0;
}


void hash_destroy(hash_t *map)
{
        int size = map->mask+1;
        mem_free(map->table, size * sizeof(hash_slot_t));
        map->fill  = 0;
        map->count = 0;
        map->mask  = 0;
        map->table = NULL;
        map->hfunc = NULL;
        map->cmp   = NULL;
}


int hash_isset(hash_t *map, void *key)
{
        hash_slot_t *slot;
        if (map == NULL)
                return 0;

        slot = hash_lookup(map, key, map->hfunc(key));
        return slot->mode == SL_ACTIVE;
}


void *hash_get(hash_t *map, void *key, void *default_value)
{
        hash_slot_t *slot;
        if (map == NULL)
                return default_value;

        slot = hash_lookup(map, key, map->hfunc(key));
        if (slot->mode == SL_ACTIVE)
                return slot->value;

        return default_value;
}


int hash_set(hash_t *map, void *key, void *value)
{
        long hash;
        hash_slot_t *slot;

        if (map == NULL)
                return -1;

        hash = map->hfunc(key);
        slot = hash_lookup(map, key, hash);

        if (slot->mode == SL_ACTIVE) {
                slot->value = value;
                return 0;
        }

        ++(map->count);
        if (slot->mode == SL_EMPTY)
                ++(map->fill);

        slot->key   = key;
        slot->value = value;
        slot->hash  = hash;
        slot->mode  = SL_ACTIVE;

        return hash_update(map);
}


int hash_del(hash_t *map, void *key)
{
        hash_slot_t *slot;

        if (map == NULL)
                return -1;

        slot = hash_lookup(map, key, map->hfunc(key));

        if (slot->mode != SL_ACTIVE)
                return 0;

        slot->key   = NULL;
        slot->value = NULL;
        slot->hash  = 0;
        slot->mode  = SL_DELETED;
        --(map->count);

        return hash_update(map);
}


/* ===== Fonctions prédéfinies ===== */

static int hash_direct_cmp(void *key1, void *key2)
{
        return key1 == key2;
}

static long hash_direct_hfunc(void *key)
{
        return (long) key;
}

static int hash_string_cmp(char *key1, char *key2)
{
        return strcmp(key1, key2) == 0;
}

/* 
 * Hash de chaînes de caractères
 * Source : Daniel J. Bernstein
 */
static long hash_string_hfunc(char *key)
{
        long i;
        long hash = 5381;

        for (i = 0; key[i]; ++i)
                hash = ((hash << 5) + hash) ^ key[i];

        return hash;
}

int hash_init_direct(hash_t *map)
{
        return hash_init(map, hash_direct_hfunc, hash_direct_cmp);
}

int hash_init_string(hash_t *map)
{
        return hash_init(map,
                        (hash_func_t) hash_string_hfunc, 
                        (hash_compare_t) hash_string_cmp);
}

