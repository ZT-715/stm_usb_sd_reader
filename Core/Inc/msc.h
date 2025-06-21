/*
 * msc.h
 *
 *  Created on: Jun 22, 2025
 *      Author: wymac
 */

#ifndef INC_MSC_H_
#define INC_MSC_H_


void msc_handle_write10(const MSC_CmdBlockWrapper_t *cbw);

void msc_handle_start_stop_unit(const MSC_CmdBlockWrapper_t *cbw);

void msc_handle_read10(const MSC_CmdBlockWrapper_t *cbw);

static void msc_handle_mode_sense6(const MSC_CmdBlockWrapper_t *cbw);

static void msc_handle_request_sense(const MSC_CmdBlockWrapper_t *cbw);

void msc_handle_read_capacity(const MSC_CmdBlockWrapper_t *cbw);

void msc_handle_test_unit_ready(const MSC_CmdBlockWrapper_t *cbw);

void msc_handle_inquiry(const MSC_CmdBlockWrapper_t *cbw);

#endif /* INC_MSC_H_ */
