#include <fltkernel.h>

NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);

BOOLEAN NTAPI RtlSuffixUnicodeStringEx(_In_ PCUNICODE_STRING String1, _In_ PCUNICODE_STRING String2, _In_ BOOLEAN CaseInSensitive)
{
	UNICODE_STRING a;

	if (String2->Length >= String1->Length)
	{
		a.Buffer = (PWCH)((PUCHAR)String2->Buffer + String2->Length - String1->Length);
		a.Length = String1->Length;
		a.MaximumLength = a.Length;

		return (0 == RtlCompareUnicodeString(String1, &a, CaseInSensitive)) ? TRUE : FALSE;
	}

	return FALSE;
}

VOID ExUnlockUserBuffer(
	__inout PVOID LockVariable
)
{
	MmUnlockPages((PMDL)LockVariable);
	ExFreePool((PMDL)LockVariable);
	return;
}

NTSTATUS ExLockUserBuffer(
	__inout_bcount(Length) PVOID Buffer,
	__in ULONG Length,
	__in KPROCESSOR_MODE ProbeMode,
	__in LOCK_OPERATION LockMode,
	__deref_out PVOID *LockedBuffer,
	__deref_out PVOID *LockVariable
)
{
	PMDL Mdl;
	SIZE_T MdlSize;

	//
	// It is the caller's responsibility to ensure zero cannot be passed in.
	//

	ASSERT(Length != 0);

	*LockedBuffer = NULL;
	*LockVariable = NULL;

	//
	// Allocate an MDL to map the request.
	//

	MdlSize = MmSizeOfMdl(Buffer, Length);
	Mdl = (PMDL)ExAllocatePoolWithQuotaTag((POOL_TYPE)((int)NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE),
		MdlSize,
		'ofnI');
	if (Mdl == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//
	// Initialize MDL for request.
	//

	MmInitializeMdl(Mdl, Buffer, Length);

	__try {

		MmProbeAndLockPages(Mdl, ProbeMode, LockMode);

	}
	__except (EXCEPTION_EXECUTE_HANDLER) {

		ExFreePool(Mdl);

		return GetExceptionCode();
	}

	Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;
	*LockedBuffer = MmGetSystemAddressForMdlSafe(Mdl, HighPagePriority);
	if (*LockedBuffer == NULL) {
		ExUnlockUserBuffer(Mdl);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	*LockVariable = Mdl;
	return STATUS_SUCCESS;
}

VOID LoadImageNotifyCallback(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO pImageInfo)
{
	UNREFERENCED_PARAMETER(pImageInfo);

	UNICODE_STRING str;
	RtlInitUnicodeString(&str, L"\\vivoxsdk_x64.dll");
	if (RtlSuffixUnicodeStringEx(&str, FullImageName, TRUE))
	{
		PEPROCESS Process;
		if (NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &Process)))
		{
			PUNICODE_STRING ProcessName;
			if (NT_SUCCESS(SeLocateProcessImageName(Process, &ProcessName)))
			{
				UNICODE_STRING Suffix;
				RtlInitUnicodeString(&Suffix, L"\\FortniteClient-Win64-Shipping.exe");
				if (RtlSuffixUnicodeStringEx(&Suffix, ProcessName, TRUE))
				{
					PVOID MappedAddress = NULL;
					PVOID LockVariable = NULL;
					NTSTATUS st1 = ExLockUserBuffer((PUCHAR)0x1419236A6,
						3,
						ExGetPreviousMode(),
						IoReadAccess,
						&MappedAddress,
						&LockVariable);
					if (st1 == STATUS_SUCCESS)
					{
						if (0 == memcmp(MappedAddress, "\x89\x34\xB8", 3)) {
							memcpy(MappedAddress, "\x90\x90\x90", 3);
						}
						ExUnlockUserBuffer(LockVariable);
					}
				}
				ExFreePool(ProcessName);
			}
			ObDereferenceObject(Process);
		}
	}
}

extern "C"
{
	_Use_decl_annotations_ static void DriverUnload(
		PDRIVER_OBJECT driver_object) {
		UNREFERENCED_PARAMETER(driver_object);
		PAGED_CODE();
		PsRemoveLoadImageNotifyRoutine(LoadImageNotifyCallback);
	}

	_Use_decl_annotations_ NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object,
		PUNICODE_STRING registry_path) {
		UNREFERENCED_PARAMETER(registry_path);
		PAGED_CODE();

		PsSetLoadImageNotifyRoutine(LoadImageNotifyCallback);
		driver_object->DriverUnload = DriverUnload;

		return STATUS_SUCCESS;
	}
}