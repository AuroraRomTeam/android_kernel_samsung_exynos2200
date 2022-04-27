// SPDX-License-Identifier: GPL-2.0
/*
 * Samsung Exynos SoC series Pablo driver
 * Pablo v9.1 specific functions
 *
 * Copyright (c) 2020 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#if IS_ENABLED(CONFIG_EXYNOS_SCI)
#include <soc/samsung/exynos-sci.h>
#endif

#include "is-config.h"
#include "is-param.h"
#include "is-type.h"
#include "is-core.h"
#include "is-hw-chain.h"
#include "is-hw-settle-5nm-lpe.h"
#include "is-device-sensor.h"
#include "is-device-csi.h"
#include "is-device-ischain.h"

#include "../../interface/is-interface-ischain.h"
#include "is-hw.h"
#include "../../interface/is-interface-library.h"
#include "votf/camerapp-votf.h"
#include "pablo-device-iommu-group.h"

/* SYSREG register description */
/* SYSREG_CSIS REG, FIELD */
static const struct is_reg sysreg_csis_regs[SYSREG_CSIS_REG_CNT] = {
	{0x0108, "MEMCLK"},
	{0x0408, "CSIS_PDP_SC_CON0"},
	{0x040C, "CSIS_PDP_SC_CON1"},
	{0x0410, "CSIS_PDP_SC_CON3"},
	{0x0414, "CSIS_PDP_SC_CON4"},
	{0x0418, "CSIS_PDP_SC_CON5"},
	{0x041C, "CSIS_PDP_SC_CON6"},
	{0x0430, "CSIS_PDP_VC_CON0"},
	{0x0434, "CSIS_PDP_VC_CON1"},
	{0x0438, "CSIS_PDP_VC_CON2"},
	{0x043C, "CSIS_PDP_VC_CON3"},
	{0x0440, "CSIS_FRAME_ID_EN"},
	{0x0444, "CSIS_PDP_SC_PDP3_IN_EN"},
	{0x0448, "CSIS_PDP_SC_PDP2_IN_EN"},
	{0x044C, "CSIS_PDP_SC_PDP1_IN_EN"},
	{0x0450, "CSIS_PDP_SC_PDP0_IN_EN"},
	{0x0470, "MIPI_PHY_CON"},
	{0x0474, "MIPI_PHY_SEL"},
	{0x048C, "CSIS_PDP_SC_CON2"},
};

static const struct is_field sysreg_csis_fields[SYSREG_CSIS_REG_FIELD_CNT] = {
	{"EN", 0, 1, RW, 0x1},
	{"GLUEMUX_PDP0_VAL", 0, 4, RW, 0x0},
	{"GLUEMUX_PDP1_VAL", 0, 4, RW, 0x0},
	{"GLUEMUX_CSIS_DMA0_OTF_SEL", 0, 5, RW, 0x0},
	{"GLUEMUX_CSIS_DMA1_OTF_SEL", 0, 4, RW, 0x0},
	{"GLUEMUX_CSIS_DMA2_OTF_SEL", 0, 4, RW, 0x0},
	{"GLUEMUX_CSIS_DMA3_OTF_SEL", 0, 4, RW, 0x0},
	{"MUX_IMG_VC_PDP0", 16, 3, RW, 0x0},
	{"MUX_AF_VC_PDP0", 0, 3, RW, 0x1},
	{"MUX_IMG_VC_PDP1", 16, 3, RW, 0x0},
	{"MUX_AF_VC_PDP1", 0, 3, RW, 0x1},
	{"MUX_IMG_VC_PDP2", 16, 3, RW, 0x0},
	{"MUX_AF_VC_PDP2", 0, 3, RW, 0x1},
	{"FRAME_ID_EN_CSIS5", 5, 1, RW, 0x0},
	{"FRAME_ID_EN_CSIS4", 4, 1, RW, 0x0},
	{"FRAME_ID_EN_CSIS3", 3, 1, RW, 0x0},
	{"FRAME_ID_EN_CSIS2", 2, 1, RW, 0x0},
	{"FRAME_ID_EN_CSIS1", 1, 1, RW, 0x0},
	{"FRAME_ID_EN_CSIS0", 0, 1, RW, 0x0},
	{"PDP2_IN_CSIS5_EN", 5, 1, RW, 0x0},
	{"PDP2_IN_CSIS4_EN", 4, 1, RW, 0x0},
	{"PDP2_IN_CSIS3_EN", 3, 1, RW, 0x0},
	{"PDP2_IN_CSIS2_EN", 2, 1, RW, 0x0},
	{"PDP2_IN_CSIS1_EN", 1, 1, RW, 0x0},
	{"PDP2_IN_CSIS0_EN", 0, 1, RW, 0x0},
	{"PDP1_IN_CSIS5_EN", 5, 1, RW, 0x0},
	{"PDP1_IN_CSIS4_EN", 4, 1, RW, 0x0},
	{"PDP1_IN_CSIS3_EN", 3, 1, RW, 0x0},
	{"PDP1_IN_CSIS2_EN", 2, 1, RW, 0x0},
	{"PDP1_IN_CSIS1_EN", 1, 1, RW, 0x0},
	{"PDP1_IN_CSIS0_EN", 0, 1, RW, 0x0},
	{"PDP0_IN_CSIS5_EN", 5, 1, RW, 0x0},
	{"PDP0_IN_CSIS4_EN", 4, 1, RW, 0x0},
	{"PDP0_IN_CSIS3_EN", 3, 1, RW, 0x0},
	{"PDP0_IN_CSIS2_EN", 2, 1, RW, 0x0},
	{"PDP0_IN_CSIS1_EN", 1, 1, RW, 0x0},
	{"PDP0_IN_CSIS0_EN", 0, 1, RW, 0x0},
	{"MIPI_RESETN_DPHY_S3", 5, 1, RW, 0x0},
	{"MIPI_RESETN_DPHY_S2", 4, 1, RW, 0x0},
	{"MIPI_RESETN_DPHY_S1", 3, 1, RW, 0x0},
	{"MIPI_RESETN_DPHY_S", 2, 1, RW, 0x0},
	{"MIPI_RESETN_DCPHY_S1", 1, 1, RW, 0x0},
	{"MIPI_RESETN_DCPHY_S", 0, 1, RW, 0x0},
	{"MIPI_SEPARATION_SEL", 0, 3, RW, 0x0},
	{"GLUEMUX_PDP2_VAL", 0, 4, RW, 0x0},
};

/* SYSREG_TAA REG, FIELD */
static const struct is_reg sysreg_taa_regs[SYSREG_TAA_REG_CNT] = {
	{0X0108, "MEMCLK"},
	{0X0404, "TAA_USER_CON1"},
};

static const struct is_field sysreg_taa_fields[SYSREG_TAA_REG_FIELD_CNT] = {
	{"EN", 0, 1, RW, 0x1},	/* 0x108 */
	{"GLUEMUX_OTFOUT_SEL", 0, 2, RW, 0x0},  /* 0x404 */
};

/* SYSREG_TNR REG, FIELD */
static const struct is_reg sysreg_tnr_regs[SYSREG_TNR_REG_CNT] = {
	{0x0108, "MEMCLK"},
	{0x0400, "TNR_USER_CON0"},
};

static const struct is_field sysreg_tnr_fields[SYSREG_TNR_REG_FIELD_CNT] = {
	{"EN", 0, 1, RW, 0x1}, /* 0x108 */
	{"SW_RESETN_LHS_AST_GLUE_OTF1_TNRISP", 7, 1, RW, 0x1}, /* 0x400 */
	{"TYPE_LHS_AST_GLUE_OTF1_TNRISP", 5, 2, RW, 0x1},
	{"EN_OTF_IN_LH_AST_SI_OTF1_TNRISP", 4, 1, RW, 0x1},
	{"SW_RESETN_LHS_AST_GLUE_OTF0_TNRISP", 3, 1, RW, 0x1},
	{"TYPE_LHS_AST_GLUE_OTF0_TNRISP", 1, 2, RW, 0x1},
	{"EN_OTF_IN_LH_AST_SI_OTF0_TNRISP", 0, 1, RW, 0x1},
};

/* SYSREG_ISP REG, FIELD */
static const struct is_reg sysreg_isp_regs[SYSREG_ISP_REG_CNT] = {
	{0x0108, "MEMCLK"},
	{0x0414, "ISP_USER_CON3"},
};

static const struct is_field sysreg_isp_fields[SYSREG_ISP_REG_FIELD_CNT] = {
	{"EN", 0, 1, RW, 0x1}, /* 0x108 */
	{"OTF_SEL", 0, 1, RW, 0x1},
};

static const struct is_reg sysreg_mcsc_regs[SYSREG_MCSC_REG_CNT] = {
	{0x0108, "MEMCLK"},
	{0x0408, "MCSC_USER_CON2"},
};

static const struct is_field sysreg_mcsc_fields[SYSREG_MCSC_REG_FIELD_CNT] = {
	{"EN", 0, 1, RW, 0x1}, /* 0x108 */
	{"EN_OTF_IN_LH_AST_MI_OTF_ISPMCSC", 0, 1, RW, 0x1}, /* 0x408 */
};

static void __iomem *hwfc_rst;

static inline void __is_isr_host(void *data, int handler_id)
{
	struct is_interface_hwip *itf_hw = NULL;
	struct hwip_intr_handler *intr_hw = NULL;

	itf_hw = (struct is_interface_hwip *)data;
	intr_hw = &itf_hw->handler[handler_id];

	if (intr_hw->valid)
		intr_hw->handler(intr_hw->id, (void *)itf_hw->hw_ip);
	else
		err_itfc("[ID:%d] empty handler handler_id:%d!!", itf_hw->id, handler_id);
}

/* 3AA0 */
static irqreturn_t __is_isr1_3aa0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP1);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr2_3aa0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP2);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr3_3aa0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP3);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr4_3aa0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP4);
	return IRQ_HANDLED;
}

/* 3AA1 */
static irqreturn_t __is_isr1_3aa1(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP1);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr2_3aa1(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP2);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr3_3aa1(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP3);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr4_3aa1(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP4);
	return IRQ_HANDLED;
}

/* 3AA2 */
static irqreturn_t __is_isr1_3aa2(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP1);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr2_3aa2(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP2);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr3_3aa2(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP3);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr4_3aa2(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP4);
	return IRQ_HANDLED;
}

