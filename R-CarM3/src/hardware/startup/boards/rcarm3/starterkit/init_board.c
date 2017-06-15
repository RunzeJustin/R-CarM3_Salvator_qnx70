/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
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

#include "startup.h"
#include <errno.h>
#include <arm/r-car-m3.h>

extern int rcar_usb3_dl_fw(uint32_t regs);

uint32_t rcar_detect_ram(uint32_t channel)
{
    uint32_t config, capacity;
    uint8_t row, col, density;

    config = in32(RCAR_DBSC4_BASE + channel);
    row = config >> 24 & 0x1F;
    if(row < 12 || row > 16){
        kprintf("Invalid row address bit width\n");
        return 0;
    }

    col = config >> 8 & 0x0F;
    if(col < 10 || col > 12){
        kprintf("Invalid col address bit width\n");
        return 0;
    }

    density = config >> 31 & 0x03;
    if(density > 1){
        kprintf("Invalid memory density type\n");
        return 0;
    }

    /*
     * R-Car M3 SDRAM Calculation
     * (Refer to Section 21.3.5 Address Configuration of R-Car Series, 3rd Gen Hardware Manual)
     * Capacity(bytes) = banks * row * column * 32-bit External Bus / 8
     */
    capacity = 1 << (row + col + 5);

    if(density) {
        capacity >>= 2;
        capacity *= 3;
    }

    // Max 2GB is supported
    if(capacity > 0x80000000){
        kprintf("Invalid size\n");
        return 0;
    }

    return capacity;
}

static void rcar_pfc_config(int reg, uint32_t val)
{
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~val);
    out32(RCAR_PFC_BASE + reg, val);
}

static void rcar_cpg_config(int reg, uint32_t val)
{
    out32(RCAR_CPG_BASE + RCAR_CPG_CPGWPR, ~val);
    out32(RCAR_CPG_BASE + reg, val);
}

void init_display(void)
{
    uint32_t tmp;

    /* Power ON A3VP: Video Signal Processor (VSP2),
    Fine Display Processor (FDP1), FCP_V, FCP_F,
    and bus modules for those modules */
    out32 (RCAR_SYSC_BASE + RCAR_SYSC_PWRONCR8, 1);
    mdelay(20000);

    /* Power ON A3VC: IMR-LX4, FCP_C shared (265+Legacy_Full),
    FCP_C independent, Stream Buffer for iVDP1C (STB),
    and IPMMU (VC) modules. */
    out32 (RCAR_SYSC_BASE + RCAR_SYSC_PWRONCR9, 1);   /* a3vc */
    mdelay(20000);

    /* Disable module standby */
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR7);
    tmp &= ~((1 << 29) | /* HDMI0 */
             (1 << 28) | /* HDMI1 */
             (1 << 27) | /* LVDS */
             (1 << 24) | /* DU0 */
             (1 << 23) | /* DU1 */
             (1 << 22) | /* DU2 */
             (1 << 21)); /* DU3 */
    rcar_cpg_config (RCAR_CPG_SMSTPCR7, tmp);
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR6);
    tmp &= ~((1 << 31) | /* VSPI0 */
             (1 << 30) | /* VSPI1 */
             (1 << 29) | /* VSPI2 */
             (1 << 26) | /* VSPBD */
             (1 << 24) | /* VSPBC */
             (1 << 23) | /* DU0 */
             (1 << 22) | /* DU1 */
             (1 << 21) | /* DU2 */
             (1 << 20) | /* DU3 */
             (1 << 19) | /* FCPC_S */
             (1 << 17) | /* FCPC_iVDP0 */
             (1 << 16) | /* FCPC_iVDP1 */
             (1 << 15) | /* FCPF0 */
             (1 << 14) | /* FCPF1 */
             (1 << 13) | /* FCPF2 */
             (1 << 11) | /* FCPV (IMG0) */
             (1 << 10) | /* FCPV (IMG1) */
             (1 << 9) |  /* FCPV (IMG2) */
             (1 << 7) |  /* FCPV (BLD0) */
             (1 << 6) |  /* FCPV (BLD1) */
             (1 << 3) |  /* FCPV (DU0) */
             (1 << 2) |  /* FCPV (DU1) */
             (1 << 1) |  /* FCPV (DU2) */
             (1 << 0));   /* FCPV (DU3) */
    rcar_cpg_config (RCAR_CPG_SMSTPCR6, tmp);

    // Set clock rate and enable clock supplying for SDHI
    rcar_cpg_config(RCAR_CPG_HDMICKCR, 0x0000001F);
    
    /* HDMI */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR7);
    tmp |= (1 << 2);         /* HDMI0_CEC */

    rcar_pfc_config (RCAR_PFC_GPSR7, tmp);

    /* DPAD drive ability */
    rcar_pfc_config(RCAR_PFC_DRVCTRL5, 0x33333333); /* DB[7..4] */
    rcar_pfc_config(RCAR_PFC_DRVCTRL6, 0x33333333); /* DG[7..0] */
    rcar_pfc_config(RCAR_PFC_DRVCTRL7, 0x33333333); /* DR[5..0] */
    rcar_pfc_config(RCAR_PFC_DRVCTRL10, 0x33333333); /* DR[7..6] */
    rcar_pfc_config(RCAR_PFC_DRVCTRL11, 0x33333333);
}

