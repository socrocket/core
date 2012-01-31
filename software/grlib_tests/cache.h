#ifndef CACHE_H
#define CACHE_H

void flush(void);
void ifill(int);
void dfill(int);
int getitag(int addr, int set);
void setitag(int addr, int set, int data);
void setidata(int addr, int set, int data);
int getidata(int addr, int set);
int asmgetitag(int addr);
void asmsetitag(int addr, int data);
int asmgetidata(int addr);
void asmsetidata(int addr, int data);
void wsysreg(int addr, int data);
int rsysreg(int addr);
void setdtag(int addr, int set, int data);
void setddata(int addr, int set, int data);
int chkdtag(int addr);
int getdtag(int addr, int set);
int getddata(int addr, int set);
void dma(int addr, int len,  int write);
int asmgetdtag(int addr);
void asmsetdtag(int addr, int data);
int asmgetddata(int addr);
void asmsetddata(int addr, int data);
void setudata(int addr,int data);
int getudata(int addr);
int asmgetudata(int addr);
int xgetpsr(void);
void setpsr(int psr);
void flushi(int addr,int data);
void flushd(int addr,int data);

extern int line0();
extern int line1();
extern int line2();
extern int line3();

int cachetest(void);
long long int getdw();

void cache_disable(void);
void cache_enable(void);
void ramfill(void);

#endif