/* ITP0 */
static irqreturn_t __is_isr1_isp0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP1);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr2_isp0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP2);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr3_isp0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP3);
	return IRQ_HANDLED;
}

static irqreturn_t __is_isr4_isp0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP4);
	return IRQ_HANDLED;
}

/* ORBMCH */
static irqreturn_t __is_isr1_orbmch0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP1);
	return IRQ_HANDLED;
}

/* MCSC */
static irqreturn_t __is_isr1_mcs0(int irq, void *data)
{
	__is_isr_host(data, INTR_HWIP1);
	return IRQ_HANDLED;
}

/*
 * HW group related functions
 */
void __is_hw_group_init(struct is_group *group)
{
	int i;

	for (i = ENTRY_SENSOR; i < ENTRY_END; i++)
		group->subdev[i] = NULL;

	INIT_LIST_HEAD(&group->subdev_list);
}

int is_hw_group_cfg(void *group_data)
{
	int ret = 0;
	struct is_group *group;
	struct is_device_sensor *sensor;
	struct is_device_ischain *device;

	FIMC_BUG(!group_data);

	group = (struct is_group *)group_data;

#ifdef CONFIG_USE_SENSOR_GROUP
	if (group->slot == GROUP_SLOT_SENSOR) {
		sensor = group->sensor;
		if (!sensor) {
			err("device is NULL");
			BUG();
		}

		__is_hw_group_init(group);
		group->subdev[ENTRY_SENSOR] = &sensor->group_sensor.leader;
		group->subdev[ENTRY_SSVC0] = &sensor->ssvc0;
		group->subdev[ENTRY_SSVC1] = &sensor->ssvc1;
		group->subdev[ENTRY_SSVC2] = &sensor->ssvc2;
		group->subdev[ENTRY_SSVC3] = &sensor->ssvc3;

		list_add_tail(&sensor->group_sensor.leader.list, &group->subdev_list);
		list_add_tail(&sensor->ssvc0.list, &group->subdev_list);
		list_add_tail(&sensor->ssvc1.list, &group->subdev_list);
		list_add_tail(&sensor->ssvc2.list, &group->subdev_list);
		list_add_tail(&sensor->ssvc3.list, &group->subdev_list);
		return ret;
	}
#endif

	device = group->device;
	if (!device) {
		err("device is NULL");
		BUG();
	}

	group->gframe_skip = true;

	switch (group->slot) {
	case GROUP_SLOT_PAF:
		__is_hw_group_init(group);
		group->subdev[ENTRY_PAF] = &device->group_paf.leader;
		group->subdev[ENTRY_PDAF] = &device->pdaf;
		group->subdev[ENTRY_PDST] = &device->pdst;

		list_add_tail(&device->group_paf.leader.list, &group->subdev_list);
		list_add_tail(&device->pdaf.list, &group->subdev_list);
		list_add_tail(&device->pdst.list, &group->subdev_list);
		break;
	case GROUP_SLOT_3AA:
		__is_hw_group_init(group);
		group->subdev[ENTRY_3AA] = &device->group_3aa.leader;
		group->subdev[ENTRY_3AC] = &device->txc;
		group->subdev[ENTRY_3AP] = &device->txp;
		group->subdev[ENTRY_3AF] = &device->txf;
		group->subdev[ENTRY_3AG] = &device->txg;
		group->subdev[ENTRY_3AO] = &device->txo;
		group->subdev[ENTRY_MEXC] = &device->mexc;

		list_add_tail(&device->group_3aa.leader.list, &group->subdev_list);
		list_add_tail(&device->txc.list, &group->subdev_list);
		list_add_tail(&device->txp.list, &group->subdev_list);
		list_add_tail(&device->txf.list, &group->subdev_list);
		list_add_tail(&device->txg.list, &group->subdev_list);
		list_add_tail(&device->txo.list, &group->subdev_list);
		list_add_tail(&device->mexc.list, &group->subdev_list);

		device->txc.param_dma_ot = PARAM_3AA_VDMA4_OUTPUT;
		device->txp.param_dma_ot = PARAM_3AA_VDMA2_OUTPUT;
		device->txf.param_dma_ot = PARAM_3AA_FDDMA_OUTPUT;
		device->txg.param_dma_ot = PARAM_3AA_MRGDMA_OUTPUT;
		device->txo.param_dma_ot = PARAM_3AA_ORBDS_OUTPUT;
		break;
	case GROUP_SLOT_ORB:
		__is_hw_group_init(group);
		group->subdev[ENTRY_ORB] = &device->group_orb.leader;
		group->subdev[ENTRY_ORBXC] = &device->orbxc;

		list_add_tail(&device->group_orb.leader.list, &group->subdev_list);
		list_add_tail(&device->orbxc.list, &group->subdev_list);
		break;
	case GROUP_SLOT_ISP:
		__is_hw_group_init(group);
		group->subdev[ENTRY_ISP] = &device->group_isp.leader;
		group->subdev[ENTRY_IXC] = &device->ixc;
		group->subdev[ENTRY_IXP] = &device->ixp;
		group->subdev[ENTRY_IXT] = &device->ixt;
		group->subdev[ENTRY_IXG] = &device->ixg;
		group->subdev[ENTRY_IXV] = &device->ixv;
		group->subdev[ENTRY_IXW] = &device->ixw;

		list_add_tail(&device->group_isp.leader.list, &group->subdev_list);
		list_add_tail(&device->ixc.list, &group->subdev_list);
		list_add_tail(&device->ixp.list, &group->subdev_list);
		list_add_tail(&device->ixt.list, &group->subdev_list);
		list_add_tail(&device->ixg.list, &group->subdev_list);
		list_add_tail(&device->ixv.list, &group->subdev_list);
		list_add_tail(&device->ixw.list, &group->subdev_list);
		break;
	case GROUP_SLOT_MCS:
		__is_hw_group_init(group);
		group->subdev[ENTRY_MCS] = &device->group_mcs.leader;
		group->subdev[ENTRY_M0P] = &device->m0p;
		group->subdev[ENTRY_M1P] = &device->m1p;
		group->subdev[ENTRY_M2P] = &device->m2p;
		group->subdev[ENTRY_M3P] = &device->m3p;
		group->subdev[ENTRY_M4P] = &device->m4p;
		group->subdev[ENTRY_M5P] = &device->m5p;

		list_add_tail(&device->group_mcs.leader.list, &group->subdev_list);
		list_add_tail(&device->m0p.list, &group->subdev_list);
		list_add_tail(&device->m1p.list, &group->subdev_list);
		list_add_tail(&device->m2p.list, &group->subdev_list);
		list_add_tail(&device->m3p.list, &group->subdev_list);
		list_add_tail(&device->m4p.list, &group->subdev_list);
		list_add_tail(&device->m5p.list, &group->subdev_list);

		device->m0p.param_dma_ot = PARAM_MCS_OUTPUT0;
		device->m1p.param_dma_ot = PARAM_MCS_OUTPUT1;
		device->m2p.param_dma_ot = PARAM_MCS_OUTPUT2;
		device->m3p.param_dma_ot = PARAM_MCS_OUTPUT3;
		device->m4p.param_dma_ot = PARAM_MCS_OUTPUT4;
		device->m5p.param_dma_ot = PARAM_MCS_OUTPUT5;
		break;
	case GROUP_SLOT_LME:
	case GROUP_SLOT_YPP:
	case GROUP_SLOT_VRA:
	case GROUP_SLOT_CLH:
	case GROUP_SLOT_BYRP:
	case GROUP_SLOT_RGBP:
	case GROUP_SLOT_MCFP:
	case GROUP_SLOT_YUVP:
		break;
	default:
		probe_err("group slot(%d) is invalid", group->slot);
		BUG();
		break;
	}

	/* for hwfc: reset all REGION_IDX registers and outputs */
	hwfc_rst = ioremap(HWFC_INDEX_RESET_ADDR, SZ_4);

	return ret;
}

int is_hw_group_open(void *group_data)
{
	int ret = 0;
	u32 group_id;
	struct is_subdev *leader;
	struct is_group *group;
	struct is_device_ischain *device;

	FIMC_BUG(!group_data);

	group = group_data;
	leader = &group->leader;
	device = group->device;
	group_id = group->id;

	switch (group_id) {
#ifdef CONFIG_USE_SENSOR_GROUP
	case GROUP_ID_SS0:
	case GROUP_ID_SS1:
	case GROUP_ID_SS2:
	case GROUP_ID_SS3:
	case GROUP_ID_SS4:
	case GROUP_ID_SS5:
		leader->constraints_width = GROUP_SENSOR_MAX_WIDTH;
		leader->constraints_height = GROUP_SENSOR_MAX_HEIGHT;
		break;
#endif
	case GROUP_ID_PAF0:
	case GROUP_ID_PAF1:
	case GROUP_ID_PAF2:
		leader->constraints_width = GROUP_PDP_MAX_WIDTH;
		leader->constraints_height = GROUP_PDP_MAX_HEIGHT;
		break;
	case GROUP_ID_3AA0:
	case GROUP_ID_3AA1:
	case GROUP_ID_3AA2:
		leader->constraints_width = GROUP_3AA_MAX_WIDTH;
		leader->constraints_height = GROUP_3AA_MAX_HEIGHT;
		break;
	case GROUP_ID_ISP0:
	case GROUP_ID_MCS0:
		leader->constraints_width = GROUP_ISP_MAX_WIDTH;
		leader->constraints_height = GROUP_ISP_MAX_HEIGHT;
		break;
	case GROUP_ID_ORB0:
		leader->constraints_width = GROUP_ORBMCH_MAX_WIDTH;
		leader->constraints_height = GROUP_ORBMCH_MAX_HEIGHT;
		break;
	default:
		merr("(%s) is invalid", group, group_id_name[group_id]);
		break;
	}

	return ret;
}

