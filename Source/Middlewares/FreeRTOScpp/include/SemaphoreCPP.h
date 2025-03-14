/**
 * @file SemaphoreCPP.h
 * @brief FreeRTOS Semaphore Wrapper
 *
 * This file contains a set of lightweight wrappers for semaphores using FreeRTOS
 * 
 * @copyright (c) 2007-2024 Richard Damon
 * @author Richard Damon <richard.damon@gmail.com>
 * @parblock
 * MIT License:
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * It is requested (but not required by license) that any bugs found or 
 * improvements made be shared, preferably to the author.
 * @endparblock
 * 
 * @todo Add Counting Semaphores
 * @ingroup FreeRTOSCpp
 */

#ifndef SEMAPHORE_CPP_H
#define SEMAPHORE_CPP_H

#include "FreeRTOScpp.h"
#include "Lock.h"
#include "FreeRTOS.h"
#include "semphr.h"

#if FREERTOSCPP_USE_NAMESPACE
namespace FreeRTOScpp {
#endif


/**
 * @brief Binary Semaphore Wrapper.
 *
 * Example Usage:
 * @code
 * Semaphore sema("MySema");
 *
 * // In some task
 * sema.give();
 *
 * // In some other task
 * sema.take();
 *
 * // In some ISR
 *
 * portBASE_TYPE woken = 0;
 * ...
 * sema.give_ISR(woken);
 * ...
 * portYIELD_FROM_ISR(woken);
 * return;
 *
 * @endcode
 * @ingroup FreeRTOSCpp 
 */

class BinarySemaphore  : public Lockable {
public:
  /**
   * @brief Constructor.
   * @param name Name to give semaphore, used for Debug Registry if setup
   */
  BinarySemaphore(char const* name = nullptr) {
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
	sema = xSemaphoreCreateBinaryStatic(&semaBuffer);
#else
	sema = xSemaphoreCreateBinary();
#endif
#if configQUEUE_REGISTRY_SIZE > 0
	if(name)
	  vQueueAddToRegistry(sema, name);
#endif
  }
  /**
   * @brief Destructor.
   *
   * Delete the semaphore.
   */
  ~BinarySemaphore() {
    vQueueDelete(sema);
  }
  /**
   * @brief Give the Semaphore.
   */
  bool give() override {
    return xSemaphoreGive(sema);
  }

  /**
   * @brief Take the semaphore.
   *
   * @param delay The number of ticks to wait for the semaphore
   */
  bool take(TickType_t delay = portMAX_DELAY) override {
    return xSemaphoreTake(sema, delay);
  }
  bool take_ISR(portBASE_TYPE& waswoken) {
    return xSemaphoreTakeFromISR(sema, &waswoken);
  }

 #if FREERTOSCPP_USE_CHRONO
  /**
   * @brief Take the semaphore.
   *
   * @param delay The number of ticks to wait for the semaphore
   */
  bool take(Time_ms delay){
    return xSemaphoreTake(sema, ms2ticks(delay));
  }
#endif
  /**
   * @brief Give the Semaphore inside an ISR
   *
   * @param waswoken The flag variable used to indicate if we need to run the 
   * scheduler when we exit the ISR.
   */
  bool give_ISR(portBASE_TYPE& waswoken) {
    return xSemaphoreGiveFromISR(sema, &waswoken);
  }
private:
  SemaphoreHandle_t sema;

#if __cplusplus < 201101L
    Semaphore(Semaphore const&);      ///< We are not copyable.
    void operator =(Semaphore const&);  ///< We are not assignable.
#else
    BinarySemaphore(BinarySemaphore const&) = delete;      ///< We are not copyable.
    void operator =(BinarySemaphore const&) = delete;  ///< We are not assignable.
#endif // __cplusplus

#if( configSUPPORT_STATIC_ALLOCATION == 1 )
    StaticSemaphore_t semaBuffer;
#endif

};

typedef BinarySemaphore Semaphore [[deprecated("Rename to BinarySemaphore")]];
#if FREERTOSCPP_USE_NAMESPACE
}   // namespace FreeRTOScpp
#endif


#endif
