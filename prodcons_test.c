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
int put(Matrix *value) {
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
  //referencing passed global counter of produced matrix 
  counter_t* matrix_produced = matrix_prod;

  //Initialize necessary minimum values for the statistics of that threat, located on the heap
  ProdConsStats* produced_stats = malloc(sizeof(ProdConsStats));
  produced_stats->sumtotal = 0;
  produced_stats->matrixtotal = 0;
  produced_stats->multtotal = 0;
  int done = 0;

  //If sufficient amount of matrices was produced, do not put additional matrix or acquire the lock, terminate
  if(get_cnt(matrix_produced) >= NUMBER_OF_MATRICES)
  {
    done = 1; 
  }

  while(!done)
  {
    Matrix* matrix = GenMatrixRandom();
    //Critical section
    pthread_mutex_lock(&mutex);

    //If sufficient amount of matrices was produced, do not put additional matrix
    if(get_cnt(matrix_produced) >= NUMBER_OF_MATRICES)
    {
      done = 1;
    }

    if (!done) {
      //If sufficient amount of matrices was not produced, and the buffer is full,
      //wait for a signal
      while (count == BOUNDED_BUFFER_SIZE && get_cnt(matrix_produced) < NUMBER_OF_MATRICES) {
        pthread_cond_wait(&empty, &mutex);
      }
      
      //If sufficient amount of matrices was not produced, and the buffer is not full
      // allows to put 1 matrix and update the counters and statistics
      if(count < BOUNDED_BUFFER_SIZE && get_cnt(matrix_produced) < NUMBER_OF_MATRICES)
      {
        put(matrix);
        produced_stats->sumtotal += SumMatrix(matrix);
        increment_cnt(matrix_produced);
        pthread_cond_signal(&fill);
        produced_stats->matrixtotal++;
      }
    }

    //If no more matrices id needed, send a signal and release the lock.
    if (done) {
      pthread_cond_signal(&empty);
      printf("\nINNER Producer, %i\n",produced_stats->matrixtotal);
      printf("\nINNER Producer, %i\n",produced_stats->sumtotal);
    }

    //Release the lock before returning a thread or acquiring the lock again before putting another matrix
    pthread_mutex_unlock(&mutex);
  }

  return produced_stats;
}


// Matrix CONSUMER worker thread
void *cons_worker(void *matrix_cons)
{
  //referencing passed global counter of produced matrix 
  counter_t* matrix_consumed = matrix_cons;

  //Initialize necessary minimum values for the statistics of that threat, located on the heap
  ProdConsStats* consumed_stats = malloc(sizeof(ProdConsStats));
  consumed_stats->sumtotal = 0;
  consumed_stats->matrixtotal = 0;
  consumed_stats->multtotal = 0;
  int done = 0;

  //State the pointer for Matrices
  Matrix* M1 = NULL;
  Matrix* M2 = NULL;
  Matrix* M3 = NULL;
  
  //If sufficient amount of matrices was consumed, do not acquire additional matrix
  if(get_cnt(matrix_consumed) >= NUMBER_OF_MATRICES)
  {
    done = 1;
  }

  while(!done)
  {
    //Acquire the lock for the first matrix
    pthread_mutex_lock(&mutex);

    //If sufficient amount of matrices was not consumed, and the buffer is empty,
    //wait for a signal
    while (count == 0 && get_cnt(matrix_consumed) < NUMBER_OF_MATRICES) {
      pthread_cond_wait(&fill, &mutex);
    }

    //If sufficient amount of matrices was not consumed, and the buffer is not empty
    // allows to get matrix and update the counters and statistics
    if(count > 0 && (get_cnt(matrix_consumed) < NUMBER_OF_MATRICES))
    {
      M1 = get();
      increment_cnt(matrix_consumed);
      consumed_stats->sumtotal += SumMatrix(M1);
      consumed_stats->matrixtotal++;
      pthread_cond_signal(&empty);
    }
    //Release the lock before attemting to acquire second matrix
    pthread_mutex_unlock(&mutex);

    if (M1) {
      //If M1 is available, repeat until either no more matrices are to be consumed or M3 is acquried
      while (!(done || M3)) {
        //Acquire the lock for the second matrix attempt
        pthread_mutex_lock(&mutex);      
        
        //If sufficient amount of matrices was not consumed, and the buffer is empty,
        //wait for a signal
        while (count == 0 && get_cnt(matrix_consumed) < NUMBER_OF_MATRICES) 
        {
          pthread_cond_wait(&fill, &mutex);
        }
        
        //If sufficient amount of matrices was not consumed, and the buffer is not empty
        // allows to get second matrix for multiplication, then update the counters and statistics
        if(count > 0 && get_cnt(matrix_consumed) < NUMBER_OF_MATRICES)
        {
          M2 = get();
          
          consumed_stats->sumtotal += SumMatrix(M2);
          increment_cnt(matrix_consumed);
          pthread_cond_signal(&empty);
          consumed_stats->matrixtotal++;
          M3 = MatrixMultiply(M1, M2);
          if (!M3) 
          {
            FreeMatrix(M2);
            M2 = NULL;
          }
        }
        //If sufficient amount of matrices was consumed, do not get additional matrix
        if(get_cnt(matrix_consumed) >= NUMBER_OF_MATRICES)
        {
          done = 1;
        }
      } 
    }
    
    //Free matrices that were acquired
    if (M1) {
      FreeMatrix(M1);
      M1 = NULL;
    }
    if (M2) 
    {
      FreeMatrix(M2);
      M2 = NULL;
    }
    if (M3) {
      DisplayMatrix(M3,stdout);
      consumed_stats->multtotal++;
      FreeMatrix(M3);
      M3 = NULL;
    }

    //If no more matrices are to be produced, done, release the lock later.
    if(get_cnt(matrix_consumed) >= NUMBER_OF_MATRICES)
    {
      done = 1;  
    }
    else
    {
      //Unlock the mutex to acuqire it at the start of the while body
      pthread_mutex_unlock(&mutex);  
    }

  }
  pthread_cond_signal(&fill);
  pthread_mutex_unlock(&mutex);

  printf("\nOUTER Consumer, %i\n",consumed_stats->matrixtotal);
  printf("\nOUTER Consumer, %i\n",consumed_stats->sumtotal);
  return consumed_stats;
}
