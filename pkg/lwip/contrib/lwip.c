/*
 * Copyright (C) Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include "lwip/tcpip.h"
#include "lwip/netif/netdev.h"
#include "lwip/netif.h"
#include "netif/lowpan6.h"

#ifdef MODULE_NETDEV_TAP
#include "netdev_tap.h"
#include "netdev_tap_params.h"
#endif

#ifdef MODULE_AT86RF2XX
#include "at86rf2xx.h"
#include "at86rf2xx_params.h"
#endif

#ifdef MODULE_MRF24J40
#include "mrf24j40.h"
#include "mrf24j40_params.h"
#endif

//written by us ag
#ifdef MODULE_ENC28J60 //ag
#include "encj28j60.h" //ag
#include "encj28j60_params.h" //ag
#endif //ag

#include "lwip.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#ifdef MODULE_NETDEV_TAP
#define LWIP_NETIF_NUMOF        (NETDEV_TAP_MAX)
#endif

#ifdef MODULE_AT86RF2XX     /* is mutual exclusive with above ifdef */
#define LWIP_NETIF_NUMOF        (sizeof(at86rf2xx_params) / sizeof(at86rf2xx_params[0]))
#endif

#ifdef MODULE_MRF24J40     /* is mutual exclusive with above ifdef */
#define LWIP_NETIF_NUMOF        (sizeof(mrf24j40_params) / sizeof(mrf24j40_params[0]))
#endif

#ifdef MODULE_ENC28J60 //ag
#define LWIP_NETIF_NUMOF   (sizeof(encj28j60_params)/sizeof(encj28j60_params[0])) //ag
#endif //ag

#ifdef LWIP_NETIF_NUMOF
static struct netif netif[LWIP_NETIF_NUMOF];
#endif

#ifdef MODULE_NETDEV_TAP
static netdev_tap_t netdev_taps[LWIP_NETIF_NUMOF];
#endif

#ifdef MODULE_AT86RF2XX
static at86rf2xx_t at86rf2xx_devs[LWIP_NETIF_NUMOF];
#endif

#ifdef MODULE_MRF24J40
static mrf24j40_t mrf24j40_devs[LWIP_NETIF_NUMOF];
#endif

#ifdef MODULE_ENC28J60 //ag
static enc28j60_t encj28j60_devs[LWIP_NETIF_NUMOF]; //ag
#endif //ag

void lwip_bootstrap(void)
{
    /* TODO: do for every eligable netdev */
#ifdef LWIP_NETIF_NUMOF
#ifdef MODULE_NETDEV_TAP
    for (int i = 0; i < LWIP_NETIF_NUMOF; i++) {
        netdev_tap_setup(&netdev_taps[i], &netdev_tap_params[i]);
        if (netif_add(&netif[i],192.168.0.11, 255.255.255.0, 0.0.0.0, &netdev_taps[i], lwip_netdev_init,  //ag
                      tcpip_input) == NULL) {  //ag
            DEBUG("Could not add netdev_tap device\n");
            return;
        }
    }
#elif defined(MODULE_MRF24J40)
    for (int i = 0; i < LWIP_NETIF_NUMOF; i++) {
        mrf24j40_setup(&mrf24j40_devs[i], &mrf24j40_params[i]);
        if (netif_add(&netif[i],192.168.0.11, 255.255.255.0, 0.0.0.0, &mrf24j40_devs[i], lwip_netdev_init,  //ag
                      tcpip_6lowpan_input) == NULL) {  //ag
            DEBUG("Could not add mrf24j40 device\n");
            return;
        }
    }
#elif defined(MODULE_AT86RF2XX)
    for (int i = 0; i < LWIP_NETIF_NUMOF; i++) {
        at86rf2xx_setup(&at86rf2xx_devs[i], &at86rf2xx_params[i]);
        if (netif_add(&netif[i],192.168.0.11, 255.255.255.0, 0.0.0.0, &at86rf2xx_devs[i], lwip_netdev_init, //ag
                      tcpip_6lowpan_input) == NULL) { //ag
            DEBUG("Could not add at86rf2xx device\n");
            return;
        }
    }
#elif defined(MODULE_ENC28J60)// ag 
    for(int i=0;i < LWIP_NETIF_NUMOF; i++){ //ag
        enc28j60_setup(&encj28j60_devs[i], &encj28j60_params[i]); //ag
        if( netif_add(&netif[i],192.168.0.11, 255.255.255.0, 0.0.0.0, &enc28j60_devs[i], lwip_netdev_init, //ag
             tcpip_6lowpan_input )==NULL){ //ag
                 DEBUG("Could not add enc28j60 device");//ag
             } //ag
    }
#endif
    if (netif[0].state != NULL) {
        /* state is set to a netdev_t in the netif_add() functions above */
        netif_set_default(&netif[0]);
    }
#endif
    /* also allow for external interface definition */
    tcpip_init(NULL, NULL);
}

/** @} */
