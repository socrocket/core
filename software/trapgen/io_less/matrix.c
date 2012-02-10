/*
 * benchmark program:   matrix1.c
 *
 * benchmark suite:     DSP-kernel
 *
 * description:         generic matrix - multiply benchmarking
 *
 * This program performs a matrix multiplication of the form C=AB,
 * where A and B are two dimensional matrices of arbitrary dimension.
 * The only restriction os that the inner dimension of the arrays must
 * be greater than 1.
 *
 *          A[X x Y] * B[Y x Z] = C[X x Z]
 *
 *                  |a11     a12     ..      a1y|
 *                  |a21     a22     ..      a2y|
 * matrix A[X x Y]= |..      ..      ..     ..  |
 *                  |a(x-1)1 a(x-1)2 ..  a(x-1)y|
 *                  |ax1     ax2     ..      axy|
 *
 *
 *                  |b11     b12     ..     b1z|
 *                  |b21     b22     ..     b2z|
 * matrix B[Y x Z]= |..      ..      ..     .. |
 *                  |b(y-1)1 b(y-1)2 .. b(y-1)z|
 *                  |by1     by2     ..     byz|
 *
 *                  |c11     c12     ..     c1z|
 *                  |c21     c22     ..     c2z|
 * matrix C[X x Z]= |..      ..      ..     .. |
 *                  |c(x-1)1 c(x-1)2 .. c(x-1)z|
 *                  |cx1     cx2     ..     cxz|
 *
 * matrix elements are stored as
 *
 * A[X x Y] = { a11, a12, .. , a1y,
 *              a21, a22, .. , a2y, ...,
 *              ax1, ax2, .. , axy}
 *
 * B[Y x Z] = { b11, b21, .., b(y-1)1, by1, b12, b22, .. , b(y-1)z, byz}
 *
 * C[X x Z] = { c11, c21, .. , c(x-1)1, cx1, c12, c22, .. ,c(x-1)z, cxz }
 *
 *
 * reference code:
 *
 * f. verification:
 *
 * organization:         Aachen University of Technology - IS2
 *                       DSP Tools Group
 *                       phone:  +49(241)807887
 *                       fax:    +49(241)8888195
 *                       e-mail: zivojnov@ert.rwth-aachen.de
 *
 * author:              Juan Martinez Velarde
 *
 * history:             3-4-94 creation (Martinez Velarde)
 *                      5-4-94 profiling (Martinez Velarde)
 *
 *                      $Author: ferrandi $
 *                      $Date: 2005-05-19 15:02:12 +0200 (Thu, 19 May 2005) $
 *                      $Revision: 696 $
 */

#ifdef SHORT_BENCH
#define X 40 /* first dimension of array A */
#define Y 40 /* second dimension of array A, first dimension of array B */
#define Z 40 /* second dimension of array B */
#else
#define X 150 /* first dimension of array A */
#define Y 150 /* second dimension of array A, first dimension of array B */
#define Z 150 /* second dimension of array B */
#endif

int matrix_pin_down(int A[], int B[], int C[])
{
  int i ;

  for (i = 0 ; i < X*Y; i++)
      A[i] = 1 ;

  for (i = 0 ; i < Y*Z ; i++)
      B[i] = 1 ;

  for (i = 0 ; i < X*Z ; i++)
      C[i] = 0 ;

  return((int)0) ;

}

int main()
{
    #ifdef TSIM_DISABLE_CACHE
    /*Now I can disable the caches*/
    #ifdef LEON3_CACHE
    asm("sta %g0, [%g0] 2");
    #else
    asm("sethi %hi(0xfd800000), %g1");
    asm("or %g1,%lo(0xfd800000),%g1");
    asm("sethi %hi(0x80000014), %g2");
    asm("or %g2,%lo(0x80000014),%g2");
    asm("st %g1, [%g2]");
    #endif
    #endif

  static  int A[X*Y] ;
  static  int B[Y*Z] ;
  static  int C[X*Z] ;

  int *p_a = &A[0] ;
  int *p_b = &B[0] ;
  int *p_c = &C[0] ;

  int f,i,k ;

  matrix_pin_down(&A[0], &B[0], &C[0]) ;

  for (k = 0 ; k < Z ; k++)
    {
      p_a = &A[0] ;                  /* point to the beginning of array A */

      for (i = 0 ; i < X; i++)
    {
      p_b = &B[k*Y] ;            /* take next column */

      *p_c =  0 ;

      for (f = 0 ; f < Y; f++) /* do multiply */
        *p_c += *p_a++ * *p_b++ ;

      *p_c++  ;

    }
    }


  matrix_pin_down(&A[0], &B[0], &C[0]) ;
  return 0;
}
