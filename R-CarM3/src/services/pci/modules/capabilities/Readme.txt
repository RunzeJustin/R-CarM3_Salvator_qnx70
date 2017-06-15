The naming of capability subdirectories in the capabilities/ directory is as
follows

All PCI Capabilities have a 2 digit HEX value (prefixed with 0x) corresponding
to the PCI SIG assigned Capability ID

All PCIe Extended Capabilities have a 4 digit HEX value (prefixed with 0x)
corresponding to the PCI SIG assigned PCIe Extended Capability ID

The standard capability modules are prefixed 'pci_cap-' (ex. pci_cap-0x10.so
corresponds to the PCI Capability for PCIe)

The PCIe extended capability modules are prefixed 'pcie_xcap-'
(ex. pcie_xcap-0x0003.so corresponding to the Device Serial Number Extended
capability ID)

In order to support VID/DID specific modules, an added suffix can be used to
name the directory for a module that corresponds to a specific VID/DID (to handle
things like errata, non-compliance, etc). The suffix should be '-<vid><did>'

For example, a VID/DID = (0x8086/0x1234) specific directory name for the Device
Serial Number PCIe Extended Capability Module would be 0x0003-80861234 which
would result in the creation of a module with the name
'pcie_xcap-0x0003-80861234.so'.

Don't forget to rename the directory specific .mk file as well
 