/*
 * Copyright (C) 2010-2017 Canonical
 * Copyright (C) 2017      ARM Ltd
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include "fwts_acpi_object_eval.h"

/*
 * ACPI methods + objects used in Linux ACPI driver:
 *
 * Name 	 Tested
 * _ACx 	 Y
 * _ADR 	 Y
 * _AEI 	 Y
 * _ALC 	 Y
 * _ALI 	 Y
 * _ALP 	 Y
 * _ALR 	 Y
 * _ALT 	 Y
 * _ALx 	 N
 * _ART 	 Y
 * _BBN 	 Y
 * _BCL 	 Y
 * _BCM 	 Y
 * _BCT 	 Y
 * _BDN 	 Y
 * _BFS 	 deprecated
 * _BIF 	 Y
 * _BIX 	 Y
 * _BLT 	 N not easily tested
 * _BMA 	 Y
 * _BMC 	 Y
 * _BMD 	 Y
 * _BMS 	 Y
 * _BQC 	 Y
 * _BST 	 Y
 * _BTH 	 Y
 * _BTM 	 Y
 * _BTP 	 Y
 * _CBA 	 Y
 * _CCA 	 Y
 * _CDM 	 Y
 * _CID 	 Y
 * _CLS 	 N requires PCI SIG class info
 * _CPC 	 Y
 * _CR3 	 Y
 * _CRS 	 Y
 * _CRT 	 Y
 * _CSD 	 Y
 * _CST 	 Y
 * _CWS 	 Y
 * _DCK 	 Y
 * _DCS 	 Y
 * _DDC 	 Y
 * _DDN 	 Y
 * _DEP 	 Y
 * _DGS 	 Y
 * _DIS 	 Y
 * _DLM 	 Y
 * _DMA 	 Y
 * _DOD 	 Y
 * _DOS 	 Y
 * _DSD 	 Y
 * _DSM 	 N
 * _DSS 	 Y
 * _DSW 	 Y
 * _DTI 	 Y
 * _Exx 	 n/a
 * _EC_ 	 Y
 * _EDL 	 Y
 * _EJD 	 Y
 * _EJx 	 Y
 * _EVT 	 Y
 * _FDE 	 N (floppy controller, ignore)
 * _FDI 	 N (floppy controller, ignore)
 * _FDM 	 N (floppy controller, ignore)
 * _FIF 	 Y
 * _FIT 	 Y
 * _FIX 	 Y
 * _FPS 	 Y
 * _FSL 	 Y
 * _FST 	 Y
 * _GAI 	 Y
 * _GCP 	 Y
 * _GHL 	 Y
 * _GL	 	 n/a
 * _GLK 	 Y
 * _GPD 	 Y
 * _GPE 	 Y
 * _GRT 	 Y
 * _GSB 	 Y
 * _GTF 	 Y
 * _GTM 	 Y
 * _GTS 	 deprecated
 * _GWS 	 Y
 * _HID 	 Y
 * _HOT 	 Y
 * _HPP 	 Y
 * _HPX 	 N
 * _HRV 	 Y
 * _IFT 	 Y
 * _INI 	 Y
 * _IRC 	 Y
 * _Lxx 	 n/a
 * _LCK 	 Y
 * _LID 	 Y
 * _LPI 	 Y
 * _MAT 	 N
 * _MBM 	 Y
 * _MLS 	 Y
 * _MSG 	 Y
 * _MSM 	 N
 * _MTL 	 Y
 * _NTT 	 Y
 * _OFF 	 Y
 * _ON_ 	 Y
 * _OSC 	 n/a
 * _OST 	 n/a
 * _PAI 	 n/a
 * _PCL 	 Y
 * _PCT 	 Y
 * _PDC 	 deprecated
 * _PDL 	 Y
 * _PIC 	 Y
 * _PIF 	 Y
 * _PLD 	 Y
 * _PMC 	 Y
 * _PMD 	 Y
 * _PMM 	 Y
 * _PPC 	 Y
 * _PPE 	 Y
 * _PR0 	 Y
 * _PR1 	 Y
 * _PR2 	 Y
 * _PR3 	 Y
 * _PRE 	 Y
 * _PRL 	 Y
 * _PRR 	 Y
 * _PRS 	 Y
 * _PRT 	 Y
 * _PRW 	 Y
 * _PS0 	 Y
 * _PS1 	 Y
 * _PS2 	 Y
 * _PS3 	 Y
 * _PSC 	 Y
 * _PSD 	 Y
 * _PSE 	 Y
 * _PSL 	 Y
 * _PSR 	 Y
 * _PSS 	 Y
 * _PSV 	 Y
 * _PSW 	 Y
 * _PTC 	 Y
 * _PTP 	 n/a
 * _PTS 	 Y
 * _PUR 	 Y
 * _PXM 	 Y
 * _Qxx 	 n/a
 * _RDI 	 Y
 * _REG 	 n/a
 * _RMV 	 Y
 * _ROM 	 Y
 * _RST 	 Y
 * _RTV 	 Y
 * _S0_ 	 Y
 * _S1_ 	 Y
 * _S2_ 	 Y
 * _S3_ 	 Y
 * _S4_ 	 Y
 * _S5_ 	 Y
 * _S1D 	 Y
 * _S2D 	 Y
 * _S3D 	 Y
 * _S4D 	 Y
 * _S0W 	 Y
 * _S1W 	 Y
 * _S2W 	 Y
 * _S3W 	 Y
 * _S4W 	 Y
 * _SBS 	 Y
 * _SCP 	 Y
 * _SDD 	 n/a
 * _SEG 	 Y
 * _SHL 	 n/a
 * _SLI 	 N
 * _SPD 	 Y
 * _SRS 	 n/a
 * _SRT 	 Y
 * _SRV 	 Y
 * _SST 	 Y
 * _STA 	 Y
 * _STM 	 n/a
 * _STP 	 Y
 * _STR 	 Y
 * _STV 	 Y
 * _SUB 	 Y
 * _SUN 	 Y
 * _SWS 	 Y
 * _T_x 	 n/a
 * _TC1 	 Y
 * _TC2 	 Y
 * _TDL 	 Y
 * _TFP 	 Y
 * _TIP 	 Y
 * _TIV 	 Y
 * _TMP 	 Y
 * _TPC 	 Y
 * _TPT 	 Y
 * _TRT 	 Y
 * _TSD 	 Y
 * _TSN 	 Y
 * _TSP 	 Y
 * _TSS 	 Y
 * _TST 	 Y
 * _TTS 	 Y
 * _TZD 	 Y
 * _TZM 	 Y
 * _TZP 	 Y
 * _UID 	 Y
 * _UPC 	 Y
 * _UPD 	 Y
 * _UPP 	 Y
 * _VPO 	 Y
 * _WAK 	 Y
 * _WPC 	 Y
 * _WPP 	 Y
 * _Wxx 	 n/a
 * _WDG 	 N
 * _WED 	 N
 */

/* Test types */
#define	METHOD_MANDATORY	1
#define METHOD_OPTIONAL		2
#define METHOD_MOBILE		4
#define METHOD_SILENT		8

#define ACPI_TYPE_INTBUF	(ACPI_TYPE_INVALID + 1)

#define method_check_type(fw, name, buf, type) 			\
	method_check_type__(fw, name, buf, type, #type)

static bool fadt_mobile_platform;	/* True if a mobile platform */

#define method_test_integer(name, type)				\
static int method_test ## name(fwts_framework *fw)		\
{ 								\
	return method_evaluate_method(fw, type, # name,		\
		NULL, 0, method_test_integer_return, # name); 	\
}

typedef void (*method_test_return)(fwts_framework *fw, char *name,
	ACPI_BUFFER *ret_buff, ACPI_OBJECT *ret_obj, void *private);

/*
 * Helper functions to facilitate the evaluations
 */

/****************************************************************************/

static bool method_type_matches(ACPI_OBJECT_TYPE t1, ACPI_OBJECT_TYPE t2)
{
	if (t1 == ACPI_TYPE_INTBUF &&
	    (t2 == ACPI_TYPE_INTEGER || t2 == ACPI_TYPE_BUFFER))
		return true;

	if (t2 == ACPI_TYPE_INTBUF &&
	    (t1 == ACPI_TYPE_INTEGER || t1 == ACPI_TYPE_BUFFER))
		return true;

	return t1 == t2;
}

/*
 *  method_passed_sane()
 *	helper function to report often used passed messages
 */
static void method_passed_sane(
	fwts_framework *fw,
	const char *name,
	const char *type)
{
	fwts_passed(fw, "%s correctly returned a sane looking %s.", name, type);
}

/*
 *  method_passed_sane_uint64()
 *	helper function to report often used passed uint64 values
 */
static void method_passed_sane_uint64(
	fwts_framework *fw,
	const char *name,
	const uint64_t value)
{
	fwts_passed(fw, "%s correctly returned sane looking "
		"value 0x%8.8" PRIx64 ".", name, value);
}

/*
 *  method_failed_null_return()
 *	helper function to report often used failed NULL object return
 */
static void method_failed_null_object(
	fwts_framework *fw,
	const char *name,
	const char *type)
{
	fwts_failed(fw, LOG_LEVEL_MEDIUM, "MethodReturnNullObj",
		"%s returned a NULL object, and did not "
		"return %s.", name, type);
}

/*
 *  method_package_count_min()
 *	check that an ACPI package has at least 'min' elements
 */
static int method_package_count_min(
	fwts_framework *fw,
	const char *name,
	const char *objname,
	const ACPI_OBJECT *obj,
	const uint32_t min)
{
	if (obj->Package.Count < min) {
		char tmp[128];

		snprintf(tmp, sizeof(tmp), "Method%sElementCount", objname);
		fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
			"%s should return package of at least %" PRIu32
			" element%s, got %" PRIu32 " element%s instead.",
			name, min, min == 1 ? "" : "s",
			obj->Package.Count, obj->Package.Count == 1 ? "" : "s");
		return FWTS_ERROR;
	}
	return FWTS_OK;
}

/*
 *  method_package_count_equal()
 *	check that an ACPI package has exactly 'count' elements
 */
static int method_package_count_equal(
	fwts_framework *fw,
	const char *name,
	const char *objname,
	const ACPI_OBJECT *obj,
	const uint32_t count)
{
	if (obj->Package.Count != count) {
		char tmp[128];

		snprintf(tmp, sizeof(tmp), "Method%sElementCount", objname);
		fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
			"%s should return package of %" PRIu32
			" element%s, got %" PRIu32 " element%s instead.",
			name, count, count == 1 ? "" : "s",
			obj->Package.Count, obj->Package.Count == 1 ? "" : "s");
		return FWTS_ERROR;
	}
	return FWTS_OK;
}

/*
 *  method_init()
 *	initialize ACPI
 */
static int sbbr_method_init(fwts_framework *fw)
{
	fwts_acpi_table_info *info;
	int i;
	bool got_fadt = false;

	fadt_mobile_platform = false;

	/* Some systems have multiple FADTs, sigh */
	for (i = 0; i < 256; i++) {
		fwts_acpi_table_fadt *fadt;
		int ret = fwts_acpi_find_table(fw, "FACP", i, &info);
		if (ret == FWTS_NULL_POINTER || info == NULL)
			break;
		fadt = (fwts_acpi_table_fadt*)info->data;
		got_fadt = true;
		if (fadt->preferred_pm_profile == 2) {
			fadt_mobile_platform = true;
			break;
		}
	}

	if (got_fadt && !fadt_mobile_platform) {
		fwts_log_info(fw,
			"FADT Preferred PM profile indicates this is not "
			"a Mobile Platform.");
	}

	if (fwts_acpi_init(fw) != FWTS_OK) {
		fwts_log_error(fw, "Cannot initialise ACPI.");
		return FWTS_ERROR;
	}

	return FWTS_OK;
}

/*
 *  method_deinit
 *	de-intialize ACPI
 */
static int sbbr_method_deinit(fwts_framework *fw)
{
	return fwts_acpi_deinit(fw);
}

/*
 *  method_evaluate_found_method
 *	find a given object name and evaluate it
 */
static void method_evaluate_found_method(
	fwts_framework *fw,
	char *name,
	method_test_return check_func,
	void *private,
	ACPI_OBJECT_LIST *arg_list)
{
	ACPI_BUFFER       buf;
	ACPI_STATUS	  ret;
	int sem_acquired;
	int sem_released;

	fwts_acpica_sem_count_clear();

	ret = fwts_acpi_object_evaluate(fw, name, arg_list, &buf);

	if (ACPI_FAILURE(ret) != AE_OK) {
		fwts_acpi_object_evaluate_report_error(fw, name, ret);
	} else {
		if (check_func != NULL) {
			ACPI_OBJECT *obj = buf.Pointer;
			check_func(fw, name, &buf, obj, private);
		}
	}
	free(buf.Pointer);

	fwts_acpica_sem_count_get(&sem_acquired, &sem_released);
	if (sem_acquired != sem_released) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "AMLLocksAcquired",
			"%s left %d locks in an acquired state.",
			name, sem_acquired - sem_released);
		fwts_advice(fw,
			"Locks left in an acquired state generally indicates "
			"that the AML code is not releasing a lock. This can "
			"sometimes occur when a method hits an error "
			"condition and exits prematurely without releasing an "
			"acquired lock. It may be occurring in the method "
			"being tested or other methods used while evaluating "
			"the method.");
	}
}

/*
 *  method_evaluate_method
 *	find all matching object names and evaluate them,
 *	also run the callback check_func to sanity check
 *	any returned values
 */
static int method_evaluate_method(fwts_framework *fw,
	int test_type,  /* Manditory or optional */
	char *name,
	ACPI_OBJECT *args,
	int num_args,
	method_test_return check_func,
	void *private)
{
	fwts_list *methods;
	size_t name_len = strlen(name);
	bool found = false;


	if ((methods = fwts_acpi_object_get_names()) != NULL) {
		fwts_list_link	*item;

		fwts_list_foreach(item, methods) {
			char *method_name = fwts_list_data(char*, item);
			ACPI_HANDLE method_handle;
			ACPI_OBJECT_TYPE type;
			ACPI_STATUS status;

			size_t len = strlen(method_name);
			if (strncmp(name, method_name + len - name_len, name_len) == 0) {
				ACPI_OBJECT_LIST  arg_list;

				status = AcpiGetHandle (NULL, method_name, &method_handle);
				if (ACPI_FAILURE(status)) {
					fwts_warning(fw, "Failed to get handle for object %s.", name);
				}
				status = AcpiGetType(method_handle, &type);
				if (ACPI_FAILURE(status)) {
					fwts_warning(fw, "Failed to get object type for %s.",name);
				}

				if (type == ACPI_TYPE_LOCAL_SCOPE)
					continue;

				found = true;
				arg_list.Count   = num_args;
				arg_list.Pointer = args;
				method_evaluate_found_method(fw, method_name,
					check_func, private, &arg_list);
			}
		}
	}

	if (found) {
		if ((test_type & METHOD_MOBILE) && (!fadt_mobile_platform)) {
			fwts_warning(fw,
				"The FADT indictates that this machine is not "
				"a mobile platform, however it has a mobile "
				"platform specific object %s defined. "
				"Either the FADT referred PM profile is "
				"incorrect or this machine has mobile "
				"platform objects defined when it should not.",
				name);
		}
		return FWTS_OK;
	} else {
		if (!(test_type & METHOD_SILENT)) {
			/* Mandatory not-found test are a failure */
			if (test_type & METHOD_MANDATORY) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM, "MethodNotExist",
					"Object %s did not exist.", name);
			}

			/* Mobile specific tests on non-mobile platform? */
			if ((test_type & METHOD_MOBILE) && (!fadt_mobile_platform)) {
				fwts_skipped(fw,
					"Machine is not a mobile platform, skipping "
					"test for non-existent mobile platform "
					"related object %s.", name);
			} else {
				fwts_skipped(fw,
					"Skipping test for non-existent object %s.",
					name);
			}
		}
		return FWTS_NOT_EXIST;

	}
}

/*
 *  method_name_check
 *	sanity check object name conforms to ACPI specification
 */
static int method_name_check(fwts_framework *fw)
{
	fwts_list *methods;

 	if ((methods = fwts_acpi_object_get_names()) != NULL) {
		fwts_list_link	*item;
		bool failed = false;

		fwts_log_info(fw, "Found %d Objects\n", methods->len);

		fwts_list_foreach(item, methods) {
			char *ptr;

			for (ptr = fwts_list_data(char *, item); *ptr; ptr++) {
				if (!((*ptr == '\\') ||
				     (*ptr == '.') ||
				     (*ptr == '_') ||
				     (isdigit(*ptr)) ||
				     (isupper(*ptr))) ) {
					fwts_failed(fw, LOG_LEVEL_HIGH,
						"MethodIllegalName",
						"Method %s contains an illegal "
						"character: '%c'. This should "
						"be corrected.",
						fwts_list_data(char *, item),
						*ptr);
					failed = true;
					break;
				}
			}
		}
		if (!failed)
			fwts_passed(fw, "Method names contain legal characters.");
	}

	return FWTS_OK;
}

/*
 *  method_check_type__
 *	check returned object type
 */
static int method_check_type__(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT_TYPE type,
	char *type_name)
{
	ACPI_OBJECT *obj;

	if ((buf == NULL) || (buf->Pointer == NULL)) {
		method_failed_null_object(fw, name, type_name);
		return FWTS_ERROR;
	}

	obj = buf->Pointer;

	if (!method_type_matches(obj->Type, type)) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "MethodReturnBadType",
			"Method %s did not return %s.", name, type_name);
		return FWTS_ERROR;
	}
	return FWTS_OK;
}

/*
 *  method_test_buffer_return
 *	check if a buffer object was returned
 */
static void method_test_buffer_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_BUFFER) == FWTS_OK)
		fwts_passed(fw, "%s correctly returned a buffer of %" PRIu32 " elements.",
			name, obj->Buffer.Length);
}

/*
 *  method_test_integer_return
 *	check if an integer object was returned
 */
static void method_test_integer_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(obj);
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) == FWTS_OK)
		fwts_passed(fw, "%s correctly returned an integer.", name);
}

/*
 *  method_test_string_return
 *	check if an string object was returned
 */
static void method_test_string_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(obj);
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_STRING) == FWTS_OK)
		fwts_passed(fw, "%s correctly returned a string.", name);
}

/*
 *  method_test_reference_return
 *	check if a reference object was returned
 */
static void method_test_reference_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(obj);
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_LOCAL_REFERENCE) == FWTS_OK)
		fwts_passed(fw, "%s correctly returned a reference.", name);
}

/*
 *  method_test_NULL_return
 *	check if no object was retuned
 */
static void method_test_NULL_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	/*
	 *  In ACPICA SLACK mode null returns can be actually
	 *  forced to return ACPI integers. Blame an errata
	 *  and Windows compatibility for this mess.
	 */
	if (fw->acpica_mode & FWTS_ACPICA_MODE_SLACK) {
		if ((buf != NULL) && (buf->Pointer != NULL)) {
			ACPI_OBJECT *objtmp = buf->Pointer;
			if (method_type_matches(objtmp->Type, ACPI_TYPE_INTEGER)) {
				fwts_passed(fw, "%s returned an ACPI_TYPE_INTEGER as expected in slack mode.",
					name);
				return;
			}
		}
	}

	if (buf && buf->Length && buf->Pointer) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "MethodShouldReturnNothing", "%s returned values, but was expected to return nothing.", name);
		fwts_log_info(fw, "Object returned:");
		fwts_acpi_object_dump(fw, obj);
		fwts_advice(fw,
			"This probably won't cause any errors, but it should "
			"be fixed as the AML code is not conforming to the "
			"expected behaviour as described in the ACPI "
			"specification.");
	} else
		fwts_passed(fw, "%s returned no values as expected.", name);
}

/*
 *  method_test_passed_failed_return
 *	check if 0 or 1 (false/true) integer is returned
 */
static void method_test_passed_failed_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	char *method = (char *)private;
	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) == FWTS_OK) {
		uint32_t val = (uint32_t)obj->Integer.Value;
		if ((val == 0) || (val == 1))
			method_passed_sane_uint64(fw, name, obj->Integer.Value);
		else {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"MethodReturnZeroOrOne",
				"%s returned 0x%8.8" PRIx32 ", should return 1 "
				"(success) or 0 (failed).", method, val);
			fwts_advice(fw,
				"Method %s should be returning the correct "
				"1/0 success/failed return values. "
				"Unexpected behaviour may occur becauses of "
				"this error, the AML code does not conform to "
				"the ACPI specification and should be fixed.",
				method);
		}
	}
}

/*
 *  method_test_polling_return
 *	check if a returned polling time is valid
 */
static void method_test_polling_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) == FWTS_OK) {
		char *method = (char *)private;
		if (obj->Integer.Value < 36000) {
			fwts_passed(fw,
				"%s correctly returned sane looking value "
				"%f seconds", method,
				(float)obj->Integer.Value / 10.0);
		} else {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"MethodPollTimeTooLong",
				"%s returned a value %f seconds > (1 hour) "
				"which is probably incorrect.",
				method, (float)obj->Integer.Value / 10.0);
			fwts_advice(fw,
				"The method is returning a polling interval "
				"which is very long and hence most probably "
				"incorrect.");
		}
	}
}


/*
 *  Common types that can be returned. This is not a complete
 *  list but it does cover the types we expect to return from
 *  an ACPI evaluation.
 */
static const char *method_type_name(const ACPI_OBJECT_TYPE type)
{
	switch (type) {
	case ACPI_TYPE_INTEGER:
		return "integer";
	case ACPI_TYPE_STRING:
		return "string";
	case ACPI_TYPE_BUFFER:
		return "buffer";
	case ACPI_TYPE_PACKAGE:
		return "package";
	case ACPI_TYPE_BUFFER_FIELD:
		return "buffer_field";
	case ACPI_TYPE_LOCAL_REFERENCE:
		return "reference";
	case ACPI_TYPE_INTBUF:
		return "integer or buffer";
	default:
		return "unknown";
	}
}

/*
 *  method_package_elements_all_type()
 *	sanity check fields in a package that all have
 *	the same type
 */
static int method_package_elements_all_type(
	fwts_framework *fw,
	const char *name,
	const char *objname,
	const ACPI_OBJECT *obj,
	const ACPI_OBJECT_TYPE type)
{
	uint32_t i;
	bool failed = false;
	char tmp[128];

	for (i = 0; i < obj->Package.Count; i++) {
		if (!method_type_matches(obj->Package.Elements[i].Type, type)) {
			snprintf(tmp, sizeof(tmp), "Method%sElementType", objname);
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
				"%s package element %" PRIu32 " was not the expected "
				"type '%s', was instead type '%s'.",
				name, i,
				method_type_name(type),
				method_type_name(obj->Package.Elements[i].Type));
			failed = true;
		}
	}

	return failed ? FWTS_ERROR: FWTS_OK;
}

typedef struct {
	ACPI_OBJECT_TYPE type;	/* Type */
	const char 	*name;	/* Field name */
} fwts_package_element;

/*
 *  method_package_elements_type()
 *	sanity check fields in a package that all have
 *	the same type
 */
