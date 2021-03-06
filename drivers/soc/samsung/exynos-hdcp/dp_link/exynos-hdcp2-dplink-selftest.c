/*
 * drivers/soc/samsung/exynos-hdcp/dplink/exynos-hdcp2-dplink-selftest.c
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <asm/cacheflush.h>

#include "../exynos-hdcp2.h"
#include "../exynos-hdcp2-config.h"
#include "../exynos-hdcp2-protocol-msg.h"
#include "../exynos-hdcp2-misc.h"
#include "../exynos-hdcp2-encrypt.h"
#include "../exynos-hdcp2-log.h"
#if defined(CONFIG_HDCP2_EMULATION_MODE)
#include "../exynos-hdcp2-testvector.h"
#endif
#include "exynos-hdcp2-dplink-protocol-msg.h"

#define DP_AKE_INIT_LEN			11
#define DP_AKE_NO_STORED_KM_LEN		128
#define DP_LC_INIT_LEN			8
#define DP_SKE_SEND_EKS_LEN		24

/* todo: define DP test vector */
static struct hdcp_tx_ctx g_tx_ctx;
static struct hdcp_rx_ctx g_rx_ctx;
static int dp_utc_ske_send_eks(struct hdcp_tx_ctx *tx_ctx, struct hdcp_rx_ctx *rx_ctx)
{
	uint8_t m[525];
	size_t m_len;
	int ret;
	int i;

	tx_ctx->share_skey = 0;

	ret = dp_cap_protocol_msg(SKE_SEND_EKS, m, &m_len, tx_ctx, rx_ctx);
	if (ret) {
		hdcp_err("SKE_Send_Eks is failed with 0x%x\n", ret);
		return -1;
	}

	if (m_len != DP_SKE_SEND_EKS_LEN) {
		hdcp_err("Message LENGTH ERROR\n");
		return -1;
	}

	/* compare encrypted session key with test vector */
	for (i = 0; i < HDCP_AKE_MKEY_BYTE_LEN; i++)
		if (tv_ske_eskey_v22[i] != m[i])
			break;

	if (i != HDCP_AKE_MKEY_BYTE_LEN) {
		hdcp_err("m doesn't match (%dth)\n", i);
		return -1;
	}

	return 0;
}

/* Test SKE APIs */
static int dp_utc_ske(struct hdcp_tx_ctx *tx_ctx, struct hdcp_rx_ctx *rx_ctx, int *cnt, int *fail)
{
	int ret;

	ret = dp_utc_ske_send_eks(tx_ctx, rx_ctx);
	if (ret) {
		hdcp_err("utc_ske_send_eks() is failed with 0x%x\n", ret);
		return -1;
	}

	return 0;
}
static int dp_utc_lc_send_l_prime(struct hdcp_tx_ctx *tx_ctx, struct hdcp_rx_ctx *rx_ctx)
{
	int ret;

	ret = dp_decap_protocol_msg(LC_SEND_L_PRIME, msg_rx_lc_send_l_prime_v22,
			sizeof(msg_rx_send_h_prime_v22),
			tx_ctx, rx_ctx);
	if (ret) {
		hdcp_err("LC_Send_L_prime is failed with 0x%x\n", ret);
		return -1;
	}

	return 0;
}

static int dp_utc_lc_init(struct hdcp_tx_ctx *tx_ctx, struct hdcp_rx_ctx *rx_ctx)
{
	uint8_t m[525];
	size_t m_len;
	int ret;

	ret = dp_cap_protocol_msg(LC_INIT, m, &m_len, tx_ctx, rx_ctx);
	if (ret) {
		hdcp_err("LC_Init is failed with 0x%x\n", ret);
		return -1;
	}

	if (m_len != DP_LC_INIT_LEN) {
		hdcp_err("Invalid Message length. len(%zu)\n", m_len);
		return -1;
	}

	return 0;
}

/* Test LC APIs */
static int dp_utc_lc(struct hdcp_tx_ctx *tx_ctx, struct hdcp_rx_ctx *rx_ctx,
		int *cnt, int *fail)
{
	int ret;

	ret = dp_utc_lc_init(tx_ctx, rx_ctx);
	if (ret < 0)
		return ret;

	ret = dp_utc_lc_send_l_prime(tx_ctx, rx_ctx);
	if (ret < 0)
		return ret;

	return 0;
}

static int dp_utc_ake_send_h_prime(struct hdcp_tx_ctx *tx_ctx,
				struct hdcp_rx_ctx *rx_ctx)
{
	int ret;

	ret = dp_decap_protocol_msg(AKE_SEND_H_PRIME, msg_rx_send_h_prime_v22,
			sizeof(msg_rx_send_h_prime_v22),
			tx_ctx, rx_ctx);
	if (ret) {
		hdcp_err("AKE_Send_H_prime is failed with 0x%x\n", ret);
		return -1;
	}

	return 0;
}

static int dp_utc_ake_send_pairing_info(struct hdcp_tx_ctx *tx_ctx,
				struct hdcp_rx_ctx *rx_ctx)
{
	/* todo: support pairing */
	int i;
	int ret;
	int found_km;

	ret = dp_decap_protocol_msg(AKE_SEND_PAIRING_INFO, msg_rx_send_pairing_info_v22,
			sizeof(msg_rx_send_pairing_info_v22),
			tx_ctx, rx_ctx);
	if (ret) {
		hdcp_err("AKE_Send_Pairing_Info is failed with 0x%x\n", ret);
		return -1;
	}

	ret = ake_find_masterkey(&found_km,
				tx_ctx->ekh_mkey, HDCP_AKE_EKH_MKEY_BYTE_LEN,
				tx_ctx->m, HDCP_AKE_M_BYTE_LEN);
	if (ret) {
		hdcp_err("find_masterkey() is failed with 0x%x\n", ret);
		return -1;
	}

	if (found_km) {
		for (i = 0; i < HDCP_AKE_MKEY_BYTE_LEN; i++)
			if (tx_ctx->ekh_mkey[i] != msg_rx_send_pairing_info_v22[i])
				break;

		if (i != HDCP_AKE_MKEY_BYTE_LEN) {
			hdcp_err("ekh(m) doesn't match (%dth)\n", i);
			return -1;
		}
	} else {
		hdcp_err("ekh(m) is not found\n");
		return -1;
	}

	return 0;
}

static int dp_utc_ake_no_stored_km(struct hdcp_tx_ctx *tx_ctx,
				struct hdcp_rx_ctx *rx_ctx)
{
	uint8_t m[525];
	size_t m_len;
	int ret;
	int i;

	memcpy(rx_ctx->cert, cert_v22, HDCP_RX_CERT_LEN);

	ret = dp_cap_protocol_msg(AKE_NO_STORED_KM, m, &m_len, tx_ctx, rx_ctx);
	if (ret) {
		hdcp_err("AKE_No_Stored_km is failed with 0x%x\n", ret);
		return -1;
	}

	if (m_len != DP_AKE_NO_STORED_KM_LEN) {
		hdcp_err("Invalid Message length. len(%zu)\n", m_len);
		return -1;
	}

	for (i = 0; i < 128; i++)
		if (m[i] != tv_emkey_v22[i])
			break;

	if (i != 128) {
		hdcp_err("Encryption Master Key ERROR\n");
		return -1;
	}

	return 0;
}

static int dp_utc_ake_send_cert(struct hdcp_tx_ctx *tx_ctx,
			struct hdcp_rx_ctx *rx_ctx)
{
	int ret;

	ret = dp_decap_protocol_msg(AKE_SEND_CERT, msg_rx_send_cert_v22,
			sizeof(msg_rx_send_cert_v22),
			tx_ctx, rx_ctx);
	if (ret) {
		hdcp_err("AKE_Send_Cert is failed with ret, 0x%x\n", ret);
		return -1;
	}

	return 0;
}

static int dp_utc_ake_init(struct hdcp_tx_ctx *tx_ctx, struct hdcp_rx_ctx *rx_ctx)
{
	uint8_t m[525];
	size_t m_len;
	int ret;

	ret = dp_cap_protocol_msg(DP_AKE_INIT, m, &m_len, tx_ctx, NULL);
	if (ret) {
		hdcp_err("AKE_Init is failed with 0x%x\n", ret);
		return -1;
	}

	/* check message length */
	if (m_len != DP_AKE_INIT_LEN) {
		hdcp_err("Invalid Message Length. len(%zu)\n", m_len);
		return -1;
	}

	return 0;
}

/* Test AKE APIs */
static int dp_utc_ake(struct hdcp_tx_ctx *tx_ctx, struct hdcp_rx_ctx *rx_ctx,
		int *cnt, int *fail)
{
	int ret;

	ret = dp_utc_ake_init(tx_ctx, rx_ctx);
	if (ret < 0)
		return ret;

	ret = dp_utc_ake_send_cert(tx_ctx, rx_ctx);
	if (ret < 0)
		return ret;

	ret = dp_utc_ake_no_stored_km(tx_ctx, rx_ctx);
	if (ret < 0)
		return ret;

	ret = dp_utc_ake_send_h_prime(tx_ctx, rx_ctx);
	if (ret < 0)
		return ret;

	ret = dp_utc_ake_send_pairing_info(tx_ctx, rx_ctx);
	if (ret < 0)
		return ret;

	return 0;
}

/* Test HDCP API functions */
int dp_hdcp_protocol_self_test(void)
{
	int total_cnt = 0, fail_cnt = 0;

	hdcp_info("[ AKE UTC]\n");
	if (dp_utc_ake(&g_tx_ctx, &g_rx_ctx, &total_cnt, &fail_cnt) < 0) {
		hdcp_info("AKE UTC: fail\n");
		return -1;
	} else {
		hdcp_info("AKE UTC: success\n");
	}

	hdcp_info("\n[ LC UTC]\n");
	if (dp_utc_lc(&g_tx_ctx, &g_rx_ctx, &total_cnt, &fail_cnt) < 0) {
		hdcp_info("LC UTC: fail\n");
		return -1;
	} else {
		hdcp_info("LC UTC: success\n");
	}

	hdcp_info("\n[ SKE UTC]\n");
	if (dp_utc_ske(&g_tx_ctx, &g_rx_ctx, &total_cnt, &fail_cnt) < 0) {
		hdcp_info("SKE UTC: fail\n");
		return -1;
	} else {
		hdcp_info("SKE UTC: success\n");
	}

	return 0;
}

MODULE_LICENSE("GPL");
