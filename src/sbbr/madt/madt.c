/*
 * Copyright (C) 2015-2017 Canonical
 * Copyright (C) 2017 ARM Ltd
 * Portions added (c) 2015, Al Stone <ahs3@redhat.com>
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
 */
#include "fwts.h"

#if defined(FWTS_HAS_SBBR)

#include "fwts_acpi_object_eval.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

/*
 * The Long, Sad, True Story of the MADT
 *
 * Once upon a time in ACPI 1.0, there was the MADT.  It was a nice table,
 * and it had two subtables all of its own.  But, it was also a pretty
 * busy table, too, so over time the MADT gathered up other nice little
 * subtables.  By the time ACPI 6.0 came around, the MADT had 16 of the
 * little guys.
 *
 * Now, the MADT kept a little counter around for the subtables.  In fact,
 * it kept two counters: one was the revision level, which was supposed to
 * change when new subtables came to be, or as the ones already around grew
 * up. The second counter was a type number, because the MADT needed a unique
 * type for each subtable so he could tell them apart.  But, sometimes the
 * MADT got so busy, he forgot to increment the revision level when he needed
 * to.  Fortunately, the type counter kept increasing since that's the only
 * way the MADT could find each little subtable.  It just wouldn't do to have
 * every subtable called Number 6.
 *
 * In the next valley over, a castle full of wizards was watching the MADT
 * and made a pact to keep their own counter.  Every time the MADT found a
 * new subtable, or a subtable grew up, the wizards promised they would
 * increment their counter.  Well, wizards being the forgetful sort, they
 * didn't alway do that.  And, since there quite a lot of them, they
 * couldn't always remember who was supposed to keep track of the MADT,
 * especially if dinner was coming up soon.  Their counter was called the
 * spec version.
 *
 * Every now and then, the MADT would gather up all its little subtables
 * and take them in to the cobbler to get new boots.  This was a very, very
 * meticulous cobbler, so every time they came, he wrote down all the boot
 * sizes for all of the little subtables.  The cobbler would ask each subtable
 * for its length, check that against his careful notes, and then go get the
 * right boots.  Sometimes, a little subtable would change a bit, and their
 * length did not match what the cobbler had written down.  If the wizards
 * or the MADT had incremented their counters, the cobbler would breath a
 * sigh of relief and write down the new length as the right one.  But, if
 * none of the counters had changed, this would make the cobbler very, very
 * mad.  He couldn't tell if he had the right size boots or not for the
 * little subtable.  He would have to *guess* and this really bugged him.
 *
 * Well, when the cobbler got mad like this, he would go into hiding.  He
 * would not make or sell any boots.  He would not go out at all.  Pretty
 * soon, the coffee shop would have to close because the cobbler wasn't
 * coming by twice a day any more.  Then the grocery store would have to
 * close because he wouldn't eat much.  After a while, everyone would panic
 * and have to move from the village and go live with all their relatives
 * (usually the ones they didn't like very much).
 *
 * Eventually, the cobbler would work his way out of his bad mood, and
 * open up his boot business again.  Then, everyone else could move back
 * to the village and restart their lives, too.
 *
 * Fortunately, we have been able to collect up all the cobbler's careful
 * notes (and we wrote them down below).  We'll have to keep checking these
 * notes over time, too, just as the cobbler does.  But, in the meantime,
 * we can avoid the panic and the reboot since we can make sure that each
 * subtable is doing okay.  And that's what our tests below check.
 *
 *
 * FADT Major Version ->       1    3    4     4     5     5     6    6     6
 * FADT Minor Version ->       x    x    x     x     x     1     0    0     1
 * MADT revision ->            1    1    2     3     3     3     3    4     4
 * Spec Version ->            1.0  2.0  3.0b  4.0a  5.0b  5.1a  6.0  6.0a  6.1
 * Subtable Name	Type  Expected Length ->
 * Processor Local APIC  0x0    8    8    8     8     8     8     8    8     8
 * IO APIC               0x1   12   12   12    12    12    12    12   12    12
 * Int Src Override      0x2   10   10   10    10    10    10    10   10    10
 * NMI Src               0x3    8    8    8     8     8     8     8    8     8
 * Local APIC NMI Struct 0x4    6    6    6     6     6     6     6    6     6
 * Local APIC Addr Ovrrd 0x5        16   12    12    12    12    12   12    12
 * IO SAPIC              0x6        20   16    16    16    16    16   16    16
 * Local SAPIC           0x7         8  >16   >16   >16   >16   >16  >16   >16
 * Platform Int Src      0x8        16   16    16    16    16    16   16    16
 * Proc Local x2APIC     0x9                   16    16    16    16   16    16
 * Local x2APIC NMI      0xa                   12    12    12    12   12    12
 * GICC CPU I/F          0xb                         40    76    80   80    80
 * GICD                  0xc                         24    24    24   24    24
 * GICv2m MSI            0xd                               24    24   24    24
 * GICR                  0xe                               16    16   16    16
 * GIC ITS               0xf                                     20   20    20
 *
 * In the table, each length entry is what should be in the length
 * field of the subtable, and -- in general -- it should match the
 * size of the struct for the subtable.  Any value that is not set
 * (i.e., is zero) indicates that the subtable is not defined for
 * that version of the ACPI spec.
 *
 */

