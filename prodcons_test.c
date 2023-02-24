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
void *prod_worker(void *matrix_prod)
{  

  counter_t* matrix_produced = matrix_prod;
  ProdConsStats* produced_stats = malloc(sizeof(ProdConsStats));
  produced_stats->sumtotal = 0;
  produced_stats->matrixtotal = 0;
  produced_stats->multtotal = 0;

  while(1)
  {
    Matrix* matrix = GenMatrixRandom();
    pthread_mutex_lock(&mutex);

    if(get_cnt(matrix_produced) >= NUMBER_OF_MATRICES)
    {
      pthread_mutex_unlock(&mutex);
      printf("\nINNER Producer, %i\n",produced_stats->matrixtotal);
      printf("\nINNER Producer, %i\n",produced_stats->sumtotal);
      return produced_stats;
    }
    while (count == BOUNDED_BUFFER_SIZE && get_cnt(matrix_produced) < NUMBER_OF_MATRICES)
      pthread_cond_wait(&empty, &mutex);
    
    if(get_cnt(matrix_produced) < NUMBER_OF_MATRICES)
    {
      put(matrix);
      pthread_cond_signal(&fill);
      produced_stats->sumtotal += SumMatrix(matrix);
      increment_cnt(matrix_produced);
      produced_stats->matrixtotal++;
      pthread_mutex_unlock(&mutex);
    }
  }
  printf("\nOuter Producer, %i\n",produced_stats->matrixtotal);
  printf("\nOuter Producer, %i\n",produced_stats->sumtotal);

  return produced_stats;
}
// Matrix CONSUMER worker thread
void *cons_worker(void *matrix_cons)
{
  counter_t* matrix_consumed = matrix_cons;
  ProdConsStats* consumed_stats = malloc(sizeof(ProdConsStats));
  consumed_stats->sumtotal = 0;
  consumed_stats->matrixtotal = 0;
  consumed_stats->multtotal = 0;

  Matrix* M1 = (Matrix *) malloc(sizeof(Matrix));
  Matrix* M2 = (Matrix *) malloc(sizeof(Matrix));
  Matrix* M3 = (Matrix *) malloc(sizeof(Matrix));
  
  //pthread_mutex_lock(&mutex);
  while(get_cnt(matrix_consumed) < NUMBER_OF_MATRICES)
  {
    pthread_mutex_lock(&mutex);
    if(get_cnt(matrix_consumed) >= NUMBER_OF_MATRICES)
    {
      pthread_mutex_unlock(&mutex);
      printf("\nINNER Consumer, %i\n",consumed_stats->matrixtotal);
      printf("\nINNER Consumer, %i\n",consumed_stats->sumtotal);

      return consumed_stats;
    }
    while (count == 0 && get_cnt(matrix_consumed) < NUMBER_OF_MATRICES)
      pthread_cond_wait(&fill, &mutex);
      
    if(count != 0 & get_cnt(matrix_consumed) < NUMBER_OF_MATRICES)
    {
      M1 = get();
      pthread_cond_signal(&empty);
      consumed_stats->sumtotal += SumMatrix(M1);
      increment_cnt(matrix_consumed);
      consumed_stats->matrixtotal++;
      pthread_mutex_unlock(&mutex);
    }

    do
    {
      pthread_mutex_lock(&mutex);      
      if(get_cnt(matrix_consumed) >= NUMBER_OF_MATRICES)
      {
        pthread_mutex_unlock(&mutex);
        printf("\nINNER1 Consumer, %i\n",consumed_stats->matrixtotal);
        printf("\nINNER1 Consumer, %i\n",consumed_stats->sumtotal);
        return consumed_stats;
      }
      
      while (count == 0 && get_cnt(matrix_consumed) < NUMBER_OF_MATRICES)
        pthread_cond_wait(&fill, &mutex);

      if(count > 0 && get_cnt(matrix_consumed) < NUMBER_OF_MATRICES)
      {
        M2 = get();
        
        consumed_stats->sumtotal += SumMatrix(M2);
        increment_cnt(matrix_consumed);
        consumed_stats->matrixtotal++;
        M3 = MatrixMultiply(M1, M2);
        if (!M3) 
        {
          FreeMatrix(M2);
        }
        pthread_mutex_unlock(&mutex);
      }
    } while(!M3);

    consumed_stats->multtotal++;

    DisplayMatrix(M3,stdout);
    FreeMatrix(M1);
    FreeMatrix(M2);
    FreeMatrix(M3);
  }
  pthread_mutex_unlock(&mutex);
  printf("\nOUTER Consumer, %i\n",consumed_stats->matrixtotal);
  printf("\nOUTER Consumer, %i\n",consumed_stats->sumtotal);
  return consumed_stats;
}
