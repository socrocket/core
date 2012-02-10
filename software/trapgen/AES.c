
typedef unsigned char     WORD8; /*Definisce la parola in Nero come
sinonimo del tipo di dato che lo precede pag. 187 (aggiunge un
nuovo nome ad un tipo di variabile già esistente */
typedef unsigned int      WORD32; /*Vedi pag. 42 per "unsigned";
Gli unsigned sono numeri sempre positivi in aritmetica modulo2;
Le operazioni su oggetti privi di segno non possono mai andare in overflow */


/*Queste 4 operazioni estraggono ognuna un byte solo dalla parola a 32 bit
ricevuta in ingresso*/
#define byte3(x) (x & 0xff) /*Primo byte di dx*/
#define byte2(x) ((x >> 8) & 0xff)    /*Secondo byte da dx*/
#define byte1(x) ((x >> 16) & 0xff)   /*Secondo byte da sx*/
#define byte0(x) (x >> 24)    /*il byte più significativo*/

/*Passandogli 4 byte crea una parola da 32 bit i byte vengono inseriti da sx a dx in ordine di
parametro*/
#define WORD8_TO_WORD32(b0, b1, b2, b3) ((WORD32)(b0) << 24 | (WORD32)(b1) << 16 | (WORD32)(b2) << 8 | (b3))

/*La moltiplicazione in Versione 1 viene eseguita via software e Xtime esegue la moltiplicazione
per 2 su ogni byte in paralelo*/
#define Xtime(x)  ((((x) & 0x7f7f7f7f) << 1) ^ ((((x) & 0x80808080) >> 7) * 0x0000001b))

static const WORD8 Sbox[256] = /*La variabile Sbox è matrice precisa tabulata negli appunti;
I suoi elementi sono char quindi 8 bit (1 byte) (infatti WORD8 è typedef in aes.h);*/
{
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

/*La matrice di S-box inversa*/
static const WORD8 InvSbox[256] =
{
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,
    0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,
    0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,
    0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,
    0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,
    0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,
    0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,
    0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,
    0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,
    0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,
    0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,
    0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,
    0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,
    0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,
    0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,
    0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};


/*Questa matrice è necessaria nel keyschedule; è fatta da 10 elementi che sono
ognuno una parola da 32 bit: dove solo il primo byte è diverso da zero e vale x elevato
alla i-1 (i parte da 1)*/
static const WORD32 rcon[10] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000/*x alla 7*/,
    0x1b000000/*x alla 8 calcolato cap.2.3 rijndal*/, 0x36000000
/*n.b. Si scrive xalla8 e si xor con il polinomio irriducibile*/
};

/*Le procedure di creazione delle chiavi, encryption*/
void keyschedule(WORD32* key, WORD32* exp);
void encryptblock(WORD32* input_pointer, WORD32* output_pointer, WORD32* expkey);
void decryptblock(WORD32* input_pointer, WORD32* output_pointer, WORD32* expkey);



