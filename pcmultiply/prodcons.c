/*
 *  prodcons module
 *  Producer Consumer module
 *
 *  Implements routines for the producer consumer module based on
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"

// Define Locks, Condition variables, and so on here

buffer bounding_buffer;

// Bounded buffer put() get()
int put(Matrix * value)
{
  bounding_buffer.tail++;
  if(bounding_buffer.tail > bounding_buffer.length) {
    printf("Too many matrices were produced| prodcons.c line: 30");
    return NULL;
  }
  bounding_buffer.bigmatrix[bounding_buffer.tail] = value;
}

Matrix * get()
{
  if(bounding_buffer.head > bounding_buffer.length) {
    printf("Trying to retrieve too many matrices| prodcons.c line: 41");
    return NULL;
  }
  // if the head is null wait for a new matrix to be added
  Matrix * out = bounding_buffer.bigmatrix[bounding_buffer.head];
  bounding_buffer.head++;
  return out;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  ProdConsStats prod_stats;
  while(bounding_buffer.tail < bounding_buffer.length) {
    put(GenMatrixRandom());
    prod_stats.matrixtotal++;
  }
  pthread_exit(&prod_stats);
  // return NULL;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  Matrix *m1;
  Matrix *m2;
  Matrix *multiplied; 
  ProdConsStats con_stats;
  while(bounding_buffer.head < bounding_buffer.length) {
    m1 = get();
    con_stats.matrixtotal++;
    m2 = get();
    con_stats.matrixtotal++;
    multiplied = MatrixMultiply(m1, m2);
    con_stats.multtotal++;
  }
  pthread_exit(&con_stats);
  // return NULL;
}

void initialize_buffer()
{
  bounding_buffer.bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);
  bounding_buffer.head = 0;
  bounding_buffer.tail = 0;
  bounding_buffer.length = BOUNDED_BUFFER_SIZE;
}
