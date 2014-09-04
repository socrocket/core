/******* PowerStone Benchmark *******/

#include <math.h>
#include <stdio.h>
#include <string.h>

/* PERSONAL ALIASES */

#define TRUE  1
#define FALSE 0
#define NOT   !
#define AND   &&
#define OR    ||
#define const static
#define dx    int
#define d8    unsigned char
#define d32   unsigned long int

/* EXTERNAL LOCAL FILE CALLS */

extern d32   error_corr(register d32);
extern void  fix_bit(register char);
extern void  find_syndromes(void);
extern char  normalized_locator(void);
extern void  validation(void);
extern dx    comp32(register d32,register d32);
extern d32   *sync_find(register d32 *);
extern dx    addr_corr(register d32);
extern dx    num_proc(register dx,register d32);
extern dx    alpha_proc(register dx,register d32);
extern dx    msg_proc(register d32 *);
extern int  main(void);

/*  file: err_corr.c
*
*   This routine provides up to two bits of error correction
*   for a BCH(31,21) word with one extra bit set for overall
*   even word parity.
*
*   Based on algorithms derived by Len Nelson
*   (Motorola Private Paging) as described in his paper
*   titled "An Algebraic Method of 2 Bit Error Correction
*   Decoding for Binary BCH Codes" dated 10/15/90, citing
*   the following publications:
*
*   "Error Correction Coding for Digital Communications"
*   by George C. Clark and Bibb J. Cain.
*
*   "Essentials of Error-Control Coding Techniques"
*   by Hedeki Imai.
*
*   "Error Control Techniques for Digital Communications"
*   by Arnold M. Michelson and Allan H. Levesque.
*
*/

#define UNDEFINED

#define EVEN_PARITY_MASK    0x00000001
#define ERROR_MASK      0x00000001
#define BIT_MASK        0x80000000

/*      Global Variables        */

d32  hexword, parity;
dx   error_code;
char s1, s3;
void fix_bit (register char eroot);

/****End Function ptototypes****/

/*  Error Corrector Tables in ROM */

const char alpha[] =
{
    0x01, 0x02, 0x04, 0x08, 0x10, 0x05,
        0x0A, 0x14, 0x0D, 0x1A, 0x11, 0x07,
        0x0E, 0x1C, 0x1D, 0x1F, 0x1B, 0x13,
        0x03, 0x06, 0x0C, 0x18, 0x15, 0x0F,
        0x1E, 0x19, 0x17, 0x0B, 0x16, 0x09,
        0x12
};  /* 5-tuple vectors for bits 0 to 30 (power of a) */

const char alpha3[] =
{
    0x01, 0x08, 0x0A, 0x1A, 0x0E, 0x1F,
        0x03, 0x18, 0x1E, 0x0B, 0x12, 0x04,
        0x05, 0x0D, 0x07, 0x1D, 0x13, 0x0C,
        0x0F, 0x17, 0x09, 0x02, 0x10, 0x14,
        0x11, 0x1C, 0x1B, 0x06, 0x15, 0x19,
        0x16
};  /* 5-tuple vectors for bits 0 to 30 (power of a^3) */

const char alpha_inv[] =
{
    0x00, 0x00, 0x01, 0x12, 0x02, 0x05,
        0x13, 0x0B, 0x03, 0x1D, 0x06, 0x1B,
        0x14, 0x08, 0x0C, 0x17, 0x04, 0x0A,
        0x1E, 0x11, 0x07, 0x16, 0x1C, 0x1A,
        0x15, 0x19, 0x09, 0x10, 0x0D, 0x0E,
        0x18, 0x0F
};  /* 'power of a' using a 5-tuple vector as the index */

const char roots[] =
{
    0x00, 0x80, 0x03, 0x80, 0x06, 0x80,
        0x01, 0x80, 0x80, 0x07, 0x80, 0x0D,
        0x80, 0x0F, 0x80, 0x1A, 0x0C, 0x80,
        0x0B, 0x80, 0x02, 0x80, 0x08, 0x80,
        0x80, 0x09, 0x80, 0x11, 0x80, 0x04,
        0x80, 0x15
        };  /* 'power of a' root using 5-tuple of normalized locator
as the index */

