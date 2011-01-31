/*
 * Author: Gregory Haynes <greg@greghaynes.net>
 *
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2011, Gregory Haynes
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "pwm16.h"
#include "../config.h"

void pwm16InitTimers(int timers)
{
	if(timers & PWM16_TIMER0)
	{
		/* Enable Timer */
		SCB_SYSAHBCLKCTRL |= SCB_SYSAHBCLKCTRL_CT16B0;

		TMR_TMR16B0MCR = (TMR_TMR16B0MCR_MR3_RESET_ENABLED);

		/* External Match Register Settings for PWM */
		TMR_TMR16B0EMR = TMR_TMR16B0EMR_EMC0_TOGGLE | TMR_TMR16B0EMR_EM0;

		/* Disable counter */
		TMR_TMR16B0TCR &= ~TMR_TMR16B0TCR_COUNTERENABLE_MASK;

		NVIC_EnableIRQ(TIMER_16_0_IRQn);
	}
	if(timers & PWM16_TIMER1)
	{
		/* Enable Timer */
		SCB_SYSAHBCLKCTRL |= SCB_SYSAHBCLKCTRL_CT16B1;

		TMR_TMR16B1MCR = (TMR_TMR16B1MCR_MR3_RESET_ENABLED);

		/* External Match Register Settings for PWM */
		TMR_TMR16B1EMR = TMR_TMR16B1EMR_EMC0_TOGGLE | TMR_TMR16B1EMR_EM0;

		/* Disable counter */
		TMR_TMR16B1TCR &= ~TMR_TMR16B1TCR_COUNTERENABLE_MASK;

		NVIC_EnableIRQ(TIMER_16_1_IRQn);
	}
}

void pwm16InitPins(int pins)
{
	if(pins & PWM16_PIN0_0)
	{
		IOCON_PIO0_8 &= ~IOCON_PIO0_8_FUNC_MASK;
		IOCON_PIO0_8 |= IOCON_PIO0_8_FUNC_CT16B0_MAT0;

		TMR_TMR16B0PWMC |= TMR_TMR16B0PWMC_PWM0_ENABLED;
	}
}

void pwm16StartTimers(int timers)
{
	if(timers & PWM16_TIMER0)
	{
		TMR_TMR16B0MCR  &= ~(TMR_TMR16B0MCR_MR3_INT_MASK);
		TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERENABLE_ENABLED;
	}
	if(timers & PWM16_TIMER1)
	{
		TMR_TMR16B1MCR  &= ~(TMR_TMR16B1MCR_MR3_INT_MASK);
		TMR_TMR16B1TCR = TMR_TMR16B1TCR_COUNTERENABLE_ENABLED;
	}
}

void pwm16SetTimerPrescaler(int timer, int value)
{
	if(timer == PWM16_TIMER0)
		TMR_TMR16B0PR = value;
	else if(timer == PWM16_TIMER1)
		TMR_TMR16B1PR = value;
}

void pwm16SetFrequencyInTicks(int pins, int value)
{
	if(pins & PWM16_PIN0_0)
		TMR_TMR16B0MR3 = value;
}

void pwm16SetDutyCycleInTicks(int pins, int value)
{
	if(pins & PWM16_PIN0_0)
		TMR_TMR16B0MR0 = value;
}
