/*
 * shared_memory.c
 *
 *  Created on: 24/03/2021
 *      Author: Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "shared_memory.h"

#include "hash.h"
#include "mem.h"
#include "debug.h"
#include "stddef.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define OBJECT_SIZE 4096

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

hash_t shm_map;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void shm_init()
{
  hash_init_string(&shm_map);
}

/*
 * Create a shared memory object and stores it into a hash table with argument
 * key as the index.
 * If an object by that key already exists, return NULL.
 * Created objects are 4096 bytes long.
 */
void *shm_create(const char *key)
{
  // a shared object by that name already exists
  if (hash_isset(&shm_map, (void *)key) != 0) return NULL;

  // size inferred from test 21
  void *object_addr = mem_alloc(OBJECT_SIZE);
  // add that memory object in the hash table
  hash_set(&shm_map, (void *)key, object_addr);

  return object_addr;
}

/*
 * Returns the address of memory object referenced by argument key.
 * If that key doesn't exist in shm_map, returns NULL.
 */
void *shm_acquire(const char *key)
{
  return hash_get(&shm_map, (void *)key, (void *)NULL);
}

/*
 * Delete a memory object referenced by key in shm_map hash table.
 * If that key doesn't exist in shm_map, does nothing.
 */
void shm_release(const char *key)
{
  void *object_addr = hash_get(&shm_map, (void *)key, (void *)NULL);
  if (object_addr == NULL) return;

  mem_free(object_addr, OBJECT_SIZE);
  assert(hash_del(&shm_map, (void *) key) == 0);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
