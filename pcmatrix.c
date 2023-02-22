/*
 *  pcmatrix module
 *  Primary module providing control flow for the pcMatrix program
 *
 *  Producer consumer bounded buffer program to produce random matrices in parallel
 *  and consume them while searching for valid pairs for matrix multiplication.
 *  Matrix multiplication requires the first matrix column count equal the
 *  second matrix row count.
 *
 *  A matrix is consumed from the bounded buffer.  Then matrices are consumed
 *  from the bounded buffer, ONE AT A TIME, until an eligible matrix for multiplication
 *  is found.
 *
 *  Totals are tracked using the ProdConsStats Struct for each thread separately:
 *  - the total number of matrices multiplied (multtotal from each consumer thread)
 *  - the total number of matrices produced (matrixtotal from each producer thread)
 *  - the total number of matrices consumed (matrixtotal from each consumer thread)
 *  - the sum of all elements of all matrices produced and consumed (sumtotal from each producer and consumer thread)
 *  
 *  Then, these values from each thread are aggregated in main thread for output
 *
 *  Correct programs will produce and consume the same number of matrices, and
 *  report the same sum for all matrix elements produced and consumed.
 *
 *  Each thread produces a total sum of the value of
 *  randomly generated elements.  Producer sum and consumer sum must match.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"

int main (int argc, char * argv[])
{
  // Process command line arguments
  int numw = NUMWORK;
  if (argc==1)
  {
    BOUNDED_BUFFER_SIZE=MAX;
    NUMBER_OF_MATRICES=LOOPS;
    MATRIX_MODE=DEFAULT_MATRIX_MODE;
    printf("USING DEFAULTS: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }
  else
  {
    if (argc==2)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=MAX;
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==3)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==4)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==5)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=atoi(argv[4]);
    }
    printf("USING: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }

  time_t t;

  printf("Hello");

  // Seed the random number generator with the system time
  srand((unsigned) time(&t));
  printf("Hello");

  bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);
  printf("Hello");
  counter_t* matrix_prod = malloc(sizeof(counter_t));
  printf("Hello");
  init_cnt(matrix_prod);
  counter_t* matrix_cons = malloc(sizeof(counter_t));
  init_cnt(matrix_cons);

  int i=0, prod_success=0, cons_sucess = 0;

  pthread_t thread_con;
  pthread_t thread_prod;
  printf("Hello");
  prod_success = pthread_create(&thread_prod, NULL, prod_worker, (void *) matrix_prod);  // CREATE MATRIX PRODUCER THREAD
  printf("World");

  cons_sucess = pthread_create(&thread_con, NULL, cons_worker, (void *) matrix_cons);  // CREATE MATRIX PRODUCER THREAD
  printf("World1");
  pthread_join(thread_prod, NULL);
  pthread_join(thread_con, NULL);

  // thread = (pthread_t *) malloc ((NUMWORK*2)*sizeof(pthread_t));

  // for (i=0;i<NUMWORK*2;i++){
  //   prod_success = pthread_create(&thread[i], NULL, *prod_worker, NULL);  // CREATE MATRIX PRODUCER THREAD
  //   i++;
  //   cons_sucess = pthread_create(&thread[i], NULL, *cons_worker, NULL);  // CREATE MATRIX CONSUMER THREAD
  //   if(prod_success != 0 || cons_sucess != 0)
  //   {
  //     printf("Failed thread, index:  %i",i);
  //     exit(0);        
  //   }
  // }

  // for(i=0;i<NUMWORK*2;i++){
  //   prod_success = pthread_join(thread[i],NULL);
  //   i++;
  //   cons_sucess = pthread_join(thread[i],NULL);
  //   if(prod_success != 0 || cons_sucess != 0)
  //   {
  //     printf("Failed thread, index: %i",i);
  //     exit(0);        
  //   }
  // }

  printf("Producing %d matrices in mode %d.\n",NUMBER_OF_MATRICES,MATRIX_MODE);
  printf("Using a shared buffer of size=%d\n", BOUNDED_BUFFER_SIZE);
  printf("With %d producer and consumer thread(s).\n",numw);
  printf("\n");

  // Add your code here to create threads and so on


  // These are used to aggregate total numbers for main thread output
  int prs = 0; // total #matrices produced
  int cos = 0; // total #matrices consumed
  int prodtot = 0; // total sum of elements for matrices produced
  int constot = 0; // total sum of elements for matrices consumed
  int consmul = 0; // total # multiplications

  // consume ProdConsStats from producer and consumer threads [HINT: return from join]
  // add up total matrix stats in prs, cos, prodtot, constot, consmul

  printf("Sum of Matrix elements --> Produced=%d = Consumed=%d\n",prs,cos);
  printf("Matrices produced=%d consumed=%d multiplied=%d\n",prodtot,constot,consmul);

  return 0;
}