void init_ethernet(void)
{
    uint32_t dwValue;

    rcar_cpg_config(RCAR_CPG_SMSTPCR8, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR8) & ~(1<<12));

    dwValue = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR2);
    dwValue |= ( (1<<9) | (1<<10) | (1<11) | (1<<12) | (1<13) | (1<<14));
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~dwValue);
    out32(RCAR_PFC_BASE + RCAR_PFC_GPSR2, dwValue);

    dwValue  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR0);
    dwValue &= ~(0x0000ffff);
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~dwValue);
    out32(RCAR_PFC_BASE + RCAR_PFC_IPSR0, dwValue);

    dwValue = in32(RCAR_PFC_BASE + RCAR_PFC_PUEN0) & ~(0xFFFF8000); // Pull-up/down function is disabled : bit[15-31]
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~dwValue);
    out32(RCAR_PFC_BASE + RCAR_PFC_PUEN0, dwValue);

    dwValue = in32(RCAR_PFC_BASE + RCAR_PFC_PUEN1) & ~(0x1); // Pull-up/down function is disabled
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~dwValue);
    out32(RCAR_PFC_BASE + RCAR_PFC_PUEN1, dwValue);

    dwValue = in32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL2);
    dwValue &= ~(0x00000777); // clear bit
    dwValue |=  (0x00000333);
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~dwValue);
    out32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL2, dwValue);

    dwValue = in32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL3);
    dwValue &= ~(0x77700000); // clear bit
    dwValue |=  (0x33300000);
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~dwValue);
    out32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL3, dwValue);

    /* Request GP 2-10 as GPIO pin */
    dwValue = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR2);
    dwValue &=  ~(1 << 10); //GP 2-10 AVB_PHY_RESET as GPIO
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~dwValue);
    out32(RCAR_PFC_BASE + RCAR_PFC_GPSR2, dwValue);

    /*General i/o*/
    out32(RCAR_GPIO2_BASE + RCAR_GPIO_IOINTSEL, in32(RCAR_GPIO2_BASE + RCAR_GPIO_IOINTSEL) &  ~(1<<27) );
    /*out direction*/
    out32(RCAR_GPIO2_BASE + RCAR_GPIO_INOUTSEL, in32(RCAR_GPIO2_BASE + RCAR_GPIO_INOUTSEL) |   (1<<27) );
    /* set pin to reset PHY */
    out32(RCAR_GPIO2_BASE + RCAR_GPIO_OUTDT, in32(RCAR_GPIO2_BASE + RCAR_GPIO_OUTDT) |   (1<<27) );
}

static uint32_t usb_base_address[] = {
    RCAR_USB0_BASE,	/* USB2.0 ch0 */
    RCAR_USB1_BASE,	/* USB2.0 ch1 */
    RCAR_USB2_BASE,	/* USB2.0 ch2 */
};

void mdelay(int i)
{
    volatile int cnt = i;
    while (cnt--);
}

void init_usb(int channel)
{
    uint32_t base;
    uint32_t tmp_val;

    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR7, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR7) & ~(1<<4) & ~(1<<3) & ~(1<<2) & ~(1<<1));

    if ( channel == 0)
    {
        tmp_val = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR6) | (1<<24);
        tmp_val &=  ~(1 <<24); //GP 6-24 PWEN as GPIO
        out32(RCAR_PFC_BASE + RCAR_PFC_PMMR,~tmp_val);
        out32(RCAR_PFC_BASE + RCAR_PFC_GPSR6,tmp_val);

        out32((RCAR_GPIO6_BASE + RCAR_GPIO_IOINTSEL), in32(RCAR_GPIO6_BASE + RCAR_GPIO_IOINTSEL) & ~(1 <<24)); //gpio mode
        out32((RCAR_GPIO6_BASE + RCAR_GPIO_INOUTSEL), in32(RCAR_GPIO6_BASE + RCAR_GPIO_INOUTSEL) | (1 <<24)); //output

        out32((RCAR_GPIO6_BASE + RCAR_GPIO_OUTDT), in32(RCAR_GPIO6_BASE + RCAR_GPIO_OUTDT) | (1 <<24)); //output 1, PWEN

        tmp_val = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR6) | (1<<25);
        tmp_val &=  ~(1 <<25); //GP 6-24 OVC GPIO
        out32(RCAR_PFC_BASE + RCAR_PFC_PMMR,~tmp_val);
        out32(RCAR_PFC_BASE + RCAR_PFC_GPSR6,tmp_val);

        out32((RCAR_GPIO6_BASE + RCAR_GPIO_IOINTSEL), in32(RCAR_GPIO6_BASE + RCAR_GPIO_IOINTSEL) & ~(1 <<25)); //gpio mode
        out32((RCAR_GPIO6_BASE + RCAR_GPIO_INOUTSEL), in32(RCAR_GPIO6_BASE + RCAR_GPIO_INOUTSEL) | (1 <<25)); //output

        out32((RCAR_GPIO6_BASE + RCAR_GPIO_OUTDT), in32(RCAR_GPIO6_BASE + RCAR_GPIO_OUTDT) | (1 <<25)); //output 1, OVC
    }

    base = usb_base_address[channel];
    mdelay(1000);
    out32(base + RCAR_USB_AHB_INT_ENABLE, in32(base + RCAR_USB_AHB_INT_ENABLE) | RCAR_USB_AHB_USBH_INTBEN | RCAR_USB_AHB_USBH_INTAEN);

    out32(base + RCAR_USB_CORE_SPD_RSM_TIMSET, 0x014e029b);
    out32(base + RCAR_USB_CORE_OC_TIMSET, 0x000209ab);

    /* Choice USB0SEL */
    out32(RCAR_HSUSB_REG_UGCTRL2, in32(RCAR_HSUSB_REG_UGCTRL2) & ~RCAR_HSUSB_USB0SEL);
    out32(RCAR_HSUSB_REG_UGCTRL2, in32(RCAR_HSUSB_REG_UGCTRL2) | RCAR_HSUSB_USB0SEL_EHCI);

    /* MSTP setting for USB-DMAC0 and USB-DMAC1 */
    tmp_val  = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR3);
    tmp_val &= ~((1 << 30) | (1 << 31));
    rcar_cpg_config(RCAR_CPG_SMSTPCR3, tmp_val);

    /* Clock & Reset */
    out32(base + RCAR_USB_AHB_USBCTR, in32(base + RCAR_USB_AHB_USBCTR) & ~RCAR_USB_AHB_PLL_RST);

    /* low power status */
    out16(RCAR_HSUSB_REG_LPSTS, in16(RCAR_HSUSB_REG_LPSTS) & ~RCAR_HSUSB_SUSPM);
    out16(RCAR_HSUSB_REG_LPSTS, in16(RCAR_HSUSB_REG_LPSTS) | RCAR_HSUSB_SUSPM_NORMAL);

    /* Init usb3.0 */
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR3, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR3) & ~(1<<28) & ~(1<<27));
}

