#include "desinterface.h"
#include "des.h"

#include <memory.h>
#include <rand.h>
#include <time.h>

#define MAX_CRYPTO_BLOCK_SIZE 16


int DesInterface_desSetKey(const_DES_cblock *key, DES_key_schedule *schedule)
{
    return DES_set_key(key, schedule);
}

void DesInterface_desEcbEncrypt(const_DES_cblock *input,DES_cblock *output, DES_key_schedule *ks,int enc)
{
    DES_ecb_encrypt(input, output, ks, enc);
}

int DesInterface_randBytes(unsigned char *buf,int num)
{
    int i;
    srand(time(NULL));

    for(i = 0; i < num; i ++)
        buf[i] = (unsigned char)rand();

    return 0;
}


void DesInterface_xor1(unsigned char *ivect, unsigned char *data, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        data[i] ^= ivect[i];
    }
}

void DesInterface_rol(unsigned char *data, const int len)
{
    int i;
    unsigned char first = data[0];
    for (i = 0; i < len-1; i++)
        data[i] = data[i+1];
    data[len-1] = first;
}

void DesInterface_lsl(unsigned char *data, int len)
{
    int n;
    for (n = 0; n < len - 1; n++) {
        data[n] = (data[n] << 1) | (data[n+1] >> 7);
    }
    data[len - 1] <<= 1;
}


void DesInterface_updateKeySchedules(MifareDESFireKey* desfireKey)
{
    DesInterface_desSetKey((DES_cblock *)desfireKey->data, &(desfireKey->ks1));
    DesInterface_desSetKey((DES_cblock *)(desfireKey->data + 8), &(desfireKey->ks2));
}

MifareDESFireKey DesInterface_mifareDesfireDesKeyNewWithVersion(unsigned char value[8])
{
    MifareDESFireKey key;
    key.type = T_DES;

    memcpy (key.data, value, 8);
    memcpy (key.data+8, value, 8);
    DesInterface_updateKeySchedules (&key);

    return key;
}

MifareDESFireKey DesInterface_mifareDesfire3DesKeyNewWithVersion(unsigned char value[16])
{
    MifareDESFireKey key;
    key.type = (int)T_3DES;

    memcpy (key.data, value, 16);
    DesInterface_updateKeySchedules (&key);

    return key;
}

void DesInterface_mifareCypherBlocksChained(MifareDESFireKey* key, unsigned char* ivect, unsigned char *data, int data_size,
                                             MifareCryptoDirection direction, MifareCryptoOperation operation)
{
    int block_size = 8;
    int offset = 0;
    while (offset < data_size) {
        DesInterface_mifareCypherSingleBlock (key, data + offset, ivect, block_size, direction, operation);
        offset += block_size;
    }
}

void DesInterface_mifareCypherSingleBlock (MifareDESFireKey* key, unsigned char *data, unsigned char *ivect, int block_size,
                                            MifareCryptoDirection direction, MifareCryptoOperation operation)
{
    unsigned char ovect[MAX_CRYPTO_BLOCK_SIZE];
	unsigned char edata[MAX_CRYPTO_BLOCK_SIZE];

    if (direction == MCD_SEND) {
        DesInterface_xor1 (ivect, data, block_size);
    } else {
        memcpy (ovect, data, block_size);
    }

    switch (key->type)
    {
    case T_DES:
    {
        switch (operation)
        {
        case MCO_ENCYPHER:
            DesInterface_desEcbEncrypt((DES_cblock *) data, (DES_cblock *) edata, &(key->ks1), DES_ENCRYPT);
            break;
        case MCO_DECYPHER:
            DesInterface_desEcbEncrypt ((DES_cblock *) data, (DES_cblock *) edata, &(key->ks1), DES_DECRYPT);
            break;
        }
        break;
    }
    case T_3DES:
    {
        switch (operation) {
        case MCO_ENCYPHER:
            DesInterface_desEcbEncrypt ((DES_cblock *) data,  (DES_cblock *) edata, &(key->ks1), DES_ENCRYPT);
            DesInterface_desEcbEncrypt ((DES_cblock *) edata, (DES_cblock *) data,  &(key->ks2), DES_DECRYPT);
            DesInterface_desEcbEncrypt ((DES_cblock *) data,  (DES_cblock *) edata, &(key->ks1), DES_ENCRYPT);
            break;
        case MCO_DECYPHER:
            DesInterface_desEcbEncrypt ((DES_cblock *) data,  (DES_cblock *) edata, &(key->ks1), DES_DECRYPT);
            DesInterface_desEcbEncrypt ((DES_cblock *) edata, (DES_cblock *) data,  &(key->ks2), DES_ENCRYPT);
            DesInterface_desEcbEncrypt ((DES_cblock *) data,  (DES_cblock *) edata, &(key->ks1), DES_DECRYPT);
            break;
        }
        break;
    }
    }

    memcpy (data, edata, block_size);

    if (direction == MCD_SEND) {
        memcpy (ivect, data, block_size);
    } else {
        DesInterface_xor1(ivect, data, block_size);
        memcpy (ivect, ovect, block_size);
    }

}

