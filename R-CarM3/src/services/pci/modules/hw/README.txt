Adding a New HW dependent module

1. Make a directory copy of the closest existing HW dependent module
2. global search the new module fixing up names/architcture strings
3. edit mod_rdwr.c
	* if the hardware does not support ECAM
	- edit the 2 entry points hw_rd() and hw_wr() to directly call the
	  corresponding hw_io_rd() and hw_io_wr() functions and all of the rest can
	  be eliminated.
	* if the hardware does support ECAM,
	- choose and appropriate number of buses to support	for the ARRAY_SZ define
	- edit find_ecam_base() to do something appropriate for the hardware
4. edit mod_io_rdwr.c
	- edit the CONFIG_ADDR_REG and CONFIG_DATA_REG defines to be values
	  appropriate for the hardware
	- edit mmap_lock_address() to use a memory location appropriate for the
	  hardware
	- if required, edit the hw_io_rd() and hw_io_wr() functions to conctruct the
	  value of 'addr' from the BDF and offset parameters. Hopefully these will
	  not need to change
5. edit mod_mem_rdwr.c
	- if the hardware does not support ECAM, this file can be eliminated
	  otherwise hopefully it can be used without change
6. edit chipset_irq.c
	- this file contains 1 entry point bdf_irq() which is called when a PIN
	  based interrupt is requested from the system. This function needs to
	  figure out which IRQ (which will be used in an InterruptAttach() call) is
	  the correct one to use based of the device (identified by BDF)
7. edit load_irqmap_file.c
	- if the HW dependent module will support file based IRQ assignment
	  (recommended) then edit this file to parse the file. The format is HW
	  module dependent so use an existing reference if possible otherwise create
	  your own. 