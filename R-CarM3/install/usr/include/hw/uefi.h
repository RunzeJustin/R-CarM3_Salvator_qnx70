/*
 * $QNXLicenseC:
 * Copyright 2015, QNX Software Systems. 
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"). You 
 * may not reproduce, modify or distribute this software except in 
 * compliance with the License. You may obtain a copy of the License 
 * at: http://www.apache.org/licenses/LICENSE-2.0 
 * 
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as 
 * contributors under the License or as licensors under other terms.  
 * Please review this entire file for other proprietary rights or license 
 * notices, as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */


/*
 * hw/uefi.h:	UEFI manifests
 *

 *
 */

#ifndef _HW_UEFI_H_INCLUDED
#define _HW_UEFI_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#define EFI_SPECIFICATION_MAJOR_REVISION 1
#define EFI_SPECIFICATION_MINOR_REVISION 2

#define EFI_MAKE_REVISION(__maj,__min)	(((__maj) << 16) | (__min))

#if defined(__X86_64__) || defined(__X86__)
	#undef __attribute__
	#define EFIAPI	__attribute__((__ms_abi__))
#else
	#define EFIAPI
#endif


typedef	_Uint16t		_Char16t;
typedef unsigned long	_Uintn;
typedef long			_Intn;
typedef void			*EFI_HANDLE;
typedef void			*EFI_EVENT;
typedef _Uintn			EFI_TPL;
typedef _Uintn			EFI_STATUS;
typedef _Uint64t		EFI_VIRTUAL_ADDRESS;
typedef _Uint64t		EFI_PHYSICAL_ADDRESS;
typedef _Uint8t			EFI_BOOLEAN;

typedef struct {
	_Uint8t		Type;
	_Uint8t		SubType;
	_Uint8t		Length[2];
} EFI_DEVICE_PATH;


