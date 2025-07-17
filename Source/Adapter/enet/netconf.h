#pragma once
#ifndef NETCONF_H
#define NETCONF_H
#include "SemaphoreCPP.h"
#include "TaskCPP.h"
#include "enet_hal.h"
#include "netcfg.h"

#define ETH_TASK_DEPTH 1024

class EthDevice {
   private:
    struct netif g_mynetif;
    bool initialized = false;

   public:
    int init();
    static void lwip_netif_status_callback(struct netif* netif);
    void lwip_stack_init(void);
};

#endif /* NETCONF_H */