inline int is_hw_slot_id(int hw_id)
{
	int slot_id = -1;

	switch (hw_id) {
	case DEV_HW_PAF0:
		slot_id = 0;
		break;
	case DEV_HW_PAF1:
		slot_id = 1;
		break;
	case DEV_HW_PAF2:
		slot_id = 2;
		break;
	case DEV_HW_3AA0:
		slot_id = 3;
		break;
	case DEV_HW_3AA1:
		slot_id = 4;
		break;
	case DEV_HW_3AA2:
		slot_id = 5;
		break;
	case DEV_HW_ORB0:
		slot_id = 6;
		break;
	case DEV_HW_ISP0:
		slot_id = 7;
		break;
	case DEV_HW_MCSC0:
		slot_id = 8;
		break;
	case DEV_HW_PAF3:
	case DEV_HW_3AA3:
	case DEV_HW_LME0:
	case DEV_HW_LME1:
	case DEV_HW_ISP1:
	case DEV_HW_YPP:
	case DEV_HW_MCSC1:
	case DEV_HW_VRA:
	case DEV_HW_CLH0:
		break;
	default:
		err("Invalid hw id(%d)", hw_id);
		break;
	}

	return slot_id;
}

int is_get_hw_list(int group_id, int *hw_list)
{
	int i;
	int hw_index = 0;

	/* initialization */
	for (i = 0; i < GROUP_HW_MAX; i++)
		hw_list[i] = -1;

	switch (group_id) {
	case GROUP_ID_PAF0:
		hw_list[hw_index] = DEV_HW_PAF0; hw_index++;
		break;
	case GROUP_ID_PAF1:
		hw_list[hw_index] = DEV_HW_PAF1; hw_index++;
		break;
	case GROUP_ID_PAF2:
		hw_list[hw_index] = DEV_HW_PAF2; hw_index++;
		break;
	case GROUP_ID_3AA0:
		hw_list[hw_index] = DEV_HW_3AA0; hw_index++;
		break;
	case GROUP_ID_3AA1:
		hw_list[hw_index] = DEV_HW_3AA1; hw_index++;
		break;
	case GROUP_ID_3AA2:
		hw_list[hw_index] = DEV_HW_3AA2; hw_index++;
		break;
	case GROUP_ID_ORB0:
		hw_list[hw_index] = DEV_HW_ORB0; hw_index++;
		break;
	case GROUP_ID_ISP0:
		hw_list[hw_index] = DEV_HW_ISP0; hw_index++;
		break;
	case GROUP_ID_MCS0:
		hw_list[hw_index] = DEV_HW_MCSC0; hw_index++;
		break;
	case GROUP_ID_MAX:
		break;
	default:
		err("Invalid group%d(%s)", group_id, group_id_name[group_id]);
		break;
	}

	return hw_index;
}
/*
 * System registers configurations
 */
static int is_hw_get_clk_gate(struct is_hw_ip *hw_ip, int hw_id)
{
	if (!hw_ip) {
		probe_err("hw_id(%d) hw_ip(NULL)", hw_id);
		return -EINVAL;
	}

	hw_ip->clk_gate_idx = 0;
	hw_ip->clk_gate = NULL;

	return 0;
}

static inline int __is_hw_get_address(struct platform_device *pdev,
				struct is_interface_hwip *itf_hwip,
				int hw_id, char *hw_name,
				u32 resource_id, enum base_reg_index reg_index,
				bool alloc_memlog)
{
	struct resource *mem_res = NULL;

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, resource_id);
	if (!mem_res) {
		dev_err(&pdev->dev, "Failed to get io memory region\n");
		return -EINVAL;
	}

	itf_hwip->hw_ip->regs_start[reg_index] = mem_res->start;
	itf_hwip->hw_ip->regs_end[reg_index] = mem_res->end;
	itf_hwip->hw_ip->regs[reg_index] =
		ioremap(mem_res->start, resource_size(mem_res));
	if (!itf_hwip->hw_ip->regs[reg_index]) {
		dev_err(&pdev->dev, "Failed to remap io region\n");
		return -EINVAL;
	}

	if (alloc_memlog)
		is_debug_memlog_alloc_dump(mem_res->start,
					resource_size(mem_res), hw_name);

	info_itfc("[ID:%2d] %s VA(0x%lx)\n", hw_id, hw_name,
		(ulong)itf_hwip->hw_ip->regs[reg_index]);

	return 0;
}

int is_hw_get_address(void *itfc_data, void *pdev_data, int hw_id)
{
	int ret = 0;
	struct platform_device *pdev = NULL;
	struct is_interface_hwip *itf_hwip = NULL;
	int idx;

	FIMC_BUG(!itfc_data);
	FIMC_BUG(!pdev_data);

	itf_hwip = (struct is_interface_hwip *)itfc_data;
	pdev = (struct platform_device *)pdev_data;

	switch (hw_id) {
	case DEV_HW_3AA0:
		__is_hw_get_address(pdev, itf_hwip, hw_id, "3AA0", IORESOURCE_3AA0, REG_SETA, false);
		/* TODO: need check if exist dump_region */
		idx = 0;
		itf_hwip->hw_ip->dump_region[REG_SETA][idx].start = 0x0;
		itf_hwip->hw_ip->dump_region[REG_SETA][idx++].end = 0x1FAF;
		itf_hwip->hw_ip->dump_region[REG_SETA][idx].start = 0x1FC0;
		itf_hwip->hw_ip->dump_region[REG_SETA][idx++].end = 0x9FAF;
		itf_hwip->hw_ip->dump_region[REG_SETA][idx].start = 0x9FC0;
		itf_hwip->hw_ip->dump_region[REG_SETA][idx++].end = 0xFFFF;

		__is_hw_get_address(pdev, itf_hwip, hw_id, "3AA0", IORESOURCE_3AA0, REG_EXT1, false);
		__is_hw_get_address(pdev, itf_hwip, hw_id, "ZSL0 DMA", IORESOURCE_ZSL0_DMA, REG_EXT2, false);
		__is_hw_get_address(pdev, itf_hwip, hw_id, "STRP0 DMA", IORESOURCE_STRP0_DMA, REG_EXT3, false);
		break;
	case DEV_HW_3AA1:
		__is_hw_get_address(pdev, itf_hwip, hw_id, "3AA1", IORESOURCE_3AA1, REG_SETA, false);
		__is_hw_get_address(pdev, itf_hwip, hw_id, "3AA0", IORESOURCE_3AA0, REG_EXT1, false);
		__is_hw_get_address(pdev, itf_hwip, hw_id, "ZSL1 DMA", IORESOURCE_ZSL1_DMA, REG_EXT2, false);
		__is_hw_get_address(pdev, itf_hwip, hw_id, "STRP1 DMA", IORESOURCE_STRP1_DMA, REG_EXT3, false);
		break;
	case DEV_HW_3AA2:
		__is_hw_get_address(pdev, itf_hwip, hw_id, "3AA2", IORESOURCE_3AA2, REG_SETA, false);
		__is_hw_get_address(pdev, itf_hwip, hw_id, "3AA0", IORESOURCE_3AA0, REG_EXT1, false);
		__is_hw_get_address(pdev, itf_hwip, hw_id, "ZSL2 DMA", IORESOURCE_ZSL2_DMA, REG_EXT2, false);
		__is_hw_get_address(pdev, itf_hwip, hw_id, "STRP2 DMA", IORESOURCE_STRP2_DMA, REG_EXT3, false);
		break;
	case DEV_HW_ORB0:
		__is_hw_get_address(pdev, itf_hwip, hw_id, "ORBMCH", IORESOURCE_ORBMCH0, REG_SETA, false);
		break;
	case DEV_HW_ISP0:
		__is_hw_get_address(pdev, itf_hwip, hw_id, "ITP", IORESOURCE_ITP, REG_SETA, true);

		__is_hw_get_address(pdev, itf_hwip, hw_id, "MCFP0", IORESOURCE_MCFP0, REG_EXT1, false);

		idx = 0;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x0;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x03FF;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x0500;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x05FF;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x0800;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x09FF;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x1800;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x1AFF;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x2000;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x23FF;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x2600;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x2BFF;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x3800;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x39FF;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x4000;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x4DFF;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx].start = 0x5200;
		itf_hwip->hw_ip->dump_region[REG_EXT1][idx++].end = 0x55FF;

		__is_hw_get_address(pdev, itf_hwip, hw_id, "DNS", IORESOURCE_DNS, REG_EXT2, true);

		__is_hw_get_address(pdev, itf_hwip, hw_id, "MCFP1", IORESOURCE_MCFP1, REG_EXT3, false);

		idx = 0;
		itf_hwip->hw_ip->dump_region[REG_EXT3][idx].start = 0x0;
		itf_hwip->hw_ip->dump_region[REG_EXT3][idx++].end = 0x0FFF;
		itf_hwip->hw_ip->dump_region[REG_EXT3][idx].start = 0x2900;
		itf_hwip->hw_ip->dump_region[REG_EXT3][idx++].end = 0x3CFF;
		break;
	case DEV_HW_MCSC0:
		__is_hw_get_address(pdev, itf_hwip, hw_id, "MCSC0", IORESOURCE_MCSC, REG_SETA, true);
		break;
	default:
		probe_err("hw_id(%d) is invalid", hw_id);
		return -EINVAL;
	}

	ret = is_hw_get_clk_gate(itf_hwip->hw_ip, hw_id);
	if (ret)
		dev_err(&pdev->dev, "is_hw_get_clk_gate is fail\n");

	return ret;
}

