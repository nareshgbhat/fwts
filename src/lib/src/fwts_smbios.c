/*
 * Copyright (C) 2010-2017 Canonical
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "fwts.h"

#if defined(FWTS_ARCH_INTEL) || defined(FWTS_ARCH_AARCH64)

/*
 *  fwts_load_file()
 *	loads file to memory
 */
static int fwts_load_file(const char* filename, void *buf, size_t size)
{
	int fd;
	ssize_t ret;

	if ((fd = open(filename, O_RDONLY)) < 0)
		return FWTS_ERROR;
	ret = read(fd, buf, size);
	(void)close(fd);
	if (ret != (ssize_t)size)
		return FWTS_ERROR;
	return FWTS_OK;
}

/*
 *  fwts_smbios_find_entry_uefi()
 *	find SMBIOS structure table entry from UEFI systab
 */
static void *fwts_smbios_find_entry_uefi(fwts_framework *fw, fwts_smbios_entry *entry, fwts_smbios_type *type)
{
	void *addr;

	if ((addr = fwts_scan_efi_systab("SMBIOS")) != NULL) {
		fwts_smbios_entry *mapped_entry;

		if ((mapped_entry = fwts_mmap((off_t)addr, sizeof(fwts_smbios_entry))) != FWTS_MAP_FAILED) {
			*entry = *mapped_entry;
			(void)fwts_munmap(mapped_entry, sizeof(fwts_smbios_entry));
			*type  = FWTS_SMBIOS;
			return addr;
		}

		if (fwts_load_file("/sys/firmware/dmi/tables/smbios_entry_point",
				entry, sizeof(fwts_smbios_entry)) == FWTS_OK && !strncmp((char*)entry, "_SM_", 4)) {
			fwts_log_info(fw, "SMBIOS entry loaded from /sys/firmware/dmi/tables/smbios_entry_point\n");
			*type  = FWTS_SMBIOS;
			return addr;
		}

		fwts_log_error(fw, "Cannot mmap SMBIOS entry at %p\n", addr);
	}
	return NULL;
}

/*
 *  fwts_smbios30_find_entry_uefi()
 *	find SMBIOS30 structure table entry from UEFI systab
 */
static void *fwts_smbios30_find_entry_uefi(fwts_framework *fw, fwts_smbios30_entry *entry)
{
	void *addr;

	if ((addr = fwts_scan_efi_systab("SMBIOS3")) != NULL) {
		fwts_smbios30_entry *mapped_entry;

		if ((mapped_entry = fwts_mmap((off_t)addr, sizeof(fwts_smbios30_entry))) != FWTS_MAP_FAILED) {
			*entry = *mapped_entry;
			(void)fwts_munmap(mapped_entry, sizeof(fwts_smbios30_entry));
			return addr;
		}

		if (fwts_load_file("/sys/firmware/dmi/tables/smbios_entry_point",
				entry, sizeof(fwts_smbios30_entry)) == FWTS_OK && !strncmp((char*)entry, "_SM3_", 5)) {
			fwts_log_info(fw, "SMBIOS30 entry loaded from /sys/firmware/dmi/tables/smbios_entry_point\n");
			return addr;
		}

		fwts_log_error(fw, "Cannot mmap SMBIOS30 entry at %p\n", addr);
	}
	return NULL;
}

#if defined(FWTS_ARCH_INTEL)
/*
 *  fwts_smbios_find_entry_bios()
 *	find SMBIOS structure table entry by scanning memory
 */
static void *fwts_smbios_find_entry_bios(fwts_framework *fw, fwts_smbios_entry *entry, fwts_smbios_type *type)
{
	uint8_t *mem;
	void 	*addr = NULL;
	int 	i;

        if ((mem = fwts_mmap(FWTS_SMBIOS_REGION_START, FWTS_SMBIOS_REGION_SIZE)) == FWTS_MAP_FAILED) {
		fwts_log_error(fw, "Cannot mmap SMBIOS region.");
		return NULL;
	}

	for (i = 0; i < FWTS_SMBIOS_REGION_SIZE; i += 16) {
		/* SMBIOS entry point */
		if ((*(mem+i)   == '_') &&
		    (*(mem+i+1) == 'S') &&
		    (*(mem+i+2) == 'M') &&
		    (*(mem+i+3) == '_') &&
		    (fwts_checksum(mem + i, 16) == 0)) {
			addr = (void *)((uint8_t *)FWTS_SMBIOS_REGION_START + i);
			memcpy(entry, (void *)(mem + i), sizeof(fwts_smbios_entry));
			*type  = FWTS_SMBIOS;
			break;
		}
		/* Legacy DMI entry point */
		if ((*(mem+i)   == '_') &&
		    (*(mem+i+1) == 'D') &&
		    (*(mem+i+2) == 'M') &&
		    (*(mem+i+3) == 'I') &&
		    (*(mem+i+4) == '_') &&
		    (fwts_checksum(mem + i, 15) == 0)) {
			memset(entry, 0, 16);
			addr = (void *)((uint8_t *)FWTS_SMBIOS_REGION_START + i);
			memcpy(16 + ((uint8_t *)entry), (void *)(mem + i), 15);
			*type = FWTS_SMBIOS_DMI_LEGACY;
			break;
		}
	}

        (void)fwts_munmap(mem, FWTS_SMBIOS_REGION_SIZE);

	return addr;
}

