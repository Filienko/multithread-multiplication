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
int count = 0;

pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Bounded buffer put() get()
int put(Matrix * value)
{
  bigmatrix[fill_ptr] = value;
  fill_ptr = (fill_ptr + 1) % BOUNDED_BUFFER_SIZE;
  count = count + 1;
  return 0;
}

Matrix* get()
{
  Matrix * tmp = bigmatrix[use_ptr];
  use_ptr = (use_ptr + 1) % BOUNDED_BUFFER_SIZE;
  count = count - 1;
  return tmp;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *matrix_produced)
{  
  printf("Started the prod\n");
  counter_t* matrix_produced = &matrix_produced;
  ProdConsStats produced;

  while(1)
  {
    Matrix* matrix = GenMatrixRandom();
    printf("Produced the matrix in the prod\n");
    pthread_mutex_lock(&mutex);
    while (count == BOUNDED_BUFFER_SIZE)
      pthread_cond_wait(&empty, &mutex);
    if(get_cnt(matrix_produced) < LOOPS)
    {
      put(matrix);
      increment_cnt(matrix_produced);
      pthread_cond_signal(&fill);
    }
    else
    {
      pthread_mutex_unlock(&mutex);
      return matrix_produced;
    }
    pthread_mutex_unlock(&mutex);
  }
  return matrix_produced;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *matrix_cons)
{
  ProdConsStats consumed;

  printf("Started the cons\n");
  counter_t* matrix_consumed = &matrix_cons;

  Matrix* M1 = (Matrix *) malloc(sizeof(Matrix));
  Matrix* M2 = (Matrix *) malloc(sizeof(Matrix));
  Matrix* M3 = (Matrix *) malloc(sizeof(Matrix));
  
  pthread_mutex_lock(&mutex);
  while(get_cnt(matrix_consumed) < LOOPS)
  {
    while (count == 0)
      pthread_cond_wait(&fill, &mutex);
    if(count != 0)
    {
      M1 = get();
      increment_cnt(matrix_consumed);
      pthread_cond_signal(&empty);
      pthread_mutex_unlock(&mutex);
    }

    if(get_cnt(matrix_consumed) >= LOOPS)
    {
      pthread_mutex_unlock(&mutex);
      return matrix_consumed;
    }

    do
    {
      pthread_mutex_lock(&mutex);
      while (count == 0)
        pthread_cond_wait(&fill, &mutex);
      
      if(get_cnt(matrix_consumed) >= LOOPS)
      {
        pthread_mutex_unlock(&mutex);
        return matrix_consumed;
      }

      if(count != 0)
      {
        M2 = get();
        increment_cnt(matrix_consumed);
        pthread_mutex_unlock(&mutex);
        M3 = MatrixMultiply(M1, M2);
        printf("Calculate Matrix");
      }
    } while(!M3);
    printf("Display Matrix");
    DisplayMatrix(M3,stdout);
    FreeMatrix(M1);
    FreeMatrix(M2);
    FreeMatrix(M3);
  }
  return matrix_consumed;
}