#define FADT_MAX_MAJOR_REVISION	((uint8_t)6)
#define FADT_MAX_MINOR_REVISION	((uint8_t)1)
#define MADT_MAX_REVISION	((uint8_t)4)

#define SUBTABLE_VARIABLE	0xff
#define NUM_SUBTABLE_TYPES	16


struct acpi_madt_subtable_lengths {
	unsigned short major_version;	/* from revision in FADT header */
	unsigned short minor_version;	/* FADT field starting with 5.1 */
	unsigned short madt_version;	/* MADT revision */
	unsigned short num_types;	/* types possible for this version */
	unsigned short lengths[NUM_SUBTABLE_TYPES];
					/* subtable lengths, indexed by type */
};

/* SBBR starts with ACPI v6.0 */
static struct acpi_madt_subtable_lengths spec_info[] = {
	{ /* for ACPI 6.0 */
		.major_version = 6,
		.minor_version = 0,
		.madt_version = 3,
		.num_types = 16,
		.lengths = { 8, 12, 10, 8, 6, 12, 16, SUBTABLE_VARIABLE,
			     16, 16, 12, 80, 24, 24, 16, 20 }
		/*
		 * The spec technically defines the GIC ITS node length to be
		 * 16, but this is a clear mistake as the fields add up to be
		 * length 20, and the length was corrected in the next revision.
		 * Test for 20, as that is what is expected to be used in real,
		 * implementations and using length 20 ensures that the test can
		 * correctly find the start of any following nodes.
		 */
	},
	{ /* for ACPI 6.0a */
		.major_version = 6,
		.minor_version = 0,
		.madt_version = 4,
		.num_types = 16,
		.lengths = { 8, 12, 10, 8, 6, 12, 16, SUBTABLE_VARIABLE,
			     16, 16, 12, 80, 24, 24, 16, 20 }
	},
	{ /* for ACPI 6.1 */
		.major_version = 6,
		.minor_version = 1,
		.madt_version = 4,
		.num_types = 16,
		.lengths = { 8, 12, 10, 8, 6, 12, 16, SUBTABLE_VARIABLE,
			     16, 16, 12, 80, 24, 24, 16, 20 }
	},
	{ /* terminator */
		.major_version = 0,
		.minor_version = 0,
		.madt_version = 0,
		.num_types = 0,
		.lengths = { 0 }
	}
};

static struct acpi_madt_subtable_lengths *spec_data;
static uint8_t fadt_major;
static uint8_t fadt_minor;

static fwts_acpi_table_info *mtable;
static fwts_acpi_table_info *ftable;

static fwts_list msi_frame_ids;
static fwts_list its_ids;
static fwts_list processor_uids;

struct acpi_integer {
	ACPI_OBJECT_TYPE type;
	uint64_t value;
};

