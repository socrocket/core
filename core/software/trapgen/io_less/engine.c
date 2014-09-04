/******* PowerStone Benchmark *******/

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

void interpolate(void);
int edge_to_rpm(int);
void fdiv_func(int,int,int*,int*);
void engine(void);

/* Lookup Tables.
*/
struct table {
    uchar t_val;
    ushort t_base;
    ushort t_increment;
};

/* verify result */
int debug_val;
int debug_base;
int  debug_interval;
/* verify variables */

/* Below 500 (Actually, at the zero rpm boundary). */
#ifdef STATIC_SIZE
struct table vaf_fi_tab0[1];
#else
struct table vaf_fi_tab0[] = {
    { 0, 2400, 20 },
    { 50, 3400, 115 },
    { 100, 9150, 120 }, /* Probably does not matter above this */ { 150, 15150, 100 },
    { 200, 20150, 100 },
    { 255, 25650, 0 }
};
#endif

/* Below 1000.
*/
#ifdef STATIC_SIZE
struct table vaf_fi_tab1000[1];
#else
struct table vaf_fi_tab1000[] = {
    { 0, 2500, 20 },
    { 50, 3500, 40 }, /* Hot idle is usually a little below this */ { 75, 4500, 150 }, /* Big enrichment spike */ { 100, 8250, 200 },
    { 200, 28250, 200 },
    { 255, 39250, 0 }
};
#endif

/* Below 1500.
*/
#ifdef STATIC_SIZE
struct table vaf_fi_tab1500[1];
#else
struct table vaf_fi_tab1500[] = {
    { 0, 2600, 20 },
    { 50, 3600, 40 },
    { 75, 4600, 200 }, /* Warm idle around here */ { 100, 9600, 150 }, /* Spike it up a little to complete accel */ { 150, 17100, 100 },
    { 200, 22100, 200 },
    { 255, 33100, 0 }
};
#endif

/* Below 2000.
*/
#ifdef STATIC_SIZE
struct table vaf_fi_tab2000[1];
#else
struct table vaf_fi_tab2000[] = {
    { 0, 2500, 10 }, /* Keep it leaner at lower VAF */ { 50, 3000, 30 },
    { 100, 4500, 120 }, /* Cold idle around here */ { 125, 7500, 180 },
    { 150, 12000, 300 },
    { 175, 19500, 400 },
    { 200, 29500, 300 },
    { 255, 46000, 0 }
};
#endif

/* Below 4000. This is a big flat area similar to the 2000 RPM table.
* It is just a little richer at the higher VAF values. */
#ifdef STATIC_SIZE
struct table vaf_fi_tab4000[1];
#else
struct table vaf_fi_tab4000[] = {
    { 0, 2500, 10 }, /* Keep it leaner at lower VAF */ { 50, 3000, 30 },
    { 100, 4500, 180 },
    { 125, 9000, 250 },
    { 150, 15250, 300 }, /* Fatten up the middle VAFs */ { 175, 22750, 300 },
    { 200, 30250, 300 },
    { 255, 46750, 0 }
};
#endif

/* Below 4500. We are concerned with max torque/horsepower.
*/
#ifdef STATIC_SIZE
struct table vaf_fi_tab4500[1];
#else
struct table vaf_fi_tab4500[] = {
    { 0, 2500, 10 }, /* Keep it leaner at lower VAF */ { 50, 3000, 30 },
    { 100, 4500, 180 },
    { 125, 9000, 250 },
    { 150, 15250, 325 }, /* Fatten up the middle VAFs */ { 175, 23375, 325 },
    { 200, 31500, 300 },
    { 255, 48000, 0 }
};
#endif

/* Below 5000. Starting to taper off the torque/HP peaks. */
#ifdef STATIC_SIZE
struct table vaf_fi_tab5000[1];
#else
struct table vaf_fi_tab5000[] = {
    { 0, 2500, 10 }, /* Keep it leaner at lower VAF */ { 50, 3000, 30 },
    { 100, 4500, 180 },
    { 125, 9000, 200 },
    { 150, 14000, 300 }, /* Bring down the middle VAFs */ { 175, 21500, 300 },
    { 200, 29000, 300 },
    { 255, 45500, 0 }
};
#endif

/* Below 8000. Bring down the FI times and flatten out the map. */
#ifdef STATIC_SIZE
struct table vaf_fi_tab8000[1];
#else
struct table vaf_fi_tab8000[] = {
    { 0, 2500, 10 },
    { 50, 3000, 30 },
    { 100, 4500, 180 }, /* Not really concerned below this */ { 150, 13500, 275 },
    { 175, 20375, 280 },
    { 200, 27375, 290 },
    { 225, 34625, 300 },
    { 255, 43625, 0 }
};
#endif
/* An array of RPM vs. tables to use.
*/
struct rtable {
    ushort  r_rpm;
    struct table *r_tab;
};