int is_hw_get_irq(void *itfc_data, void *pdev_data, int hw_id)
{
	struct is_interface_hwip *itf_hwip = NULL;
	struct platform_device *pdev = NULL;
	int ret = 0;

	FIMC_BUG(!itfc_data);

	itf_hwip = (struct is_interface_hwip *)itfc_data;
	pdev = (struct platform_device *)pdev_data;

	switch (hw_id) {
	case DEV_HW_3AA0:
		itf_hwip->irq[INTR_HWIP1] = platform_get_irq(pdev, 0);
		if (itf_hwip->irq[INTR_HWIP1] < 0) {
			err("Failed to get irq 3aa0-1\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP2] = platform_get_irq(pdev, 1);
		if (itf_hwip->irq[INTR_HWIP2] < 0) {
			err("Failed to get irq 3aa0-2\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP3] = platform_get_irq(pdev, 2);
		if (itf_hwip->irq[INTR_HWIP3] < 0) {
			err("Failed to get irq 3aa0 zsl dma\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP4] = platform_get_irq(pdev, 3);
		if (itf_hwip->irq[INTR_HWIP4] < 0) {
			err("Failed to get irq 3aa0 strp dma\n");
			return -EINVAL;
		}
		break;
	case DEV_HW_3AA1:
		itf_hwip->irq[INTR_HWIP1] = platform_get_irq(pdev, 4);
		if (itf_hwip->irq[INTR_HWIP1] < 0) {
			err("Failed to get irq 3aa1-1\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP2] = platform_get_irq(pdev, 5);
		if (itf_hwip->irq[INTR_HWIP2] < 0) {
			err("Failed to get irq 3aa1-2\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP3] = platform_get_irq(pdev, 6);
		if (itf_hwip->irq[INTR_HWIP3] < 0) {
			err("Failed to get irq 3aa1 zsl dma\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP4] = platform_get_irq(pdev, 7);
		if (itf_hwip->irq[INTR_HWIP4] < 0) {
			err("Failed to get irq 3aa1 strp dma\n");
			return -EINVAL;
		}
		break;
	case DEV_HW_3AA2:
		itf_hwip->irq[INTR_HWIP1] = platform_get_irq(pdev, 8);
		if (itf_hwip->irq[INTR_HWIP1] < 0) {
			err("Failed to get irq 3aa2-1\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP2] = platform_get_irq(pdev, 9);
		if (itf_hwip->irq[INTR_HWIP2] < 0) {
			err("Failed to get irq 3aa2-2\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP3] = platform_get_irq(pdev, 10);
		if (itf_hwip->irq[INTR_HWIP3] < 0) {
			err("Failed to get irq 3aa2 zsl dma\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP4] = platform_get_irq(pdev, 11);
		if (itf_hwip->irq[INTR_HWIP4] < 0) {
			err("Failed to get irq 3aa2 strp dma\n");
			return -EINVAL;
		}
		break;
	case DEV_HW_ORB0:
		itf_hwip->irq[INTR_HWIP1] = platform_get_irq(pdev, 12);
		if (itf_hwip->irq[INTR_HWIP1] < 0) {
			err("Failed to get irq ORBMCH0-1\n");
			return -EINVAL;
		}
		break;
	case DEV_HW_ISP0:
		itf_hwip->irq[INTR_HWIP1] = platform_get_irq(pdev, 13);
		if (itf_hwip->irq[INTR_HWIP1] < 0) {
			err("Failed to get irq isp0-1\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP2] = platform_get_irq(pdev, 14);
		if (itf_hwip->irq[INTR_HWIP2] < 0) {
			err("Failed to get irq isp0-2\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP3] = platform_get_irq(pdev, 15);
		if (itf_hwip->irq[INTR_HWIP3] < 0) {
			err("Failed to get irq tnr0\n");
			return -EINVAL;
		}

		itf_hwip->irq[INTR_HWIP4] = platform_get_irq(pdev, 16);
		if (itf_hwip->irq[INTR_HWIP4] < 0) {
			err("Failed to get irq tnr1\n");
			return -EINVAL;
		}
		break;
	case DEV_HW_MCSC0:
		itf_hwip->irq[INTR_HWIP1] = platform_get_irq(pdev, 17);
		if (itf_hwip->irq[INTR_HWIP1] < 0) {
			err("Failed to get irq mcsc0\n");
			return -EINVAL;
		}
		break;
	default:
		probe_err("hw_id(%d) is invalid", hw_id);
		return -EINVAL;
	}

	return ret;
}

static inline int __is_hw_request_irq(struct is_interface_hwip *itf_hwip,
	const char *name, int isr_num,
	unsigned int added_irq_flags,
	irqreturn_t (*func)(int, void *))
{
	size_t name_len = 0;
	int ret = 0;

	name_len = sizeof(itf_hwip->irq_name[isr_num]);
	snprintf(itf_hwip->irq_name[isr_num], name_len, "%s-%d", name, isr_num);

	ret = is_request_irq(itf_hwip->irq[isr_num], func,
		itf_hwip->irq_name[isr_num],
		added_irq_flags,
		itf_hwip);
	if (ret) {
		err_itfc("[HW:%s] request_irq [%d] fail", name, isr_num);
		return -EINVAL;
	}

	itf_hwip->handler[isr_num].id = isr_num;
	itf_hwip->handler[isr_num].valid = true;

	return ret;
}

static inline int __is_hw_free_irq(struct is_interface_hwip *itf_hwip, int isr_num)
{
	is_free_irq(itf_hwip->irq[isr_num], itf_hwip);

	return 0;
}

int is_hw_request_irq(void *itfc_data, int hw_id)
{
	struct is_interface_hwip *itf_hwip = NULL;
	int ret = 0;

	FIMC_BUG(!itfc_data);


	itf_hwip = (struct is_interface_hwip *)itfc_data;

	switch (hw_id) {
	case DEV_HW_3AA0:
		ret = __is_hw_request_irq(itf_hwip, "3a0-0", INTR_HWIP1, IRQF_TRIGGER_NONE, __is_isr1_3aa0);
		ret = __is_hw_request_irq(itf_hwip, "3a0-1", INTR_HWIP2, IRQF_TRIGGER_NONE, __is_isr2_3aa0);
		ret = __is_hw_request_irq(itf_hwip, "3a0-zsl", INTR_HWIP3, IRQF_TRIGGER_NONE, __is_isr3_3aa0);
		ret = __is_hw_request_irq(itf_hwip, "3a0-strp", INTR_HWIP4, IRQF_SHARED, __is_isr4_3aa0);
		break;
	case DEV_HW_3AA1:
		ret = __is_hw_request_irq(itf_hwip, "3a1-0", INTR_HWIP1, IRQF_TRIGGER_NONE, __is_isr1_3aa1);
		ret = __is_hw_request_irq(itf_hwip, "3a1-1", INTR_HWIP2, IRQF_TRIGGER_NONE, __is_isr2_3aa1);
		ret = __is_hw_request_irq(itf_hwip, "3a1-zsl", INTR_HWIP3, IRQF_TRIGGER_NONE, __is_isr3_3aa1);
		ret = __is_hw_request_irq(itf_hwip, "3a1-strp", INTR_HWIP4, IRQF_TRIGGER_NONE, __is_isr4_3aa1);
		break;
	case DEV_HW_3AA2:
		ret = __is_hw_request_irq(itf_hwip, "3a2-0", INTR_HWIP1, IRQF_TRIGGER_NONE, __is_isr1_3aa2);
		ret = __is_hw_request_irq(itf_hwip, "3a2-1", INTR_HWIP2, IRQF_TRIGGER_NONE, __is_isr2_3aa2);
		ret = __is_hw_request_irq(itf_hwip, "3a2-zsl", INTR_HWIP3, IRQF_TRIGGER_NONE, __is_isr3_3aa2);
		ret = __is_hw_request_irq(itf_hwip, "3a2-strp", INTR_HWIP4, IRQF_TRIGGER_NONE, __is_isr4_3aa2);
		break;
	case DEV_HW_ORB0:
		/* To apply ORBMCH SW W/A, irq request for ORB was moved after power on */
		if (!IS_ENABLED(USE_ORBMCH_WA))
			ret = __is_hw_request_irq(itf_hwip, "orbmch0", INTR_HWIP1, IRQF_TRIGGER_NONE, __is_isr1_orbmch0);
		break;
	case DEV_HW_ISP0:
		ret = __is_hw_request_irq(itf_hwip, "dns-0", INTR_HWIP1, IRQF_TRIGGER_NONE, __is_isr1_isp0);
		ret = __is_hw_request_irq(itf_hwip, "dns-1", INTR_HWIP2, IRQF_TRIGGER_NONE, __is_isr2_isp0);
		ret = __is_hw_request_irq(itf_hwip, "tnr-0", INTR_HWIP3, IRQF_TRIGGER_NONE, __is_isr3_isp0);
		ret = __is_hw_request_irq(itf_hwip, "tnr-1", INTR_HWIP4, IRQF_TRIGGER_NONE, __is_isr4_isp0);
		break;
	case DEV_HW_MCSC0:
		ret = __is_hw_request_irq(itf_hwip, "mcs0", INTR_HWIP1, IRQF_TRIGGER_NONE, __is_isr1_mcs0);
		break;
	default:
		probe_err("hw_id(%d) is invalid", hw_id);
		return -EINVAL;
	}

	return ret;
}

int is_hw_s_ctrl(void *itfc_data, int hw_id, enum hw_s_ctrl_id id, void *val)
{
	int ret = 0;

	switch (id) {
	case HW_S_CTRL_FULL_BYPASS:
		break;
	case HW_S_CTRL_CHAIN_IRQ:
		break;
	case HW_S_CTRL_HWFC_IDX_RESET:
		if (hw_id == IS_VIDEO_M2P_NUM) {
			struct is_video_ctx *vctx = (struct is_video_ctx *)itfc_data;
			struct is_device_ischain *device;
			unsigned long data = (unsigned long)val;

			FIMC_BUG(!vctx);
			FIMC_BUG(!GET_DEVICE(vctx));

			device = GET_DEVICE(vctx);

			/* reset if this instance is reprocessing */
			if (test_bit(IS_ISCHAIN_REPROCESSING, &device->state))
				writel(data, hwfc_rst);
		}
		break;
	case HW_S_CTRL_MCSC_SET_INPUT:
		{
			unsigned long mode = (unsigned long)val;

			info_itfc("%s: mode(%lu)\n", __func__, mode);
		}
		break;
	default:
		break;
	}

	return ret;
}

int is_hw_g_ctrl(void *itfc_data, int hw_id, enum hw_g_ctrl_id id, void *val)
{
	int ret = 0;

	switch (id) {
	case HW_G_CTRL_FRM_DONE_WITH_DMA:
		if (hw_id == DEV_HW_YPP)
			*(bool *)val = false;
		else
			*(bool *)val = true;
		break;
	case HW_G_CTRL_HAS_MCSC:
		*(bool *)val = true;
		break;
	default:
		break;
	}

	return ret;
}

int is_hw_query_cap(void *cap_data, int hw_id)
{
	int ret = 0;

	FIMC_BUG(!cap_data);

	switch (hw_id) {
	case DEV_HW_MCSC0:
		{
			struct is_hw_mcsc_cap *cap = (struct is_hw_mcsc_cap *)cap_data;

			cap->hw_ver = HW_SET_VERSION(10, 0, 20, 0);
			cap->max_output = 3;
			cap->max_djag = 1;
			cap->max_cac = 0;
			cap->max_uvsp = 0;
			cap->hwfc = MCSC_CAP_SUPPORT;
			cap->in_otf = MCSC_CAP_SUPPORT;
			cap->in_dma = MCSC_CAP_SUPPORT;
			cap->out_dma[0] = MCSC_CAP_SUPPORT;
			cap->out_dma[1] = MCSC_CAP_SUPPORT;
			cap->out_dma[2] = MCSC_CAP_SUPPORT;
			cap->out_dma[3] = MCSC_CAP_NOT_SUPPORT;
			cap->out_dma[4] = MCSC_CAP_NOT_SUPPORT;
			cap->out_dma[5] = MCSC_CAP_NOT_SUPPORT;
			cap->out_otf[0] = MCSC_CAP_NOT_SUPPORT;
			cap->out_otf[1] = MCSC_CAP_NOT_SUPPORT;
			cap->out_otf[2] = MCSC_CAP_NOT_SUPPORT;
			cap->out_otf[3] = MCSC_CAP_NOT_SUPPORT;
			cap->out_otf[4] = MCSC_CAP_NOT_SUPPORT;
			cap->out_otf[5] = MCSC_CAP_NOT_SUPPORT;
			cap->out_hwfc[1] = MCSC_CAP_SUPPORT;
			cap->out_hwfc[2] = MCSC_CAP_SUPPORT;
			cap->out_post[0] = MCSC_CAP_SUPPORT;
			cap->out_post[1] = MCSC_CAP_SUPPORT;
			cap->out_post[2] = MCSC_CAP_SUPPORT;
			cap->out_post[3] = MCSC_CAP_NOT_SUPPORT;
			cap->out_post[4] = MCSC_CAP_NOT_SUPPORT;
			cap->out_post[5] = MCSC_CAP_NOT_SUPPORT;
			cap->enable_shared_output = false;
			cap->tdnr = MCSC_CAP_NOT_SUPPORT;
			cap->djag = MCSC_CAP_SUPPORT;
			cap->cac = MCSC_CAP_NOT_SUPPORT;
			cap->uvsp = MCSC_CAP_NOT_SUPPORT;
			cap->ysum = MCSC_CAP_NOT_SUPPORT;
			cap->ds_vra = MCSC_CAP_NOT_SUPPORT;
		}
		break;
	case DEV_HW_MCSC1:
		break;
	default:
		break;
	}

	return ret;
}

void __iomem *is_hw_get_sysreg(ulong core_regs)
{
	if (core_regs)
		err_itfc("%s: core_regs(%p)\n", __func__, (void *)core_regs);

	/* deprecated */

	return NULL;
}

u32 is_hw_find_settle(u32 mipi_speed, u32 use_cphy)
{
	u32 align_mipi_speed;
	u32 find_mipi_speed;
	const u32 *settle_table;
	size_t max;
	int s, e, m;

	if (use_cphy) {
		settle_table = is_csi_settle_table_cphy;
		max = sizeof(is_csi_settle_table_cphy) / sizeof(u32);
	} else {
		settle_table = is_csi_settle_table;
		max = sizeof(is_csi_settle_table) / sizeof(u32);
	}
	align_mipi_speed = ALIGN(mipi_speed, 10);

	s = 0;
	e = max - 2;

	if (settle_table[s] < align_mipi_speed)
		return settle_table[s + 1];

	if (settle_table[e] > align_mipi_speed)
		return settle_table[e + 1];

	/* Binary search */
	while (s <= e) {
		m = ALIGN((s + e) / 2, 2);
		find_mipi_speed = settle_table[m];

		if (find_mipi_speed == align_mipi_speed)
			break;
		else if (find_mipi_speed > align_mipi_speed)
			s = m + 2;
		else
			e = m - 2;
	}

	return settle_table[m + 1];
}

unsigned int get_dma(struct is_device_sensor *device, u32 *dma_ch)
{
	*dma_ch = 0;

	return 0;
}

void is_hw_camif_init(void)
{
	/* TODO */
}

#ifdef USE_CAMIF_FIX_UP
int is_hw_camif_fix_up(struct is_device_sensor *sensor)
{
	int ret = 0;

	/* TODO */

	return ret;
}
#endif

#if (IS_ENABLED(CONFIG_CAMERA_CIS_ZEBU_OBJ))
static void is_hw_tzpc_info(void)
{
	void __iomem *reg;
	int i, j;

	/* CSIS WDMA */
	reg = ioremap(0x15090080, 0x20);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);

	/* PDP DMA */
	reg = ioremap(0x150C0210, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);
	reg = ioremap(0x150D0210, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);
	reg = ioremap(0x150E0210, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);

	/* 3AA DMA */
	reg = ioremap(0x15543b10, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);
	reg = ioremap(0x15553b10, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);
	reg = ioremap(0x15563b10, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);

	/* ORB DMA */
	reg = ioremap(0x15750050, 0x10);
	for (i = 0, j = 0; i < 4; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);
	reg = ioremap(0x15750700, 0x10);
	for (i = 0, j = 0; i < 4; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);

	/* TNR DMA */
	reg = ioremap(0x14B00810, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);
	reg = ioremap(0x14B40810, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);

	/* DNS/ITP DMA */
	reg = ioremap(0x15434010, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);

	/* MCSC DMA */
	reg = ioremap(0x156401A0, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);
	/* GDC DMA */
	reg = ioremap(0x15660610, 0x30);
	for (i = 0, j = 0; i < 8; j = i * 0x04, i++)
		writel(0xFFFFFFFF, reg + j);
	iounmap(reg);
}

static void is_hw_s2mpu_cfg(void)
{
	void __iomem *reg;

	/* for CSIS */
	reg = ioremap(0x15120000, 0x4);
	writel(0, reg);
	iounmap(reg);

	reg = ioremap(0x15150000, 0x4);
	writel(0, reg);
	iounmap(reg);

	reg = ioremap(0x15180000, 0x4);
	writel(0, reg);
	iounmap(reg);

	reg = ioremap(0x151B0000, 0x4);
	writel(0, reg);
	iounmap(reg);

	/* for TAA */
	reg = ioremap(0x15590000, 0x4);
	writel(0, reg);
	iounmap(reg);

	/* for TNR */
	reg = ioremap(0x153C0000, 0x4);
	writel(0, reg);
	iounmap(reg);

	reg = ioremap(0x153F0000, 0x4);
	writel(0, reg);
	iounmap(reg);

	/* for ISP */
	reg = ioremap(0x154A0000, 0x4);
	writel(0, reg);
	iounmap(reg);

	/* for MCSC */
	reg = ioremap(0x156A0000, 0x4);
	writel(0, reg);
	iounmap(reg);

	reg = ioremap(0x156D0000, 0x4);
	writel(0, reg);
	iounmap(reg);
}
#endif

int is_hw_camif_cfg(void *sensor_data)
{
	int ret = 0;
	int i;
	void __iomem *csis_sys_regs;
	struct is_core *core;
	struct is_device_sensor *sensor;
	struct is_device_csi *csi;
	bool csi_f_enabled = false;
	bool csi_e_enabled = false;
	u32 sensor_open_cnt = 1;

	FIMC_BUG(!sensor_data);

	sensor = (struct is_device_sensor *)sensor_data;

	core = (struct is_core *)sensor->private_data;
	if (!core) {
		merr("core is null\n", sensor);
		ret = -ENODEV;
		return ret;
	}

	csi = (struct is_device_csi *)v4l2_get_subdevdata(sensor->subdev_csi);
	if (!csi) {
		merr("csi is null\n", sensor);
		ret = -ENODEV;
		return ret;
	}

	csis_sys_regs = ioremap(SYSREG_CSIS_BASE_ADDR, 0x1000);

	/* When more than one is opened */
	for (i = 0; i < IS_SENSOR_COUNT; i++) {
		if (test_bit(IS_SENSOR_OPEN, &(core->sensor[i].state))
				&& core->sensor[i].device_id != sensor->device_id) {
			csi = (struct is_device_csi *)v4l2_get_subdevdata(core->sensor[i].subdev_csi);
			if (!csi) {
				merr("csi is null\n", sensor);
				ret = -ENODEV;
				iounmap(csis_sys_regs);
				return ret;
			}
			sensor_open_cnt++;

			if (csi->ch == CSI_ID_E) {
				info("remain mipi phy mux val for CSI_E");
				csi_e_enabled = true;
			} else if (csi->ch == CSI_ID_F) {
				info("remain mipi phy mux val for CSI_F");
				csi_f_enabled = true;
			}
		}
	}

	/* When only one is open */
	if (sensor_open_cnt == 1 && test_bit(IS_SENSOR_OPEN, &sensor->state)) {
		if (csi->ch == CSI_ID_E) {
			info("set mipi phy mux val for CSI_E");
			csi_e_enabled = true;
		} else if (csi->ch == CSI_ID_F) {
			info("set mipi phy mux val for CSI_F");
			csi_f_enabled = true;
		}
	}

	if (csi_e_enabled && csi_f_enabled)		/* 2 + 1 + 1 */
		is_hw_set_field(csis_sys_regs,
			&sysreg_csis_regs[SYSREG_R_CSIS_MIPI_PHY_SEL],
			&sysreg_csis_fields[SYSREG_F_CSIS_MIPI_SEPARATION_SEL], BIT(2));
	else if (csi_e_enabled && !csi_f_enabled)	/* 2 + 2 */
		is_hw_set_field(csis_sys_regs,
			&sysreg_csis_regs[SYSREG_R_CSIS_MIPI_PHY_SEL],
			&sysreg_csis_fields[SYSREG_F_CSIS_MIPI_SEPARATION_SEL], BIT(1));
	else
		is_hw_set_field(csis_sys_regs,
			&sysreg_csis_regs[SYSREG_R_CSIS_MIPI_PHY_SEL],
			&sysreg_csis_fields[SYSREG_F_CSIS_MIPI_SEPARATION_SEL], BIT(0));

	iounmap(csis_sys_regs);

#if (IS_ENABLED(CONFIG_CAMERA_CIS_ZEBU_OBJ))
	printk(KERN_INFO "[DBG] S2MPU disable\n");
	is_hw_s2mpu_cfg();
	printk(KERN_INFO "[DBG] TZPC disable\n");
	is_hw_tzpc_info();
#endif

	return ret;
}

int is_hw_camif_open(void *sensor_data)
{
	struct is_device_sensor *sensor;
	struct is_device_csi *csi;

	FIMC_BUG(!sensor_data);

	sensor = sensor_data;
	csi = (struct is_device_csi *)v4l2_get_subdevdata(sensor->subdev_csi);

	if (csi->ch >= CSI_ID_MAX) {
		merr("CSI channel is invalid(%d)\n", sensor, csi->ch);
		return -EINVAL;
	}

	set_bit(CSIS_DMA_ENABLE, &csi->state);

	if (IS_ENABLED(SOC_SSVC0))
		csi->dma_subdev[CSI_VIRTUAL_CH_0] = &sensor->ssvc0;
	else
		csi->dma_subdev[CSI_VIRTUAL_CH_0] = NULL;

	if (IS_ENABLED(SOC_SSVC1))
		csi->dma_subdev[CSI_VIRTUAL_CH_1] = &sensor->ssvc1;
	else
		csi->dma_subdev[CSI_VIRTUAL_CH_1] = NULL;

	if (IS_ENABLED(SOC_SSVC2))
		csi->dma_subdev[CSI_VIRTUAL_CH_2] = &sensor->ssvc2;
	else
		csi->dma_subdev[CSI_VIRTUAL_CH_2] = NULL;

	if (IS_ENABLED(SOC_SSVC3))
		csi->dma_subdev[CSI_VIRTUAL_CH_3] = &sensor->ssvc3;
	else
		csi->dma_subdev[CSI_VIRTUAL_CH_3] = NULL;

	return 0;
}

void is_hw_ischain_qe_cfg(void)
{
	dbg_hw(2, "%s()\n", __func__);
}

int blk_isp_mux_control(struct is_device_ischain *device, u32 value)
{
	/* Do not need */

	return 0;
}

int is_hw_ischain_cfg(void *ischain_data)
{
	int ret = 0;
	struct is_device_ischain *device;

	FIMC_BUG(!ischain_data);

	device = (struct is_device_ischain *)ischain_data;
	if (test_bit(IS_ISCHAIN_REPROCESSING, &device->state))
		return ret;

	return ret;
}

#ifdef USE_ORBMCH_WA
int is_hw_orbmch_isr_clear_register(u32 hw_id, bool enable)
{
	struct is_interface_hwip *itf_hwip = NULL;
	struct is_hw_ip *hw_ip = NULL;
	struct is_lib_support *lib = is_get_lib_support();
	int hw_slot = -1;
	int ret = 0;

	/* SW WA for ORBMCH ISR */
	if (hw_id == DEV_HW_ORB0)
		hw_slot = is_hw_slot_id(DEV_HW_ORB0);

	if (hw_slot == -1)
		return ret;

	itf_hwip = &(lib->itfc->itf_ip[hw_slot]);
	hw_ip = itf_hwip->hw_ip;
	writel(0x0, hw_ip->regs[REG_SETA] + 0x60);	/* isr all bit are disabled */
	writel(0x3FF, hw_ip->regs[REG_SETA] + 0x64);	/* isr all bit clear */

	if (enable) {
		dbg_hw(2, "%s: SW WA for ORBMCH[hw_id = %d]\n", __func__, hw_id);

		if (hw_id == DEV_HW_ORB0)
			ret = __is_hw_request_irq(itf_hwip, "orbmch0",
					INTR_HWIP1, IRQF_TRIGGER_NONE, __is_isr1_orbmch0);
	} else {
		dbg_hw(2, "%s: SW WA disable for ORBMCH[hw_id = %d]\n", __func__, hw_id);

		ret = __is_hw_free_irq(itf_hwip, INTR_HWIP1);
	}
	return ret;
}
#endif

int is_hw_ischain_enable(struct is_device_ischain *device)
{
	int ret = 0;
	struct is_interface_hwip *itf_hwip = NULL;
	struct is_lib_support *lib = is_get_lib_support();
	int hw_slot = -1;

	FIMC_BUG(!device);

	ret = blk_isp_mux_control(device, 1);
	if (!ret)
		merr("blk_isp_mux_control is failed (%d)\n", device, ret);

	/* irq affinity should be restored after S2R at gic600 */
	hw_slot = is_hw_slot_id(DEV_HW_3AA0);
	if (!valid_hw_slot_id(hw_slot)) {
		merr("invalid slot (%d,%d)", device, DEV_HW_3AA0, hw_slot);
	} else {
		itf_hwip = &(lib->itfc->itf_ip[hw_slot]);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP1], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP2], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP3], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP4], true);
	}

	hw_slot = is_hw_slot_id(DEV_HW_3AA1);
	if (!valid_hw_slot_id(hw_slot)) {
		merr("invalid slot (%d,%d)", device, DEV_HW_3AA1, hw_slot);
	} else {
		itf_hwip = &(lib->itfc->itf_ip[hw_slot]);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP1], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP2], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP3], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP4], true);
	}

	hw_slot = is_hw_slot_id(DEV_HW_3AA2);
	if (!valid_hw_slot_id(hw_slot)) {
		merr("invalid slot (%d,%d)", device, DEV_HW_3AA2, hw_slot);
	} else {
		itf_hwip = &(lib->itfc->itf_ip[hw_slot]);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP1], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP2], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP3], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP4], true);
	}

	hw_slot = is_hw_slot_id(DEV_HW_ORB0);
	if (!valid_hw_slot_id(hw_slot)) {
		merr("invalid slot (%d,%d)", device, DEV_HW_ORB0, hw_slot);
	} else {
		itf_hwip = &(lib->itfc->itf_ip[hw_slot]);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP1], true);
	}

	hw_slot = is_hw_slot_id(DEV_HW_ISP0);
	if (!valid_hw_slot_id(hw_slot)) {
		merr("invalid slot (%d,%d)", device, DEV_HW_ISP0, hw_slot);
	} else {
		itf_hwip = &(lib->itfc->itf_ip[hw_slot]);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP1], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP2], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP3], true);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP4], true);
	}

	hw_slot = is_hw_slot_id(DEV_HW_MCSC0);
	if (!valid_hw_slot_id(hw_slot)) {
		merr("invalid slot (%d,%d)", device, DEV_HW_MCSC0, hw_slot);
	} else {
		itf_hwip = &(lib->itfc->itf_ip[hw_slot]);
		is_set_affinity_irq(itf_hwip->irq[INTR_HWIP1], true);
	}

	votfitf_disable_service();

	info("%s: complete\n", __func__);

	return ret;
}

