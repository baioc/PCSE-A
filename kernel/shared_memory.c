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

// Mapping of physical addresses of pages along with the number of processes
// sharing a page
hash_t shm_map; // "key" -> (address, process_count)

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

  // object_info will contain address of shared object along with the number
  // of processes sharing that object
  int *object_info = mem_alloc(2*sizeof(int));

  void *object = mem_alloc(OBJECT_SIZE);
  *object_info = (int)object;
  *(object_info + 1) = 1;

  // add that memory object in the hash table
  hash_set(&shm_map, (void *)key, object_info);

  return object;
}

/*
 * Returns the address of memory object referenced by argument key.
 * If that key doesn't exist in shm_map, returns NULL.
 */
void *shm_acquire(const char *key)
{
  int *object_info = hash_get(&shm_map, (void *)key, (void *)NULL);
  if (object_info == NULL) return NULL;

  void *object = (void *)*object_info;

  // register the new process as sharing this mem object
  (*(object_info + 1))++;

  return object;
}

/*
 * Release the shared memory object for a process
 * If process_count gets down to zero, delete the memory object referenced
 * by key in shm_map hash table.
 * If that key doesn't exist in shm_map, does nothing.
 */
void shm_release(const char *key)
{
  int *object_info = hash_get(&shm_map, (void *)key, (void *)NULL);
  if (object_info == NULL) return;

  void *object = (void *)*object_info;

  // unregister the process
  (*(object_info + 1))--;

  if (*(object_info + 1) == 0) {
    mem_free(object, OBJECT_SIZE);
    assert(hash_del(&shm_map, (void *)key) == 0);
  }
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
