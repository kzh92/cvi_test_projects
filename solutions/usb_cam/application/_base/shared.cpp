#include "shared.h"

#include <string.h>

#include "DBManager.h"
#include "i2cbase.h"
#include "drv_gpio.h"
#include "sha1.h"
#include "mutex.h"
#include "aescrypt.h"
#include <debug/debug_overview.h>

#define SPI_NOR_MAX_ID_LEN      8

mymutex_ptr   g_xLastDetectMutex = my_mutex_init();
float   g_rLastDetectTime = 0;
int     g_iUniqueID = 0;

extern "C" int my_spi_nor_get_id(void *buf);

void ResetDetectTimeout()
{
    my_mutex_lock(g_xLastDetectMutex);
    g_rLastDetectTime = Now();
    my_mutex_unlock(g_xLastDetectMutex);
}

unsigned char g_abKey[16] = { 0x12, 0x84, 0xA3, 0x9B, 0xEE, 0x34, 0x7F, 0xFE };

static const char *codes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
int base64_encode(unsigned char *in, int inlen,
    unsigned char *out, int *outlen)
{
    int i, len2, leven;
    unsigned char *p;
    /* valid output size ? */
    len2 = 4 * ((inlen + 2) / 3);
    if (*outlen < len2 + 1) {
        *outlen = len2 + 1;
        return 0;
    }
    p = out;
    leven = 3 * (inlen / 3);
    for (i = 0; i < leven; i += 3) {
        *p++ = codes[(in[0] >> 2) & 0x3F];
        *p++ = codes[(((in[0] & 3) << 4) + (in[1] >> 4)) & 0x3F];
        *p++ = codes[(((in[1] & 0xf) << 2) + (in[2] >> 6)) & 0x3F];
        *p++ = codes[in[2] & 0x3F];
        in += 3;
    }
    /* Pad it if necessary...  */
    if (i < inlen) {
        unsigned a = in[0];
        unsigned b = (i + 1 < inlen) ? in[1] : 0;
        *p++ = codes[(a >> 2) & 0x3F];
        *p++ = codes[(((a & 3) << 4) + (b >> 4)) & 0x3F];
        *p++ = (i + 1 < inlen) ? codes[(((b & 0xf) << 2)) & 0x3F] : '=';
        *p++ = '=';
    }
    /* append a NULL byte */
    *p = '\0';
    /* return ok */
    *outlen = p - out;
    return 1;
}

int GetAesKey4ChipID(void* buf)
{
    if (!buf)
        return 1;
    char tok[128] = { 0 };
    unsigned int anUID[10] = { 0 };
    unsigned long long uuid = GetSSDID(anUID);
    unsigned char spinor_id[SPI_NOR_MAX_ID_LEN];
    my_spi_nor_get_id(spinor_id);
    sprintf(tok, "%llx%llx", uuid, *((long long*)spinor_id));

    dbug_printf("[%s] uuid:%s\n", __func__, tok);
    SHA1 sha1;
    sha1.addBytes(tok, strlen(tok));

    unsigned char* digest = sha1.getDigest(); //length is 20 bytes
    memcpy(buf, digest, AES_BLOCK_SIZE);
    my_free(digest);
    return 0;
}

void GetSerialNumber(char* serialData)
{
    if(serialData == NULL)
        return;
    char tok[128] = { 0 };
    unsigned int anUID[10] = { 0 };
    unsigned long long uuid = GetSSDID(anUID);
    sprintf(tok, "%llx", uuid);

    dbug_printf("=======get serial num %s\n", tok);
    SHA1 sha1;
    sha1.addBytes(tok, strlen(tok));

    unsigned char* digest = sha1.getDigest();

    unsigned char *szEncData = (unsigned char*)my_malloc(512);
    int iEncLen = 512;
    base64_encode(digest, 20, szEncData, &iEncLen);

    int index = 0;
    for(int i = 0; i < 13; i ++)
    {
        int code = ((unsigned int)szEncData[i * 2] * (unsigned int)szEncData[i * 2 + 1]) % 36;
        if(code < 26)
            serialData[index] = (char)('A' + code);
        else
            serialData[index] = (char)('0' + (code - 26));

        index ++;
        if(index == 4 || index == 9 || index == 14)
        {
            serialData[index] = '-';
            index ++;
        }
    }
    serialData[index] = 0;

    my_free(digest);
    my_free(szEncData);

    dbug_printf("serial: %s\n", serialData);
}