static int method_package_elements_type(
	fwts_framework *fw,
	const char *name,
	const char *objname,
	const ACPI_OBJECT *obj,
	const fwts_package_element *info,
	const uint32_t count)
{
	uint32_t i;
	bool failed = false;
	char tmp[128];

	if (obj->Package.Count != count)
		return FWTS_ERROR;

	for (i = 0; i < obj->Package.Count; i++) {
		if (!method_type_matches(obj->Package.Elements[i].Type, info[i].type)) {
			snprintf(tmp, sizeof(tmp), "Method%sElementType", objname);
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
				"%s package element %" PRIu32 " (%s) was not the expected "
				"type '%s', was instead type '%s'.",
				name, i, info[i].name,
				method_type_name(info[i].type),
				method_type_name(obj->Package.Elements[i].Type));
			failed = true;
		}
	}

	return failed ? FWTS_ERROR: FWTS_OK;
}

/****************************************************************************/

/*
 * Section 5.6 ACPI Event Programming Model
 */
static void method_test_AEI_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	ACPI_STATUS status;
	ACPI_RESOURCE *resource;
	ACPI_RESOURCE_GPIO* gpio;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;

	status = AcpiBufferToResource(obj->Buffer.Pointer, obj->Buffer.Length, &resource);
	if (ACPI_FAILURE(status))
		return;

	do {
		if (resource->Type == ACPI_RESOURCE_TYPE_GPIO) {
			gpio = &resource->Data.Gpio;
			if (gpio->ConnectionType != ACPI_RESOURCE_GPIO_TYPE_INT) {
				failed = true;
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_AEIBadGpioElement",
					"%s should contain only GPIO Connection Type 0, got %" PRIu32,
					name, gpio->ConnectionType);
			}
		} else {
			failed = true;
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_AEIBadElement",
				"%s should contain only Resource Type 17, got%" PRIu32,
					name, resource->Type);
		}

		resource = ACPI_NEXT_RESOURCE(resource);
	} while (resource->Type != ACPI_RESOURCE_TYPE_END_TAG);

	if (!failed)
		method_passed_sane(fw, name, "buffer");
}

static int method_test_AEI(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_AEI", NULL, 0, method_test_AEI_return, NULL);
}

static void check_evt_event (
	fwts_framework *fw,
	ACPI_RESOURCE_GPIO *gpio)
{
	ACPI_OBJECT arg[1];
	ACPI_HANDLE evt_handle;
	ACPI_STATUS status;
	char path[256];
	uint16_t i;

	/* Skip the leading spaces in ResourceSource. */
	for (i = 0; i < gpio->ResourceSource.StringLength; i++) {
		if (gpio->ResourceSource.StringPtr[i] != ' ')
			break;
	}

	if (i == gpio->ResourceSource.StringLength) {
		fwts_log_warning(fw, "Invalid ResourceSource");
		return;
	}

	/* Get the handle of return;the _EVT method. */
	snprintf (path, 251, "%s._EVT", &gpio->ResourceSource.StringPtr[i]);

	status = AcpiGetHandle (NULL, path, &evt_handle);
	if (ACPI_FAILURE(status)) {
		fwts_log_warning(fw, "Failed to find valid handle for _EVT method (0x%x), %s",	status, path);
		return;
	}

	/* Call the _EVT method with all the pins defined for the GpioInt */
	for (i = 0; i < gpio->PinTableLength; i++) {
		ACPI_OBJECT_LIST arg_list;

		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = gpio->PinTable[i];
		arg_list.Count = 1;
		arg_list.Pointer = arg;

		method_evaluate_found_method(fw, path, method_test_NULL_return, NULL, &arg_list);
	}
}

static void method_test_EVT_return (
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	ACPI_RESOURCE *resource;
	ACPI_STATUS   status;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;

	status = AcpiBufferToResource(obj->Buffer.Pointer, obj->Buffer.Length, &resource);
	if (ACPI_FAILURE(status))
		return;

	do {
		if (!resource->Length) {
			fwts_log_warning(fw, "Invalid zero length descriptor in resource list\n");
			break;
		}

		if (resource->Type == ACPI_RESOURCE_TYPE_GPIO &&
				resource->Data.Gpio.ConnectionType == ACPI_RESOURCE_GPIO_TYPE_INT)
				check_evt_event(fw, &resource->Data.Gpio);

		resource = ACPI_NEXT_RESOURCE(resource);
	} while (resource->Type != ACPI_RESOURCE_TYPE_END_TAG);
}

static int method_test_EVT(fwts_framework *fw)
{
	int ret;

	/* Only test the _EVT method with pins defined in AEI. */
	ret = method_evaluate_method(fw, METHOD_OPTIONAL | METHOD_SILENT,
		"_AEI", NULL, 0, method_test_EVT_return, NULL);

	if (ret == FWTS_NOT_EXIST)
		fwts_skipped(fw, "Skipping test for non-existant object _EVT.");

	return ret;
}

/*
 * Section 5.7 Predefined Objects
 */

static void method_test_DLM_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_DLM", obj, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Could be one or more packages */
	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg = &obj->Package.Elements[i];

		if (pkg->Package.Count != 2) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_DLMSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 2 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		if (pkg->Package.Elements[0].Type != ACPI_TYPE_LOCAL_REFERENCE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_DLMBadSubPackageReturnType",
				"%s sub-package %" PRIu32
				" element 0 is not a reference.",
				name, i);
			failed = true;
		}

		if (pkg->Package.Elements[1].Type != ACPI_TYPE_LOCAL_REFERENCE &&
		    pkg->Package.Elements[1].Type != ACPI_TYPE_BUFFER) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_DLMBadSubPackageReturnType",
				"%s sub-package %" PRIu32
				" element 1 is not a reference or a buffer.",
				name, i);
			failed = true;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_DLM(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DLM", NULL, 0, method_test_DLM_return, NULL);
}

/*
 * Section 5.8 System Configuration Objects
 */
static int method_test_PIC(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	int i, ret;
	arg[0].Type = ACPI_TYPE_INTEGER;

	for (i = 0; i < 3; i++) {
		arg[0].Integer.Value = i;
		ret = method_evaluate_method(fw, METHOD_OPTIONAL,
			"_PIC", arg, 1, method_test_NULL_return, NULL);

		if (ret != FWTS_OK)
			break;
	}
	return ret;
}


/*
 * Section 6.1 Device Identification Objects
 */
static int method_test_DDN(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DDN", NULL, 0, method_test_string_return, NULL);
}

static bool method_valid_HID_string(char *str)
{
	if (strlen(str) == 7) {
		/* PNP ID, must be 3 capitals followed by 4 hex */
		if (!isupper(str[0]) ||
		    !isupper(str[1]) ||
		    !isupper(str[2])) return false;
		if (!isxdigit(str[3]) ||
		    !isxdigit(str[4]) ||
		    !isxdigit(str[5]) ||
		    !isxdigit(str[6])) return false;
		return true;
	}

	if (strlen(str) == 8) {
		/* ACPI ID, must be 4 capitals or digits followed by 4 hex */
		if ((!isupper(str[0]) && !isdigit(str[0])) ||
		    (!isupper(str[1]) && !isdigit(str[1])) ||
		    (!isupper(str[2]) && !isdigit(str[2])) ||
		    (!isupper(str[3]) && !isdigit(str[3]))) return false;
		if (!isxdigit(str[4]) ||
		    !isxdigit(str[5]) ||
		    !isxdigit(str[6]) ||
		    !isxdigit(str[7])) return false;
		return true;
	}

	return false;
}

static bool method_valid_EISA_ID(uint32_t id, char *buf, size_t buf_len)
{
	snprintf(buf, buf_len, "%c%c%c%02" PRIX32 "%02" PRIX32,
		0x40 + ((id >> 2) & 0x1f),
		0x40 + ((id & 0x3) << 3) + ((id >> 13) & 0x7),
		0x40 + ((id >> 8) & 0x1f),
		(id >> 16) & 0xff, (id >> 24) & 0xff);

	/* 3 chars in EISA ID must be upper case */
	if (!isupper(buf[0]) ||
	    !isupper(buf[1]) ||
	    !isupper(buf[2])) return false;

	/* Last 4 digits are always going to be hex, so pass */
	return true;
}

static void method_test_HID_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	char tmp[8];

	FWTS_UNUSED(buf);
	FWTS_UNUSED(private);

	if (obj == NULL) {
		method_failed_null_object(fw, name, "a buffer or integer");
		return;
	}

	switch (obj->Type) {
	case ACPI_TYPE_STRING:
		if (obj->String.Pointer) {
			if (method_valid_HID_string(obj->String.Pointer))
				fwts_passed(fw,
					"%s returned a string '%s' "
					"as expected.",
					name, obj->String.Pointer);
			else
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"MethodHIDInvalidString",
					"%s returned a string '%s' "
					"but it was not a valid PNP ID or a "
					"valid ACPI ID.",
					name, obj->String.Pointer);
		} else {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_HIDNullString",
				"%s returned a NULL string.", name);
		}
		break;
	case ACPI_TYPE_INTEGER:
		if (method_valid_EISA_ID((uint32_t)obj->Integer.Value,
			tmp, sizeof(tmp)))
			fwts_passed(fw, "%s returned an integer "
				"0x%8.8" PRIx64 " (EISA ID %s).",
				name, (uint64_t)obj->Integer.Value, tmp);
		else
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"MethodHIDInvalidInteger",
				"%s returned a integer 0x%8.8" PRIx64 " "
				"(EISA ID %s) but the this is not a valid "
				"EISA ID encoded PNP ID.",
				name, (uint64_t)obj->Integer.Value, tmp);
		break;
	default:
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_HIDBadReturnType",
			"%s did not return a string or an integer.", name);
		break;
	}
}

static int method_test_HID(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_HID", NULL, 0, method_test_HID_return, NULL);
}

static void method_valid_CID_Type(
	fwts_framework *fw,
	char *name,
	ACPI_OBJECT *obj)
{
	char tmp[8];

	switch (obj->Type) {
	case ACPI_TYPE_STRING:
		if (obj->String.Pointer) {
			if (method_valid_HID_string(obj->String.Pointer))
				fwts_passed(fw,
					"%s returned a string '%s' "
					"as expected.",
					name, obj->String.Pointer);
			else
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"MethodCIDInvalidString",
					"%s returned a string '%s' "
					"but it was not a valid PNP ID or a "
					"valid ACPI ID.",
					name, obj->String.Pointer);
		} else {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_CIDNullString",
				"%s returned a NULL string.", name);
		}
		break;
	case ACPI_TYPE_INTEGER:
		if (method_valid_EISA_ID((uint32_t)obj->Integer.Value,
			tmp, sizeof(tmp)))
			fwts_passed(fw, "%s returned an integer "
				"0x%8.8" PRIx64 " (EISA ID %s).",
				name, (uint64_t)obj->Integer.Value, tmp);
		else
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"MethodCIDInvalidInteger",
				"%s returned a integer 0x%8.8" PRIx64 " "
				"(EISA ID %s) but the this is not a valid "
				"EISA ID encoded PNP ID.",
				name, (uint64_t)obj->Integer.Value, tmp);
		break;
	}
}

static void method_test_CID_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;

	FWTS_UNUSED(buf);
	FWTS_UNUSED(private);

	if (obj == NULL) {
		method_failed_null_object(fw, name, "a buffer or integer");
		return;
	}

	switch (obj->Type) {
	case ACPI_TYPE_STRING:
	case ACPI_TYPE_INTEGER:
		method_valid_CID_Type(fw, name, obj);
		break;
	case ACPI_TYPE_PACKAGE:
		if (method_package_count_min(fw, name, "_CID", obj, 1) != FWTS_OK)
			return;

		for (i = 0; i < obj->Package.Count; i++){
			ACPI_OBJECT *pkg = &obj->Package.Elements[i];
			method_valid_CID_Type(fw, name, pkg);
		}
		break;
	default:
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_CIDBadReturnType",
			"%s did not return a string or an integer.", name);
		break;
	}
}

static int method_test_CID(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_CID", NULL, 0, method_test_CID_return, NULL);
}

static void method_test_MLS_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_MLS", obj, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Could be one or more packages */
	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg = &obj->Package.Elements[i];

		if (pkg->Package.Count != 2) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_MLSSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 2 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		if (pkg->Package.Elements[0].Type != ACPI_TYPE_STRING) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_MLSBadSubPackageReturnType",
				"%s sub-package %" PRIu32
				" element 0 is not a string.",
				name, i);
			failed = true;
		}

		if (pkg->Package.Elements[1].Type != ACPI_TYPE_BUFFER) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_MLSBadSubPackageReturnType",
				"%s sub-package %" PRIu32
				" element 1 is not a buffer.",
				name, i);
			failed = true;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_MLS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_MLS", NULL, 0, method_test_MLS_return, NULL);
}
static int method_test_HRV(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_HRV", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_STR(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_STR", NULL, 0, method_test_buffer_return, NULL);
}

static void method_test_PLD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* All elements in the package must be buffers */
	if (method_package_elements_all_type(fw, name, "_PLD", obj, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_PLD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PLD", NULL, 0, method_test_PLD_return, NULL);
}

static void method_test_SUB_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(buf);
	FWTS_UNUSED(private);

	if (obj == NULL) {
		method_failed_null_object(fw, name, "a buffer or integer");
		return;
	}

	if (obj->Type == ACPI_TYPE_STRING)
		if (obj->String.Pointer) {
			if (method_valid_HID_string(obj->String.Pointer))
				fwts_passed(fw,
					"%s returned a string '%s' "
					"as expected.",
					name, obj->String.Pointer);
			else
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"MethodSUBInvalidString",
					"%s returned a string '%s' "
					"but it was not a valid PNP ID or a "
					"valid ACPI ID.",
					name, obj->String.Pointer);
		} else {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_SUBNullString",
				"%s returned a NULL string.", name);
		}
	else {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_UIDBadReturnType",
			"Method _SUB did not return a string or an integer.");
	}
}


static int method_test_SUB(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_SUB", NULL, 0, method_test_SUB_return, NULL);
}

static int method_test_SUN(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_SUN", NULL, 0, method_test_integer_return, NULL);
}

static void method_test_UID_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(buf);
	FWTS_UNUSED(private);

	if (obj == NULL) {
		method_failed_null_object(fw, name, "a buffer or integer");
		return;
	}

	switch (obj->Type) {
	case ACPI_TYPE_STRING:
		if (obj->String.Pointer)
			fwts_passed(fw,
				"%s returned a string '%s' as expected.",
				name, obj->String.Pointer);
		else
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_UIDNullString",
				"%s returned a NULL string.", name);
		break;
	case ACPI_TYPE_INTEGER:
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
		break;
	default:
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_UIDBadReturnType",
			"Method %s did not return a string or an integer.", name);
		break;
	}
}

static int method_test_UID(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_UID", NULL, 0, method_test_UID_return, NULL);
}

/*
 *  Section 6.2 Device Configurations Objects
 */
static void method_test_CRS_size(
	fwts_framework *fw,
	const char *name,		/* full _CRS or _PRS path name */
	const char *objname,		/* name of _CRS or _PRS object */
	const char *tag,		/* error log tag */
	const size_t crs_length,	/* size of _CRS buffer */
	const size_t hdr_length,	/* size of _CRS header */
	const size_t data_length,	/* length of _CRS data w/o header */
	const size_t min,		/* minimum allowed _CRS data size */
	const size_t max,		/* maximum allowed _CRS data size */
	bool *passed)			/* pass/fail flag */
{
	if (crs_length < data_length + hdr_length) {
		fwts_failed(fw, LOG_LEVEL_HIGH, tag,
			"%s Resource size is %zd bytes long but "
			"the size stated in the %s buffer header  "
			"is %zd and hence is longer. The resource "
			"buffer is too short.",
			name, crs_length, objname, data_length);
		*passed = false;
		return;
	}

	if ((data_length < min) || (data_length > max)) {
		if (min != max) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tag,
				"%s Resource data size was %zd bytes long, "
				"expected it to be between %zd and %zd bytes",
				name, data_length, min, max);
			*passed = false;
		} else {
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tag,
				"%s Resource data size was %zd bytes long, "
				"expected it to be %zd bytes",
				name, data_length, min);
			*passed = false;
		}
	}
}

static void method_test_CRS_small_size(
	fwts_framework *fw,
	const char *name,
	const char *objname,
	const uint8_t *data,
	const size_t crs_length,
	const size_t min,
	const size_t max,
	bool *passed)
{
	size_t data_length = data[0] & 7;
	char tmp[128];

	snprintf(tmp, sizeof(tmp), "Method%sSmallResourceSize", objname);

	method_test_CRS_size(fw, name, objname, tmp,
		crs_length, 1, data_length, min, max, passed);
}


/*
 *  CRS small resource checks, simple checking
 */
static void method_test_CRS_small_resource_items(
	fwts_framework *fw,
	const char *name,
	const char *objname,
	const uint8_t *data,
	const size_t length,
	bool *passed,
	const char **tag)
{
	uint8_t tag_item = (data[0] >> 3) & 0xf;
	char tmp[128];

	static const char *types[] = {
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"IRQ Descriptor",
		"DMA Descriptor",
		"Start Dependent Functions Descriptor",
		"End Dependent Functions Descriptor",
		"I/O Port Descriptor",
		"Fixed Location I/O Port Descriptor",
		"Fixed DMA Descriptor",
		"Reserved",
		"Reserved",
		"Reserved",
		"Vendor Defined Descriptor",
		"End Tag Descriptor"
	};

	switch (tag_item) {
	case 0x4: /* 6.4.2.1 IRQ Descriptor */
		method_test_CRS_small_size(fw, name, objname, data, length, 2, 3, passed);
		break;
	case 0x5: /* 6.4.2.2 DMA Descriptor */
		method_test_CRS_small_size(fw, name, objname, data, length, 2, 2, passed);
		if (!*passed)	/* Too short, abort */
			break;
		if ((data[2] & 3) == 3) {
			snprintf(tmp, sizeof(tmp), "Method%sDmaDescriptor", objname);
			fwts_failed(fw, LOG_LEVEL_HIGH, tmp,
				"%s DMA transfer type preference is 0x%" PRIx8
				" which is reserved and invalid. See "
				"Section 6.4.2.2 of the ACPI specification.",
				name, data[2] & 3);
			*passed = false;
		}
		break;
	case 0x6: /* 6.4.2.3 Start Dependent Functions Descriptor */
		method_test_CRS_small_size(fw, name, objname, data, length, 0, 1, passed);
		break;
	case 0x7: /* 6.4.2.4 End Dependent Functions Descriptor */
		method_test_CRS_small_size(fw, name, objname, data, length, 0, 0, passed);
		break;
	case 0x8: /* 6.4.2.5 I/O Port Descriptor */
		method_test_CRS_small_size(fw, name, objname, data, length, 7, 7, passed);
		if (!*passed)	/* Too short, abort */
			break;
		if (data[1] & 0xfe) {
			snprintf(tmp, sizeof(tmp), "Method%sIoPortInfoReservedNonZero", objname);
			fwts_failed(fw, LOG_LEVEL_LOW, tmp,
				"%s I/O Port Descriptor Information field "
				"has reserved bits that are non-zero, got "
				"0x%" PRIx8 " and expected 0 or 1 for this "
				"field. ", name, data[1]);
			*passed = false;
		}
		if (((data[1] & 1) == 0) && (data[3] > 3)) {
			snprintf(tmp, sizeof(tmp), "Method%sIoPortInfoMinBase10BitAddr", objname);
			fwts_failed(fw, LOG_LEVEL_LOW, tmp,
				"%s I/O Port Descriptor range minimum "
				"base address is more than 10 bits however "
				"the Information field indicates that only "
				"a 10 bit address is being used.", name);
			*passed = false;
		}
		if (((data[1] & 1) == 0) && (data[5] > 3)) {
			snprintf(tmp, sizeof(tmp), "Method%sIoPortInfoMaxBase10BitAddr", objname);
			fwts_failed(fw, LOG_LEVEL_LOW, tmp,
				"%s I/O Port Descriptor range maximum "
				"base address is more than 10 bits however "
				"the Information field indicates that only "
				"a 10 bit address is being used.", name);
			*passed = false;
		}
		break;
	case 0x9: /* 6.4.2.6 Fixed Location I/O Port Descriptor */
		method_test_CRS_small_size(fw, name, objname, data, length, 3, 3, passed);
		break;
	case 0xa: /* 6.4.2.7 Fixed DMA Descriptor */
		method_test_CRS_small_size(fw, name, objname, data, length, 5, 5, passed);
		if (!*passed)	/* Too short, abort */
			break;
		if (data[5] > 5) {
			snprintf(tmp, sizeof(tmp), "Method%sFixedDmaTransferWidth", objname);
			fwts_failed(fw, LOG_LEVEL_HIGH, tmp,
				"%s DMA transfer width is 0x%" PRIx8
				" which is reserved and invalid. See "
				"Section 6.4.2.7 of the ACPI specification.",
				name, data[5]);
			*passed = false;
		}
		break;
	case 0xe: /* 6.4.2.8 Vendor-Defined Descriptor */
		method_test_CRS_small_size(fw, name, objname, data, length, 1, 7, passed);
		break;
	case 0xf: /* 6.4.2.9 End Tag */
		method_test_CRS_small_size(fw, name, objname, data, length, 1, 1, passed);
		break;
	default:
		snprintf(tmp, sizeof(tmp), "Method%sUnkownSmallResourceItem", objname);
		fwts_failed(fw, LOG_LEVEL_LOW, tmp,
			"%s tag bits 6:3 is an undefined "
			"small tag item name, value 0x%" PRIx8 ".",
			name, tag_item);
		fwts_advice(fw,
			"A small resource data type tag (byte 0, "
			"bits 6:3 of the %s buffer) contains "
			"an undefined small tag item 'name'. "
			"The %s buffer is therefore undefined "
			"and can't be used.  See section "
			"'6.4.2 Small Resource Data Type' of the ACPI "
			"specification, and also table 6-161.",
			objname, objname);
		*passed = false;
		break;
	}

	*tag = types[tag_item];
}

static void method_test_CRS_large_size(
	fwts_framework *fw,
	const char *name,
	const char *objname,
	const uint8_t *data,
	const size_t crs_length,
	const size_t min,
	const size_t max,
	bool *passed)
{
	size_t data_length;
	char tmp[128];

	/* Small _CRS resources have a 3 byte header */
	if (crs_length < 3) {
		snprintf(tmp, sizeof(tmp), "Method%sBufferTooSmall", objname);
		fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
			"%s should return a buffer of at least three bytes in length.", name);
		*passed = false;
		return;
	}

	data_length = (size_t)data[1] + ((size_t)data[2] << 8);

	snprintf(tmp, sizeof(tmp), "Method%sLargeResourceSize",objname);
	method_test_CRS_size(fw, name, objname, tmp,
		crs_length, 3, data_length, min, max, passed);
}

/*
 * Some CRS value fetching helper functions.  We handle all the
 * addresses and lengths in 64 bits to make life easier
 */
static uint64_t method_CRS_val64(const uint8_t *data)
{
	uint64_t val =
		((uint64_t)data[7] << 56) | ((uint64_t)data[6] << 48) |
		((uint64_t)data[5] << 40) | ((uint64_t)data[4] << 32) |
		((uint64_t)data[3] << 24) | ((uint64_t)data[2] << 16) |
		((uint64_t)data[1] << 8)  | (uint64_t)data[0];

	return val;
}

static uint64_t method_CRS_val32(const uint8_t *data)
{
	uint64_t val =
		((uint64_t)data[3] << 24) | ((uint64_t)data[2] << 16) |
		((uint64_t)data[1] << 8)  | (uint64_t)data[0];

	return val;
}

static uint64_t method_CRS_val24(const uint8_t *data)
{
	/* 24 bit values assume lower 8 bits are zero */
	uint64_t val =
		((uint64_t)data[1] << 16) | ((uint64_t)data[0] << 8);

	return val;
}

