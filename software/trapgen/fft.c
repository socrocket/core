#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/*#include "libmt.h"*/
#define SLAVESTART                 1
#define PAGE_SIZE               4096
#define NUM_CACHE_LINES        65536
#define LOG2_LINE_SIZE             4
#define DEFAULT_M                 4
#define DEFAULT_P                 1
#define PI                         3.1416

#define SHORT_BENCH

#define SWAP(a,b) {double tmp; tmp=a; a=b; b=tmp;}

struct GlobalMemory {
  int id;
  int *transtimes;
  int *totaltimes;
  int starttime;
  int finishtime;
  int initdonetime;
};
//--------------5 GPTR-----------------------------
struct GlobalMemory* Global;
double *x;              /* x is the original time-domain data     */
double *trans;          /* trans is used as scratch space         */
double *umain;          /* umain is roots of unity for 1D FFTs    */
double *umain2;         /* umain2 is entire roots of unity matrix */

//--------------9 GVAL----------------------------------
int P = DEFAULT_P;
int M = DEFAULT_M;
int N;                  /* N = 2^M                                */
int rootN;              /* rootN = N^1/2                          */
int test_result = 0;
int doprint = 1;
int dostats = 0;
int num_cache_lines = NUM_CACHE_LINES;    /* number of cache lines */
int pad_length;
//----------------------------------------------------------

int transtime = 0;
int transtime2 = 0;
int avgtranstime = 0;
int avgcomptime = 0;
unsigned int transstart = 0;
unsigned int transend = 0;
int maxtotal=0;
int mintotal=0;
double maxfrac=0;
double minfrac=0;
double avgfractime=0;
int orig_num_lines = NUM_CACHE_LINES;     /* number of cache lines */
int log2_line_size = LOG2_LINE_SIZE;
int line_size;
int rowsperproc;
double ck1;
double ck3;                        /* checksums for testing answer */

void SlaveStart();
double TouchArray(double *,double *,double *,double *,int,int,int,int);
void FFT1D(int,int,int,double *,double *,double *,double *,int,int *,int,
         int,int,int,int,int,int,struct GlobalMemory *);
double CheckSum();
double drand48();
int log_2(int);
void printerr(char *);
void Transpose(int n1,double *src,double *dest,int MyNum,int MyFirst,int MyLast,int pad_length);
void FFT1DOnce(int direction,int M,int N,double *u,double *x);
void TwiddleOneCol(int direction,int n1,int N,int j,double *u,double *x,int pad_length);
void Scale(int n1,int N,double *x);



void SlaveStart(int my_id)
{
  int i;
//  int j;
  int MyNum;
//  double error;
  double *upriv;
//  int initdone;
//  int finish;
  int l_transtime=0;
  int MyFirst;
  int MyLast;

    MyNum = my_id ;

// POSSIBLE ENHANCEMENT:  Here is where one might pin processes to
// processors to avoid migration


  upriv = (double *) malloc(2*(rootN-1)*sizeof(double));
  for (i=0;i<2*(rootN-1);i++) {
    upriv[i] = umain[i];
  }

  MyFirst = rootN*MyNum/P;
  MyLast = rootN*(MyNum+1)/P;
//  printf("my first %d - my last %d\n",MyFirst,MyLast);
  TouchArray(x, trans, umain2, upriv, N, MyNum, MyFirst, MyLast);

// POSSIBLE ENHANCEMENT:  Here is where one might reset the
//   statistics that one is measuring about the parallel execution

  // perform forward FFT
  FFT1D(1, M, N, x, trans, upriv, umain2, MyNum, &l_transtime, MyFirst,
    MyLast, pad_length, P, test_result, doprint, dostats, Global);

  // perform backward FFT
  if (test_result) {
    FFT1D(-1, M, N, x, trans, upriv, umain2, MyNum, &l_transtime, MyFirst,
      MyLast, pad_length, P, test_result, doprint, dostats, Global);
  }
//  printf("prima della fine\n");
}


double TouchArray(x, scratch, u, upriv, N, MyNum, MyFirst, MyLast)

double *x;
double *scratch;
double *u;
double *upriv;
int N;
int MyNum;
int MyFirst;
int MyLast;

