#pragma once

#include <vector>
#include <stdio.h>

#ifndef byte
	typedef unsigned char byte;
#endif

enum eChunks
{
	SERIAL_CHUNK_VERSION = 0x01,	//	1 byte of data - version
	SERIAL_CHUNK_USER_NAME = 0x02,	//	1 + N bytes - length + N bytes of customer's name (without enging \0).
	SERIAL_CHUNK_EMAIL = 0x03,	//	1 + N bytes - length + N bytes of customer's email (without ending \0).
	SERIAL_CHUNK_HWID = 0x04,	//	1 + N bytes - length + N bytes of hardware id (N % 4 == 0)
	SERIAL_CHUNK_EXP_DATE = 0x05,	//	4 bytes - (year << 16) + (month << 8) + (day)
	SERIAL_CHUNK_RUNNING_TIME_LIMIT = 0x06,	//	1 byte - number of minutes
	SERIAL_CHUNK_PRODUCT_CODE = 0x07,	//	8 bytes - used for decrypting some parts of exe-file
	SERIAL_CHUNK_USER_DATA = 0x08,	//	1 + N bytes - length + N bytes of user data
	SERIAL_CHUNK_MAX_BUILD = 0x09,	//	4 bytes - (year << 16) + (month << 8) + (day)

	SERIAL_CHUNK_END = 0xFF	//	4 bytes - checksum: the first four bytes of sha-1 hash from the data before that chunk
};


enum VMProtectErrors
{
	ALL_RIGHT = 0,
	UNSUPPORTED_ALGORITHM = 1,
	UNSUPPORTED_NUMBER_OF_BITS = 2,
	USER_NAME_IS_TOO_LONG = 3,
	EMAIL_IS_TOO_LONG = 4,
	USER_DATA_IS_TOO_LONG = 5,
	HWID_HAS_BAD_SIZE = 6,
	PRODUCT_CODE_HAS_BAD_SIZE = 7,
	SERIAL_NUMBER_TOO_LONG = 8,
	BAD_PRODUCT_INFO = 9,
	BAD_SERIAL_NUMBER_INFO = 10,
	BAD_SERIAL_NUMBER_CONTAINER = 11,
	NOT_EMPTY_SERIAL_NUMBER_CONTAINER = 12,
	BAD_PRIVATE_EXPONENT = 13,
	BAD_MODULUS = 14,
};
enum VMProtectSerialNumberFlags
{
	HAS_USER_NAME		= 0x00000001,
	HAS_EMAIL			= 0x00000002,
	HAS_EXP_DATE		= 0x00000004,
	HAS_MAX_BUILD_DATE	= 0x00000008,
	HAS_TIME_LIMIT		= 0x00000010,
	HAS_HARDWARE_ID		= 0x00000020,
	HAS_USER_DATA		= 0x00000040,
	SN_FLAGS_PADDING	= 0xFFFFFFFF
};

enum VMProtectAlgorithms
{
	ALGORITHM_RSA = 0,
	ALGORITHM_PADDING = 0xFFFFFFFF
};

#define MAKEDATE(y, m, d) (DWORD)((y << 16) + (m << 8) + d)

#pragma pack(push, 1)
struct VMProtectProductInfo
{
	VMProtectAlgorithms	algorithm;
	size_t		nBits;
	size_t		nPrivateSize;
	byte		*pPrivate;
	size_t		nModulusSize;
	byte		*pModulus;
	size_t		nProductCodeSize;
	byte		*pProductCode;
};

struct CustomSerialNumberInfo
{
    int			flags;
	char		*pHardwareID;
	size_t		nUserDataLength;
    byte		*pUserData;
};
#pragma pack(pop)

using namespace std;