/****End of Error Corrector Tables****/



/*
*
*   This routine operates on a 32-bit BCH code word
*   contained in hexword,
*   with the even parity bit in the LSB of the last byte
*   (index of 3) and attempts to correct the word
*   for up to two bits of error.  If the word can be
*   corrected, the overall parity is checked.
*
*   The left most bit (MSB) in hexword[0] is bit 30
*   of the BCH code word, and the bit immediately
*   to the left of the even parity bit
*   (LSB of hexword[0]) is bit 0 of the codeword.
*
*   A global variable called 'error_code' is used
*   to keep track of the total number of errors found.
*   Bit corrections are made to the 'hexword' array
*   elements.  If more than two errors are found,
*   or the word is uncorrectable, the 'hexword' array
*   elements are not modified.
*
*/

d32 err_corr (register d32 datain)

{
    char locator_constant, root2;
    signed char root1;

    hexword = datain;

    /* find syndromes s1 and s3, and check overall parity */

    find_syndromes ();

    /* if both syndromes are zero, there are no errors */

    if (s1 == 0 && s3 == 0)

        error_code = 0;

    else        /* at least one error exists */

    {
        locator_constant = normalized_locator ();

        if (locator_constant == 0)

        {

            /* only one error found and the power of S1 */
            /* is the bit position of the error */

            root1 = alpha_inv[s1];

            /* fix the bit in error */

            fix_bit (root1);

            error_code = 1;

        }

        else    /* more than one error exists */

        {
            /* look up the first root */

            root1 = roots[locator_constant];

            /* if the root doesn't exist the word cannot be corrected */
            /* roots that don't exist appear as negative numbers */

            if (root1 < 0)

                error_code = 4;

            else

            {

                /* the bit position of 1st error = S1(root) */

                root1 = alpha_inv[s1] + root1;

                /* place error within bits 0 to 30 */

                while (root1 > 30)

                    root1 -= 31;

                /* fix first error */

                fix_bit (root1);

                /* position of 2nd error = S1 + S1(root) */

                root2 = s1 ^ alpha[root1];

                root2 = alpha_inv[root2];

                /* fix second error */

                fix_bit (root2);

                /* check the overall parity */

                if (parity)
                    error_code = 3;
                else
                    error_code = 2;
            }
        }
    }

    /* printf("error code %d, corrected BCH = %x\n\n", error_code, hexword);  */

    return(hexword);

}   /****End correct (hexword)****/



    /* given the location of the error in eroot, fix the error
in hexword  in the following function */

void fix_bit (register char eroot)

{

    eroot++;    /* adjust bit position to be 1 to 31 */

    hexword = hexword ^ (ERROR_MASK << eroot);

}



/*
*
*   This routine calculates the S1 and S3 syndromes
*   of the received BCH code word and checks the
*   overall parity.
*
*   This routine operates on the received 32-bit
*   BCH code word found in the globally defined array
*   hexword[4] with the even parity bit in the LSB
*   of the last byte (index of 3).  The S1 ans S3
*   syndromes are calculated using table lookups.
*   The syndromes are saved in global variables
*   's1' and 's3' in their 5-tuple
*   vector form.
*
*   The 32-bit word is also checked for even parity.
*   If the overall parity of the received word is even,
*   a 0 is placed in the global variable 'parity',
*   otherwise a 1 is stored in 'parity '.
*
*   This routine uses two 5-tuple vector lookup tables,
*   'alpha' and 'alpha cubed'.  The 'alpha' table could
*   also be used to calculate syndrome s3 as follows:
*
*           s3 ^= alpha[(i*3)%31];
*
*   or, without the multiplication and modulo operators:
*
*           j = i + i + i;
*           while (j > 30);
*               j -= 31;
*           s3 ^= alpha[j];
*
*/

void find_syndromes ()