static ACPI_STATUS sbbr_madt_processor_handler(ACPI_HANDLE ObjHandle, uint32_t level,
					  void *context, void **returnvalue)
{
	ACPI_OBJECT_TYPE acpi_type;
	ACPI_STATUS status;
	ACPI_OBJECT obj;
	struct acpi_buffer buf = {sizeof(ACPI_OBJECT), &obj};
	struct acpi_integer *listint;

	/* Prevent -Werror=unused-parameter from complaining */
	FWTS_UNUSED(level);
	FWTS_UNUSED(context);
	FWTS_UNUSED(returnvalue);

	listint = malloc(sizeof(struct acpi_integer));
	if (!listint)
		return (!AE_OK);

	status = AcpiGetType(ObjHandle, &acpi_type);
	if (ACPI_FAILURE(status)) {
		free(listint);
		return (!AE_OK);
	}

	switch(acpi_type) {
	case ACPI_TYPE_PROCESSOR:
		status = AcpiEvaluateObject(ObjHandle, NULL, NULL, &buf);
		if (ACPI_FAILURE(status)) {
			free(listint);
			return status;
		}
		listint->value = obj.Processor.ProcId;
		break;
	case ACPI_TYPE_DEVICE:
		status = AcpiEvaluateObject(ObjHandle, "_UID", NULL, &buf);
		if (ACPI_FAILURE(status)) {
			free(listint);
			return status;
		}
		listint->value = obj.Integer.Value;
		break;
	default:
		free(listint);
		return (!AE_OK);
	}
	listint->type = acpi_type;
	fwts_list_append(&processor_uids, listint);

	return (AE_OK);
}

static ACPI_OBJECT_TYPE sbbr_madt_find_processor_uid(fwts_framework *fw,
						uint64_t uid,
						char *table_name)
{
	char table_label[64];
	fwts_list_link *item;

	fwts_list_foreach(item, &processor_uids) {
		struct acpi_integer *listint = fwts_list_data(struct acpi_integer *, item);
		if (uid == listint->value) {
			fwts_passed(fw, "MADT %s has matching processor "
				    "UID %" PRIu64 ".", table_name, uid);
			return listint->type;
		}
	}

	sprintf(table_label, "MADT%sUidMismatch", table_name);
	fwts_failed(fw, LOG_LEVEL_MEDIUM,
		    table_label, "%s has no matching processor UID %" PRIu64,
		    table_name, uid);
	return ACPI_NUM_TYPES;
}

static int sbbr_madt_init(fwts_framework *fw)
{
	fwts_acpi_table_madt *madt;
	fwts_acpi_table_fadt *fadt;
	struct acpi_madt_subtable_lengths *ms = spec_info;

	/* find the ACPI tables needed */
	if (fwts_acpi_find_table(fw, "APIC", 0, &mtable) != FWTS_OK) {
		fwts_log_error(fw, "Cannot find ACPI MADT tables.");
		return FWTS_ERROR;
	}
	if (!mtable) {
		fwts_log_error(fw, "Cannot read ACPI MADT tables.");
		return FWTS_ERROR;
	}

	if (fwts_acpi_find_table(fw, "FACP", 0, &ftable) != FWTS_OK) {
		fwts_log_error(fw, "Cannot find ACPI FADT tables.");
		return FWTS_ERROR;
	}
	if (!ftable) {
		fwts_log_error(fw, "Cannot read ACPI FADT tables.");
		return FWTS_ERROR;
	}

	if (!mtable || mtable->length == 0) {
		fwts_log_error(fw, "Required ACPI MADT (APIC) table not found");
		return FWTS_ERROR;
	}
	if (!ftable || ftable->length == 0) {
		fwts_log_error(fw, "Required ACPI FADT (FACP) table not found");
		return FWTS_ERROR;
	}

	/* determine the reference data needed */
	madt = (fwts_acpi_table_madt *)mtable->data;
	fadt = (fwts_acpi_table_fadt *)ftable->data;

	fadt_major = fadt->header.revision;
	fadt_minor = 0;
	if (fadt_major >= 5 && fadt->header.length >= 268)
		fadt_minor = fadt->minor_version;   /* field added ACPI 5.1 */

	/* find the first occurrence for this version of MADT */
	while (ms->num_types != 0) {
		if (ms->madt_version == madt->header.revision)
			break;
		ms++;
	}

	/* now, find the largest FADT version supported */
	spec_data = NULL;
	while (ms->num_types && ms->madt_version == madt->header.revision) {
		if (ms->major_version <= fadt_major &&
		    ms->minor_version <= fadt_minor) {
			spec_data = ms;
		}
		ms++;
	}

	/*
	 * Initialize the MSI frame ID and ITS ID lists should we need
	 * them later
	 */
	fwts_list_init(&msi_frame_ids);
	fwts_list_init(&its_ids);
	fwts_list_init(&processor_uids);

	if (fwts_acpica_init(fw) != FWTS_OK)
		return FWTS_ERROR;

	AcpiWalkNamespace(0x0c, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
			  sbbr_madt_processor_handler, NULL, NULL, NULL);
	AcpiGetDevices("ACPI0007", sbbr_madt_processor_handler, NULL, NULL);

	return (spec_data) ? FWTS_OK : FWTS_ERROR;
}

