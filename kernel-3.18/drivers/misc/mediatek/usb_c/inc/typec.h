/*
 * Copyright (C) 2016 MediaTek Inc.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

/*
 * This is used to for host and peripheral modes of the tyoec driver.
 */

#ifndef USB_TYPEC_H
#define	USB_TYPEC_H

/* ConnSide */
#define DONT_CARE	0
#define UP_SIDE		1
#define DOWN_SIDE	2

/* Stat */
#define	DISABLE		0
#define	ENABLE		1

/* DriverType */
#define DEVICE_TYPE	1
#define HOST_TYPE	2

/* USBRdCtrlPin */
#define U3_EQ_C1	0
#define U3_EQ_C2	1

/* USBRdStat */
#define U3_EQ_LOW	0
#define U3_EQ_HZ	1
#define U3_EQ_HIGH	2

#ifdef CONFIG_USB_C_SWITCH_FUSB302
struct usb3_switch {
	int sel_gpio;
	int en_gpio;
	int sel;
	int en;
};
#endif

struct usb_redriver {
	int c1_gpio;
	int c2_gpio;
	int eq_c1;
	int eq_c2;
};

struct typec_switch_data {
	char *name;
	int type;
	int on;
	int (*enable)(void *);
	int (*disable)(void *);
	void *priv_data;
};

#ifdef CONFIG_USB_C_SWITCH_FUSB302
struct usbc_pin_ctrl {
	struct pinctrl_state *re_c1_init;
	struct pinctrl_state *re_c1_low;
	struct pinctrl_state *re_c1_hiz;
	struct pinctrl_state *re_c1_high;

	struct pinctrl_state *re_c2_init;
	struct pinctrl_state *re_c2_low;
	struct pinctrl_state *re_c2_hiz;
	struct pinctrl_state *re_c2_high;

	struct pinctrl_state *fusb340_oen_init;
	struct pinctrl_state *fusb340_oen_low;
	struct pinctrl_state *fusb340_oen_high;

	struct pinctrl_state *fusb340_sel_init;
	struct pinctrl_state *fusb340_sel_low;
	struct pinctrl_state *fusb340_sel_high;
};
#elif defined(CONFIG_USB_C_SWITCH_SII70XX)
struct usbc_pin_ctrl {
	struct pinctrl_state *re_c1_init;
	struct pinctrl_state *re_c2_init;
	struct pinctrl_state *sii7033_rst_init;
	struct pinctrl_state *sii7033_rst_low;
	struct pinctrl_state *sii7033_rst_high;
};
#elif defined(CONFIG_USB_C_SWITCH_ANX7418)
struct usbc_pin_ctrl {
	struct pinctrl_state *rst_n_init;
	struct pinctrl_state *rst_n_low;
	struct pinctrl_state *rst_n_high;

	struct pinctrl_state *pwr_en_init;
	struct pinctrl_state *pwr_en_low;
	struct pinctrl_state *pwr_en_high;

	struct pinctrl_state *cbl_det_init;
	struct pinctrl_state *intp_init;
};
/*[Arima_8103][bozhi_lin] TUSB320 TypeC driver porting 20170510 begin*/
#elif defined(CONFIG_USB_C_SWITCH_TUSB320)
struct usbc_pin_ctrl {
	struct pinctrl_state *tusb320_en_init;
	struct pinctrl_state *tusb320_en_low;
	struct pinctrl_state *tusb320_en_high;
};
/*[Arima_8103][bozhi_lin] 20170510 end*/
#endif

/*
 * struct usbtypc - Driver instance data.
 */
