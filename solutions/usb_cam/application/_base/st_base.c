#include "st_base.h"
#include "mi_sys.h"
#include "mi_vif.h"
#include "mi_vif_impl.h"
#include "mi_vpe_impl.h"
#include "mi_venc_impl.h"
#include "mi_sys_datatype.h"

static int mi_module_init = 0;
static int mi_venc_inited = 0;

#define STCHECKRESULT02(result, execfunc)\
    result = execfunc;    \
    if (result != MI_SUCCESS)\
    {\
        CamOsPrintf("[%s %d]exec function failed ret x%X,\r\n", __FUNCTION__, __LINE__, result);\
        return 1;\
    }

int ST_Base_Init()
{
	MI_S32 s32Ret = MI_SUCCESS;
	if(mi_module_init == 0)
    {
        mi_module_init = 1;
        STCHECKRESULT02(s32Ret, MI_SYS_Init());
        // extern MI_S32 MI_MipiTx_IMPL_Init(void);
        // extern MI_S32 mi_divp_Init(void);
        MI_VIF_IMPL_Init();
        MI_VPE_IMPL_Init();
    }

	return 0;
}

int ST_Base_Exit()
{
	return 0;
}

int ST_Base_VENC_Init()
{
	if (mi_venc_inited == 0)
	{
		mi_venc_inited = 1;
        extern void MI_VENC_IMPL_Init(void);
		MI_VENC_IMPL_Init();
	}
	return 0;
}