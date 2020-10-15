#pragma once
#ifndef _SYS_DELAY_H
#define _SYS_DELAY_H

#include "board.h"
#include "fsl_ftm.h"
#include "fsl_pit.h"
#include "clock_config.h"
#include "pin_mux.h"

#if defined(HITSIC_USE_SYS_DELAY) && (HITSIC_USE_SYS_DELAY>0)

#ifdef __cplusplus
extern "C" {
#endif
#if defined(HITSIC_SYS_DELAY_FREE) && (HITSIC_SYS_DELAY_FREE > 0)
/**
 * @brief	Systick��ʱ����
 *
 * @param  time : ��Ҫ��ʱ��ʱ�������� 
 * Sample usage : SysTick_Delay(1000);  ��ʱ1000���ں�ʱ������ 
 */
void SysTick_Delay(uint32_t time);
/**
 * @brief	���뼶Systick��ʱ����
 *
 * @param  time : ��Ҫ��ʱ��ʱ��
 * Sample usage : SysTick_Delay_Ms(1000);  ��ʱ1000����
 */
void SysTick_Delay_Ms(uint32_t ms);
/**
 * @brief	Systick��ʱ��
 *
 * @param  time : ��ʱʱ��
 * Sample usage : SysTick_Timing(1000);  ��ʱ1000���ں�ʱ������
 */
void SysTick_Timing(uint32_t time);
/**
 * @brief	��ȡ��ǰSystick Timerֵ
 * @return  ��ǰSystick Timerֵ
 *
 * Sample usage : uint32_t SysTick_time = SysTick_Getval(); ��ȡ��ǰSystick Timerֵ  
 */
uin32_t SysTick_Getval(void);
#endif


/**
 * @brief	PITʱ�ӳ�ʼ��
 * @return
 *
 * Sample usage :
 */
void Sys_Pit_My_Init();

/**
 * @brief	����PIT�ж�ʱ��
 * @return
 *
 * Sample usage :
 */
void Sys_Pit_Set_Period(PIT_Type* base, pit_chnl_t channel, uint32_t count);

/**
 * @brief	��ʼ��ʱ
 * @return
 *
 * Sample usage :
 */
void Sys_Pit_Start_Timer(PIT_Type *base, pit_chnl_t channel);

/**
 * @brief	��ͣ��ʱ
 * @return
 *
 * Sample usage :
 */
void Sys_Pit_Stop_Timer(PIT_Type* base, pit_chnl_t channel);

/**
 * @brief	��ñ�־λ
 * @return
 *
 * Sample usage :
 */
void Sys_Pit_Flag_Get(PIT_Type* base, pit_chnl_t channel);
#endif


