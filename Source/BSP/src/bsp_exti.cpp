#include "bsp_exti.hpp"
ExtiBase* ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI_NUM];
bool ExtiBase::is_bsp_init = false;
extern "C" {
    void EXTI0_IRQHandler(void){
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI0] != nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI0]->irq_handler();
        }
    }
    void EXTI1_IRQHandler(void){
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI1]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI1]->irq_handler();
        } 
    }
    void EXTI2_IRQHandler(void){
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI2]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI2]->irq_handler();
        } 
    }
    void EXTI3_IRQHandler(void){
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI3]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI3]->irq_handler();
        } 
    }
    void EXTI4_IRQHandler(void){
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI4]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI4]->irq_handler(); 
        } 
    }
    void EXTI5_9_IRQHandler(void){
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI5]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI5]->irq_handler();
        } 
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI6]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI6]->irq_handler(); 
        }
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI7]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI7]->irq_handler(); 
        }
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI8]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI8]->irq_handler(); 
        }
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI9]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI9]->irq_handler(); 
        }
    }
    void EXTI10_15_IRQHandler(void){
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI10]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI10]->irq_handler(); 
        } 
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI11]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI11]->irq_handler(); 
        }
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI12]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI12]->irq_handler(); 
        }
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI13]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI13]->irq_handler(); 
        }
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI14]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI14]->irq_handler(); 
        }
        if(ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI15]!= nullptr){
            ExtiBase::exit[ExtiBase::EXTI_LINE::_EXTI15]->irq_handler(); 
        }
    }
}