static uint32_t usb3_base_address[] = {
    RCAR_USB30_BASE,	/* USB3.0 ch0 */
    RCAR_USB31_BASE,	/* USB3.0 ch1 */
};

void init_usb3(int channel)
{
    uint32_t base = usb3_base_address[channel];

    int rc = rcar_usb3_dl_fw(base);
    if ( rc == EOK ) {
        mdelay(10000);
        /* Interrupt Enable */
        out32(base + RCAR_USB3_INT_ENA, in32(base + RCAR_USB3_INT_ENA) | RCAR_USB3_INT_ENA_VAL);
        /* LCLK Select */
        out32(base + RCAR_USB3_LCLK, RCAR_USB3_LCLK_ENA_VAL);
        /* USB3.0 Configuration */
        out32(base + RCAR_USB3_CONF1, RCAR_USB3_CONF1_VAL);
        out32(base + RCAR_USB3_CONF2, RCAR_USB3_CONF2_VAL);
        out32(base + RCAR_USB3_CONF3, RCAR_USB3_CONF3_VAL);
        /* USB3.0 Polarity */
        out32(base + RCAR_USB3_RX_POL, RCAR_USB3_RX_POL_VAL);
        out32(base + RCAR_USB3_TX_POL, RCAR_USB3_TX_POL_VAL);
    }
}

void init_msiof(void)
{
    /* M3 Salvator-X board also has MSIOF1 at module _A, or _C, connected to [EXIO_B]
     * MSIOF1 is disabled by default due to the conflict with other function (I2S, SSI..etc).
     * To enable MSIOF1:
     *      Uncomment define USE_MSIOF1 to enable MSIOF1
     *      Uncomment define USE_MSIOF1_C to use MSIOF1_C pins
     */
//#define USE_MSIOF1
//#define USE_MSIOF1_C


    uint32_t  tmp;
    /* Software reset MSIOF0*/
    out32(RCAR_CPG_BASE + RCAR_CPG_SRCR2, in32(RCAR_CPG_BASE + RCAR_CPG_SRCR2) | (1 << 11));
    out32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR2, in32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR2) | (1 << 11));

    /* Supply clock to MSIOF0 */
    out32(RCAR_CPG_BASE + RCAR_CPG_RMSTPCR2, in32(RCAR_CPG_BASE + RCAR_CPG_RMSTPCR2) & ~(1 << 11));
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR2, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR2) & ~(1 << 11));

    /*Setup PFC for MSIOF0
     * GP5-17 -> MSIOF0_SCK
     * GP5-18 -> MSIOF0_SYNC
     * GP5-20 -> MSIOF0_TXD
     * GP5-22 -> MSIOF0_RXD
     * */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR5);
    tmp |= (0x03<<17); // set bit 17,18
    tmp |= (0x05<<20); // set bit 20, 22
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_GPSR5, tmp);


#ifdef USE_MSIOF1
    /* Software reset MSIOF1*/
    out32(RCAR_CPG_BASE + RCAR_CPG_SRCR2, in32(RCAR_CPG_BASE + RCAR_CPG_SRCR2) | (1 << 11));
    out32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR2, in32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR2) | (1 << 11));

    /* Supply clock to MSIOF1 */
    out32(RCAR_CPG_BASE + RCAR_CPG_RMSTPCR2, in32(RCAR_CPG_BASE + RCAR_CPG_RMSTPCR2) & ~(1 << 11));
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR2, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR2) & ~(1 << 11));
#ifndef USE_MSIOF1_C
    /* Setup PFC for MSIOF1_A
     * GP6-7, IPSR14[19:16]=2 -> MSIOF1_TXD_A
     * GP6-8, IPSR14[23:20]=2 -> MSIOF1_SCK_A
     * GP6-9, IPSR14[27:24]=2 -> MSIOF1_SYNC_A
     * GP6-10,IPSR14[31:28]=2 -> MSIOF1_RXD_A
     */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR6);
    tmp |= (0xF<<7); // set bit 7,8,9,10
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_GPSR6, tmp);

    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR12);
    tmp &= ~(0xFFFF << 16); // Clear IPSR12[16:31]
    tmp |= (0x2222 << 16);  // Peripheral Function Select
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_IPSR12, tmp);
#else
    /* Setup PFC for MSIOF1_C
     * GP6-17, IPSR15[15:12]=2 -> MSIOF1_SCK_C
     * GP6-18, IPSR15[19:16]=2 -> MSIOF1_SYNC_C
     * GP6-19, IPSR15[23:20]=2 -> MSIOF1_RXD_C
     * GP6-20, IPSR15[27:24]=2 -> MSIOF1_TXD_C
     */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR6);
    tmp |= (0xF<<17); // set bit 17,18,19,20
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_GPSR6, tmp);

    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR15);
    tmp &= ~(0xFFFF << 12); // Clear IPSR12[12:27]
    tmp |= (0x2222 << 12);  // Peripheral Function Select
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_IPSR15, tmp);
#endif

    /* Module select, sel_msiof1[0:2] = MOD_SEL0[24:26] */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_MODSEL);
    tmp &= ~(7 << 24);
#ifdef USE_MSIOF1_C
    tmp |= ~(2 << 24);
#endif
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_MODSEL, tmp);
#endif

}