{

    d32 mask = BIT_MASK;
    dx  i;

    /* initialize syndromes */

    s1 = 0;

    s3 = 0;

    /* save the even parity bit of the BCH code word */

    parity = hexword & EVEN_PARITY_MASK;

    i = 30;

    while (i >= 0)

    {

        /* if hexword[MSB] == 1 */

        if (hexword & mask)

        {

            /* compute syndromes */

            s1 ^= alpha[i];

            s3 ^= alpha3[i];

            /* update parity count */

            parity++;
        }

        mask >>= 1;

        i--;

    }

    /* compute even parity bit */

    parity &= EVEN_PARITY_MASK;

}   /****End find_syndromes (hexword)****/



    /*
    *
    *   This routine calculates the constant term
    *   of the normalizederror locator polynomial
    *   described below:
    *
    *       R'(y) = y^2 + y + [1 + S3/(S1)^3]
    *
    *   Specifically, the constant is 1 + S3/(S1)^3
    *
    *   The arguments passed to this routine are
    *   the 5-tuple vectors of syndromes S1 and S3.
    *   The calculated constant is returned
    *   as a 5-tuple vector.
    *
*/

char normalized_locator ()

{
    char power_s1;
    signed char tau;

    /* convert s1 to power of a */

    power_s1 = alpha_inv[s1];

    /* convert s3 to power of a, then find S3/(S1)^3 */

    tau = alpha_inv[s3] - power_s1 - power_s1 - power_s1;

    /* make sure tau is between 0 and 30 */

    while (tau < 0)

        tau += 31;

    /* convert tau to it's 5-tuple vector and */
    /* add 1 to the vector then return the result */

    return (alpha[tau] ^ 1);

}   /****End normalized_locator ()****/



/* CONSTANT DEFINITIONS */

#undef STATIC_SIZE

#ifdef STATIC_SIZE
#define MAX_MSG       1
#else
#define MAX_MSG       256         /* maximum length of message */
#endif
#define PREAMBLE      0xaaaaaaaa  /* preamble pattern */
#define POCSAG_SYNC   0x3e4ba81b  /* POCSAG sync word */
#define CAPCODE_A     0x2a74e     /* 19-bit Capcode A */
#define CAPCODE_B     0x1d25a     /* 19-bit Capcode B */
#define MSG_MASK      0xfffff     /* 20-bit data message mask */
#define ALPHA_MASK    0x7f        /* 7-bit mask for alphanumeric data */
#define NUM_MASK      0xf         /* 4-bit mask for numeric data */
#define FUNC_MASK     0x3         /* 2-bit function code mask */
#define BAD_BYTE      0x80        /* bad message byte (uncorrectable) */

/* EXTERNAL VARIABLES */

extern dx error_code;             /* output error code for given code word, */
/* error_code = 0, no errors exist */
/*            = 1, 1 error existed and is fixed */
/*            = 2, 2 errors existed and are fixed */
/*            = 3, 3 errors exist and are uncorrectable */
/*            = 4, error is uncorrectable */

/* GLOBAL VARIABLES */

d8  msg[MAX_MSG];                 /* MAX_MSG x 8 numeric or alpha message array */
dx  alpha_count=0;                /* initialize alphanumeric loop count */
dx  func;                         /* 2-bit function code of current page */

/* LOOK-UP TABLES */

/*
* 256-byte table lookup to determine the number of 1's in
* a given byte
*/

const d8 err_tab[] =
#ifdef STATIC_SIZE
{ 0 };
#else
{

    0x0,0x1,0x1,0x2,0x1,0x2,0x2,0x3,0x1,0x2,0x2,0x3,0x2,0x3,0x3,0x4,
        0x1,0x2,0x2,0x3,0x2,0x3,0x3,0x4,0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,
        0x1,0x2,0x2,0x3,0x2,0x3,0x3,0x4,0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,
        0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,
        0x1,0x2,0x2,0x3,0x2,0x3,0x3,0x4,0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,
        0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,
        0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,
        0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,0x4,0x5,0x5,0x6,0x5,0x6,0x6,0x7,
        0x1,0x2,0x2,0x3,0x2,0x3,0x3,0x4,0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,
        0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,
        0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,
        0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,0x4,0x5,0x5,0x6,0x5,0x6,0x6,0x7,
        0x2,0x3,0x3,0x4,0x3,0x4,0x4,0x5,0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,
        0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,0x4,0x5,0x5,0x6,0x5,0x6,0x6,0x7,
        0x3,0x4,0x4,0x5,0x4,0x5,0x5,0x6,0x4,0x5,0x5,0x6,0x5,0x6,0x6,0x7,
        0x4,0x5,0x5,0x6,0x5,0x6,0x6,0x7,0x5,0x6,0x6,0x7,0x6,0x7,0x7,0x8

};
#endif