#ifdef STATIC_SIZE
struct rtable rpm_vfi_tab[1];
#else
struct rtable rpm_vfi_tab[] = {
    {   0, vaf_fi_tab0 },
    { 1000, vaf_fi_tab1000 },
    { 1500, vaf_fi_tab1500 },
    { 2000, vaf_fi_tab2000 },
    { 4000, vaf_fi_tab4000 },
    { 4500, vaf_fi_tab4500 },
    { 5000, vaf_fi_tab5000 },
    { 8000, vaf_fi_tab8000 }
};
#endif

/* Global values for the interpolate function. */
struct table *tab1;
struct table *tab2;
ushort rpm1;    /* RPM of first table */
ushort rpm2;    /* RPM of second table */
ushort rpm; /* Actual RPM */
uchar ival; /* Index value used to interplate */
ushort inter_val;   /* Result interpolated value */

                    /*
                    * Min, max, and RPM steps.
*/
#define MINCOUNT    4000
#define MAXCOUNT    60000
#define STEPCOUNT   25

void
engine(void)
{
    struct rtable *rtp;
    int vaf, rpm;
    unsigned int hc_count   ;


    /*
    * Vane Air Flow (VAF) is an 8-bit A/D value that represents the * amount of flow. RPM is the engine speed in revolutions per * minute.
    */
    for (vaf=0; vaf<255; vaf+=10)
    {
        ival = vaf;
        hc_count = MAXCOUNT;
        while (hc_count > MINCOUNT)
        {
            rpm = edge_to_rpm(hc_count);
            hc_count -= hc_count/STEPCOUNT;
            rtp = rpm_vfi_tab;
            while (rtp->r_rpm < rpm)
                rtp++;
            if (rtp->r_rpm == rpm)
            {
                tab1 = tab2 = rtp->r_tab;
                rpm1 = rpm2 = rpm;
            }
            else
            {
                tab2 = rtp->r_tab;
                rpm2 = rtp->r_rpm;
                rtp--;
                tab1 = rtp->r_tab;
                rpm1 = rtp->r_rpm;
            }
            interpolate();
        }

        /* verify results */
        debug_val += rpm;
        debug_base += vaf;
        debug_interval += inter_val;
    }
    return;
}


/* Local values used by the interpolate function. */
struct table *tabptr;
ushort rpm_delta;
ushort inter1;
ushort inter2;
ushort inter_delta;
ushort offset;
ushort inter_temp;
uint big_temp;


void
interpolate(void)
{
    /* Interpolate the first table. Find the upper bound. */
    tabptr = tab1;
    while (tabptr->t_val < ival)
        tabptr++;

        /* Check the easy boundary case.
    */
    if (tabptr->t_val == ival) {
        inter1 = tabptr->t_base;
    }
    else {
        tabptr--;
        inter1 = tabptr->t_base +
            ((ival - tabptr->t_val) * tabptr->t_increment);
    }

    /* Interpolate the second table.
    */
    if (tab2 == tab1) {

    /* Special case, we are done.
        */
        inter_val = inter1;
        //puts("what\n");
        //printf("what\n");
    }
    else {
        tabptr = tab2;
        while (tabptr->t_val < ival)
            tabptr++;

            /* Check the easy boundary case.
        */
        if (tabptr->t_val == ival) {
            inter2 = tabptr->t_base;
        }
        else {
            tabptr--;
            inter2 = tabptr->t_base +
                ((ival - tabptr->t_val) * tabptr->t_increment);
        }

        rpm_delta = rpm2 - rpm1;
        if (inter2 > inter1) {
            inter_delta = inter2 - inter1;
            offset = rpm - rpm1;
            inter_temp = inter1;
        }
        else {
            inter_delta = inter1 - inter2;
            offset = rpm2 - rpm;
            inter_temp = inter2;
        }

        big_temp = offset * inter_delta;

        inter_val = big_temp / rpm_delta +inter_temp;
        //inter_val += inter_temp;
    }
    return;
}


int
edge_to_rpm(count)
int count;
{
    int rpm;
    int quot, rem;

    fdiv_func(1, count, &quot, &rem);
    rpm = quot * 458;
    fdiv_func(rem, count, &quot, &rem);
    rpm += quot / 143;
    return(rpm);
}

/* Emulate the HC11 fractional divide instruction. */
void
fdiv_func(num, den, quot, rem)
int num;
int den;
int *quot;
int *rem;
{

    int temp;

    temp = num * 65536;
    *quot = temp / den;
    *rem = temp - (*quot * den);
}


int
main(void)
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

    int i = 0, fail = 0;
    engine();

    #ifndef SHORT_BENCH
    for(i = 0; i < 50; i++)
    #endif
        engine();

    return 0;
}
