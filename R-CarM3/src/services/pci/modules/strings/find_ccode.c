/*
 * $QNXLicenseC:
 * Copyright (c) 2012, 2016 QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.   Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 * $
 */

/*
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 PCI Class codes descriptions obtained from

 	 "PCI CODE AND ID ASSIGNMENT SPECIFICATION, REV. 1.54", dated Mar 6, 2014

 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pci/pci_strings.h>

#include "strings.h"

typedef struct
{
	pci_ccode_t	ccode;
	const char * const str;
} ccode_str_t;

static ccode_str_t ccode_str[] =
{
	{ PCI_CCODE_VGA_NON_COMPAT, "Pre-2.0 PCI Specification Device Non-VGA" },
	{ PCI_CCODE_VGA_COMPAT, "Pre-2.0 PCI Specification Device VGA Compatible" },

	{ PCI_CCODE_STORAGE_SCSI_VEND_IF, "SCSI Controller (Vendor Specific Interface)" },
	{ PCI_CCODE_STORAGE_SCSI_DEV_PQI, "SCSI Storage Device (PCIe Queuing Interface)" },
	{ PCI_CCODE_STORAGE_SCSI_HBA_PQI, "SCSI Controller (PCIe Queuing Interface)" },
	{ PCI_CCODE_STORAGE_SCSI_DEV_HBA_PQI, "SCSI Device and Controller (PCIe Queuing Interface)" },
	{ PCI_CCODE_STORAGE_SCSI_DEV_NVMe, "SCSI Storage Device (NVMe Interface)" },
	{ PCI_CCODE_STORAGE_IDE, "IDE Controller (ANSI INCITS370-2004)" },
	{ PCI_CCODE_STORAGE_FLOPPY, "Floppy Disk Controller (Vendor Specific Interface)" },
	{ PCI_CCODE_STORAGE_IPI, "IPI Bus Controller (Vendor Specific Interface)" },
	{ PCI_CCODE_STORAGE_RAID, "RAID Controller (Vendor Specific Interface)" },
	{ PCI_CCODE_STORAGE_ATA_SINGLE_STEP, "ATA single DMA Mass Storage Controller" },
	{ PCI_CCODE_STORAGE_ATA_CONTINUOUS, "ATA chained DMA Mass Storage Controller" },
	{ PCI_CCODE_STORAGE_SATA_VEND_IF, "SATA Mass Storage Controller (Vendor Specific Interface)" },
	{ PCI_CCODE_STORAGE_SATA_AHCI_IF, "SATA Mass Storage Controller (AHCI Interface)" },
	{ PCI_CCODE_STORAGE_SATA_SERIAL_IF, "SATA Mass Storage Controller (Serial Interface)" },
	{ PCI_CCODE_STORAGE_SAS, "SAS Mass Storage Controller" },
	{ PCI_CCODE_STORAGE_SAS_SERIAL_IF, "SAS Mass Storage Controller (Serial Interface)" },
	{ PCI_CCODE_STORAGE_SSD_VEND_IF, "Non-volatile Memory Subsystem (Vendor Specific Interface)" },
	{ PCI_CCODE_STORAGE_SSD_NVMHCI, "Non-volatile Memory Subsystem (NVMHCI)" },
	{ PCI_CCODE_STORAGE_SSD_NVMe, "Non-volatile Memory Subsystem (NVMe Interface)" },
	{ PCI_CCODE_STORAGE_UFS_VEND_IF, "Universal Flash Storage Controller (Vendor Specific Interface)" },
	{ PCI_CCODE_STORAGE_UFS_UFSHCI_JESD223a, "Universal Flash Storage Controller (UFSHCI)" },
	{ PCI_CCODE_STORAGE_OTHER, "Other Mass Storage Controller" },

	{ PCI_CCODE_NET_ETHERNET, "Ethernet Network Controller" },
	{ PCI_CCODE_NET_TOKEN_RING, "Token Ring Network Controller" },
	{ PCI_CCODE_NET_FDDI, "FDDI Network Controller" },
	{ PCI_CCODE_NET_ATM, "ATM Network Controller" },
	{ PCI_CCODE_NET_ISDN, "ISDN Network Controller" },
	{ PCI_CCODE_NET_WORLDFIP, "WorldFip Network Controller" },
	{ PCI_CCODE_NET_PICMG_2_14_MULTI, "PICMG 2.14 Multi Computing Network Controller" },
	{ PCI_CCODE_NET_INFINIBAND, "InfiniBand Network Controller" },
	{ PCI_CCODE_NET_OTHER, "Other Network Controller" },

	{ PCI_CCODE_DISPLAY_VGA_COMPAT, "PC Compatible VGA Display Controller" },
	{ PCI_CCODE_DISPLAY_8514_COMPAT, "PC Compatible 8514 Display Controller" },
	{ PCI_CCODE_DISPLAY_XGA, "XGA Display Controller" },
	{ PCI_CCODE_DISPLAY_3D, "3D Display Controller" },
	{ PCI_CCODE_DISPLAY_OTHER, "Other Display Controller" },

	{ PCI_CCODE_MMEDIA_VIDEO, "Video Multi-media Device" },
	{ PCI_CCODE_MMEDIA_AUDIO, "Audio Multi-media Device" },
	{ PCI_CCODE_MMEDIA_TELEPHONY, "Telephony Multi-media Device" },
	{ PCI_CCODE_MMEDIA_MIXED_MODE, "Mixed Mode Multi-media Device" },
	{ PCI_CCODE_MMEDIA_OTHER, "Other Multi-media Device" },

	{ PCI_CCODE_MEMORY_RAM, "RAM Memory Controller" },
	{ PCI_CCODE_MEMORY_FLASH, "Flash Memory Controller" },
	{ PCI_CCODE_MEMORY_OTHER, "Other Memory Controller" },

	{ PCI_CCODE_BRIDGE_HOST, "Host-to-PCI Bridge Device" },
	{ PCI_CCODE_BRIDGE_ISA, "PCI-to-ISA Bridge Device" },
	{ PCI_CCODE_BRIDGE_EISA, "PCI-to-EISA Bridge Device" },
	{ PCI_CCODE_BRIDGE_MCA, "PCI-to-Micro Channel Bridge Device" },
	{ PCI_CCODE_BRIDGE_PCItoPCI, "PCI-to-PCI Bridge Device" },
	{ PCI_CCODE_BRIDGE_PCItoPCIsub, "PCI-to-PCI Bridge Device (+ Subtractive Decode)" },
	{ PCI_CCODE_BRIDGE_PCMCIA, "PCI-to-PCMCIA Bridge Device" },
	{ PCI_CCODE_BRIDGE_NuBUS, "PCI-to-NuBus Bridge Device" },
	{ PCI_CCODE_BRIDGE_CARDBUS, "PCI-to-CardBus Bridge Device" },
	{ PCI_CCODE_BRIDGE_RACEWAY_TRANS, "PCI-to-Raceway Bridge Device (Transparent Mode)" },
	{ PCI_CCODE_BRIDGE_RACEWAY_ENDP, "PCI-to-Raceway Bridge Device (Endpoint Mode)" },
	{ PCI_CCODE_BRIDGE_PCItoPCI_PRICPU, "PCI-to-PCI Bridge Device (Host CPU on Primary)" },
	{ PCI_CCODE_BRIDGE_PCItoPCI_SECCPU, "PCI-to-PCI Bridge Device (Host CPU on Secondary)" },
	{ PCI_CCODE_BRIDGE_INFIBANDtoPCI, "InfiniBand-to-PCI Host Bridge Device" },
	{ PCI_CCODE_BRIDGE_ADVSW_HOST, "HOST Bridge Device (Advanced Switching, Custom I/F))" },
	{ PCI_CCODE_BRIDGE_ADVSW_HOST_ASISIG, "HOST Bridge Device (Advanced Switching, ASI-SIG I/F)" },
	{ PCI_CCODE_BRIDGE_OTHER, "Other Bridge Device" },

	{ PCI_CCODE_COMM_UART_XT, "Generic XT Compatible Simple Communications Controller (UART)" },
	{ PCI_CCODE_COMM_UART_16450, "16450 Compatible Simple Communications Controller (UART)" },
	{ PCI_CCODE_COMM_UART_16550, "16550 Compatible Simple Communications Controller (UART)" },
	{ PCI_CCODE_COMM_UART_16650, "16650 Compatible Simple Communications Controller (UART)" },
	{ PCI_CCODE_COMM_UART_16750, "16750 Compatible Simple Communications Controller (UART)" },
	{ PCI_CCODE_COMM_UART_16850, "16850 Compatible Simple Communications Controller (UART)" },
	{ PCI_CCODE_COMM_UART_16950, "16950 Compatible Simple Communications Controller (UART)" },
	{ PCI_CCODE_COMM_PARALLEL_PORT, "Parallel Port Simple Communications Controller" },
	{ PCI_CCODE_COMM_PARALLEL_PORT_BI, "Bidirectional Parallel Port Simple Communications Controller" },
	{ PCI_CCODE_COMM_PARALLEL_ECP_1_X, "ECP 1.X Compliant Parallel Port Simple Communications Controller" },
	{ PCI_CCODE_COMM_PARALLEL_IEEE1284_C, "IEEE 1284 Simple Communications Controller" },
	{ PCI_CCODE_COMM_PARALLEL_IEEE1284_C, "IEEE 1284 Simple Communications Controller (Target Device)" },
	{ PCI_CCODE_COMM_MULTIPORT_SERIAL, "Multi-port Simple Communications Controller" },
	{ PCI_CCODE_COMM_MODEM_GENERIC, "Generic Modem Simple Communications Controller" },
	{ PCI_CCODE_COMM_MODEM_HAYES_16450, "Hayes 16450 Modem Simple Communications Controller" },
	{ PCI_CCODE_COMM_MODEM_HAYES_16550, "Hayes 16550 Modem Simple Communications Controller" },
	{ PCI_CCODE_COMM_MODEM_HAYES_16650, "Hayes 16650 Modem Simple Communications Controller" },
	{ PCI_CCODE_COMM_MODEM_HAYES_16750, "Hayes 16750 Modem Simple Communications Controller" },
	{ PCI_CCODE_COMM_GPIB_IEEE488, "IEEE 488 Simple Communications Controller (GPIB)" },
	{ PCI_CCODE_COMM_SMARTCARD, "Smartcard Simple Communications Controller (GPIB)" },
	{ PCI_CCODE_COMM_OTHER, "Other Simple Communications Controller" },

	{ PCI_CCODE_PERIPHERAL_PIC_8259, "Generic 8259 PIC Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_PIC_ISA, "ISA PIC Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_PIC_EISA, "EISA PIC Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_PIC_IOAPIC, "IO-APIC Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_PIC_IOxAPIC, "IOx-APIC Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_DMA_8237, "Generic 8237 DMA Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_DMA_ISA, "ISA DMA Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_DMA_EISA, "EISA DMA Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_TIMER_8254, "Generic 8254 Timer Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_TIMER_ISA, "ISA Timer Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_TIMER_EISA, "EISA Timer Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_TIMER_HPET, "HPET Timer Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_RTC_GENERIC, "Generic RTC Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_RTC_ISA, "ISA RTC Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_PCI_HPLUG_CTRL, "PCI Hot-Plug Controller Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_SD_HOST_CTRL, "SD Host Controller Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_IOMMU, "IOMMU Base Systems Peripheral" },
	{ PCI_CCODE_PERIPHERAL_RC_EVTCOLLECT, "Root Complex Event Collector" },
	{ PCI_CCODE_PERIPHERAL_OTHER, "Other Base Systems Peripheral" },

	{ PCI_CCODE_INPUT_KEYBOARD, "Keyboard Input Device" },
	{ PCI_CCODE_INPUT_PEN, "Pen Input Device" },
	{ PCI_CCODE_INPUT_MOUSE, "Mouse Input Device" },
	{ PCI_CCODE_INPUT_SCANNER, "Scanner Input Device" },
	{ PCI_CCODE_INPUT_GAMEPORT_GENERIC, "Gameport Input Device (Generic)" },
	{ PCI_CCODE_INPUT_GAMEPORT, "Gameport Input Device" },
	{ PCI_CCODE_INPUT_OTHER, "Other Input Device" },

	{ PCI_CCODE_DOCK_GENERIC, "Generic Docking Station" },
	{ PCI_CCODE_DOCK_OTHER, "Other Docking Station" },

	{ PCI_CCODE_PROCESSOR_386, "i386 Processor" },
	{ PCI_CCODE_PROCESSOR_486, "i486 Processor" },
	{ PCI_CCODE_PROCESSOR_PENTIUM, "Pentium Processor" },
	{ PCI_CCODE_PROCESSOR_ALPHA, "Alpha Processor" },
	{ PCI_CCODE_PROCESSOR_PPC, "PowerPC Processor" },
	{ PCI_CCODE_PROCESSOR_MIPS, "MIPS Processor" },
	{ PCI_CCODE_PROCESSOR_COPROCESSOR, "Co-Processor" },
	{ PCI_CCODE_PROCESSOR_OTHER, "Other Processor" },

	{ PCI_CCODE_SBUS_IEEE1394, "IEEE 1394 Serial Bus Controller (Firewire)" },
	{ PCI_CCODE_SBUS_IEEE1394_OPEN_HCI, "IEEE 1394 Serial Bus Controller (Open HCI)" },
	{ PCI_CCODE_SBUS_ACCESS_BUS, "ACCESS.bus Serial Bus Controller" },
	{ PCI_CCODE_SBUS_SSA, "SSA Serial Bus Controller (Serial Storage Architecture)" },
	{ PCI_CCODE_SBUS_USB_XHCI, "USB Serial Bus Controller (Intel eXtensible HCI)" },
	{ PCI_CCODE_SBUS_USB_EHCI, "USB Serial Bus Controller (EHCI)" },
	{ PCI_CCODE_SBUS_USB_OHCI, "USB Serial Bus Controller (OHCI)" },
	{ PCI_CCODE_SBUS_USB_UHCI, "USB Serial Bus Controller (UHCI)" },
	{ PCI_CCODE_SBUS_USB_NON_SPECIFIC, "USB Serial Bus Controller (Non Specifc I/F)" },
	{ PCI_CCODE_SBUS_USB_DEVICE, "USB Serial Bus Controller (Target Device)" },
	{ PCI_CCODE_SBUS_FIBRE_CHAN, "Fibre Channel Serial Bus Controller" },
	{ PCI_CCODE_SBUS_SMBUS, "SMBus Serial Bus Controller" },
	{ PCI_CCODE_SBUS_INFINIBAND, "InfiniBand Serial Bus Controller" },
	{ PCI_CCODE_SBUS_IPMI_SMIC, "IPMI SMIC I/F Serial Bus Controller" },
	{ PCI_CCODE_SBUS_IPMI_KEYBOARD_STYLE, "IPMI Keyboard Style I/F Serial Bus Controller" },
	{ PCI_CCODE_SBUS_IPMI_BLOCK_XFER, "IPMI Block Xfer I/F Serial Bus Controller" },
	{ PCI_CCODE_SBUS_SERCOS, "SERCOS Serial Bus Controller (IEC 61491)" },
	{ PCI_CCODE_SBUS_CANBUS, "CANbus Serial Bus Controller" },
	{ PCI_CCODE_SBUS_OTHER, "Other Serial Bus Controller" },

	{ PCI_CCODE_WIRELESS_IRDA, "IRDA Wireless Controller" },
	{ PCI_CCODE_WIRELESS_CONSUMER_IR, "Consumer IR Wireless Controller" },
	{ PCI_CCODE_WIRELESS_UWB, "UWB Wireless Controller" },
	{ PCI_CCODE_WIRELESS_RF, "RF Wireless Controller" },
	{ PCI_CCODE_WIRELESS_BLUETOOTH, "Bluetooth Wireless Controller" },
	{ PCI_CCODE_WIRELESS_BROADBAND, "Broadband Wireless Controller" },
	{ PCI_CCODE_WIRELESS_ENET_802_11A, "Ethernet 802.11a Wireless Controller (5 GHz)" },
	{ PCI_CCODE_WIRELESS_ENET_802_11B, "Ethernet 802.11b Wireless Controller (2.4 GHz)" },
	{ PCI_CCODE_WIRELESS_OTHER, "IRDA Wireless Controller" },

	{ PCI_CCODE_SMARTIO_I2O_1_0, "Intelligent I/O Controller (I2O v1.0)" },
	{ PCI_CCODE_SMARTIO_MSG_FIFO_OFF_40, "Intelligent I/O Controller (FIFO)" },

	{ PCI_CCODE_SATCOMM_TV, "TV Satellite Communication Controller" },
	{ PCI_CCODE_SATCOMM_AUDIO, "Audio Satellite Communication Controller" },
	{ PCI_CCODE_SATCOMM_VOICE, "Voice Satellite Communication Controller" },
	{ PCI_CCODE_SATCOMM_DATA, "Data Satellite Communication Controller" },
	{ PCI_CCODE_SATCOMM_OTHER, "Other Satellite Communication Controller" },

	{ PCI_CCODE_CRYPT_NET_CPU, "Network and Computing Encryption/Decryption Controller" },
	{ PCI_CCODE_CRYPT_ENTERTAINMENT, "Entertainment Device Encryption/Decryption Controller" },
	{ PCI_CCODE_CRYPT_OTHER, "Other Encryption/Decryption Controller" },

	{ PCI_CCODE_DA_DSP_DPIO, "DPIO DA/DSP Controller" },
	{ PCI_CCODE_DA_DSP_PERF_COUNTER, "Performance Counter DA/DSP Controller" },
	{ PCI_CCODE_DA_DSP_SYNC_and_TF_MEAS, "Time Sync & T/F Measurement DA/DSP Controller" },
	{ PCI_CCODE_DA_DSP_MGMT_CARD, "Management DA/DSP Controller" },
	{ PCI_CCODE_DA_DSP_OTHER, "Other DA/DSP Controller" },

	{ PCI_CCODE_ACCELERATOR, "Processing Accelerator (Vendor Specific Interface)" },

	{ PCI_CCODE_NONESSENTIAL_INSTRUMENTATION_VEND_IF, "Non-essential Instrumentation Function (Vendor Specific Interface)" },

	{ PCI_CCODE(0xFF,0x00,0x00), "Unknown Class code"},
};

/*
 ===============================================================================
 pci_strings_find_ccode

 If the need ever arises, we could probably speed up the lookup by utilizing an
 array of variable sized arrays (indexed by class code) limiting the linear
 search to only the subclass/reg interface (null entry terminated). Since these
 strings are only meant for tools and utilities though, its not pressing.
 
*/
const char *pci_strings_find_ccode(const pci_ccode_t ccode)
{
	uint_t i;
	for (i=0; i<NELEMENTS(ccode_str); i++)
	{
		if (ccode == ccode_str[i].ccode) return ccode_str[i].str;
	}
	/* if not found, run the list again, this time ignoring the Programming Interface */
	for (i=0; i<NELEMENTS(ccode_str); i++)
	{
		if ((PCI_CCODE_CLASS(ccode) == PCI_CCODE_CLASS(ccode_str[i].ccode)) &&
			(PCI_CCODE_SUBCLASS(ccode) == PCI_CCODE_SUBCLASS(ccode_str[i].ccode)))
		{
			return ccode_str[i].str;
		}
	}
	return "<Classcode is undefined>";
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/strings/find_ccode.c $ $Rev: 811591 $")
#endif
