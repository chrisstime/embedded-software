/*
 * FTM.c
 *
 *  Created on: 4 Sep 2017
 *      Author: 11970744
 */

//#include "LEDs.h"
#include "FTM.h"
#include "PE_Types.h"
#include "MK70F12.h"
#include "Cpu.h"

bool FTM_Init()
{
  //set clock mask
  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

  //free running clock - 43.4.3.3
  FTM0_CNTIN 		= ~FTM_CNTIN_INIT_MASK;
  FTM0_MOD 			= FTM_MOD_MOD_MASK;
  FTM0_CNT 			= ~FTM_CNT_COUNT_MASK;
  FTM0_SC 			|= FTM_SC_CLKS(2);
  FTM0_CnSC(0) 		|= FTM_CnSC_CHF_MASK;

  FTM0_MODE			|= FTM_MODE_FTMEN_MASK;
  //NVIC
  NVICICPR1 		= (1<<(62 % 32));
  NVICISER1 		= (1<<(62 % 32));
  return true;
}


bool FTM_Set(const TFTMChannel* const aFTMChannel)
{
  EnterCritical();
  if(aFTMChannel->timerFunction == TIMER_FUNCTION_OUTPUT_COMPARE)
  {
	  //43.3.6 Channel (n) Status And Control (FTMx_CnSC)
	  //set output on match
	  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSB_MASK;
	  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSA_MASK;

	  //set output compare
	  FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK; // set 0
	  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK; // set 1
  }
  ExitCritical();

  return true;
}

bool FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{
  EnterCritical();

  //set delay in channel 0
  FTM0_CnV(aFTMChannel->channelNb) = FTM0_CNT + aFTMChannel->delayCount;
  //ensure there is no flag set
  FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHF_MASK;
  //set interrupts on channel 0
  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_CHIE_MASK;

  ExitCritical();

  return true; //Timer successfully initialised.


}
void __attribute__ ((interrupt)) FTM0_ISR(void)
{
	int i;
	for (i=0; i<8 ; i++)
	{
	  if(FTM0_CnSC(i) & FTM_CnSC_CHF_MASK && FTM_CnSC_CHIE_MASK)
		{
		  //clear mask
		  FTM0_CnSC(i) 		&= ~FTM_CnSC_CHF_MASK;
		  FTM_BLED_Off(0);
		}
	}

}