int is_hw_ischain_disable(struct is_device_ischain *device)
{
	int ret = 0;

	FIMC_BUG(!device);

	info("%s: complete\n", __func__);

	return ret;
}

/* TODO: remove this, compile check only */
#ifdef ENABLE_HWACG_CONTROL
void is_hw_csi_qchannel_enable_all(bool enable)
{
	void __iomem *csi0_regs;
	void __iomem *csi1_regs;
	void __iomem *csi2_regs;
	void __iomem *csi3_regs;
	void __iomem *csi4_regs;
	void __iomem *csi5_regs;

	u32 reg_val;

	csi0_regs = ioremap(CSIS0_QCH_EN_ADDR, SZ_4);
	csi1_regs = ioremap(CSIS1_QCH_EN_ADDR, SZ_4);
	csi2_regs = ioremap(CSIS2_QCH_EN_ADDR, SZ_4);
	csi3_regs = ioremap(CSIS3_QCH_EN_ADDR, SZ_4);
	csi4_regs = ioremap(CSIS4_QCH_EN_ADDR, SZ_4);
	csi5_regs = ioremap(CSIS5_QCH_EN_ADDR, SZ_4);

	reg_val = readl(csi0_regs);
	reg_val &= ~(1 << 20);
	writel(enable << 20 | reg_val, csi0_regs);

	reg_val = readl(csi1_regs);
	reg_val &= ~(1 << 20);
	writel(enable << 20 | reg_val, csi1_regs);

	reg_val = readl(csi2_regs);
	reg_val &= ~(1 << 20);
	writel(enable << 20 | reg_val, csi2_regs);

	reg_val = readl(csi3_regs);
	reg_val &= ~(1 << 20);
	writel(enable << 20 | reg_val, csi3_regs);

	reg_val = readl(csi4_regs);
	reg_val &= ~(1 << 20);
	writel(enable << 20 | reg_val, csi4_regs);

	reg_val = readl(csi5_regs);
	reg_val &= ~(1 << 20);
	writel(enable << 20 | reg_val, csi5_regs);

	iounmap(csi0_regs);
	iounmap(csi1_regs);
	iounmap(csi2_regs);
	iounmap(csi3_regs);
	iounmap(csi4_regs);
	iounmap(csi5_regs);
}
#endif

