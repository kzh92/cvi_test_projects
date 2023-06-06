#include <stdio.h>
#include <stdlib.h>
#include <aos/cli.h>
#include "cvi_tempsen.h"

void test_tempsen(int32_t argc, char **argv)
{
    cvi_tempsen_t tps;
	int ret = 0;
	unsigned int force_val = 0x30;

	cvi_tempsen_init(&tps);
	unsigned int temp;
	for (int i = 0; i < 10; ++i)
    {
		if (i >= 5) {
			temp = cvi_tempsen_test_force_val(&tps, force_val);	
			aos_cli_printf("forceal = 0x%x\n", temp);
			if (temp != force_val)
				ret = -1;
		} else {
			temp = cvi_tempsen_read_temp_mC(&tps, 1000);
			aos_cli_printf("temp = %d mC\n", temp);
		}
        mdelay(1000);
    }
	aos_cli_printf("test tempsen %s\n", ret ? "failed" : "success");
}
ALIOS_CLI_CMD_REGISTER(test_tempsen, test_tempsen, test tempsen function);
