#ifndef VENDIAN_H
#define VENDIAN_H

inline void swap_Endianess( unsigned int &datum ) {
    unsigned char helperByte = 0;
    for(unsigned int i = 0; i < sizeof(unsigned int)/2; i++){
        helperByte = ((unsigned char *)&datum)[i];
        ((unsigned char *)&datum)[i] = ((unsigned char *)&datum)[sizeof(unsigned int) -1 -i];
        ((unsigned char *)&datum)[sizeof(unsigned int) -1 -i] = helperByte;
    }
}

inline void swap_Endianess( unsigned short int &datum ) {
    unsigned char helperByte = 0;
    for(unsigned int i = 0; i < sizeof(unsigned short int)/2; i++){
        helperByte = ((unsigned char *)&datum)[i];
        ((unsigned char *)&datum)[i] = ((unsigned char *)&datum)[sizeof(unsigned short int) \
            -1 -i];
        ((unsigned char *)&datum)[sizeof(unsigned short int) -1 -i] = helperByte;
    }
}

#endif