void is_hw_djag_adjust_out_size(struct is_device_ischain *ischain,
					u32 in_width, u32 in_height,
					u32 *out_width, u32 *out_height)
{
	struct is_global_param *g_param;
	int bratio;
	bool is_down_scale;

	if (!ischain) {
		err_hw("device is NULL");
		return;
	}

	g_param = &ischain->resourcemgr->global_param;
	is_down_scale = (*out_width < in_width) || (*out_height < in_height);

	if (test_bit(IS_ISCHAIN_OFFLINE_REPROCESSING, &ischain->state))
		bratio = ischain->binning;
	else
		bratio = is_sensor_g_bratio(ischain->sensor);

	if (bratio < 0) {
		err_hw("failed to get sensor_bratio");
		return;
	}

	dbg_hw(2, "%s:video_mode %d is_down_scale %d bratio %d\n", __func__,
			g_param->video_mode, is_down_scale, bratio);

	if (g_param->video_mode
		&& is_down_scale
		&& bratio >= MCSC_DJAG_ENABLE_SENSOR_BRATIO) {
		dbg_hw(2, "%s:%dx%d -> %dx%d\n", __func__,
				*out_width, *out_height, in_width, in_height);

		*out_width = in_width;
		*out_height = in_height;
	}
}

void __nocfi is_hw_interrupt_relay(struct is_group *group, void *hw_ip_data)
{
	struct is_group *child;
	struct is_hw_ip *hw_ip = (struct is_hw_ip *)hw_ip_data;

	child = group->child;
	if (child) {
		int hw_list[GROUP_HW_MAX], hw_slot;
		enum is_hardware_id hw_id;

		is_get_hw_list(child->id, hw_list);
		hw_id = hw_list[0];
		hw_slot = is_hw_slot_id(hw_id);
		if (!valid_hw_slot_id(hw_slot)) {
			serr_hw("invalid slot (%d,%d)", hw_ip, hw_id, hw_slot);
		} else {
			struct is_hardware *hardware;
			struct is_hw_ip *hw_ip_child;
			struct hwip_intr_handler *intr_handler;

			hardware = hw_ip->hardware;
			if (!hardware) {
				serr_hw("hardware is NILL", hw_ip);
			} else {
				hw_ip_child = &hardware->hw_ip[hw_slot];
				intr_handler = hw_ip_child->intr_handler[INTR_HWIP3];
				if (intr_handler && intr_handler->handler) {
					is_fpsimd_get_isr();
					intr_handler->handler(intr_handler->id, intr_handler->ctx);
					is_fpsimd_put_isr();
				}
			}
		}
	}
}

