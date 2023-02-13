// GetIDKey.cpp : Defines the exported functions for the DLL application.
//

#include "KeyGenBase.h"
#include "KeyGenAPI.h"
// #include <vector>
// #include <iostream>
// #include <algorithm>
#include <string.h>

#include "sshbn.h"
#include "b64.h"
#include "common_types.h"

// using namespace std;

VMProtectAlgorithms g_Algorithm = ALGORITHM_RSA;

int g_iFLBits = 512;
unsigned char g_module_FL[] = { 0xef, 0xdb, 0xea, 0x35, 0x18, 0x94, 0x3c, 0x9d, 0x62, 0x3b, 0x8e, 0x77, 0xf8, 0x77, 
								0x71, 0xbc, 0x2a, 0x71, 0x75, 0xab, 0xdd, 0x10, 0x32, 0x9d, 0x02, 0x42, 0x43, 0x90, 0x44, 
								0x9f, 0x44, 0x76, 0x77, 0xa4, 0xb2, 0x69, 0x19, 0xd8, 0xd1, 0x4a, 0x9b, 0xdc, 0xff, 0xf3, 
								0xa2, 0x0b, 0x0a, 0xa4, 0xa0, 0xd6, 0xef, 0x57, 0x93, 0xbc, 0x51, 0xf2, 0x1f, 0x4e, 0xcb, 
								0x73, 0x90, 0x84, 0xf8, 0xd3 };
unsigned char g_private_FL[] = { 0xa7, 0x35, 0xc0, 0xdd, 0x08, 0xe8, 0x76, 0x25, 0x25, 0x50, 0xed, 0x6d, 0xb7, 0x19, 
								0xd5, 0x80, 0xde, 0x79, 0xc0, 0x15, 0x72, 0x83, 0x93, 0x35, 0x1f, 0x59, 0x00, 0x6e, 0xb7, 
								0x57, 0xec, 0x49, 0xec, 0x53, 0x9c, 0xc6, 0xa4, 0x6d, 0xbb, 0xe5, 0xeb, 0x30, 0xb6, 0xb6, 
								0x76, 0xb3, 0x69, 0x7e, 0x49, 0x39, 0x6f, 0xae, 0x87, 0x6c, 0x28, 0x9c, 0xdd, 0xfe, 0xe9, 
								0x72, 0x05, 0x8d, 0x1e, 0x49 };

const char* g_public_FL = "AAEAAQ==";


int _decodeFaceLicense(char* szLicense, CustomSerialNumberInfo& vInfo)
{
	// std::vector<byte> vModule, vPublic, vProductCode, vLicense;
	// vModule = std::vector<byte>(g_module_FL, g_module_FL + sizeof(g_module_FL));
 	// base64_decode(g_public_FL, strlen(g_public_FL), vPublic);
	// base64_decode(szLicense, strlen(szLicense), vLicense);    

	unsigned char* vModule;
	unsigned char* vPublic;
	// unsigned char* vProductCode;
	unsigned char* vLicense;
	VMProtectProductInfo	pi;
	unsigned int nLicenseLen = 0;

	vModule = (unsigned char*)g_module_FL;
	vPublic = b64_decode_ex(g_public_FL, strlen(g_public_FL), &pi.nPrivateSize);
	vLicense = b64_decode_ex(szLicense, strlen(szLicense), &nLicenseLen);
	
	pi.algorithm = g_Algorithm;
	pi.nBits = g_iFLBits;
	pi.nModulusSize = sizeof(g_module_FL);
	pi.pModulus = vModule;
	// pi.nPrivateSize = sizeof((char*)vPublic);
	pi.pPrivate = (byte *)vPublic;
	// pi.nProductCodeSize = sizeof(vProductCode);
	// pi.pProductCode = (byte *)vProductCode;

	Bignum e = bignum_from_bytes(pi.pPrivate, pi.nPrivateSize);
	Bignum n = bignum_from_bytes(pi.pModulus, pi.nModulusSize);
	Bignum x = bignum_from_bytes(vLicense, nLicenseLen);
	if (bignum_cmp(n, x) < 0) // is it possible after check in AddPadding()?
	{
		freebn(e);
		freebn(n);
		freebn(x);
		return SERIAL_NUMBER_TOO_LONG; // data is too long to crypt
	}

	Bignum y = modpow(x, e, n);
	int nBytes;
	byte *pRes = bignum_to_bytes(y, &nBytes);
	memset(vLicense, 0, nBytes);
	// vLicense.clear();
	memcpy(vLicense, pRes, nBytes);

	// vLicense.insert(vLicense.end(), pRes, pRes + nBytes);

	my_free(pRes);
	freebn(y);
	freebn(x);
	freebn(e);
	freebn(n);

	if (nBytes < 64)
	{
		return HWID_HAS_BAD_SIZE;
	}

	if (vLicense[0] != 0 || vLicense[1] != 2)
	{
		return BAD_SERIAL_NUMBER_INFO;
	}

	int idx = -1;
	for (unsigned int i = 2; i < (unsigned int)nBytes; i++)
	{
		if (vLicense[i] == 0x00)
		{
			idx = i + 1;
			break;
		}
	}

	if (vLicense[idx] != SERIAL_CHUNK_VERSION || vLicense[idx + 1] != 1)
	{
		return BAD_SERIAL_NUMBER_CONTAINER;
	}

	memset(&vInfo, 0, sizeof(vInfo));

	idx += 2;

	if (vLicense[idx] == (byte)SERIAL_CHUNK_HWID)
	{
		vInfo.flags |= HAS_HARDWARE_ID;
		int len = vLicense[idx + 1];

        char* szHWID = (char*)my_malloc(1024);
		szHWID = base64_encode(&vLicense[idx + 2], len);
		vInfo.pHardwareID = szHWID;

		idx += len + 2;
	}	
	
	if (vLicense[idx] == (byte)SERIAL_CHUNK_USER_DATA)
	{
		vInfo.flags |= HAS_EMAIL;
		vInfo.nUserDataLength = vLicense[idx + 1];
        vInfo.pUserData = (byte*)my_malloc(vInfo.nUserDataLength);
		memcpy(vInfo.pUserData, &vLicense[idx + 2], vInfo.nUserDataLength);
		idx += (vInfo.nUserDataLength + 2);
	}
	
	if (vLicense[idx] != (byte)SERIAL_CHUNK_END)
	{
		return BAD_SERIAL_NUMBER_INFO;
	}

	return ALL_RIGHT;
}