static uint64_t method_CRS_val16(const uint8_t *data)
{
	uint64_t val =
		((uint64_t)data[1] << 8) | (uint64_t)data[0];

	return val;
}

/*
 *  Sanity check addresses according to table 6-179 of ACPI spec
 */
static void method_test_CRS_mif_maf(
	fwts_framework *fw,
	const char *name,		/* Full _CRS or _PRS path name */
	const char *objname,		/* _CRS or _PRS name */
	const uint8_t flag,		/* _MIF _MAF flag field */
	const uint64_t min,		/* Min address */
	const uint64_t max,		/* Max address */
	const uint64_t len,		/* Range length */
	const uint64_t granularity,	/* Address granularity */
	const char *tag,		/* failed error tag */
	const char *type,		/* Resource type */
	bool *passed)
{
	char tmp[128];
	uint8_t mif = (flag >> 2) & 1;
	uint8_t maf = (flag >> 3) & 1;

	static char *mif_maf_advice =
		"See section '6.4.3.5 Address Space Resource Descriptors' "
		"table 6-179 of the ACPI specification for more details "
		"about how the _MIF, _MAF and memory range and granularity "
		"rules apply. Typically the kernel does not care about these "
		"being correct, so this is a minor issue.";

	/* Table 6-179 Valid combination of Address Space Descriptors fields */
	if (len == 0) {
		if ((mif == 1) && (maf == 1)) {
			snprintf(tmp, sizeof(tmp), "Method%s%sMifMafBothOne", objname, tag);
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				tmp,
				"%s %s _MIF and _MAF flags are both "
				"set to one which is invalid when "
				"the length field is 0.",
				name, type);
			fwts_advice(fw, "%s", mif_maf_advice);
			*passed = false;
		}
		if ((mif == 1) && (min % (granularity + 1) != 0)) {
			snprintf(tmp, sizeof(tmp), "Method%s%sMinNotMultipleOfGran", objname, tag);
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				tmp,
				"%s %s _MIN address is not a multiple "
				"of the granularity when _MIF is 1.",
				name, type);
			fwts_advice(fw, "%s", mif_maf_advice);
			*passed = false;
		}
		if ((maf == 1) && (max % (granularity - 1) != 0)) {
			snprintf(tmp, sizeof(tmp), "Method%s%sMaxNotMultipleOfGran", objname, tag);
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				tmp,
				"%s %s _MAX address is not a multiple "
				"of the granularity when _MAF is 1.",
				name, type);
			fwts_advice(fw, "%s", mif_maf_advice);
			*passed = false;
		}
	} else {
		if ((mif == 0) && (maf == 0) &&
		    (len % (granularity + 1) != 0)) {
			snprintf(tmp, sizeof(tmp), "Method%s%sLenNotMultipleOfGran", objname, tag);
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				tmp,
				"%s %s length is not a multiple "
				"of the granularity when _MIF "
				"and _MIF are 0.",
				name, type);
			fwts_advice(fw, "%s", mif_maf_advice);
			*passed = false;
		}
		if (((mif == 0) && (maf == 1)) ||
 		    ((mif == 1) && (maf == 0))) {
			snprintf(tmp, sizeof(tmp), "Method%s%sMifMafInvalid", objname, tag);
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				tmp,
				"%s %s _MIF and _MAF flags are either "
				"0 and 1 or 1 and 0 which is invalid when "
				"the length field is non-zero.",
				name, type);
			fwts_advice(fw, "%s", mif_maf_advice);
			*passed = false;
		}
		if ((mif == 1) && (maf == 1)) {
			if (granularity != 0) {
				snprintf(tmp, sizeof(tmp), "Method%s%sGranularityNotZero", objname, tag);
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					tmp,
					"%s %s granularity 0x%" PRIx64
					" is not zero as expected when "
					"_MIF and _MAF are both 1.",
					name, type, granularity);
				fwts_advice(fw, "%s", mif_maf_advice);
				*passed = false;
			}
			if (min > max) {
				snprintf(tmp, sizeof(tmp), "Method%s%sMaxLessThanMin", objname, tag);
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					tmp,
					"%s %s minimum address range 0x%" PRIx64
					" is greater than the maximum address "
					"range 0x%" PRIx64 ".",
					name, type, min, max);
				fwts_advice(fw, "%s", mif_maf_advice);
				*passed = false;
			}
			if (max - min + 1 != len) {
				snprintf(tmp, sizeof(tmp), "Method%s%sLengthInvalid", objname, tag);
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					tmp,
					"%s %s length 0x%" PRIx64
					" does not match the difference between "
					"the minimum and maximum address ranges "
					"0x%" PRIx64 "-0x%" PRIx64 ".",
					name, type, len, min, max);
				fwts_advice(fw, "%s", mif_maf_advice);
				*passed = false;
			}
		}
	}
}

/*
 *  CRS large resource checks, simple checking
 */
static void method_test_CRS_large_resource_items(
	fwts_framework *fw,
	const char *name,
	const char *objname,
	const uint8_t *data,
	const uint64_t length,
	bool *passed,
	const char **tag)
{
	uint64_t min, max, len, gra;
	uint8_t tag_item = data[0] & 0x7f;
	char tmp[128];

	static const char *types[] = {
		"Reserved",
		"24-bit Memory Range Descriptor",
		"Generic Register Descriptor",
		"Reserved",
		"Vendor Defined Descriptor",
		"32-bit Memory Range Descriptor",
		"32-bit Fixed Location Memory Range Descriptor",
		"DWORD Address Space Descriptor",
		"WORD Address Space Descriptor",
		"Extended IRQ Descriptor",
		"QWORD Address Space Descriptor",
		"Extended Addresss Space Descriptor",
		"GPIO Connection Descriptor",
		"Reserved",
		"Generic Serial Bus Connection Descriptor",
		"Reserved",
	};

	switch (tag_item) {
	case 0x1: /* 6.4.3.1 24-Bit Memory Range Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 9, 9, passed);
		if (!*passed)	/* Too short, abort */
			break;
		min = method_CRS_val24(&data[4]);
		max = method_CRS_val24(&data[6]);
		len = method_CRS_val16(&data[10]);
		if (max < min) {
			snprintf(tmp, sizeof(tmp), "Method%s24BitMemRangeMaxLessThanMin", objname);
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
				"%s 24-Bit Memory Range Descriptor minimum "
				"address range 0x%" PRIx64 " is greater than "
				"the maximum address range 0x%" PRIx64 ".",
				name, min, max);
			*passed = false;
		}
		if (len > max + 1 - min) {
			snprintf(tmp, sizeof(tmp), "Method%s24BitMemRangeLengthTooLarge", objname);
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
				"%s 24-Bit Memory Range Descriptor length "
				"0x%" PRIx64 " is greater than size between the "
				"the minimum and maximum address ranges "
				"0x%" PRIx64 "-0x%" PRIx64 ".",
				name, len, min, max);
			*passed = false;
		}
		break;
	case 0x2: /* 6.4.3.7 Generic Register Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 12, 12, passed);
		if (!*passed)
			break;
		switch (data[3]) {
		case 0x00 ... 0x04:
		case 0x0a:
		case 0x7f:
			/* Valid values */
			break;
		default:
			snprintf(tmp, sizeof(tmp), "Method%sGenericRegAddrSpaceIdInvalid", objname);
			fwts_failed(fw, LOG_LEVEL_HIGH, tmp,
				"%s Generic Register Descriptor has an invalid "
				"Address Space ID 0x%" PRIx8 ".",
				name, data[3]);
			*passed = false;
		}
		if (data[6] > 4) {
			snprintf(tmp, sizeof(tmp), "Method%sGenericRegAddrSizeInvalid", objname);
			fwts_failed(fw, LOG_LEVEL_HIGH, tmp,
				"%s Generic Register Descriptor has an invalid "
				"Address Access Size 0x%" PRIx8 ".",
				name, data[6]);
			*passed = false;
		}
		break;
	case 0x4: /* 6.4.3.2 Vendor-Defined Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 0, 65535, passed);
		break;
	case 0x5: /* 6.4.3.3 32-Bit Memory Range Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 17, 17, passed);
		if (!*passed)
			break;
		min = method_CRS_val32(&data[4]);
		max = method_CRS_val32(&data[8]);
		len = method_CRS_val32(&data[16]);
		if (max < min) {
			snprintf(tmp, sizeof(tmp), "Method%s32BitMemRangeMaxLessThanMin", objname);
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
				"%s 32-Bit Memory Range Descriptor minimum "
				"address range 0x%" PRIx64 " is greater than "
				"the maximum address range 0x%" PRIx64 ".",
				name, min, max);
			*passed = false;
		}
		if (len > max + 1 - min) {
			snprintf(tmp, sizeof(tmp), "Method%s32BitMemRangeLengthTooLarge", objname);
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
				"%s 32-Bit Memory Range Descriptor length "
				"0x%" PRIx64 " is greater than size between the "
				"the minimum and maximum address ranges "
				"0x%" PRIx64 "-0x%" PRIx64 ".",
				name, len, min, max);
			*passed = false;
		}
		break;
	case 0x6: /* 6.4.3.4 32-Bit Fixed Memory Range Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 9, 9, passed);
		/* Not much can be checked for this descriptor */
		break;
	case 0x7: /* 6.4.3.5.2 DWord Address Space Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 23, 65535, passed);
		if (!*passed)	/* Too short, abort */
			break;
		gra = method_CRS_val32(&data[6]);
		min = method_CRS_val32(&data[10]);
		max = method_CRS_val32(&data[14]);
		len = method_CRS_val32(&data[22]);

		method_test_CRS_mif_maf(fw, name, objname, data[4],
			min, max, len, gra,
			"64BitDWordAddrSpace",
			types[0x7], passed);
		break;
	case 0x8: /* 6.4.3.5.3 Word Address Space Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 13, 65535, passed);
		if (!*passed)	/* Too short, abort */
			break;
		gra = method_CRS_val16(&data[6]);
		min = method_CRS_val16(&data[8]);
		max = method_CRS_val16(&data[10]);
		len = method_CRS_val16(&data[14]);

		method_test_CRS_mif_maf(fw, name, objname, data[4],
			min, max, len, gra,
			"64BitWordAddrSpace",
			types[0x8], passed);
		break;
	case 0x9: /* 6.4.3.6 Extended Interrupt Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 6, 65535, passed);
		/* Not much can be checked for this descriptor */
		break;
	case 0xa: /* 6.4.3.5.1 QWord Address Space Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 43, 65535, passed);
		if (!*passed)	/* Too short, abort */
			break;
		gra = method_CRS_val64(&data[6]);
		min = method_CRS_val64(&data[14]);
		max = method_CRS_val64(&data[22]);
		len = method_CRS_val64(&data[38]);

		method_test_CRS_mif_maf(fw, name, objname, data[4],
			min, max, len, gra,
			"64BitQWordAddrSpace",
			types[0xa], passed);
		break;
	case 0xb: /* 6.4.3.5.4 Extended Address Space Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 53, 53, passed);
		if (!*passed)	/* Too short, abort */
			break;
		gra = method_CRS_val64(&data[8]);
		min = method_CRS_val64(&data[16]);
		max = method_CRS_val64(&data[24]);
		len = method_CRS_val64(&data[40]);

		method_test_CRS_mif_maf(fw, name, objname, data[4],
			min, max, len, gra,
			"64BitExtAddrSpace",
			types[0xb], passed);
		break;
	case 0xc: /* 6.4.3.8.1 GPIO Connection Descriptor */
		method_test_CRS_large_size(fw, name, objname, data, length, 22, 65535, passed);
		if (!*passed)	/* Too short, abort */
			break;
		if (data[4] > 2) {
			snprintf(tmp, sizeof(tmp), "Method%sGpioConnTypeInvalid", objname);
			fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
				"%s GPIO Connection Descriptor has an invalid "
				"Connection Type 0x%" PRIx8 ".",
				name, data[2]);
			*passed = false;
			fwts_advice(fw,
				"The GPIO pin connection type is "
				"not recognised. It should be either "
				"0x00 (interrupt connection) or "
				"0x01 (I/O connection). See table "
				"6-189 in section 6.4.3.8.1 of the ACPI "
                                "specification.");
		}
		if ((data[9] > 0x03) && (data[9] < 0x80)) {
			snprintf(tmp, sizeof(tmp), "Method%sGpioConnTypeInvalid", objname);
			fwts_failed(fw, LOG_LEVEL_LOW, tmp,
				"%s GPIO Connection Descriptor has an invalid "
				"Pin Configuration Type 0x%" PRIx8 ".",
				name, data[9]);
			*passed = false;
			fwts_advice(fw,
				"The GPIO pin configuration type "
				"is not recognised. It should be one of:"
				"0x00 (default), 0x01 (pull-up), "
				"0x02 (pull-down), 0x03 (no-pull), "
				"0x80-0xff (vendor defined). See table "
				"6-189 in section 6.4.3.8.1 of the ACPI "
				"specification.");
		}
		break;
	case 0xe: /* 6.4.3.8.2 Serial Bus Connection Descriptors */
		method_test_CRS_large_size(fw, name, objname, data, length, 11, 65535, passed);
		/* Don't care */
		break;
	default:
		snprintf(tmp, sizeof(tmp), "Method%sUnkownLargeResourceItem", objname);
		fwts_failed(fw, LOG_LEVEL_LOW, tmp,
			"%s tag bits 6:0 is an undefined "
			"large tag item name, value 0x%" PRIx8 ".",
			name, tag_item);
		fwts_advice(fw,
			"A large resource data type tag (byte 0 of the "
			"%s buffer) contains an undefined large tag "
			"item 'name'. The %s buffer is therefore "
			"undefined and can't be used.  See section "
			"'6.4.3 Large Resource Data Type' of the ACPI "
			"specification, and also table 6-173.",
			objname, objname);
		*passed = false;
		break;
	}

	*tag = types[tag_item < 16 ? tag_item : 0];
}

static void method_test_CRS_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint8_t *data;
	bool passed = true;
	const char *tag = "Unknown";
	char *objname = (char*)private;
	char tmp[128];

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;
	if (obj->Buffer.Pointer == NULL) {
		snprintf(tmp, sizeof(tmp), "Method%sNullBuffer", objname);
		fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
			"%s returned a NULL buffer pointer.", name);
		return;
	}
	if (obj->Buffer.Length < 1) {
		snprintf(tmp, sizeof(tmp), "Method%sBufferTooSmall", objname);
		fwts_failed(fw, LOG_LEVEL_MEDIUM, tmp,
			"%s should return a buffer of at least one byte in length.", name);
		return;
	}

	data = (uint8_t*)obj->Buffer.Pointer;

	if (data[0] & 128)
		method_test_CRS_large_resource_items(fw, name, objname, data, obj->Buffer.Length, &passed, &tag);
	else
		method_test_CRS_small_resource_items(fw, name, objname, data, obj->Buffer.Length, &passed, &tag);

	if (passed)
		fwts_passed(fw, "%s (%s) looks sane.", name, tag);
}

static int method_test_CRS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MANDATORY,
		"_CRS", NULL, 0, method_test_CRS_return, "_CRS");
}

static int method_test_PRS(fwts_framework *fw)
{
	/* Re-use the _CRS checking on the returned buffer */
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PRS", NULL, 0, method_test_CRS_return, "_PRS");
}

static void method_test_PRT_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i, j;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_PRT", obj, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		ACPI_OBJECT *element;
		pkg = &obj->Package.Elements[i];

		/* check size of sub-packages */
		if (pkg->Package.Count != 4) {
			fwts_failed(fw, LOG_LEVEL_CRITICAL,
				"Method_PRTSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to have 4"
				"elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		/* check types of sub-packages' elements */
		for (j = 0; j < 4; j++) {
			element = &pkg->Package.Elements[j];

			if (j == 2) {
				if (element->Type != ACPI_TYPE_INTEGER && element->Type != ACPI_TYPE_LOCAL_REFERENCE) {
					fwts_failed(fw, LOG_LEVEL_CRITICAL,
						"Method_PRTBadSubElementType",
						"%s element %" PRIu32 " is not an integer or a NamePath.", name, j);
					failed = true;
				}
				continue;
			}

			if (element->Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_CRITICAL,
					"Method_PRTBadSubElementType",
					"%s element %" PRIu32 " is not an integer.", name, j);
				failed = true;
			}
		}

		/* check sub-packages's PCI address */
		element = &pkg->Package.Elements[0];
		if ((element->Integer.Value & 0xFFFF) != 0xFFFF) {
			fwts_failed(fw, LOG_LEVEL_CRITICAL,
				"Method_PRTBadSubElement",
				"%s element 0 is expected to end with 0xFFFF, got 0x%" PRIx32 ".",
				name, (uint32_t) element->Integer.Value);
			failed = true;
		}

		/* check sub-packages's PCI pin number */
		element = &pkg->Package.Elements[1];
		if (element->Integer.Value > 3) {
			fwts_failed(fw, LOG_LEVEL_CRITICAL,
				"Method_PRTBadSubElement",
				"%s element 1 is expected to be 0..3, got 0x%" PRIx32 ".",
				name, (uint32_t) element->Integer.Value);
			failed = true;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_PRT(fwts_framework *fw)
{
	/* Re-use the _CRS checking on the returned buffer */
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PRT", NULL, 0, method_test_PRT_return, "_PRT");
}

static int method_test_DMA(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DMA", NULL, 0, method_test_buffer_return, NULL);
}

static void method_test_FIX_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	char tmp[8];
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* All elements in the package must be integers */
	if (method_package_elements_all_type(fw, name, "_FIX", obj, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	/* And they need to be valid IDs */
	for (i = 0; i < obj->Package.Count; i++) {
		if (!method_valid_EISA_ID(
			(uint32_t)obj->Package.Elements[i].Integer.Value,
			tmp, sizeof(tmp))) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_FIXInvalidElementValue",
				"%s returned an integer "
				"0x%8.8" PRIx64 " in package element "
				"%" PRIu32 " that is not a valid "
				"EISA ID.", name,
				(uint64_t)obj->Integer.Value, i);
			failed = true;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_FIX(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_FIX", NULL, 0, method_test_FIX_return, NULL);
}

/*
 *  Section 6.2.5 _DSD Device Specific Data, ACPI 5.1
 */
static void method_test_DSD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Must be an even number of items in package */
	if (obj->Package.Count & 1) {
		fwts_failed(fw, LOG_LEVEL_HIGH, "Method_DSDElementCount",
			"There must be an even number of items in the %s "
			"package, instead, got %" PRIu32 " elements.",
			name, obj->Package.Count);
		return;
	}
	for (i = 0; i < obj->Package.Count; i += 2) {
		/* UUID should be a buffer */
		if (!method_type_matches(obj->Package.Elements[i].Type, ACPI_TYPE_BUFFER)) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_DSDElementBuffer",
				"%s package element %" PRIu32 " was not the expected "
				"type '%s', was instead type '%s'.",
				name, i,
				method_type_name(ACPI_TYPE_BUFFER),
				method_type_name(obj->Package.Elements[i].Type));
		}

		/* Data should be a package */
		if (!method_type_matches(obj->Package.Elements[i + 1].Type, ACPI_TYPE_PACKAGE)) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_DSDElementPackage",
				"%s package element %" PRIu32 " was not the expected "
				"type '%s', was instead type '%s'.",
				name, i + 1,
				method_type_name(ACPI_TYPE_PACKAGE),
				method_type_name(obj->Package.Elements[i + 1].Type));
		}
	}
}

static int method_test_DSD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DSD", NULL, 0, method_test_DSD_return, NULL);
}

static int method_test_DIS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DIS", NULL, 0, method_test_NULL_return, NULL);
}

static int method_test_GSB(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GSB", NULL, 0, method_test_integer_return, NULL);
}

static void method_test_HPP_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Must be 4 elements in the package */
	if (method_package_count_equal(fw, name, "_HPP", obj, 4) != FWTS_OK)
		return;

	/* All 4 elements in the package must be integers */
	if (method_package_elements_all_type(fw, name, "_HPP", obj, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_HPP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_HPP", NULL, 0, method_test_HPP_return, NULL);
}

static int method_test_PXM(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PXM", NULL, 0, method_test_integer_return, NULL);
}

/* Section 6.2.17 _CCA */
static int method_test_CCA(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_CCA", NULL, 0, method_test_integer_return, NULL);
}

/*
 * Section 6.3 Device Insertion, Removal and Status Objects
 */
static void method_test_EDL_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_EDL",
		obj, ACPI_TYPE_LOCAL_REFERENCE) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_EDL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_EDL", NULL, 0, method_test_EDL_return, NULL);
}

static int method_test_EJD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_EJD", NULL, 0, method_test_string_return, NULL);
}

#define method_test_EJx(name)					\
static int method_test ## name(fwts_framework *fw)		\
{								\
	ACPI_OBJECT arg[1];					\
								\
	arg[0].Type = ACPI_TYPE_INTEGER;			\
	arg[0].Integer.Value = 1;				\
								\
	return method_evaluate_method(fw, METHOD_OPTIONAL,	\
		# name, arg, 1, method_test_NULL_return, # name); \
}

method_test_EJx(_EJ0)
method_test_EJx(_EJ1)
method_test_EJx(_EJ2)
method_test_EJx(_EJ3)
method_test_EJx(_EJ4)

static int method_test_LCK(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_LCK", arg, 1, method_test_NULL_return, NULL);
}

static int method_test_RMV(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_RMV",
		NULL, 0, method_test_passed_failed_return, "_RMV");
}

static void method_test_STA_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if ((obj->Integer.Value & 3) == 2) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_STAEnabledNotPresent",
			"%s indicates that the device is enabled "
			"but not present, which is impossible.", name);
		failed = true;
	}
	if ((obj->Integer.Value & ~0x1f) != 0) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_STAReservedBitsSet",
			"%s is returning non-zero reserved "
			"bits 5-31. These should be zero.", name);
		failed = true;
	}

	if (!failed)
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_STA(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_STA",
		NULL, 0, method_test_STA_return, "_STA");
}


/*
 * Section 6.5 Other Objects and Controls
 */
static int method_test_BBN(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_BBN",
		NULL, 0, method_test_integer_return, "_BBN");
}

static int method_test_BDN(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MOBILE, "_BDN",
		NULL, 0, method_test_integer_return, "_BDN");
}

static void method_test_DEP_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_DEP", obj, ACPI_TYPE_LOCAL_REFERENCE) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_DEP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DEP", NULL, 0, method_test_DEP_return, NULL);
}

static int method_test_FIT(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_FIT", NULL, 0, method_test_buffer_return, NULL);
}

static int method_test_DCK(fwts_framework *fw)
{
	int i;

	for (i = 0; i <= 1; i++) {	/* Undock, Dock */
		ACPI_OBJECT arg[1];
		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = i;
		if (method_evaluate_method(fw, METHOD_MOBILE, "_DCK", arg,
			1, method_test_passed_failed_return, "_DCK") != FWTS_OK)
			break;
		fwts_log_nl(fw);
	}
	return FWTS_OK;
}

static int method_test_INI(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_INI", NULL, 0, method_test_NULL_return, NULL);
}

static void method_test_SEG_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if ((obj->Integer.Value & 0xffff0000)) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_SEGIllegalReserved",
			"%s returned value 0x%8.8" PRIx64 " and some of the "
			"upper 16 reserved bits are set when they "
			"should in fact be zero.",
			name, (uint64_t)obj->Integer.Value);
	} else
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_SEG(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_SEG",
		NULL, 0, method_test_SEG_return, "_SEG");
}

