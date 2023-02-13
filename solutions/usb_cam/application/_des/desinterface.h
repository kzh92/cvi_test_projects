#ifndef MIFAREDESFIRE_H
#define MIFAREDESFIRE_H

#include "des.h"
#include "rand.h"

#ifdef  __cplusplus
extern "C" {
#endif

enum
{
    T_DES,
    T_3DES
};

typedef struct _tagMifareDESFireKey {
    unsigned char data[24];
    int type;
    DES_key_schedule ks1;
    DES_key_schedule ks2;
    unsigned char cmac_sk1[24];
    unsigned char cmac_sk2[24];
    unsigned char aes_version;
}MifareDESFireKey;


typedef enum {
    MCD_SEND,
    MCD_RECEIVE
} MifareCryptoDirection;

typedef enum {
    MCO_ENCYPHER,
    MCO_DECYPHER
} MifareCryptoOperation;

void xor1(unsigned char *ivect, unsigned char *data, int len);
void rol (unsigned char *data, const int len);
void lsl (unsigned char *data, int len);

int DesInterface_desSetKey(const_DES_cblock *key, DES_key_schedule *schedule);
void DesInterface_desEcbEncrypt(const_DES_cblock *input,DES_cblock *output, DES_key_schedule *ks,int enc);
int  DesInterface_randBytes(unsigned char *buf,int num);
void DesInterface_xor1(unsigned char *ivect, unsigned char *data, int len);
void DesInterface_rol(unsigned char *data, const int len);
void DesInterface_lsl(unsigned char *data, int len);

void DesInterface_iso14443a_crc(unsigned char *pbtData, int szLen, unsigned char *pbtCrc);

void DesInterface_updateKeySchedules(MifareDESFireKey* desfireKey);
MifareDESFireKey DesInterface_mifareDesfireSessionKeyNew (unsigned char rnda[8], unsigned char rndb[8], MifareDESFireKey* authentication_key);
MifareDESFireKey DesInterface_mifareDesfireDesKeyNewWithVersion(unsigned char value[8]);
MifareDESFireKey DesInterface_mifareDesfire3DesKeyNewWithVersion(unsigned char value[16]);
void DesInterface_mifareCypherBlocksChained(MifareDESFireKey* key, unsigned char* ivect, unsigned char *data, int data_size, MifareCryptoDirection direction, MifareCryptoOperation operation);
void DesInterface_mifareCypherSingleBlock (MifareDESFireKey* key, unsigned char *data, unsigned char *ivect, int block_size, MifareCryptoDirection direction, MifareCryptoOperation operation);

//example:
//unsigned char des3Key[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
//DesEncrypt(des3Key, data, sizeof(data), MCO_ENCYPHER);
//DesEncrypt(des3Key, data, sizeof(data), MCO_DECYPHER);

void DesEncrypt(unsigned char key[8], unsigned char* data, int data_size, MifareCryptoOperation operation);
void Des3Encrypt(unsigned char key[16], unsigned char* data, int data_size, MifareCryptoOperation operation);

#ifdef  __cplusplus
}
#endif


#endif // MIFAREDESFIRE_H
