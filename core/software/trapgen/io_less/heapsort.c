/*-------------------Start heapsort.c program-------------------*/

/****************************************************************/
/*                         HEAPSORT                             */
/*                     C Program Source                         */
/*          Heapsort program for variable sized arrays          */
/*                 Version 1.0, 04 Oct 1992                     */
/*                Al Aburto, aburto@nosc.mil                    */
/*                                                              */
/* Based on the Heap Sort code in 'Numerical Recipes in C' by   */
/* William H. Press, Brian P. Flannery, Saul A. Teukolsky, and  */
/* William T. Vetterling, Cambridge University Press, 1990,     */
/* ISBN 0-521-35465-X.                                          */
/*                                                              */
/* The MIPS rating is based upon the program run time (runtime) */
/* for one iteration and a gcc 2.1 unoptimized (gcc -DUNIX)     */
/* assembly dump count of instructions per iteration for a i486 */
/* machine (assuming 80386 code).  This is the reference used.  */
/*                                                              */
/* The maximum amount of memory allocated is based on the 'imax'*/
/* variable in main(). Memory size = (2000*sizeof(long))*2^imax.*/
/* imax is currently set to 8, but this value may be increased  */
/* or decreased depending upon your system memory limits. For   */
/* standard Intel PC CPU machines a value of imax = 3 must be   */
/* used else your system may crash or hang up despite code in   */
/* the program to prevent this.                                 */
/****************************************************************/

#ifdef SHORT_BENCH
#define J_SIZE 32
#define K_SIZE 4
#else
#define J_SIZE 2000
#define K_SIZE 64
#endif

long bplong,ErrorFlag;

long NLoops[21];

void HSORT();

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

long  i,j,k,p,imax;

bplong = sizeof(long);

for( i=0 ; i<= 20 ; i++)
{
 NLoops[i] = 1;
}

HSORT();

return 0;
}                                  /* End of main */


/*************************/
/*  Heap Sort Program    */
/*************************/

void HSORT()
{

register long *base;
long tempStorage[J_SIZE*4];
register long i,j,k,l;
register long size;

long  iter,iran,ia,ic,im,ih,ir;
long  count,ca,cb,cc,cd,ce,cf;

size = J_SIZE - 1;
base = tempStorage;

ia = 106;
ic = 1283;
im = 6075;
ih = 1001;

   ErrorFlag = 0L;

   if( !base )
     {
     ErrorFlag = 2L;
     return;
     }

   count = 0;
   for(iter=1 ; iter<=K_SIZE ; iter++)       /* Do 'n' iterations */

   {
       iran = 47;                        /* Fill with 'random' numbers */
       for(i=1 ; i<=size ; i++)
       {
       iran = (iran * ia + ic) % im;
       *(base+i) = 1 + (ih * iran) / im;
       }

       k = (size >> 1) + 1;              /* Heap sort the array */
       l = size;
       ca = 0; cb = 0; cc = 0;
       cd = 0; ce = 0; cf = 0;

       for (;;)
       {
       ca++;
       if (k > 1)
       {
	  cb++;
	  ir = *(base+(--k));
       }
       else
       {
	  cc++;
	  ir = *(base+l);
	  *(base+l) = *(base+1);
	  if (--l == 1)
	  {
	       *(base+1) = ir;
	       goto Done;
	  }
       }

       i = k;
       j = k << 1;

       while (j <= l)
       {
	  cd++;
	  if ( (j < l) && (*(base+j) < *(base+j+1)) ) ++j;
	  if (ir < *(base+j))
	  {
	       ce++;
	       *(base+i) = *(base+j);
	       j += (i=j);
	  }
	  else
	  {
	       cf++;
	       j = l + 1;
	  }
       }
       *(base+i) = ir;
       }
Done:
   count = count + ca;
   }

   ir = count / K_SIZE;
   ir = (ir + ca) / 2;
}