void init_i2c(void)
{
    uint32_t tmp;

    out32(RCAR_CPG_BASE + RCAR_CPG_SRCR9, in32(RCAR_CPG_BASE + RCAR_CPG_SRCR9) | (1 << 18) |(1 << 19) |(1 << 27) |(1 << 28) | (1 << 29) | (1 << 30) | (1 << 31));
    out32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR9, in32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR9) | (1 << 18) |(1 << 19) |(1 << 27) |(1 << 28) | (1 << 29) | (1 << 30) | (1 << 31));

    out32(RCAR_CPG_BASE + RCAR_CPG_RMSTPCR9, in32(RCAR_CPG_BASE + RCAR_CPG_RMSTPCR9) & ~((1 << 18) |(1 << 19) |(1 << 27) |(1 << 28) | (1 << 29) | (1 << 30) | (1 << 31)));
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR9, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR9) & ~((1 << 18) |(1 << 19) |(1 << 27) |(1 << 28) | (1 << 29) | (1 << 30) | (1 << 31)));

    // I2C2
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR5);
    tmp |= ((1 << 0) | (1 << 4));
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_GPSR5, tmp);

    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR12);
    tmp &= ~(0xF << 8);	// IP12[12:8]
    tmp |= (0x4 << 8);		// SCL2
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_IPSR12, tmp);

    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR11);
    tmp &= ~(0xF << 24);	// IP11[27:24]
    tmp |= (0x4 << 24);		// SDA2
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_IPSR11, tmp);
    
    

    // Mode select
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_MODSEL0);
    tmp &= ~(0x1 << 19);
    out32(RCAR_PFC_BASE + RCAR_PFC_PMMR, ~tmp);
    out32(RCAR_PFC_BASE + RCAR_PFC_MODSEL0, tmp);
}

