#ifndef _AES_CRYPT_H
#define _AES_CRYPT_H

#include "aes1.h"

#define AES_KEY_BITS 128
#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE (AES_KEY_BITS / 8)
#endif

#if 0
int  AES_SetKey(AES_Context *pContext, unsigned char abKey[AES_BLOCK_SIZE]);
void AES_Encrypt(AES_Context *pContext, unsigned char* pbInData, int nInLen, unsigned char** ppbOutData, int* pnOutLen);
void AES_Decrypt(AES_Context *pContext, unsigned char* pbInData, int nInLen, unsigned char** ppbOutData, int* pnOutLen);
#else

void AES_Encrypt(unsigned char* pbKey, unsigned char* pbInData, int nInLen, unsigned char** ppbOutData, int* pnOutLen);
void AES_Decrypt(unsigned char* pbKey, unsigned char* pbInData, int nInLen, unsigned char** ppbOutData, int* pnOutLen);

#endif
#endif /* aes.h */
