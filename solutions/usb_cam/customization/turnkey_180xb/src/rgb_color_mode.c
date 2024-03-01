#include "appdef.h"

#if (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_103v3)
#include "rgb_color_v1.0.3.3.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_103v8)
#include "rgb_color_v1.0.3.8.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_104v1)
#include "rgb_color_v1.0.4.1.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_103v9)
#include "rgb_color_v1.0.3.9.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v0)
#include "rgb_color_v2.1.0.0.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v1)
#include "rgb_color_v2.1.0.1.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v2)
#include "rgb_color_v2.1.0.2.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v4)
#include "rgb_color_v2.1.0.4.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v7)
#include "rgb_color_v2.1.0.7.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v8)
#include "rgb_color_v2.1.0.8.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v9)
#include "rgb_color_v2.1.0.9.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v10)
#include "rgb_color_v2.1.0.10.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v11)
#include "rgb_color_v2.1.0.11.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_22v0)
#include "rgb_color_v2.2.0.0.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_301v9)
#include "rgb_color_v3.0.1.9.inc"
#else // DEFAULT_ISP_BIN_VER
#error "invalid isp bin version"
#endif // DEFAULT_ISP_BIN_VER
