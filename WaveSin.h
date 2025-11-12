/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007-2023 Adobe Inc.                                  */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Inc. and its suppliers, if                    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Inc. and its                    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Inc.            */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*
	WaveSin.h
*/

#pragma once

#ifndef WAVESIN_H
#define WAVESIN_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#define PF_DEEP_COLOR_AWARE 1	// make sure we get 16bpc pixels; 
								// AE_Effect.h checks for this.

#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "WaveSin_Strings.h"

/* Versioning information */

#define	MAJOR_VERSION	1
#define	MINOR_VERSION	1
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1


/* Parameter defaults */

#define	WAVESIN_GAIN_MIN		0
#define	WAVESIN_GAIN_MAX		100000
#define	WAVESIN_GAIN_DFLT		10

enum {
	WAVESIN_INPUT = 0,
	WAVESIN_ANCHOR,
	WAVESIN_GAIN,
	WAVESIN_WAVE_WIDTH,
	WAVESIN_OFFSET,
	WAVESIN_REVERSE_DIRECTION,
	WAVESIN_WAVE_FORM,
	WAVESIN_NUM_PARAMS
};

enum {
	ANCHOR_DISK_ID = 1,
	GAIN_DISK_ID,
	WAVE_WIDTH_ID,
	OFFSET_ID,
	REVERSE_DIRECTION_ID,
	WAVE_FORM_ID
};

typedef struct WaveInfo{
	PF_InData	*in_data;
	PF_Fixed	anchorX;
	PF_Fixed	anchorY;
	PF_FpLong	waveWidth;
	PF_FpLong 	gain;
	PF_FpLong	offset;
	bool		reverseDirection;
	A_long		waveForm;
	PF_SampPB	samp_pb;
} WaveInfo, *WaveInfoP, **WaveInfoH;


extern "C" {

	DllExport
	PF_Err
	EffectMain(
		PF_Cmd			cmd,
		PF_InData		*in_data,
		PF_OutData		*out_data,
		PF_ParamDef		*params[],
		PF_LayerDef		*output,
		void			*extra);

}

#endif // WAVESIN_H