{
  int i,j,k;
  double tot = 0.0;

  /* touch my data */
  for (j=0;j<2*(rootN-1);j++) {
    tot += upriv[j];
  }
  for (j=MyFirst; j<MyLast; j++) {
    k = j * (rootN + pad_length);
    for (i=0;i<rootN;i++) {
      tot += x[2*(k+i)] + x[2*(k+i)+1] +
             scratch[2*(k+i)] + scratch[2*(k+i)+1] +
         u[2*(k+i)] + u[2*(k+i)+1];
    }
  }
  return tot;
}


double CheckSum(N, x)

int N;
double *x;

{
  int i,j,k;
  double cks;

  cks = 0.0;
  for (j=0; j<rootN; j++) {
    k = j * (rootN + pad_length);
    for (i=0;i<rootN;i++) {
      cks += x[2*(k+i)] + x[2*(k+i)+1];
    }
  }

  return(cks);
}


void InitX(N, x)

int N;
double *x;

{
  int i,j,k;
  for (j=0; j<rootN; j++) {
    k = j * (rootN + pad_length);
    for (i=0;i<rootN;i++) {
      x[2*(k+i)] = 0.5;
      x[2*(k+i)+1] = 0.6;
    }
  }
}


void InitU(N, u)

int N;
double *u;

{
  int q;
  int j;
  int base;
  int n1;

  for (q=0; 1<<q<N; q++) {
    n1 = 1<<q;
    base = n1-1;
    for (j=0; j<n1; j++) {
      if (base+j > rootN-1) {
    return;
      }
      //u[2*(base+j)] = 1;
      u[2*(base+j)] = cos(2.0*PI*j/(2*n1));
      //u[2*(base+j)+1] = -0.5;
      u[2*(base+j)+1] = -sin(2.0*PI*j/(2*n1));
    }
  }
}


void InitU2(N, u, n1)

int N;
double *u;
int n1;

{
  int i,j,k;
//  int base;

  for (j=0; j<n1; j++) {
    k = j*(rootN+pad_length);
    for (i=0; i<n1; i++) {
      //u[2*(k+i)] = cos(2.0*PI*i*j/(N));
      u[2*(k+i)] = 0.7;
      //u[2*(k+i)+1] = -sin(2.0*PI*i*j/(N));
      u[2*(k+i)+1] = -0.8;
    }
  }
}


int BitReverse(M, k)

int M;
int k;

{
  int i;
  int j;
  int tmp;

  j = 0;
  tmp = k;
  for (i=0; i<M; i++) {
    j = 2*j + (tmp&0x1);
    tmp = tmp>>1;
  }
  return(j);
}


void FFT1D(direction, M, N, x, scratch, upriv, umain2, MyNum, l_transtime,
           MyFirst, MyLast, pad_length, P, test_result, doprint, dostats,
       Global)

int direction;
int M;
int N;
int *l_transtime;
double *x;
double *upriv;
double *scratch;
double *umain2;
int MyFirst;
int MyLast;
int pad_length;
int P;
int test_result;
int doprint;
int dostats;
struct GlobalMemory *Global;

{
//  int i;
  int j;
  int m1;
  int n1;
//  int flag = 0;
//  unsigned int clocktime1;
//  unsigned int clocktime2;

  m1 = M/2;
  n1 = 1<<m1;

  /* transpose from x into scratch */
  Transpose(n1, x, scratch, MyNum, MyFirst, MyLast, pad_length);

  /* do n1 1D FFTs on columns */
  for (j=MyFirst; j<MyLast; j++) {
    FFT1DOnce(direction, m1, n1, upriv, &scratch[2*j*(n1+pad_length)]);
    TwiddleOneCol(direction, n1, N, j, umain2, &scratch[2*j*(n1+pad_length)],
          pad_length);
  }

  /* transpose */
  Transpose(n1, scratch, x, MyNum, MyFirst, MyLast, pad_length);

  /* do n1 1D FFTs on columns again */
  for (j=MyFirst; j<MyLast; j++) {
    FFT1DOnce(direction, m1, n1, upriv, &x[2*j*(n1+pad_length)]);
    if (direction == -1)
      Scale(n1, N, &x[2*j*(n1+pad_length)]);
  }

  /* transpose back */
  Transpose(n1, x, scratch, MyNum, MyFirst, MyLast, pad_length);


}


void TwiddleOneCol(direction, n1, N, j, u, x, pad_length)

