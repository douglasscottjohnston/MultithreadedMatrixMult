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
pthread_mutex_t producer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int bufferHasSpace = 1;
int bufferHasMatrices = 1;

buffer bounding_buffer;

// Bounded buffer put() get()

// return 1 if the put was successful or 0 if the buffer is full
int put(Matrix *value)
{

  int put_index_has_matrix = bounding_buffer.bigmatrix[bounding_buffer.put_index] != NULL;
  if (put_index_has_matrix || bounding_buffer.put_index == -1) // if the put index is already -1 we need to double check if the buffer is full
  {
    bounding_buffer.put_index = get_next_put_index();
  }

  int buffer_is_full = bounding_buffer.put_index == -1;
  if (buffer_is_full)
  {
    bufferHasSpace = 0;
    return 0;
  }

  bounding_buffer.bigmatrix[bounding_buffer.put_index] = value;
  bounding_buffer.put_index = get_next_put_index();
  bufferHasMatrices = 1; // signal
  return 1;
}

// return a matrix if the get was successful and NULL if there are no matricies
Matrix *get()
{

  int get_index_is_null = bounding_buffer.bigmatrix[bounding_buffer.get_index] == NULL;
  if (get_index_is_null || bounding_buffer.get_index == -1) // if the get index is already NULL we need to double check if the buffer is empty
  {
    bounding_buffer.get_index = get_next_get_index();
  }

  int buffer_is_empty = bounding_buffer.get_index == -1;
  if (buffer_is_empty)
  {
    bufferHasMatrices = 0;
    return NULL;
  }

  Matrix *out = bounding_buffer.bigmatrix[bounding_buffer.get_index];
  bounding_buffer.get_index = get_next_get_index();
  return out;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  int *NUMBER_OF_PRODUCED_MATRICES = (int *)arg;
  ProdConsStats prod_stats;
  while (*NUMBER_OF_PRODUCED_MATRICES < NUMBER_OF_MATRICES)
  {
    Matrix *m = GenMatrixRandom();

    prod_stats.sumtotal += SumMatrix(m);

    // lock and put a matrix into the buffer
    pthread_mutex_lock(&producer_lock);
    pthread_cond_signal(&cond);

    while (put(m))
      pthread_cond_wait(&cond, &producer_lock); // the matrix is full so wait untill some are consumed

    (*NUMBER_OF_PRODUCED_MATRICES)++;

    pthread_mutex_unlock(&producer_lock);

    prod_stats.matrixtotal++;
  }
  pthread_exit(&prod_stats);
  // return NULL;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  int *NUMBER_OF_CONSUMED_MATRICES = (int *)arg;
  Matrix *m1;
  Matrix *m2;
  Matrix *multiplied;
  ProdConsStats con_stats;
  while (*NUMBER_OF_CONSUMED_MATRICES < NUMBER_OF_MATRICES)
  {
    pthread_mutex_lock(&consumer_lock);
    pthread_cond_signal(&cond);
    m1 = get();
    while (m1 == NULL) {
      pthread_cond_wait(&cond, &consumer_lock);
      m1 = get();
    }
    pthread_mutex_unlock(&consumer_lock);
    con_stats.sumtotal += SumMatrix(m1);
    con_stats.matrixtotal++;
    pthread_mutex_lock(&consumer_lock);
    pthread_cond_signal(&cond);
    m2 = get();
    while (m2 == NULL) {
      pthread_cond_wait(&cond, &consumer_lock);
      m2 = get();
    }
    pthread_mutex_unlock(&consumer_lock);
    con_stats.sumtotal += SumMatrix(m2);
    con_stats.matrixtotal++;
    multiplied = MatrixMultiply(m1, m2);
    while (multiplied == NULL)
    {
      FreeMatrix(m2);
      con_stats.matrixtotal++;
      bufferHasSpace = 1; // signal
      (*NUMBER_OF_CONSUMED_MATRICES)++;
      pthread_cond_signal(&cond);
      m2 = get();
      while (m2 == NULL) {
        pthread_cond_wait(&cond, &consumer_lock);
        m2 = get();
      }
      pthread_mutex_unlock(&consumer_lock);
      con_stats.sumtotal += SumMatrix(m2);
      multiplied = MatrixMultiply(m1, m2);
    }

    FreeMatrix(m1);
    FreeMatrix(m2);
    FreeMatrix(multiplied);
    bufferHasSpace = 1; // signal

    DisplayMatrix(multiplied, stdout);

    con_stats.matrixtotal += 2;
    con_stats.multtotal += 1;
  }
  pthread_exit(&con_stats);
  // return NULL;
}

void initialize_buffer()
{
  bounding_buffer.bigmatrix = (Matrix **)malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);
  bounding_buffer.put_index = 0;
  bounding_buffer.get_index = 0;
  bounding_buffer.length = BOUNDED_BUFFER_SIZE;
}

void free_buffer()
{
  for (int i = 0; i < bounding_buffer.length; i++)
  {
    FreeMatrix(bounding_buffer.bigmatrix[i]);
  }
}

int get_next_put_index()
{
  for (int i = 0; i < bounding_buffer.length; i++)
  {
    if (bounding_buffer.bigmatrix[i] == NULL)
    {
      return i;
    }
  }
  return -1;
}

int get_next_get_index()
{
  for (int i = 0; i < bounding_buffer.length; i++)
  {
    if (bounding_buffer.bigmatrix[i] != NULL)
    {
      return i;
    }
  }
  return -1;
}