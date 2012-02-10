/****************************************************************************/
/*         Copyright (C) 1996-1998 Texas Instruments Incorporated           */
/*                      All Rights Reserved                                 */
/*                                                                          */
/* C_FIR.C - Example from Programmer's Guide on optimizing C code.          */
/*                                                                          */
/* cl6x -qko3 c_fir.c -z -llnk.cmd -lrts6201.lib -o c_fir.out               */
/* load6x -q -k c_fir.out                                                   */
/*                                                                          */
/****************************************************************************/

typedef short restricted_short;

void fir3(const short x[], const short h[], short y[], int n, int m, int s);

#define N  32
#define M  32
#define S  24

short y1_[] =    { 0x0000, 0x0018, 0x8B84, 0xCD4C, 0x9713, 0x5719, 0x3E75,
                  0x9ADB, 0xFD0A, 0x173C, 0xF544, 0x8C8D, 0x8B14, 0x354E,
                  0xBF18, 0xC57C, 0x4D81, 0x239D, 0x85C6, 0xE58C, 0xEB0A,
                  0x8D04, 0x8E4E, 0x86EF, 0x2956, 0xDC6C, 0xFB5D, 0xBEA3,
                  0x6678, 0xB548, 0x4ACA, 0xA2F0 };

short y2[] =    { 0x0000, 0x0018, 0x8B84, 0xCD4C, 0x9713, 0x5719, 0x3E75,
                  0x9ADB, 0xFD0A, 0x173C, 0xF544, 0x8C8D, 0x8B14, 0x354E,
                  0xBF18, 0xC57C, 0x4D81, 0x239D, 0x85C6, 0xE58C, 0xEB0A,
                  0x8D04, 0x8E4E, 0x86EF, 0x2956, 0xDC6C, 0xFB5D, 0xBEA3,
                  0x6678, 0xB548, 0x4ACA, 0xA2F0 };

short x[] = { 0x1F6D, 0x7916, 0xD41B, 0xC0EC, 0x6BA0, 0xC5E3, 0x5100,
              0xA2F9, 0x7E67, 0xF2C6, 0x51D5, 0x5044, 0x2986, 0x9461,
              0xC322, 0xCC46, 0x56A9, 0xFC65, 0x701C, 0x9645, 0x8EEC,
              0xB80D, 0x7E24, 0x27D2, 0x7555, 0x0FA0, 0xF045, 0x784A,
              0x4B56, 0x88F2, 0x22B8, 0x7B57, 0x6B4E, 0x62E1, 0x5061,
              0x9F6E, 0xCD07, 0x1CDA, 0x0E51, 0x4E50, 0x8638, 0xED54,
              0xCF45, 0x708C, 0x4402, 0x4A51, 0x6023, 0xE7FA, 0xD574,
              0x65E2, 0x6C82, 0x123F, 0x9F09, 0xAAA2, 0xF821, 0x4D4D,
              0x2727, 0x4238, 0xE66C, 0x6EE2, 0x8DA0, 0x92D8, 0x74FD,
              0x4806, 0x0C33, 0xBBC1 };

short h[] = { 0xBC16, 0x2990, 0x7D0A, 0x1ABE, 0x342A, 0x5DA0, 0xD33B,
              0xC7A6, 0x2E53, 0xAB24, 0x9E49, 0x18DE, 0x57DB, 0xD556,
              0x89A2, 0x1DD4, 0x3AB6, 0x1B3A, 0xDE21, 0x2484, 0xBB04,
              0xB721, 0xFF8B, 0x35F6, 0x9E92, 0x5F2B, 0xED14, 0x94BB,
              0x0058, 0xC8BF, 0xC4DA, 0x4475 };

/****************************************************************************/
/* TOP LEVEL DRIVER FOR THE TEST.                                           */
/****************************************************************************/
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

    int i;
    #ifdef SHORT_BENCH
    int num_iter = 20;
    #else
    int num_iter = 2000; //int num_iter = 20;
    #endif

    /************************************************************************/
    /* TIME FIR3.                                                           */
    /************************************************************************/
    for(i=0;i<5;i++)
       fir3(x, h, y1_, N, M, S);
    for(i=0;i<num_iter;i++)
        fir3(x, h, y1_, N, M, S);
    return 0;
}


/****************************************************************************/
/* FIR3 - Lx optimized form                                                 */
/****************************************************************************/
void fir3(const short x[], const short h[], short y[], int n, int m, int s)
{
    int  j;
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9;
    int tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, tmp16, tmp17, tmp18, tmp19;

    for (j = 0; j < m; j++)
    {
        tmp0 = x[j] * h[0]  + x[j+1] * h[1];
        tmp1 = x[j+2] *  h[2]  + x[j+3] *  h[3];
        tmp2 = x[j+4] *  h[4]  + x[j+5] *  h[5];
        tmp3 = x[j+6] *  h[6]  + x[j+7] *  h[7];
        tmp4 = x[j+8] *  h[8]  + x[j+9] *  h[9];
        tmp5 = x[j+10] *  h[10] + x[j+11] *  h[11];
        tmp6 = x[j+12] *  h[12]+ x[j+13] *  h[13];
        tmp7 = x[j+14] *  h[14]+ x[j+15] *  h[15];
        tmp8 = x[j+16] *  h[16]  + x[j+17] *  h[17];
        tmp9 = x[j+18] *  h[18]  + x[j+19] *  h[19];
        tmp10 = x[j+20] *  h[20]  + x[j+21] *  h[21];
        tmp11 = x[j+22] *  h[22]  + x[j+23] *  h[23];
        tmp12 = x[j+24] *  h[24]  + x[j+25] *  h[25];
        tmp13 = x[j+26] *  h[26] + x[j+27] *  h[27];
        tmp14 = x[j+28] *  h[28]+ x[j+29] *  h[29];
        tmp15 = x[j+30] *  h[30]+ x[j+31] *  h[31];

    tmp16 = tmp0+tmp1+tmp2+tmp3;
    tmp17 = tmp4+tmp5+tmp6+tmp7;
        tmp18 = tmp8+tmp9+tmp10+tmp11;
        tmp19 = tmp12+tmp13+tmp14+tmp15;
        y[j] = (tmp16+tmp17+tmp18+tmp19)>>s;
    }
}


