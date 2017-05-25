/*
 * drivers/battery/s2mu005_charger.h
 *
 * Header of S2MU005 Charger Driver
 *
 * Copyright (C) 2015 Samsung Electronics
 * Develope by Nguyen Tien Dat <tiendat.nt@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef S2MU005_CHARGER_H
#define S2MU005_CHARGER_H
#include <linux/mfd/samsung/s2mu005.h>
#include <linux/mfd/samsung/s2mu005-private.h>

#define MASK(width, shift)      (((0x1 << (width)) - 1) << shift)

#define S2MU005_CHG_STATUS0		0x08
#define S2MU005_CHG_STATUS1		0x09
#define S2MU005_CHG_STATUS2		0x0A
#define S2MU005_CHG_STATUS3		0x0B
#define S2MU005_CHG_STATUS4		0x0C
#define S2MU005_CHG_STATUS5		0x0D
#define S2MU005_CHG_CTRL0       0x0E
#define S2MU005_CHG_CTRL1       0x0F
#define S2MU005_CHG_CTRL2       0x10
#define S2MU005_CHG_CTRL3       0x11
#define S2MU005_CHG_CTRL4       0x12
#define S2MU005_CHG_CTRL5       0x13
#define S2MU005_CHG_CTRL6       0x14
#define S2MU005_CHG_CTRL7       0x15
#define S2MU005_CHG_CTRL8       0x16
#define S2MU005_CHG_CTRL9		0x17
#define S2MU005_CHG_CTRL10		0x18
#define S2MU005_CHG_CTRL11		0x19
#define S2MU005_CHG_CTRL12		0x1A
#define S2MU005_CHG_CTRL13		0x1B
#define S2MU005_CHG_CTRL14		0x1C
#define S2MU005_CHG_CTRL15		0x1D
#define S2MU005_CHG_CTRL16		0x1E
#define S2MU005_CHG_CTRL17		0x1F
#define S2MU005_CHG_CTRL18		0x20

/* S2MU005_SC_INT_MASK */
#define VMID_M_SHIFT	5
#define VMID_M_MASK		BIT(VMID_M_SHIFT)

/* S2MU005_CHG_STATUS0 */
#define VBUS_OK_SHIFT	7
#define VBUS_OK_MASK	BIT(VBUS_OK_SHIFT)

#define WCIN_OK_SHIFT	6
#define S2MU005_WCIN_OK_MASK	BIT(WCIN_OK_SHIFT)

#define VMID_OK_SHIFT	5
#define VMID_OK_MASK	BIT(VMID_OK_SHIFT)

#define CHG_OK_SHIFT	4
#define CHG_OK_MASK	BIT(CHG_OK_SHIFT)

#define CHG_STATUS_SHIFT	0
#define CHG_STATUS_WIDTH	4
#define CHG_STATUS_MASK		MASK(CHG_STATUS_WIDTH, CHG_STATUS_SHIFT)

#define CHG_STATUS_PRE_CHARGE	2
#define	CHG_STATUS_COOL_CHARGE	3
#define CHG_STATUS_CC		4
#define CHG_STATUS_CV		5
#define CHG_STATUS_DONE_FLAG	6
#define CHG_STATUS_TOP_OFF	7
#define CHG_STATUS_DONE		8

/* S2MU005_CHG_STATUS1 */
#define DET_BAT_STATUS_SHIFT	7
#define DET_BAT_STATUS_MASK	BIT(DET_BAT_STATUS_SHIFT)


/* S2MU005_CHG_STATUS2 */
#define BAT_STATUS_SHIFT	4
#define BAT_STATUS_WIDTH	3
#define BAT_STATUS_MASK		MASK(BAT_STATUS_WIDTH, BAT_STATUS_SHIFT)

#define BAT_OVP_DET		0
#define BAT_SELF_DISCHARGING	1
#define LOW_BATTERY_DETECTION_IN_CHARGING	2
#define	COOL_CHARGE_DET		5
#define FAST_CHARGE_DET		6
#define	BAT_VOL_DET		7

/* S2MU005_CHG_STATUS3 */
#define CHG_EVENT_STATUS_SHIFT	0
#define CHG_EVENT_STATUS_WIDTH	4
#define CHG_EVENT_STATUS_MASK	MASK(CHG_EVENT_STATUS_WIDTH, CHG_EVENT_STATUS_SHIFT)

#define THREMAL_SHUT_DOWN	1
#define THERMAL_FOLDBACK	2
#define VSYS_OVER_VOVP		3
#define VSYS_OVER_VUVLO		4
#define WATCHDOG_SUSPENSION	5
#define WATCHDOG_AP_RESET	6

