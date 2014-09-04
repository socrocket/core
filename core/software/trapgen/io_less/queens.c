/*
**  Queens.c    --  Find solutions to the Eight-Queens chess problem.
**                  Roberto Sierra  7/25/93  Version 1.1
**                                  3/19/84  Version 1.0
**
**  Description:
**      This program finds all the possible ways that N queens can
**      be placed on an NxN chessboard so that the queens cannot
**      capture one another -- that is, so that no rank, file or
**      diagonal is occupied by more than one queen.  By default,
**      the program prints the first solution it finds.  You can
**      use the -a option to print all solutions, or the -c option
**      just to count them.  The program allows the chess board
**      to be from 1x1 (trivial case) to 100x100.  Warning: the
**      larger the chess board, the longer it typically takes to
**      find each solution, even though there may be more of them.
**
**      This is a terrific example of the utility of recursion. The
**      algorithm uses recursion to drastically limit the number
**      of board positions that are tested.  The program is able
**      to find all 8x8 queen solutions in a fraction of a second
**      (not counting print time).  The code makes no attempt to
**      eliminate symmetrical solutions, so the number of solutions
**      reported will always be higher than the actual number of
**      distinct solutions.
**
**
**  Usage:
**      Queens [-ac] n
**
**      n       number of queens (rows and columns).
**              An integer from 1 to 100.
**      -a      Find (and print) all solutions.
**      -c      Count all solutions, but do not print them.
**
**      The output is sent to stdout.  All errors messages are
**      sent to stderr.  If a problem arises, the return code is -1.
**
**
**  Examples:
**
**      Queens 8        ## Show an 8x8 solution
**      8 queens on a 8x8 board...
**       Q - - - - - - -
**       - - - - Q - - -
**       - - - - - - - Q
**       - - - - - Q - -
**       - - Q - - - - -
**       - - - - - - Q -
**       - Q - - - - - -
**       - - - Q - - - -
**
**      Queens -c 8     ## Count all 8x8 solutions
**      8 queens on a 8x8 board...
**      ...there are 92 solutions.
**
**      Queens -a 4     ## Show all 4x4 solutions
**      4 queens on a 4x4 board...
**
**      Solution #1:
**       - Q - -
**       - - - Q
**       Q - - -
**       - - Q -
**
**      Solution #2:
**       - - Q -
**       Q - - -
**       - - - Q
**       - Q - -
**
**      ...there are 2 solutions.
**
**
**  Build Instructions:
**      You'll need an ANSI C compiler (or the willingness to edit
**      the program a bit).  If you've got Gnu C, then you can
**      compile and load the program as follows:
**
**              gcc Queens.c -ansi -o Queens
**
**      [If you're using MPW on the Mac, define '-d MPW' on the
**      compile line so that background processing will occur.]
**
**
**  Algorithm:
**      In a 1984 Byte article, I ran across an interesting letter
**      from a high school student who was attempting to solve the
**      Eight Queens problem using a BASIC interpreter.  He had
**      developed a program which placed eight queens successively
**      on all sixty-four squares, testing for conflicts at each
**      iteration.  Of course, such a program would require 64^8
**      iterations (about 2.8x10^14 iterations).  Even in C on a,
**      fast CPU, this could take months or years.  Byte's answer was
**      to alter the loops so that the queens resided on separate
**      ranks, thereby reducing the number of iterations required
**      to find all solutions to 8^8 iterations (about 16 million).
**      More reasonable, but still requiring a chunk of CPU time.
**
**      I puzzled about this problem a bit, and came to realize that
**      this was still wasting a lot of CPU cycles.  Though I'm sure
**      others have come up with good algorithms, I decided to come
**      up with my own, with a particular eye on efficiency.  The
**      resulting algorithm finds all 8x8 solutions in a fraction
**      of a second (there are 92 solutions, including rotations).
**      On a Sun 4, it'll find all 365,596 solutions on a 14x14 board
**      in a bit over 2 minutes (printing them out requires extra
**      time, of course).  Even Byte's solution would require 14^14
**      iterations (about 10^16) which would take aeons.
**
**      My algorithm works as follows:
**      (1)  Place a queen in the top left corner.
**      (2)  Place another queen immediately below.
**      (3)  Test for conflicts.  If the second queen conflicts (it
**           does at first), then move it one square to the right.
**      (4)  Loop step 3 until there are no conflicts.  Place
**           the next queen on the board and recurse.
**      (5)  If any queen reaches the right edge of the board,
**           remove it and 'pop' to the previous recursion level.
**      (6)  Now repeat these steps recursively until all eight
**           queens (or however many) have been placed without
**           conflict -- the result is a solution to the problem,
**           which is counted and optionally printed.
**
**      Because conflicts are tested as the recursion proceeds,
**      this has the effect of 'pruning' the recursion so that
**      a large number of board positions are not even attempted.
**      The result is that the algorithm runs in reasonable time.
**
**      I used a few tricks to make the test-for-conflict code
**      extremely efficient -- there is no 'inner' loop to search
**      along ranks, files, or diagonals.  A series of arrays are
**      maintained instead which indicate which queen currently
**      'owns' each rank, file or diagonal.  This makes the
**      algorithm really fly, though the code is a little hard to
**      read.  Lastly, pointer arithmetic is used to reduce the
**      number of implicit multiplications used in array addressing.
**
**
**  Contact:
**      For queries regarding this program, contact Roberto Sierra
**      at any of the following addresses:
**
**              Roberto Sierra
**              bert@netcom.com   (preferred address)
**              73557.2101@compuserve.com
**
**              Tempered MicroDesigns
**              P.O. Box 170638
**              San Francisco, CA  94117
**
**
**  Fine Print:
**      This program is in the public domain and can be used for
**      any purpose whatsoever, including commercial application.
**      [I'd like to hear what you do with it, though.]
**      Absolutely no warranty or liability is implied or extended
**      by the author.
**
**
**  Modification History:
**      PRS  3/19/84  v1.0 -- Original version.
**      PRS  7/25/93  v1.1 -- ANSIfied the code.  More efficient pointers.
**
*/