void is_hw_configure_llc(bool on, struct is_device_ischain *device, ulong *llc_state)
{
	dbg("not supported");
}

void is_hw_configure_bts_scen(struct is_resourcemgr *resourcemgr, int scenario_id)
{
	int bts_index = 0;

	switch (scenario_id) {
	case IS_DVFS_SN_REAR_SINGLE_VIDEO_8K24:
	case IS_DVFS_SN_REAR_SINGLE_VIDEO_8K30:
		bts_index = 1;
		break;
	default:
		bts_index = 0;
		break;
	}

	/* If default scenario & specific scenario selected,
	 * off specific scenario first.
	 */
	if (resourcemgr->cur_bts_scen_idx && bts_index == 0)
		is_bts_scen(resourcemgr, resourcemgr->cur_bts_scen_idx, false);

	if (bts_index && bts_index != resourcemgr->cur_bts_scen_idx)
		is_bts_scen(resourcemgr, bts_index, true);
	resourcemgr->cur_bts_scen_idx = bts_index;
}

int is_hw_get_output_slot(u32 vid)
{
	int ret = -1;

	switch (vid) {
	case IS_VIDEO_SS0_NUM:
	case IS_VIDEO_SS1_NUM:
	case IS_VIDEO_SS2_NUM:
	case IS_VIDEO_SS3_NUM:
	case IS_VIDEO_SS4_NUM:
	case IS_VIDEO_SS5_NUM:
	case IS_VIDEO_PAF0S_NUM:
	case IS_VIDEO_PAF1S_NUM:
	case IS_VIDEO_PAF2S_NUM:
	case IS_VIDEO_PAF3S_NUM:
	case IS_VIDEO_30S_NUM:
	case IS_VIDEO_31S_NUM:
	case IS_VIDEO_32S_NUM:
	case IS_VIDEO_33S_NUM:
	case IS_VIDEO_I0S_NUM:
	case IS_VIDEO_YPP_NUM:
	case IS_VIDEO_ORB0_NUM:
	case IS_VIDEO_ORB1_NUM:
		ret = 0;
		break;
	default:
		ret = -1;
	}

	return ret;
}

int is_hw_get_capture_slot(struct is_frame *frame, u32 **taddr, u64 **taddr_k, u32 vid)
{
	int ret = 0;

	switch(vid) {
	/* TAA */
	case IS_VIDEO_30C_NUM:
	case IS_VIDEO_31C_NUM:
	case IS_VIDEO_32C_NUM:
		*taddr = frame->txcTargetAddress;
		break;
	case IS_VIDEO_30P_NUM:
	case IS_VIDEO_31P_NUM:
	case IS_VIDEO_32P_NUM:
		*taddr = frame->txpTargetAddress;
		break;
	case IS_VIDEO_30G_NUM:
	case IS_VIDEO_31G_NUM:
	case IS_VIDEO_32G_NUM:
		*taddr = frame->mrgTargetAddress;
		break;
	case IS_VIDEO_30F_NUM:
	case IS_VIDEO_31F_NUM:
	case IS_VIDEO_32F_NUM:
		*taddr = frame->efdTargetAddress;
		break;
	case IS_VIDEO_30D_NUM:
	case IS_VIDEO_31D_NUM:
	case IS_VIDEO_32D_NUM:
		*taddr = frame->txdgrTargetAddress;
		break;
	case IS_VIDEO_30O_NUM:
	case IS_VIDEO_31O_NUM:
	case IS_VIDEO_32O_NUM:
		*taddr = frame->txodsTargetAddress;
		break;
	case IS_VIDEO_30L_NUM:
	case IS_VIDEO_31L_NUM:
	case IS_VIDEO_32L_NUM:
		*taddr = frame->txldsTargetAddress;
		break;
	case IS_VIDEO_30H_NUM:
	case IS_VIDEO_31H_NUM:
	case IS_VIDEO_32H_NUM:
	case IS_VIDEO_33H_NUM:
		*taddr = frame->txhfTargetAddress;
		break;
	/* ISP */
	case IS_VIDEO_I0C_NUM:
		*taddr = frame->ixcTargetAddress;
		break;
	case IS_VIDEO_I0P_NUM:
		*taddr = frame->ixpTargetAddress;
		break;
	case IS_VIDEO_I0V_NUM:
		*taddr = frame->ixvTargetAddress;
		break;
	case IS_VIDEO_I0W_NUM:
		*taddr = frame->ixwTargetAddress;
		break;
	case IS_VIDEO_I0T_NUM:
		*taddr = frame->ixtTargetAddress;
		break;
	case IS_VIDEO_I0G_NUM:
		*taddr = frame->ixgTargetAddress;
		break;
	case IS_VIDEO_IMM_NUM:
		*taddr = frame->ixmTargetAddress;
		if (taddr_k)
			*taddr_k = frame->ixmKTargetAddress;
		break;
	case IS_VIDEO_IRG_NUM:
		*taddr = frame->ixrrgbTargetAddress;
		break;
	case IS_VIDEO_ISC_NUM:
		*taddr = frame->ixscmapTargetAddress;
		break;
	case IS_VIDEO_IDR_NUM:
		*taddr = frame->ixdgrTargetAddress;
		break;
	case IS_VIDEO_INR_NUM:
		*taddr = frame->ixnoirTargetAddress;
		break;
	case IS_VIDEO_IND_NUM:
		*taddr = frame->ixnrdsTargetAddress;
		break;
	case IS_VIDEO_IDG_NUM:
		*taddr = frame->ixdgaTargetAddress;
		break;
	case IS_VIDEO_ISH_NUM:
		*taddr = frame->ixsvhistTargetAddress;
		break;
	case IS_VIDEO_IHF_NUM:
		*taddr = frame->ixhfTargetAddress;
		break;
	case IS_VIDEO_INW_NUM:
		*taddr = frame->ixnoiTargetAddress;
		break;
	case IS_VIDEO_INRW_NUM:
		*taddr = frame->ixnoirwTargetAddress;
		break;
	case IS_VIDEO_IRGW_NUM:
		*taddr = frame->ixwrgbTargetAddress;
		break;
	case IS_VIDEO_INB_NUM:
		*taddr = frame->ixbnrTargetAddress;
		break;
	/* MCSC */
	case IS_VIDEO_M0P_NUM:
		*taddr = frame->sc0TargetAddress;
		break;
	case IS_VIDEO_M1P_NUM:
		*taddr = frame->sc1TargetAddress;
		break;
	case IS_VIDEO_M2P_NUM:
		*taddr = frame->sc2TargetAddress;
		break;
	case IS_VIDEO_M3P_NUM:
		*taddr = frame->sc3TargetAddress;
		break;
	case IS_VIDEO_M4P_NUM:
		*taddr = frame->sc4TargetAddress;
		break;
	case IS_VIDEO_M5P_NUM:
		*taddr = frame->sc5TargetAddress;
		break;
	/* ORBMCH */
	case IS_VIDEO_ORB0C_NUM:
		*taddr = frame->orbxcTargetAddress;
		if (taddr_k)
			*taddr_k = frame->orbxcKTargetAddress;
		break;
	case IS_VIDEO_ORB0M_NUM:
		/* No DMA out */
		if (taddr_k)
			*taddr_k = frame->orbxmKTargetAddress;
		break;
	case IS_VIDEO_MCH0S_NUM:
		*taddr = frame->mchxsTargetAddress;
		if (taddr_k)
			*taddr_k = frame->mchxsKTargetAddress;
		/* Should not clear buffer to keep mch previous input data */
		return ret;
	default:
		err_hw("Unsupported vid(%d)", vid);
		ret = -EINVAL;
		break;
	}

	/* Clear subdev frame's target address before set */
	if (taddr && *taddr)
		memset(*taddr, 0x0, sizeof(typeof(**taddr)) * IS_MAX_PLANES);
	if (taddr_k && *taddr_k)
		memset(*taddr_k, 0x0, sizeof(typeof(**taddr_k)) * IS_MAX_PLANES);

	return ret;
}

void * is_get_dma_blk(int type)
{
	struct is_lib_support *lib = is_get_lib_support();
	struct lib_mem_block * mblk = NULL;

	switch (type) {
	case ID_DMA_3AAISP:
		mblk = &lib->mb_dma_taaisp;
		break;
	case ID_DMA_MEDRC:
		mblk = &lib->mb_dma_medrc;
		break;
	case ID_DMA_ORBMCH:
		mblk = &lib->mb_dma_orbmch;
		break;
	case ID_DMA_TNR:
		mblk = &lib->mb_dma_tnr;
		break;
	case ID_DMA_CLAHE:
		mblk = &lib->mb_dma_clahe;
		break;
	default:
		err_hw("Invalid DMA type: %d\n", type);
		return NULL;
	}

	return (void *)mblk;
}

int is_get_dma_id(u32 vid)
{
	return -EINVAL;
}

int is_hw_check_changed_chain_id(struct is_hardware *hardware, u32 hw_id, u32 instance)
{
	int hw_idx;
	int mapped_hw_id = 0;

	FIMC_BUG(!hardware);

	switch (hw_id) {
	case DEV_HW_PAF0:
	case DEV_HW_PAF1:
	case DEV_HW_PAF2:
		for (hw_idx = DEV_HW_PAF0; hw_idx <= DEV_HW_PAF2; hw_idx++) {
			if (test_bit(hw_idx, &hardware->hw_map[instance]) && hw_idx != hw_id) {
				mapped_hw_id = hw_idx;
				break;
			}
		}
		break;
	case DEV_HW_3AA0:
	case DEV_HW_3AA1:
	case DEV_HW_3AA2:
		for (hw_idx = DEV_HW_3AA0; hw_idx <= DEV_HW_3AA2; hw_idx++) {
			if (test_bit(hw_idx, &hardware->hw_map[instance]) && hw_idx != hw_id) {
				mapped_hw_id = hw_idx;
				break;
			}
		}
		break;
	default:
		break;
	}

	return mapped_hw_id;
}

