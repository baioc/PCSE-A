/*
 * shared_memory.h
 *
 *  Created on: 24/03/2021
 *      Author: Thibault Cantori
 */

#ifndef _shared_memory_H_
#define _shared_memory_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void shm_init();

/*
 * Create a shared memory object and stores it into a hash table with argument
 * key as the index.
 * If an object by that key already exists, return NULL.
 * Created objects are 4096 bytes long.
 */
void *shm_create(const char*);

/*
 * Returns the address of memory object referenced by argument key.
 * If that key doesn't exist in shm_map, returns NULL.
 */
void *shm_acquire(const char*);

/*
 * Delete a memory object referenced by key in shm_map hash table.
 * If that key doesn't exist in shm_map, does nothing.
 */
void shm_release(const char*);

#endif /* _shared_memory_H_ */