static void method_test_GLK_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(buf);
	FWTS_UNUSED(private);

	switch (obj->Type) {
	case ACPI_TYPE_INTEGER:
		if (obj->Integer.Value == 0 || obj->Integer.Value == 1)
			fwts_passed(fw, "%s returned an integer 0x%8.8" PRIx64,
				name, (uint64_t)obj->Integer.Value);
		else
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"MethodGLKInvalidInteger",
				"%s returned an invalid integer 0x%8.8" PRIx64,
				name, (uint64_t)obj->Integer.Value);
		break;
	default:
		fwts_failed(fw, LOG_LEVEL_HIGH, "Method_GLKBadReturnType",
			"%s did not return an integer.", name);
		break;
	}
}

static int method_test_GLK(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_GLK",
		NULL, 0, method_test_GLK_return, "_GLK");
}

/*
 * Section 7.1 Declaring a Power Resource Object
 */
static int method_test_ON_(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_ON_", NULL, 0, method_test_NULL_return, NULL);
}

static int method_test_OFF(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_OFF", NULL, 0, method_test_NULL_return, NULL);
}


/*
 * Section 7.2  Device Power Management Objects
 */
static int method_test_DSW(fwts_framework *fw)
{
	ACPI_OBJECT arg[3];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;
	arg[1].Type = ACPI_TYPE_INTEGER;
	arg[1].Integer.Value = 0;
	arg[2].Type = ACPI_TYPE_INTEGER;
	arg[2].Integer.Value = 3;

	return method_evaluate_method(fw, METHOD_OPTIONAL, "_DSW",
		arg, 3, method_test_NULL_return, NULL);
}

static int method_test_PSx(fwts_framework *fw, char *name)
{
	/*
	 *  iASL (ACPICA commit 6922796cfdfca041fdb96dc9e3918cbc7f43d830)
	 *  checks that _PS0 must exist if we have _PS1, _PS2, _PS3
	 *  so check this here too.
	 */
	if ((fwts_acpi_object_exists(name) != NULL) &&
            (fwts_acpi_object_exists("_PS0") == NULL)) {
		fwts_failed(fw, LOG_LEVEL_HIGH, "Method_PSx",
			"%s requires that the _PS0 "
			"control method must also exist, however, "
			"it was not found.", name);
	}
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		name, NULL, 0, method_test_NULL_return, name);
}

static void method_test_PRW_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_min(fw, name, "_PRW", obj, 2) != FWTS_OK)
		return;

	if (obj->Package.Elements[0].Type != ACPI_TYPE_INTEGER &&
	    obj->Package.Elements[0].Type != ACPI_TYPE_PACKAGE) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PRWBadPackageReturnType",
			"%s element 0 is not an integer or an package.", name);
		failed = true;
	}

	if (obj->Package.Elements[0].Type == ACPI_TYPE_PACKAGE) {
		ACPI_OBJECT *pkg;
		pkg = &obj->Package.Elements[0];
		if (pkg->Package.Count != 2) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PRWSubPackageElementCount",
				"%s sub-package 0  was expected to have 2"
				"elements, got %" PRIu32 " elements instead.",
				name, pkg->Package.Count);
			failed = true;
		}

		if (pkg->Package.Elements[0].Type != ACPI_TYPE_LOCAL_REFERENCE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PRWBadSubPackageElementType",
				"%s sub-package 0 element 0 is not "
				"a reference.",name);
			failed = true;
		}

		if (pkg->Package.Elements[1].Type != ACPI_TYPE_INTEGER) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PRWBadSubPackageElementType",
				"%s sub-package 0 element 0 is not "
				"an integer.",name);
			failed = true;
		}
	}

	if (obj->Package.Elements[1].Type != ACPI_TYPE_INTEGER) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PRWBadPackageReturnType",
			"%s element 1 is not an integer.", name);
		failed = true;
	}

	for (i = 2; i < obj->Package.Count - 1; i++) {
		if (obj->Package.Elements[i].Type != ACPI_TYPE_LOCAL_REFERENCE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PRWBadPackageReturnType",
				"%s package %" PRIu32
				" element 0 is not a reference.",
				name, i);
			failed = true;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_PRW(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PRW", NULL, 0, method_test_PRW_return, NULL);
}

static int method_test_PS0(fwts_framework *fw)
{
	/*
	 *  iASL (ACPICA commit 6922796cfdfca041fdb96dc9e3918cbc7f43d830)
	 *  checks that one of _PS1, _PS2, _PS3 must exist if _PS0 exists.
	 */
	if (fwts_acpi_object_exists("_PS0") != NULL) {
		bool ok = false;
		int i;

		for (i = 1; i < 4; i++) {
			char name[5];

			snprintf(name, sizeof(name), "_PS%1d", i);
			if (fwts_acpi_object_exists(name) != NULL) {
				ok = true;
				break;
			}
		}
		if (!ok) {
			fwts_failed(fw, LOG_LEVEL_HIGH, "Method_PS0",
				"_PS0 requires that one of the _PS1, _PS2, _PS3 "
				"control methods must also exist, however, "
				"none were found.");
		}
	}
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_PS0",
		 NULL, 0, method_test_NULL_return, "_PS0");
}

static int method_test_PS1(fwts_framework *fw)
{
	return method_test_PSx(fw, "_PS1");
}

static int method_test_PS2(fwts_framework *fw)
{
	return method_test_PSx(fw, "_PS2");
}

static int method_test_PS3(fwts_framework *fw)
{
	return method_test_PSx(fw, "_PS3");
}

static int method_test_PSC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PSC", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_PSE(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PSE", arg, 1, method_test_NULL_return, NULL);
}

static void method_test_power_resources_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	char *objname = (char *)private;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, objname,
		obj, ACPI_TYPE_LOCAL_REFERENCE) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

#define method_test_POWER(name)						\
static int method_test ## name(fwts_framework *fw)			\
{									\
	return method_evaluate_method(fw, METHOD_OPTIONAL,		\
		# name, NULL, 0, method_test_power_resources_return, # name);\
}

method_test_POWER(_PR0)
method_test_POWER(_PR1)
method_test_POWER(_PR2)
method_test_POWER(_PR3)
method_test_POWER(_PRE)

static int method_test_PSW(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PSW", arg, 1, method_test_NULL_return, NULL);
}

#define method_test_SxD(name)						\
static int method_test ## name(fwts_framework *fw)			\
{									\
	return method_evaluate_method(fw, METHOD_OPTIONAL,		\
		# name, NULL, 0, method_test_integer_return, # name);	\
}

method_test_SxD(_S1D)
method_test_SxD(_S2D)
method_test_SxD(_S3D)
method_test_SxD(_S4D)

#define method_test_SxW(name)						\
static int method_test ## name(fwts_framework *fw)			\
{									\
	return method_evaluate_method(fw, METHOD_OPTIONAL,		\
		# name, NULL, 0, method_test_integer_return, # name);	\
}

method_test_SxW(_S0W)
method_test_SxW(_S1W)
method_test_SxW(_S2W)
method_test_SxW(_S3W)
method_test_SxW(_S4W)

static int method_test_RST(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_RST", NULL, 0, method_test_NULL_return, NULL);
}

static void method_test_PRR_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_PRR", obj, 1) != FWTS_OK)
		return;

	if (obj->Package.Elements[0].Type != ACPI_TYPE_LOCAL_REFERENCE) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PRRElementType",
			"%s returned a package that does not contain "
			"a reference.", name);
		return;
	}

	method_passed_sane(fw, name, "package");
}

static int method_test_PRR(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PRR", NULL, 0, method_test_PRR_return, NULL);
}

static int method_test_IRC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_IRC", NULL, 0, method_test_NULL_return, NULL);
}

/*
 * Section 7.3 OEM Supplied System-Level Control Methods
 */
static void method_test_Sx__return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/*
	 * The ACPI spec states it should have 1 integer, with the
	 * values packed into each byte. However, nearly all BIOS
	 * vendors don't do this, instead they return a package of
	 * 2 or more integers with each integer lower byte containing
	 * the data we are interested in. The kernel handles this
	 * the non-compliant way. Doh. See drivers/acpi/acpica/hwxface.c
	 * for the kernel implementation and also
	 * source/components/hardware/hwxface.c in the reference ACPICA
	 * sources.
 	 */

	/* Something is really wrong if we don't have any elements in _Sx_ */
	if (obj->Package.Count < 1) {
		fwts_failed(fw, LOG_LEVEL_HIGH, "Method_SxElementCount",
			"The kernel expects a package of at least two "
			"integers, and %s only returned %" PRIu32
			" elements in the package.",
			name, obj->Package.Count);
		return;
	}

	/*
	 * Oh dear, BIOS is conforming to the spec but won't work in
	 * Linux
	 */
	if (obj->Package.Count == 1) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_SxElementCount",
			"The ACPI specification states that %s should "
			"return a package of a single integer which "
			"this firmware does do. However, nearly all of the "
			"BIOS vendors return the values in the low 8 bits "
			"in a package of 2 to 4 integers which is not "
			"compliant with the specification BUT is the way "
			"that the ACPICA reference engine and the kernel "
			"expect. So, while this is conforming to the ACPI "
			"specification it will in fact not work in the "
			"Linux kernel.", name);
		return;
	}

	/* Yes, we really want integers! */
	if ((obj->Package.Elements[0].Type != ACPI_TYPE_INTEGER) ||
	    (obj->Package.Elements[1].Type != ACPI_TYPE_INTEGER)) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_SxElementType",
			"%s returned a package that did not contain "
			"an integer.", name);
		return;
	}

	if (obj->Package.Elements[0].Integer.Value & 0xffffff00) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_SxElementValue",
			"%s package element 0 had upper 24 bits "
			"of bits that were non-zero.", name);
		failed = true;
	}

	if (obj->Package.Elements[1].Integer.Value & 0xffffff00) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_SxElementValue",
			"%s package element 1 had upper 24 bits "
			"of bits that were non-zero.", name);
		failed = true;
	}

	fwts_log_info(fw, "%s PM1a_CNT.SLP_TYP value: 0x%8.8" PRIx64, name,
		(uint64_t)obj->Package.Elements[0].Integer.Value);
	fwts_log_info(fw, "%s PM1b_CNT.SLP_TYP value: 0x%8.8" PRIx64, name,
		(uint64_t)obj->Package.Elements[1].Integer.Value);

	if (!failed)
		method_passed_sane(fw, name, "package");
}

#define method_test_Sx_(name)						\
static int method_test ## name(fwts_framework *fw)			\
{									\
	return method_evaluate_method(fw, METHOD_OPTIONAL,		\
		# name, NULL, 0, method_test_Sx__return, # name);	\
}

method_test_Sx_(_S0_)
method_test_Sx_(_S1_)
method_test_Sx_(_S2_)
method_test_Sx_(_S3_)
method_test_Sx_(_S4_)
method_test_Sx_(_S5_)

static int method_test_SWS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_SWS", NULL, 0, method_test_integer_return, NULL);
}

/*
 * Section 8.4 Declaring Processors
 */
static void method_test_CPC_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint8_t revision;

	static fwts_package_element elementsv1[] = {
		{ ACPI_TYPE_INTEGER,	"Number of Entries" },
		{ ACPI_TYPE_INTEGER,	"Revision" },
		{ ACPI_TYPE_INTBUF,	"Highest Performance" },
		{ ACPI_TYPE_INTBUF,	"Nominal Performance" },
		{ ACPI_TYPE_INTBUF,	"Lowest Non Linear Performance" },
		{ ACPI_TYPE_INTBUF,	"Lowest Performance" },
		{ ACPI_TYPE_BUFFER,	"Guaranteed Performance Register" },
		{ ACPI_TYPE_BUFFER,	"Desired Performance Register" },
		{ ACPI_TYPE_BUFFER,	"Minimum Performance Register" },
		{ ACPI_TYPE_BUFFER,	"Maximum Performance Register" },
		{ ACPI_TYPE_BUFFER,	"Performance Reduction Tolerance Register" },
		{ ACPI_TYPE_BUFFER,	"Timed Window Register" },
		{ ACPI_TYPE_INTBUF,	"Counter Wraparound Time" },
		{ ACPI_TYPE_BUFFER,	"Nominal Counter Register" },
		{ ACPI_TYPE_BUFFER,	"Delivered Counter Register" },
		{ ACPI_TYPE_BUFFER,	"Performance Limited Register" },
		{ ACPI_TYPE_BUFFER,	"Enable Register" }
	};

	static fwts_package_element elementsv2[] = {
		{ ACPI_TYPE_INTEGER,	"Number of Entries" },
		{ ACPI_TYPE_INTEGER,	"Revision" },
		{ ACPI_TYPE_INTBUF,	"Highest Performance" },
		{ ACPI_TYPE_INTBUF,	"Nominal Performance" },
		{ ACPI_TYPE_INTBUF,	"Lowest Non Linear Performance" },
		{ ACPI_TYPE_INTBUF,	"Lowest Performance" },
		{ ACPI_TYPE_BUFFER,	"Guaranteed Performance Register" },
		{ ACPI_TYPE_BUFFER,	"Desired Performance Register" },
		{ ACPI_TYPE_BUFFER,	"Minimum Performance Register" },
		{ ACPI_TYPE_BUFFER,	"Maximum Performance Register" },
		{ ACPI_TYPE_BUFFER,	"Performance Reduction Tolerance Register" },
		{ ACPI_TYPE_BUFFER,	"Timed Window Register" },
		{ ACPI_TYPE_INTBUF,	"Counter Wraparound Time" },
		{ ACPI_TYPE_BUFFER,	"Reference Performance Counter Register" },
		{ ACPI_TYPE_BUFFER,	"Delivered Performance Counter Register" },
		{ ACPI_TYPE_BUFFER,	"Performance Limited Register" },
		{ ACPI_TYPE_BUFFER,	"CPPC Enable Register" },
		{ ACPI_TYPE_INTBUF,	"Autonomous Selection Enable" },
		{ ACPI_TYPE_BUFFER,	"Autonomous Activity Window Register" },
		{ ACPI_TYPE_BUFFER,	"Energy Performance Preference Register" },
		{ ACPI_TYPE_INTBUF,	"Reference Performance" }
	};

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	revision = obj->Package.Elements[1].Integer.Value;

	if (revision == 1) {		// acpi 5.0
		/* Something is really wrong if we don't have any elements in _CPC */
		if (method_package_count_equal(fw, name, "_CPC", obj, 17) != FWTS_OK)
			return;

		/* For now, just check types */
		if (method_package_elements_type(fw, name, "_CPC", obj, elementsv1, 17) != FWTS_OK)
			return;
	} else if (revision == 2) {	// acpi 5.1 and later
		/* Something is really wrong if we don't have any elements in _CPC */
		if (method_package_count_equal(fw, name, "_CPC", obj, 21) != FWTS_OK)
			return;

		/* For now, just check types */
		if (method_package_elements_type(fw, name, "_CPC", obj, elementsv2, 21) != FWTS_OK)
			return;
	} else {
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"Method_CPCBadRevision",
			"_CPC's _REV is incorrect, "
			"expecting 1 or 2, got 0x%" PRIx8 , revision);

		return;
	}

	method_passed_sane(fw, name, "package");
}

static int method_test_CPC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_CPC", NULL,
		0, method_test_CPC_return, NULL);
}

static void method_test_CSD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Something is really wrong if we don't have any elements in _CSD */
	if (method_package_count_min(fw, name, "_CSD", obj, 1) != FWTS_OK)
		return;

	/* Could be one or more packages */
	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		uint32_t j;
		bool elements_ok = true;

		if (method_package_elements_all_type(fw, name, "_CSD",
			obj, ACPI_TYPE_PACKAGE) != FWTS_OK) {
			failed = true;
			continue;	/* Skip processing sub-package */
		}

		pkg = &obj->Package.Elements[i];
		/*
		 *  Currently we expect a package of 6 integers.
		 */
		if (pkg->Package.Count != 6) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_CSDSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 5 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		for (j = 0; j < 6; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_CSDSubPackageElementCount",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"an integer.",
					name, i, j);
				elements_ok = false;
			}
		}

		if (!elements_ok) {
			failed = true;
			continue;
		}

		/* Element 0 must equal the number elements in the package */
		if (pkg->Package.Elements[0].Integer.Value != pkg->Package.Count) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_CSDSubPackageElement0",
				"%s sub-package %d element 0 (NumEntries) "
				"was expected to have value 0x%" PRIx64 ".",
				name, i,
				(uint64_t)pkg->Package.Elements[0].Integer.Value);
			failed = true;
		}
		/* Element 1 should contain zero */
		if (pkg->Package.Elements[1].Integer.Value != 0) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_CSDSubPackageElement1",
				"%s sub-package %d element 1 (Revision) "
				"was expected to have value 1, instead it "
				"was 0x%" PRIx64 ".",
				name, i,
				(uint64_t)pkg->Package.Elements[1].Integer.Value);
			failed = true;
		}
		/* Element 3 should contain 0xfc..0xfe */
		if ((pkg->Package.Elements[3].Integer.Value != 0xfc) &&
		    (pkg->Package.Elements[3].Integer.Value != 0xfd) &&
		    (pkg->Package.Elements[3].Integer.Value != 0xfe)) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_CSDSubPackageElement1",
				"%s sub-package %d element 3 (CoordType) "
				"was expected to have value 0xfc (SW_ALL), "
				"0xfd (SW_ANY) or 0xfe (HW_ALL), instead it "
				"was 0x%" PRIx64 ".",
				name, i,
				(uint64_t)pkg->Package.Elements[3].Integer.Value);
			failed = true;
		}
		/* Element 4 number of processors, skip check */
		/* Element 5 index, check */
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_CSD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_CSD", NULL, 0, method_test_CSD_return, NULL);
}

static void method_test_CST_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i, j;
	bool failed = false;
	bool *cst_elements_ok;
	bool an_element_ok = false;

	typedef struct {
		const uint32_t	type;
		const char 	*name;
	} cstate_info;

	static const cstate_info cstate_types[] = {
		{ ACPI_TYPE_BUFFER,	"buffer" },
		{ ACPI_TYPE_INTEGER,	"integer" },
		{ ACPI_TYPE_INTEGER,	"integer" },
		{ ACPI_TYPE_INTEGER,	"integer" },
	};

	FWTS_UNUSED(private);

	if (obj == NULL)
		return;

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* _CST has at least two elements */
	if (method_package_count_min(fw, name, "_CST", obj, 2) != FWTS_OK)
		return;

	/* Element 1 must be an integer */
	if (obj->Package.Elements[0].Type != ACPI_TYPE_INTEGER) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_CSTElement0NotInteger",
			"%s should return package with element zero being an integer "
			"count of the number of C state sub-packages.", name);
		return;
	}

	if (obj->Package.Elements[0].Integer.Value != obj->Package.Count - 1) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_CSTElement0CountMismatch",
			"%s should return package with element zero containing "
			"the number of C state sub-elements.  However, _CST has "
			"%" PRIu32 " returned C state sub-elements yet _CST "
			"reports it has %" PRIu64 " C states.",
			name, obj->Package.Count - 1,
			(uint64_t)obj->Package.Elements[0].Integer.Value);
		return;
	}

	cst_elements_ok = calloc(obj->Package.Count, sizeof(bool));
	if (cst_elements_ok == NULL) {
		fwts_log_error(fw, "Cannot allocate an array. Test aborted.");
		return;
	}

	/* Could be one or more packages */
	for (i = 1; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;

		cst_elements_ok[i] = true;
		if (obj->Package.Elements[i].Type != ACPI_TYPE_PACKAGE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_CSTElementType",
				"%s package element %" PRIu32 " was not a package.",
				name, i);
			cst_elements_ok[i] = false;
			failed = true;
			continue;	/* Skip processing sub-package */
		}

		pkg = &obj->Package.Elements[i];

		if (pkg->Package.Count != 4) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_CSTElementPackageCountInvalid",
				"%s package element %" PRIu32 " should have "
				"4 elements, instead it had %" PRIu32 ".",
				name, i, pkg->Package.Count);
			cst_elements_ok[i] = false;
			failed = true;
			continue;
		}

		for (j = 0; j < 4; j++) {
			if (pkg->Package.Elements[j].Type != cstate_types[j].type) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_CSTCStatePackageElementInvalidType",
					"%s C-State package %" PRIu32 " element %" PRIu32
					" was not a %s.",
					name, i, j, cstate_types[j].name);
				cst_elements_ok[i] = false;
				failed = true;
			}
		}

		/* Some very simple sanity checks on Register Resource Buffer */
		if (pkg->Package.Elements[0].Type == ACPI_TYPE_BUFFER) {
			if (pkg->Package.Elements[0].Buffer.Pointer == NULL) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_CSTCStateRegisterResourceBufferNull",
					"%s C-State package %" PRIu32 " has a NULL "
					"Register Resource Buffer", name, i);
				failed = true;
			} else {
				uint8_t *data = (uint8_t *)pkg->Package.Elements[0].Buffer.Pointer;
				size_t  length = (size_t)pkg->Package.Elements[0].Buffer.Length;

				if (data[0] != 0x82) {
					fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_CSTCStateResourceBufferWrongType",
					"%s C-State package %" PRIu32 " has a Resource "
					"type 0x%2.2" PRIx8 ", however, was expecting a Register "
					"Resource type 0x82.", name, i, data[0]);
					failed = true;
				}
				else {
					bool passed = true;
					method_test_CRS_large_size(fw, name, "_CST", data, length, 12, 12, &passed);
					if (!passed)
						failed = true;
				}
			}
		}

		if (cst_elements_ok[i])
			an_element_ok = true;
	}

	/*  Now dump out per CPU C-state information */
	if (an_element_ok) {
		fwts_log_info_verbatim(fw, "%s values:", name);
		fwts_log_info_verbatim(fw, "#   C-State   Latency     Power");
		fwts_log_info_verbatim(fw, "                (us)      (mW)");
		for (i = 1; i < obj->Package.Count; i++){
			if (cst_elements_ok[i]) {
				ACPI_OBJECT *pkg = &obj->Package.Elements[i];
				fwts_log_info_verbatim(fw,
					"%2" PRIu32 "     C%" PRIu64 "     %6" PRIu64 "    %6" PRIu64,
					i,
					(uint64_t)pkg->Package.Elements[1].Integer.Value,
					(uint64_t)pkg->Package.Elements[2].Integer.Value,
					(uint64_t)pkg->Package.Elements[3].Integer.Value);
			} else {
				fwts_log_info_verbatim(fw,
					"%2" PRIu32 "     --      -----     -----", i);
			}
		}
	}

	free(cst_elements_ok);

	if (!failed)
		method_passed_sane(fw, name, "values");
}

static int method_test_CST(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_CST", NULL, 0, method_test_CST_return, NULL);
}

static void method_test_PCT_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Something is really wrong if we don't have any elements in _PCT */
	if (method_package_count_min(fw, name, "_PCT", obj, 2) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_PCT",
		obj, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_PCT(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_PCT", NULL,
		0, method_test_PCT_return, NULL);
}

