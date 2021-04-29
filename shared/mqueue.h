#ifndef _MQUEUE_H_
#define _MQUEUE_H_

/*
 * Creates a message queue
 * count : size of the message queue we want to create
 * return value is the fid of the newly created message queue, or -1 if it was
 * impossible to create a new message queue
 */
int pcreate(int count);

/*
 * Deletes a message queue, frees its structure, all messages are forgotten
 * and processes waiting to send or receive a message from this queue are
 * liberated
 * fid : index of the message queue we want to delete
 * return value is 0 if everything went as expected, -1 if the fid was
 * incorrect or if there was an issue
 */
int pdelete(int fid);

/*
 * Sends a message into a message queue
 * fid : index of the message queue we want to send the message to
 * message : the integer we want to send in the message queue
 * return value is 0 if everything went as expected, -1 if the fid was
 * incorrect or if pdelete or preset were executed while we were trying to
 * send the message
 */
int psend(int fid, int message);

/*
 * Receives a message from a message queue
 * fid : index of the message queue we want to receive a message from
 * *message : the integer where we stock the message received
 * return value is 0 if everything went as expected, -1 if the fid was
 * incorrect or if pdelete or preset were executed while we were trying to
 * send the message
 */
int preceive(int fid, int *message);

/*
 * Resets a message queue, all messages are forgotten and processes waiting
 * to send or receive a message from this queue are liberated
 * fid : index of the message queue we want to reset
 * return value is 0 if everything went as expected, -1 if the fid was
 * incorrect or if pdelete or preset were executed while we were trying to
 * send the message
 */
int preset(int fid);

/*
 * Gives the current state of a message queue
 * fid : index of the message queue we want to know the state of
 * *count : the integer where we stock the state of the message queue fid
 * return value is 0 if everything went as expected, -1 if the fid was
 * incorrect
 */
int pcount(int fid, int *count);

/*
 * Display the following information about all existing mesage_queues:
 *    - their id
 *    - waiting on receiving messages processes (pid and name)
 *    - waiting on sending messages processes (pid and name)
 *    - buffer space
 */
void pinfo();

#endif /* _MQUEUE_H_ */
