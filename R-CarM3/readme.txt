=====================================================================
             BSP Release Notes
=====================================================================  
This is a release note for QNX7.0 Beta BSP for R-Car M3 boards

This BSP is distributed under QNX Apache License, version 2.0.

                         Board:  RENESAS R-Car M3 boards
                      BSP file:  R_CarM3_xxxx_qnx70_beta_bsp_yyyy_mm_dd.zip
                       Version:  0.1.0
                  Release Date:  
                        Author:  Renesas Electronics America Inc.
 Software Development Platform:  QNX SDP 7.0 Beta
===================================================================== 

---------------------------------------------------------------------
 Installation 
---------------------------------------------------------------------
Please refer to the BSP Setup Manual for specific instructions on 
using this BSP with the specified target device.      

---------------------------------------------------------------------
 Uninstallation  
---------------------------------------------------------------------

---------------------------------------------------------------------
 QNX7.0 BSP Change History
---------------------------------------------------------------------
    CREATED   : 02.13.2017
    MODIFIED  : 
    BSP CONTENTS  :
    +-- <images>        : where the resultant boot images are places
    +-- <install>       : gets populated at the beginning of the BSP build process
    +-- <prebuilt>      : contains the binaries, system binaries, 
                          buidfiles, libraries, and header files that 
                          are shipped with the BSP
    |   +-- <aarch64le> : 
    |   |   +-- <lib>        : 
    |   |   +-- <etc>        :
    |   |   +-- <usr>        : 
    |   +-- <usr>       : 
    |   |   +-- <include>   : stores header files that are shipped with the BSP
    +-- <src>               : stored the whole source files of BSP
    |   +-- <hardware>      : BSP source files
    |   |   +-- <deva>      : store Audio (SSI, SCU) driver source files
    |   |   +-- <devb>      : store SD/eMMC driver source files
    |   |   +-- <devc>      : stores SCIF and HSCIF driver source files
    |   |   +-- <devnp>     : stores AVB network driver source files
    |   |   +-- <devu>      : stores USB 2.0 Function driver source files
    |   |   +-- <flash>     : stores Flash driver source files
    |   |   +-- <i2c>       : stores I2C driver source files
    |   |   +-- Makefile    : compiling makefile
    |   |   +-- <startup>   : stores Startup source files
    |   |   +-- <support>   : utility programs
    |   |   |   +-- Makefile          : compiling makefile
    |   |   |   +-- <genmac>          : stores MAC address generation utility source files
    |   |   |   +-- <wdtkick-rcar>    : stores Watchdog timer utility source files
    |   +-- <lib>               : drivers libraries and includes   
    |   |   +-- <dma>           : stores SYS-DMAC, Audio-DMAC and PP-DMAC libraries source file
    |   +-- <service>           : test programs    
    |   |   +-- Makefile        : compiling makefile 
    |   |   +-- <pci>           : stores pci server source file    
    |   +-- Makefile    : compiling makefile   
    +-- Makefile        : compiling make file
    +-- source.xml      : some information about BSP, CPU name

    HISTORY:
    02.13.2017 : Ver.0.1.0
    - Supported modules/drivers:
        Startup
        Debug serial driver
        I2C driver
        Network (devnp) driver
        DMAC library
        Audio driver
        SD/eMMC driver
        WDT kick utility
-----------------------------------------------------------------------------
 Known limitations with this release

-----------------------------------------------------------------------------