static void method_test_PSS_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;
	uint32_t max_freq = 0;
	uint32_t prev_power = 0;
	bool max_freq_valid = false;
	bool dump_elements = false;
	bool *element_ok;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Something is really wrong if we don't have any elements in _PSS */
	if (method_package_count_min(fw, name, "_PSS", obj, 1) != FWTS_OK)
		return;

	element_ok = calloc(obj->Package.Count, sizeof(bool));
	if (element_ok == NULL) {
		fwts_log_error(fw, "Cannot allocate an array. Test aborted.");
		return;
	}

	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pstate;

		if (obj->Package.Elements[i].Type != ACPI_TYPE_PACKAGE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PSSElementType",
				"%s package element %" PRIu32
				" was not a package.", name, i);
			failed = true;
			continue;	/* Skip processing sub-package */
		}

		pstate = &obj->Package.Elements[i];
		if (pstate->Package.Count != 6) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PSSSubPackageElementCount",
				"%s P-State sub-package %" PRIu32
				" was expected to "
				"have 6 elements, got %" PRIu32 " elements instead.",
				name, i, obj->Package.Count);
			failed = true;
			continue;	/* Skip processing sub-package */
		}

		/* Elements need to be all ACPI integer types */
		if ((pstate->Package.Elements[0].Type != ACPI_TYPE_INTEGER) ||
		    (pstate->Package.Elements[1].Type != ACPI_TYPE_INTEGER) ||
		    (pstate->Package.Elements[2].Type != ACPI_TYPE_INTEGER) ||
		    (pstate->Package.Elements[3].Type != ACPI_TYPE_INTEGER) ||
		    (pstate->Package.Elements[4].Type != ACPI_TYPE_INTEGER) ||
		    (pstate->Package.Elements[5].Type != ACPI_TYPE_INTEGER)) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PSSSubPackageElementType",
				"%s P-State sub-package %" PRIu32 " was expected to "
				"have 6 Integer elements but didn't", name, i);
			failed = true;
			continue;
		}

		/*
	 	 *  Parses OK, so this element can be dumped out
		 */
		element_ok[i] = true;
		dump_elements = true;

		/*
		 * Collect maximum frequency.  The sub-packages are sorted in
		 * descending power dissipation order, so one would assume that
		 * the highest frequency is first.  However, it is not clear
		 * from the ACPI spec that this is necessarily an assumption we
		 * should make, so instead we should probably scan through all
		 * the valid sub-packages and find the highest frequency.
		 */
		if (max_freq < pstate->Package.Elements[0].Integer.Value) {
			max_freq = pstate->Package.Elements[0].Integer.Value;
			max_freq_valid = true;
		}

		/* Sanity check descending power dissipation levels */
		if ((i > 0) && (prev_power != 0) &&
		    (pstate->Package.Elements[1].Integer.Value > prev_power)) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PSSSubPackagePowerNotDecending",
				"%s P-State sub-package %" PRIu32 " has a larger "
				"power dissipation setting than the previous "
				"sub-package.", name, i);
			fwts_advice(fw,
				"_PSS P-States must be ordered in decending "
				"order of power dissipation, so that the "
				"zero'th entry has the highest power "
				"dissipation level and the Nth has the "
				"lowest.");
			failed = true;
		}
		prev_power = pstate->Package.Elements[1].Integer.Value;
	}

	/*
	 *  If we have some valid data then dump it out, it is useful to see
	 */
	if (dump_elements) {
		fwts_log_info_verbatim(fw, "%s values:", name);
		fwts_log_info_verbatim(fw, "P-State  Freq     Power  Latency   Bus Master");
		fwts_log_info_verbatim(fw, "         (MHz)    (mW)    (us)    Latency (us)");
		for (i = 0; i < obj->Package.Count; i++) {
			ACPI_OBJECT *pstate = &obj->Package.Elements[i];
			if (element_ok[i]) {
				fwts_log_info_verbatim(fw, " %3d   %7" PRIu64 " %8" PRIu64
					" %5" PRIu64 "     %5" PRIu64,
					i,
					(uint64_t)pstate->Package.Elements[0].Integer.Value,
					(uint64_t)pstate->Package.Elements[1].Integer.Value,
					(uint64_t)pstate->Package.Elements[2].Integer.Value,
					(uint64_t)pstate->Package.Elements[3].Integer.Value);
			} else {
				fwts_log_info_verbatim(fw,
					" %3d      ----    -----    --        -- (invalid)", i);
			}
		}
	}

	free(element_ok);

	/*
	 * Sanity check maximum frequency.  We could also check the DMI data
	 * for a BIOS date (but this can be wrong) or check the CPU identity
	 * (which requires adding in new CPU identity checks) to make a decision
	 * on when it is reasonable to assume a CPU is modern and hence clocked
	 * incorrectly.  For now, just flag up a low level error that the
	 * frequency looks rather low rather than try to be intelligent (and
	 * possibly make a mistake).  I'd rather flag up a few false positives
	 * on older machines than miss flagging up bad _PSS settings on new
	 * machines.
	 */
	if (max_freq_valid && max_freq < 1000) {
		fwts_failed(fw, LOG_LEVEL_LOW, "Method_PSSSubPackageLowFreq",
			"Maximum CPU frequency is %dHz and this is low for "
			"a modern processor. This may indicate the _PSS "
			"P-States are incorrect\n", max_freq);
		fwts_advice(fw,
			"The _PSS P-States are used by the Linux CPU frequency "
			"driver to set the CPU frequencies according to system "
			"load.  Sometimes the firmware sets these incorrectly "
			"and the machine runs at a sub-optimal speed.  One can "
			"view the firmware defined CPU frequencies via "
			"/sys/devices/system/cpu/cpu*/cpufreq/"
			"scaling_available_frequencies");
		failed = true;
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_PSS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL, "_PSS", NULL, 0, method_test_PSS_return, NULL);
}

static int method_test_PPC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PPC", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_PPE(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PPE", NULL, 0, method_test_integer_return, NULL);
}

static void method_test_PSD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_PSD", obj, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Could be one or more packages */
	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		uint32_t j;
		bool elements_ok = true;

		pkg = &obj->Package.Elements[i];
		if (pkg->Package.Count != 5) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_PSDSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 5 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		/* Elements in Sub-packages are integers */
		for (j = 0; j < 5; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_PSDBadSubPackageReturnType",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"an integer.",
					name, i, j);
				elements_ok = false;
			}
		}

		if (!elements_ok) {
			failed = true;
			continue;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_PSD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PSD", NULL, 0, method_test_PSD_return, NULL);
}

static int method_test_PDL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PDL", NULL, 0, method_test_integer_return, NULL);
}


static void method_test_PTC_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_PTC", obj, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_PTC", obj, 2) != FWTS_OK)
		return;

	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_RESOURCE *resource;
		ACPI_STATUS   status;
		ACPI_OBJECT *element_buf = &obj->Package.Elements[i];

		status = AcpiBufferToResource(element_buf->Buffer.Pointer, element_buf->Buffer.Length, &resource);
		if (ACPI_FAILURE(status)) {
			failed = true;
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"Method_PTCBadElement",
				"%s should contain only Resource Descriptors", name);
			continue;
		}

		if (resource->Type != ACPI_RESOURCE_TYPE_GENERIC_REGISTER) {
			failed = true;
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"Method_PTCBadElement",
				"%s should contain only Resource Type 16, got %" PRIu32 "\n",
					name, resource->Type);
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_PTC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PTC", NULL, 0, method_test_PTC_return, NULL);
}

static int method_test_TDL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TDL", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_TPC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TPC", NULL, 0, method_test_integer_return, NULL);
}

static void method_test_TSD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Something is really wrong if we don't have any elements in _TSD */
	if (method_package_count_min(fw, name, "_TSD", obj, 1) != FWTS_OK)
		return;

	/* Could be one or more packages */
	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		uint32_t j;
		bool elements_ok = true;

		if (obj->Package.Elements[i].Type != ACPI_TYPE_PACKAGE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TSDElementType",
				"%s package element %" PRIu32
				" was not a package.", name, i);
			failed = true;
			continue;	/* Skip processing sub-package */
		}

		pkg = &obj->Package.Elements[i];
		/*
		 *  Currently we expect a package of 5 integers.
		 */
		if (pkg->Package.Count != 5) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TSDSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 5 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		for (j = 0; j < 5; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_TSDSubPackageElementCount",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"an integer.", name, i, j);
				elements_ok = false;
			}
		}

		if (!elements_ok) {
			failed = true;
			continue;
		}

		/* Element 0 must equal the number elements in the package */
		if (pkg->Package.Elements[0].Integer.Value != pkg->Package.Count) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TSDSubPackageElement0",
				"%s sub-package %" PRIu32
				" element 0 (NumEntries) "
				"was expected to have value 0x%" PRIx64 ".",
				name, i,
				(uint64_t)pkg->Package.Elements[0].Integer.Value);
			failed = true;
		}
		/* Element 1 should contain zero */
		if (pkg->Package.Elements[1].Integer.Value != 0) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TSDSubPackageElement1",
				"%s sub-package %" PRIu32
				" element 1 (Revision) "
				"was expected to have value 1, instead it "
				"was 0x%" PRIx64 ".",
				name, i,
				(uint64_t)pkg->Package.Elements[1].Integer.Value);
			failed = true;
		}
		/* Element 3 should contain 0xfc..0xfe */
		if ((pkg->Package.Elements[3].Integer.Value != 0xfc) &&
		    (pkg->Package.Elements[3].Integer.Value != 0xfd) &&
		    (pkg->Package.Elements[3].Integer.Value != 0xfe)) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TSDSubPackageElement1",
				"%s sub-package %" PRIu32
				" element 3 (CoordType) "
				"was expected to have value 0xfc (SW_ALL), "
				"0xfd (SW_ANY) or 0xfe (HW_ALL), instead it "
				"was 0x%" PRIx64 ".",
				name, i,
				(uint64_t)pkg->Package.Elements[3].Integer.Value);
			failed = true;
		}
		/* Element 4 number of processors, skip check */
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_TSD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TSD", NULL, 0, method_test_TSD_return, NULL);
}

static void method_test_TSS_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;
	bool *tss_elements_ok;
	bool an_element_ok = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Something is really wrong if we don't have any elements in _TSS */
	if (method_package_count_min(fw, name, "_TSS", obj, 1) != FWTS_OK)
		return;

	tss_elements_ok = calloc(obj->Package.Count, sizeof(bool));
	if (tss_elements_ok == NULL) {
		fwts_log_error(fw, "Cannot allocate an array. Test aborted.");
		return;
	}

	/* Could be one or more packages */
	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		uint32_t j;

		tss_elements_ok[i] = true;

		if (obj->Package.Elements[i].Type != ACPI_TYPE_PACKAGE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TSSElementType",
				"%s package element %" PRIu32
				" was not a package.", name, i);
			tss_elements_ok[i] = false;
			failed = true;
			continue;	/* Skip processing sub-package */
		}

		pkg = &obj->Package.Elements[i];
		/*
		 *  We expect a package of 5 integers.
		 */
		if (pkg->Package.Count != 5) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TSSSubPackageElementCount",
				"%s sub-package %" PRIu32
				" was expected to have 5 elements, "
				"got %" PRIu32" elements instead.",
				name, i, pkg->Package.Count);
			tss_elements_ok[i] = false;
			failed = true;
			continue;	/* Skip processing sub-package */
		}

		for (j = 0; j < 5; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_TSSSubPackageElementCount",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"an integer.", name, i, j);
				tss_elements_ok[i] = false;
			}
		}
		if (!tss_elements_ok[i]) {
			failed = true;
			continue;
		}

		/* At least one element is OK, so remember that */
		an_element_ok = true;

		/* Element 0 must be 1..100 */
		if ((pkg->Package.Elements[0].Integer.Value < 1) ||
		    (pkg->Package.Elements[0].Integer.Value > 100)) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TSDSubPackageElement0",
				"%s sub-package %" PRIu32 " element 0"
				"was expected to have value 1..100, instead "
				"was %" PRIu64 ".",
				name, i,
				(uint64_t)pkg->Package.Elements[0].Integer.Value);
			failed = true;
		}
		/* Skip checking elements 1..4 */
	}

	/* Summary info */
	if (an_element_ok) {
		fwts_log_info_verbatim(fw, "%s values:", name);
		fwts_log_info_verbatim(fw, "T-State  CPU     Power   Latency  Control  Status");
		fwts_log_info_verbatim(fw, "         Freq    (mW)    (usecs)");
		for (i = 0; i < obj->Package.Count; i++) {
			if (tss_elements_ok[i]) {
				ACPI_OBJECT *pkg = &obj->Package.Elements[i];

				fwts_log_info_verbatim(fw,
					"  %3d    %3" PRIu64 "%%  %7" PRIu64 "  %7" PRIu64
					"      %2.2" PRIx64 "      %2.2" PRIx64, i,
					(uint64_t)pkg->Package.Elements[0].Integer.Value,
					(uint64_t)pkg->Package.Elements[1].Integer.Value,
					(uint64_t)pkg->Package.Elements[2].Integer.Value,
					(uint64_t)pkg->Package.Elements[3].Integer.Value,
					(uint64_t)pkg->Package.Elements[4].Integer.Value);
			} else {
				fwts_log_info_verbatim(fw,
					"  %3d    ----    -----    -----      --      -- (invalid)", i);
			}
		}
	}
	free(tss_elements_ok);

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_TSS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TSS", NULL, 0, method_test_TSS_return, NULL);
}

/*
 * Section 8.4.4 Lower Power Idle States
*/

static void method_test_LPI_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i, j;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_min(fw, name, "_LPI", obj, 3) != FWTS_OK)
		return;

	/* first 3 elements are integers, and rests are packages */
	for (i = 0; i < obj->Package.Count; i++) {
		if (i < 3) {
			if (obj->Package.Elements[i].Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_HIGH,
					"Method_LPIBadElementType",
					"%s element %" PRIu32 " is not an integer.", name, i);
				failed = true;
				continue;
			}

			if (i == 0) {
				if (obj->Package.Elements[i].Integer.Value != 0) {
					fwts_failed(fw, LOG_LEVEL_HIGH,
						"Method_LPIBadRevision",
						"%s: Expected Revision to be 0, "
						"got 0x%4.4" PRIx64 ".", name,
						(uint64_t)obj->Package.Elements[i].Integer.Value);
					failed = true;
				}
			} else if (i == 2) {
				if (obj->Package.Elements[i].Integer.Value != obj->Package.Count - 3) {
					fwts_failed(fw, LOG_LEVEL_HIGH,
					"Method_LPIBadCount",
					"%s Count reports %" PRIu32 ", but there are %" PRIu32 " sub-packages.",
					name, (uint32_t) obj->Package.Elements[i].Integer.Value,
					obj->Package.Count - 3);
					failed = true;
				}
			}
		} else {
			ACPI_OBJECT *pkg;
			if (obj->Package.Elements[i].Type != ACPI_TYPE_PACKAGE) {
				fwts_failed(fw, LOG_LEVEL_HIGH,
					"Method_LPIBadElementType",
					"%s element %" PRIu32 " is not a package.", name, i);
				failed = true;
				continue;
			}

			pkg = &obj->Package.Elements[i];
			for (j = 0; j < pkg->Package.Count; j++) {
				switch (j) {
				case 0 ... 5:
					if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER) {
						fwts_failed(fw, LOG_LEVEL_HIGH,
							"Method_LPIBadESublementType",
							"%s sub-package %" PRIu32 " element %" PRIu32 " is not "
							"an integer.", name, i, j);
						failed = true;
					}
					break;
				case 6:
					if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER &&
					    pkg->Package.Elements[j].Type != ACPI_TYPE_BUFFER) {
						fwts_failed(fw, LOG_LEVEL_HIGH,
							"Method_LPIBadESublementType",
							"%s sub-package %" PRIu32 " element %" PRIu32 " is not "
							"a buffer or an integer.", name, i, j);
						failed = true;
					}
					break;
				case 7 ... 8:
					if (pkg->Package.Elements[j].Type != ACPI_TYPE_BUFFER) {
						fwts_failed(fw, LOG_LEVEL_HIGH,
							"Method_LPIBadESublementType",
							"%s sub-package %" PRIu32 " element %" PRIu32 " is not "
							"a buffer.", name, i, j);
						failed = true;
					}
					break;
				case 9:
					if (pkg->Package.Elements[j].Type != ACPI_TYPE_STRING) {
						fwts_failed(fw, LOG_LEVEL_HIGH,
							"Method_LPIBadESublementType",
							"%s sub-package %" PRIu32 " element %" PRIu32 " is not "
							"a string.", name, i, j);
						failed = true;
					}
					break;
				default:
					fwts_failed(fw, LOG_LEVEL_HIGH,
						"Method_LPIBadESublement",
						"%s sub-package %" PRIu32 " element %" PRIu32 " should have "
						"9 elements, got .", name, i, j+1);
					failed = true;
					break;
				}
			}
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_LPI(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_LPI", NULL, 0, method_test_LPI_return, NULL);
}

static void method_test_RDI_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i, j;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* First element is Revision */
	if (obj->Package.Elements[0].Integer.Value != 0) {
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"Method_RDIBadID",
			"%s: Expected Revision to be 0, "
			"got 0x%4.4" PRIx64 ".", name,
			(uint64_t)obj->Package.Elements[0].Integer.Value);
		failed = true;
	}

	/* The rest of elements are packages with references */
	for (i = 1; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		pkg = &obj->Package.Elements[i];

		if (pkg->Type != ACPI_TYPE_PACKAGE) {
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"Method_RDIBadElementType",
				"%s element %" PRIu32 " is not a package.", name, i);
			failed = true;
			continue;
		}

		for (j = 0; j < pkg->Package.Count; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_LOCAL_REFERENCE) {
				fwts_failed(fw, LOG_LEVEL_HIGH,
					"Method_RDIBadESublementType",
					"%s sub-package %" PRIu32 " element %" PRIu32 " is not "
					"a Reference.", name, i, j);
				failed = true;
			}
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_RDI(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_RDI", NULL, 0, method_test_RDI_return, NULL);
}

/*
 * Section 8.5 Processor Aggregator Device
 */

static void method_test_PUR_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	static fwts_package_element elements[] = {
		{ ACPI_TYPE_INTEGER,	"RevisionID" },
		{ ACPI_TYPE_INTEGER,	"NumProcessors" },
	};

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_PUR", obj, 2) != FWTS_OK)
		return;

	if (method_package_elements_type(fw, name, "_PUR", obj, elements, 2) != FWTS_OK)
		return;

	/* RevisionID */
	if (obj->Package.Elements[0].Integer.Value != 1) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PURBadID",
			"%s: Expected RevisionID to be 1, "
			"got 0x%8.8" PRIx64 ".", name,
			(uint64_t)obj->Package.Elements[0].Integer.Value);
		return;
	}

	method_passed_sane(fw, name, "package");
}

static int method_test_PUR(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PUR", NULL, 0, method_test_PUR_return, NULL);
}

/*
 * Section 9.1 System Indicators
 */

static int method_test_SST(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	int ret, i;

	arg[0].Type = ACPI_TYPE_INTEGER;
	for (i = 0; i <= 4; i++) {
		arg[0].Integer.Value = i;
		ret = method_evaluate_method(fw, METHOD_OPTIONAL,
			"_SST", arg, 1, method_test_NULL_return, NULL);

		if (ret != FWTS_OK)
			break;
	}
	return ret;
}

static int method_test_MSG(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 0;

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_MSG", arg, 1, method_test_NULL_return, NULL);
}

/*
 * Section 9.2 Ambient Light Sensor Device
 */
method_test_integer(_ALC, METHOD_OPTIONAL)
method_test_integer(_ALI, METHOD_OPTIONAL)
method_test_integer(_ALT, METHOD_OPTIONAL)

static void method_test_ALR_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;
	uint32_t adjustment = 0, illuminance = 0;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Could be one or more sub-packages */
	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;

		pkg = &obj->Package.Elements[i];
		if (pkg->Package.Count != 2) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_ALRBadSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 2 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
		} else {
			/* elements should be listed in monotonically increasing order */
			if (pkg->Package.Elements[0].Type != ACPI_TYPE_INTEGER ||
			    adjustment > pkg->Package.Elements[0].Integer.Value) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_ALRBadSubPackageReturnType",
					"%s sub-package %" PRIu32
					" element 0 is an invalid integer.",
					name, i);
				failed = true;
			}

			if (pkg->Package.Elements[1].Type != ACPI_TYPE_INTEGER ||
			    illuminance > pkg->Package.Elements[1].Integer.Value) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_ALRBadSubPackageReturnType",
					"%s sub-package %" PRIu32
					" element 1 is an invalid integer.",
					name, i);
				failed = true;
			}
			adjustment = pkg->Package.Elements[0].Integer.Value;
			illuminance = pkg->Package.Elements[1].Integer.Value;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_ALR(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_ALR", NULL, 0, method_test_ALR_return, NULL);
}

static int method_test_ALP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_ALP", NULL, 0, method_test_polling_return, "_ALP");
}


/*
 * Section 9.4 Lid control
 */
static void method_test_LID_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) == FWTS_OK)
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_LID(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MOBILE,
		"_LID", NULL, 0, method_test_LID_return, NULL);
}


/*
 * Section 9.8 ATA Controllers
 */

static void method_test_GTF_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;

	if (obj->Buffer.Length % 7)
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_GTFBadBufferSize",
			"%s should return a buffer with size of multiple of 7.",
			name);
	else
		method_passed_sane(fw, name, "buffer");
}

static int method_test_GTF(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GTF", NULL, 0, method_test_GTF_return, NULL);
}

static void method_test_GTM_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;

	if (obj->Buffer.Length != (5 * sizeof(uint32_t)))
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_GTMBadBufferSize",
			"%s should return a buffer with size of 20.",
			name);
	else
		method_passed_sane(fw, name, "buffer");
}

static int method_test_GTM(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GTM", NULL, 0, method_test_GTM_return, NULL);
}

/*
 * Section 9.12 Memory Devices
 */

static void method_test_MBM_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	static fwts_package_element elements[] = {
		{ ACPI_TYPE_INTEGER,	"Revision" },
		{ ACPI_TYPE_INTEGER,	"Window Size" },
		{ ACPI_TYPE_INTEGER,	"Sampling Interval" },
		{ ACPI_TYPE_INTEGER,	"Maximum Bandwidth" },
		{ ACPI_TYPE_INTEGER,	"Average Bandwidth" },
		{ ACPI_TYPE_INTEGER,	"Low Bandwidth" },
		{ ACPI_TYPE_INTEGER,	"Low Notification Threshold" },
		{ ACPI_TYPE_INTEGER,	"High Notification Threshold" },
	};

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_MBM", obj, 8) != FWTS_OK)
		return;

	/* For now, just check types */
	if (method_package_elements_type(fw, name, "_MBM", obj, elements, 8) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_MBM(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_MBM", NULL, 0, method_test_MBM_return, NULL);
}

/*
 * Section 9.13 USB Port Capabilities
 */

static void method_test_UPC_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i, connector_type;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_UPC", obj, 4) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_UPC", obj, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	connector_type = obj->Package.Elements[1].Integer.Value;
	if (connector_type  > 0x0a && connector_type < 0xFF) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_UPCBadReturnType",
			"%s element 1 returned reserved value.", name);
		return;
	}

	for (i = 2; i < 4; i++) {
		if (obj->Package.Elements[i].Integer.Value != 0) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_UPCBadReturnType",
				"%s element %" PRIu32 " is not zero.", name, i);
			return;
		}
	}

	method_passed_sane(fw, name, "package");
}

static int method_test_UPC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_UPC", NULL, 0, method_test_UPC_return, NULL);
}

/*
 * Section 9.16 User Presence Detection Device
 */

static int method_test_UPD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_UPD", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_UPP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_UPP", NULL, 0, method_test_integer_return, NULL);
}

/*
 * Section 9.18 Wake Alarm Device
 */
static void method_test_GCP_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if (obj->Integer.Value & ~0x1f)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_GCPReturn",
			"%s returned %" PRId64 ", should be between 0 and 31, "
			"one or more of the reserved bits 5..31 seem "
			"to be set.",
			name, (uint64_t)obj->Integer.Value);
	else
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_GCP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GCP", NULL, 0, method_test_GCP_return, "_GCP");
}