static int sbbr_madt_checksum(fwts_framework *fw)
{
	const uint8_t *data = mtable->data;
	ssize_t length = mtable->length;
	uint8_t checksum = 0;

	/* verify the table checksum */
	checksum = fwts_checksum(data, length);
	if (checksum == 0)
		fwts_passed(fw, "MADT checksum is correct");
	else
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "SPECMADTChecksum",
			    "MADT checksum is incorrect: 0x%x", checksum);

	return FWTS_OK;
}

static int sbbr_madt_revision(fwts_framework *fw)
{
	fwts_acpi_table_madt *madt = (fwts_acpi_table_madt *)mtable->data;
	struct acpi_madt_subtable_lengths *ms = spec_data;

	/* check the table revision */
	fwts_log_advice(fw, "Most recent FADT revision is %d.%d.",
			FADT_MAX_MAJOR_REVISION, FADT_MAX_MINOR_REVISION);
	fwts_log_advice(fw, "Most recent MADT revision is %d.",
			MADT_MAX_REVISION);

	/* is the madt revision defined at all? */
	if (!ms->num_types) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "SPECMADTRevision",
			    "Undefined MADT revision being used: %d",
			    madt->header.revision);
	} else {
			fwts_passed(fw, "MADT revision %d is defined.",
				    madt->header.revision);
	}

	/* is the madt revision in sync with the fadt revision? */
	if (ms->major_version != fadt_major ||
	    ms->minor_version != fadt_minor) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "SPECMADTFADTRevisions",
			    "MADT revision is not in sync with "
			    "the FADT revision;\n"
			    "MADT %d expects FADT %d.%d "
			    "but found %d.%d instead.",
			    madt->header.revision,
			    ms->major_version, ms->minor_version,
			    fadt_major, fadt_minor);
	} else {
		fwts_passed(fw, "MADT revision %d is in sync "
				"with FADT revision %d.%d.",
			madt->header.revision, fadt_major, fadt_minor);
	}

	return FWTS_OK;
}

static int sbbr_madt_arch_revision(fwts_framework *fw)
{
	fwts_acpi_table_madt *madt = (fwts_acpi_table_madt *)mtable->data;
	uint8_t minrev;
	const char *arch;

	/* initialize starting assumptions */
	minrev = 3;
	arch = "aarch64";


	/* check the supported revision for this architecture */
	if (madt->header.revision >= minrev)
		fwts_passed(fw, "MADT revision %d meets the minimum needed "
			    "(%d) for the %s architecture.",
			    madt->header.revision, minrev, arch);
	else
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "SPECMADTArchRevision",
			    "MADT revision is %d, must be >= %d "
			"when running on %s",
			madt->header.revision, minrev, arch);

	return FWTS_OK;
}

static int sbbr_madt_flags(fwts_framework *fw)
{
	fwts_acpi_table_madt *madt = (fwts_acpi_table_madt *)mtable->data;

	/* make sure the reserved bits in the flag field are zero */
	if (madt->flags & 0xfffffffe)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"MADTFlagsNonZero",
			"MADT flags field, bits 1..31 are reserved and "
			"should be zero, but are set as: %" PRIx32 ".\n",
			madt->flags);
	else
		fwts_passed(fw, "MADT flags reserved bits are not set.");

	return FWTS_OK;
}

static const char *madt_sub_names[] = {
	/* 0x00 */ "Processor Local APIC",
	/* 0x01 */ "I/O APIC",
	/* 0x02 */ "Interrupt Source Override",
	/* 0x03 */ "Non-maskable Interrupt Source (NMI)",
	/* 0x04 */ "Local APIC NMI",
	/* 0x05 */ "Local APIC Address Override",
	/* 0x06 */ "I/O SAPIC",
	/* 0x07 */ "Local SAPIC",
	/* 0x08 */ "Platform Interrupt Sources",
	/* 0x09 */ "Processor Local x2APIC",
	/* 0x0a */ "Local x2APIC NMI",
	/* 0x0b */ "GICC CPU Interface",
	/* 0x0c */ "GICD GIC Distributor",
	/* 0x0d */ "GICv2m MSI Frame",
	/* 0x0e */ "GICR Redistributor",
	/* 0x0f */ "GIC Interrupt Translation Service (ITS)",
	/* 0x10 - 0x7f */ "Reserved. OSPM skips structures of the reserved type.",
	/* 0x80 - 0xff */ "Reserved for OEM use",
	NULL
};

