/*
 * Copyright (C) 2015 Loci Controls Inc.
 *               2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_cc2538
 * @ingroup     drivers_periph_spi
 * @{
 *
 * @file
 * @brief       Low-level SPI driver implementation
 *
 * @author      Ian Martin <ian@locicontrols.com>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "mutex.h"
#include "assert.h"
#include "periph/spi.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

/**
 * @brief   Array holding one pre-initialized mutex for each SPI device
 */
static mutex_t locks[SPI_NUMOF];

static inline cc2538_ssi_t *dev(spi_t bus)
{
    return spi_config[bus].dev;
}

static inline void poweron(spi_t bus)
{
    SYS_CTRL_RCGCSSI |= (1 << bus);
    SYS_CTRL_SCGCSSI |= (1 << bus);
    SYS_CTRL_DCGCSSI |= (1 << bus);
}

static inline void poweroff(spi_t bus)
{
    SYS_CTRL_RCGCSSI &= ~(1 << bus);
    SYS_CTRL_SCGCSSI &= ~(1 << bus);
    SYS_CTRL_DCGCSSI &= ~(1 << bus);
}

void spi_init(spi_t bus)
{
    assert(bus <= SPI_NUMOF);

    /* temporarily power on the device */
    poweron(bus);
    /* configure device to be a master and disable SSI operation mode */
    dev(bus)->CR1 = 0;
    /* configure system clock as SSI clock source */
    dev(bus)->CC = SSI_SS_IODIV;
    /* and power off the bus again */
    poweroff(bus);

    /* trigger SPI pin configuration */
    spi_init_pins(bus);
}

void spi_init_pins(spi_t bus)
{
    switch ((uintptr_t)spi_config[bus].dev) {
        case (uintptr_t)SSI0:
            IOC_PXX_SEL[spi_config[bus].mosi_pin] = SSI0_TXD;
            IOC_PXX_SEL[spi_config[bus].sck_pin ] = SSI0_CLKOUT;
            IOC_PXX_SEL[spi_config[bus].cs_pin  ] = SSI0_FSSOUT;

            IOC_SSIRXD_SSI0 = spi_config[bus].miso_pin;
            break;

        case (uintptr_t)SSI1:
            IOC_PXX_SEL[spi_config[bus].mosi_pin] = SSI1_TXD;
            IOC_PXX_SEL[spi_config[bus].sck_pin ] = SSI1_CLKOUT;
            IOC_PXX_SEL[spi_config[bus].cs_pin  ] = SSI1_FSSOUT;

            IOC_SSIRXD_SSI1 = spi_config[bus].miso_pin;
            break;
    }

    IOC_PXX_OVER[spi_config[bus].mosi_pin] = IOC_OVERRIDE_OE;
    IOC_PXX_OVER[spi_config[bus].miso_pin] = IOC_OVERRIDE_DIS;
    IOC_PXX_OVER[spi_config[bus].sck_pin ] = IOC_OVERRIDE_OE;
    IOC_PXX_OVER[spi_config[bus].cs_pin  ] = IOC_OVERRIDE_OE;

    gpio_hardware_control(spi_config[bus].mosi_pin);
    gpio_hardware_control(spi_config[bus].miso_pin);
    gpio_hardware_control(spi_config[bus].sck_pin);
    gpio_hardware_control(spi_config[bus].cs_pin);
}

int spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
{
    /* lock the bus */
    mutex_lock(&locks[bus]);
    /* power on device */
    poweron(bus);
    /* configure SCR clock field, data-width and mode */
    dev(bus)->CR0 = 0;
    dev(bus)->CPSR = (spi_clk_config[clk].cpsr); // clock register
    dev(bus)->CR0 = ((spi_clk_config[clk].scr << 8) | mode | SSI_CR0_DSS(8));  // set SSI_CR0
    /* enable SSI device */
    dev(bus)->CR1 = SSI_CR1_SSE;

    return SPI_OK;
}

void spi_release(spi_t bus)
{
    /* disable and power off device */
    dev(bus)->CR1 = 0;
    poweroff(bus);
    /* and release lock... */
    mutex_unlock(&locks[bus]);
}

void pp(uint8_t b){
    int byte;
    for (int j=7;j>=0;j--)
    {
        byte = (b >> j) & 1;
        printf("%u", byte);
    }
}
void pp1(uint8_t b){
    printf("%2x", b);
}

void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out/*NULL*/, void *in/*res*/, size_t len /*2*/)
{
    const uint8_t *out_buf = out;
    uint8_t *in_buf = in;

    assert(out_buf || in_buf);

    if (cs != SPI_CS_UNDEF) {
        gpio_clear((gpio_t)cs); // su: clear chip select
    }

    //uint16_t c = 0;
    // Transmit the data
    if (!in_buf) {
        printf("SU => %s\n", "sending data");
        for (size_t i = 0; i < len; i++) {
            while (!(dev(bus)->SR & SSI_SR_TNF)) {}
            dev(bus)->DR = out_buf[i]; // su: writes to Transmit FIFO
            if (i%8==0){
                printf("\n");
            }
            pp1(out_buf[i]);
            printf(" | ");
        }
        /* flush RX FIFO while busy*/
        while ((dev(bus)->SR & SSI_SR_BSY)) {
            dev(bus)->DR; // su: clear Receive FIFO
        }
    }
    // Receive the data
    else if (!out_buf) { /*TODO this case is currently untested */
        // su: changed by Subhasis
        size_t in_cnt = 0;
        // while (!(dev(bus)->SR & SSI_SR_TNF)) {}
        for (size_t i = 0; i < len; i++) {
            dev(bus)->DR = 0;
            while (!(dev(bus)->SR & SSI_SR_RNE)) {}
            in_buf[in_cnt++] = dev(bus)->DR;
            // if (i%8==0){
            //     printf("\n");
            // }
            // pp1(in_buf[in_cnt-1]);
            // printf(" | ");
        }
        /* get remaining bytes */
        // while (dev(bus)->SR & SSI_SR_RNE) {
        //     in_buf[in_cnt++] = dev(bus)->DR;
        //     c++;
        //     if (c%8==0){
        //         printf("\n");
        //     }
        //     pp(in_buf[in_cnt-1]);
        //     printf(" | ");
        // }
        puts("\n------------------------------\n");
    }
    // first Transmit the data then Receive the data
    else {
        for (size_t i = 0; i < len; i++) {
            while (!(dev(bus)->SR & SSI_SR_TNF)) {}
            dev(bus)->DR = out_buf[i];
            while (!(dev(bus)->SR & SSI_SR_RNE)){}
            in_buf[i] = dev(bus)->DR;
        }
    /* wait until no more busy */
    while ((dev(bus)->SR & SSI_SR_BSY)) {}
    }

    if ((!cont) && (cs != SPI_CS_UNDEF)) {
        gpio_set((gpio_t)cs);
    }
}
