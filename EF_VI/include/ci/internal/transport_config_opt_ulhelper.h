/* SPDX-License-Identifier: GPL-2.0 */
/* X-SPDX-Copyright-Text: (c) Copyright 2019-2020 Xilinx, Inc. */
#ifndef __CI_INTERNAL_TRANSPORT_CONFIG_OPT_ULHELPER_H__
#define __CI_INTERNAL_TRANSPORT_CONFIG_OPT_ULHELPER_H__

/* Use default values for the most of options: */
#include <ci/internal/transport_config_opt_extra.h>

#define ONLOAD_BUILD_PROFILE "ulhelper"

#undef CI_CFG_UL_INTERRUPT_HELPER
#define CI_CFG_UL_INTERRUPT_HELPER 1

/* This mode does not support some features for now. */
#undef CI_CFG_FD_CACHING
#define CI_CFG_FD_CACHING 0
#undef CI_CFG_ENDPOINT_MOVE
#define CI_CFG_ENDPOINT_MOVE 0
#undef CI_CFG_EPOLL2
#define CI_CFG_EPOLL2 0
#undef CI_CFG_EPOLL3
#define CI_CFG_EPOLL3 0
#undef CI_CFG_WANT_BPF_NATIVE
#define CI_CFG_WANT_BPF_NATIVE 0
#undef CI_CFG_INJECT_PACKETS
#define CI_CFG_INJECT_PACKETS 0
#undef CI_CFG_NIC_RESET_SUPPORT
#define CI_CFG_NIC_RESET_SUPPORT 0
#undef CI_CFG_HANDLE_ICMP
#define CI_CFG_HANDLE_ICMP 0
#undef CI_CFG_TCP_SHARED_LOCAL_PORTS
#define CI_CFG_TCP_SHARED_LOCAL_PORTS 0

/* See also BREAK_SCALABLE_FILTERS in src/lib/efthrm/tcp_helper_endpoint.c */

#endif