int direction;
int n1;
int N;
int j;
double *u;
double *x;
int pad_length;

{
  int i;
  double omega_r;
  double omega_c;
  double x_r;
  double x_c;
//  double r1;
//  double c1;
//  double r2;
//  double c2;

  for (i=0; i<n1; i++) {
    omega_r = u[2*(j*(n1+pad_length)+i)];
    omega_c = direction*u[2*(j*(n1+pad_length)+i)+1];
    x_r = x[2*i];
    x_c = x[2*i+1];
    x[2*i] = omega_r*x_r - omega_c*x_c;
    x[2*i+1] = omega_r*x_c + omega_c*x_r;
  }
}


void Scale(n1, N, x)

int n1;
int N;
double *x;

{
  int i;

  for (i=0; i<n1; i++) {
    x[2*i] /= N;
    x[2*i+1] /= N;
  }
}


void Transpose(n1, src, dest, MyNum, MyFirst, MyLast, pad_length)

int n1;
double *src;
double *dest;
int MyNum;
int MyFirst;
int MyLast;
int pad_length;

{
  int i;
  int j;
  int k;
  int l;
  int m;
  int blksize;
  int numblks;
  int firstfirst;
  int h_off;
  int v_off;
  int v;
  int h;
  int n1p;
  int row_count;

  blksize = MyLast-MyFirst;
  numblks = (2*blksize)/num_cache_lines;
  if (numblks * num_cache_lines != 2 * blksize) {
    numblks ++;
  }
  blksize = blksize / numblks;
  firstfirst = MyFirst;
  row_count = n1/P;
  n1p = n1+pad_length;
  for (l=MyNum+1;l<P;l++) {
    v_off = l*row_count;
    for (k=0; k<numblks; k++) {
      h_off = firstfirst;
      for (m=0; m<numblks; m++) {
        for (i=0; i<blksize; i++) {
      v = v_off + i;
          for (j=0; j<blksize; j++) {
        h = h_off + j;
            dest[2*(h*n1p+v)] = src[2*(v*n1p+h)];
            dest[2*(h*n1p+v)+1] = src[2*(v*n1p+h)+1];
          }
        }
    h_off += blksize;
      }
      v_off+=blksize;
    }
  }

  for (l=0;l<MyNum;l++) {
    v_off = l*row_count;
    for (k=0; k<numblks; k++) {
      h_off = firstfirst;
      for (m=0; m<numblks; m++) {
        for (i=0; i<blksize; i++) {
      v = v_off + i;
          for (j=0; j<blksize; j++) {
            h = h_off + j;
            dest[2*(h*n1p+v)] = src[2*(v*n1p+h)];
            dest[2*(h*n1p+v)+1] = src[2*(v*n1p+h)+1];
          }
        }
    h_off += blksize;
      }
      v_off+=blksize;
    }
  }

  v_off = MyNum*row_count;
  for (k=0; k<numblks; k++) {
    h_off = firstfirst;
    for (m=0; m<numblks; m++) {
      for (i=0; i<blksize; i++) {
        v = v_off + i;
        for (j=0; j<blksize; j++) {
          h = h_off + j;
          dest[2*(h*n1p+v)] = src[2*(v*n1p+h)];
          dest[2*(h*n1p+v)+1] = src[2*(v*n1p+h)+1];
    }
      }
      h_off += blksize;
    }
    v_off+=blksize;
  }
}


void CopyColumn(n1, src, dest)

int n1;
double *src;
double *dest;

{
  int i;

  for (i=0; i<n1; i++) {
    dest[2*i] = src[2*i];
    dest[2*i+1] = src[2*i+1];
  }
}


void Reverse(N, M, x)

int N;
int M;
double *x;

{
  int j, k;

  for (k=0; k<N; k++) {
    j = BitReverse(M, k);
    if (j > k) {
      SWAP(x[2*j], x[2*k]);
      SWAP(x[2*j+1], x[2*k+1]);
    }
  }
}


void FFT1DOnce(direction, M, N, u, x)

int direction;
int M;
int N;
double *u;
double *x;

