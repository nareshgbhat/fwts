/*
 * Copyright (C) 2015-2017 Canonical
 * Copyright (C) 2017 ARM Ltd
 *
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>

static const fwts_acpi_table_spcr *spcr;

static int sbbr_spcr_init(fwts_framework *fw)
{
	fwts_acpi_table_info *table;

	if (fwts_acpi_find_table(fw, "SPCR", 0, &table) != FWTS_OK) {
		fwts_log_error(fw, "Cannot read ACPI tables.");
		return FWTS_ERROR;
	}
	if (table == NULL || (table && table->length == 0)) {
		fwts_log_error(fw, "ACPI SPCR table does not exist");
		return FWTS_ERROR;
	}
	spcr = (const fwts_acpi_table_spcr*)table->data;

	return FWTS_OK;
}

/*
 *  For SPCR and serial port types refer to:
 *	https://msdn.microsoft.com/en-gb/library/windows/hardware/dn639132%28v=vs.85%29.aspx
 *	https://msdn.microsoft.com/en-us/library/windows/hardware/dn639131%28v=vs.85%29.aspx
 */
static int sbbr_spcr_revision_test(fwts_framework *fw)
{
	const uint8_t SBBR_LATEST_REVISION = 2;

	if (spcr->header.revision >= SBBR_LATEST_REVISION)
		fwts_passed(fw, "SPCR revision is up to date.");
	else
		fwts_failed(fw, LOG_LEVEL_CRITICAL, "spcr_revision:", "SPCR revision is outdated: %d",
				spcr->header.revision);

	return FWTS_OK;
}

static int sbbr_spcr_gsiv_test(fwts_framework *fw)
{
	const uint8_t ARMH_GIC_INTR_MASK = 0x08;

	if ( (spcr->interrupt_type == ARMH_GIC_INTR_MASK) &&
	     (spcr->gsi            != 0x0000000000000000)    )
		fwts_passed(fw, "SPCR appears to be populated with correct GSIV interrupt"
				        "routing information for ARM PL011 UART Device");

	else
		fwts_failed(fw, LOG_LEVEL_CRITICAL, "sbbr_gsiv:", "SPCR GSIV Information is set incorrectly.");

	return FWTS_OK;

}


