/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates.
 * All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _RXR_PKT_TYPE_H
#define _RXR_PKT_TYPE_H

/* This header file contain the ID of all RxR packet types, and
 * the necessary data structures and functions for each packet type
 *
 * RxR packet types can be classified into 3 categories:
 *     data packet, control packet and context packet
 *
 * For each packet type, the following items are needed:
 *
 *   First, each packet type need to define a struct for its header,
 *       and the header must be start with ```struct rxr_base_hdr```.
 *
 *   Second, each control packet type need to define an init()
 *       function and a handle_sent() function. These functions
 *       are called by rxr_pkt_post_ctrl_or_queue().
 *
 *   Finally, each packet type (except context packet) need to
 *     define a handle_recv() functions which is called by
 *     rxr_pkt_handle_recv_completion().
 */

/* ID of each packet type. Changing ID would break inter
 * operability thus is strictly prohibited.
 */

#define RXR_RTS_PKT		1
#define RXR_CONNACK_PKT		2
#define RXR_CTS_PKT		3
#define RXR_DATA_PKT		4
#define RXR_READRSP_PKT		5
#define RXR_RMA_CONTEXT_PKT	6
#define RXR_EOR_PKT		7
/*
 * The following packet types are part of protocol version 4
 */
#define RXR_HANDSHAKE_PKT	8

#define RXR_REQ_PKT_BEGIN		64
#define RXR_BASELINE_REQ_PKT_BEGIN	64
#define RXR_EAGER_MSGRTM_PKT		64
#define RXR_EAGER_TAGRTM_PKT		65
#define RXR_MEDIUM_MSGRTM_PKT		66
#define RXR_MEDIUM_TAGRTM_PKT		67
#define RXR_LONG_MSGRTM_PKT		68
#define RXR_LONG_TAGRTM_PKT		69
#define RXR_EAGER_RTW_PKT		70
#define RXR_LONG_RTW_PKT		71
#define RXR_SHORT_RTR_PKT		72
#define RXR_LONG_RTR_PKT		73
#define RXR_WRITE_RTA_PKT		74
#define RXR_READFETCH_RTA_PKT		75
#define RXR_BASELINE_REQ_PKT_END	76

#define RXR_EXTRA_REQ_PKT_BEGIN		76
#define RXR_READ_MSGRTM_PKT		76
#define RXR_READ_TAGRTM_PKT		77
#define RXR_READ_RTW_PKT		78
#define RXR_READ_RTR_PKT		79

/*
 *  Packet fields common to all rxr packets. The other packet headers below must
 *  be changed if this is updated.
 */
struct rxr_base_hdr {
	uint8_t type;
	uint8_t version;
	uint16_t flags;
};

#if defined(static_assert) && defined(__x86_64__)
static_assert(sizeof(struct rxr_base_hdr) == 4, "rxr_base_hdr check");
#endif

static inline struct rxr_base_hdr *rxr_get_base_hdr(void *pkt)
{
	return (struct rxr_base_hdr *)pkt;
}

struct rxr_ep;
struct rxr_peer;
struct rxr_tx_entry;
struct rxr_rx_entry;
struct rxr_read_entry;

/*
 *  RTS packet data structures and functions. the implementation of
 *  these functions are in rxr_pkt_type_rts.c
 */
struct rxr_rts_hdr {
	uint8_t type;
	uint8_t version;
	uint16_t flags;
	/* end of rxr_base_hdr */
	/* TODO: need to add msg_id -> tx_id mapping to remove tx_id */
	uint16_t credit_request;
	uint8_t addrlen;
	uint8_t rma_iov_count;
	uint32_t tx_id;
	uint32_t msg_id;
	uint64_t tag;
	uint64_t data_len;
};

#if defined(static_assert) && defined(__x86_64__)
static_assert(sizeof(struct rxr_rts_hdr) == 32, "rxr_rts_hdr check");
#endif

static inline
struct rxr_rts_hdr *rxr_get_rts_hdr(void *pkt)
{
	return (struct rxr_rts_hdr *)pkt;
}

uint64_t rxr_get_rts_data_size(struct rxr_ep *ep,
			       struct rxr_rts_hdr *rts_hdr);

