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

// Note that the number of worker threads is handled using a local variable called numw in pcmatrix.c. 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"


// Define Locks, Condition variables, and so on here
int fill_ptr = 0;
int use_ptr = 0;
counter_t* matrix_produced;
counter_t* matrix_consumed;
counter_t* count;

pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Bounded buffer put() get()
int put(Matrix * value)
{
  bigmatrix[fill_ptr] = value;
  fill_ptr = (fill_ptr + 1) % MAX;
  count->value=+1;
}

Matrix * get()
{
  Matrix * tmp = bigmatrix[use_ptr];
  use_ptr = (use_ptr + 1) % MAX;
  count->value=-1;
  return tmp;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{  
  Matrix* matrix = GenMatrixRandom();
  
  while(1)
  {
    Pthread_mutex_lock(&mutex);
    while (get_cnt(count) == MAX)
      Pthread_cond_wait(&empty, &mutex);
    if(get_cnt(matrix_produced) < LOOPS)
    {
      put(matrix);
      increment_cnt(matrix_produced);
      Pthread_cond_signal(&fill);
    }
    else
    {
      Pthread_mutex_unlock(&mutex);
      return 0;
    }
    Pthread_mutex_unlock(&mutex);
  }

  return 0;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  Matrix* M1;
  Matrix* M2;
  Matrix* M3;
  
  pthread_mutex_lock(&mutex);
  while(get(matrix_consumed) < LOOPS)
  {
    while (get_cnt(count) == 0)
      pthread_cond_wait(&fill, &mutex);
    if(get_cnt(count) != 0)
    {
      M1 = get();
      increment_cnt(matrix_consumed);
      pthread_cond_signal(&empty);
      pthread_mutex_unlock(&mutex);
    }

    if(get(matrix_consumed) >= LOOPS)
    {
      Pthread_mutex_unlock(&mutex);
      return 0;
    }

    do
    {
      pthread_mutex_lock(&mutex);
      while (get_cnt(count) == 0)
        thread_cond_wait(&fill, &mutex);
      
      if(get(matrix_consumed) >= LOOPS)
      {
        Pthread_mutex_unlock(&mutex);
        return 0;
      }

      if(get_cnt(count) != 0)
      {
        M2 = get();
        increment_cnt(matrix_consumed);
        pthread_mutex_unlock(&mutex);
        M3 = MatrixMultiply(M1, M2);
        FreeMatrix(M2);
      }
    } while(!M3);

    DisplayMatrix(M3,stdout);
    FreeMatrix(M1);
    FreeMatrix(M3);
  }
  return 0;
}