/* FUNCTIONS */

/*
* this routine accepts two 32-bit words, sample and reference,
* and returns the number of mismatches
*/

dx comp32(register d32 sample,register d32 reference)

{
    dx i, num_err;
    sample = sample ^ reference;             /* compare operands */
    num_err = 0;                             /* initialize # of errors to be zero */
    i = 4;                                   /* initialize to 4 iterations thru error */
    /* accumulation loop */
    while (i > 0)
    {
        num_err = num_err +
            err_tab[(d8) sample];        /* look-up errors in 256x8 table using */
        /* 8 LSB's of sample */
        sample    = sample >> 8;               /* align next 8-bits for error accumulation */

        i--;

    }

    return(num_err);                         /* return the number of mismatches */

}

/*
* This routine looks for a sync word match during a valid 576-bit
* sync window. If a sync match is found during the specified window,
* a pointer to the next code word is returned, otherwise a "0" is returned.
*/

d32 *sync_find(register d32 *dptr)

{
    dx  num_err;                             /* number of match errors */
    dx  sync_window=18;                      /* set sync window to 18 x 32 = 576 bits */
    dx  bit_cnt;                             /* keeps track of the number of bits shifted */
    /* out of sample2 */
    d32 sample1, sample2;                    /* two 32-bit data samples from SPI */

    sample1 = *dptr++;                       /* acquire first sample data from SPI */

    while (sync_window > 0)                  /* while sync window is less than 576 bits */

    {

        sample2 = *dptr++;                     /* acquire second sample data from SPI */

        bit_cnt = 0;                           /* initialize bit counter */

        while (bit_cnt < 32)                   /* while sample2 is still being shifted */
            /* into sample1 */

        {

            num_err = comp32(sample1, POCSAG_SYNC); /* compare to POCSAG sync word */

            if (num_err <= 2)                       /* if sync word match */

                return(dptr);                         /* return pointer to next code word */

            num_err = comp32(sample1, PREAMBLE);    /* otherwise, compare to preamble */

            if ((num_err <= 2) OR (num_err >= 30))  /* if preamble match */

            {

                sample1 = sample2 >> bit_cnt;      /* flush preamble data sample and reset it to */
                /* a properly aligned sample2 */

                sync_window = 19;                  /* reset sync window (to 18 due to decrement later) */

                break;                             /* exit inner while */

            }

            else                                 /* shift MSB of sample2 into LSB of sample1 */

            {

                sample1 = (sample1 << 1) |         /* shift MSB of sample2 into LSB of sample1 */
                    (sample2 >> 31);

                sample2 = sample2 << 1;            /* update sample2 */


            }

            bit_cnt++;                           /* update bit counter */

        }

        sync_window--;                         /* update sync window */

    } /* outer while */

    return(NULL);                            /* return sync acquisition failure */

}

/*
* This function looks for a capcode A or B match in frame 4.
* If the number of mismatches is less or equal to 2, 2-bit
* error correction is attempted. At its completion, a
* re-matching process is attempted with the corrected word.
* If there are no errors, a "1" is returned to indicate that
* the address correlation was successful. Otherwise, a "0"
* is returned to indicate correlation failure.
*/

dx addr_corr(register d32 data)

