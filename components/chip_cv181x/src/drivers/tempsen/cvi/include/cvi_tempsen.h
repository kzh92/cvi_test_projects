#ifndef __CVI_TEMPSEN_H
#define __CVI_TEMPSEN_H

#include <soc.h>
#include <drv/common.h>

#ifdef __cplusplus
extern "C" {
#endif

// #define CSI_DRV_DEBUG

#ifdef CSI_DRV_DEBUG
#define pr_debug(x, args...) aos_cli_printf("[%s|%d] - " x, __func__, __LINE__, ##args)
#else
#define pr_debug(x, args...) 
#endif


#define BIT(nr)      (UINT64_C(1) << (nr))
#define CHAN0 BIT(0)

typedef csi_dev_t cvi_tempsen_t;

cvi_error_t  cvi_tempsen_init(cvi_tempsen_t *tps);
void	     cvi_tempsen_uninit(cvi_tempsen_t *tps);
unsigned int cvi_tempsen_read_temp_mC(cvi_tempsen_t *tps, unsigned int timeout_ms);
unsigned int cvi_tempsen_read_max_temp(cvi_tempsen_t *tps);
unsigned int cvi_tempsen_test_force_val(cvi_tempsen_t *tps, unsigned int force_val);

#ifdef __cplusplus
}
#endif

#endif /* __CVI_TEMPSEN_H */