void is_hw_fill_target_address(u32 hw_id, struct is_frame *dst, struct is_frame *src,
	bool reset)
{
	/* A previous address should not be cleared for sysmmu debugging. */
	reset = false;

	switch (hw_id) {
	case DEV_HW_PAF0:
	case DEV_HW_PAF1:
	case DEV_HW_PAF2:
		break;
	case DEV_HW_3AA0:
	case DEV_HW_3AA1:
	case DEV_HW_3AA2:
		TADDR_COPY(dst, src, txcTargetAddress, reset);
		TADDR_COPY(dst, src, txpTargetAddress, reset);
		TADDR_COPY(dst, src, mrgTargetAddress, reset);
		TADDR_COPY(dst, src, efdTargetAddress, reset);
		TADDR_COPY(dst, src, txdgrTargetAddress, reset);
		TADDR_COPY(dst, src, txodsTargetAddress, reset);
		TADDR_COPY(dst, src, txldsTargetAddress, reset);
		TADDR_COPY(dst, src, txhfTargetAddress, reset);
		break;
	case DEV_HW_ORB0:
		TADDR_COPY(dst, src, orbxmKTargetAddress, reset);
		TADDR_COPY(dst, src, orbxcTargetAddress, reset);
		TADDR_COPY(dst, src, orbxcKTargetAddress, reset);
		TADDR_COPY(dst, src, mchxsTargetAddress, reset);
		TADDR_COPY(dst, src, mchxsKTargetAddress, reset);
		break;
	case DEV_HW_ISP0:
		TADDR_COPY(dst, src, ixcTargetAddress, reset);
		TADDR_COPY(dst, src, ixpTargetAddress, reset);
		TADDR_COPY(dst, src, ixtTargetAddress, reset);
		TADDR_COPY(dst, src, ixgTargetAddress, reset);
		TADDR_COPY(dst, src, ixvTargetAddress, reset);
		TADDR_COPY(dst, src, ixwTargetAddress, reset);
		TADDR_COPY(dst, src, ixmTargetAddress, reset);
		TADDR_COPY(dst, src, ixmKTargetAddress, reset);
		TADDR_COPY(dst, src, ixdgrTargetAddress, reset);
		TADDR_COPY(dst, src, ixrrgbTargetAddress, reset);
		TADDR_COPY(dst, src, ixnoirTargetAddress, reset);
		TADDR_COPY(dst, src, ixscmapTargetAddress, reset);
		TADDR_COPY(dst, src, ixnrdsTargetAddress, reset);
		TADDR_COPY(dst, src, ixdgaTargetAddress, reset);
		TADDR_COPY(dst, src, ixhfTargetAddress, reset);
		TADDR_COPY(dst, src, ixwrgbTargetAddress, reset);
		TADDR_COPY(dst, src, ixnoirwTargetAddress, reset);
		TADDR_COPY(dst, src, ixbnrTargetAddress, reset);
		TADDR_COPY(dst, src, ixnoiTargetAddress, reset);
		break;
	case DEV_HW_MCSC0:
		TADDR_COPY(dst, src, sc0TargetAddress, reset);
		TADDR_COPY(dst, src, sc1TargetAddress, reset);
		TADDR_COPY(dst, src, sc2TargetAddress, reset);
		TADDR_COPY(dst, src, sc3TargetAddress, reset);
		TADDR_COPY(dst, src, sc4TargetAddress, reset);
		TADDR_COPY(dst, src, sc5TargetAddress, reset);
		break;
	default:
		err("[%d] Invalid hw id(%d)", src->instance, hw_id);
		break;
	}
}

void is_hw_chain_probe(void *core)
{
	size_t i, size;

	size = ARRAY_SIZE(is_video_probe_fns);

	for (i = 0; i < size; i++)
		is_video_probe_fns[i](core);
}

struct is_mem *is_hw_get_iommu_mem(u32 vid)
{
	struct is_core *core = is_get_is_core();
	struct pablo_device_iommu_group *iommu_group;

	if (!core)
		return NULL;

	switch (vid) {
	case IS_VIDEO_SS0_NUM:
	case IS_VIDEO_SS1_NUM:
	case IS_VIDEO_SS2_NUM:
	case IS_VIDEO_SS3_NUM:
	case IS_VIDEO_SS4_NUM:
	case IS_VIDEO_SS5_NUM:
	case IS_VIDEO_SS6_NUM:
	case IS_VIDEO_SS7_NUM:
	case IS_VIDEO_SS8_NUM:
	case IS_VIDEO_SS0VC0_NUM:
	case IS_VIDEO_SS0VC1_NUM:
	case IS_VIDEO_SS0VC2_NUM:
	case IS_VIDEO_SS0VC3_NUM:
	case IS_VIDEO_SS1VC0_NUM:
	case IS_VIDEO_SS1VC1_NUM:
	case IS_VIDEO_SS1VC2_NUM:
	case IS_VIDEO_SS1VC3_NUM:
	case IS_VIDEO_SS2VC0_NUM:
	case IS_VIDEO_SS2VC1_NUM:
	case IS_VIDEO_SS2VC2_NUM:
	case IS_VIDEO_SS2VC3_NUM:
	case IS_VIDEO_SS3VC0_NUM:
	case IS_VIDEO_SS3VC1_NUM:
	case IS_VIDEO_SS3VC2_NUM:
	case IS_VIDEO_SS3VC3_NUM:
	case IS_VIDEO_SS4VC0_NUM:
	case IS_VIDEO_SS4VC1_NUM:
	case IS_VIDEO_SS4VC2_NUM:
	case IS_VIDEO_SS4VC3_NUM:
	case IS_VIDEO_SS5VC0_NUM:
	case IS_VIDEO_SS5VC1_NUM:
	case IS_VIDEO_SS5VC2_NUM:
	case IS_VIDEO_SS5VC3_NUM:
	case IS_VIDEO_SS6VC0_NUM:
	case IS_VIDEO_SS6VC1_NUM:
	case IS_VIDEO_SS6VC2_NUM:
	case IS_VIDEO_SS6VC3_NUM:
	case IS_VIDEO_PAF0S_NUM:
	case IS_VIDEO_PAF1S_NUM:
	case IS_VIDEO_PAF2S_NUM:
	case IS_VIDEO_PAF3S_NUM:
	case IS_VIDEO_30S_NUM:
	case IS_VIDEO_30C_NUM:
	case IS_VIDEO_30P_NUM:
	case IS_VIDEO_30F_NUM:
	case IS_VIDEO_30G_NUM:
	case IS_VIDEO_30O_NUM:
	case IS_VIDEO_30L_NUM:
	case IS_VIDEO_32O_NUM:
	case IS_VIDEO_32L_NUM:
	case IS_VIDEO_31S_NUM:
	case IS_VIDEO_31C_NUM:
	case IS_VIDEO_31P_NUM:
	case IS_VIDEO_31F_NUM:
	case IS_VIDEO_31G_NUM:
	case IS_VIDEO_31O_NUM:
	case IS_VIDEO_31L_NUM:
	case IS_VIDEO_33O_NUM:
	case IS_VIDEO_33L_NUM:
	case IS_VIDEO_32S_NUM:
	case IS_VIDEO_32C_NUM:
	case IS_VIDEO_32P_NUM:
	case IS_VIDEO_32F_NUM:
	case IS_VIDEO_32G_NUM:
	case IS_VIDEO_33S_NUM:
	case IS_VIDEO_33C_NUM:
	case IS_VIDEO_33P_NUM:
	case IS_VIDEO_33F_NUM:
	case IS_VIDEO_33G_NUM:
	case IS_VIDEO_30D_NUM:
	case IS_VIDEO_31D_NUM:
	case IS_VIDEO_32D_NUM:
	case IS_VIDEO_33D_NUM:
	case IS_VIDEO_30H_NUM:
	case IS_VIDEO_31H_NUM:
	case IS_VIDEO_32H_NUM:
	case IS_VIDEO_33H_NUM:
		iommu_group = pablo_iommu_group_get(0);
		return &iommu_group->mem;
	case IS_VIDEO_I0S_NUM:
	case IS_VIDEO_I0C_NUM:
	case IS_VIDEO_I0P_NUM:
	case IS_VIDEO_I0V_NUM:
	case IS_VIDEO_I0W_NUM:
	case IS_VIDEO_I0T_NUM:
	case IS_VIDEO_I0G_NUM:
	case IS_VIDEO_IMM_NUM:
	case IS_VIDEO_IRG_NUM:
	case IS_VIDEO_ISC_NUM:
	case IS_VIDEO_IDR_NUM:
	case IS_VIDEO_INR_NUM:
	case IS_VIDEO_IND_NUM:
	case IS_VIDEO_IDG_NUM:
	case IS_VIDEO_ISH_NUM:
	case IS_VIDEO_IHF_NUM:
	case IS_VIDEO_INW_NUM:
	case IS_VIDEO_INRW_NUM:
	case IS_VIDEO_IRGW_NUM:
	case IS_VIDEO_INB_NUM:
	case IS_VIDEO_ORB0_NUM:
	case IS_VIDEO_ORB1_NUM:
	case IS_VIDEO_ORB0C_NUM:
	case IS_VIDEO_ORB0M_NUM:
	case IS_VIDEO_MCH0S_NUM:
	case IS_VIDEO_ORB1C_NUM:
	case IS_VIDEO_ORB1M_NUM:
	case IS_VIDEO_MCH1S_NUM:
	case IS_VIDEO_M0P_NUM:
	case IS_VIDEO_M1P_NUM:
	case IS_VIDEO_M2P_NUM:
	case IS_VIDEO_M3P_NUM:
	case IS_VIDEO_M4P_NUM:
	case IS_VIDEO_M5P_NUM:
		iommu_group = pablo_iommu_group_get(1);
		return &iommu_group->mem;
	default:
		err("Invalid vid(%d)", vid);
		return NULL;
	}
}