static int sbbr_madt_gicc(fwts_framework *fw,
		     fwts_acpi_madt_sub_table_header *hdr,
		     const uint8_t *data)
{
	/* specific checks for subtable type 0xb: GICC */
	fwts_acpi_madt_gic *gic = (fwts_acpi_madt_gic *)data;
	uint32_t mask;
	int start;

	if (gic->reserved)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "MADTGICCReservedNonZero",
			    "MADT %s reserved field should be zero, but is "
			    "instead 0x%" PRIx32 ".",
			    madt_sub_names[hdr->type], gic->reserved);
	else
		fwts_passed(fw,
			    "MADT %s reserved field properly set to zero.",
			    madt_sub_names[hdr->type]);

	sbbr_madt_find_processor_uid(fw, gic->processor_uid, "GICC");

	mask = 0xfffffffc;
	start = 2;
	if (hdr->length == 80) {	/* ACPI 6.0 */
		mask = 0xfffffff8;
		start = 3;
	}
	if (gic->flags & mask)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "MADTGICFLags",
			    "MADT %s, flags, bits %d..31 are reserved "
			    "and should be zero, but are set as: %" PRIx32 ".",
			    madt_sub_names[hdr->type], start, gic->flags);
	else
		fwts_passed(fw,
			    "MADT %s, flags, bits %d..31 are reserved and "
			    "properly set to zero.",
			    madt_sub_names[hdr->type], start);

	if (gic->parking_protocol_version != 0 &&
	    gic->parking_protocol_version != 1)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "SPECMADTGICCParkingVersion",
			    "MADT %s, protocol versions defined are 0..1 but "
			    "%d is being used.",
			    madt_sub_names[hdr->type],
			    gic->parking_protocol_version);
	else
		fwts_passed(fw,
			    "MADT %s, is using a defined parking protocol "
			    "version.",
			    madt_sub_names[hdr->type]);

	/*
	 * TODO: is it even possible to verify the MPIDR is valid?  Or,
	 * is there sufficient variation that it is not predictable?
	 */

	if (hdr->length == 80) {	/* added in ACPI 6.0 */
		uint8_t tmp = gic->reserved2[0] | gic->reserved2[1] |
			      gic->reserved2[2];

		if (tmp)
			fwts_failed(fw, LOG_LEVEL_LOW,
				    "MADTGICCReserved2NonZero",
				    "MADT %s second reserved field must "
				    "be zero.", madt_sub_names[hdr->type]);
		else
			fwts_passed(fw,
				    "MADT %s second reserved field properly "
				    "set to zero.",
				    madt_sub_names[hdr->type]);
	}

	/*
	 * TODO: the local GICC corresponding to the boot processor must
	 * be the the first entry in the interrupt controller structure
	 * list.
	 */

	return (hdr->length - sizeof(fwts_acpi_madt_sub_table_header));
}

static int sbbr_madt_gicd(fwts_framework *fw,
		     fwts_acpi_madt_sub_table_header *hdr,
		     const uint8_t *data)
{
	/* specific checks for subtable type 0xc: GIC Distributor */
	fwts_acpi_madt_gicd *gicd = (fwts_acpi_madt_gicd *)data;

	uint32_t gicd_reserve2 = gicd->reserved2[0] +
				 (gicd->reserved2[1] << 4) +
				 (gicd->reserved2[2] << 8);

	if (gicd->reserved)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "MADTGICDReservedNonZero",
			    "MADT %s reserved field should be zero, "
			    "instead got 0x%" PRIx32 ".",
			    madt_sub_names[hdr->type], gicd->reserved);
	else
		fwts_passed(fw,
			    "MADT %s reserved field properly set to zero.",
			    madt_sub_names[hdr->type]);

	/* TODO: is the physical base address required to be non-zero? */

	if (gicd->gic_version != 0 && gicd->gic_version > 4)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "SPECMADTGICDVersion",
			    "MADT %s GIC version field should be in 0..4, "
			    "but instead have 0x%" PRIx32 ".",
			    madt_sub_names[hdr->type], gicd->gic_version);
	else
		fwts_passed(fw,
			    "MADT %s GIC version field is in 0..4.",
			    madt_sub_names[hdr->type]);

	if (gicd_reserve2)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "MADTGICDReserved2NonZero",
			    "MADT %s second reserved field should be zero, "
			    "instead got 0x%" PRIx32 ".",
			    madt_sub_names[hdr->type], gicd_reserve2);
	else
		fwts_passed(fw,
			    "MADT %s second reserved field is properly set "
			    "to zero.", madt_sub_names[hdr->type]);

	return (hdr->length - sizeof(fwts_acpi_madt_sub_table_header));
}

