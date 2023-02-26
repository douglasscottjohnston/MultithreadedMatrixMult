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
#include <assert.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"

// Define Locks, Condition variables, and so on here
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

Matrix **bounded_buffer;
int fill = 0;
int use = 0;
counter_t count;
counter_t produced_matrices;
counter_t consumed_matrices;

// Bounded buffer put() get()

// return 1 if the put was successful or 0 if the buffer is full
int put(Matrix *value)
{
  assert(get_cnt(&count) < BOUNDED_BUFFER_SIZE); // assert that the bounding buffer is not full
  bounded_buffer[fill] = value;
  fill = (fill + 1) % BOUNDED_BUFFER_SIZE;
  increment_cnt(&count);
  return 1;
}

// return a matrix if the get was successful and NULL if there are no matricies
Matrix *get()
{
  // assert(get_cnt(&count) > 0);
  Matrix *out = bounded_buffer[use];
  use = (use + 1) % BOUNDED_BUFFER_SIZE;
  decrement_cnt(&count);
  return out;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  ProdConsStats *prod_stats = (ProdConsStats *)arg;
  Matrix *m;

  (*prod_stats).sumtotal = 0;
  (*prod_stats).multtotal = 0;
  (*prod_stats).matrixtotal = 0;

  if(MATRIX_MODE == 0) {
    m = GenMatrixRandom();
  } else {
    m = GenMatrixBySize(MATRIX_MODE, MATRIX_MODE);
  }

  while (get_cnt(&produced_matrices) < NUMBER_OF_MATRICES)
  {
    // lock and put a matrix into the buffer
    pthread_mutex_lock(&mutex);
    while (get_cnt(&produced_matrices) < NUMBER_OF_MATRICES && get_cnt(&count) == BOUNDED_BUFFER_SIZE)
      pthread_cond_wait(&empty, &mutex); // the matrix is full so wait untill some are consumed
    if(get_cnt(&produced_matrices) < NUMBER_OF_MATRICES) {
      put(m);
      increment_cnt(&produced_matrices);
      (*prod_stats).sumtotal += SumMatrix(m);
      (*prod_stats).matrixtotal++;
    }
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex);
    
    if(MATRIX_MODE == 0) {
      m = GenMatrixRandom();
    } else {
      m = GenMatrixBySize(MATRIX_MODE, MATRIX_MODE);
    }
  }
  pthread_exit(&prod_stats);
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  Matrix *m1;
  Matrix *m2;
  Matrix *multiplied;
  ProdConsStats *con_stats = (ProdConsStats *)arg;

  (*con_stats).sumtotal = 0;
  (*con_stats).multtotal = 0;
  (*con_stats).matrixtotal = 0;

  while (get_cnt(&consumed_matrices) < NUMBER_OF_MATRICES)
  {
    pthread_mutex_lock(&mutex);
    if(get_cnt(&consumed_matrices) < NUMBER_OF_MATRICES) {
      while (get_cnt(&count) == 0)
        pthread_cond_wait(&full, &mutex);
      m1 = get();
      increment_cnt(&consumed_matrices);
      (*con_stats).sumtotal += SumMatrix(m1);
      (*con_stats).matrixtotal++;
    } else {
      break;
    }
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    if(get_cnt(&consumed_matrices) >= NUMBER_OF_MATRICES) break; // break if m1 is the last matrix
    pthread_mutex_lock(&mutex);
    if(get_cnt(&consumed_matrices) < NUMBER_OF_MATRICES) {
      while (get_cnt(&count) == 0)
        pthread_cond_wait(&full, &mutex);
      m2 = get();
      increment_cnt(&consumed_matrices);
      (*con_stats).sumtotal += SumMatrix(m2);
      (*con_stats).matrixtotal++;
    }
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    
    multiplied = MatrixMultiply(m1, m2);
    while (multiplied == NULL && get_cnt(&consumed_matrices) < NUMBER_OF_MATRICES)
    {
      // if(get_cnt(&consumed_matrices) >= NUMBER_OF_MATRICES) break;
      pthread_mutex_lock(&mutex);
      if(m2 != NULL) FreeMatrix(m2);
      if(get_cnt(&consumed_matrices) < NUMBER_OF_MATRICES) {
        while (get_cnt(&consumed_matrices) < NUMBER_OF_MATRICES && get_cnt(&count) == 0)
          pthread_cond_wait(&full, &mutex);
        m2 = get();
        increment_cnt(&consumed_matrices);
        (*con_stats).sumtotal += SumMatrix(m2);
        (*con_stats).matrixtotal++;
      } else {
        break;
      }
      pthread_cond_signal(&empty);
      pthread_mutex_unlock(&mutex);
      
      multiplied = MatrixMultiply(m1, m2);
    }

    if (multiplied != NULL)
    {
      DisplayMatrix(m1, stdout);
      printf("    X\n");
      DisplayMatrix(m2, stdout);
      printf("    =\n");
      DisplayMatrix(multiplied, stdout);
      FreeMatrix(multiplied);
      (*con_stats).multtotal++;
    }

    pthread_mutex_lock(&mutex);
    if(m1 != NULL) FreeMatrix(m1);
    if(m2 != NULL) FreeMatrix(m2);
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(&con_stats);
}

void initialize_buffer()
{
  bounded_buffer = (Matrix **)malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);
  init_cnt(&count);
  init_cnt(&produced_matrices);
  init_cnt(&consumed_matrices);
}