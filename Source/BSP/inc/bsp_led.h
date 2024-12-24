/**
 * @file led.h
 * @author wang.yunlong (wang.yunlong9@byd.com)
 * @brief
 * @version 0.1
 * @date 2024-10-22
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "gd32f4xx.h"

#ifdef __cplusplus
}
#endif

class LED {
   private:
    uint32_t port;
    uint32_t pin;

   public:
    LED(uint32_t port, uint32_t pin);
    ~LED();
    void on(void);
    void off(void);
    void toggle(void);
};