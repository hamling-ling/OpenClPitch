#ifndef _PEAKDETECTMACHINE_H_
#define _PEAKDETECTMACHINE_H_

#include "PeakDetectMachineCommon.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	typedef struct _PaekInfo_t {
		float    value;
		uint16_t index;
	} PeakInfo_t;

	//struct _MachineContext_t;
	typedef struct _MachineContext_t MachineContext_t;

	MachineContext_t* CreatePeakDetectMachineContext();
	void DestroyPeakDetectMachineContext(MachineContext_t* ctx);
	void Input(MachineContext_t* ctx, const float x);
	void ResetMachine(MachineContext_t* ctx);
	void GetKeyMaximums(MachineContext_t* ctx, const float filter, PeakInfo_t* list, const int listmaxlen, int *num);
	bool ParabolicInterp(MachineContext_t* ctx, const int index, const osk_float_t* xs, const int N, float* x);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