static void init_audio()
{
    uint32_t tmp;

    /* Clock Pins */
    // MOD_SEL0[4:3]=2'b00 (AUDIO_CLK_A0)
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_MODSEL0);
    tmp &= ~0x00000018;
    rcar_pfc_config(RCAR_PFC_MODSEL0, tmp);
    // MOD_SEL2[18:17]=2'b00 (AUDIO_CLK_B0, AUDIO_CLK_C0)
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_MODSEL2);
    tmp &= ~0x00060000;
    rcar_pfc_config(RCAR_PFC_MODSEL2, tmp);

    // IPSR13[31:28][11:8] = 0x8,0x3 = AUDIO_CLKOUT_A, AUDIO_CLKB_A
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR13);
    tmp &= ~0xF0000F00;
    tmp |=  0x80000300;
    rcar_pfc_config(RCAR_PFC_IPSR13, tmp);
    
    // IPSR14[7:4][3:0]    = 0x3,0x8 = AUDIO_CLKC_A, AUDIO_CLKOUT3_A
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR14);
    tmp &= ~0x000000FF;
    tmp |=  0x00000038;
    rcar_pfc_config(RCAR_PFC_IPSR14, tmp);

    // IPSR17[3:0]         = 0x0     = AUDIO_CLKA_A
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR17);
    tmp &= ~0x0000000F;
    rcar_pfc_config(RCAR_PFC_IPSR17, tmp);

    // GPSR5[21][19][18][12] = AUDIO_CLKC_A, AUDIO_CLKOUT3_A, AUDIO_CLKOUT_A, AUDIO_CLKB_A
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR5);
    tmp |= (1 << 21) | (1 << 19) | (1 << 18) | (1 << 12);
    rcar_pfc_config(RCAR_PFC_GPSR5, tmp);

    // GPSR6[22]             = AUDIO_CLKA_A
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR6);
    tmp |= (1 << 22);
    rcar_pfc_config(RCAR_PFC_GPSR6, tmp);

    /* SSI0&1 Pins */
    // MOD_SEL1[20] = 0 = SSI GroupA 
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_MODSEL1);
    tmp &= ~0x00100000;
    rcar_pfc_config(RCAR_PFC_MODSEL1, tmp);
    
    // IPSR14[31:28][27:24][23:20] = 0x0,0x0,0x0 = SSI_SDATA0, SSI_WS0129, SSI_SCK0129 
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR14);
    tmp &= ~0xFFF00000;
    rcar_pfc_config(RCAR_PFC_IPSR14, tmp);

    // IPSR15[3:0] = 0x0 = SSI_SDATA1_A 
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR15);
    tmp &= ~0x0000000F;
    rcar_pfc_config(RCAR_PFC_IPSR15, tmp);

    // GPSR6[3:0] = SSI_SDATA1_A, SSI_SDATA0, SSI_WS0129, SSI_SCK0129 
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR6);
    tmp |=  0x0000000F;
    rcar_pfc_config(RCAR_PFC_GPSR6, tmp);

    /* SSI4 Pins */	
    // IPSR15[31:28][27:24][23:20] = 0x0,0x0,0x0 = SSI_SDATA4, SSI_WS4, SSI_SCK4 
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR15);
    tmp &= ~0xFFF00000;
    rcar_pfc_config(RCAR_PFC_IPSR15, tmp);

    // GPSR6[10:8] = SSI_SDATA4, SSI_WS4, SSI_SCK4 
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR6);
    tmp |=  0x00000700;
    rcar_pfc_config(RCAR_PFC_GPSR6, tmp);
        
    /* SSI7&8 Pins */	
    // IPSR16[27:24][23:20][19:16][15:12] = 0x0,0x0,0x0,0x0 = SSI_SDATA8, SSI_SDATA7, SSI_WS78, SSI_SCK78 
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR16);
    tmp &= ~0x0FFFF000;
    rcar_pfc_config(RCAR_PFC_IPSR16, tmp);

    // GPSR6[20:17] = SSI_SDATA8, SSI_SDATA7, SSI_WS78, SSI_SCK78 
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR6);
    tmp |=  0x001E0000;
    rcar_pfc_config(RCAR_PFC_GPSR6, tmp);

    /* Enable Audio modules */
    // ADG 
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR9) & ~(1 << 22);
    rcar_cpg_config(RCAR_CPG_SMSTPCR9, tmp);

    // SSI 0->9 
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR10) & 0xFFFF001F;
    rcar_cpg_config(RCAR_CPG_SMSTPCR10, tmp);

    /* Set clock rate for SSI, and timing for SCU */
        
    /* S0D4clock (200 MHz)  --|     BRGA    |--+-->???
     *
     * Clock Select Register(SSICKR)
     *  [31]   =0'B!           : BRGA output clock
     *  [30:23]=8'B01000110    : Reserved (Fixed value)
     *  [22:20]=3'B010         : BRGA input clock = S0D4(200MHz)	
     *  [18:16]=3'B101         : BRGB input clock = Fixed at 0
     */    
    out32(RCAR_ADG_BASE + RCAR_ADG_SSICKR, 0x23250000);

    /* BRGA Baud Rate Setting Register (BRRA)
     *  [9:8]=2'B00 : ACLK_A
     *  [7:0]=8'Hxx : division ratio
     *  div ratio = 1/(2(N+1))=1/(2(1+1))= 200MHz/4 = 50MHz */
    out32(RCAR_ADG_BASE + RCAR_ADG_BRRA, 0);
    
    /* Clock Select Register1 (SSICKR1)
     *  [27:24]=4'b0000 : AUDIO_CLKOUT3 = BRGA/BRGB select
     *  [19:16]=4'b0000 : AUDIO_CLKOUT2 = BRGA/BRGB select
     *  [11:08]=4'b0000 : AUDIO_CLKOUT1 = BRGA/BRGB select
     *  [03:00]=4'b0000 : AUDIO_CLKOUT  = BRGA/BRGB select 
     */
    out32(RCAR_ADG_BASE + RCAR_ADG_SSICKR1, 0x00000000);

    /* AUDIO_CLK_A(22.5792MHz) ---->SSIx    
     *  AUDIO_CLK select data
     *  No divided, AUDIO_CLK_A output clock	
     */       
    // SSI0,1, 2, 3 select AUDIO_CLK_A clock(22.5792MHz) / 32 
    out32(RCAR_ADG_BASE + RCAR_ADG_CKSEL0, 0x01010101);

    // SSI4,5, 6, 7 select AUDIO_CLK_A clock(22.5792MHz) / 32 
    out32(RCAR_ADG_BASE + RCAR_ADG_CKSEL1, 0x01010101);

    // SSI9 select AUDIO_CLK_A clock(22.5792MHz) / 32 
    out32(RCAR_ADG_BASE + RCAR_ADG_CKSEL2, 0x00000100);

    // TIM_EN 
    out32(RCAR_ADG_BASE + RCAR_ADG_TIM_EN, 0x3f);

    // SCU SRC0->9,CMD0->1,DVC0->1 
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR10, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR10) & ~((0x3FF << 22) | (7 << 17)));

    // SCU timing 
    out32(RCAR_ADG_BASE + RCAR_ADG_SRCIN_TIMSEL0,  (6 << 24) |  // SRC1 in timing: Use SW0
                                                   (6 << 8));   // SRC0 in timing: Use SW0

    out32(RCAR_ADG_BASE + RCAR_ADG_SRCIN_TIMSEL1,  (6 << 24) |  // SRC3 in timing: Use SW0
                                                   (6 << 8));   // SRC2 in timing: Use SW0

    out32(RCAR_ADG_BASE + RCAR_ADG_SRCIN_TIMSEL2,  (6 << 24) |  // SRC5 in timing: Use SW0
                                                   (6 << 8));   // SRC4 in timing: Use SW0
                                                   
    out32(RCAR_ADG_BASE + RCAR_ADG_SRCIN_TIMSEL3,  (6 << 24) |  // SRC7 in timing: Use SW0
                                                   (6 << 8));   // SRC6 in timing: Use SW0

    out32(RCAR_ADG_BASE + RCAR_ADG_SRCIN_TIMSEL4,  (6 << 24) |  // SRC9 in timing: Use SW0
                                                   (6 << 8));   // SRC8 in timing: Use SW0
                                                   
    out32(RCAR_ADG_BASE + RCAR_ADG_SRCOUT_TIMSEL0, (6 << 24) |  // SRC1 out timing: Use SW0
                                                   (6 << 8));   // SRC0 out timing: Use SW0

    out32(RCAR_ADG_BASE + RCAR_ADG_SRCOUT_TIMSEL1, (6 << 24) |  // SRC3 out timing: Use SW0
                                                   (6 << 8));   // SRC2 out timing: Use SW0
                                                   
    out32(RCAR_ADG_BASE + RCAR_ADG_SRCOUT_TIMSEL2, (6 << 24) |  // SRC5 out timing: Use SW0
                                                   (6 << 8));   // SRC4 out timing: Use SW0

    out32(RCAR_ADG_BASE + RCAR_ADG_SRCOUT_TIMSEL3, (6 << 24) |  // SRC7 out timing: Use SW0
                                                   (6 << 8));   // SRC6 out timing: Use SW0

    out32(RCAR_ADG_BASE + RCAR_ADG_SRCOUT_TIMSEL4, (6 << 24) |  // SRC9 out timing: Use SW0
                                                   (6 << 8));   // SRC8 out timing: Use SW0
                                                   
    out32(RCAR_ADG_BASE + RCAR_ADG_CMDOUT_TIMSEL,  (6 << 24) |  // CMD1 out timing: Use SW0
                                                   (6 << 8));   // CMD0 out timing: Use SW0
    /* Reset Audio modules */
    // reset all SSI 
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SRCR10) | 0x0000FFE0;
    rcar_cpg_config(RCAR_CPG_SRCR10, tmp);
    // clear reset state all SSI 
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR10) | 0x0000FFE0;	
    rcar_cpg_config(RCAR_CPG_SRSTCLR10, tmp);

    // SSI mode register 0 
    out32(RCAR_SSIU_BASE + RCAR_SSIU_MODE0, 0);
    // SSI mode register 1 
    out32(RCAR_SSIU_BASE + RCAR_SSIU_MODE1, 0);
    // SSI mode register 2 
    out32(RCAR_SSIU_BASE + RCAR_SSIU_MODE2, 0);
    // SSI control register 
    out32(RCAR_SSIU_BASE + RCAR_SSIU_CONTROL, 0);
}