static int sbbr_madt_gic_msi_frame(fwts_framework *fw,
			      fwts_acpi_madt_sub_table_header *hdr,
			      const uint8_t *data)
{
	/* specific checks for subtable type 0xd: GIC MSI Frame */
	fwts_acpi_madt_gic_msi *gic_msi = (fwts_acpi_madt_gic_msi *)data;
	fwts_list_link *item;
	bool found;

	/*
	 * TODO: is there some way to test that the entries found are
	 * for only non-secure MSI frames?
	 */

	if (gic_msi->reserved)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "MADTGICMSIReservedNonZero",
			    "MADT %s reserved field should be zero, "
			    "instead got 0x%" PRIx32 ".",
			    madt_sub_names[hdr->type], gic_msi->reserved);
	else
		fwts_passed(fw,
			    "MADT %s reserved field properly set to zero.",
			    madt_sub_names[hdr->type]);

	/*
	 * Check MSI Frame ID against previously found IDs to see if it
	 * is unique.  According to the spec, they must be.
	 */
	found = false;
	fwts_list_foreach(item, &msi_frame_ids) {
		uint32_t *frame_id = fwts_list_data(uint32_t *, item);

		if (*frame_id == gic_msi->frame_id)
			found = true;
	}
	if (found) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "MADTGICMSINonUniqueFrameId",
			    "MADT %s Frame ID 0x%" PRIx32 " is not unique "
			    "and has already be defined in a previous %s.",
			    madt_sub_names[hdr->type],
			    gic_msi->frame_id,
			    madt_sub_names[hdr->type]);
	} else {
		fwts_list_append(&msi_frame_ids, &(gic_msi->frame_id));
		fwts_passed(fw,
			    "MADT %s Frame ID 0x%" PRIx32 " is unique "
			    "as is required.",
			    madt_sub_names[hdr->type],
			    gic_msi->frame_id);
	}

	/*
	 * TODO: can the physical base address be tested, or is zero
	 * allowed?
	 */

	if (gic_msi->flags & 0xfffffffe)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "MADTGICMSIFLags",
			    "MADT %s, flags, bits 1..31 are reserved "
			    "and should be zero, but are set as: %" PRIx32 ".",
			    madt_sub_names[hdr->type],
			    gic_msi->flags);
	else
		fwts_passed(fw,
			    "MADT %s, flags, bits 1..31 are reserved "
			    "and properly set to zero.",
			    madt_sub_names[hdr->type]);

	/*
	 * TODO: can we check the SPI Count and SPI Base against the MSI_TYPER
	 * register in the frame at this point?  Or is this something that
	 * can only been done when running on the arch we're testing for?
	 */

	return (hdr->length - sizeof(fwts_acpi_madt_sub_table_header));
}

static int sbbr_madt_gicr(fwts_framework *fw,
		     fwts_acpi_madt_sub_table_header *hdr,
		     const uint8_t *data)
{
	/* specific checks for subtable type 0xe: GICR */
	fwts_acpi_madt_gicr *gicr = (fwts_acpi_madt_gicr *)data;

	/*
	 * TODO: GICR structures should only be used when GICs implement
	 * version 3 or higher.
	 */

	if (gicr->reserved)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "MADTGICRReservedNonZero",
			    "MADT %s reserved field should be zero, "
			    "instead got 0x%" PRIx32 ".",
			    madt_sub_names[hdr->type],
			    gicr->reserved);
	else
		fwts_passed(fw,
			    "MADT %s reserved field properly set to zero.",
			    madt_sub_names[hdr->type]);

	/*
	 * TODO: can Discovery Range Base Address ever be zero?
	 * Or, can we assume it must be non-zero?
	 */

	if (gicr->discovery_range_length == 0)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "SPECMADTGICRZeroLength",
			    "MADT %s discovery range length should be > 0.",
			    madt_sub_names[hdr->type]);
	else
		fwts_passed(fw,
			    "MADT %s discovery range length of %d > 0.",
			    madt_sub_names[hdr->type],
			    gicr->discovery_range_length);

	return (hdr->length - sizeof(fwts_acpi_madt_sub_table_header));
}

