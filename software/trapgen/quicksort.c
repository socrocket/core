#include <stdlib.h>
#include <stdio.h>

#define SHORT_BENCH

#ifdef SHORT_BENCH
#define TO_SORT_SIZE 2048
#else
#define TO_SORT_SIZE 16384
#endif

void QS(int[] ,int,int);
void swap(int[],int,int);
int main(int argc, char * argv[])
{
 int o;
 int a[TO_SORT_SIZE];
 for (o=0;o<TO_SORT_SIZE;o++)
  a[o]=rand() % 16384;
 QS(a,0,TO_SORT_SIZE - 1);
 return 0;
}

void QS(int v[], int left, int right)
{
 int i,last;
 if (left>=right) return;
 swap(v,left,(left+right)/2);
 last = left;
 for (i=left+1;i<=right;i++)
  if (v[i]<v[left]) swap(v,++last,i);
 swap(v,left,last);
 QS(v,left,last-1);
 QS(v,last+1,right);
}

void swap(int v[],int i, int j)
{
 int temp;
 temp=v[i];
 v[i]=v[j];
 v[j]=temp;
}
