#ifndef TIMER_REGISTER_H
#define TIMER_REGISTER_H

#define TIM_AHB_BASE      (0x00000000)
#define TIM_SCALER        (TIM_AHB_BASE+0x00)
#define TIM_SCRELOAD      (TIM_AHB_BASE+0x04)
#define TIM_CONF          (TIM_AHB_BASE+0x08)
#define TIM_VALUE(nr)     (TIM_AHB_BASE+0x10*(nr+1)+0x0)
#define TIM_RELOAD(nr)    (TIM_AHB_BASE+0x10*(nr+1)+0x4)
#define TIM_CTRL(nr)      (TIM_AHB_BASE+0x10*(nr+1)+0x8)

#define TIM_CONF_DF       9
#define TIM_CONF_SI       8
#define TIM_CONF_IQ_MA    0x000000F8
#define TIM_CONF_IQ_OS    3
#define TIM_CONF_NR_MA    0x00000007
#define TIM_CONF_NR_OS    0

#define TIM_CTRL_DH       6
#define TIM_CTRL_CH       5
#define TIM_CTRL_IP       4
#define TIM_CTRL_IE       3
#define TIM_CTRL_LD       2
#define TIM_CTRL_RS       1
#define TIM_CTRL_EN       0

#endif