int main()
{
    WORD32 m_expanded_key[44];
    int i=0,n_blk;
    WORD32 key[4]=
    {0x2b7e1516,0x28aed2a6,0xabf71588,0x09cf4f3c};
    WORD32 block[16]=
    {0x3243f6a8,0x885a308d,0x313198a2,0xe0370734};
    WORD32 outencry[4]; /*Uscita crittografia*/
    WORD32 outdecry[4]; /*UScita decrittografia*/
    #ifdef SHORT_BENCH
    for(i = 0; i < 20; i++){
    #else
    for(i = 0; i < 1000; i++){
    #endif
        for (n_blk=0;n_blk<10;n_blk++)
        {
            keyschedule(key, m_expanded_key); /*Lancia Init che crea le chiavi
                di sessione e controlla di avere tutti
            i dati e restituisce SUCCESS.*/
            encryptblock(block, outencry, m_expanded_key);
            decryptblock(outencry, outdecry, m_expanded_key);
            block[0]= block[0]+block[1]-block[2]+block[3]-block[2];
            block[1]= block[2]+block[3]+block[3]-block[1]-block[0];
            block[2]= block[2]-block[1]+block[3]+block[0]+block[2];
            block[3]= block[0]-block[3]+block[1]+block[0]-block[2];
        }
    }

    return 0;
}

/*implementazione della proc.di creazioni chiavi di sessione con chiave
a 128 bit*/
void keyschedule(WORD32* key, WORD32* exp)
{
    register WORD32* local_pointer = exp;
    /*register WORD32 copy[4];*/
    register WORD32 copy0;
    register WORD32 copy1;
    register WORD32 copy2;
    register WORD32 copy3;
    register int i = 0;
    copy0 = ( key[0] & 0xff000000)        |/*le celle Key[0] sono di 32 bit con & estraggo
                                             solo il byte di interesse*/
              ((key[1] & 0xff000000) >> 8)  |
              ((key[2] & 0xff000000) >> 16) |
              ((key[3] & 0xff000000) >> 24);
    copy1 = ((key[0] & 0x00ff0000) << 8)  |
              ( key[1] & 0x00ff0000)        |
              ((key[2] & 0x00ff0000) >> 8)  |
              ((key[3] & 0x00ff0000) >> 16);
    copy2 = ((key[0] & 0x0000ff00) << 16) |
              ((key[1] & 0x0000ff00) << 8)  |
              ( key[2] & 0x0000ff00)        |
              ((key[3] & 0x0000ff00) >> 8);
    copy3 = ((key[0] & 0x000000ff) << 24) |
              ((key[1] & 0x000000ff) << 16) |
              ((key[2] & 0x000000ff) << 8)  |
              ( key[3] & 0x000000ff);/*Fine trasposizione matrice chiave*/

        local_pointer[0] = copy0;
        local_pointer[1] = copy1;
        local_pointer[2] = copy2;
        local_pointer[3] = copy3;/*Salvataggio prima chiave (l'originale)*/
        for (; i < 10; i++)
        {
            /*Lavora su copy*/
            copy0 ^= ((WORD32)(Sbox[byte3(copy1)]) << 24) ^ rcon[i];
            copy1 ^=  (WORD32)(Sbox[byte3(copy2)]) << 24;
            copy2 ^=  (WORD32)(Sbox[byte3(copy3)]) << 24;
            copy3 ^=  (WORD32)(Sbox[byte3(copy0)]) << 24;
            copy0 ^= (copy0 >> 8) ^ (copy0 >> 16) ^ (copy0 >> 24);
            copy1 ^= (copy1 >> 8) ^ (copy1 >> 16) ^ (copy1 >> 24);
            copy2 ^= (copy2 >> 8) ^ (copy2 >> 16) ^ (copy2 >> 24);
            copy3 ^= (copy3 >> 8) ^ (copy3 >> 16) ^ (copy3 >> 24);
            local_pointer += 4;/*Incrementa il puntatore così usa sempre gli stessi indici
                               ma si sposta lungo l'array*/
            local_pointer[0] = copy0;
            local_pointer[1] = copy1;
            local_pointer[2] = copy2;
            local_pointer[3] = copy3;/*Salvataggio chiave successiva appena calcolata*/
        }

    /*copy[4] = ( key[4] & 0xff000000)        |
              ((key[5] & 0xff000000) >> 8);
    copy[5] = ((key[4] & 0x00ff0000) << 8)  |
              ( key[5] & 0x00ff0000);
    copy[6] = ((key[4] & 0x0000ff00) << 16) |
              ((key[5] & 0x0000ff00) << 8);
    copy[7] = ((key[4] & 0x000000ff) << 24) |
              ((key[5] & 0x000000ff) << 16);
    return 0;*/
}


/*Procedura di criptaggio di un blocco di 128 bit */
void encryptblock(WORD32* input_pointer, WORD32* output_pointer, WORD32* expkey)
{
    register WORD32 t0;
    register WORD32 t1;
    register WORD32 t2;
    register WORD32 t3;
    register WORD32 s0;
    register WORD32 s1;
    register WORD32 s2;
    register WORD32 s3;
    register int r = 10 >> 1;
    register WORD32* local_pointer = expkey;
    s0 =(( input_pointer[0] & 0xff000000)        |
           ((input_pointer[1] & 0xff000000) >> 8)  |
           ((input_pointer[2] & 0xff000000) >> 16) |
           ((input_pointer[3] & 0xff000000) >> 24))  ^ local_pointer[0];
    s1 =(((input_pointer[0] & 0x00ff0000) << 8)  |
           ( input_pointer[1] & 0x00ff0000)        |
           ((input_pointer[2] & 0x00ff0000) >> 8)  |
           ((input_pointer[3] & 0x00ff0000) >> 16))  ^ local_pointer[1];
    s2 =(((input_pointer[0] & 0x0000ff00) << 16) |
           ((input_pointer[1] & 0x0000ff00) << 8)  |
           ( input_pointer[2] & 0x0000ff00)        |
           ((input_pointer[3] & 0x0000ff00) >> 8))   ^ local_pointer[2];
    s3 =(((input_pointer[0] & 0x000000ff) << 24) |
           ((input_pointer[1] & 0x000000ff) << 16) |
           ((input_pointer[2] & 0x000000ff) << 8)  |
           ( input_pointer[3] & 0x000000ff))         ^ local_pointer[3];
    /* fine del passaggio TRASPOSTA + ADD KEY*/
    for (;;)
    {
        s0 = WORD8_TO_WORD32( Sbox[byte0(s0)],
                                Sbox[byte1(s0)],
                                Sbox[byte2(s0)],
                                Sbox[byte3(s0)]);
        s1 = WORD8_TO_WORD32( Sbox[byte1(s1)],
                                Sbox[byte2(s1)],
                                Sbox[byte3(s1)],
                                Sbox[byte0(s1)]);
        s2 = WORD8_TO_WORD32( Sbox[byte2(s2)],
                                Sbox[byte3(s2)],
                                Sbox[byte0(s2)],
                                Sbox[byte1(s2)]);
        s3 = WORD8_TO_WORD32( Sbox[byte3(s3)],
                                Sbox[byte0(s3)],
                                Sbox[byte1(s3)],
                                Sbox[byte2(s3)]);/* Fine del Passaggio SBOX + Shift ROW*/
        t0 = s1 ^ s2 ^ s3;
        t1 = s0 ^ s2 ^ s3;
        t2 = s0 ^ s1 ^ s3;
        t3 = s0 ^ s1 ^ s2;
        s0 = Xtime(s0);
        s1 = Xtime(s1);
        s2 = Xtime(s2);
        s3 = Xtime(s3);
        t0 ^= s0 ^ s1 ^ local_pointer[4];
        t1 ^= s1 ^ s2 ^ local_pointer[5];
        t2 ^= s2 ^ s3 ^ local_pointer[6];
        t3 ^= s0 ^ s3 ^ local_pointer[7];/*Fine di Mix column*/
        local_pointer += 8; /*incremento puntatore alla stringa delle chiavi*/
        if (--r == 0) /*Decremento contatore dei round*/
        {
            break;/*Se finisce i round esce*/
        }
        t0 = WORD8_TO_WORD32( Sbox[byte0(t0)],/*Altro SBOX del secondo round
                                                    con variaile ausiliaria 't'*/
                                Sbox[byte1(t0)],
                                Sbox[byte2(t0)],
                                Sbox[byte3(t0)]);
        t1 = WORD8_TO_WORD32( Sbox[byte1(t1)],
                                Sbox[byte2(t1)],
                                Sbox[byte3(t1)],
                                Sbox[byte0(t1)]);
        t2 = WORD8_TO_WORD32( Sbox[byte2(t2)],
                                Sbox[byte3(t2)],
                                Sbox[byte0(t2)],
                                Sbox[byte1(t2)]);
        t3 = WORD8_TO_WORD32( Sbox[byte3(t3)],
                                Sbox[byte0(t3)],
                                Sbox[byte1(t3)],
                                Sbox[byte2(t3)]);
        s0 = t1 ^ t2 ^ t3;
        s1 = t0 ^ t2 ^ t3;
        s2 = t0 ^ t1 ^ t3;
        s3 = t0 ^ t1 ^ t2;
        t0 = Xtime(t0);
        t1 = Xtime(t1);
        t2 = Xtime(t2);
        t3 = Xtime(t3);
        s0 ^= t0 ^ t1 ^ local_pointer[0];
        s1 ^= t1 ^ t2 ^ local_pointer[1];
        s2 ^= t2 ^ t3 ^ local_pointer[2];
        s3 ^= t0 ^ t3 ^ local_pointer[3];
    }/* Fine del ciclo di for(;;)*/

    t0 = WORD8_TO_WORD32( Sbox[byte0(t0)],
                            Sbox[byte1(t0)],
                            Sbox[byte2(t0)],
                            Sbox[byte3(t0)]) ^ local_pointer[0];
    t1 = WORD8_TO_WORD32( Sbox[byte1(t1)],
                            Sbox[byte2(t1)],
                            Sbox[byte3(t1)],
                            Sbox[byte0(t1)]) ^ local_pointer[1];
    t2 = WORD8_TO_WORD32( Sbox[byte2(t2)],
                            Sbox[byte3(t2)],
                            Sbox[byte0(t2)],
                            Sbox[byte1(t2)]) ^ local_pointer[2];
    t3 = WORD8_TO_WORD32( Sbox[byte3(t3)],
                            Sbox[byte0(t3)],
                            Sbox[byte1(t3)],
                            Sbox[byte2(t3)]) ^ local_pointer[3];
    /*Fine ultimo SBOX e ADD KEY*/
    output_pointer[0] = ( t0 & 0xff000000)        |
                        ((t1 & 0xff000000) >> 8)  |
                        ((t2 & 0xff000000) >> 16) |
                        ((t3 & 0xff000000) >> 24);
    output_pointer[1] = ((t0 & 0x00ff0000) << 8)  |
                        ( t1 & 0x00ff0000)        |
                        ((t2 & 0x00ff0000) >> 8)  |
                        ((t3 & 0x00ff0000) >> 16);
    output_pointer[2] = ((t0 & 0x0000ff00) << 16) |
                        ((t1 & 0x0000ff00) << 8)  |
                        ( t2 & 0x0000ff00)        |
                        ((t3 & 0x0000ff00) >> 8);
    output_pointer[3] = ((t0 & 0x000000ff) << 24) |
                        ((t1 & 0x000000ff) << 16) |
                        ((t2 & 0x000000ff) << 8)  |
                        ( t3 & 0x000000ff);
}/*TRasposizione anche del risultato finale*/

void decryptblock(WORD32* input_pointer, WORD32* output_pointer, WORD32* expkey)
{
    register WORD32 t0;
    register WORD32 t1;
    register WORD32 t2;
    register WORD32 t3;
    register WORD32 s0;
    register WORD32 s1;
    register WORD32 s2;
    register WORD32 s3;
    register int r =  10 >> 1;
    register WORD32* local_pointer = expkey + (10) * 4;
    s0 =(( input_pointer[0] & 0xff000000)        |
           ((input_pointer[1] & 0xff000000) >> 8)  |
           ((input_pointer[2] & 0xff000000) >> 16) |
           ((input_pointer[3] & 0xff000000) >> 24))  ^ local_pointer[0];
    s1 =(((input_pointer[0] & 0x00ff0000) << 8)  |
           ( input_pointer[1] & 0x00ff0000)        |
           ((input_pointer[2] & 0x00ff0000) >> 8)  |
           ((input_pointer[3] & 0x00ff0000) >> 16))  ^ local_pointer[1];
    s2 =(((input_pointer[0] & 0x0000ff00) << 16) |
           ((input_pointer[1] & 0x0000ff00) << 8)  |
           ( input_pointer[2] & 0x0000ff00)        |
           ((input_pointer[3] & 0x0000ff00) >> 8))   ^ local_pointer[2];
    s3 =(((input_pointer[0] & 0x000000ff) << 24) |
           ((input_pointer[1] & 0x000000ff) << 16) |
           ((input_pointer[2] & 0x000000ff) << 8)  |
           ( input_pointer[3] & 0x000000ff))         ^ local_pointer[3];
    for (;;)
    {
        local_pointer -= 8;
        s0 = WORD8_TO_WORD32( InvSbox[byte0(s0)],
                                InvSbox[byte1(s0)],
                                InvSbox[byte2(s0)],
                                InvSbox[byte3(s0)]) ^ local_pointer[4];
        s1 = WORD8_TO_WORD32( InvSbox[byte3(s1)],
                                InvSbox[byte0(s1)],
                                InvSbox[byte1(s1)],
                                InvSbox[byte2(s1)]) ^ local_pointer[5];
        s2 = WORD8_TO_WORD32( InvSbox[byte2(s2)],
                                InvSbox[byte3(s2)],
                                InvSbox[byte0(s2)],
                                InvSbox[byte1(s2)]) ^ local_pointer[6];
        s3 = WORD8_TO_WORD32( InvSbox[byte1(s3)],
                                InvSbox[byte2(s3)],
                                InvSbox[byte3(s3)],
                                InvSbox[byte0(s3)]) ^ local_pointer[7];
        t0 = s1 ^ s2 ^ s3;
        t1 = s0 ^ s2 ^ s3;
        t2 = s0 ^ s1 ^ s3;
        t3 = s0 ^ s1 ^ s2;
        s0 = Xtime(s0);
        s1 = Xtime(s1);
        s2 = Xtime(s2);
        s3 = Xtime(s3);
        t0 ^= s0 ^ s1;
        t1 ^= s1 ^ s2;
        t2 ^= s2 ^ s3;
        t3 ^= s0 ^ s3;
        s0 = Xtime(s0 ^ s2);
        s1 = Xtime(s1 ^ s3);
        t0 ^= s0;
        t1 ^= s1;
        t2 ^= s0;
        t3 ^= s1;
        s0 = Xtime(s0 ^ s1);
        t0 ^= s0;
        t1 ^= s0;
        t2 ^= s0;
        t3 ^= s0;
        if (--r == 0)
        {
            break;
        }
        t0 = WORD8_TO_WORD32( InvSbox[byte0(t0)],
                                InvSbox[byte1(t0)],
                                InvSbox[byte2(t0)],
                                InvSbox[byte3(t0)]) ^ local_pointer[0];
        t1 = WORD8_TO_WORD32( InvSbox[byte3(t1)],
                                InvSbox[byte0(t1)],
                                InvSbox[byte1(t1)],
                                InvSbox[byte2(t1)]) ^ local_pointer[1];
        t2 = WORD8_TO_WORD32( InvSbox[byte2(t2)],
                                InvSbox[byte3(t2)],
                                InvSbox[byte0(t2)],
                                InvSbox[byte1(t2)]) ^ local_pointer[2];
        t3 = WORD8_TO_WORD32( InvSbox[byte1(t3)],
                                InvSbox[byte2(t3)],
                                InvSbox[byte3(t3)],
                                InvSbox[byte0(t3)]) ^ local_pointer[3];
        s0 = t1 ^ t2 ^ t3;
        s1 = t0 ^ t2 ^ t3;
        s2 = t0 ^ t1 ^ t3;
        s3 = t0 ^ t1 ^ t2;
        t0 = Xtime(t0);
        t1 = Xtime(t1);
        t2 = Xtime(t2);
        t3 = Xtime(t3);
        s0 ^= t0 ^ t1;
        s1 ^= t1 ^ t2;
        s2 ^= t2 ^ t3;
        s3 ^= t0 ^ t3;
        t0 = Xtime(t0 ^ t2);
        t1 = Xtime(t1 ^ t3);
        s0 ^= t0;
        s1 ^= t1;
        s2 ^= t0;
        s3 ^= t1;
        t0 = Xtime(t0 ^ t1);
        s0 ^= t0;
        s1 ^= t0;
        s2 ^= t0;
        s3 ^= t0;
    }
    t0 = WORD8_TO_WORD32( InvSbox[byte0(t0)],
                            InvSbox[byte1(t0)],
                            InvSbox[byte2(t0)],
                            InvSbox[byte3(t0)]) ^ local_pointer[0];
    t1 = WORD8_TO_WORD32( InvSbox[byte3(t1)],
                            InvSbox[byte0(t1)],
                            InvSbox[byte1(t1)],
                            InvSbox[byte2(t1)]) ^ local_pointer[1];
    t2 = WORD8_TO_WORD32( InvSbox[byte2(t2)],
                            InvSbox[byte3(t2)],
                            InvSbox[byte0(t2)],
                            InvSbox[byte1(t2)]) ^ local_pointer[2];
    t3 = WORD8_TO_WORD32( InvSbox[byte1(t3)],
                            InvSbox[byte2(t3)],
                            InvSbox[byte3(t3)],
                            InvSbox[byte0(t3)]) ^ local_pointer[3];
    output_pointer[0] = ( t0 & 0xff000000)        |
                        ((t1 & 0xff000000) >> 8)  |
                        ((t2 & 0xff000000) >> 16) |
                        ((t3 & 0xff000000) >> 24);
    output_pointer[1] = ((t0 & 0x00ff0000) << 8)  |
                        ( t1 & 0x00ff0000)        |
                        ((t2 & 0x00ff0000) >> 8)  |
                        ((t3 & 0x00ff0000) >> 16);
    output_pointer[2] = ((t0 & 0x0000ff00) << 16) |
                        ((t1 & 0x0000ff00) << 8)  |
                        ( t2 & 0x0000ff00)        |
                        ((t3 & 0x0000ff00) >> 8);
    output_pointer[3] = ((t0 & 0x000000ff) << 24) |
                        ((t1 & 0x000000ff) << 16) |
                        ((t2 & 0x000000ff) << 8)  |
                        ( t3 & 0x000000ff);
}