/* S2MU005_CHG_CTRL0 */
#define EN_CHG_SHIFT		4
#define EN_CHG_MASK		BIT(EN_CHG_SHIFT)

#define REG_MODE_SHIFT		0
#define REG_MODE_WIDTH		3
#define REG_MODE_MASK		MASK(REG_MODE_WIDTH, REG_MODE_SHIFT)

/* S2MU005_CHG_CTRL1 */
#define SET_VIN_DROP_SHIFT	4
#define SET_VIN_DROP_WIDTH	3
#define SET_VIN_DROP_MASK	MASK(SET_VIN_DROP_WIDTH, SET_VIN_DROP_SHIFT)

/* S2MU005_CHG_CTRL2 */
#define INPUT_CURRENT_LIMIT_SHIFT	0
#define INPUT_CURRENT_LIMIT_WIDTH	6
#define INPUT_CURRENT_LIMIT_MASK	MASK(INPUT_CURRENT_LIMIT_WIDTH,\
					INPUT_CURRENT_LIMIT_SHIFT)

/* S2MU005_CHG_CTRL4 */
#define OTG_OCP_SW_ON_SHIFT		5
#define OTG_OCP_SW_ON_MASK		BIT(OTG_OCP_SW_ON_SHIFT)

#define OTG_OCP_SW_OFF_SHIFT	4
#define OTG_OCP_SW_OFF_MASK		BIT(OTG_OCP_SW_OFF_SHIFT)

#define SET_OTG_OCP_SHIFT	2
#define SET_OTG_OCP_WIDTH	2
#define SET_OTG_OCP_MASK	MASK(SET_OTG_OCP_WIDTH, SET_OTG_OCP_SHIFT)

/* S2MU005_CHG_CTRL5 */
#define SET_VF_VMID_BST_SHIFT	0
#define SET_VF_VMID_BST_WIDTH	5
#define SET_VF_VMID_BST_MASK	MASK(SET_VF_VMID_BST_WIDTH, SET_VF_VMID_BST_SHIFT)

/* S2MU005_CHG_CTRL7 */
#define FAST_CHARGING_CURRENT_SHIFT	0
#define FAST_CHARGING_CURRENT_WIDTH	6
#define FAST_CHARGING_CURRENT_MASK	MASK(FAST_CHARGING_CURRENT_WIDTH,\
					FAST_CHARGING_CURRENT_SHIFT)

/* S2MU005_CHG_CTRL8 */
#define SET_VF_VBAT_SHIFT	1
#define SET_VF_VBAT_WIDTH	6
#define SET_VF_VBAT_MASK	MASK(SET_VF_VBAT_WIDTH, SET_VF_VBAT_SHIFT)

/* S2MU005_CHG_CTRL10 */
#define SECOND_TOPOFF_CURRENT_SHIFT	4
#define SECOND_TOPOFF_CURRENT_WIDTH	4
#define SECOND_TOPOFF_CURRENT_MASK	MASK(SECOND_TOPOFF_CURRENT_WIDTH,\
					SECOND_TOPOFF_CURRENT_SHIFT)

#define FIRST_TOPOFF_CURRENT_SHIFT	0
#define FIRST_TOPOFF_CURRENT_WIDTH	4
#define FIRST_TOPOFF_CURRENT_MASK	MASK(FIRST_TOPOFF_CURRENT_WIDTH,\
					FIRST_TOPOFF_CURRENT_SHIFT)

/* S2MU005_CHG_CTRL11 */
#define SET_OSC_BST_SHIFT	5
#define SET_OSC_BST_WIDTH	2
#define SET_OSC_BST_MASK	MASK(SET_OSC_BST_WIDTH, SET_OSC_BST_SHIFT)

#define SET_OSC_BUCK_SHIFT	3
#define SET_OSC_BUCK_WIDTH	2
#define SET_OSC_BUCK_MASK	MASK(SET_OSC_BUCK_WIDTH, SET_OSC_BUCK_SHIFT)

/* S2MU005_CHG_CTRL15 */
#define T_EN_OTG_SHIFT		2
#define T_EN_OTG_WIDTH		2
#define T_EN_OTG_MASK		MASK(T_EN_OTG_WIDTH, T_EN_OTG_SHIFT)

enum {
	CHG_REG = 0,
	CHG_DATA,
	CHG_REGS,
};

struct charger_info {
	int dummy;
};

#endif /*S2MU005_CHARGER_H*/