unsigned long long GetSSDID(unsigned int* piSSDID)
{
    unsigned long long uuid = my_get_chip_id();
    piSSDID[0] = (uuid >> 32);
    piSSDID[1] = (uuid & 0xFFFFFFFF);
    piSSDID[2] = piSSDID[0] | piSSDID[1];
    piSSDID[3] = piSSDID[0] & piSSDID[1];
    return uuid;
}

void GetUniquID(char* szDst)
{
    unsigned int anUID[10] = { 0 };

    anUID[0] = UNIQUIE_MARK;
    GetSSDID(anUID + 1);

    char szUniqueID[512] = { 0 };
    int iUniqueLen = sizeof(szUniqueID);
    base64_encode((unsigned char*)anUID, sizeof(int) * 5, (unsigned char*)szUniqueID, &iUniqueLen);

    g_iUniqueID = GetIntCheckSum((int*)anUID, sizeof(int) * 5);

    strcpy(szDst, szUniqueID);
}

void PrintFreeMem()
{
    debug_mm_overview(printf);
    printf("\n");
}

/**
 * @brief UpdateUserCount
 * 전체 카드기록개수를 갱신한다.
 */
void UpdateUserCount()
{
    int iUpdateFlag = 0;
    if (g_xCS.x.bUserCount != dbm_GetPersonCount())
    {
        g_xCS.x.bUserCount = dbm_GetPersonCount();
        iUpdateFlag = 1;
    }
#if (N_MAX_HAND_NUM)
    if (g_xCS.x.bHandCount != dbm_GetHandCount())
    {
        g_xCS.x.bHandCount = dbm_GetHandCount();
        iUpdateFlag = 1;
    }
#endif // N_MAX_HAND_NUM
    if (iUpdateFlag)
        UpdateCommonSettings();
}

void ShowUsbUpgradeFail()
{
    //notify error of upgrading.
    for (int i = 0; i < 5; i ++)
    {
        GPIO_fast_setvalue(IR_LED, 1);
        my_usleep(200*1000);
        GPIO_fast_setvalue(IR_LED, 0);
        my_usleep(200*1000);
    }
}

