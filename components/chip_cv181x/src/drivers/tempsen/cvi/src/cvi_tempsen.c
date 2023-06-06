#include "cvi_tempsen.h"
#include "hal_tempsen.h"
#include "aos/cli.h"

cvi_error_t cvi_tempsen_init(cvi_tempsen_t *tps)
{
	if (target_get(DEV_DW_TEMPSEN_TAG, 0, tps) != CSI_OK) {
		aos_cli_printf("tempsen %d init failed!\n", 0);
		return CVI_ERROR;
	}
	if (!tps->reg_base)
		return CVI_ERROR;
	set_hal_tempsen_base(tps->reg_base);
	tempsen_init();

	return CVI_OK;
}

void cvi_tempsen_uninit(cvi_tempsen_t *tps)
{
	tempsen_uninit();
	set_hal_tempsen_base(0);
}

unsigned int cvi_tempsen_read_temp_mC(cvi_tempsen_t *tps,
				      unsigned int   timeout_ms)
{
	if (!get_hal_tempsen_base())
		return CVI_ERROR;
	unsigned int temp;
	if (wait_for_finish(CHAN0, timeout_ms)) {
		return CVI_TIMEOUT;
	}
	temp = read_temp(CHAN0);
	pr_debug("temp regval:0x%x\n", temp);
	return ((temp * 1000) * 716 / 2048 - 273000);
}

unsigned int cvi_tempsen_read_max_temp(cvi_tempsen_t *tps)
{
	if (!get_hal_tempsen_base())
		return CVI_ERROR;
	unsigned int temp;
	temp = read_max_temp(CHAN0);
	return ((temp * 1000) * 716 / 2048 - 273000);
}

unsigned int cvi_tempsen_test_force_val(cvi_tempsen_t *tps, unsigned int force_val)
{
	return tempsen_set_force_val(force_val);
}