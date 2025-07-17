#pragma once
#ifndef ENET_H
#define ENET_H

#include "gd32f4xx.h"
#include "netif.h"

class Enet {
   private:
    static __IO uint32_t enet_init_status;

   public:
    static int enet_system_setup(void);
    static void enet_gpio_config(void);
    static void enet_mac_dma_config(void);
    static void nvic_configuration(void);
};

#endif /* ENET_H */