static int sbbr_madt_gic_its(fwts_framework *fw,
			     fwts_acpi_madt_sub_table_header *hdr,
			     const uint8_t *data)
{
	/* specific checks for subtable type 0xf: GIC ITS */
	fwts_acpi_madt_gic_its *gic_its = (fwts_acpi_madt_gic_its *)data;
	fwts_list_link *item;
	bool found;

	if (gic_its->reserved)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "SPECMADTGICITSReservedNonZero",
			    "MADT %s first reserved field should be zero, "
			    "instead got 0x%" PRIx32 ".",
			    madt_sub_names[hdr->type], gic_its->reserved);
	else
		fwts_passed(fw,
			    "MADT %s first reserved field is properly set "
			    "to zero.",
			    madt_sub_names[hdr->type]);

	/*
	 * Check ITS ID against previously found IDs to see if it
	 * is unique.  According to the spec, they must be.
	 */
	found = false;
	fwts_list_foreach(item, &its_ids) {
		uint32_t *its_id = fwts_list_data(uint32_t *, item);

		if (*its_id == gic_its->its_id)
			found = true;
	}
	if (found) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "SPECMADTGICITSNonUniqueId",
			    "MADT %s ITS ID 0x%" PRIx32 " is not unique "
			    "and has already be defined in a previous %s.",
			    madt_sub_names[hdr->type],
			    gic_its->its_id,
			    madt_sub_names[hdr->type]);
	} else {
		fwts_list_append(&its_ids, &(gic_its->its_id));
		fwts_passed(fw,
			    "MADT %s ITS ID 0x%" PRIx32 " is unique "
			    "as is required.",
			    madt_sub_names[hdr->type],
			    gic_its->its_id);
	}

	/*
	 * TODO: can the physical base address be tested, or is zero
	 * allowed?
	 */

	if (gic_its->reserved2)
		fwts_failed(fw, LOG_LEVEL_LOW,
			    "SPECMADTGICITSReserved2NonZero",
			    "MADT %s second reserved field should be zero, "
			    "instead got 0x%" PRIx32 ".",
			    madt_sub_names[hdr->type], gic_its->reserved2);
	else
		fwts_passed(fw,
			    "MADT %s second reserved field is properly set "
			    "to zero.",
			    madt_sub_names[hdr->type]);

	return (hdr->length - sizeof(fwts_acpi_madt_sub_table_header));
}