typedef enum {
	AllocateAnyPages,
	AllocateMaxAddress,
	AllocateAddress,
	MAx_AllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef struct {
	_Uint32t				Type;
	_Uint32t				Pad;
	EFI_PHYSICAL_ADDRESS	PhysicalStart;
	EFI_VIRTUAL_ADDRESS		VirtualStart;
	_Uint64t				NumberOfPages;
	_Uint64t				Attribute;
} EFI_MEMORY_DESCRIPTOR;


typedef struct {
	_Uint32t	Data1;
	_Uint16t	Data2;
	_Uint16t	Data3;
	_Uint8t		Data4[8];
} EFI_GUID;


typedef struct {
	_Uint64t	Signature;
	_Uint32t	Revision;
	_Uint32t	HeaderSize;
	_Uint32t	CRC32;
	_Uint32t	Reserved;
} EFI_TABLE_HEADER;


typedef struct {
	_Uint16t	Year;
	_Uint8t		Month;
	_Uint8t		Day;
	_Uint8t		Hour;
	_Uint8t		Minute;
	_Uint8t		Second;
	_Uint8t		Pad1;
	_Uint32t	Nanosecond;
	_Int16t		TimeZone;
	_Uint8t		Daylight;
	_Uint8t		Pad2;
} EFI_TIME;

typedef struct {
	_Uint32t	Resolution;
	_Uint32t	Accuracy;
	EFI_BOOLEAN	SetsToZero;
} EFI_TIME_CAPABILITIES;

typedef enum {
	EfiResetCold,
	EfiResetWarm,
	EfiResetShutdown
} EFI_RESET_TYPE;

typedef struct {
	EFI_GUID		CapsuleGuid;
	_Uint32t		HeaderSize;
	_Uint32t		Flags;
	_Uint32t		CapsuleImageSize;
} EFI_CAPSULE_HEADER;

#define EFI_RUNTIME_SERVICES_SIGNATURE	0x56524553544f4f42
#define EFI_RUNTIME_SERVICES_REVISION	EFI_MAKE_REVISION(EFI_SPECIFICATION_MAJOR_REVISION, EFI_SPECIFICATION_MINOR_REVISION)

typedef struct {
	EFI_TABLE_HEADER		Hdr;

	EFI_STATUS				(EFIAPI *GetTime)(EFI_TIME *Time, EFI_TIME_CAPABILITIES *Capabilities);
	EFI_STATUS				(EFIAPI *SetTime)(EFI_TIME *Time);
	EFI_STATUS				(EFIAPI	*GetWakeupTime)(EFI_BOOLEAN *Enabled, EFI_BOOLEAN *Pending, EFI_TIME *Time);
	EFI_STATUS				(EFIAPI *SetWakeupTime)(EFI_BOOLEAN Enabled, EFI_TIME *Time);

	EFI_STATUS				(EFIAPI *SetVirtualAddressMap)(_Uintn MemoryMapSize, _Uintn DescriptorSize, _Uint32t DescriptorVersion, EFI_MEMORY_DESCRIPTOR *VirtualMap);
	EFI_STATUS				(EFIAPI *ConvertPointer)(_Uintn DebugDisposition, void **Address);;

	EFI_STATUS				(EFIAPI *GetVariable)(_Char16t *VariableName, EFI_GUID *VendorGuid, _Uint32t *Attributes, _Uintn *DataSize, void *Data);
	EFI_STATUS				(EFIAPI *GetNextVariableBane)(_Uintn *VariableNameSize, _Char16t *VariableName, EFI_GUID *VendorGuid);
	EFI_STATUS				(EFIAPI *SetVariable)(_Char16t *VariableName, EFI_GUID *VendorGuid, _Uintn DataSize, void *Data);

	EFI_STATUS				(EFIAPI *GetNextHighMonotonicCount)(_Uint32t *HighCount);
	EFI_STATUS				(EFIAPI *ResetSystem)(EFI_RESET_TYPE ResetTyep, EFI_STATUS ResetStatus, _Uintn DataSize, _Char16t *ResetData);

	EFI_STATUS				(EFIAPI *UpdateCapsule)(EFI_CAPSULE_HEADER **CapsuleHeaderArray, _Uintn CapsuleCount, EFI_PHYSICAL_ADDRESS ScatterGatherList);
	EFI_STATUS				(EFIAPI *QueryCapsuleCapabilities)(EFI_CAPSULE_HEADER **CapsuleHeaderArray, _Uintn CapsuleCount, _Uint64t *MaximumCapsuleSize, EFI_RESET_TYPE *ResetType);
	EFI_STATUS				(EFIAPI *QueryVariableInfo)(_Uint32t Attributes, _Uint64t *MaximumVariableStorageSize, _Uint64t *RemainingVariableStorageSize, _Uint64t MaximumVariableSize);
} EFI_RUNTIME_SERVICES;

#define EFI_BOOT_SERVICES_SIGNATURE	0x56524553544f4f42
#define EFI_BOOT_SERVICES_REVISION	EFI_MAKE_REVISION(EFI_SPECIFICATION_MAJOR_REVISION, EFI_SPECIFICATION_MINOR_REVISION)

typedef void (EFIAPI *EFI_EVENT_NOTIFY)(EFI_EVENT Event, void *Context);

typedef enum {
	TimerCancel,
	TimerPeriodic,
	TimerRelative,
	TimerTypeMax
} EFI_TIMER_DELAY;


typedef enum {
	EFI_NATIVE_INTERFACE,
	EFI_PCODE_INTERFACE
} EFI_INTERFACE_TYPE;


typedef enum {
	AllHandles,
	ByRegisterNotify,
	ByProtocol
} EFI_LOCATE_SEARCH_TYPE;


typedef struct {
	EFI_HANDLE	AgentHandle;
	EFI_HANDLE	ControllerHandle;
	_Uint32t	Attributes;
	_Uint32t	OpenCount;
} EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;


typedef struct {
	EFI_TABLE_HEADER		Hdr;

	EFI_TPL					(EFIAPI *RaiseTPL)(EFI_TPL	NewTbl);
	void					(EFIAPI *RestoreTPL)(EFI_TPL OldTpl);

	EFI_STATUS				(EFIAPI *AllocatePages)(EFI_ALLOCATE_TYPE Type, EFI_MEMORY_TYPE MemoryType, _Uintn NoPages, EFI_PHYSICAL_ADDRESS *Memory);
	EFI_STATUS				(EFIAPI *FreePages)(EFI_PHYSICAL_ADDRESS, _Uintn NoPages);
	EFI_STATUS				(EFIAPI *GetMemoryMap)(_Uintn *MemoryMapSize, EFI_MEMORY_DESCRIPTOR *MemoryMap, _Uintn *MapKey, _Uintn *DescriptorSize, _Uint32t *DescriptorVersion);
	EFI_STATUS				(EFIAPI *AllocatePool)(EFI_MEMORY_TYPE PoolType, _Uintn Size, void **Buffer);
	EFI_STATUS				(EFIAPI *FreePool)(void *Buffer);

	EFI_STATUS				(EFIAPI *CreateEvent)(_Uint32t Type, EFI_TPL NotifyTPL, EFI_EVENT_NOTIFY NotifyFunction, void *NotifyContext, EFI_EVENT *Event);
	EFI_STATUS				(EFIAPI *SetTimer)(EFI_EVENT Event, EFI_TIMER_DELAY Type, _Uint64t TriggerTime);
	EFI_STATUS				(EFIAPI *WaitForEvent)(_Uintn NumberOfEvents, EFI_EVENT *Event, _Uintn *Index);
	EFI_STATUS				(EFIAPI *SignalEvent)(EFI_EVENT Event);
	EFI_STATUS				(EFIAPI	*CloseEvent)(EFI_EVENT Event);
	EFI_STATUS				(EFIAPI	*CheckEvent)(EFI_EVENT Event);

	EFI_STATUS				(EFIAPI	*InstallProtocolInterface)(EFI_HANDLE *Handle, EFI_GUID *Protocol, EFI_INTERFACE_TYPE InterfaceType, void *Interface);
	EFI_STATUS				(EFIAPI *ReinstallProtocolInterface)(EFI_HANDLE Handle, EFI_GUID *Protocol, void *OldInterface, void *NewInterface);
	EFI_STATUS				(EFIAPI *UninstallProtocolInterface)(EFI_HANDLE Handle, EFI_GUID *Protocol, void *Interface);
	EFI_STATUS				(EFIAPI *HandleProtocol)(EFI_HANDLE Handle, EFI_GUID *Protocol, void **Interface);
	EFI_STATUS				(EFIAPI *PCHandleProtocol)(EFI_HANDLE Handle, EFI_GUID *Protocol, void **Interface);
	EFI_STATUS				(EFIAPI *RegisterProtocolNotify)(EFI_GUID *Protocol, EFI_EVENT Event, void **Registration);
	EFI_STATUS				(EFIAPI *LocateHandle)(EFI_LOCATE_SEARCH_TYPE SearchType, EFI_GUID *Protocol, void *SearchKey, _Uintn *BufferSize, EFI_HANDLE *Buffer);
	EFI_STATUS				(EFIAPI *LocateDevicePath)(EFI_GUID *Protocol, EFI_DEVICE_PATH **DevicePath, EFI_HANDLE *Device);
	EFI_STATUS				(EFIAPI *InstallConfigurationTable)(EFI_GUID *Guid, void *Table);

	EFI_STATUS				(EFIAPI *LoadImage)(EFI_BOOLEAN BootPolicy, EFI_HANDLE ParentImageHandle, EFI_DEVICE_PATH *FilePath, void *SourceBuffer, _Uintn SourceSize, EFI_HANDLE *ImageHandle);
	EFI_STATUS				(EFIAPI *StartImage)(EFI_HANDLE *ImageHandle, _Uintn *ExitDataSize, _Char16t **ExitData);
	EFI_STATUS				(EFIAPI *Exit)(EFI_HANDLE ImageHandle, EFI_STATUS ExitStatus, _Uintn ExitDataSize, _Char16t *ExitData);
	EFI_STATUS				(EFIAPI *UnloadImage)(EFI_HANDLE ImageHandle);
	EFI_STATUS				(EFIAPI *ExitBootServices)(EFI_HANDLE ImageHandle, _Uintn MapKey);

	EFI_STATUS				(EFIAPI *GetNextMonotonicCount)(_Uint64t *Count);
	EFI_STATUS				(EFIAPI *Stall)(_Uintn Microseconds);
	EFI_STATUS				(EFIAPI *SetWatchdogTimer)(EFI_HANDLE ControllerHandle, _Uint64t WatchdogCode, _Uintn DataSize, _Char16t *WatchdogData);

	EFI_STATUS				(EFIAPI *ConnectController)(EFI_HANDLE ControllerHandle, EFI_HANDLE *DriverImageHandle, EFI_DEVICE_PATH *RemainingDevicePath, EFI_BOOLEAN Recursive);
	EFI_STATUS				(EFIAPI *DisconnectController)(EFI_HANDLE ControllerHandle, EFI_HANDLE DriverImageHandle, EFI_HANDLE ChildHandle);

	EFI_STATUS				(EFIAPI *OpenProtocol)(EFI_HANDLE Handle, EFI_GUID *Protocol, void **Interface, EFI_HANDLE AgentHandle, EFI_HANDLE ControllerHandle, _Uint32t Attributes);
	EFI_STATUS				(EFIAPI *CloseProtocol)(EFI_HANDLE Handle, EFI_GUID *Protocol, EFI_HANDLE AgentHandle, EFI_HANDLE ControllerHandle);
	EFI_STATUS				(EFIAPI *OpenProtocolInformation)(EFI_HANDLE Handle, EFI_GUID *Protocol, EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer, _Uintn *EntryCount);

	EFI_STATUS				(EFIAPI *ProtocolsPerHandle)(EFI_HANDLE Handle, EFI_GUID **ProtocolBuffer, _Uintn *ProtocolBufferCount);
	EFI_STATUS				(EFIAPI *LocateHandleBuffer)(EFI_LOCATE_SEARCH_TYPE SearchType, EFI_GUID *Protocol, void *SearchKey, _Uintn *NoHandles, EFI_HANDLE **Buffer);
	EFI_STATUS				(EFIAPI *LocateProtocol)(EFI_GUID *Protocol, void *Registration, void **Interface);
	EFI_STATUS				(EFIAPI *InstallMultipleProtocolInterfaces)(EFI_HANDLE *Handle, ...);
	EFI_STATUS				(EFIAPI *UninstallMultipleProtocolInterfaces)(EFI_HANDLE Handle, ...);

	EFI_STATUS				(EFIAPI *CalculateCrc32)(void *Data, _Uintn DataSize, _Uint32t *Crc32);
	EFI_STATUS				(EFIAPI *CopyMem)(void *Destination, void *Source, _Uintn Length);
	EFI_STATUS				(EFIAPI *SetMem)(void *Buffer, _Uintn Size, _Uint8t Value);

	EFI_STATUS				(EFIAPI *CreateEventEx)(_Uint32t Type, EFI_TPL NotifyTpl, EFI_EVENT_NOTIFY NotifyFunction, const void *NotifyContext, const EFI_GUID *EventGroup, EFI_EVENT *Event);

} EFI_BOOT_SERVICES;

#define EFI_ACPI_TABLE_GUID \
		{ \
			.Data1 = 0x8868e871, .Data2 = 0xe4f1, .Data3 = 0x11d3, \
			.Data4 = \
			{ \
				[0] = 0xbc, [1] = 0x22, [2] = 0x0, [3] = 0x80, \
				[4] = 0xc7, [5] = 0x3c, [6] = 0x88, [7] = 0x81, \
			}, \
		}
#define ACPI_10_TABLE_GUID \
		{ \
			.Data1 = 0xeb9d2d30, .Data2 = 0x2d88, .Data3 = 0x11d3, \
			.Data4 = \
			{ \
				[0] = 0x9a, [1] = 0x16, [2] = 0x0, [3] = 0x90, \
				[4] = 0x27,[5] = 0x3f, [6] = 0xc1, [7] = 0x4d, \
			}, \
		}

typedef struct {
	EFI_GUID			VendorGuid;
	void				*VendorTable;
} EFI_CONFIGURATION_TABLE;

#define EFI_SYSTEM_TABLE_SIGNATURE	0x5453595320494249
#define EFI_SYSTEM_TABLE_REVISION	EFI_MAKE_REVISION(EFI_SPECIFICATION_MAJOR_REVISION, EFI_SPECIFICATION_MINOR_REVISION)

/* We don't actually use these, so the following are dummy definitions
 * to get things to compile.
 */ 
typedef struct simple_input_interface	SIMPLE_INPUT_INTERFACE;
typedef struct simple_text_output_interface	SIMPLE_TEXT_OUTPUT_INTERFACE;

typedef struct {
	EFI_TABLE_HEADER				Hdr;
	_Char16t						*FirmwareVendor;
	_Uint32t						FirmwareRevision;
	EFI_HANDLE						ConsoleInHandle;
	SIMPLE_INPUT_INTERFACE			*ConIn;
	EFI_HANDLE						ConsoleOutHandle;
	SIMPLE_TEXT_OUTPUT_INTERFACE	*ConOut;
	EFI_HANDLE						StandardErrorHandle;
	SIMPLE_TEXT_OUTPUT_INTERFACE	*StdErr;
	EFI_RUNTIME_SERVICES			*RuntimeServices;
	EFI_BOOT_SERVICES				*BootServices;
	_Uintn							NumberOfTableEntries;
	EFI_CONFIGURATION_TABLE			*ConfigurationTable;
} EFI_SYSTEM_TABLE;


extern EFI_HANDLE		efi_image_handle;
extern EFI_SYSTEM_TABLE	*efi_system_table;

extern void uefi_init(void);
extern void *uefi_find_config_tbl(const EFI_GUID *uid);
#endif

/* __SRCVERSION("acpi.h $Rev: 793185 $"); */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/public/hw/uefi.h $ $Rev: 793185 $")
#endif
