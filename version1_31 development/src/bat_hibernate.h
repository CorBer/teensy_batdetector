/* TEENSYBAT DETECTOR (for TEENSY 3.6/4.1)

 * Copyright (c) 2018/2019/2020/2021 Cor Berrevoets, registax@gmail.com
 *
 *  CODE ADAPTED FROM MicroSound Recorder for Teensy 3.6 https://github.com/WMXZ-EU/microSoundRecorder
 *  Copyright (c) 2018, Walter Zimmer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef _BAT_HIBERNATE_H
#define _BAT_HIBERNATE_H

 // derived from duff's lowpower modes
 // 06-jun-17: changed some compiler directives
 //            added high speed mode of K66
 // using hybernate disconnects from USB so serial monitor will break (pre TD 1.42)
 //
 // adapated by Cor Berrevoets:
//  based on https://forum.pjrc.com/threads/58484-issue-to-reporogram-T4-0?p=222925&viewfull=1#post222925
// also adapted to follow the current code used in https://github.com/WMXZ-EU/microSoundRecorder audio_hibernate.h

#include "core_pins.h"

#if defined (__IMXRT1062__)
#define SNVS_LPCR_LPTA_EN_MASK          (0x2U)

void rtc_clearAlarm()  //T4.1
    {
    SNVS_LPSR |= 1;
    SNVS_LPCR &= ~SNVS_LPCR_LPTA_EN_MASK;
    while (SNVS_LPCR & SNVS_LPCR_LPTA_EN_MASK);
    SNVS_LPTAR = 0;
    while (SNVS_LPTAR != 0);
    }


void rtc_setAlarm(uint32_t alarmSeconds) //T4.1
    {
    uint32_t tmp = SNVS_LPCR; //save control register

 /* disable SRTC alarm interrupt */
    rtc_clearAlarm();

    SNVS_LPTAR = alarmSeconds;

    while (SNVS_LPTAR != alarmSeconds);
    // D_PRINTXY("ALARM", alarmSeconds);
    // delay(100);
    SNVS_LPCR = tmp | SNVS_LPCR_LPTA_EN_MASK; // restore control register and set alarm
    while (!(SNVS_LPCR & SNVS_LPCR_LPTA_EN_MASK));
    }

uint32_t rtc_getAlarm() //T4.1
    {
    return SNVS_LPTAR;
    }

void setWakeupCallandSleep(uint32_t nsec) //T4.1
    {
    CCM_CCGR5 &= ~CCM_CCGR5_SAI1(CCM_CCGR_ON); //I2SOFF
    uint32_t to = now();
    //D_PRINTXY("TIME", to);
    rtc_setAlarm(to + nsec);
    SNVS_LPCR |= (1 << 6); // turn off power
    while (1) continue;
    }

#elif defined(__MK66FX1M0__)

#define RTC_IER_TAIE_MASK       0x4u
#define RTC_SR_TAF_MASK         0x4u

void rtcSetup() //T3.6
    {
    //System Clock Gating Control Register 6
    SIM_SCGC6 &= ~SIM_SCGC6_I2S; //shutdown I2S Clock Gate Control

    SIM_SCGC6 |= SIM_SCGC6_RTC;// enable RTC clock
    RTC_CR |= RTC_CR_OSCE;// enable RTC
    }

void rtcSetAlarm(uint32_t nsec) //T3.6
    {
    // set alarm nsec seconds in the future
    // RTC setalarm

    RTC_TAR = RTC_TSR + nsec;
    RTC_IER |= RTC_IER_TAIE_MASK; //RTC Interrupt Enable Register
    while (!(RTC_IER & RTC_IER_TAIE_MASK)); //wait until it is set
    }

/********************LLWU**********************************/
#define LLWU_ME_WUME5_MASK       0x20u
#define LLWU_F3_MWUF5_MASK       0x20u
#define LLWU_MF5_MWUF5_MASK      0x20u