// Constants are the integer part of the sines of integers (in radians) * 2^32.
const uint32_t k[64] = {
0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee ,
0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 ,
0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be ,
0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 ,
0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa ,
0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8 ,
0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed ,
0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a ,
0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c ,
0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70 ,
0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05 ,
0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 ,
0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039 ,
0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1 ,
0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1 ,
0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

// r specifies the per-round shift amounts
const uint32_t r[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                      5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                      4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                      6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

// leftrotate function definition
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

void to_bytes(uint32_t val, uint8_t *bytes)
{
    bytes[0] = (uint8_t) val;
    bytes[1] = (uint8_t) (val >> 8);
    bytes[2] = (uint8_t) (val >> 16);
    bytes[3] = (uint8_t) (val >> 24);
}

uint32_t to_int32(const uint8_t *bytes)
{
    return (uint32_t) bytes[0]
        | ((uint32_t) bytes[1] << 8)
        | ((uint32_t) bytes[2] << 16)
        | ((uint32_t) bytes[3] << 24);
}

void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest)
{

    // These vars will contain the hash
    uint32_t h0, h1, h2, h3;

    // Message (to prepare)
    uint8_t *msg = NULL;

    size_t new_len, offset;
    uint32_t w[16];
    uint32_t a, b, c, d, i, f, g, temp;

    // Initialize variables - simple count in nibbles:
    h0 = 0x67452301;
    h1 = 0xefcdab89;
    h2 = 0x98badcfe;
    h3 = 0x10325476;

    //Pre-processing:
    //append "1" bit to message
    //append "0" bits until message length in bits ≡ 448 (mod 512)
    //append length mod (2^64) to message

    for (new_len = initial_len + 1; new_len % (512/8) != 448/8; new_len++)
        ;

    msg = (uint8_t*)my_malloc(new_len + 8);
    memcpy(msg, initial_msg, initial_len);
    msg[initial_len] = 0x80; // append the "1" bit; most significant bit is "first"
    for (offset = initial_len + 1; offset < new_len; offset++)
        msg[offset] = 0; // append "0" bits

    // append the len in bits at the end of the buffer.
    to_bytes(initial_len*8, msg + new_len);
    // initial_len>>29 == initial_len*8>>32, but avoids overflow.
    to_bytes(initial_len>>29, msg + new_len + 4);

    // Process the message in successive 512-bit chunks:
    //for each 512-bit chunk of message:
    for(offset=0; offset<new_len; offset += (512/8)) {

        // break chunk into sixteen 32-bit words w[j], 0 ≤ j ≤ 15
        for (i = 0; i < 16; i++)
            w[i] = to_int32(msg + offset + i*4);

        // Initialize hash value for this chunk:
        a = h0;
        b = h1;
        c = h2;
        d = h3;

        // Main loop:
        for(i = 0; i<64; i++) {

            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5*i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3*i + 5) % 16;
            } else {
                f = c ^ (b | (~d));
                g = (7*i) % 16;
            }

            temp = d;
            d = c;
            c = b;
            b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
            a = temp;

        }

        // Add this chunk's hash to result so far:
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;

    }

    // cleanup
    my_free(msg);

    //var char digest[16] := h0 append h1 append h2 append h3 //(Output is in little-endian)
    to_bytes(h0, digest);
    to_bytes(h1, digest + 4);
    to_bytes(h2, digest + 8);
    to_bytes(h3, digest + 12);
}

void GetSN(char* serialData)
{
    if(serialData == NULL)
        return;

    char tok[20] = { 0 };
    unsigned int anUID[10] = { 0 };
    unsigned long long uuid = GetSSDID(anUID);
    sprintf(tok, "%llx", uuid);

    strcpy(serialData, tok);
}

#ifdef USE_TWIN_ENGINE
int MainSTM_GetDict(unsigned char *pbData, int len)
{
    unsigned char g_abEncData[0xa0] =
    {
      0x5C, 0x21, 0x00, 0x00, 0x01, 0xA8, 0x73, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0x5D, 0xFF, 0xFF, 0xFF,
      0x94, 0xFF, 0xFF, 0xFF, 0x10, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
      0xFE, 0xFF, 0xFF, 0xFF, 0x6B, 0xFF, 0xFF, 0xFF, 0x81, 0xFF, 0xFF, 0xFF, 0xEB, 0xFF, 0xFF, 0xFF,
      0x1B, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xA0, 0xFF, 0xFF, 0xFF,
      0x87, 0xFF, 0xFF, 0xFF, 0xAD, 0xFF, 0xFF, 0xFF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFA, 0xFF, 0xFF, 0xFF,
      0x01, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x26, 0x00, 0x3E, 0x00, 0x9E, 0x00, 0xB6, 0x00, 0xCE, 0x00,
      0x2E, 0x01, 0x46, 0x01, 0x5E, 0x01, 0xBE, 0x01, 0xD6, 0x01, 0xEE, 0x01, 0x4E, 0x02, 0x66, 0x02,
      0x7E, 0x02, 0xDE, 0x02, 0xF6, 0x02, 0x0E, 0x03, 0x6E, 0x03, 0x86, 0x03, 0x9E, 0x03, 0xFE, 0x03,
      0x16, 0x04, 0x2E, 0x04, 0x8E, 0x04, 0xA6, 0x04, 0xBE, 0x04, 0x1E, 0x05, 0x36, 0x05, 0x4E, 0x05,
      0xAE, 0x05, 0xC6, 0x05, 0x0E, 0x10, 0x14, 0x17, 0x1B, 0x20, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    memcpy(pbData, g_abEncData, len);
    return 0;
}
#endif // USE_TWIN_ENGINE