ssize_t rxr_pkt_init_rts(struct rxr_ep *ep,
			 struct rxr_tx_entry *tx_entry,
			 struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_rts_sent(struct rxr_ep *ep, struct rxr_pkt_entry *pkt_entry);

ssize_t rxr_pkt_proc_matched_msg_rts(struct rxr_ep *ep,
				     struct rxr_rx_entry *rx_entry,
				     struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_rts_send_completion(struct rxr_ep *ep,
					struct rxr_pkt_entry *pkt_entry);

ssize_t rxr_pkt_post_shm_rndzv_read(struct rxr_ep *ep, struct rxr_rx_entry *rx_entry);

ssize_t rxr_pkt_proc_rts(struct rxr_ep *ep, struct rxr_pkt_entry *pkt_entry);


void rxr_pkt_handle_rts_recv(struct rxr_ep *ep, struct rxr_pkt_entry *pkt_entry);

/*
 *  CONNACK packet header and functions
 *  implementation of the functions are in rxr_pkt_type_misc.c
 */
struct rxr_connack_hdr {
	uint8_t type;
	uint8_t version;
	uint16_t flags;
	/* end of rxr_base_hdr */
};

#if defined(static_assert) && defined(__x86_64__)
static_assert(sizeof(struct rxr_connack_hdr) == 4, "rxr_connack_hdr check");
#endif

#define RXR_CONNACK_HDR_SIZE		(sizeof(struct rxr_connack_hdr))

static inline
struct rxr_connack_hdr *rxr_get_connack_hdr(void *pkt)
{
	return (struct rxr_connack_hdr *)pkt;
}

ssize_t rxr_pkt_init_connack(struct rxr_ep *ep,
			     struct rxr_pkt_entry *pkt_entry,
			     fi_addr_t addr);

void rxr_pkt_post_connack(struct rxr_ep *ep,
			  struct rxr_peer *peer,
			  fi_addr_t addr);

void rxr_pkt_handle_connack_recv(struct rxr_ep *ep,
				 struct rxr_pkt_entry *pkt_entry,
				 fi_addr_t addr);
/*
 *  CTS packet data structures and functions.
 *  Definition of the functions is in rxr_pkt_type_misc.c
 */
struct rxr_cts_hdr {
	uint8_t type;
	uint8_t version;
	uint16_t flags;
	/* end of rxr_base_hdr */
	uint8_t pad[4];
	/* TODO: need to add msg_id -> tx_id/rx_id mapping */
	uint32_t tx_id;
	uint32_t rx_id;
	uint64_t window;
};

#if defined(static_assert) && defined(__x86_64__)
static_assert(sizeof(struct rxr_cts_hdr) == 24, "rxr_cts_hdr check");
#endif

#define RXR_CTS_HDR_SIZE		(sizeof(struct rxr_cts_hdr))

static inline
struct rxr_cts_hdr *rxr_get_cts_hdr(void *pkt)
{
	return (struct rxr_cts_hdr *)pkt;
}

void rxr_pkt_calc_cts_window_credits(struct rxr_ep *ep, struct rxr_peer *peer,
				     uint64_t size, int request,
				     int *window, int *credits);

ssize_t rxr_pkt_init_cts(struct rxr_ep *ep,
			 struct rxr_rx_entry *rx_entry,
			 struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_cts_sent(struct rxr_ep *ep,
			     struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_cts_recv(struct rxr_ep *ep,
			     struct rxr_pkt_entry *pkt_entry);

/*
 *  DATA packet data structures and functions
 *  Definition of the functions is in rxr_pkt_data.c
 */
struct rxr_data_hdr {
	uint8_t type;
	uint8_t version;
	uint16_t flags;
	/* end of rxr_base_hdr */
	/* TODO: need to add msg_id -> tx_id/rx_id mapping */
	uint32_t rx_id;
	uint64_t seg_size;
	uint64_t seg_offset;
};

#if defined(static_assert) && defined(__x86_64__)
static_assert(sizeof(struct rxr_data_hdr) == 24, "rxr_data_hdr check");
#endif

#define RXR_DATA_HDR_SIZE		(sizeof(struct rxr_data_hdr))

struct rxr_data_pkt {
	struct rxr_data_hdr hdr;
	char data[];
};

static inline
struct rxr_data_pkt *rxr_get_data_pkt(void *pkt)
{
	return (struct rxr_data_pkt *)pkt;
}

ssize_t rxr_pkt_send_data(struct rxr_ep *ep,
			  struct rxr_tx_entry *tx_entry,
			  struct rxr_pkt_entry *pkt_entry);

ssize_t rxr_pkt_send_data_mr_cache(struct rxr_ep *ep,
				   struct rxr_tx_entry *tx_entry,
				   struct rxr_pkt_entry *pkt_entry);

int rxr_pkt_proc_data(struct rxr_ep *ep,
		      struct rxr_rx_entry *rx_entry,
		      struct rxr_pkt_entry *pkt_entry,
		      char *data, size_t seg_offset,
		      size_t seg_size);

void rxr_pkt_handle_data_send_completion(struct rxr_ep *ep,
					 struct rxr_pkt_entry *pkt_entry);


void rxr_pkt_handle_data_recv(struct rxr_ep *ep,
			      struct rxr_pkt_entry *pkt_entry);

/*
 *  READRSP packet data structures and functions
 *  The definition of functions are in rxr_pkt_type_misc.c
 */
struct rxr_readrsp_hdr {
	uint8_t type;
	uint8_t version;
	uint16_t flags;
	/* end of rxr_base_hdr */
	uint8_t pad[4];
	uint32_t rx_id;
	uint32_t tx_id;
	uint64_t seg_size;
};

static inline struct rxr_readrsp_hdr *rxr_get_readrsp_hdr(void *pkt)
{
	return (struct rxr_readrsp_hdr *)pkt;
}

#define RXR_READRSP_HDR_SIZE	(sizeof(struct rxr_readrsp_hdr))

#if defined(static_assert) && defined(__x86_64__)
static_assert(sizeof(struct rxr_readrsp_hdr) == sizeof(struct rxr_data_hdr), "rxr_readrsp_hdr check");
#endif

struct rxr_readrsp_pkt {
	struct rxr_readrsp_hdr hdr;
	char data[];
};

int rxr_pkt_init_readrsp(struct rxr_ep *ep,
			 struct rxr_tx_entry *tx_entry,
			 struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_readrsp_sent(struct rxr_ep *ep,
				 struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_readrsp_send_completion(struct rxr_ep *ep,
					    struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_readrsp_recv(struct rxr_ep *ep,
				 struct rxr_pkt_entry *pkt_entry);

/*
 *  RMA context packet, used to differentiate the normal RMA read, normal RMA
 *  write, and the RMA read in two-sided large message transfer
 *  Implementation of the function is in rxr_pkt_type_misc.c
 */
struct rxr_rma_context_pkt {
	uint8_t type;
	uint8_t version;
	uint16_t flags;
	/* end of rxr_base_hdr */
	uint32_t context_type;
	uint32_t tx_id; /* used by write context */
	uint32_t read_id; /* used by read context */
	size_t seg_size; /* used by read context */
};

enum rxr_rma_context_pkt_type {
	RXR_READ_CONTEXT = 1,
	RXR_WRITE_CONTEXT,
};

void rxr_pkt_init_write_context(struct rxr_tx_entry *tx_entry,
				struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_init_read_context(struct rxr_ep *rxr_ep,
			       struct rxr_read_entry *read_entry,
			       size_t seg_size,
			       struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_rma_completion(struct rxr_ep *ep,
				   struct rxr_pkt_entry *pkt_entry);

/*
 *  EOR packet, used to acknowledge the sender that large message
 *  copy has been finished.
 *  Implementaion of the functions are in rxr_pkt_misc.c
 */
struct rxr_eor_hdr {
	uint8_t type;
	uint8_t version;
	uint16_t flags;
	/* end of rxr_base_hdr */
	uint32_t tx_id;
	uint32_t rx_id;
};

#if defined(static_assert) && defined(__x86_64__)
static_assert(sizeof(struct rxr_eor_hdr) == 12, "rxr_eor_hdr check");
#endif

int rxr_pkt_init_eor(struct rxr_ep *ep,
		     struct rxr_rx_entry *rx_entry,
		     struct rxr_pkt_entry *pkt_entry);

static inline
void rxr_pkt_handle_eor_sent(struct rxr_ep *ep, struct rxr_pkt_entry *pkt_entry)
{
}

void rxr_pkt_handle_eor_send_completion(struct rxr_ep *ep,
					struct rxr_pkt_entry *pkt_entry);

void rxr_pkt_handle_eor_recv(struct rxr_ep *ep,
			     struct rxr_pkt_entry *pkt_entry);

/*
 * Control header without completion data. We will send more data with the RTS
 * packet if RXR_REMOTE_CQ_DATA is not set.
 */
struct rxr_ctrl_hdr {
	union {
		struct rxr_base_hdr base_hdr;
		struct rxr_rts_hdr rts_hdr;
		struct rxr_connack_hdr connack_hdr;
		struct rxr_cts_hdr cts_hdr;
	};
};

#define RXR_CTRL_HDR_SIZE              (sizeof(struct rxr_ctrl_cq_hdr))

struct rxr_ctrl_pkt {
	struct rxr_ctrl_hdr hdr;
	char data[];
};

/*
 * Control header with completion data. CQ data length is static.
 */
#define RXR_CQ_DATA_SIZE (8)
struct rxr_ctrl_cq_hdr {
	union {
		struct rxr_base_hdr base_hdr;
		struct rxr_rts_hdr rts_hdr;
		struct rxr_connack_hdr connack_hdr;
		struct rxr_cts_hdr cts_hdr;
	};
	uint64_t cq_data;
};

#define RXR_CTRL_HDR_SIZE_NO_CQ                (sizeof(struct rxr_ctrl_hdr))

struct rxr_ctrl_cq_pkt {
	struct rxr_ctrl_cq_hdr hdr;
	char data[];
};

#endif

#include "rxr_pkt_type_req.h"