MifareDESFireKey DesInterface_mifareDesfireSessionKeyNew (unsigned char rnda[8], unsigned char rndb[8], MifareDESFireKey* authentication_key)
{
    MifareDESFireKey key;

    unsigned char buffer[24];

    switch (authentication_key->type) {
    case T_DES:
    {
        memcpy (buffer, rnda, 4);
        memcpy (buffer+4, rndb, 4);
        key = DesInterface_mifareDesfireDesKeyNewWithVersion (buffer);
    }
    break;
    case T_3DES:
    {
        memcpy (buffer, rnda, 4);
        memcpy (buffer+4, rndb, 4);
        memcpy (buffer+8, rnda+4, 4);
        memcpy (buffer+12, rndb+4, 4);
        key = DesInterface_mifareDesfire3DesKeyNewWithVersion (buffer);
    }
    break;
    }

    return key;
}

void DesInterface_iso14443a_crc(unsigned char *pbtData, int szLen, unsigned char *pbtCrc)
{
  unsigned char  bt;
  unsigned int wCrc = 0x6363;

  do {
    bt = *pbtData++;
    bt = (bt ^ (unsigned char)(wCrc & 0x00FF));
    bt = (bt ^ (bt << 4));
    wCrc = (wCrc >> 8) ^ ((unsigned int) bt << 8) ^ ((unsigned int) bt << 3) ^ ((unsigned int) bt >> 4);
  } while (--szLen);

  *pbtCrc++ = (unsigned char)(wCrc & 0xFF);
  *pbtCrc = (unsigned char)((wCrc >> 8) & 0xFF);
}

void DesEncrypt(unsigned char key[8], unsigned char* data, int data_size, MifareCryptoOperation operation)
{
    MifareDESFireKey desKey = DesInterface_mifareDesfireDesKeyNewWithVersion(key);
    unsigned char ivect[8] = { 0 };

    if(operation == MCO_ENCYPHER)
        DesInterface_mifareCypherBlocksChained(&desKey, ivect, (unsigned char*)data, data_size, MCD_SEND, MCO_ENCYPHER);
    else
        DesInterface_mifareCypherBlocksChained(&desKey, ivect, (unsigned char*)data, data_size, MCD_RECEIVE, MCO_DECYPHER);
}

void Des3Encrypt(unsigned char key[16], unsigned char* data, int data_size, MifareCryptoOperation operation)
{
    MifareDESFireKey desKey = DesInterface_mifareDesfire3DesKeyNewWithVersion(key);
    unsigned char ivect[8] = { 0 };

    if(operation == MCO_ENCYPHER)
        DesInterface_mifareCypherBlocksChained(&desKey, ivect, (unsigned char*)data, data_size, MCD_SEND, MCO_ENCYPHER);
    else
        DesInterface_mifareCypherBlocksChained(&desKey, ivect, (unsigned char*)data, data_size, MCD_RECEIVE, MCO_DECYPHER);
}