static int sbbr_spcr_generic_test(fwts_framework *fw)
{
	char *str;
	bool reserved = false;
	bool pci = true;
	bool passed = true;

	/*
	 * Assuming revision 2, full list from
	 * http://go.microsoft.com/fwlink/p/?LinkId=234837)
	 */
	switch (spcr->interface_type) {
	case 0x00:
		str = "16550 compatible";
		break;
	case 0x01:
		str = "16450 compatible";
		break;
	case 0x03:
		str = "ARM PL011 UART";
		break;
	case 0x02:
	case 0x04 ... 0x09:
		str = "Reserved (Do not Use)";
		reserved = true;
		break;
	case 0x0d:
		str = "(deprecated) ARM SBSA";
		break;
	case 0x0e:
		str = "ARM SBSA Generic UART";
		break;
	case 0x0f:
		str = "ARM DCC";
		break;
	case 0x10:
		str = "BCM2835";
		break;
	default:
		str = "Reserved";
		reserved = true;
		break;
	}

	fwts_log_info_verbatim(fw, "Serial Interface: %s", str);
	if (reserved) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRInterfaceReserved",
			"SPCR Serial interface type 0x%2.2" PRIx8
			" is a reserved interface", spcr->interface_type);
	}

	if ((spcr->reserved1[0] |
	     spcr->reserved1[1] |
	     spcr->reserved1[2])) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_LOW,
			"SPCRReservedNonZero",
			"SPCR reserved field must be zero, got "
			"0x%2.2" PRIx8 "%2.2" PRIx8 "%2.2" PRIx8 " instead",
				spcr->reserved1[0],
				spcr->reserved1[1],
				spcr->reserved1[2]);
	}

	if (spcr->interrupt_type == 0) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRUnknownInterruptType",
			"SPCR interrupt type field is zero, expecting support bits to be set");
	}
	if (spcr->interrupt_type & 0xf0) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRIllegalReservedInterruptType",
			"SPCR interrupt type reserved bits are non-zero zero, got 0x%" PRIx8,
				spcr->interrupt_type);
	}

	/* Check PC-AT compatible UART IRQs */
	if (spcr->interrupt_type & 1) {
		switch (spcr->irq) {
		case  2 ...  7:
		case  9 ... 12:
		case 14 ... 15:
			break;
		default:
			passed = false;
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"SPCRIllegalIRQ",
				"SPCR PC-AT compatible IRQ 0x%" PRIx8 " is invalid", spcr->irq);
			break;
		}
	}

	reserved = false;
	switch (spcr->baud_rate) {
	case 0x03:
		str = "9600";
		break;
	case 0x04:
		str = "19200";
		break;
	case 0x06:
		str = "57600";
		break;
	case 0x07:
		str = "115200";
		break;
	default:
		str = "Reserved";
		reserved = true;
	}
	fwts_log_info_verbatim(fw, "Baud Rate:        %s", str);
	if (reserved) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRBaudRateReserved",
			"SPCR Serial baud rate type 0x%2.2" PRIx8
			" is a reserved baud rate", spcr->baud_rate);
	}

	if (spcr->parity & ~1) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRParityReserved",
			"SPCR parity reserved bits 1..7 are non-zero, value is 0x%2.2" PRIx8,
			spcr->parity);
	}

	/* Stop bit *really* is bit 1 according to the spec */
	if (spcr->parity & ~2) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRStopBitReserved",
			"SPCR stop bit reserved bits 0,2..7 are non-zero, value is 0x%2.2" PRIx8,
			spcr->parity);
	}

	if (spcr->flow_control & ~7) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRFlowControlReserved",
			"SPCR flow control reserved bits 3..7 are non-zero, value is 0x%2.2" PRIx8,
			spcr->flow_control);
	}

	reserved = false;
	switch (spcr->terminal_type) {
	case 0x00:
		str = "VT100";
		break;
	case 0x01:
		str = "VT100+";
		break;
	case 0x02:
		str = "VT-UTF8";
		break;
	case 0x03:
		str = "ANSI";
		break;
	default:
		str = "Reserved";
		reserved = true;
	}
	fwts_log_info_verbatim(fw, "Terminal Type:    %s", str);
	if (reserved) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRTerminalTypeReserved",
			"SPCR terminal type type 0x%2.2" PRIx8
			" is a reserved terminal type", spcr->terminal_type);
	}

	if (spcr->reserved2) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_LOW,
			"SPCRReservedNonZero",
			"SPCR reserved field must be zero, got "
			"0x%2.2" PRIx8 " instead", spcr->reserved2);
	}

	/* According to the spec, these values indicate NOT a PCI device */
	if ((spcr->pci_device_id == 0xffff) &&
	    (spcr->pci_vendor_id == 0xffff) &&
	    (spcr->pci_bus_number == 0) &&
	    (spcr->pci_device_number == 0) &&
	    (spcr->pci_function_number == 0))
		pci = false;

	/* Now validate all pci specific fields if not-PCI enabled */
	if (pci) {
		if (spcr->pci_device_id == 0xffff) {
			passed = false;
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"SPCRPciDeviceID",
				"SPCR PCI device ID is 0x%4.4" PRIx16
				", expecting non-0xffff for PCI device",
				spcr->pci_device_id);
		}
		if (spcr->pci_vendor_id == 0xffff) {
			passed = false;
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"SPCRPciVendorID",
				"SPCR PCI vendor ID is 0x%4.4" PRIx16
				", expecting non-0xffff for non-PCI device",
				spcr->pci_vendor_id);
		}
		if ((spcr->pci_flags & 1) == 0) {
			passed = false;
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"SPCRPciFlagsBit0",
				"SPCR PCI flags compatibility bit 0 is %" PRIx32
				", expecting 1 for PCI device",
				spcr->pci_flags & 1);
		}
	}

	if (spcr->pci_flags & 0xfffe) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"SPCRPciFlags",
			"SPCR PCI flags reserved bits 1-31 are %" PRIx32
			", expecting 0",
			spcr->pci_flags);
	}

	if (spcr->reserved3) {
		passed = false;
		fwts_failed(fw, LOG_LEVEL_LOW,
			"SPCRReservedNonZero",
			"SPCR reserved field must be zero, got "
			"0x%2.2" PRIx8 " instead", spcr->reserved3);
	}

	if (passed)
		fwts_passed(fw, "No issues found in SPCR table.");

	return FWTS_OK;
}

static fwts_framework_minor_test sbbr_spcr_tests[] = {
	{ sbbr_spcr_revision_test, "SPCR Revision Test." },
	{ sbbr_spcr_gsiv_test, "SPCR GSIV Interrupt Test." },
	{ sbbr_spcr_generic_test, "SPCR Serial Port Console Redirection Table test." },
	{ NULL, NULL }
};

static fwts_framework_ops sbbr_spcr_ops = {
	.description = "SPCR Serial Port Console Redirection Table test.",
	.init        = sbbr_spcr_init,
	.minor_tests = sbbr_spcr_tests
};

FWTS_REGISTER("sbbr_spcr", &sbbr_spcr_ops, FWTS_TEST_ANYTIME, FWTS_FLAG_TEST_SBBR)

#endif
