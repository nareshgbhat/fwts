/*
 * Copyright (C) 2010-2017 Canonical
 * Copyright (C) 2017 ARM Ltd
 * Portions of this code original from the Linux-ready Firmware Developer Kit
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "fwts.h"

#if defined(FWTS_HAS_SBBR)

#include "fwts_acpi_object_eval.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>

static const fwts_acpi_table_fadt *fadt;
static int fadt_size;

static int sbbr_fadt_init(fwts_framework *fw)
{
	fwts_acpi_table_info *table;

	if (fwts_acpi_find_table(fw, "FACP", 0, &table) != FWTS_OK) {
		fwts_log_error(fw, "Cannot read ACPI table FACP.");
		return FWTS_ERROR;
	}
	if (table == NULL) {
		fwts_log_error(fw, "ACPI table FACP does not exist!");
		return FWTS_ERROR;
	}
	fadt = (const fwts_acpi_table_fadt *)table->data;
	fadt_size = table->length;

	/*  Not having a FADT is a failure on ARM SBBR Architecture */
	if (fadt_size == 0) {
    	fwts_log_error(fw, "ACPI table FACP has zero length!");
		return FWTS_ERROR;
	}

	return FWTS_OK;
}

static int sbbr_fadt_revision_test1(fwts_framework *fw)
{
	const uint8_t SBBR_LATEST_MAJOR = 6;
	const uint8_t SBBR_LATEST_MINOR = 0;
	uint8_t major;
	uint8_t minor;

	major = fadt->header.revision;
	minor = 0;
	if (major >= 5 && fadt->header.length >= 268)
		minor = fadt->minor_version;   /* field added ACPI 5.1 */

	fwts_log_info(fw, "FADT revision: %d.%d", major, minor);

	if (major >= SBBR_LATEST_MAJOR && minor >= SBBR_LATEST_MINOR)
		fwts_passed(fw, "FADT revision is up to date.");
	else {
		fwts_failed(fw, LOG_LEVEL_CRITICAL, "fadt_revision:", "FADT revision is outdated: %d.%d",
			     major, minor);
	}

	return FWTS_OK;
}