{

    dx  num_err;                        /* number of match errors */
    d32 addr;                           /* 19-bit address code (including message flag bit) */
    d32 capcode=CAPCODE_A;              /* initialize capcode variable to capcode A */

    addr    = data >> 13;               /* extract address portion from code word */

    num_err = comp32(addr, capcode);    /* compare address to capcode A */

    if (num_err > 2)                    /* if greater than 2 mismatches */

    {

        capcode = CAPCODE_B;              /* set capcode to capcode B */

        num_err = comp32(addr, capcode);  /* compare address to capcode B */

    }

    if (num_err <= 2)                   /* if 2 mismatches or less */

    {

        data = err_corr(data);            /* perform up to 2-bit error correction on orig "data" */

        if (error_code < 3)               /* if error correction successful */

        {

            if (error_code != FALSE)        /* if 1 or 2 errors were fixed */

            {

                addr    = data >> 13;         /* re-extract address from corrected code word */

                num_err = comp32(addr, capcode); /* perform full correlation to preset capcode */

            }

            if (num_err == FALSE)            /* if no mismatches exist */

            {

                func = (d8) ((data >> 11) &
                    FUNC_MASK);      /* extract 2-bit function code global variable */

                return(TRUE);                 /* address correlation successful */

            }

        }

    }

    return(FALSE);                      /* address correlation unsuccessful */

}


/*
* This function processes a 20-bit numeric data message and index into message
* storage array where the storage of this message is to begin. It extracts and
* stores 5 numerals from the indicated message in msg[] array. Also, it returns
* the array index where the next storage element in next message is to be stored.
*/

dx num_proc(register dx i,register d32 codeword)

/* "i" is index into msg[] array where message */
/* is to be stored */
/* "codeword" is 20-bit data code word */

{

    dx count=5;                              /* set numeric data count in current message */
    dx shift=0;                              /* reset numeric shift amount */
    d8 digit;                                /* extracted numeral digit */

    while (count > 0)

    {
        digit = (d8) ((codeword >> shift) &
            NUM_MASK);            /* extract digit */

        if (error_code < 3)                    /* if error correction successful */
        {
            if (digit < 0xa)                     /* if digit is BCD character: 0 .. 9 */
                msg[i] = digit + 0x30;             /* store as 7-bit ASCII */
            else                                 /* otherwise, for the other symbols */

                msg[i] = digit;                    /* store as is */

        }

        else                                   /* error correction unsuccessful */

            msg[i] = BAD_BYTE;                   /* set stored numeral as bad data */

        shift = shift + 4;                     /* update numeric shift amount */

        i++;                                   /* update message array index */

        count--;                               /* update numeric data count */

    }

    return(i);                               /* return new array index */

}

/*
* This function processes a 20-bit alphanumeric message and index into message
* storage array where its storage is to begin. It extracts and stores
* 2 and 7/8 characters in msg[] array. Refer to pp. 47 of Rattler software
* description to understand this function better.
*/

dx alpha_proc(register dx i,register d32 codeword)

/* "i" is index into msg[] array where message */
/* is to be stored */
/* "codeword" 20-bit msg code word */

{

    dx num_bits_left;                               /* number of alpha bits left from */
    /* previous alpha code word */
    d8 rem_bits;                                    /* actual remainder bits in new code word */
    dx char_count=3;                                /* intialize new char extract count */
    dx char_shift=0;                                /* initialize char shift amount */

    if (alpha_count > 0)                            /* if NOT first iteration in alpha loop, */

    {                                               /* derive the left over char */

        num_bits_left = 7 - alpha_count;              /* compute number of bits left over */

        rem_bits      = (d8) (codeword
            << num_bits_left);      /* align remainder bits */

        if (error_code < 3)                           /* error correction successful */

            msg[i]      = (msg[i] | rem_bits) &
            ALPHA_MASK;                  /* compute left over char; */
        /* if left over char was a bad char, */
        /* its MSB is already set to a "1" */

        else                                          /* error correction unsuccessful */

            msg[i]      = BAD_BYTE;                     /* store left over char as bad char */

        i++;                                          /* update array index */

        char_shift = alpha_count;                     /* set shift amount for 1st new char */

    }

    while (char_count > 0)                          /* while char count is less than 3, */
        /* derive the 3 new chars */
    {

        if (error_code < 3)                           /* error correction successful */

            msg[i]   = (d8) ((codeword >> char_shift) &
            ALPHA_MASK);               /* compute new char */

        else

            msg[i]  = BAD_BYTE;                         /* store new char as bad char */

        char_shift = char_shift + 7;                  /* compute new char shift amount */

        i++;                                          /* update message array index */

        char_count--;                                 /* update new char count */

    }

    i--;                                            /* update msg array index to point to */
    /* left over char or new char (if */
    /* alpha_count is 6) */

    if (alpha_count == 6)                           /* if terminal alpha loop count */

        alpha_count = 0;                              /* reset alpha loop count */

    else

        alpha_count++;                                /* update alpha loop count */

    return(i);                                      /* return msg array index */

}


