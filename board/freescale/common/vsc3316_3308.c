/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "vsc3316_3308.h"

#define REVISION_ID_REG		0x7E
#define INTERFACE_MODE_REG		0x79
#define CURRENT_PAGE_REGISTER		0x7F
#define CONNECTION_CONFIG_PAGE		0x00
#define INPUT_STATE_REG		0x13
#define GLOBAL_INPUT_ISE1		0x51
#define GLOBAL_INPUT_ISE2		0x52
#define GLOBAL_INPUT_LOS		0x55
#define GLOBAL_CORE_CNTRL		0x5D
#define OUTPUT_MODE_PAGE		0x23
#define CORE_CONTROL_PAGE		0x25
#define CORE_CONFIG_REG		0x75

int vsc_if_enable(unsigned int vsc_addr)
{
	u8 data;

	debug("VSC:Configuring VSC at I2C address 0x%2x"
			" for 2-wire interface\n", vsc_addr);

	/* enable 2-wire Serial InterFace (I2C) */
	data = 0x02;
	return i2c_write(vsc_addr, INTERFACE_MODE_REG, 1, &data, 1);
}

int vsc3316_config(unsigned int vsc_addr, const int8_t con_arr[][2],
		unsigned int num_con)
{
	unsigned int i;
	u8 rev_id = 0;
	int ret;

	debug("VSC:Initializing VSC3316 at I2C address 0x%2x"
		" for Tx\n", vsc_addr);

	ret = i2c_read(vsc_addr, REVISION_ID_REG, 1, &rev_id, 1);
	if (ret < 0) {
		printf("VSC:0x%x could not read REV_ID from device.\n",
			vsc_addr);
		return ret;
	}

	if (rev_id != 0xab) {
		printf("VSC: device at address 0x%x is not VSC3316/3308.\n",
			vsc_addr);
		return -ENODEV;
	}

	ret = vsc_if_enable(vsc_addr);
	if (ret) {
		printf("VSC:0x%x could not configured for 2-wire I/F.\n",
			vsc_addr);
		return ret;
	}

	/* config connections - page 0x00 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, CONNECTION_CONFIG_PAGE);

	/* Making crosspoint connections, by connecting required
	 * input to output */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][1], con_arr[i][0]);

	/* input state - page 0x13 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, INPUT_STATE_REG);
	/* Configuring the required input of the switch */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][0], 0x80);

	/* Setting Global Input LOS threshold value */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_LOS, 0x60);

	/* config output mode - page 0x23 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, OUTPUT_MODE_PAGE);
	/* Turn ON the Output driver correspond to required output*/
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr,  con_arr[i][1], 0);

	/* configure global core control register, Turn on Global core power */
	i2c_reg_write(vsc_addr, GLOBAL_CORE_CNTRL, 0);

	vsc_wp_config(vsc_addr);

	return 0;
}

int vsc3308_config(unsigned int vsc_addr, const int8_t con_arr[][2],
		unsigned int num_con)
{
	unsigned int i;
	u8 rev_id = 0;
	int ret;

	debug("VSC:Initializing VSC3308 at I2C address 0x%x"
		" for Tx\n", vsc_addr);

	ret = i2c_read(vsc_addr, REVISION_ID_REG, 1, &rev_id, 1);
	if (ret < 0) {
		printf("VSC:0x%x could not read REV_ID from device.\n",
			vsc_addr);
		return ret;
	}

	if (rev_id != 0xab) {
		printf("VSC: device at address 0x%x is not VSC3316/3308.\n",
			vsc_addr);
		return -ENODEV;
	}

	ret = vsc_if_enable(vsc_addr);
	if (ret) {
		printf("VSC:0x%x could not configured for 2-wire I/F.\n",
			vsc_addr);
		return ret;
	}

	/* config connections - page 0x00 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, CONNECTION_CONFIG_PAGE);

	/* Making crosspoint connections, by connecting required
	 * input to output */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][1], con_arr[i][0]);

	/*Configure Global Input ISE and gain */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_ISE1, 0x12);
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_ISE2, 0x12);

	/* input state - page 0x13 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, INPUT_STATE_REG);
	/* Turning ON the required input of the switch */
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr, con_arr[i][0], 0);

	/* Setting Global Input LOS threshold value */
	i2c_reg_write(vsc_addr, GLOBAL_INPUT_LOS, 0x60);

	/* config output mode - page 0x23 */
	i2c_reg_write(vsc_addr, CURRENT_PAGE_REGISTER, OUTPUT_MODE_PAGE);
	/* Turn ON the Output driver correspond to required output*/
	for (i = 0; i < num_con ; i++)
		i2c_reg_write(vsc_addr,  con_arr[i][1], 0);

	/* configure global core control register, Turn on Global core power */
	i2c_reg_write(vsc_addr, GLOBAL_CORE_CNTRL, 0);

	vsc_wp_config(vsc_addr);

	return 0;
}

void vsc_wp_config(unsigned int vsc_addr)
{
	debug("VSC:Configuring VSC at address:0x%x for WP\n", vsc_addr);

	/* For new crosspoint configuration to occur, WP bit of
	 * CORE_CONFIG_REG should be set 1 and then reset to 0 */
	i2c_reg_write(vsc_addr, CORE_CONFIG_REG, 0x01);
	i2c_reg_write(vsc_addr, CORE_CONFIG_REG, 0x0);
}