#include <stdlib.h>

#ifdef SHORT_BENCH
#define MAXQUEENS   16                          /* Maximum number of queens */
#else
#define MAXQUEENS   20                          /* Maximum number of queens */
#endif
#define MAXRANKS    MAXQUEENS                   /* Maximum number of ranks (rows) */
#define MAXFILES    MAXQUEENS                   /* Maximum number of files (columns) */
#define MAXDIAGS    (MAXRANKS+MAXFILES-1)       /* Maximum number of diagonals */
#define EMPTY       (MAXQUEENS+1)               /* Marks unoccupied file or diagonal */

/* GLOBAL VARIABLES */

int             queens;                         /* Number of queens to place */
int             ranks;                          /* Number of ranks (rows) */
int             files;                          /* Number of files (columns) */
int             findall = 0;                    /* TRUE if finding all solutions */

unsigned long   solutions = 0;                  /* Number of solutions found */
int             queen[MAXRANKS];                /* File on which each queen is located */
int             file[MAXFILES];                 /* Which queen 'owns' each file */
int             fordiag[MAXDIAGS];              /* Which queen 'owns' forward diagonals */
int             bakdiag[MAXDIAGS];              /* Which queen 'owns' reverse diagonals */




/***********************/
/****   ROUTINES    ****/
/***********************/

/* Internal prototypes */
void    find(int level);                        /* Algorithm to find solutions */

/*---------------------- main() ---------------------------
**  MAIN program.  The main purpose of this routine is
**  to deal with decoding the command line arguments,
**  initializing the various arrays, and starting the
**  recursive search routine.
*/

int main(int argc,char **argv){
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

    register int    i;                          /* Loop variable */
    register char   *p;                         /* Pointer to argument */

    findall = 1;                /* Also forces findall option */
    queens = MAXQUEENS/2;

    ranks = files = queens;                     /* NxN board for N queens */

    /*  Initialization  */
    solutions = 0;                              /* No solutions yet */
    for (i=0; i<MAXFILES; ++i) file[i] = EMPTY;
    for (i=0; i<MAXDIAGS; ++i) fordiag[i] = bakdiag[i] = EMPTY;

    /* Find all solutions (begin recursion) */
    find(0);

    return 0;
}                                               /* End of main() */



/*-------------------------- find() ----------------------------
**  FIND is the recursive heart of the program, and finds all
**  solutions given a set of level-1 fixed queen positions.
**  The routine moves a single queen through all files (columns)
**  at the current rank (recursion level).  As the queen is moved,
**  conflict tests are made.  If the queen can be placed without
**  conflict, then the routine recurses to the next level.  When
**  all queens have been placed without conflict, a solution is
**  counted and reported.
*/

void find(register int level){
    register int    f;                          /* Indexes through files */
    register int    *fp,*fdp,*bdp;              /* Ptrs to file/diagonal entries */

    if (level == queens) {                      /* Placed all queens?  Stop. */
    ++solutions;                            /* Congrats, this is a solution! */

    if (!findall) exit(0);                  /* May stop after first solution */
    } else {                                    /* Not at final level yet */

    for (                                   /* MOVE QUEEN THROUGH ALL FILES */
        f = 0,                              /* Queen starts at left (file 0) */
        fp = file,                          /* Ptr to base of file array */
        fdp = &fordiag[level],              /* Ptr to first fwd diag entry */
        bdp = &bakdiag[level+files-1]       /* Ptr to first bak diag entry */
    ;
        f < files                           /* Loop through all files */
    ;
        ++f,                                /* Advance index */
        ++fp, ++fdp, --bdp                  /* Advance pointers */
    ) {
        if (*fp >= level &&                 /* No queen on the file? */
        *fdp >= level && *bdp >= level  /* No queens on diagonals? */
        ) {
        queen[level] = f;               /* Note new position of queen */
        *fp = *fdp = *bdp = level;      /* Place queen on file & diags */
        find(level+1);                  /* This level OK, recurse to next */
        *fp = *fdp = *bdp = EMPTY;      /* Remove queen from file & diags */
        }                                   /* End of conflict test */
    }                                       /* End of file loop */
    }                                           /* End if (level == queens) */
}                                               /* End of find() */
