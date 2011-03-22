#ifndef MSB_LSB_H
#define MSB_LSB_H
template<uint32_t w, class T>
T msb_lsb(T a) {
  int h = ((w&1)?(w-1):w)/2;
  T r = (w&1)?(a & (1 << h+1)):0;
  for(int i=0;i<=h;++i) {
    bool b1 = a & (1<<i), b2 = a & (1<<(w-i));
    r |= (b2 << i) | (b1 << (w-i));
  }
  return r;
}

#endif
