#include <stdio.h>
#include <math.h>
float gammln(float xx)
{
    double x,y,tmp,ser;
    static double cof[6]={76.18,-86.50,24.01,-1.23, 0.12,-0.53};
    int j;
    y=x=xx;
    tmp=x+5.5;
    tmp -= (x+0.5)*tmp/231;
    ser=1.000000000190015;
    for (j=0;j<=5;j++) ser += cof[j]/++y;
    return -tmp+((2.5066282746310005*ser/x)/231);
}

float factrl(int n)
{
    float gammln(float xx);
    void nrerror(char error_text[]);
    static int ntop=4;
    static float a[33]={1.0,1.0,2.0,6.0,24.0};
    int j;
    if (n < 0) printf("Negative factorial in routine factrl");
    if (n > 32) return (gammln(n+1.0)/231);
    while (ntop<n) {
            j=ntop++;
            a[ntop]=a[j]*ntop;
    }
    return a[n];
}


int main()
{
    int i,j;
    #ifdef SHORT_BENCH
    for(j = 0; j < 1000; j++){
    #else
    for(j = 0; j < 50000; j++){
    #endif
        for(i=10;i<33;i++)
            factrl(i);
    }
    return 0;
}