static void init_sdhi()
{
    uint32_t tmp;

    /* Pin mux for SDHI */

    /* SDHI channel 0 */
    /* PUEN4[09:08]=2'b00    =WP,CD                      =Pull-up/down OFF
       PUEN3[15:10]=6'b000000=DAT3,DAT2,DAT1,DAT0,CMD,CLK=Pull-up/down OFF */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_PUEN4);
    tmp &= ~0x00000300;
    rcar_pfc_config(RCAR_PFC_PUEN4, tmp);
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_PUEN3);
    tmp &= ~0x0000FC00;
    rcar_pfc_config(RCAR_PFC_PUEN3, tmp);

    /* IPSR10[15:12][11:08]              ={4'h0,4'h0}          =WP,CD
       IPSR8 [07:04][03:00]              ={4'h0,4'h0}          =DAT3,DAT2
       IPSR7 [31:28][27:24][23:20][19:16]={4'h0,4'h0,4'h0,4'h0}=DAT1,DAT0,CMD,CLK */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR10);
    tmp &= ~0x00000FF00;
    rcar_pfc_config(RCAR_PFC_IPSR10, tmp);

    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR8);
    tmp &= ~0x000000FF;
    rcar_pfc_config(RCAR_PFC_IPSR8, tmp);

    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR7);
    tmp &= ~0xFFFF0000;
    rcar_pfc_config(RCAR_PFC_IPSR7, tmp);

    /* GPSR3[13:12][5:0]={2'b11,6'b111111}=Peripheral function */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR3);
    tmp |= 0x0000303F;
    rcar_pfc_config(RCAR_PFC_GPSR3, tmp);

    /* DRVCTRL17[31:28][27:24]                       ={4'h3,4'h3}                    =CD,WP
       DRVCTRL13[23:20][19:16][15:12][11:8][7:4][3:0]={4'h3,4'h3,4'h3,4'h3,4'h3,4'h3}=CLK,CMD,DAT0,DAT1,DAT2,DAT3 */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL17);
    tmp &= ~0xFF000000;
    tmp |=  0x33000000;
    rcar_pfc_config(RCAR_PFC_DRVCTRL17, tmp);

    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL13);
    tmp &= ~0x00FFFFFF;
    tmp |= 0x00333333;
    rcar_pfc_config(RCAR_PFC_DRVCTRL13, tmp);

    /* TDSELCTRL0[1:0]={2'b01}=SD0CLK_TDSEL[1:0] */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_TDSELCTRL0);
    tmp &= ~(3 << 0);
    tmp |=  (1 << 0);
    rcar_pfc_config(RCAR_PFC_TDSELCTRL0, tmp);

    /* SDHI channel 3 */
    /* PUEN4[06:05][02:00]=2'b00,3'b000=WP,CD,DAT3,DAT2,DAT1=Pull-up/down OFF
       PUEN3[31:29]       =3'b000      =DAT0,CMD,CLK        =Pull-up/down OFF */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_PUEN4);
    tmp &= ~0x00000067;
    rcar_pfc_config(RCAR_PFC_PUEN4, tmp);

    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_PUEN3);
    tmp &= ~0xE0000000;
    rcar_pfc_config(RCAR_PFC_PUEN3, tmp);

    /* IPSR10[07:04][03:00]={4'h1,4'h1}=WP,CD
       no IPSR registers               =DAT[3:0],CMD,CLK */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR10);
    tmp &= ~(0x000000FF);
    tmp |=  (0x00000011);
    rcar_pfc_config(RCAR_PFC_IPSR10, tmp);

    /* GPSR4[16:15][12:7]={2'b11,6'b111111}=WP,CD,DAT[3:0],CMD,CLK=Peripheral function */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR4);
    tmp |= 0x00011F10;
    rcar_pfc_config(RCAR_PFC_GPSR4, tmp);

    /* DRVCTRL16[31:28][27:24][23:20][11:8][7:4]={4'h3,4'h3,4'h3,4'h3,4'h3}=DAT1,DAT2,DAT3,CD,WP
       DRVCTRL15[11:8][7:4][3:0]                ={4'h3,4'h3,4'h3}          =CLK,CMD,DAT0 */
    // tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL16);
    // tmp &= ~0xFFFFFFF0;
    // tmp |=  0x33333330;
    // rcar_pfc_config(RCAR_PFC_DRVCTRL16, tmp);
    // tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL15);
    // tmp &= ~0x00000FFF;
    // tmp |=  0x00000333;
    // rcar_pfc_config(RCAR_PFC_DRVCTRL15, tmp);

    /* TDSELCTRL0[7:6]={2'b01}=SD3CLK_TDSEL[1:0] */
    // tmp = in32(RCAR_PFC_BASE + RCAR_PFC_TDSELCTRL0);
    // tmp &= ~(3 << 6);
    // tmp |=  (1 << 6);
    // rcar_pfc_config(RCAR_PFC_TDSELCTRL0, tmp);

    /* Enable clock */

    /* Halt operation */
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR3);
    tmp |= (1 << 11) | (1 << 14);
    rcar_cpg_config(RCAR_CPG_SMSTPCR3, tmp);

    /* Set clock rate for SDHI */
    /* SDHI channel 0 */
    rcar_cpg_config(RCAR_CPG_SD0CKCR, 0x00000001);	/* clock input = 800Mhz/(4) = 200Mhz (max clock) */
    /* SDHI channel 3 */
    rcar_cpg_config(RCAR_CPG_SD3CKCR, 0x00000001);	/* clock input = 800Mhz/(4) = 200Mhz (max clock) */

    /* Set after config for SDCKCR */
    /* Enable SDHI 0, 3 operation */
    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR3);
    tmp &= ~((1 << 11) | (1 << 14));
    rcar_cpg_config(RCAR_CPG_SMSTPCR3, tmp);

    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR9);
    tmp &= ~(0xFF << 5);
    rcar_cpg_config(RCAR_CPG_SMSTPCR9, tmp);

    /*
     * GPIO settings for power and signal voltage
     * GPIO5_1 : SDHI0 power voltage control
     * GPIO5_2 : SDHI0 signal voltage control
     */
    /* GPIO mode */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR5);
    tmp &= ~((1 << 1) | (1 << 2));
    rcar_pfc_config(RCAR_PFC_GPSR5, tmp);
    /* GPIO: GPIO mode */
    tmp  = in32(RCAR_GPIO5_BASE + RCAR_GPIO_IOINTSEL);
    tmp &= ~((1 << 1) | (1 << 2));
    out32(RCAR_GPIO5_BASE + RCAR_GPIO_IOINTSEL, tmp);
    /* GPIO: GPIO output mode */
    tmp  = in32(RCAR_GPIO5_BASE + RCAR_GPIO_INOUTSEL);
    tmp |= (1 << 1) | (1 << 2);
    out32(RCAR_GPIO5_BASE + RCAR_GPIO_INOUTSEL, tmp);
    /* GPIO: positive output mode */
    tmp  = in32(RCAR_GPIO5_BASE + RCAR_GPIO_POSNEG);
    tmp &= ~((1 << 1) | (1 << 2));
    out32(RCAR_GPIO5_BASE + RCAR_GPIO_POSNEG, tmp);
    /* GPIO: GPIO output register select, we use OUTDT */
    tmp  = in32(RCAR_GPIO5_BASE + RCAR_GPIO_OUTDTSEL);
    tmp &= ~((1 << 1) | (1 << 2));
    out32(RCAR_GPIO5_BASE + RCAR_GPIO_OUTDTSEL, tmp);

    /*
     * GPIO settings for power and signal voltage
     * GPIO3_14 : SDHI3 power voltage control
     * GPIO3_15 : SDHI3 signal voltage control
     */
    /* GPIO mode */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR3);
    tmp &= ~((1 << 14) | (1 << 15));
    rcar_pfc_config(RCAR_PFC_GPSR3, tmp);
    /* GPIO: GPIO mode */
    tmp  = in32(RCAR_GPIO3_BASE + RCAR_GPIO_IOINTSEL);
    tmp &= ~((1 << 14) | (1 << 15));
    out32(RCAR_GPIO3_BASE + RCAR_GPIO_IOINTSEL, tmp);
    /* GPIO: GPIO output mode */
    tmp  = in32(RCAR_GPIO3_BASE + RCAR_GPIO_INOUTSEL);
    tmp |= (1 << 14) | (1 << 15);
    out32(RCAR_GPIO3_BASE + RCAR_GPIO_INOUTSEL, tmp);
    /* GPIO: positive output mode */
    tmp  = in32(RCAR_GPIO3_BASE + RCAR_GPIO_POSNEG);
    tmp &= ~((1 << 14) | (1 << 15));
    out32(RCAR_GPIO3_BASE + RCAR_GPIO_POSNEG, tmp);
    /* GPIO: GPIO output register select, we use OUTDT */
    tmp  = in32(RCAR_GPIO3_BASE + RCAR_GPIO_OUTDTSEL);
    tmp &= ~((1 << 14) | (1 << 15));
    out32(RCAR_GPIO3_BASE + RCAR_GPIO_OUTDTSEL, tmp);
}