/*
* This function first performs address correlation. If it succeeds,
* it processes a numeric or alphanumeric message storing the output
* message in the msg[] array. This message is terminated by either 2
* consecutive bad syncs or if an address code word correlation is
* successful. The indicated function returns the length of the message
* array as its function output.
*/

dx msg_proc(register d32 *dptr)

{

    dx  i=0;                                 /* initialize message array index */
    dx  word_count=0;                        /* initialize word count for new batch; it can */
    /* take on values between 0 and 16 where 16 is */
    /* the sync word slot */
    dx  num_bad_sync=0;                      /* initialize bad sync count */
    dx  num_err;                             /* number of mismatches variable */
    dx  addr_search_mode=TRUE;               /* initialize address (capcode) search mode */
    d32 addr;                                /* 19-bit extracted address field */
    d32 data;                                /* SPI data */

    while (TRUE)                             /* for duration of message */

    {

        data = *dptr++;                        /* read next codeword */

        /* if new batch */

        if (word_count == 16)                  /* if sync word slot, acquire sync */

        {

            num_err = comp32(data, POCSAG_SYNC); /* compute # of mismatches */

            if (num_err <= 2)                    /* sync word match */

                num_bad_sync = 0;                  /* reset bad sync count */

            else                                 /* sync acquisition failure */

            {

                if (num_bad_sync == 1)             /* if 2 consecutive bad syncs */

                    return(i);                       /* return message array length */

                else

                    num_bad_sync++;                  /* update the bad sync count */

            }

            word_count = 0;                      /* reset word count for new batch */

        }


        /* otherwise, perform address correlation or message processing */

        else

        {

            if (addr_search_mode)                    /* if in initial address search mode */

            {

                if ((word_count == 6) OR               /* if frame 4 (capcode frame) */
                    (word_count == 7))

                {

                    if (addr_corr(data))              /* if address correlation success */
                    {
                        addr_search_mode = FALSE;       /* end of initial address search */
#ifdef DEBUG
                        printf("Address Correlation Successful\n");
#endif
                    }
                }
            }
            else                          /* otherwise, continue to message decode */
            {
                data = err_corr(data);                 /* perform 2-bit error correction on BCH data */

                /* if frame 4, attempt to address correlate first */
                if ((word_count == 6) OR               /* if frame 4 (capcode frame) */
                    (word_count == 7))
                {
                    num_err = TRUE;  /* initialize number of address correlation errors */
                    if (error_code < 3)              /* if error correction successful, */
                        /* perform address correlation if right frame */
                    {
                        addr = data >> 13;      /* extract address portion from code word */
                        num_err = comp32(addr, CAPCODE_A);   /* correlate to capcode A */
                        if (num_err)           /* if capcode A address correlation unsuccessful */
                            num_err = comp32(addr, CAPCODE_B); /* perform address correlation to capcode B */
                    }
                    if (num_err == FALSE)    /* if address correlation successful, */
                        return(i);   /* terminate message and return message array length */
                }

                /* otherwise, since address correlation unsuccessful, continue to process message */

                data = (data >> 11) & MSG_MASK;      /* extract 20-bit message */

                switch (func)           /* process message according to function code */
                {
                case 0:                            /* process NUMERIC code word */
                    i = num_proc(i, data);           /* process next numeric data word, and */
                    /* return new array index */


                    break;


                case 3:                            /* process ALPHANUMERIC code word */

                    i = alpha_proc(i, data);         /* process next alphanumeric data word, */
                    /* and return new array index */

                    break;

                default: return(FALSE);            /* message array not updated */

                } /* end switch */

            } /* end message decode else */

            word_count++;                            /* update word count in current batch */

        } /* end not sync slot else */

  } /* end while */

}