static void method_test_GRT_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_BUFFER) != FWTS_OK)
		return;

	if (obj->Buffer.Length != 16) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_GRTBadBufferSize",
			"%s should return a buffer of 16 bytes, but "
			"instead just returned %" PRIu32,
			name, obj->Buffer.Length);
		return;
	}

	/*
	 * Should sanity check this, but we can't read the
	 * the data in this emulated mode, so ignore
	 */
	method_passed_sane(fw, name, "buffer");
}

static int method_test_GRT(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GRT", NULL, 0, method_test_GRT_return, NULL);
}

static void method_test_GWS_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if (obj->Integer.Value & ~0x3)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_GWSReturn",
			"%s returned %" PRIu64 ", should be between 0 and 3, "
			"one or more of the reserved bits 2..31 seem "
			"to be set.",
			name, (uint64_t)obj->Integer.Value);
	else
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_GWS(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;	/* DC timer */

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GWS", arg, 1, method_test_GWS_return, "_GWS");
}

static void method_test_CWS_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if (obj->Integer.Value != 0 && obj->Integer.Value != 1)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_CWSInvalidInteger",
			"%s returned %" PRIu64 ", should be 0 or 1.",
			name, (uint64_t)obj->Integer.Value);
	else
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_CWS(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	int i, ret;
	arg[0].Type = ACPI_TYPE_INTEGER;

	for (i = 0; i < 2; i++) {
		arg[0].Integer.Value = i;
		ret = method_evaluate_method(fw, METHOD_OPTIONAL,
			"_CWS", arg, 1, method_test_CWS_return, NULL);

		if (ret != FWTS_OK)
			break;
	}
	return ret;
}

static void method_test_SRT_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if (obj->Integer.Value & ~0x1)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_SRTReturn",
			"%s returned %" PRId64 ", should be between 0 and 1, "
			"one or more of the reserved bits 1..31 seem "
			"to be set.",
			name, (uint64_t)obj->Integer.Value);
	else
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_SRT(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_SRT", NULL, 0, method_test_SRT_return, NULL);
}

static int method_test_STP(fwts_framework *fw)
{
	ACPI_OBJECT arg[2];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;	/* DC timer */
	arg[1].Type = ACPI_TYPE_INTEGER;
	arg[1].Integer.Value = 0;	/* wake up instantly */

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_STP", arg, 2, method_test_passed_failed_return, "_STP");
}

static int method_test_STV(fwts_framework *fw)
{
	ACPI_OBJECT arg[2];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;	/* DC timer */
	arg[1].Type = ACPI_TYPE_INTEGER;
	arg[1].Integer.Value = 100;	/* timer value */

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_STV", arg, 2, method_test_passed_failed_return, "_STV");
}

static int method_test_TIP(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;	/* DC timer */

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TIP", arg, 1, method_test_integer_return, NULL);
}

static int method_test_TIV(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;	/* DC timer */

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TIV", arg, 1, method_test_integer_return, NULL);
}


/*
 * Section 10.1 Smart Battery
 */
static void method_test_SBS_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	static char *sbs_info[] = {
		"Maximum 1 Smart Battery, system manager/selector not present",
		"Maximum 1 Smart Battery, system manager/selector present",
		"Maximum 2 Smart Batteries, system manager/selector present",
		"Maximum 3 Smart Batteries, system manager/selector present",
		"Maximum 4 Smart Batteries, system manager/selector present"
	};

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	switch (obj->Integer.Value) {
	case 0 ... 4:
		fwts_passed(fw, "%s correctly returned value %" PRIu64 " %s",
			name, (uint64_t)obj->Integer.Value,
			sbs_info[obj->Integer.Value]);
		break;
	default:
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_SBSReturn",
			"%s returned %" PRIu64 ", should be between 0 and 4.",
			name, (uint64_t)obj->Integer.Value);
		fwts_advice(fw,
			"Smart Battery %s is incorrectly informing "
			"the OS about the smart battery "
			"configuration. This is a bug and needs to be "
			"fixed.", name);
		break;
	}
}

static int method_test_SBS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MOBILE,
		"_SBS", NULL, 0, method_test_SBS_return, NULL);
}


/*
 * Section 10.2 Battery Control Methods
 */
static int method_test_BCT(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 50;	/* 50% */

	/*
	 * For now, just check that we get some integer back, values
	 * can be 0x00000000, 0x00000001-0xfffffffe and 0xffffffff,
	 * so anything is valid as long as it is an integer
	 */
	return method_evaluate_method(fw, METHOD_MOBILE,
		"_BCT", arg, 1, method_test_integer_return, NULL);
}

static void method_test_BIF_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	bool failed = false;

	static fwts_package_element elements[] = {
		{ ACPI_TYPE_INTEGER,	"Power Unit" },
		{ ACPI_TYPE_INTEGER,	"Design Capacity" },
		{ ACPI_TYPE_INTEGER,	"Last Full Charge Capacity" },
		{ ACPI_TYPE_INTEGER,	"Battery Technology" },
		{ ACPI_TYPE_INTEGER,	"Design Voltage" },
		{ ACPI_TYPE_INTEGER,	"Design Capacity of Warning" },
		{ ACPI_TYPE_INTEGER,	"Design Capactty of Low" },
		{ ACPI_TYPE_INTEGER,	"Battery Capacity Granularity 1" },
		{ ACPI_TYPE_INTEGER,	"Battery Capacity Granularity 2" },
		{ ACPI_TYPE_STRING,	"Model Number" },
		{ ACPI_TYPE_STRING,	"Serial Number" },
		{ ACPI_TYPE_STRING,	"Battery Type" },
		{ ACPI_TYPE_STRING,	"OEM Information" }
	};

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_BIF", obj, 13) != FWTS_OK)
		return;

	if (method_package_elements_type(fw, name, "_BIF", obj, elements, 13) != FWTS_OK)
		return;

	/* Sanity check each field */
	/* Power Unit */
	if (obj->Package.Elements[0].Integer.Value > 0x00000002) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_BIFBadUnits",
			"%s: Expected Power Unit (Element 0) to be "
			"0 (mWh) or 1 (mAh), got 0x%8.8" PRIx64 ".",
			name, (uint64_t)obj->Package.Elements[0].Integer.Value);
		failed = true;
	}
#ifdef FWTS_METHOD_PEDANDTIC
	/*
	 * Since this information may be evaluated by communicating with
	 * the EC we skip these checks as we can't do this from userspace
	 */
	/* Design Capacity */
	if (obj->Package.Elements[1].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIFBadCapacity",
			"%s: Design Capacity (Element 1) is "
			"unknown: 0x%8.8" PRIx64 ".",
			name, obj->Package.Elements[1].Integer.Value);
		failed = true;
	}
	/* Last Full Charge Capacity */
	if (obj->Package.Elements[2].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIFChargeCapacity",
			"%s: Last Full Charge Capacity (Element 2) "
			"is unknown: 0x%8.8" PRIx64 ".",
			name, obj->Package.Elements[2].Integer.Value);
		failed = true;
	}
#endif
	/* Battery Technology */
	if (obj->Package.Elements[3].Integer.Value > 0x00000002) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_BIFBatTechUnit",
			"%s: Expected Battery Technology Unit "
			"(Element 3) to be 0 (Primary) or 1 "
			"(Secondary), got 0x%8.8" PRIx64 ".",
			name, (uint64_t)obj->Package.Elements[3].Integer.Value);
		failed = true;
	}
#ifdef FWTS_METHOD_PEDANDTIC
	/*
 	 * Since this information may be evaluated by communicating with
	 * the EC we skip these checks as we can't do this from userspace
	 */
	/* Design Voltage */
	if (obj->Package.Elements[4].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIFDesignVoltage",
			"%s: Design Voltage (Element 4) is "
			"unknown: 0x%8.8" PRIx64 ".",
			name, obj->Package.Elements[4].Integer.Value);
		failed = true;
	}
	/* Design capacity warning */
	if (obj->Package.Elements[5].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIFDesignCapacityE5",
			"%s: Design Capacity Warning (Element 5) "
			"is unknown: 0x%8.8" PRIx64 ".",
			name, obj->Package.Elements[5].Integer.Value);
		failed = true;
	}
	/* Design capacity low */
	if (obj->Package.Elements[6].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIFDesignCapacityE6",
			"%s: Design Capacity Warning (Element 6) "
			"is unknown: 0x%8.8" PRIx64 ".",
			name, obj->Package.Elements[6].Integer.Value);
		failed = true;
	}
#endif
	if (failed)
		fwts_advice(fw,
			"Battery %s package contains errors. It is "
			"worth running the firmware test suite "
			"interactive 'battery' test to see if this "
			"is problematic.  This is a bug an needs to "
			"be fixed.", name);
	else
		method_passed_sane(fw, name, "package");
}

static int method_test_BIF(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MOBILE,
		"_BIF", NULL, 0, method_test_BIF_return, NULL);
}

static void method_test_BIX_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	bool failed = false;

	static fwts_package_element elements[] = {
		{ ACPI_TYPE_INTEGER,	"Revision" },
		{ ACPI_TYPE_INTEGER,	"Power Unit" },
		{ ACPI_TYPE_INTEGER,	"Design Capacity" },
		{ ACPI_TYPE_INTEGER,	"Last Full Charge Capacity" },
		{ ACPI_TYPE_INTEGER,	"Battery Technology" },
		{ ACPI_TYPE_INTEGER,	"Design Voltage" },
		{ ACPI_TYPE_INTEGER,	"Design Capacity of Warning" },
		{ ACPI_TYPE_INTEGER,	"Design Capactty of Low" },
		{ ACPI_TYPE_INTEGER,	"Cycle Count" },
		{ ACPI_TYPE_INTEGER,	"Measurement Accuracy" },
		{ ACPI_TYPE_INTEGER,	"Max Sampling Time" },
		{ ACPI_TYPE_INTEGER,	"Min Sampling Time" },
		{ ACPI_TYPE_INTEGER,	"Max Averaging Interval" },
		{ ACPI_TYPE_INTEGER,	"Min Averaging Interval" },
		{ ACPI_TYPE_INTEGER,	"Battery Capacity Granularity 1" },
		{ ACPI_TYPE_INTEGER,	"Battery Capacity Granularity 2" },
		{ ACPI_TYPE_STRING,	"Model Number" },
		{ ACPI_TYPE_STRING,	"Serial Number" },
		{ ACPI_TYPE_STRING,	"Battery Type" },
		{ ACPI_TYPE_STRING,	"OEM Information" }
	};

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_BIX", obj, 20) != FWTS_OK)
		return;

	if (method_package_elements_type(fw, name, "_BIX", obj, elements, 20) != FWTS_OK)
		return;

	/* Sanity check each field */
	/* Power Unit */
	if (obj->Package.Elements[1].Integer.Value > 0x00000002) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_BIXPowerUnit",
			"%s: Expected %s (Element 1) to be "
			"0 (mWh) or 1 (mAh), got 0x%8.8" PRIx64 ".",
			name, elements[1].name,
			(uint64_t)obj->Package.Elements[1].Integer.Value);
		failed = true;
	}
#ifdef FWTS_METHOD_PEDANDTIC
	/*
	 * Since this information may be evaluated by communicating with
	 * the EC we skip these checks as we can't do this from userspace
 	 */
	/* Design Capacity */
	if (obj->Package.Elements[2].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIXDesignCapacity",
			"%s: %s (Element 2) is "
			"unknown: 0x%8.8" PRIx64 ".",
			name, elements[2].name,
			obj->Package.Elements[2].Integer.Value);
		failed = true;
	}
	/* Last Full Charge Capacity */
	if (obj->Package.Elements[3].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIXFullChargeCapacity",
			"%s: %s (Element 3) "
			"is unknown: 0x%8.8" PRIx64 ".",
			name, elements[3].name,
			obj->Package.Elements[3].Integer.Value);
		failed = true;
	}
#endif
	/* Battery Technology */
	if (obj->Package.Elements[4].Integer.Value > 0x00000002) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_BIXBatteryTechUnit",
			"%s: %s "
			"(Element 4) to be 0 (Primary) or 1 "
			"(Secondary), got 0x%8.8" PRIx64 ".",
			name, elements[4].name,
			(uint64_t)obj->Package.Elements[4].Integer.Value);
		failed = true;
	}
#ifdef FWTS_METHOD_PEDANDTIC
	/*
	 * Since this information may be evaluated by communicating with
	 * the EC we skip these checks as we can't do this from userspace
 	 */
	/* Design Voltage */
	if (obj->Package.Elements[5].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIXDesignVoltage",
			"%s: %s (Element 5) is unknown: "
			"0x%8.8" PRIx64 ".",
			name, elements[5].name,
			obj->Package.Elements[5].Integer.Value);
		failed = true;
	}
	/* Design capacity warning */
	if (obj->Package.Elements[6].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIXDesignCapacityE6",
			"%s: %s (Element 6) "
			"is unknown: 0x%8.8" PRIx64 ".",
			name, elements[6].name,
			obj->Package.Elements[6].Integer.Value);
		failed = true;
	}
	/* Design capacity low */
	if (obj->Package.Elements[7].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW,
			"Method_BIXDesignCapacityE7",
			"%s: %s (Element 7) "
			"is unknown: 0x%8.8" PRIx64 ".",
			name, elements[7].name,
			obj->Package.Elements[7].Integer.Value);
		failed = true;
	}
	/* Cycle Count */
	if (obj->Package.Elements[8].Integer.Value > 0x7fffffff) {
		fwts_failed(fw, LOG_LEVEL_LOW, "Method_BIXCyleCount",
			"%s: %s (Element 8) is unknown: "
			"0x%8.8" PRIx64 ".", Elements[8].name,
			name, obj->Package.Elements[8].Integer.Value);
		failed = true;
	}
#endif
	if (failed)
		fwts_advice(fw,
			"Battery %s package contains errors. It is "
			"worth running the firmware test suite "
			"interactive 'battery' test to see if this "
			"is problematic.  This is a bug an needs to "
			"be fixed.", name);
	else
		method_passed_sane(fw, name, "package");
}

static int method_test_BIX(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MOBILE,
		"_BIX", NULL, 0, method_test_BIX_return, NULL);
}

static int method_test_BMA(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;

	return method_evaluate_method(fw, METHOD_MOBILE,
		"_BMA", arg, 1, method_test_integer_return, NULL);
}

static int method_test_BMS(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 1;

	return method_evaluate_method(fw, METHOD_MOBILE,
		"_BMS", arg, 1, method_test_integer_return, NULL);
}

static void method_test_BST_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_BST", obj, 4) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_BST", obj, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	/* Sanity check each field */
	/* Battery State */
	if ((obj->Package.Elements[0].Integer.Value) > 7) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_BSTBadState",
			"%s: Expected Battery State (Element 0) to "
			"be 0..7, got 0x%8.8" PRIx64 ".",
			name, (uint64_t)obj->Package.Elements[0].Integer.Value);
		failed = true;
	}
	/* Ensure bits 0 (discharging) and 1 (charging) are not both set, see 10.2.2.6 */
	if (((obj->Package.Elements[0].Integer.Value) & 3) == 3) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_BSTBadState",
			"%s: Battery State (Element 0) is "
			"indicating both charging and discharginng "
			"which is not allowed. Got value 0x%8.8" PRIx64 ".",
			name, (uint64_t)obj->Package.Elements[0].Integer.Value);
		failed = true;
	}
	/* Battery Present Rate - cannot check, pulled from EC */
	/* Battery Remaining Capacity - cannot check, pulled from EC */
	/* Battery Present Voltage - cannot check, pulled from EC */
	if (failed)
		fwts_advice(fw,
			"Battery %s package contains errors. It is "
			"worth running the firmware test suite "
			"interactive 'battery' test to see if this "
			"is problematic.  This is a bug an needs to "
			"be fixed.", name);
		else
			method_passed_sane(fw, name, "package");
}

static int method_test_BST(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MOBILE,
		"_BST", NULL, 0, method_test_BST_return, NULL);
}

static int method_test_BTP(fwts_framework *fw)
{
	static int values[] = { 0, 1, 100, 200, 0x7fffffff };
	int i;

	for (i = 0; i < 5; i++) {
		ACPI_OBJECT arg[1];
		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = values[i];
		if (method_evaluate_method(fw, METHOD_MOBILE, "_BTP", arg, 1,
			method_test_NULL_return, NULL) == FWTS_NOT_EXIST)
			break;
		fwts_log_nl(fw);
	}
	return FWTS_OK;
}

static void method_test_PCL_return(fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(fw);
	FWTS_UNUSED(name);
	FWTS_UNUSED(buf);
	FWTS_UNUSED(obj);
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_PCL", obj, ACPI_TYPE_LOCAL_REFERENCE) != FWTS_OK)
		return;

	fwts_passed(fw,	"%s returned a sane package of %" PRIu32 " references.", name, obj->Package.Count);
}

static int method_test_PCL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MOBILE,
		"_PCL", NULL, 0, method_test_PCL_return, "_PCL");
}

static int method_test_BTH(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	int i, ret;
	arg[0].Type = ACPI_TYPE_INTEGER;

	for (i = 0; i <= 100; i++) {
		arg[0].Integer.Value = i;
		ret = method_evaluate_method(fw, METHOD_OPTIONAL,
			"_BTH", arg, 1, method_test_NULL_return, NULL);

		if (ret != FWTS_OK)
			break;
	}
	return ret;
}

static int method_test_BTM(fwts_framework *fw)
{
	static int values[] = { 0, 1, 100, 200, 0x7fffffff };
	int i;

	for (i = 0 ; i < 5; i++) {
		ACPI_OBJECT arg[1];
		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = values[i];
		if (method_evaluate_method(fw, METHOD_MOBILE, "_BTM", arg, 1,
			method_test_NULL_return, NULL) == FWTS_NOT_EXIST)
			break;
		fwts_log_nl(fw);
	}
	return FWTS_OK;
}

static void method_test_BMD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_BMD", obj, 5) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_BMD", obj, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	fwts_acpi_object_dump(fw, obj);

	method_passed_sane(fw, name, "package");
}

static int method_test_BMD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MOBILE,
		"_BMD", NULL, 0, method_test_BMD_return, NULL);
}

static int method_test_BMC(fwts_framework *fw)
{
	static int values[] = { 0, 1, 2, 4 };
	int i;

	for (i = 0; i < 4; i++) {
		ACPI_OBJECT arg[1];
		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = values[i];
		if (method_evaluate_method(fw, METHOD_MOBILE, "_BMC", arg, 1,
			method_test_NULL_return, NULL) == FWTS_NOT_EXIST)
			break;
		fwts_log_nl(fw);
	}
	return FWTS_OK;
}


/*
 * Section 10.3 AC Adapters and Power Sources Objects
 */
static void method_test_PRL_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_PRL", obj, ACPI_TYPE_LOCAL_REFERENCE) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_PRL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PRL", NULL, 0, method_test_PRL_return, NULL);
}

static void method_test_PSR_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if (obj->Integer.Value > 2) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PSRZeroOrOne",
			"%s returned 0x%8.8" PRIx64 ", expected 0 "
			"(offline) or 1 (online)",
			name, (uint64_t)obj->Integer.Value);
	} else
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_PSR(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PSR", NULL, 0, method_test_PSR_return, NULL);
}

static void method_test_PIF_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	static fwts_package_element elements[] = {
		{ ACPI_TYPE_INTEGER,	"Power Source State" },
		{ ACPI_TYPE_INTEGER,	"Maximum Output Power" },
		{ ACPI_TYPE_INTEGER,	"Maximum Input Power" },
		{ ACPI_TYPE_STRING,	"Model Number" },
		{ ACPI_TYPE_STRING,	"Serial Number" },
		{ ACPI_TYPE_STRING,	"OEM Information" }
	};

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_PIF", obj, 6) != FWTS_OK)
		return;

	if (method_package_elements_type(fw, name, "_PIF", obj, elements, 6) != FWTS_OK)
		return;

	fwts_acpi_object_dump(fw, obj);

	method_passed_sane(fw, name, "package");
}

static int method_test_PIF(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PIF", NULL, 0, method_test_PIF_return, NULL);
}

/*
 * Section 10.4 Power Meters
 */

static int method_test_GAI(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GAI", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_GHL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GHL", NULL, 0, method_test_integer_return, NULL);
}

static void method_test_PMC_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;
	ACPI_OBJECT *element;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_PMC", obj, 14) != FWTS_OK)
		return;

	/* check element types */
	for (i = 0; i < 14; i++) {
		element = &obj->Package.Elements[i];
		if (i > 10) {
			if (element->Type != ACPI_TYPE_STRING) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_PMCBadElementType",
					"%s element %" PRIu32 " is not a string.", name, i);
				failed = true;
			}
		} else {
				if (element->Type != ACPI_TYPE_INTEGER) {
					fwts_failed(fw, LOG_LEVEL_MEDIUM,
						"Method_PMCBadElementType",
						"%s element %" PRIu32 " is not an integer.", name, i);
					failed = true;
				}
		}
	}

	/* check element's constraints */
	element = &obj->Package.Elements[0];
	if (element->Integer.Value & 0xFFFFFEF0) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PMCBadElement",
			"%s element 0 has reserved bits that are non-zero, got "
			"0x%" PRIx32 " and expected 0 for these field. ", name,
			(uint32_t) element->Integer.Value);
		failed = true;
	}

	element = &obj->Package.Elements[1];
	if (element->Integer.Value != 0) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PMCBadElement",
			"%s element 1 is expected to be 0, got 0x%" PRIx32 ".",
			name, (uint32_t) element->Integer.Value);
		failed = true;
	}

	element = &obj->Package.Elements[2];
	if (element->Integer.Value != 0 && element->Integer.Value != 1) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PMCBadElement",
			"%s element 2 is expected to be 0 or 1, got 0x%" PRIx32 ".",
			name, (uint32_t) element->Integer.Value);
		failed = true;
	}

	element = &obj->Package.Elements[3];
	if (element->Integer.Value > 100000) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PMCBadElement",
			"%s element 3 exceeds 100000 (100 percent) = 0x%" PRIx32 ".",
			name, (uint32_t) element->Integer.Value);
		failed = true;
	}

	/* nothing to check for elements 4~7 */

	element = &obj->Package.Elements[8];
	if (element->Integer.Value != 0 && element->Integer.Value != 0xFFFFFFFF) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_PMCBadElement",
			"%s element 8 is expected to be 0 or 1, got 0x%" PRIx32 ".",
			name, (uint32_t) element->Integer.Value);
		failed = true;
	}

	/* nothing to check for elements 9~13 */

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_PMC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PMC", NULL, 0, method_test_PMC_return, NULL);
}

static void method_test_PMD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_PMD", obj, ACPI_TYPE_LOCAL_REFERENCE) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_PMD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PMD", NULL, 0, method_test_PMD_return, NULL);
}

static int method_test_PMM(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PMM", NULL, 0, method_test_integer_return, NULL);
}

/*
 * Section 10.5 Wireless Power Controllers
 */
static void method_test_WPC_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if (obj->Integer.Value <= 0x02 || obj->Integer.Value == 0xff)
		method_passed_sane(fw, name, "integer");
	else
		fwts_failed(fw, LOG_LEVEL_HIGH,
			"Method_WPCInvalidInteger",
			"%s returned an invalid integer 0x%8.8" PRIx64,
			name, (uint64_t)obj->Integer.Value);
}