static int sbbr_fadt_reduced_hw_test2(fwts_framework *fw)
{
	bool rhw;
	bool passed;
	static const fwts_acpi_gas null_gas;
	uint32_t flag_mask;

	rhw = fwts_acpi_is_reduced_hardware(fadt);
	if (rhw == 0)
		fwts_failed(fw, LOG_LEVEL_CRITICAL, "fadt_reduced_hw:", "FADT indicates ACPI is not in reduced hardware mode.");
	else
		fwts_passed(fw, "FADT indicates ACPI is in reduced hardware mode.");


	if (!rhw)
		return FWTS_OK;

	passed = true;

	/* check all the fields that will be ignored */
	/* if the HW_REDUCED_ACPI flag in the table is set,
	   OSPM will ignore fields related to the ACPI HW register interface:
	   Fields at offsets 46 through 108 and 148 through 232, as well as
	   FADT Flag bits 1, 2, 3,7,8,12,13, 14, 16 and 17 */

	if (fadt->sci_int != 0) {
		passed = false;
		fwts_log_warning(fw, "SCI_INT is non-zero: 0x%x",
			      fadt->smi_cmd);
	}
	if (fadt->smi_cmd != 0) {
		passed = false;
		fwts_log_warning(fw, "SMI_CMD is non-zero: 0x%x",
			      fadt->smi_cmd);
	}
	if (fadt->acpi_enable != 0) {
		passed = false;
		fwts_log_warning(fw, "ACPI_ENABLE is non-zero: 0x%x",
			      fadt->acpi_enable);
	}
	if (fadt->acpi_disable != 0) {
		passed = false;
		fwts_log_warning(fw, "ACPI_DISABLE is non-zero: 0x%x",
			      fadt->acpi_disable);
	}
	if (fadt->s4bios_req != 0) {
		passed = false;
		fwts_log_warning(fw, "S4BIOS_REQ is non-zero: 0x%x",
			      fadt->s4bios_req);
	}
	if (fadt->pstate_cnt != 0) {
		passed = false;
		fwts_log_warning(fw, "PSTATE_CNT is non-zero: 0x%x",
			      fadt->pstate_cnt);
	}
	if (fadt->pm1a_evt_blk != 0) {
		passed = false;
		fwts_log_warning(fw, "PM1A_EVT_BLK is non-zero: 0x%x",
			      fadt->pm1a_evt_blk);
	}
	if (fadt->pm1b_evt_blk != 0) {
		passed = false;
		fwts_log_warning(fw, "PM1B_EVT_BLK is non-zero: 0x%x",
			      fadt->pm1b_evt_blk);
	}
	if (fadt->pm1a_cnt_blk != 0) {
		passed = false;
		fwts_log_warning(fw, "PM1A_CNT_BLK is non-zero: 0x%x",
			      fadt->pm1a_cnt_blk);
	}
	if (fadt->pm1b_cnt_blk != 0) {
		passed = false;
		fwts_log_warning(fw, "PM1B_CNT_BLK is non-zero: 0x%x",
			      fadt->pm1b_cnt_blk);
	}
	if (fadt->pm2_cnt_blk != 0) {
		passed = false;
		fwts_log_warning(fw, "PM2_CNT_BLK is non-zero: 0x%x",
			      fadt->pm2_cnt_blk);
	}
	if (fadt->pm_tmr_blk != 0) {
		passed = false;
		fwts_log_warning(fw, "PM_TMR_BLK is non-zero: 0x%x",
			      fadt->pm_tmr_blk);
	}
	if (fadt->gpe0_blk != 0) {
		passed = false;
		fwts_log_warning(fw, "GPE0_BLK is non-zero: 0x%x",
			      fadt->gpe0_blk);
	}
	if (fadt->gpe1_blk != 0) {
		passed = false;
		fwts_log_warning(fw, "GPE1_BLK is non-zero: 0x%x",
			      fadt->gpe1_blk);
	}
	if (fadt->pm1_evt_len != 0) {
		passed = false;
		fwts_log_warning(fw, "PM1_EVT_LEN is non-zero: 0x%x",
			      fadt->pm1_evt_len);
	}
	if (fadt->pm1_cnt_len != 0) {
		passed = false;
		fwts_log_warning(fw, "PM1_CNT_LEN is non-zero: 0x%x",
			      fadt->pm1_cnt_len);
	}
	if (fadt->pm2_cnt_len != 0) {
		passed = false;
		fwts_log_warning(fw, "PM2_CNT_LEN is non-zero: 0x%x",
			      fadt->pm2_cnt_len);
	}
	if (fadt->pm_tmr_len != 0) {
		passed = false;
		fwts_log_warning(fw, "PM_TMR_LEN is non-zero: 0x%x",
			      fadt->pm_tmr_len);
	}
	if (fadt->gpe0_blk_len != 0) {
		passed = false;
		fwts_log_warning(fw, "GPE0_BLK_LEN is non-zero: 0x%x",
			      fadt->gpe0_blk_len);
	}
	if (fadt->gpe1_blk_len != 0) {
		passed = false;
		fwts_log_warning(fw, "GPE1_BLK_LEN is non-zero: 0x%x",
			      fadt->gpe1_blk_len);
	}
	if (fadt->gpe1_base != 0) {
		passed = false;
		fwts_log_warning(fw, "GPE1_BASE is non-zero: 0x%x",
			      fadt->gpe1_base);
	}
	if (fadt->cst_cnt != 0) {
		passed = false;
		fwts_log_warning(fw, "CST_CNT is non-zero: 0x%x",
			      fadt->cst_cnt);
	}
	if (fadt->p_lvl2_lat != 0) {
		passed = false;
		fwts_log_warning(fw, "P_LVL2_LAT is non-zero: 0x%x",
			      fadt->p_lvl2_lat);
	}
	if (fadt->p_lvl3_lat != 0) {
		passed = false;
		fwts_log_warning(fw, "P_LVL3_LAT is non-zero: 0x%x",
			      fadt->p_lvl3_lat);
	}
	if (fadt->flush_size != 0) {
		passed = false;
		fwts_log_warning(fw, "FLUSH_SIZE is non-zero: 0x%x",
			      fadt->flush_size);
	}
	if (fadt->flush_stride != 0) {
		passed = false;
		fwts_log_warning(fw, "FLUSH_STRIDE is non-zero: 0x%x",
			      fadt->flush_stride);
	}
	if (fadt->duty_offset != 0) {
		passed = false;
		fwts_log_warning(fw, "DUTY_OFFSET is non-zero: 0x%x",
			      fadt->duty_offset);
	}
	if (fadt->duty_width != 0) {
		passed = false;
		fwts_log_warning(fw, "DUTY_WIDTH is non-zero: 0x%x",
			      fadt->duty_width);
	}
	if (fadt->day_alrm != 0) {
		passed = false;
		fwts_log_warning(fw, "DAY_ALRM is non-zero: 0x%x",
			      fadt->day_alrm);
	}
	if (fadt->mon_alrm != 0) {
		passed = false;
		fwts_log_warning(fw, "MON_ALRM is non-zero: 0x%x",
			      fadt->mon_alrm);
	}
	if (fadt->century != 0) {
		passed = false;
		fwts_log_warning(fw, "CENTURY is non-zero: 0x%x",
			      fadt->century);
	}
	if (memcmp((void *)&fadt->x_pm1a_evt_blk,
		   (void *)&null_gas,
		   sizeof(fwts_acpi_gas))) {
		passed = false;
		fwts_log_warning(fw,
			      "X_PM1A_EVT_BLK is a non-zero general "
			      "address structure.");
	}
	if (memcmp((void *)&fadt->x_pm1b_evt_blk,
		   (void *)&null_gas,
		   sizeof(fwts_acpi_gas))) {
		passed = false;
		fwts_log_warning(fw,
			      "X_PM1B_EVT_BLK is a non-zero general "
			      "address structure.");
	}
	if (memcmp((void *)&fadt->x_pm1a_cnt_blk,
		   (void *)&null_gas,
		   sizeof(fwts_acpi_gas))) {
		passed = false;
		fwts_log_warning(fw,
			      "X_PM1A_CNT_BLK is a non-zero general "
			      "address structure.");
	}
	if (memcmp((void *)&fadt->x_pm1b_cnt_blk,
		   (void *)&null_gas,
		   sizeof(fwts_acpi_gas))) {
		passed = false;
		fwts_log_warning(fw,
			      "X_PM1B_CNT_BLK is a non-zero general "
			      "address structure.");
	}
	if (memcmp((void *)&fadt->x_pm2_cnt_blk,
		   (void *)&null_gas,
		   sizeof(fwts_acpi_gas))) {
		passed = false;
		fwts_log_warning(fw,
			      "X_PM2_CNT_BLK is a non-zero general "
			      "address structure.");
	}
	if (memcmp((void *)&fadt->x_pm_tmr_blk,
		   (void *)&null_gas,
		   sizeof(fwts_acpi_gas))) {
		passed = false;
		fwts_log_warning(fw,
			      "X_PM_TMR_BLK is a non-zero general "
			      "address structure.");
	}
	if (memcmp((void *)&fadt->x_gpe0_blk,
		   (void *)&null_gas,
		   sizeof(fwts_acpi_gas))) {
		passed = false;
		fwts_log_warning(fw,
			      "X_GPE0_BLK is a non-zero general "
			      "address structure.");
	}
	if (memcmp((void *)&fadt->x_gpe1_blk,
		   (void *)&null_gas,
		   sizeof(fwts_acpi_gas))) {
		passed = false;
		fwts_log_warning(fw,
			      "X_GPE1_BLK is a non-zero general "
			      "address structure.");
	}

	if (passed)
		fwts_passed(fw, "All FADT reduced hardware fields are zero.");
	else
		fwts_failed(fw, LOG_LEVEL_CRITICAL, "fadt_reduced_hw:",
			    "Some FADT reduced hardware fields are non-zero, but it will be ignored by OSPM.");

	/* now check all the reserved flags */
	flag_mask = FWTS_FACP_FLAG_WBINVD_FLUSH		|
		    FWTS_FACP_FLAG_PROC_C1		|
		    FWTS_FACP_FLAG_P_LVL2_UP		|
		    FWTS_FACP_FLAG_RTC_S4		|
		    FWTS_FACP_FLAG_TMR_VAL_EXT		|
		    FWTS_FACP_FLAG_CPU_SW_SLP		|
		    FWTS_FACP_FLAG_PCI_EXP_WAK		|
		    FWTS_FACP_FLAG_S4_RTC_STS_VALID	|
		    FWTS_FACP_FLAG_REMOTE_POWER_ON_CAPABLE;

	if (fadt->flags & flag_mask)
		fwts_failed(fw, LOG_LEVEL_CRITICAL, "fadt_reduced_hw:",
			    "Some FADT reduced hardware flags are set, but it will be ignored by OSPM.");
	else
		fwts_passed(fw, "All FADT reduced hardware flags are not set.");


	if ((fadt->flags & FWTS_FACP_FLAG_FORCE_APIC_CLUSTER_MODEL) ||
	    (fadt->flags & FWTS_FACP_FLAG_FORCE_APIC_PHYSICAL_DESTINATION_MODE))
		fwts_failed(fw, LOG_LEVEL_CRITICAL,
			    "fadt_reduced_hw:",
			    "FADT APIC flags are set for reduced hardware "
			    "mode but may be irrelevant.");
	else
		fwts_passed(fw,
			    "FADT APIC flags are not set in reduced "
			    "hardware mode.");

	return FWTS_OK;
}

