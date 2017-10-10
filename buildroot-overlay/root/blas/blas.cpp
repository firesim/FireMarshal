#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

/*****
Total size used should be dim*dim*sizeof(double)
compute the matrix multiply dgemm
C = alpha* op(A)*op(B)+beta*C

op(B) = B if TRANSB = n ,  op(B) = B' if TRANSB = y

you can change what routine is run by using -DSLOW  for tripple for loop -DACML for acml
Does work with acml-mp  but must set OMP_NUM_THREADS and compile with -mp

Run both tests
pgCC dgemm_speed.cpp $ACML_INC $ACML_LINK -lacml -lpgftnrtl -DSLOW -DBLAS3 -DACML

use MKL
icc dgemm_speed.cpp -DBLAS3 -mkl -Ddim=10000 -DMKL

*****/
#ifdef ACML
  #include <acml.h>
#endif

#ifdef CBLAS
  #include <cblas.h>
#endif

#ifdef MKL
  #include <mkl_cblas.h>
#endif

//OpenMP
#ifdef _OPENMP
  #include <omp.h>
#endif

struct timeval tv1, tv2;
struct timezone tz;

using namespace std;

int main(int argc, char *argv[]){

  if(argc != 2) {
    cout << "Usage: ./blas dimENSION\n";
    return EXIT_FAILURE;
  }

  uint64_t dim = atol(argv[1]);
  cout << "Using dim: " << dim << "\n";

  //openMP support
#ifdef _OPENMP
  cout << "Will use: "<< omp_get_max_threads() << " threads"<<endl;
  cout << "Will use: "<< omp_get_num_procs() << " threads"<<endl;
#endif

  cout <<"Size of double: " <<sizeof(double)<<endl;
  cout << "Will use: " << dim*dim*sizeof(double)*3/1024/1024 << " MB"<<endl;

  unsigned long int M = dim;                      /* number of rows of A and C */
  unsigned long int N = dim;                      /* number of col. of B and C */
  unsigned long int K = dim;                      /* number of columns of A and rows of B */
  unsigned long int lda = dim;                    /* first dim of A when TRNSA=n LDA >= max(1, M) 
                                  otherwise LDA >= max (1,K) */
  unsigned long int ldb = dim;                    /* first dim of B when TRNSB=n LDB >= max(1, K) 
                                  otherwise LDB >= max (1,N) */
  unsigned long int ldc = dim;                    /* first dimension of C LDC >= max(1,M) */
  double alpha = 1;               /* Scalar to scale matrix A by */
  double beta = alpha;            /* Scalar to scale matrix C by */

                          /* buffer for result or added to result C[LDC, N] */
  double *c = (double *) malloc(dim*dim*sizeof(double) );
                          /* input matrix A[LDA, K] if TRANSA=n otherwise A[LDA, M]*/
  double *a = (double *) malloc(dim*dim*sizeof(double) );
                          /* input matrix B[LDB, N] if TRANSB=n otherwise B[LDB, K]*/
  double *b = (double *) malloc(dim*dim*sizeof(double) );

    /* These macros allow access to 1-d arrays as though
       they are 2-d arrays stored in column-major order,
       as required by ACML C routines. */
#define A(I,J) a[((J)-1)*lda+(I)-1]
#define B(I,J) b[((J)-1)*ldb+(I)-1]
#define C(I,J) c[((J)-1)*ldc+(I)-1]

  /* populate the matrix random */
  gettimeofday( &tv1, &tz);
  int j=0;
  int i=0;

  for (i=1; i<=dim; i++){
          for (j=1; j<=dim; j++){
                  A(i,j)=rand();
                  B(i,j)=rand();
          }
  }

  gettimeofday( &tv2, &tz);

  double matrix_fill_time = (tv2.tv_sec-tv1.tv_sec);
  cout <<" Matrix full in: "<< matrix_fill_time << " sec" <<endl;

#ifdef ACML
  /* 
  We will try out acml here,  we will use the fucntion dgemm() */
  gettimeofday( &tv1, &tz);
  /* call the function */
  dgemm('N','N', M, N, K, alpha, a, lda, b, ldb, beta, c, ldc);
  gettimeofday( &tv2, &tz);

  double acml_runtime = (tv2.tv_sec-tv1.tv_sec);
  cout << "ACML Runtime: "<< acml_runtime << " sec." <<endl;
#endif

  //cblas interface 
#ifdef CBLAS
  gettimeofday( &tv1, &tz);
  cblas_dgemm(CblasColMajor,CblasNoTrans, CblasNoTrans, M, N, K, alpha, a, lda, b, ldb, beta, c, ldc);
  gettimeofday( &tv2, &tz);
  double cblas_runtime = (tv2.tv_sec-tv1.tv_sec);
  cout << "CBLAS Runtime: "<< cblas_runtime << " sec." <<endl;

#endif

#ifdef MKL
  /* 
  We will try out acml here,  we will use the fucntion dgemm() */
  gettimeofday( &tv1, &tz);
  /* call the function */
  cblas_dgemm(CblasColMajor,CblasNoTrans, CblasNoTrans, M, N, K, alpha, a, lda, b, ldb, beta, c, ldc);
  //cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans, M, N, K, alpha, a, lda, b, ldb, beta, c, ldc);
  gettimeofday( &tv2, &tz);

  double mkl_runtime = (tv2.tv_sec-tv1.tv_sec);
  cout << "MKL Runtime: "<< mkl_runtime << " sec." <<endl;
#endif

#ifdef SLOW
  /* This will compare how long a user retean takes */
  gettimeofday( &tv1, &tz);
  for (unsigned long int i=1; i<=dim; i++){
          for (unsigned long int j=1; j<=dim; j++){
                  for (unsigned long int k=1; k<=dim; k++){
                          C(i,j)=C(i,j)+A(i,k)*B(k,j);
                  }
          }
  }
  gettimeofday( &tv2, &tz);

  double slow_runtime = (tv2.tv_sec-tv1.tv_sec);
  cout << "SLOW Runtime: "<< slow_runtime << " sec." <<endl;

#endif


  /* this prints out the matrixs only for debuggin this code */

#ifdef DEBUG
  /* print a and b */
  cout << "Matrix a" << endl;
  for ( long int i = 1; i<=M; i++){
          for (long int j = 1; j<=M; j++){
                  cout << A(i,j) << " ";
          }
          cout << endl;
  }
  cout << "Matrix b" << endl;
  for ( long int i = 1; i<=M; i++){
          for (long int j = 1; j<=M; j++){
                  cout << B(i,j) << " ";
          }
          cout << endl;
  }
  cout << "Matrix c" << endl;
  for ( long int i = 1; i<=M; i++){
          for (long int j = 1; j<=M; j++){
                  cout << C(i,j) << " ";
          }
          cout << endl;
  }
#endif

  return 0;
}
/* http://docs.sun.com/source/819-3691/dgemm.html */
