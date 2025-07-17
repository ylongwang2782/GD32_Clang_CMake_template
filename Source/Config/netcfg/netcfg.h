#pragma once
#ifndef MAIN_H
#define MAIN_H

#include "FreeRTOS.h"
#include "gd32f4xx.h"
#include "stdint.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define USE_DHCP       /* enable DHCP, if disabled static address is used */

/* MAC address: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_ADDR0 2
#define MAC_ADDR1 0xA
#define MAC_ADDR2 0xD
#define MAC_ADDR3 0xE
#define MAC_ADDR4 0xF
#define MAC_ADDR5 6

/* static IP address: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0 192
#define IP_ADDR1 168
#define IP_ADDR2 0
#define IP_ADDR3 2

/* remote IP address: IP_S_ADDR0.IP_S_ADDR1.IP_S_ADDR2.IP_S_ADDR3 */
#define IP_S_ADDR0 192
#define IP_S_ADDR1 168
#define IP_S_ADDR2 0
#define IP_S_ADDR3 3

/* net mask */
#define NETMASK_ADDR0 255
#define NETMASK_ADDR1 255
#define NETMASK_ADDR2 255
#define NETMASK_ADDR3 0

/* gateway address */
#define GW_ADDR0 192
#define GW_ADDR1 168
#define GW_ADDR2 0
#define GW_ADDR3 1

/* MII and RMII mode selection */
#define RMII_MODE    // user have to provide the 50 MHz clock by soldering a 50
                     // MHz oscillator
// #define MII_MODE

/* clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef MII_MODE
#define PHY_CLOCK_MCO
#endif

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */
