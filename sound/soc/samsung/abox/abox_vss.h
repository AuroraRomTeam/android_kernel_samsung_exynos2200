/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * ALSA SoC - Samsung Abox VSS
 *
 * Copyright (c) 2017 Samsung Electronics Co. Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SND_SOC_ABOX_VSS_H
#define __SND_SOC_ABOX_VSS_H

/**
 * Notify Call start or stop
 * @param[in]	dev		pointer to calling device
 * @param[in]	data		abox_data
 * @param[in]	en		if enable 1, if not 0
 * @return	0 or error code
 */
extern int abox_vss_notify_call(struct device *dev, struct abox_data *data,
		int en);

/**
 * Notify Call start or stop
 * @param[in]	evt		call event
 * @param[in]	data		NULL
 * @return	none
 */
extern void abox_call_notify_event(enum abox_call_event evt, void *data);

/**
 * Notify VSS start or stop
 * @param[in]	start		true for start, false for stop
 */
extern void abox_vss_notify_status(bool start);

#endif /* __SND_SOC_ABOX_VSS_H */