{
  int j;
  int k;
  int q;
  int L;
  int r;
  int Lstar;
  double *u1;
  double *x1;
  double *x2;
  double omega_r;
  double omega_c;
  double tau_r;
  double tau_c;
  double x_r;
  double x_c;

  Reverse(N, M, x);

  for (q=1; q<=M; q++) {
    L = 1<<q; r = N/L; Lstar = L/2;
    u1 = &u[2*(Lstar-1)];
    for (k=0; k<r; k++) {
      x1 = &x[2*(k*L)];
      x2 = &x[2*(k*L+Lstar)];
      for (j=0; j<Lstar; j++) {
    omega_r = u1[2*j];
        omega_c = direction*u1[2*j+1];
    x_r = x2[2*j];
        x_c = x2[2*j+1];
    tau_r = omega_r*x_r - omega_c*x_c;
    tau_c = omega_r*x_c + omega_c*x_r;
    x_r = x1[2*j];
        x_c = x1[2*j+1];
    x2[2*j] = x_r - tau_r;
    x2[2*j+1] = x_c - tau_c;
    x1[2*j] = x_r + tau_r;
    x1[2*j+1] = x_c + tau_c;
      }
    }
  }
}




void printerr(s)

char *s;

{
  fprintf(stderr,"ERROR: %s\n",s);
}


int log_2(number)

int number;

{
  int cumulative = 1;
  int out = 0;
  int done = 0;

  while ((cumulative < number) && (!done) && (out < 50)) {
    if (cumulative == number) {
      done = 1;
    } else {
      cumulative = cumulative * 2;
      out ++;
    }
  }

  if (cumulative == number) {
    return(out);
  } else {
    return(-1);
  }
}

int main(argc, argv)

int argc;
char *argv;

{
    int numIter = 0;
//  int i;
//  int j;
//  int c;
//  extern char *optarg;
//  int m1;
  int factor;
  int pages;
//  unsigned int start;

  N = 1<<M;
  rootN = 1<<(M/2);
  rowsperproc = rootN/P;

  line_size = 1 << log2_line_size;
  if (line_size < 2*sizeof(double)) {
    factor = (2*sizeof(double)) / line_size;
    num_cache_lines = orig_num_lines / factor;
  }
  if (line_size <= 2*sizeof(double)) {
    pad_length = 1;
  } else {
    pad_length = line_size / (2*sizeof(double));
  }

  if (rowsperproc * rootN * 2 * sizeof(double) >= PAGE_SIZE) {
    pages = (2 * pad_length * sizeof(double) * rowsperproc) / PAGE_SIZE;
    if (pages * PAGE_SIZE != 2 * pad_length * sizeof(double) * rowsperproc) {
      pages ++;
    }
    pad_length = (pages * PAGE_SIZE) / (2 * sizeof(double) * rowsperproc);
  } else {
    pad_length = (PAGE_SIZE - (rowsperproc * rootN * 2 * sizeof(double))) /

                 (2 * sizeof(double) * rowsperproc);
    if (pad_length * (2 * sizeof(double) * rowsperproc) !=
        (PAGE_SIZE - (rowsperproc * rootN * 2 * sizeof(double)))) {
      printerr("Padding algorithm unsuccessful\n");
      exit(-1);
    }
  }

  Global = (struct GlobalMemory *) malloc(sizeof(struct GlobalMemory));
  x = (double *) malloc(2*(N+rootN*pad_length)*sizeof(double)+PAGE_SIZE);
  trans = (double *) malloc(2*(N+rootN*pad_length)*sizeof(double)+PAGE_SIZE);
  umain = (double *) malloc(2*rootN*sizeof(double));
  umain2 = (double *) malloc(2*(N+rootN*pad_length)*sizeof(double)+PAGE_SIZE);

  x = (double *) (((unsigned) x) + PAGE_SIZE - ((unsigned) x) % PAGE_SIZE);
  trans = (double *) (((unsigned) trans) + PAGE_SIZE - ((unsigned) trans) % PAGE_SIZE);
  umain2 = (double *) (((unsigned) umain2) + PAGE_SIZE - ((unsigned) umain2) % PAGE_SIZE);

  InitX(N, x);                  /* place random values in x */
  InitU(N,umain);               /* initialize u arrays*/
  InitU2(N,umain2,rootN);
  #ifdef SHORT_BENCH
  for(numIter = 0; numIter < 5; numIter++){
  #else
  for(numIter = 0; numIter < 500; numIter++){
  #endif
        SlaveStart(0);
    }
  return 0;
}


