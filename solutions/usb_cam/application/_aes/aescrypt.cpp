/*
 *  FIPS-197 compliant AES implementation
 *
 *  Copyright (C) 2001-2004  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "aescrypt.h"
#include <malloc.h>
#include <string.h>

#if 0
int  AES_SetKey(AES_Context *pContext, unsigned char abKey[AES_BLOCK_SIZE])
{
    return aes_set_key(pContext, abKey, AES_KEY_BITS);
}

void AES_Encrypt(AES_Context *pContext, unsigned char* pbInData, int nInLen, unsigned char** ppbOutData, int* pnOutLen)
{
    int nOutLen = (nInLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    unsigned char* pbOutData = (unsigned char*)malloc(nOutLen);
    for(int i = 0; i < (nInLen / AES_BLOCK_SIZE); i ++)
        aes_encrypt(pContext, pbInData + i * AES_BLOCK_SIZE, pbOutData + i * AES_BLOCK_SIZE);

    if(nInLen % AES_BLOCK_SIZE)
    {
        unsigned char bTmp[AES_BLOCK_SIZE] = { 0 };
        memcpy(bTmp, pbInData + (nInLen / AES_BLOCK_SIZE) * AES_BLOCK_SIZE, nInLen % AES_BLOCK_SIZE);
        memset(bTmp + nInLen % AES_BLOCK_SIZE, (unsigned char)nInLen % AES_BLOCK_SIZE, AES_BLOCK_SIZE - (nInLen % AES_BLOCK_SIZE));

        aes_encrypt(pContext, bTmp, pbOutData + (nInLen / AES_BLOCK_SIZE) * AES_BLOCK_SIZE);
    }
    else
    {
        unsigned char bTmp[AES_BLOCK_SIZE] = { 0 };
        memset(bTmp, 0, AES_BLOCK_SIZE);

        aes_encrypt(pContext, bTmp, pbOutData + (nInLen / AES_BLOCK_SIZE) * AES_BLOCK_SIZE);
    }

    *ppbOutData = pbOutData;
    *pnOutLen = nOutLen;
}

void AES_Decrypt(AES_Context *pContext, unsigned char* pbInData, int nInLen, unsigned char** ppbOutData, int* pnOutLen)
{
    int nPaddingLen = 0;
    int nOutLen = (nInLen / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char* pbOutData = (unsigned char*)malloc(nOutLen);
    for(int i = 0; i < (nInLen / AES_BLOCK_SIZE); i ++)
    {
        aes_decrypt(pContext, pbInData + i * AES_BLOCK_SIZE, pbOutData + i * AES_BLOCK_SIZE);
        nPaddingLen = AES_BLOCK_SIZE - pbOutData[(i + 1) * AES_BLOCK_SIZE - 1];
    }

    *ppbOutData = pbOutData;
    *pnOutLen = nOutLen - nPaddingLen;
}

#else


void AES_Encrypt(unsigned char* pbKey, unsigned char* pbInData, int nInLen, unsigned char** ppbOutData, int* pnOutLen)
{
    AES_KEY key;
    AES_set_encrypt_key((unsigned char*)pbKey, 128, &key);

    int nOutLen = ((nInLen + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char* pbOutData = (unsigned char*)malloc(nOutLen);
    memset(pbOutData, 0, nOutLen);
    for(int i = 0; i < nOutLen / AES_BLOCK_SIZE; i ++)
        AES_ecb_encrypt(pbInData + i * AES_BLOCK_SIZE, pbOutData + i * AES_BLOCK_SIZE, &key, AES_ENCRYPT);

    *ppbOutData = pbOutData;
    *pnOutLen = nOutLen;
}

void AES_Decrypt(unsigned char* pbKey, unsigned char* pbInData, int nInLen, unsigned char** ppbOutData, int* pnOutLen)
{
    if(nInLen % AES_BLOCK_SIZE)
        return;

    AES_KEY key;
    AES_set_decrypt_key((unsigned char*)pbKey, 128, &key);

    int nOutLen = nInLen;
    unsigned char* pbOutData = (unsigned char*)malloc(nOutLen);
    for(int i = 0; i < nInLen / AES_BLOCK_SIZE; i ++)
        AES_ecb_encrypt(pbInData + i * AES_BLOCK_SIZE, pbOutData + i * AES_BLOCK_SIZE, &key, AES_DECRYPT);

    *ppbOutData = pbOutData;
    *pnOutLen = nOutLen;
}


#endif