static int sbbr_madt_subtables(fwts_framework *fw)
{
	fwts_acpi_table_madt *madt = (fwts_acpi_table_madt *)mtable->data;
	fwts_acpi_madt_sub_table_header *hdr;
	struct acpi_madt_subtable_lengths *ms = spec_data;
	const uint8_t *data = mtable->data;
	ssize_t length = mtable->length;
	int ii = 0;

	/*
	 * check the correctness of each subtable type, and whether or
	 * not the subtable is allowed for this revision of the MADT
	 */

	data += sizeof(fwts_acpi_table_madt);
	length -= sizeof(fwts_acpi_table_madt);

	if (!ms->num_types) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			    "SPECMADTRevision",
			    "Undefined MADT revision being used: %d",
			    madt->header.revision);
	} else {
		fwts_passed(fw, "MADT revision %d is defined.",
			    madt->header.revision);
	}

	while (length > (ssize_t)sizeof(fwts_acpi_madt_sub_table_header)) {
		ssize_t skip = 0;
		int len = 0;
		int type;
		int offset = 0;

		hdr = (fwts_acpi_madt_sub_table_header *)data;
		ii++;

		data += sizeof(fwts_acpi_madt_sub_table_header);
		offset = (int)(mtable->length - length);
		length -= sizeof(fwts_acpi_madt_sub_table_header);

		/* set initial type value, will be overriden for OEM and
		 * reserved entries */
		type = hdr->type;

		/* check for OEM and reserved entries */
		if (hdr->type >= NUM_SUBTABLE_TYPES) {
			if (hdr->type < 0x80)
				type = FWTS_ACPI_MADT_RESERVED;
			else
				type = FWTS_ACPI_MADT_OEM;
			len = hdr->length;
		} else {
			/* this subtable is defined */
			len = ms->lengths[hdr->type];
		}

		if (!len) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				    "SPECMADTSubType",
				    "Undefined MADT subtable type for this "
				    "version of the MADT: %d (%s)",
				    hdr->type, madt_sub_names[type]);
		} else {
			fwts_passed(fw,
				    "MADT subtable type %d (%s) is defined.",
				    hdr->type, madt_sub_names[type]);
		}

		/* perform checks specific to subtable types */
		switch (type) {

		case FWTS_ACPI_MADT_GIC_C_CPU_INTERFACE:
			skip = sbbr_madt_gicc(fw, hdr, data);
			break;

		case FWTS_ACPI_MADT_GIC_D_GOC_DISTRIBUTOR:
			skip = sbbr_madt_gicd(fw, hdr, data);
			break;

		case FWTS_ACPI_MADT_GIC_V2M_MSI_FRAME:
			skip = sbbr_madt_gic_msi_frame(fw, hdr, data);
			break;

		case FWTS_ACPI_MADT_GIC_R_REDISTRIBUTOR:
			skip = sbbr_madt_gicr(fw, hdr, data);
			break;

		case FWTS_ACPI_MADT_GIC_ITS:
			skip = sbbr_madt_gic_its(fw, hdr, data);
			break;

		case FWTS_ACPI_MADT_RESERVED:
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				    "SPECMADTSubReservedID",
				    "MADT subtable %d (offset 0x%x) is "
				    "using the reserved value 0x%x for a "
				    "type.  Subtable type values 0x10..0x7f "
				    "are reserved; 0x80..0xff can be "
				    "used by OEMs.",
				    ii, offset, hdr->type);
			skip = (hdr->length -
				sizeof(fwts_acpi_madt_sub_table_header));
			break;
		case FWTS_ACPI_MADT_OEM:
			/* OEM entries must be assumed to be valid */
			skip = (hdr->length -
				sizeof(fwts_acpi_madt_sub_table_header));
			break;
		default:
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				    "SPECMADTSubReservedID",
				    "MADT subtable %d (offset 0x%x) is "
				    "using value 0x%x for a type.  This "
				    "value is out of the expected range "
				    "of 0x00 .. 0xff.",
				    ii, offset, hdr->type);
			skip = (hdr->length -
				sizeof(fwts_acpi_madt_sub_table_header));
			break;
		}

		if (hdr->length == 0) {
			fwts_log_error(fw, "INTERNAL ERROR: "
				       "zero length subtable means something "
				       "is seriously broken. Subtable %d "
				       "(offset 0x%0x) has the problem.",
				       ii, offset);
			break;
		}
		data   += skip;
		length -= skip;
	}

	return FWTS_OK;
}

static int sbbr_madt_deinit(fwts_framework *fw)
{
	fwts_acpica_deinit();

	/* only minor clean up needed */
	fwts_list_free_items(&msi_frame_ids, NULL);
	fwts_list_free_items(&its_ids, NULL);
	fwts_list_free_items(&processor_uids, NULL);

	return (fw) ? FWTS_ERROR : FWTS_OK;
}

static fwts_framework_minor_test sbbr_madt_tests[] = {
	{ sbbr_madt_checksum, "MADT checksum test." },
	{ sbbr_madt_revision, "MADT revision test." },
	{ sbbr_madt_arch_revision, "MADT architecture minimum revision test." },
	{ sbbr_madt_flags, "MADT flags field reserved bits test." },
	{ sbbr_madt_subtables, "MADT subtable tests related to GIC." },
	{ NULL, NULL }
};

static fwts_framework_ops sbbr_madt_ops = {
	.description = "MADT Multiple APIC Description Table (spec compliant).",
	.init        = sbbr_madt_init,
	.deinit      = sbbr_madt_deinit,
	.minor_tests = sbbr_madt_tests
};

FWTS_REGISTER("sbbr_madt", &sbbr_madt_ops, FWTS_TEST_ANYTIME, FWTS_FLAG_TEST_SBBR)

#endif
