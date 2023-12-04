/* SPDX-License-Identifier: GPL-2.0 */
/* X-SPDX-Copyright-Text: (c) Copyright 2009-2020 Xilinx, Inc. */
#ifndef __ONLOAD_OOF_HW_FILTER_H__
#define __ONLOAD_OOF_HW_FILTER_H__

#include <ci/internal/transport_config_opt.h>

struct tcp_helper_resource_s;
struct tcp_helper_cluster_s;


struct oo_hw_filter {
  struct tcp_helper_resource_s* trs;
  struct tcp_helper_cluster_s*  thc;
  unsigned dlfilter_handle;
#if CI_CFG_TCP_OFFLOAD_RECYCLER
  int plugin_vi;
#endif
  int filter_id[CI_CFG_MAX_HWPORTS];
};


#endif  /* __ONLOAD_OOF_HW_FILTER_H__ */