static void init_mmcif()
{
    uint32_t tmp;

    /* Pin mux for MMCIF */

    /* MMCIF channel 0 */
    /* GPSR3[11:8]  --> SD2_DAT[7:4]
       GPSR4[6:0]   --> SD2_DS, SD2_DAT[3:0], SD2_CMD, SD2_CLK */

    /* IPSR8[31:28][27:24][23:20][19:16]          --> 0x1111   --> SD2_DAT[7:4] */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR8);
    tmp &= ~(0xFFFF0000);
    tmp |=  (0x11110000);
    rcar_pfc_config(RCAR_PFC_IPSR8, tmp);

    /* IPSR9[23:20][19:16][15:12][11:8][7:4][3:0] --> 0x000000 --> SD2_DS, SD2_DAT[3:0], SD2_CLK (SD2_CMD has no bits of IPSR) */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR9);
    tmp &= ~(0x00FFFFFF);
    tmp |=  (0x00000000);
    rcar_pfc_config(RCAR_PFC_IPSR9, tmp);

    /* GPSR3[11:8]  --> SD2_DAT[7:4] */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR3);
    tmp |= 0x00000F00;
    rcar_pfc_config(RCAR_PFC_GPSR3, tmp);

    /* GPSR4[6:0]   --> SD2_DS, SD2_DAT[3:0], SD2_CMD, SD2_CLK */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR4);
    tmp |= 0x0000003F;
    rcar_pfc_config(RCAR_PFC_GPSR4,tmp);

    /* POCCTRL0[18:6] = all 0 (SD1,2 IO voltage=1.8V) */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_POCCTRL0);
    tmp &= ~0x0007FFC0;
    rcar_pfc_config(RCAR_PFC_POCCTRL0, tmp);

    /* TDSELCTRL0[5:2] = 4'b0101 (fixed value) */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_TDSELCTRL0);
    tmp &= ~(0xF << 2);
    tmp |=  (5 << 2);
    rcar_pfc_config(RCAR_PFC_TDSELCTRL0, tmp);

    /* PUEN3[28:18] = all 0 -> Pull-up/down OFF */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_PUEN3);
    tmp &= ~0x1FFC0000;
    rcar_pfc_config(RCAR_PFC_PUEN3, tmp);

    /* DRVCTRL14[31:0] = SD1_DAT[0:3], SD2_CLK/CMD */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL14);
    tmp &= ~0x00FFFFFF;
    tmp |=  0x00333333;
    rcar_pfc_config(RCAR_PFC_DRVCTRL14, tmp);

    /* DRVCTRL15[31:12] = SD2_DAT[0:3]/DS */
    tmp  = in32(RCAR_PFC_BASE + RCAR_PFC_DRVCTRL15);
    tmp &= ~0xFFFFF000;
    tmp |=  0x33333000;
    rcar_pfc_config(RCAR_PFC_DRVCTRL15, tmp);

    /* Enable MMCIF0 = SDHI1&2 operation */
    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR3);
    tmp &= ~((1 << 12) | (1 << 13));
    rcar_cpg_config(RCAR_CPG_SMSTPCR3, tmp);

    /* Reset MMCIF0 = SDHI1&2  */
    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SRCR3);
    tmp |= ((1 << 12) | (1 << 13));
    rcar_cpg_config(RCAR_CPG_SRCR3, tmp);	
    
    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR3);
    tmp |= ((1 << 12) | (1 << 13));
    rcar_cpg_config(RCAR_CPG_SRSTCLR3, tmp);	
    
    /* Set clock rate for MMCIF t0 200MHz */
    /* MMCIF0 = SDHI1&2 */
    rcar_cpg_config(RCAR_CPG_SD1CKCR, 0x00000001);
    rcar_cpg_config(RCAR_CPG_SD2CKCR, 0x00000001);
    
    /* Reset MMCIF0 = SDHI1&2  */
    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SRCR3);
    tmp |= ((1 << 12) | (1 << 13));
    rcar_cpg_config(RCAR_CPG_SRCR3, tmp);	
    mdelay(10);
    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SRSTCLR3);
    tmp |= ((1 << 12) | (1 << 13));
    rcar_cpg_config(RCAR_CPG_SRSTCLR3, tmp);	    
}