static int method_test_WPC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_WPC", NULL, 0, method_test_WPC_return, "_WPC");
}

static int method_test_WPP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_WPP", NULL, 0, method_test_integer_return, NULL);
}

/*
 * Section 11.3 Fan Devices
 */
static void method_test_FIF_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_FIF", obj, 4) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_FIF",
		obj, ACPI_TYPE_INTEGER) != FWTS_OK) {
		fwts_advice(fw,
			"%s is not returning the correct "
			"fan information. It may be worth "
			"running the firmware test suite "
			"interactive 'fan' test to see if "
			"this affects the control and "
			"operation of the fan.", name);
		return;
	}

	fwts_acpi_object_dump(fw, obj);

	method_passed_sane(fw, name, "package");
}

static int method_test_FIF(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_FIF", NULL, 0, method_test_FIF_return, NULL);
}

static void method_test_FPS_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (obj->Package.Elements[0].Type == ACPI_TYPE_INTEGER) {
		if (obj->Package.Elements[0].Integer.Value != 0) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_FPSBadRevision",
				"%s element 0 is not revision 0.", name);
			failed = true;
		}
	} else {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_FPSBadReturnType",
			"%s element 0 is not an integer.", name);
		failed = true;
	}

	/* Could be one or more sub-packages */
	for (i = 1; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		uint32_t j;
		bool elements_ok = true;

		if (obj->Package.Elements[i].Type != ACPI_TYPE_PACKAGE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_FPSBadReturnType",
				"%s element %" PRIu32 " is not a package.",
				name, i);
			failed = true;
			continue;
		}

		pkg = &obj->Package.Elements[i];
		if (pkg->Package.Count != 5) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_FPSBadSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 5 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		for (j = 0; j < 5; j++) {
			/* TODO - field 0 and 1 can be related to other control method */
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_FPSBadSubPackageReturnType",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"an integer.",
					name, i, j);
				elements_ok = false;
			}
		}

		if (!elements_ok) {
			failed = true;
			continue;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");

	method_passed_sane(fw, name, "package");
}

static int method_test_FPS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_FPS", NULL, 0, method_test_FPS_return, NULL);
}

static int method_test_FSL(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 50;

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_FSL", arg, 1, method_test_NULL_return, NULL);
}

static void method_test_FST_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_FST", obj, 3) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_FST",
		obj, ACPI_TYPE_INTEGER) != FWTS_OK) {
		fwts_advice(fw,
			"%s is not returning the correct "
			"fan status information. It may be "
			"worth running the firmware test "
			"suite interactive 'fan' test to see "
			"if this affects the control and "
			"operation of the fan.", name);
		return;
	}

	fwts_acpi_object_dump(fw, obj);

	method_passed_sane(fw, name, "package");
}

static int method_test_FST(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_FST", NULL, 0, method_test_FST_return, NULL);
}


/*
 * Section 11.4 Thermal
 */
static void method_test_THERM_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	char *method = (char*)private;

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if (fwts_acpi_region_handler_called_get()) {
		/*
		 *  We accessed some memory or I/O region during the
		 *  evaluation which returns spoofed values, so we
		 *  should not test the value being returned. In this
		 *  case, just pass this as a valid return type.
		 */
		method_passed_sane(fw, name, "return type");
	} else {
		/*
		 *  The evaluation probably was a hard-coded value,
		 *  so sanity check it
		 */
		if (obj->Integer.Value >= 2732) {
			fwts_passed(fw,
				"%s correctly returned sane looking "
				"value 0x%8.8" PRIx64 " (%5.1f degrees K)",
				method,
				(uint64_t)obj->Integer.Value,
				(float)((uint64_t)obj->Integer.Value) / 10.0);
		} else {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"MethodBadTemp",
				"%s returned a dubious value below "
				"0 degrees C: 0x%8.8" PRIx64 " (%5.1f "
				"degrees K)",
				method,
				(uint64_t)obj->Integer.Value,
				(float)((uint64_t)obj->Integer.Value) / 10.0);
			fwts_advice(fw,
				"The value returned was probably a "
				"hard-coded thermal value which is "
				"out of range because fwts did not "
				"detect any ACPI region handler "
				"accesses of I/O or system memeory "
				"to evaluate the thermal value. "
				"It is worth sanity checking these "
				"values in "
				"/sys/class/thermal/thermal_zone*.");
		}
	}
}

#define method_test_THERM(name, type)			\
static int method_test ## name(fwts_framework *fw)	\
{							\
	fwts_acpi_region_handler_called_set(false);	\
	return method_evaluate_method(fw, type, # name, \
		NULL, 0, method_test_THERM_return, # name);	\
}

method_test_THERM(_CRT, METHOD_OPTIONAL)
method_test_THERM(_CR3, METHOD_OPTIONAL)
method_test_THERM(_HOT, METHOD_OPTIONAL)
method_test_THERM(_TMP, METHOD_OPTIONAL)
method_test_THERM(_NTT, METHOD_OPTIONAL)
method_test_THERM(_PSV, METHOD_OPTIONAL)
method_test_THERM(_TST, METHOD_OPTIONAL)

static void method_test_MTL_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint64_t val;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	val = (uint64_t) obj->Integer.Value;
	if (val > 100) {
		failed = true;
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_MTLBadReturnType",
			"%s should return a percentage, got %" PRIu64 " instead", name, val);
	}

	if (!failed)
		method_passed_sane_uint64(fw, name, obj->Integer.Value);

	return;
}

static int method_test_MTL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_MTL", NULL, 0, method_test_MTL_return, NULL);
}

static void method_test_ART_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (obj->Package.Elements[0].Type == ACPI_TYPE_INTEGER) {
		if (obj->Package.Elements[0].Integer.Value != 0) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_ARTBadRevision",
				"%s element 0 is not revision 0.", name);
			failed = true;
		}
	} else {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_ARTBadReturnType",
			"%s element 0 is not an integer.", name);
		failed = true;
	}

	/* Could be one or more sub-packages */
	for (i = 1; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		uint32_t j;
		bool elements_ok = true;

		if (obj->Package.Elements[i].Type != ACPI_TYPE_PACKAGE) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_ARTBadReturnType",
				"%s element %" PRIu32 " is not a package.",
				name, i);
			failed = true;
			continue;
		}

		pkg = &obj->Package.Elements[i];
		if (pkg->Package.Count != 13) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_ARTBadSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 13 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		/* First two elements are references, and rests are integers */
		for (j = 0; j < 2; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_LOCAL_REFERENCE) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_ARTBadSubPackageReturnType",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"a reference.",
					name, i, j);
				elements_ok = false;
			}
		}

		for (j = 2; j < 13; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_ARTBadSubPackageReturnType",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"an integer.",
					name, i, j);
				elements_ok = false;
			}
		}

		if (!elements_ok) {
			failed = true;
			continue;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_ART(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_ART", NULL, 0, method_test_ART_return, "_ART");
}

static void method_test_PSL_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_PSL", obj, ACPI_TYPE_LOCAL_REFERENCE) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_PSL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_PSL", NULL, 0, method_test_PSL_return, "_PSL");
}

static void method_test_TRT_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_TRT", obj, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	/* Could be one or more packages */
	for (i = 0; i < obj->Package.Count; i++) {
		ACPI_OBJECT *pkg;
		uint32_t j;
		bool elements_ok = true;

		pkg = &obj->Package.Elements[i];
		if (pkg->Package.Count != 8) {
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_TRTSubPackageElementCount",
				"%s sub-package %" PRIu32 " was expected to "
				"have 8 elements, got %" PRIu32 " elements instead.",
				name, i, pkg->Package.Count);
			failed = true;
			continue;
		}

		/* First two elements are references, and rests are integers */
		for (j = 0; j < 2; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_LOCAL_REFERENCE) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_TRTBadSubPackageReturnType",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"a reference.",
					name, i, j);
				elements_ok = false;
			}
		}

		for (j = 2; j < 8; j++) {
			if (pkg->Package.Elements[j].Type != ACPI_TYPE_INTEGER) {
				fwts_failed(fw, LOG_LEVEL_MEDIUM,
					"Method_TRTBadSubPackageReturnType",
					"%s sub-package %" PRIu32
					" element %" PRIu32 " is not "
					"an integer.",
					name, i, j);
				elements_ok = false;
			}
		}

		if (!elements_ok) {
			failed = true;
			continue;
		}
	}

	if (!failed)
		method_passed_sane(fw, name, "package");
}

static int method_test_TRT(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TRT", NULL, 0, method_test_TRT_return, "_TRT");
}

static int method_test_TSN(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TSN", NULL, 0, method_test_reference_return, "_TSN");
}

static int method_test_TSP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TSP", NULL, 0, method_test_polling_return, "_TSP");
}

static void method_test_TCx_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) == FWTS_OK)
		method_passed_sane_uint64(fw, (char*)private, obj->Integer.Value);
}

static int method_test_TC1(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TC1", NULL, 0, method_test_TCx_return, "_TC1");
}

static int method_test_TC2(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TC2", NULL, 0, method_test_TCx_return, "_TC1");
}

static int method_test_TFP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TFP", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_ACx(fwts_framework *fw)
{
	int i;

	for (i = 0; i < 10; i++) {
		char buffer[5];

		snprintf(buffer, sizeof(buffer), "_AC%d", i);
		method_evaluate_method(fw, METHOD_OPTIONAL,
			buffer, NULL, 0, method_test_THERM_return, buffer);
		fwts_log_nl(fw);
	}
	return FWTS_OK;
}

static int method_test_DTI(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 2732 + 800; /* 80 degrees C */

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DTI", arg, 1, method_test_NULL_return, NULL);
}

static int method_test_SCP(fwts_framework *fw)
{
	int i;

	for (i = 0; i < 2; i++) {
		ACPI_OBJECT arg[3];

		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = i;		/* Mode */
		arg[1].Type = ACPI_TYPE_INTEGER;
		arg[1].Integer.Value = 5;		/* Acoustic limit */
		arg[2].Type = ACPI_TYPE_INTEGER;
		arg[2].Integer.Value = 5;		/* Power limit */

		if (method_evaluate_method(fw, METHOD_OPTIONAL,
			"_DTI", arg, 1, method_test_NULL_return,
			NULL) == FWTS_NOT_EXIST)
			break;
		fwts_log_nl(fw);

		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = i;		/* Mode */
		arg[1].Type = ACPI_TYPE_INTEGER;
		arg[1].Integer.Value = 1;		/* Acoustic limit */
		arg[2].Type = ACPI_TYPE_INTEGER;
		arg[2].Integer.Value = 1;		/* Power limit */

		if (method_evaluate_method(fw, METHOD_OPTIONAL,
			"_DTI", arg, 1, method_test_NULL_return,
			NULL) == FWTS_NOT_EXIST)
			break;
	}
	return FWTS_OK;
}

static void method_test_RTV_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_INTEGER) == FWTS_OK)
		method_passed_sane_uint64(fw, name, obj->Integer.Value);
}

static int method_test_RTV(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_RTV", NULL, 0, method_test_RTV_return, "_RTV");
}

static int method_test_TPT(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 2732 + 900; /* 90 degrees C */

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TPT", arg, 1, method_test_NULL_return, NULL);
}

static void method_test_TZD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_TZD", obj, ACPI_TYPE_LOCAL_REFERENCE) != FWTS_OK)
		return;

	fwts_passed(fw,	"%s returned a sane package of %" PRIu32 " references.", name, obj->Package.Count);
}

static int method_test_TZD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TZD", NULL, 0, method_test_TZD_return, "_TZD");
}

static int method_test_TZM(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TZM", NULL, 0, method_test_reference_return, "_TZM");
}

static int method_test_TZP(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_TZP", NULL, 0, method_test_polling_return, "_TZP");
}

/*
 * Section 12 Embedded Controller
 */

static void method_test_GPE_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);
	FWTS_UNUSED(buf);
	bool failed = false;

	switch (obj->Type) {
	case ACPI_TYPE_INTEGER:
		if (obj->Integer.Value <= 255)
			fwts_passed(fw, "%s returned an integer 0x%8.8" PRIx64,
				name, (uint64_t)obj->Integer.Value);
		else
			fwts_failed(fw, LOG_LEVEL_HIGH,
				"MethodGPEInvalidInteger",
				"%s returned an invalid integer 0x%8.8" PRIx64,
				name, (uint64_t)obj->Integer.Value);
		break;
	case ACPI_TYPE_PACKAGE:
		if (obj->Package.Elements[0].Type != ACPI_TYPE_LOCAL_REFERENCE) {
			failed = true;
			fwts_failed(fw, LOG_LEVEL_HIGH, "Method_GPEBadSubPackageReturnType",
			"%s sub-package element 0 is not a reference.",	name);
		}

		if (obj->Package.Elements[1].Type != ACPI_TYPE_INTEGER) {
			failed = true;
			fwts_failed(fw, LOG_LEVEL_HIGH, "Method_GPEBadSubPackageReturnType",
			"%s sub-package element 1 is not an integer.", name);
		}

		if (!failed)
			method_passed_sane(fw, name, "package");

		break;
	default:
		fwts_failed(fw, LOG_LEVEL_HIGH, "Method_GPEBadSubReturnType",
			"%s did not return an integer or a package.", name);
		break;
	}
}

static int method_test_GPE(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GPE", NULL, 0, method_test_GPE_return, "_GPE");
}

static int method_test_EC_(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_EC_", NULL, 0, method_test_integer_return, NULL);
}

/*
 * Section 16 Waking and Sleeping
 */
static int method_test_PTS(fwts_framework *fw)
{
	int i;

	for (i = 1; i < 6; i++) {
		ACPI_OBJECT arg[1];

		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = i;

		fwts_log_info(fw, "Test _PTS(%d).", i);

		if (method_evaluate_method(fw, METHOD_MANDATORY, "_PTS", arg, 1,
			method_test_NULL_return, NULL) == FWTS_NOT_EXIST) {
			fwts_advice(fw,
				"Could not find _PTS. This method provides a "
				"mechanism to do housekeeping functions, such "
				"as write sleep state to the embedded "
				"controller before entering a sleep state. If "
				"the machine cannot suspend (S3), "
				"hibernate (S4) or shutdown (S5) then it "
				"could be because _PTS is missing.  Note that "
				"ACPI 1.0 wants _PTS to be executed before "
				"suspending devices.");
			break;
		}
		fwts_log_nl(fw);
	}
	return FWTS_OK;
}

static int method_test_TTS(fwts_framework *fw)
{
	if (fwts_acpi_object_exists("_TTS") != NULL) {
		int i;

		for (i = 1; i < 6; i++) {
			ACPI_OBJECT arg[1];

			arg[0].Type = ACPI_TYPE_INTEGER;
			arg[0].Integer.Value = i;

			fwts_log_info(fw,
				"Test _TTS(%d) Transition To State S%d.", i, i);

			if (method_evaluate_method(fw, METHOD_MANDATORY,
				"_TTS", arg, 1, method_test_NULL_return,
				NULL) == FWTS_NOT_EXIST) {
				fwts_advice(fw,
					"Could not find _TTS. This method is invoked "
					"at the beginning of the the sleep transition "
					"for S1, S2, S3, S4 and S5 shutdown. The Linux "
					"kernel caters for firmware that does not implement "
					"_TTS, however, it will issue a warning that this "
					"control method is missing.");
				break;
			}
			fwts_log_nl(fw);
		}
	}
	else {
		fwts_skipped(fw,
			"Optional control method _TTS does not exist.");
	}
	return FWTS_OK;
}

static void method_test_WAK_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_equal(fw, name, "_WAK", obj, 2) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_WAK", obj, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	method_passed_sane(fw, name, "package");
}

static int method_test_WAK(fwts_framework *fw)
{
	uint32_t i;

	for (i = 1; i < 6; i++) {
		ACPI_OBJECT arg[1];
		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = i;
		fwts_log_info(fw, "Test _WAK(%d) System Wake, State S%d.", i, i);
		if (method_evaluate_method(fw, METHOD_MANDATORY, "_WAK", arg, 1,
			method_test_WAK_return, &i) == FWTS_NOT_EXIST) {
			fwts_advice(fw,
				"Section 7.3.7 states that a system that wakes "
				"from a sleeping state will invoke the _WAK "
				"control to issue device, thermal and other "
				"notifications to ensure that the operating system "
				"checks the states of various devices, thermal "
				"zones, etc.  The Linux kernel will report an "
				"ACPI exception if _WAK is does not exist when "
				"it returns from a sleep state.");
			break;
		}
		fwts_log_nl(fw);
	}
	return FWTS_OK;
}


/*
 * Appendix B ACPI Extensions for Display Adapters
 */
static int method_test_DOS(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 0 << 2 | 1;

	/*
	 * BIOS should toggle active display, BIOS controls brightness of
	 * LCD on AC/DC power changes
 	 */
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DOS", arg, 1, method_test_NULL_return, NULL);
}

static void method_test_DOD_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;

	static char *dod_type[] = {
		"Other",
		"VGA, CRT or VESA Compatible Analog Monitor",
		"TV/HDTV or other Analog-Video Monitor",
		"External Digital Monitor",

		"Internal/Integrated Digital Flat Panel",
		"Reserved",
		"Reserved",
		"Reserved",

		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",

		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved"
	};

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	for (i = 0; i < obj->Package.Count; i++) {
		if (obj->Package.Elements[i].Type != ACPI_TYPE_INTEGER)
			failed = true;
		else {
			uint32_t val = obj->Package.Elements[i].Integer.Value;
			fwts_log_info_verbatim(fw, "Device %" PRIu32 ":", i);
			if ((val & 0x80000000)) {
				fwts_log_info_verbatim(fw, "  Video Chip Vendor Scheme %" PRIu32, val);
			} else {
				fwts_log_info_verbatim(fw, "  Instance:                %" PRIu32, val & 0xf);
				fwts_log_info_verbatim(fw, "  Display port attachment: %" PRIu32, (val >> 4) & 0xf);
				fwts_log_info_verbatim(fw, "  Type of display:         %" PRIu32 " (%s)",
					(val >> 8) & 0xf, dod_type[(val >> 8) & 0xf]);
				fwts_log_info_verbatim(fw, "  BIOS can detect device:  %" PRIu32, (val >> 16) & 1);
				fwts_log_info_verbatim(fw, "  Non-VGA device:          %" PRIu32, (val >> 17) & 1);
				fwts_log_info_verbatim(fw, "  Head or pipe ID:         %" PRIu32, (val >> 18) & 0x7);
			}
		}
	}

	if (failed)
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_DODNoPackage",
			"Method _DOD did not return a package of "
			"%" PRIu32 " integers.", obj->Package.Count);
	else
		method_passed_sane(fw, name, "package");
}

static int method_test_DOD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DOD", NULL, 0, method_test_DOD_return, NULL);
}

static void method_test_ROM_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	FWTS_UNUSED(obj);
	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_BUFFER) == FWTS_OK)
		method_passed_sane(fw, name, "package");
}

static int method_test_ROM(fwts_framework *fw)
{
	ACPI_OBJECT arg[2];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 0;
	arg[1].Type = ACPI_TYPE_INTEGER;
	arg[1].Integer.Value = 4096;

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_ROM", arg, 2, method_test_ROM_return, NULL);
}

static int method_test_GPD(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_GPD", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_SPD(fwts_framework *fw)
{
	ACPI_OBJECT arg[2];
	int i;

	for (i = 0; i < 4; i++) {
		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = i;	/* bits 00..11, post device */

		if (method_evaluate_method(fw, METHOD_OPTIONAL,
			"_SPD", arg, 1, method_test_passed_failed_return, NULL) == FWTS_NOT_EXIST)
			break;
	}
	return FWTS_OK;
}

static int method_test_VPO(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_VPO", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_ADR(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_ADR", NULL, 0, method_test_integer_return, NULL);
}

static void method_test_BCL_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t i;
	bool failed = false;
	bool ascending_levels = false;
	char *str = NULL;

	FWTS_UNUSED(private);

	if (method_check_type(fw, name, buf, ACPI_TYPE_PACKAGE) != FWTS_OK)
		return;

	if (method_package_count_min(fw, name, "_BCL", obj, 3) != FWTS_OK)
		return;

	if (method_package_elements_all_type(fw, name, "_BCL", obj, ACPI_TYPE_INTEGER) != FWTS_OK)
		return;

	if (obj->Package.Elements[0].Integer.Value <
	    obj->Package.Elements[1].Integer.Value) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_BCLMaxLevel",
			"Brightness level when on full "
			" power (%" PRIu64 ") is less than "
				"brightness level when on "
			"battery power (%" PRIu64 ").",
			(uint64_t)obj->Package.Elements[0].Integer.Value,
			(uint64_t)obj->Package.Elements[1].Integer.Value);
		failed = true;
	}

	for (i = 2; i < obj->Package.Count - 1; i++) {
		if (obj->Package.Elements[i].Integer.Value >
		    obj->Package.Elements[i+1].Integer.Value) {
			fwts_log_info(fw,
				"Brightness level %" PRIu64
				" (index %" PRIu32 ") is greater "
				"than brightness level %" PRIu64
				" (index %d" PRIu32 "), should "
				"be in ascending order.",
				(uint64_t)obj->Package.Elements[i].Integer.Value, i,
				(uint64_t)obj->Package.Elements[i+1].Integer.Value, i+1);
			ascending_levels = true;
			failed = true;
		}
	}
	if (ascending_levels) {
		fwts_failed(fw, LOG_LEVEL_MEDIUM,
			"Method_BCLAscendingOrder",
			"Some or all of the brightness "
			"level are not in ascending "
			"order which should be fixed "
			"in the firmware.");
		failed = true;
	}

	fwts_log_info(fw, "Brightness levels for %s:" ,name);
	fwts_log_info_verbatim(fw, "  Level on full power   : %" PRIu64,
		(uint64_t)obj->Package.Elements[0].Integer.Value);
	fwts_log_info_verbatim(fw, "  Level on battery power: %" PRIu64,
		(uint64_t)obj->Package.Elements[1].Integer.Value);
	for (i = 2; i < obj->Package.Count; i++) {
		char tmp[12];

		snprintf(tmp, sizeof(tmp), "%s%" PRIu64,
			i == 2 ? "" : ", ",
			(uint64_t)obj->Package.Elements[i].Integer.Value);
		str = fwts_realloc_strcat(str, tmp);
		if (!str)
			break;
	}
	if (str) {
		fwts_log_info_verbatim(fw, "  Brightness Levels     : %s", str);
		free(str);
	}

	if (failed)
		fwts_advice(fw,
			"%s seems to be "
			"misconfigured and is "
			"returning incorrect "
			"brightness levels."
			"It is worth sanity checking "
			"this with the firmware test "
			"suite interactive test "
			"'brightness' to see how "
			"broken this is. As it is, "
			"_BCL is broken and needs to "
			"be fixed.", name);
	else
		fwts_passed(fw,
			"%s returned a sane "
			"package of %" PRIu32 " integers.",
			name, obj->Package.Count);
}

static int method_test_BCL(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_BCL", NULL, 0, method_test_BCL_return, NULL);
}

static int method_test_BCM(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 0;

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_BCM", arg, 1, method_test_NULL_return, NULL);
}

static int method_test_BQC(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_BQC", NULL, 0, method_test_integer_return, NULL);
}