static void llwuSetup(void) //T3.6
    {

    LLWU_PE1 = 0;
    LLWU_PE2 = 0;
    LLWU_PE3 = 0;
    LLWU_PE4 = 0;
#if defined(HAS_KINETIS_LLWU_32CH)
    LLWU_PE5 = 0;
    LLWU_PE6 = 0;
    LLWU_PE7 = 0;
    LLWU_PE8 = 0;
#endif
    LLWU_ME = LLWU_ME_WUME5_MASK; //rtc alarm
    //   
    SIM_SOPT1CFG |= SIM_SOPT1CFG_USSWE; // USB voltage regulator stop standby write enable
    SIM_SOPT1 |= SIM_SOPT1_USBSSTBY; // USB regulator standby in Stop, VLPS, LLS and VLLS

    PORTA_PCR0 = PORT_PCR_MUX(0);  //ports at MUX(0) to remove power
    PORTA_PCR1 = PORT_PCR_MUX(0);
    PORTA_PCR2 = PORT_PCR_MUX(0);
    PORTA_PCR3 = PORT_PCR_MUX(0);

    PORTB_PCR2 = PORT_PCR_MUX(0);
    PORTB_PCR3 = PORT_PCR_MUX(0);

    }

/********************* go to deep sleep *********************/
#define SMC_PMPROT_AVLLS_MASK   0x2u
#define SMC_PMCTRL_STOPM_MASK   0x7u
#define SCB_SCR_SLEEPDEEP_MASK  0x4u

// see SMC section (e.g. p 339 of K66) 
#define VLLS3 0x3 // RAM retained I/O states held
#define VLLS2 0x2 // RAM partially retained
#define VLLS1 0x1 // I/O states held
#define VLLS0 0x0 // all stop

static void gotoSleep(void) //T3.6
    {
    //  /* Make sure clock monitor is off so we don't get spurious reset */
    MCG_C6 &= ~MCG_C6_CME0; //disable CLOCK MONITOR ENABLE BIT
    //
 // if K66 is running in highspeed mode (>120 MHz) reduce speed
 // is defined in kinetis.h and mk20dx128c

#if defined(HAS_KINETIS_HSRUN) && F_CPU > 120000000
    kinetis_hsrun_disable();
#endif   

    //SYST_CSR &= ~SYST_CSR_TICKINT;      // disable systick timer interrupt ORIGINAL LOCATION
    //SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK;  // Set the SLEEPDEEP bit to enable deep sleep mode (STOP) ORIGINAL LOCATION
    /* Write to PMPROT to allow all possible power modes */
    SMC_PMPROT = SMC_PMPROT_AVLLS_MASK;
    /* Set the STOPM field to 0b100 for VLLSx mode */
    //SMC_PMCTRL &= ~SMC_PMCTRL_STOPM_MASK; //clear stopmask 7 ORIGINAL
    //SMC_PMCTRL |= SMC_PMCTRL_STOPM(0x4); // set stopmask bit 4 ORIGINAL

    SMC_PMCTRL = SMC_PMCTRL_STOPM(0x4); // just set the PMCTRL to stop in VLLSx
// 000  Normal Stop (STOP)
// 001  Reserved
// 010  Very-Low-Power Stop (VLPS)
// 011  Low-Leakage Stop (LLSx)
// 100  Very-Low-Leakage Stop (VLLSx)

    SMC_VLLSCTRL = SMC_VLLSCTRL_VLLSM(VLLS0);
    /*wait for write to complete to SMC before stopping core */
    (void)SMC_PMCTRL;

    SYST_CSR &= ~SYST_CSR_TICKINT;      // disable systick timer interrupt LOCATION just as in current microsoundrecorder (20210825)
    SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK;  // Set the SLEEPDEEP bit to enable deep sleep mode (STOP)

    asm volatile("wfi");  // WFI instruction will start entry into STOP mode


    // will never return, but wake-up results in call to ResetHandler() in mk20dx128.c
    }

void setWakeupCallandSleep(uint32_t nsec) //T3.6
    {  // set alarm to nsec seconds in future and go to hibernate
    rtcSetup();
    llwuSetup();
    rtcSetAlarm(nsec);
    yield();
    gotoSleep();
    }

#endif
#endif
