#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include "Htypes.h"
#define FEATURE_PATCH_COUNT	7

extern unsigned char g_abEncData[0xa0];
extern unsigned char pE048AFC8Dic_tmp[72];
#define OFF_1 0     //[0 - 4]: 4
#define OFF_2 4     //[4 - 12]: 8
#define OFF_3 12    //[12 - 84]: 72
#define OFF_4 84    //[84 - 156]: 64
#define OFF_5 148    //[148 - 155]: 7

//_u8 *g_featureDict;
extern _u8 *g_NewDic;
extern _u16 *word_E0492BB4;
extern _u8 *unk_E0493600;
extern _s32 *unk_E0493680;
extern _s32 *unk_E0493878;
extern _u8 *dword_E0493A70;
extern _u16 *unk_E04941BC;

extern _s32 *dword_E04944DC;//exp(nIndex * 4) * 32
extern _u8 *byte_E0494504;
extern _s16 *word_E0495144;//exp(nIndex / 1024) * 32
extern _s32 *unk_E0497148;
extern _s16 *unk_E0497264;
extern _s16 *unk_E04972F4;
extern _u8 *unk_E0497384;
extern _u16 *word_E0499CA0;

extern _u16 *word_E0499D58;//sqrt(nIndex) * 1024
extern _s16 *unk_E04CB71C;//atan(nYDelta, nXDelta) * 0x1000 / pi
extern _u8 *unk_E04CB728;
extern _u8 *unk_E04CBC48;
extern _u8 *unk_E04CC230;
extern _u8 *byte_E0492D50;
extern _s16 *word_E04CA95C;
extern _s16 *word_E04CAA7C;
extern _u8 *byte_E04CAF9C;
extern _u8 *unk_E04CB49C;
extern _u8 *unk_E04CB4EC;
extern _u8 *unk_E04CB53C;
extern _u8 *unk_E04CB5B4;
extern _u8 *unk_E04CB62C;
extern _u8 *unk_E04CB6A4;
extern _u8* pE04CAB9CDic;
extern _u8* unk_E0497884_1;//(nIndex)/3
extern _u16* unk_E04941BC_1;//2^16 / (nIndex + 1)
extern _s32* unk_E0497B84_1;//
extern _s16* unk_E0492E00_1;
extern _u8* unk_E048B010_1;
extern _s16* unk_E049B55C_1;
extern _s16* unk_E04A335C_1;
extern _s16* unk_E04AB15C_1;
extern _s16* unk_E04B2F5C_1;
extern _s16* unk_E04BAD5C_1;
extern _s16* unk_E04C2B5C_1;
extern _s32* unk_E049A55C_1;
extern _u8* unk_E0497384_1;		//ok

extern _u8* pFDHaarDic;
extern _s32* pFDClassifyThresDic;
extern _s32* pFDClassifyWeightDic;
extern _s32* pFDClassifyStrongThresDic;
extern _u8* pDE676614Dic;
extern _u8* pDE677700Dic;
extern _u32* pDE210354Dic;
extern _s32* pDE219F94Dic;
extern _s32* pDE2101C4Dic;
extern _s32* dword_E0499B84;
extern _u8* pDE678750Dic;
extern _u8* pDE67C44CDic;
extern _s32* pDE281B68Dic;//feature extract
extern _s32* pDE21A128Dic;//feature Extract

extern _s32* pE0537244Dic;//modeling
extern _s32* pE0538DD4Dic;//modeling
extern _u8* pE053710CDic;//feature
extern _s32* pE053A964Dic;//modeling
extern _u8* pE064296CDic;//feature


extern _u8* pE053713CDic[FEATURE_PATCH_COUNT];
extern _u8* pE0537120Dic[FEATURE_PATCH_COUNT];
extern _s16* pE05371ACDic[FEATURE_PATCH_COUNT];
extern _s16* pE0537190Dic[FEATURE_PATCH_COUNT];
extern _s16* pE0537200Dic[FEATURE_PATCH_COUNT];
extern _s16* pE053721CDic[FEATURE_PATCH_COUNT];

extern _u8* pE048B018Dic;//feature extract
extern _s16* pE0493B0CDic;//refine
extern _u8* pE04CB39CDic;//refine
extern _u8* pE048AFA4Dic;//match
extern _u8* pE048AFC8Dic;//match

//int loadDictionary(char* szDictionaryFile);
int releaseDictionary();
int loadDictionary(unsigned char* pbDictData);
int loadDictionary_part1(unsigned char* pbDictData);
int loadDictionary_part2(unsigned char* pbDictData);

void  SetDecodedData(unsigned int iKey, unsigned char* pbData);


#endif
