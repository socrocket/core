#ifndef _STIMULI_H_
#define _STIMULI_H_

#define VECTORS 15

enum t_itype {inop, fetch};
enum t_dtype {dnop, load, store};

typedef struct {

  t_itype itype;
  unsigned int iaddr;
  unsigned int iflush;
  unsigned int iflushl;
  unsigned int ifline;
  unsigned int iinstr;

  t_dtype dtype;
  unsigned int daddr;
  unsigned int dlength;
  unsigned int dasi;
  unsigned int dflush;
  unsigned int dflushl;
  unsigned int dlock;
  unsigned int dwdata;
  unsigned int dhrdata;

} t_stim;

#endif // __STIMULI_H_

