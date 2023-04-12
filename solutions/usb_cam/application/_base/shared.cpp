#include "shared.h"

#include <string.h>

#include "DBManager.h"
#include "i2cbase.h"
#include "drv_gpio.h"
#include "sha1.h"
#include "mutex.h"
#include <debug/debug_overview.h>

#define SUNXI_SID_BASE		(0x01c23800)

mymutex_ptr   g_xLastDetectMutex = my_mutex_init();
float   g_rLastDetectTime = 0;
int     g_iUniqueID = 0;

#if 0
void WriteAverageBatt(int batt)
{
    FILE* fp = fopen("/db/averBatt.ini", "wb");
    if(fp)
    {
        fwrite(&batt, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
    LOG_PRINT("writeAverageBatt %d\n", batt);
}

int ReadAverageBatt()
{
    int batt = 0;
    FILE* fp = fopen("/db/averBatt.ini", "rb");
    if(fp)
    {
        fread(&batt, sizeof(int), 1, fp);
        fclose(fp);
    }

    if(batt < 0)
        return 0;

    return batt;
}

int ReadLowBatteryCount()
{
    int iLow_count = 0;
    int fpBat = open("/db/battery.ini", O_RDWR);
    if (fpBat > -1)
    {
        read(fpBat, &iLow_count, sizeof(iLow_count));
        close(fpBat);
    }
    if (iLow_count < 0)
        iLow_count = 0;
    return iLow_count;
}

int UpdateLowBatteryCount(int iLow_count)
{
    FILE* fpBat = fopen("/db/battery.ini", "wb");
    if (fpBat)
    {
        fwrite(&iLow_count, sizeof(iLow_count), 1, fpBat);
        my_fsync(fileno(fpBat));
        fclose(fpBat);
        return 0;
    }
    return 1;
}
#endif

void ResetDetectTimeout()
{
    my_mutex_lock(g_xLastDetectMutex);
    g_rLastDetectTime = Now();
    my_mutex_unlock(g_xLastDetectMutex);
}

void CSI_PWDN_ON()
{
#if 0
    //인식성공한다음에 재차 기동할때 색카메라 오유가 나오는 문제를 퇴치하기 위해서 전원을 끄기전에 CSI_PWDN을 H로 하게 하였다.
    GPIO_fast_config(CSI_PWDN, OUT);
    GPIO_fast_setvalue(CSI_PWDN, ON);
#endif
}

void CSI_PWDN_ON1()
{    
#if 0
    //인식성공한다음에 재차 기동할때 색카메라 오유가 나오는 문제를 퇴치하기 위해서 전원을 끄기전에 CSI_PWDN을 H로 하게 하였다.
    GPIO_fast_config(CSI_RSTN, OUT);
    GPIO_fast_setvalue(CSI_RSTN, ON);

//    GPIO_fast_config(CSI_PWDN, OUT);
//    GPIO_fast_setvalue(CSI_PWDN, ON);
//    my_usleep(10 * 1000);
//    GPIO_fast_setvalue(CSI_PWDN, OFF);
#endif
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


void GetSerialNumber(char* serialData)
{
    if(serialData == NULL)
        return;
#if 0 //V3s
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if(fp == NULL)
        return;

    char szLine[1024] = { 0 };
    while(!feof(fp))
    {
        fgets(szLine, sizeof(szLine), fp);

        if(strstr(szLine, "Serial"))
            break;
    }

    fclose(fp);

    char* tok = strtok(szLine, ": ");
    if(tok == NULL)
        return;

    tok = strtok(NULL, ": \t");
    if(tok == NULL)
        return;

    int i = 0;
    while (tok[i])
    {
        char c=tok[i];
        tok[i] = toupper(c);
        i++;
    }
#endif
#if 1
    char tok[20] = { 0 };
    unsigned int anUID[10] = { 0 };
    unsigned long long uuid = GetSSDID(anUID);
    sprintf(tok, "%llx", uuid);

    LOG_PRINT("=======get serail num %s\n", tok);
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

    my_free(digest);
    my_free(szEncData);

    LOG_PRINT("serial: %s\n", serialData);
#endif
}

int GetV3SID(unsigned int* piV3SID)
{
#if 0
    int fd;
    unsigned int addr_start, addr_offset;
    unsigned int PageSize, PageMask;
    unsigned int sid_base;
    void *pc;

    fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        my_printf("Unable to open /dev/mem\n");
        return -1;
    }

    PageSize = sysconf(_SC_PAGESIZE);
    PageMask = (~(PageSize-1));

    addr_start  = SUNXI_SID_BASE &  PageMask;
    addr_offset = SUNXI_SID_BASE & ~PageMask;

    pc = (void *)mmap(0, 0x10, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr_start);
    if (pc == MAP_FAILED) {
        my_printf("Unable to mmap file\n");
        my_printf("pc:%8.8x\n", (unsigned int)pc);
        return -1;
    }

    sid_base = (unsigned int)pc;
    sid_base += addr_offset;
    close(fd);

    piV3SID[0] = *((unsigned int *)sid_base);
    piV3SID[1] = *((unsigned int *)(sid_base + 4));
    piV3SID[2] = *((unsigned int *)(sid_base + 8));
    piV3SID[3] = *((unsigned int *)(sid_base + 0xc));

//    my_printf("V3S SID:\n");
//    my_printf("new id = 0x%08x 0x%08x \n", piV3SID[0], piV3SID[1]);

    munmap(pc, 0x10);
#endif
    return 0;
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


///////////////////////////TTF Font///////////////////////
#if 0
char* my_fgetws(MY_WCHAR* out, int max_len, char* pos)
{
    int i = 0;
    *out = 0;
#if 0
    my_debug("[%s] start\n", __FUNCTION__);
#endif
    do
    {
        out[i] = *(unsigned short*)pos;
        i ++;
        if (*pos == 0x0a && *(pos+1) == 0)
        {
            pos += 2; //sizeof(ushort);
            break;
        }
        pos += 2; //sizeof(ushort);
        if (i >= max_len -2)
            break;
    }while(1);
    out[i] = 0;
#if 0
    my_debug("my_fgetws %d:", i);
    wcsprint(out);
    my_debug("[%s] end\n", __FUNCTION__);
#endif
    return pos;
}

void wcsprint(const MY_WCHAR* text)
{
#ifdef USE_DEBUG
    int len = (int)wcslen(text);
    my_debug("len=%d:", len);
    for (int i = 0; i < len; i++)
        my_debug("%08X ", text[i]);
    my_debug("%s", "\n");
#endif /* USE_DEBUG */
}

char* my_strupr(char* str)
{
    int len;
    int i;
    len = strlen(str);
    for (i = 0; i < len; i ++)
    {
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] = str[i] - 'a' + 'A';
    }
    return str;
}

#endif
////////////////////////////////////////////////////////


#if 0
typedef struct _tagMEMINFO
{
    int     iTotalMem;
    int     iFreeMem;
    int     iBuffers;
    int     iCaches;

    int     iRealFreeMem;
} MEMINFO;

void GetMemInfo(MEMINFO* pxMemInfo)
{
    if(pxMemInfo == NULL)
        return;
#if 0
    FILE* fp = fopen("/proc/meminfo", "rt");
    if(fp)
    {
        for(int i = 0; i < 4; i ++)
        {
            char szLine[256] = { 0 };
            fgets(szLine, sizeof(szLine), fp);
//            my_printf("%s", szLine);

            char* szToken = strtok(szLine, " :\r\n");
            szToken = strtok(NULL, " :\r\n");

            if(szToken)
            {
                if(i == 0)
                    pxMemInfo->iTotalMem = atoi(szToken);
                else if(i == 1)
                    pxMemInfo->iFreeMem = atoi(szToken);
                else if(i == 2)
                    pxMemInfo->iBuffers = atoi(szToken);
                else if(i == 3)
                    pxMemInfo->iCaches = atoi(szToken);
            }
        }

        fclose(fp);
    }

    int iUsedMem = pxMemInfo->iTotalMem - pxMemInfo->iFreeMem - pxMemInfo->iCaches - pxMemInfo->iBuffers;
    pxMemInfo->iRealFreeMem = pxMemInfo->iTotalMem - iUsedMem;
#endif
}
#endif

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
    if (g_xCS.x.bUserCount != dbm_GetPersonCount())
    {
        g_xCS.x.bUserCount = dbm_GetPersonCount();
        UpdateCommonSettings();
    }
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
#if 0
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if(fp == NULL)
        return;

    char szLine[1024] = { 0 };
    while(!feof(fp))
    {
        fgets(szLine, sizeof(szLine), fp);

        if(strstr(szLine, "Serial"))
            break;
    }

    fclose(fp);

    char* tok = strtok(szLine, ": ");
    if(tok == NULL)
        return;

    tok = strtok(NULL, ": \t\r\n");
    if(tok == NULL)
        return;
#else
    char tok[20] = { 0 };
    unsigned int anUID[10] = { 0 };
    unsigned long long uuid = GetSSDID(anUID);
    sprintf(tok, "%llx", uuid);

    strcpy(serialData, tok);
#endif
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
