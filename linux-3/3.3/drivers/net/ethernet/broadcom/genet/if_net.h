/*
 *
 * Copyright (c) 2002-2005 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *if_net.h
 * Purpose: network ioctl function data definations.
*/
#ifndef _IF_NET_H_
#define _IF_NET_H_

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Ioctl definitions.
 */
/* go into ACPI WoL mode*/
#define SIOCSACPISET		(SIOCDEVPRIVATE + 2)
/* cancel ACPI WoL Mode */
#define SIOCSACPICANCEL		(SIOCDEVPRIVATE + 3)
/* get ACPI pattern */
#define SIOCGPATTERN		(SIOCDEVPRIVATE + 4)
/* set ACPI pattern */
#define SIOCSPATTERN		(SIOCDEVPRIVATE + 5)
#if defined(CONFIG_TIVO_GEN10)
/* Get atheros switch register */
#define BCMIOGSWREG		(SIOCDEVPRIVATE + 6)
/* Set atheros switch register */
#define BCMIOSSWREG		(SIOCDEVPRIVATE + 7)

struct athsw_data {
	int  address;
	int  val_in;
	int  val_out;
};

#endif
#ifdef CONFIG_PACE_BCMGENET_CABLE_DETECT

/* Support for enabling/disabling cable insertion detection: two additional ioctls to
   allow the cable insertion detection at the PHY to be either enabled or disabled.
   Intended for testing purposes only */

/* Set the cable in/out detection interrupt flags */
#define SIOCSDETECTCABLESET      (SIOCDEVPRIVATE + 6)

/* Unset (clear) the cable in/out detection interrupt flags */
#define SIOCSDETECTCABLEUNSET    (SIOCDEVPRIVATE + 7)

/* Force the carrier on */
#define SIOCSFORCECARRIERON      (SIOCDEVPRIVATE + 8)

/* Force the carrier off */
#define SIOCSFORCECARRIEROFF     (SIOCDEVPRIVATE + 9)

/* Set activity LED to a particular state/behavior */
#define SIOCSLEDSTATE            (SIOCDEVPRIVATE + 10)

#endif /* #ifdef CONFIG_PACE_BCMGENET_CABLE_DETECT */

/*
 * ACPI pattern (or generic HFB ) data
 */
union acpi_pattern {
	unsigned long value;
	struct {
#ifdef __BIG_ENDIAN
		unsigned long unused:12;
		unsigned long mask:4;
		unsigned long data:16;
#else
		unsigned long data:16;
		unsigned long mask:4;
		unsigned long unused:12;
#endif
	} pattern;
};

/* used by ioctl call */
struct acpi_data {
	/* current filter index */
	int fltr_index;
	/* pointer to the pattern data.*/
	union acpi_pattern *p_data;
	/* total count of the data */
	int count;
};

#ifdef __cplusplus
}
#endif

#endif /* _IF_NET_H_ */