/*
 *  fwts_smbios30_find_entry_bios()
 *	find SMBIOS30 structure table entry by scanning memory
 */
static void *fwts_smbios30_find_entry_bios(fwts_framework *fw, fwts_smbios30_entry *entry)
{
	uint8_t *mem;
	void 	*addr = NULL;
	int 	i;

        if ((mem = fwts_mmap(FWTS_SMBIOS_REGION_START, FWTS_SMBIOS_REGION_SIZE)) == FWTS_MAP_FAILED) {
		fwts_log_error(fw, "Cannot mmap SMBIOS region.");
		return NULL;
	}

	for (i = 0; i < FWTS_SMBIOS_REGION_SIZE; i += 16) {
		/* SMBIOS30 entry point */
		if ((*(mem+i)   == '_') &&
		    (*(mem+i+1) == 'S') &&
		    (*(mem+i+2) == 'M') &&
		    (*(mem+i+3) == '3') &&
		    (*(mem+i+4) == '_') &&
		    (fwts_checksum(mem + i, 24 ) == 0)) {
			addr = (void *)((uint8_t *)FWTS_SMBIOS_REGION_START + i);
			memcpy(entry, (void*)(mem + i), sizeof(fwts_smbios30_entry));
			break;
		}
	}

        (void)fwts_munmap(mem, FWTS_SMBIOS_REGION_SIZE);

	return addr;
}
#endif

/*
 *  fwts_smbios_find_entry()
 *	find SMBIOS structure table entry
 */
void *fwts_smbios_find_entry(fwts_framework *fw,
	fwts_smbios_entry *entry,
	fwts_smbios_type  *type,
	uint16_t	  *version)
{
	void *addr;
	*type = FWTS_SMBIOS_UNKNOWN;

	/* Check EFI first */
	addr = fwts_smbios_find_entry_uefi(fw, entry, type);
	if (addr) {
		*version = (entry->major_version << 8) +
					(entry->minor_version & 0xff);
	} else {
#if defined(FWTS_ARCH_INTEL)
		/* Failed? then scan x86 memory for SMBIOS tag  */
		addr = fwts_smbios_find_entry_bios(fw, entry, type);
		if (addr) {
			switch (*type) {
			case FWTS_SMBIOS:
				*version = (entry->major_version << 8) +
				           (entry->minor_version & 0xff);
				break;
			case FWTS_SMBIOS_DMI_LEGACY:
				*version = ((entry->smbios_bcd_revision & 0xf0) << 4) +
				 	    (entry->smbios_bcd_revision & 0x0f);
				break;
			default:
				break;
			}
		}
#else
		(void)version;
#endif
	}
	return addr;
}

/*
 *  fwts_smbios30_find_entry()
 *	find SMBIOS30 structure table entry
 */
void *fwts_smbios30_find_entry(fwts_framework *fw,
	fwts_smbios30_entry *entry,
	uint16_t	  *version)
{
	void *addr;

	/* Check EFI first */
	addr = fwts_smbios30_find_entry_uefi(fw, entry);
	if (addr) {
		*version = (entry->major_version << 8) +
					(entry->minor_version & 0xff);
	} else {
#if defined(FWTS_ARCH_INTEL)
		/* Failed? then scan x86 memory for SMBIOS30 tag  */
		addr = fwts_smbios30_find_entry_bios(fw, entry);
		if (addr) {
			*version = (entry->major_version << 8) +
				           (entry->minor_version & 0xff);
		}
#else
		(void)version;
#endif
	}
	return addr;
}

#else
/*
 *  fwts_smbios_find_entry()
 *	find SMBIOS structure table entry, currently void for non-x86 specific code
 */
void *fwts_smbios_find_entry(fwts_framework *fw,
	fwts_smbios_entry *entry,
	fwts_smbios_type  *type,
	uint16_t	  *version)
{
	FWTS_UNUSED(fw);

	/* Return a dummy values */
	memset(entry, 0, sizeof(fwts_smbios_entry));
	*type = FWTS_SMBIOS_UNKNOWN;
	*version = 0;

	return NULL;
}

/*
 *  fwts_smbios30_find_entry()
 *	find SMBIOS30 structure table entry, currently void for non-x86 specific code
 */
void *fwts_smbios30_find_entry(fwts_framework *fw,
	fwts_smbios30_entry *entry,
	uint16_t	  *version)
{
	FWTS_UNUSED(fw);

	/* Return a dummy values */
	memset(entry, 0, sizeof(fwts_smbios30_entry));
	*version = 0;

	return NULL;
}

#endif