/* MAIN MODULE */

/*
* This main module searches thru preamble for sync acquisition. Once
* this is achieved it performs address correlation and if it is
* successful, it processes a single numeric or alphanumeric message
*/

/* file: alpha.h   */
/* int len = 59;   NOT USED */
unsigned long alpha_data[] = {
    0xaa8a2aaa,
        0x55551545,
        0x11555555,
        0xaaa22aaa,
        0x2a8aaaaa,
        0x55415555,
        0xafaaaaaa,
        0xf5555555,
        0xaa2aaaaa,
        0x5555555f,
        0x50555555,
        0x51555545,
        0xaaaa0aaa,
        0x2aaaaaa8,
        0xaae2aaaa,
        0x5555053e,
        0x4ba90b00,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x3a4b5832,
        0xc396212e,
        0x99072006,
        0xe688b86e,
        0x8efbf05e,
        0x90e5a59e,
        0xf120f237,
        0xe3dd935e,
        0xc3b146cd,
        0x3e0ba81b,
        0x975f3e6d,
        0xd2ce5ebe,
        0xfb2281a8,
        0x883aeea9,
        0xfb61d407,
        0xea4191c5,
        0xe9cfabbb,
        0x97221e4b,
        0xa6fc78b4,
        0x8c9972c8,
        0x9bb1a0f9,
        0xdc3847cf,
        0xe8c4b1e1,
        0xc3cb0f25,
        0x97a63952,
        0x8a0e4d29,
        0x3e4ba81b,
        0x8e1b30ca,
        0xbbb0ecfa,
        0xb2ede9bf,
        0xe3d3c8d9,
        0xcbb503f0,
        0x879f3efb,
        0x8e5cfaee,
        0x54e9d8d5
};

/* file: numeric.h    */
/* int len = 41;  NOT USED */
unsigned long numeric_data[] = {
    0xaa8a2aaa,
        0x55551545,
        0x11555555,
        0xaaa22aaa,
        0x2a8aaaaa,
        0x55415555,
        0xafaaaaaa,
        0xf5555555,
        0xaa2aaaaa,
        0x5555555f,
        0x50555555,
        0x51555545,
        0xaaaa0aaa,
        0x2aaaaaa8,
        0xaae2aaaa,
        0x5555053e,
        0x4b801b00,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x380b45a7,
        0xbeb804bb,
        0xc4689285,
        0xdfe2a81b,
        0xe66474a7,
        0xe66667d1,
        0xe66627d1,
        0xe66667d1,
        0xe66667d1,
        0x3e4b881b,
        0xe66667d1,
        0xe66667d1,
        0xe66667d1,
        0xe66667d1,
        0xe66667d1,
        0xe66667d1,
        0x54e8d9d5
};

int main()

{
    d32 *dptr;                     /* data pointer to SPI */
    dx  msg_length;                /* length of message processed */
//    dx  i;                         /* message array index */
    dx  j;                         /* msg process type index */
    int m = 0;

    #ifdef SHORT_BENCH
    for(m = 0; m < 4; m++){
    #else
    for(m = 0; m < 100; m++){
    #endif
        for (j = 0; j < 2; j++)
        {
            alpha_count=0;
            if (!j)                                   /* process numeric data set */
            {
                dptr = numeric_data;
            }
            else                                    /* process alphanumeric data set */
            {
                dptr = alpha_data;
            }

            if (sync_find(dptr) != NULL)               /* if sync acquisition success */
            {
                msg_length = msg_proc(sync_find(dptr));  /* return length of message */

                if (func == 0)                           /* if numeric message */
                {
                }
                else if (func == 3)                       /* alphanumeric message */
                {
                }
                else
                {
                    puts("pocsag: failed\n");
                    return -1;
                }
            }
            else {

                puts("pocsag: failed\n");
                return -1;
            }

        }
    }

    if( strncmp((char*)msg, "Dear fellow ACP benchmarker", 27) == 0 &&
        msg_length == 88 ) {
        puts("pocsag: success\n");
        return 0;
    }
    else {
        puts("pocsag: failed\n");
        return -1;
    }
    return 0;
}
