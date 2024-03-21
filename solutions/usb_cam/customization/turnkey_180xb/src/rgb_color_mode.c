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
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v12)
#include "rgb_color_v2.1.0.12.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v13)
#include "rgb_color_v2.1.0.13.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v15)
#include "rgb_color_v2.1.0.15.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v16)
#include "rgb_color_v2.1.0.16.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v17)
#include "rgb_color_v2.1.0.17.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v18)
#include "rgb_color_v2.1.0.18.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v19)
#include "rgb_color_v2.1.0.19.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v20)
#include "rgb_color_v2.1.0.20.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v21)
#include "rgb_color_v2.1.0.21.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_22v0)
#include "rgb_color_v2.2.0.0.inc"
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_301v9)
#include "rgb_color_v3.0.1.9.inc"
#else // DEFAULT_ISP_BIN_VER
#error "invalid isp bin version"
#endif // DEFAULT_ISP_BIN_VER