static int sbbr_fadt_profile_test3(fwts_framework *fw)
{
	const uint8_t SBBR_ENT_SERVER   = 4;
	const uint8_t SBBR_SOHO_SERVER  = 5;
	const uint8_t SBBR_PERF_SERVER  = 7;
	fwts_log_info(fw, "FADT Preferred PM Profile: %hhu (%s)\n",
		fadt->preferred_pm_profile,
		FWTS_ACPI_FADT_PREFERRED_PM_PROFILE(fadt->preferred_pm_profile));

	if ( (fadt->preferred_pm_profile == SBBR_ENT_SERVER)  ||
		 (fadt->preferred_pm_profile == SBBR_SOHO_SERVER) ||
		 (fadt->preferred_pm_profile == SBBR_PERF_SERVER) )
		fwts_passed(fw, "FADT has a recommended server PM profile.");
	else
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "fadt_profile:", "FADT preferred PM profile is not recommended.");

	return FWTS_OK;
}

static int sbbr_fadt_boot_arch_psci_compliant_test4(fwts_framework *fw)
{
	/* ARM SBBR 4.2.1.3 FADT */
	if (fadt->arm_boot_flags &
		FWTS_FACP_ARM_BOOT_ARCH_PSCI_COMPLIANT)
		  fwts_passed(fw, "PSCI_COMPLIANT is set, PSCI is implemented.");
	else
	  fwts_failed(fw, LOG_LEVEL_CRITICAL,
	      "ARCH_PSCI_COMPLIANT:",
	      "PSCI is NOT implemented.");

	return FWTS_OK;
}

static fwts_framework_minor_test sbbr_fadt_tests[] = {
	{ sbbr_fadt_revision_test1, "FADT Revision Test." },
	{ sbbr_fadt_reduced_hw_test2, "FADT Reduced HW Test." },
	{ sbbr_fadt_profile_test3, "FADT Server Profile Test." },
	{ sbbr_fadt_boot_arch_psci_compliant_test4, "FADT PSCI Compliant Test." },
	{ NULL, NULL }
};

static fwts_framework_ops sbbr_fadt_ops = {
	.description = "SBBR FADT Fixed ACPI Description Table tests.",
	.init        = sbbr_fadt_init,
	.minor_tests = sbbr_fadt_tests
};

FWTS_REGISTER("sbbr_fadt", &sbbr_fadt_ops, FWTS_TEST_ANYTIME, FWTS_FLAG_TEST_SBBR)
#endif