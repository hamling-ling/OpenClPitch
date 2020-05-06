#ifndef _PEAKDETECTMACHINECOMMON_H_
#define _PEAKDETECTMACHINECOMMON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define	kKeyMax	16

typedef __fp16 osk_float_t;

typedef enum {
	kStateSearchingBell,
	kStateWalkingOnBell,
	kStateEnd,
	kStateNum
} PeakDetectMachineState_t;

typedef enum {
	kEventPosCross,
	kEventNegCross,
	kEventNmlData,
	kEventEndOfData,
	kEventNum
} PeakDetectMachineEvent_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

