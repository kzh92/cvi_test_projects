#include "appdef.h"

#if (USE_WHITE_LED != 1)

#if (DEFAULT_ISP_BINM_VER == ISP_BINM_VER_100v1)
#include "rgb_mono_v1.0.0.1.inc"
#else // DEFAULT_ISP_BINM_VER
#error "invalid isp mono bin version"
#endif // DEFAULT_ISP_BINM_VER

#endif // !USE_WHITE_LED