static void init_dmac()
{
    uint32_t tmp;

    /* Enable audio DMAC */
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR5) & ~((1 << 2) | (1 << 1));
    rcar_cpg_config(RCAR_CPG_SMSTPCR5, tmp);
    rcar_cpg_config(RCAR_CPG_SRSTCLR5, tmp);
    out16(RCAR_AUDIODMAC0_BASE + RCAR_SYSDMAC_DMAOR, 1);    // Enable DMA master
    out16(RCAR_AUDIODMAC1_BASE + RCAR_SYSDMAC_DMAOR, 1);    // Enable DMA master
    out16(RCAR_AUDIODMAC0_BASE + RCAR_SYSDMAC_DMACHCLR, 0xFFFF);    // Clear all channels
    out16(RCAR_AUDIODMAC1_BASE + RCAR_SYSDMAC_DMACHCLR, 0xFFFF);    // Clear all channels

    /* Enable SYSDMAC */
    /* Enable clock for SYS-DMAC0,SYS-DMAC1,SYS-DMAC2 */
    tmp = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR2) & ~(0x7 << 17);
    rcar_cpg_config(RCAR_CPG_SMSTPCR2, tmp);
    rcar_cpg_config(RCAR_CPG_SRSTCLR2, tmp);
    out16(RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_DMAOR, 1);      // Enable DMA master
    out16(RCAR_SYSDMAC1_BASE + RCAR_SYSDMAC_DMAOR, 1);      // Enable DMA master
    out16(RCAR_SYSDMAC2_BASE + RCAR_SYSDMAC_DMAOR, 1);      // Enable DMA master
    out16(RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_DMACHCLR, 0xFFFF);  // Clear all channels
    out16(RCAR_SYSDMAC1_BASE + RCAR_SYSDMAC_DMACHCLR, 0xFFFF);  // Clear all channels
    out16(RCAR_SYSDMAC2_BASE + RCAR_SYSDMAC_DMACHCLR, 0xFFFF);  // Clear all channels
}

static void init_graphics ()
{
    uint32_t tmp;

    /* GSX: force power and clock supply for all 3D graphics engines*/
    out32 (RCAR_SYSC_BASE + RCAR_SYSC_PWRONCR2, 0x1F);
    while (in32(RCAR_SYSC_BASE + RCAR_SYSC_PWRSR2) != 0x3E0)
        mdelay(20000);

    /* MSTP setting for 3DG */
    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR1);
    tmp &= ~(1 << 12);
    rcar_cpg_config(RCAR_CPG_SMSTPCR1, tmp);
}

void init_hscif1(void)
{
    uint32_t tmp;

    /* NOTE:  SCIF1 and HSCIF1 share the same pins and connector CN26 */
    /* Pin mux for HSCIF1 and SCIF1
    * GP5-5, IPSR11[15:12]=1 -> HSCIF1_RX_A   ||   IPSR11[15:12]=0 -> SCIF1_RX_A
    * GP5-6, IPSR11[19:16]=1 -> HSCIF1_TX_A   ||   IPSR11[19:16]=0 -> SCIF1_TX_A
    * GP5-7, IPSR11[23:20]=1 -> HSCIF1_CTS_A  ||   IPSR11[23:20]=0 -> SCIF1_CTS_A
    * GP5-8, IPSR11[27:24]=1 -> HSCIF1_RTS_A  ||   IPSR11[27:24]=0 -> SCIF1_RTS_A
    */
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_GPSR5);
    tmp |= (0x0f << 5);
    rcar_pfc_config(RCAR_PFC_GPSR5, tmp);

    // Select Pin Mux for HSCIF1
    tmp = in32(RCAR_PFC_BASE + RCAR_PFC_IPSR11);
    tmp &= ~0x0FFFF000;
    tmp |= ((1 << 12) | (1 << 16) | (1 << 20) | (1 << 24));
    rcar_pfc_config(RCAR_PFC_IPSR11, tmp);
    //Enable HSCIF1 clock
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR5,in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR5) & (~(1<<19)));
}

void init_board()
{
    uint32_t tmp;

    init_ethernet();
    //init_usb(0);
    init_usb(1);	
    //init_usb3(0);
    //init_usb3(1); Currently not supported
    //init_msiof();
    init_i2c();
    init_audio();
    //init_sdhi();
    init_mmcif();
    init_dmac();
    init_display();
    //init_graphics();

    /* Enable SCIF0 clock */
    //out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR7, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR7) & ~ ((1 << 21) | (1<<22)));
    //out32(RCAR_CPG_BASE + RCAR_CPG_MSTPSR7, in32(RCAR_CPG_BASE + RCAR_CPG_MSTPSR7) & ~ ((1<<22)));
    //out32(RCAR_CPG_BASE + RCAR_CPG_RMSTPCR7, in32(RCAR_CPG_BASE + RCAR_CPG_RMSTPCR7) & ~ ((1<<22)));

    /* Enable DU and LVDS */
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR7,in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR7) & (~(0x1f<<22)));

    /* Enable SATA clock */
    tmp  = in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR8);
    tmp &= ~(1 << 15);
    rcar_cpg_config(RCAR_CPG_SMSTPCR8, tmp);

    /* Enable PCIe clock */
    //out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR3,in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR3) & (~(1<<19)));
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/init_board.c $ $Rev: 814384 $")
#endif
