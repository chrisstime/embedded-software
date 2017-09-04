/*
 * FTM.c
 *
 *  Created on: 4 Sep 2017
 *      Author: 11970744
 */

#include "LEDs.h"
#include "FTM.h"
#include "PE_Types.h"
#include "MK70F12.h"
#include "Cpu.h"

bool FTM_Init()
{
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

  FTM0_CNTIN = ~FTM_CNTIN_INIT_MASK;
  FTM0_MOD = FTM_MOD_MOD_MASK;
  FTM0_CNT = ~FTM_CNT_COUNT_MASK;
  FTM0_SC |= FTM_SC_CLKS(2);

  NVICICPR1 = (1<<(62 % 32));
  NVICISER1 = (1<<(62 % 32));

return true;
}


bool FTM_Set(const TFTMChannel* const aFTMChannel)
{
				FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSB_MASK;
				FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSA_MASK;

				FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK; // set 0
				FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK; // set 1

		return true;
}

bool FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{
			FTM0_CnV(aFTMChannel->channelNb) = FTM0_CNT + aFTMChannel->delayCount;	// Sets the channels initial countss

				FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHF_MASK;

			FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_CHIE_MASK; //enables channel interrupts

			return true; //Timer successfully initialised.

}
void __attribute__ ((interrupt)) FTM0_ISR(void)
    {
  if(FTM0_C0SC & FTM_CnSC_CHF_MASK && FTM_CnSC_CHIE_MASK)
    {
      //FTM0_CnSC(aFTMChannel.channelNb) 	&= ~FTM_CnSC_CHF_MASK;
      FTM0_MODE				&= ~FTM_MODE_FTMEN_MASK;
      FTM_BLED_Off(0);
    }

    }
