#ifndef _PCSPATTERNMATCHER_H_
#define _PCSPATTERNMATCHER_H_

#include "armFeaturePattern.h"

#ifndef __EMULATOR_MODE__

#ifdef __cplusplus
extern	"C"
{
#endif

int MinuteMatchByCMatcher(ARM_Matcher* psMatcher, ARM_MatchInfo* pMatchInfo, ARM_MatchResult* psMatchResult, int nFilterNum, float rDeltaClassifierThreshold);
int CoarseMatchByCMatcher(ARM_Matcher* pxMatcher, ARM_MatchResult* pxMatchResult, ARM_MatchInfo* psMatchInfo, float rDist, int nFilterNum, float rDeltaClassifierThreshold);
void CMatchResultByConstruction(ARM_MatchResult *psMatchResult);

int MinuteMatchByCMatcher_Fix(ARM_Matcher* psMatcher, ARM_MatchInfo* pMatchInfo, ARM_MatchResult* psMatchResult, int nFilterNum  );
int CoarseMatchByCMatcher_Fix(ARM_Matcher* psMatcher, ARM_MatchInfo* pMatchInfo, int arg_0_rUnk, int nFilterNum );		//sub_10113B30
void CMatchResultByConstruction_Fix(ARM_MatchResult *psMatchResult);

#ifdef __cplusplus
}
#endif

#endif

#endif