#ifdef CONFIG_USB_C_SWITCH_FUSB302
struct usbtypc {
	int irqnum;
	int en_irq;
#ifdef CONFIG_MTK_SIB_USB_SWITCH
	bool sib_enable;
#endif
	struct pinctrl *pinctrl;
	struct usbc_pin_ctrl *pin_cfg;
	spinlock_t	fsm_lock;
	struct delayed_work fsm_work;
	struct i2c_client *i2c_hd;
	struct hrtimer toggle_timer;
	struct hrtimer debounce_timer;
	struct typec_switch_data *host_driver;
	struct typec_switch_data *device_driver;
	struct usb3_switch *u3_sw;
	struct usb_redriver *u_rd;
};
#elif defined(CONFIG_USB_C_SWITCH_SII70XX)
struct usbtypc {
	int irqnum;
	int en_irq;
	struct pinctrl *pinctrl;
	struct usbc_pin_ctrl *pin_cfg;
	struct delayed_work eint_work;
	struct i2c_client *i2c_hd;
	struct typec_switch_data *host_driver;
	struct typec_switch_data *device_driver;
	struct usb_redriver *u_rd;
};
#elif defined(CONFIG_USB_C_SWITCH_ANX7418)
struct usbtypc {
	struct pinctrl *pinctrl;
	struct usbc_pin_ctrl *pin_cfg;
	struct device *pinctrl_dev;
	struct i2c_client *i2c_hd;
	struct typec_switch_data *host_driver;
	struct typec_switch_data *device_driver;
};
#elif defined(CONFIG_USB_C_SWITCH_MT6353)
struct usbtypc {
	struct typec_switch_data *host_driver;
	struct typec_switch_data *device_driver;
};
/*[Arima_8100][bozhi_lin] RichTek rt1711 usb type c driver porting 20161019 begin*/
#elif defined(CONFIG_TCPC_CLASS)
struct usbtypc {
	struct typec_switch_data *host_driver;
	struct typec_switch_data *device_driver;
#ifdef CONFIG_MTK_SIB_USB_SWITCH
	bool sib_enable;
#endif /* CONFIG_MTK_SIB_USB_SWITCH */
};
/*[Arima_8100][bozhi_lin] 20161019 end*/
/*[Arima_8103][bozhi_lin] TUSB320 TypeC driver porting 20170510 begin*/
#elif defined(CONFIG_USB_C_SWITCH_TUSB320)
struct usbtypc {
	struct typec_switch_data *host_driver;
	struct typec_switch_data *device_driver;
#ifdef CONFIG_MTK_SIB_USB_SWITCH
	bool sib_enable;
#endif /* CONFIG_MTK_SIB_USB_SWITCH */
	struct pinctrl *pinctrl;
	struct usbc_pin_ctrl *pin_cfg;
};
/*[Arima_8103][bozhi_lin] 20170510 end*/
#endif


extern int register_typec_switch_callback(struct typec_switch_data *new_driver);
extern int unregister_typec_switch_callback(struct typec_switch_data *new_driver);
extern int usb_redriver_config(struct usbtypc *typec, int ctrl_pin, int stat);
extern int usb_redriver_enter_dps(struct usbtypc *typec);
extern int usb_redriver_exit_dps(struct usbtypc *typec);
#if defined(CONFIG_USB_C_SWITCH_MT6353)
extern void typec_hanlder(void);
#endif

/*[Arima_8103][bozhi_lin] TUSB320 TypeC driver porting 20170510 begin*/
#if defined(CONFIG_USB_C_SWITCH_TUSB320)
enum typec_current_mode {
	TYPEC_CURRENT_MODE_DEFAULT = 0,
	TYPEC_CURRENT_MODE_MID,
	TYPEC_CURRENT_MODE_HIGH,
	TYPEC_CURRENT_MODE_UNSPPORTED
};
enum typec_attached_state {
	TYPEC_NOT_ATTACHED = 0,
	TYPEC_ATTACHED_AS_UFP,
	TYPEC_ATTACHED_AS_DFP,
	TYPEC_ATTACHED_TO_ACCESSORY
};
enum typec_port_mode {
	TYPEC_MODE_ACCORDING_TO_PROT = 0,
	TYPEC_UFP_MODE,
	TYPEC_DFP_MODE,
	TYPEC_DRP_MODE
};
enum typec_event {
	TYPEC_SINK_DETECTED = 0,
	TYPEC_SINK_REMOVED
};
struct typec_device_ops {
	/* to get the Type-C Current mode */
	enum typec_current_mode (*current_detect) (void);
	/* to get the Type-C Current mode dynamically */
	enum typec_current_mode (*dynamic_current_detect) (void);
	/* to get the attached state and determine what was attached */
	enum typec_attached_state (*attached_state_detect) (void);
	/* to get the current advertisement in DFP or DRP modes */
	enum typec_current_mode (*current_advertise_get) (void);
	/* to set the current advertisement in DFP or DRP modes */
	int (*current_advertise_set) (enum typec_current_mode current_mode);
	/* to get the port mode (UFP, DFP or DRP) */
	enum typec_port_mode (*port_mode_get) (void);
	/* to set the port mode (UFP, DFP or DRP), the chip will operate according the mode */
	int (*port_mode_set) (enum typec_port_mode port_mode);
	/* to get all the register value */
	 ssize_t(*dump_regs) (char *buf);
};

/*[Arima_8103][bozhi_lin] TUSB320 TypeC support pd charger and set current to 1.5A 20170511 begin*/
extern enum typec_current_mode tusb320_current_mode_detect(void);
/*[Arima_8103][bozhi_lin] 20170511 end*/
#endif
/*[Arima_8103][bozhi_lin] 20170510 end*/

#endif	/* USB_TYPEC_H */