static void method_test_DDC_return(
	fwts_framework *fw,
	char *name,
	ACPI_BUFFER *buf,
	ACPI_OBJECT *obj,
	void *private)
{
	uint32_t requested = *(uint32_t*)private;

	FWTS_UNUSED(buf);

	if (obj == NULL) {
		method_failed_null_object(fw, name, "a buffer or integer");
		return;
	}

	switch (obj->Type) {
	case ACPI_TYPE_BUFFER:
		if (requested != obj->Buffer.Length)
			fwts_failed(fw, LOG_LEVEL_MEDIUM,
				"Method_DDCElementCount",
				"%s returned a buffer of %" PRIu32 " items, "
				"expected %" PRIu32 ".",
				name, obj->Buffer.Length, requested);
		else
			fwts_passed(fw,
				"Method %s returned a buffer of %d items "
				"as expected.",
				name, obj->Buffer.Length);
		break;
	case ACPI_TYPE_INTEGER:
			fwts_passed(fw,
				"%s could not return a buffer of %d "
					"items and instead returned an error "
					"status.",
				name, obj->Buffer.Length);
		break;
	default:
		fwts_failed(fw, LOG_LEVEL_MEDIUM, "Method_DDCBadReturnType",
			"%s did not return a buffer or an integer.", name);
		break;
	}
}

static int method_test_DDC(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	uint32_t i;

	for (i = 128; i <= 256; i <<= 1) {
		arg[0].Type = ACPI_TYPE_INTEGER;
		arg[0].Integer.Value = 128;

		if (method_evaluate_method(fw, METHOD_OPTIONAL,
			"_DDC", arg, 1, method_test_DDC_return,
			&i) == FWTS_NOT_EXIST)
			break;
	}
	return FWTS_OK;
}

static int method_test_DCS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DCS", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_DGS(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DGS", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_DSS(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];

	arg[0].Type = ACPI_TYPE_INTEGER;
	arg[0].Integer.Value = 0;

	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_DSS", arg, 1, method_test_NULL_return, NULL);
}

static int method_test_CBA(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_CBA", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_CDM(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_CDM", NULL, 0, method_test_integer_return, NULL);
}

/*
 * Intelligent Platform Management Interface (IPMI) Specification
 */
static int method_test_IFT(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_IFT", NULL, 0, method_test_integer_return, NULL);
}

static int method_test_SRV(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_OPTIONAL,
		"_SRV", NULL, 0, method_test_integer_return, NULL);
}

/* ARM SBBR Test Definitions */
static int sbbr_method_test_ADR(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MANDATORY,
		"_ADR", NULL, 0, method_test_integer_return, NULL);
}

static int sbbr_method_test_AEI(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MANDATORY,
		"_AEI", NULL, 0, method_test_AEI_return, NULL);
}

static int sbbr_method_test_CCA(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MANDATORY,
		"_CCA", NULL, 0, method_test_integer_return, NULL);
}

static int sbbr_method_test_EVT(fwts_framework *fw)
{
	int ret;

	/* Only test the _EVT method with pins defined in AEI. */
	ret = method_evaluate_method(fw, METHOD_MANDATORY,
		"_AEI", NULL, 0, method_test_EVT_return, NULL);

	if (ret == FWTS_NOT_EXIST)
		fwts_failed(fw, LOG_LEVEL_HIGH, "SbbrAcpiEvtDoesNotExist", "Method _EVT does not exist.");

	return ret;
}

static int sbbr_method_test_HID(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MANDATORY,
		"_HID", NULL, 0, method_test_HID_return, NULL);
}

static int sbbr_method_test_SST(fwts_framework *fw)
{
	ACPI_OBJECT arg[1];
	int ret, i;

	arg[0].Type = ACPI_TYPE_INTEGER;
	for (i = 0; i <= 4; i++) {
		arg[0].Integer.Value = i;
		ret = method_evaluate_method(fw, METHOD_MANDATORY,
			"_SST", arg, 1, method_test_NULL_return, NULL);

		if (ret != FWTS_OK)
			break;
	}
	return ret;
}

static int sbbr_method_test_STA(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MANDATORY,
		"_STA", NULL, 0, method_test_STA_return, "_STA");
}

static int sbbr_method_test_UID(fwts_framework *fw)
{
	return method_evaluate_method(fw, METHOD_MANDATORY,
		"_UID", NULL, 0, method_test_UID_return, NULL);
}


/*
 * Tests
 */
static fwts_framework_minor_test sbbr_method_tests[] = {
	{ method_name_check, "Test Method Names." },

	/* Section 5.3 */
	/* { method_test_PR , "Test _PR  (Processor)." }, */

	/* Section 5.6 ACPI Event Programming Model */
	/* { method_test_Wxx, "Test _Wxx (Wake Event)." }, */

	{ method_test_AEI, "Test _AEI." },
	{ method_test_EVT, "Test _EVT (Event Method)." },

	/* Section 5.7 Predefined Objects */
	{ method_test_DLM, "Test _DLM (Device Lock Mutex)." },
	/* { method_test_GL , "Test _GL  (Global Lock)." }, */
	/* { method_test_OS , "Test _OS  (Operating System)." }, */
	/* { method_test_REV, "Test _REV (Revision)." }, */

	/* Section 5.8 System Configuration Objects */
	{ method_test_PIC, "Test _PIC (Inform AML of Interrupt Model)." },

	/* Section 6.1 Device Identification Objects  */

	{ method_test_CID, "Test _CID (Compatible ID)." },
	/* { method_test_CLS, "Test _CLS (Class Code)." }, */
	{ method_test_DDN, "Test _DDN (DOS Device Name)." },
	{ method_test_HID, "Test _HID (Hardware ID)." },
	{ method_test_HRV, "Test _HRV (Hardware Revision Number)." },
	{ method_test_MLS, "Test _MLS (Multiple Language String)." },
	{ method_test_PLD, "Test _PLD (Physical Device Location)." },
	{ method_test_SUB, "Test _SUB (Subsystem ID)." },
	{ method_test_SUN, "Test _SUN (Slot User Number)." },
	{ method_test_STR, "Test _STR (String)." },
	{ method_test_UID, "Test _UID (Unique ID)." },

	/* Section 6.2 Device Configurations Objects */

	{ method_test_CDM, "Test _CDM (Clock Domain)." },
	{ method_test_CRS, "Test _CRS (Current Resource Settings)." },
	{ method_test_DSD, "Test _DSD (Device Specific Data)." },
	{ method_test_DIS, "Test _DIS (Disable)." },
	{ method_test_DMA, "Test _DMA (Direct Memory Access)." },
	{ method_test_FIX, "Test _FIX (Fixed Register Resource Provider)." },
	{ method_test_GSB, "Test _GSB (Global System Interrupt Base)." },
	{ method_test_HPP, "Test _HPP (Hot Plug Parameters)." },
	/* { method_test_HPX, "Test _HPX (Hot Plug Extensions)." }, */
	/* { method_test_MAT, "Test _MAT (Multiple APIC Table Entry)." }, */
	{ method_test_PRS, "Test _PRS (Possible Resource Settings)." },
	{ method_test_PRT, "Test _PRT (PCI Routing Table)." },
	{ method_test_PXM, "Test _PXM (Proximity)." },
	/* { method_test_SLI, "Test _SLI (System Locality Information)." }, */
	/* { method_test_SRS, "Test _SRS (Set Resource Settings)." }, */
	{ method_test_CCA, "Test _CCA (Cache Coherency Attribute)." },

	/* Section 6.3 Device Insertion, Removal and Status Objects */

	{ method_test_EDL, "Test _EDL (Eject Device List)." },
	{ method_test_EJD, "Test _EJD (Ejection Dependent Device)." },
	{ method_test_EJ0, "Test _EJ0 (Eject)." },
	{ method_test_EJ1, "Test _EJ1 (Eject)." },
	{ method_test_EJ2, "Test _EJ2 (Eject)." },
	{ method_test_EJ3, "Test _EJ3 (Eject)." },
	{ method_test_EJ4, "Test _EJ4 (Eject)." },
	{ method_test_LCK, "Test _LCK (Lock)." },
	/* { method_test_OST, "Test _OST (OSPM Status Indication)." }, */
	{ method_test_RMV, "Test _RMV (Remove)." },
	{ method_test_STA, "Test _STA (Status)." },

	/* Section 6.4 Resource Data Types for ACPI */

	/* Section 6.5 Other Objects and Controls */

	{ method_test_DEP, "Test _DEP (Operational Region Dependencies)." },
	{ method_test_FIT, "Test _FIT (Firmware Interface Table)." },
	{ method_test_BDN, "Test _BDN (BIOS Dock Name)." },
	{ method_test_BBN, "Test _BBN (Base Bus Number)." },
	{ method_test_DCK, "Test _DCK (Dock)." },
	{ method_test_INI, "Test _INI (Initialize)." },
	{ method_test_GLK, "Test _GLK (Global Lock)." },
	/* { method_test_REG, "Test _REG (Region)." }, */
	{ method_test_SEG, "Test _SEG (Segment)." },

	/* Section 7.1 Declaring a Power Resource Object */

	{ method_test_OFF, "Test _OFF (Set resource off)." },
	{ method_test_ON_,  "Test _ON_ (Set resource on)." },

	/* Section 7.2 Device Power Management Objects */

	{ method_test_DSW, "Test _DSW (Device Sleep Wake)." },
	{ method_test_IRC, "Test _IRC (In Rush Current)." },
	{ method_test_PRE, "Test _PRE (Power Resources for Enumeration)." },
	{ method_test_PR0, "Test _PR0 (Power Resources for D0)." },
	{ method_test_PR1, "Test _PR1 (Power Resources for D1)." },
	{ method_test_PR2, "Test _PR2 (Power Resources for D2)." },
	{ method_test_PR3, "Test _PR3 (Power Resources for D3)." },
	{ method_test_PRW, "Test _PRW (Power Resources for Wake)." },
	{ method_test_PS0, "Test _PS0 (Power State 0)." },
	{ method_test_PS1, "Test _PS1 (Power State 1)." },
	{ method_test_PS2, "Test _PS2 (Power State 2)." },
	{ method_test_PS3, "Test _PS3 (Power State 3)." },
	{ method_test_PSC, "Test _PSC (Power State Current)." },
	{ method_test_PSE, "Test _PSE (Power State for Enumeration)." },
	{ method_test_PSW, "Test _PSW (Power State Wake)." },
	{ method_test_S1D, "Test _S1D (S1 Device State)." },
	{ method_test_S2D, "Test _S2D (S2 Device State)." },
	{ method_test_S3D, "Test _S3D (S3 Device State)." },
	{ method_test_S4D, "Test _S4D (S4 Device State)." },
	{ method_test_S0W, "Test _S0W (S0 Device Wake State)." },
	{ method_test_S1W, "Test _S1W (S1 Device Wake State)." },
	{ method_test_S2W, "Test _S2W (S2 Device Wake State)." },
	{ method_test_S3W, "Test _S3W (S3 Device Wake State)." },
	{ method_test_S4W, "Test _S4W (S4 Device Wake State)." },
	{ method_test_RST, "Test _RST (Device Reset)." },
	{ method_test_PRR, "Test _PRR (Power Resource for Reset)." },

	/* Section 7.3 OEM-Supplied System-Level Control Methods */
	{ method_test_S0_, "Test _S0_ (S0 System State)." },
	{ method_test_S1_, "Test _S1_ (S1 System State)." },
	{ method_test_S2_, "Test _S2_ (S2 System State)." },
	{ method_test_S3_, "Test _S3_ (S3 System State)." },
	{ method_test_S4_, "Test _S4_ (S4 System State)." },
	{ method_test_S5_, "Test _S5_ (S5 System State)." },
	{ method_test_SWS, "Test _SWS (System Wake Source)." },

	/* Section 8.4 Declaring Processors */

	{ method_test_PSS, "Test _PSS (Performance Supported States)." },
	{ method_test_CPC, "Test _CPC (Continuous Performance Control)." },
	{ method_test_CSD, "Test _CSD (C State Dependencies)." },
	{ method_test_CST, "Test _CST (C States)." },
	{ method_test_PCT, "Test _PCT (Performance Control)." },
	/* { method_test_PDC, "Test _PDC (Processor Driver Capabilities)." }, */
	{ method_test_PDL, "Test _PDL (P-State Depth Limit)." },
	{ method_test_PPC, "Test _PPC (Performance Present Capabilities)." },
	{ method_test_PPE, "Test _PPE (Polling for Platform Error)." },
	{ method_test_PSD, "Test _PSD (Power State Dependencies)." },
	{ method_test_PTC, "Test _PTC (Processor Throttling Control)." },
	{ method_test_TDL, "Test _TDL (T-State Depth Limit)." },
	{ method_test_TPC, "Test _TPC (Throttling Present Capabilities)." },
	{ method_test_TSD, "Test _TSD (Throttling State Dependencies)." },
	{ method_test_TSS, "Test _TSS (Throttling Supported States)." },

	/* Section 8.4.4 Lower Power Idle States */
	{ method_test_LPI, "Test _LPI (Low Power Idle States)." },
	{ method_test_RDI, "Test _RDI (Resource Dependencies for Idle)." },

	/* Section 8.5 Processor Aggregator Device */
	{ method_test_PUR, "Test _PUR (Processor Utilization Request)." },

	/* Section 9.1 System Indicators */
	{ method_test_MSG, "Test _MSG (Message)." },
	{ method_test_SST, "Test _SST (System Status)." },

	/* Section 9.2 Ambient Light Sensor Device */

	{ method_test_ALC, "Test _ALC (Ambient Light Colour Chromaticity)." },
	{ method_test_ALI, "Test _ALI (Ambient Light Illuminance)." },
	{ method_test_ALT, "Test _ALT (Ambient Light Temperature)." },
	{ method_test_ALP, "Test _ALP (Ambient Light Polling)."},
	{ method_test_ALR, "Test _ALR (Ambient Light Response)."},

	/* Section 9.3 Battery Device */

	/* Section 9.4 Lid Device */

	{ method_test_LID, "Test _LID (Lid Status)." },

	/* Section 9.8 ATA Controllers */
	{ method_test_GTF, "Test _GTF (Get Task File)." },
	{ method_test_GTM, "Test _GTM (Get Timing Mode)." },
	/* { method_test_SDD, "Test _SDD (Set Device Data)." }, */
	/* { method_test_STM, "Test _STM (Set Timing Mode)." }, */

	/* Section 9.9 Floppy Controllers */
	/* { method_test_FDE, "Test _FDE (Floppy Disk Enumerate)." }, */
	/* { method_test_FDI, "Test _FDI (Floppy Drive Information)." }, */
	/* { method_test_FDM, "Test _FDM (Floppy Drive Mode)." }, */

	/* Section 9.12 Memory Devices */
	{ method_test_MBM, "Test _MBM (Memory Bandwidth Monitoring Data)." },
	/* { method_test_MSM, "Test _MSM (Memory Set Monitoring)." }, */

	/* Section 9.13 USB Port Capabilities */
	{ method_test_UPC, "Test _UPC (USB Port Capabilities)." },

	/* Section 9.14 Device Object Name Collision */
	/* { method_test_DSM, "Test _DSM (Device Specific Method)." }, */

	/* Section 9.16 User Presence Detection Device */
	{ method_test_UPD, "Test _UPD (User Presence Detect)." },
	{ method_test_UPP, "Test _UPP (User Presence Polling)." },

	/* Section 9.18 Wake Alarm Device */

	{ method_test_GCP, "Test _GCP (Get Capabilities)." },
	{ method_test_GRT, "Test _GRT (Get Real Time)." },
	{ method_test_GWS, "Test _GWS (Get Wake Status)." },
	{ method_test_CWS, "Test _CWS (Clear Wake Status)." },
	{ method_test_SRT, "Test _SRT (Set Real Time)." },
	{ method_test_STP, "Test _STP (Set Expired Timer Wake Policy)." },
	{ method_test_STV, "Test _STV (Set Timer Value)." },
	{ method_test_TIP, "Test _TIP (Expired Timer Wake Policy)." },
	{ method_test_TIV, "Test _TIV (Timer Values)." },

	/* Section 10.1 Smart Battery */

	{ method_test_SBS, "Test _SBS (Smart Battery Subsystem)." },

	/* Section 10.2 Battery Controls */

	{ method_test_BCT, "Test _BCT (Battery Charge Time)." },
	{ method_test_BIF, "Test _BIF (Battery Information)." },
	{ method_test_BIX, "Test _BIX (Battery Information Extended)." },
	{ method_test_BMA, "Test _BMA (Battery Measurement Averaging)." },
	{ method_test_BMC, "Test _BMC (Battery Maintenance Control)." },
	{ method_test_BMD, "Test _BMD (Battery Maintenance Data)." },
	{ method_test_BMS, "Test _BMS (Battery Measurement Sampling Time)." },
	{ method_test_BST, "Test _BST (Battery Status)." },
	{ method_test_BTP, "Test _BTP (Battery Trip Point)." },
	{ method_test_BTH, "Test _BTH (Battery Throttle Limit)." },
	{ method_test_BTM, "Test _BTM (Battery Time)." },
	/* { method_test_BLT, "Test _BLT (Battery Level Threshold)." }, */

	/* Section 10.3 AC Adapters and Power Source Objects */

	{ method_test_PCL, "Test _PCL (Power Consumer List)." },
	{ method_test_PIF, "Test _PIF (Power Source Information)." },
	{ method_test_PRL, "Test _PRL (Power Source Redundancy List)." },
	{ method_test_PSR, "Test _PSR (Power Source)." },

	/* Section 10.4 Power Meters */
	{ method_test_GAI, "Test _GAI (Get Averaging Level)." },
	{ method_test_GHL, "Test _GHL (Get Harware Limit)." },
	/* { method_test_PAI, "Test _PAI (Power Averaging Interval)." }, */
	{ method_test_PMC, "Test _PMC (Power Meter Capabilities)." },
	{ method_test_PMD, "Test _PMD (Power Meter Devices)." },
	{ method_test_PMM, "Test _PMM (Power Meter Measurement)." },
	/* { method_test_PTP, "Test _PTP (Power Trip Points)." }, */
	/* { method_test_SHL, "Test _SHL (Set Hardware Limit)." }, */

	/* Section 10.5 Wireless Power Controllers */
	{ method_test_WPC, "Test _WPC (Wireless Power Calibration)." },
	{ method_test_WPP, "Test _WPP (Wireless Power Polling)." },

	/* Section 11.3 Fan Devices */

	{ method_test_FIF, "Test _FIF (Fan Information)." },
	{ method_test_FPS, "Test _FPS (Fan Performance States)." },
	{ method_test_FSL, "Test _FSL (Fan Set Level)." },
	{ method_test_FST, "Test _FST (Fan Status)." },

	/* Section 11.4 Thermal Objects */

	{ method_test_ACx, "Test _ACx (Active Cooling)." },
	{ method_test_ART, "Test _ART (Active Cooling Relationship Table)." },
	/* { method_test_ALx, "Test _ALx (Active List)". }, */
	{ method_test_CRT, "Test _CRT (Critical Trip Point)." },
	{ method_test_CR3, "Test _CR3 (Warm/Standby Temperature)." },
	{ method_test_DTI, "Test _DTI (Device Temperature Indication)." },
	{ method_test_HOT, "Test _HOT (Hot Temperature)." },
	{ method_test_MTL, "Test _MTL (Minimum Throttle Limit)." },
	{ method_test_NTT, "Test _NTT (Notification Temp Threshold)." },
	{ method_test_PSL, "Test _PSL (Passive List)." },
	{ method_test_PSV, "Test _PSV (Passive Temp)." },
	{ method_test_RTV, "Test _RTV (Relative Temp Values)." },
	{ method_test_SCP, "Test _SCP (Set Cooling Policy)." },
	{ method_test_TC1, "Test _TC1 (Thermal Constant 1)." },
	{ method_test_TC2, "Test _TC2 (Thermal Constant 2)." },
	{ method_test_TFP, "Test _TFP (Thermal fast Sampling Period)." },
	{ method_test_TMP, "Test _TMP (Thermal Zone Current Temp)." },
	{ method_test_TPT, "Test _TPT (Trip Point Temperature)." },
	{ method_test_TRT, "Test _TRT (Thermal Relationship Table)." },
	{ method_test_TSN, "Test _TSN (Thermal Sensor Device)." },
	{ method_test_TSP, "Test _TSP (Thermal Sampling Period)." },
	{ method_test_TST, "Test _TST (Temperature Sensor Threshold)." },
	{ method_test_TZD, "Test _TZD (Thermal Zone Devices)." },
	{ method_test_TZM, "Test _TZM (Thermal Zone member)." },
	{ method_test_TZP, "Test _TZP (Thermal Zone Polling)." },

	/* Section 12 Embedded Controller Interface */
	{ method_test_GPE, "Test _GPE (General Purpose Events)." },
	{ method_test_EC_,  "Test _EC_ (EC Offset Query)." },

	/* Section 16 Waking and Sleeping */

	{ method_test_PTS, "Test _PTS (Prepare to Sleep)." },
	{ method_test_TTS, "Test _TTS (Transition to State)." },
	{ method_test_WAK, "Test _WAK (System Wake)." },

	/* Appendix B, ACPI Extensions for Display Adapters */

	{ method_test_ADR, "Test _ADR (Return Unique ID for Device)." },
	{ method_test_BCL, "Test _BCL (Query List of Brightness Control Levels Supported)." },
	{ method_test_BCM, "Test _BCM (Set Brightness Level)." },
	{ method_test_BQC, "Test _BQC (Brightness Query Current Level)." },
	{ method_test_DCS, "Test _DCS (Return the Status of Output Device)." },
	{ method_test_DDC, "Test _DDC (Return the EDID for this Device)." },
	{ method_test_DSS, "Test _DSS (Device Set State)." },
	{ method_test_DGS, "Test _DGS (Query Graphics State)." },
	{ method_test_DOD, "Test _DOD (Enumerate All Devices Attached to Display Adapter)." },
	{ method_test_DOS, "Test _DOS (Enable/Disable Output Switching)." },
	{ method_test_GPD, "Test _GPD (Get POST Device)." },
	{ method_test_ROM, "Test _ROM (Get ROM Data)." },
	{ method_test_SPD, "Test _SPD (Set POST Device)." },
	{ method_test_VPO, "Test _VPO (Video POST Options)." },

	/* From PCI Specification */
	{ method_test_CBA, "Test _CBA (Configuration Base Address)." },

	/* From IPMI Specification 2.0 */
	{ method_test_IFT, "Test _IFT (IPMI Interface Type)." },
	{ method_test_SRV, "Test _SRV (IPMI Interface Revision)." },

	/* From ARM SBBR */
	{ sbbr_method_test_ADR, "SBBR Test _ADR (Return Unique ID for Device)." },
	{ sbbr_method_test_AEI, "SBBR Test _AEI (Event Information)." },
	{ sbbr_method_test_CCA, "SBBR Test _CCA (Cache Coherency Attribute)." },
	{ sbbr_method_test_EVT, "SBBR Test _EVT (Event Method)." },
	{ sbbr_method_test_HID, "SBBR Test _HID (Hardware ID)." },
	{ sbbr_method_test_SST, "SBBR Test _SST (System Status)." },
	{ sbbr_method_test_STA, "SBBR Test _STA (Status)." },
	{ sbbr_method_test_UID, "SBBR Test _UID (Unique ID)." },

	/* End! */

	{ NULL, NULL }
};

static fwts_framework_ops sbbr_method_ops = {
	.description = "ACPI DSDT Method Semantic tests.",
	.init        = sbbr_method_init,
	.deinit      = sbbr_method_deinit,
	.minor_tests = sbbr_method_tests
};

FWTS_REGISTER("sbbr_method", &sbbr_method_ops, FWTS_TEST_ANYTIME, FWTS_FLAG_TEST_SBBR)

#endif
