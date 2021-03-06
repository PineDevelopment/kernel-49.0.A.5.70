/*
 *
 * FocalTech TouchScreen driver.
 *
 * Copyright (c) 2010-2017, Focaltech Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*****************************************************************************
*
* File Name: focaltech_core.h

* Author: Focaltech Driver Team
*
* Created: 2016-08-08
*
* Abstract:
*
* Reference:
*
*****************************************************************************/

#ifndef __LINUX_FOCALTECH_CORE_H__
#define __LINUX_FOCALTECH_CORE_H__

#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/i2c.h>
#include <linux/vmalloc.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/jiffies.h>

#include <linux/dma-mapping.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/version.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
//#include <linux/rtpm_prio.h>
#include <linux/fs.h>
#include <linux/device.h>
#include "tpd.h"

/*****************************************************************************
* 1.Included header files
*****************************************************************************/
#include "focaltech_common.h"
#include "focaltech_flash.h"


/*[Arima_8103][allen_yu] SW ESD Work around 20170822 begin*/
//#define ESD_CHECK
#ifdef ESD_CHECK
extern int live_state;
extern int fwupd_flag;
#endif
/*[Arima_8103][allen_yu] 20170822 end*/

/*[Arima_8103][allen_yu] Modify the maximum number of fingers supported 20170711 begin*/
#define FTS_MAX_POINTS                          5
/*[Arima_8103][allen_yu] 20170711 end*/
#define FTS_MAX_ID                              0x0F
#define FTS_TOUCH_STEP                          6
#define FTS_FACE_DETECT_POS                     1
#define FTS_TOUCH_X_H_POS                       3
#define FTS_TOUCH_X_L_POS                       4
#define FTS_TOUCH_Y_H_POS                       5
#define FTS_TOUCH_Y_L_POS                       6
#define FTS_TOUCH_EVENT_POS                     3
#define FTS_TOUCH_ID_POS                        5
#define FT_TOUCH_POINT_NUM                      2
#define FTS_TOUCH_XY_POS                        7
#define FTS_TOUCH_MISC                          8
#define POINT_READ_BUF                          (3 + FTS_TOUCH_STEP * FTS_MAX_POINTS)
#define FT_FW_NAME_MAX_LEN                      50
#define TPD_DELAY                               (2*HZ/100)
#define FTS_SYSFS_ECHO_ON(buf)      ((strnicmp(buf, "1", 1)  == 0) || \
                                        (strnicmp(buf, "on", 2) == 0))
#define FTS_SYSFS_ECHO_OFF(buf)     ((strnicmp(buf, "0", 1)  == 0) || \
                                        (strnicmp(buf, "off", 3) == 0))


/*****************************************************************************
* Private enumerations, structures and unions using typedef
*****************************************************************************/
struct touch_info
{
    int y[FTS_MAX_POINTS];
    int x[FTS_MAX_POINTS];
    int p[FTS_MAX_POINTS];
    int id[FTS_MAX_POINTS];
    int count;
};


/*touch event info*/
struct ts_event
{
    u16 au16_x[FTS_MAX_POINTS];               /* x coordinate */
    u16 au16_y[FTS_MAX_POINTS];               /* y coordinate */
    u8 au8_touch_event[FTS_MAX_POINTS];       /* touch event: 0 -- down; 1-- up; 2 -- contact */
    u8 au8_finger_id[FTS_MAX_POINTS];         /* touch ID */
    u16 pressure[FTS_MAX_POINTS];
    u16 area[FTS_MAX_POINTS];
    u8 touch_point;
    int touchs;
    u8 touch_point_num;
};

/*****************************************************************************
* Static variables
*****************************************************************************/

/*****************************************************************************
* Global variable or extern global variabls/functions
*****************************************************************************/
extern struct input_dev *fts_input_dev;
extern struct tpd_device *tpd;
extern unsigned int tpd_rst_gpio_number;
extern struct i2c_client *fts_i2c_client;
/*[Arima_8100][allen_yu] Modify dynamic detect SM31_module and SM_32_module 20170719 begin*/
extern int chip_id;
enum IC_TYPE
{
	NO_VALUE,
	SM31_IC,
	SM32_IC
};
/*[Arima_8100][allen_yu] 20170719 end*/
#ifdef CONFIG_MTK_SENSOR_HUB_SUPPORT
void fts_sensor_suspend(struct i2c_client *i2c_client);
int fts_sensor_init(void);
void fts_sensor_enable(struct i2c_client *client);
ssize_t show_scp_ctrl(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t store_scp_ctrl(struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
#endif

#if FTS_PSENSOR_EN
int fts_proximity_init(struct i2c_client *client);
int fts_proximity_exit(struct i2c_client *client);
int fts_proximity_readdata(struct i2c_client *client);
int fts_proximity_suspend(void);
int fts_proximity_resume(void);
int fts_proximity_recovery(struct i2c_client *client);
#endif

#endif /* __LINUX_FOCALTECH_CORE_H__ */


