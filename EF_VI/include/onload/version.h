/* SPDX-License-Identifier: GPL-2.0 OR BSD-2-Clause */
/* X-SPDX-Copyright-Text: (c) Copyright 2009-2020 Xilinx, Inc. */
/**************************************************************************\
*//*! \file
** <L5_PRIVATE L5_HEADER >
** \author  djr
**  \brief  Onload version.
**   \date  2009/07/22
**    \cop  (c) Solarflare Communications Inc.
** </L5_PRIVATE>
*//*
\**************************************************************************/

#ifndef __ONLOAD_VERSION_H__
#define __ONLOAD_VERSION_H__


/* onload_version and onload_version_private are the same data, but _private
 * is not exported so can't be overridden by the dynamic linker so has the
 * correct value at early startup */
extern const char* onload_version;
extern const char onload_version_private[];
extern const char onload_short_version[];
extern const char onload_product[];
extern const char onload_copyright[];

/* Max length of version string used for version skew checking. */
enum { OO_VER_STR_LEN = 80 };


/* We use an md5sum over certain headers to ensure that userland and kernel
 * drivers are built against a compatible interface.
 */
enum { CI_CHSUM_STR_LEN = 32 };


typedef struct oo_version_check_s {
  char                    in_version[OO_VER_STR_LEN + 1];
  char                    in_uk_intf_ver[CI_CHSUM_STR_LEN + 1];
  int32_t                 debug;
} oo_version_check_t;


#endif  /* __ONLOAD_VERSION_H__ */
