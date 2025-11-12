/*	
WaveSin.cpp	
*/

#include "WaveSin.h"
#include "WaveForm.cpp"
#include <Param_Utils.h>

static A_char lang_tagZ[PF_APP_LANG_TAG_SIZE - 1];

#define	STR(_foo)	GetStringPtr(_foo, lang_tagZ)

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name), 
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	suites.AppSuite6()->PF_AppGetLanguage(lang_tagZ);

	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

	out_data->out_flags |=  PF_OutFlag_DEEP_COLOR_AWARE | PF_OutFlag_I_EXPAND_BUFFER;	// just 16bpc, not 32bpc

	out_data->out_flags2 |= PF_OutFlag2_SUPPORTS_THREADED_RENDERING;

	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;

	PF_ParamDef	def;	

	AEFX_CLR_STRUCT(def);
	// 锚点
	PF_ADD_POINT(STR(StrID_Anchor_Param_Name),
		50,
		50,
		0,
		ANCHOR_DISK_ID
	);

	AEFX_CLR_STRUCT(def);
	// 波形高度
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Gain_Param_Name), 
							WAVESIN_GAIN_MIN, 
							WAVESIN_GAIN_MAX, 
							0, 
							1000, 
							WAVESIN_GAIN_DFLT,
							PF_Precision_HUNDREDTHS,
							0,
							0,
							GAIN_DISK_ID);

	AEFX_CLR_STRUCT(def);
	// 波形宽度
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Wave_Width_Param_Name), 
							0, 
							1000, 
							0, 
							20, 
							2,
							PF_Precision_HUNDREDTHS,
							0,
							0,
							WAVE_WIDTH_ID);

	AEFX_CLR_STRUCT(def);

	// 偏移量
	PF_ADD_ANGLE(
		STR(StrID_Offset_Param_Name),
		0,
		OFFSET_ID
	);

	AEFX_CLR_STRUCT(def);
	// 反转方向
	PF_ADD_CHECKBOXX(	
					STR(StrID_Reverse_Direction_Param_Name),
					FALSE,
					0,
					REVERSE_DIRECTION_ID);

	AEFX_CLR_STRUCT(def);
	// 函数形选择
	PF_ADD_POPUP(
		STR(StrID_Wave_Form_Param_Name),
		3,
		1,
		STR(StrID_Wave_Form_Choices),
		WAVE_FORM_ID
	);
	
	out_data->num_params = WAVESIN_NUM_PARAMS;

	return err;
}

void CalcWavePixel(
	PF_Fixed oriX,
	PF_Fixed oriY,
	PF_Fixed anchorX,
	PF_Fixed anchorY,
	PF_FpLong gain,
	PF_FpLong waveWidth,
	PF_FpLong angleOffset,
	bool reverseDirection,
	A_long waveForm,
	PF_FpLong downSampleScale,
	PF_Fixed *new_xFi,
	PF_Fixed *new_yFi
)
{
	// 计算原点位置相对于锚点的位置
	PF_FpLong deltaX = FIX_2_FLOAT(oriX - anchorX);
	PF_FpLong deltaY = FIX_2_FLOAT(oriY - anchorY);
	// 计算长度
	PF_FpLong length = sqrt(deltaX * deltaX + deltaY * deltaY);
	// 根据偏移值计算角度
	PF_FpLong angle = atan2(deltaY, deltaX) - fmod(angleOffset * (PI / 180.0), 2 * PI);
	while(angle < 0){
		angle += 2 * PI;
	}
	// 根据角度计算波动程度
	PF_FpLong waveOffset = 0.5 * (GetWaveFuncValue(waveForm, angle * waveWidth) + 1) * gain;
	if(reverseDirection){
		waveOffset = -waveOffset;
	}
	// 新的像素位置，就是沿着原点到锚点的方向，移动波动程度后的坐标
	*new_xFi = FLOAT2FIX(FIX_2_FLOAT(oriX) + waveOffset * (deltaX / length) * downSampleScale);
	*new_yFi = FLOAT2FIX(FIX_2_FLOAT(oriY) + waveOffset * (deltaY / length) * downSampleScale);
}

static PF_Err
MySimpleGainFunc16 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel16	*inP, 
	PF_Pixel16	*outP)
{
	PF_Err		err = PF_Err_NONE;

	WaveInfo	*giP	= reinterpret_cast<WaveInfo*>(refcon);
	PF_FpLong	tempF	= 0;
	
	PF_Fixed	new_xFi = 0;
	PF_Fixed	new_yFi = 0;

	PF_FpLong offset_anchorX = giP->anchorX + (giP->in_data->output_origin_x << 16);
	PF_FpLong offset_anchorY = giP->anchorY + (giP->in_data->output_origin_y << 16);

	// 下采样比例
	PF_FpLong downSampleScale = (static_cast<PF_FpLong>(giP->in_data->downsample_x.num) / giP->in_data->downsample_x.den);

	CalcWavePixel(	INT2FIX(xL),
					INT2FIX(yL),
					offset_anchorX,
					offset_anchorY,
					giP->gain,
					giP->waveWidth,
					giP->offset,
					giP->reverseDirection,
					giP->waveForm,
					downSampleScale,
					&new_xFi,
					&new_yFi);
	new_xFi -= giP->in_data->output_origin_x << 16;
	new_yFi -= giP->in_data->output_origin_y << 16;
	ERR(giP->in_data->utils->subpixel_sample16(giP->in_data->effect_ref,
                                            new_xFi,
                                            new_yFi,
                                            &giP->samp_pb,
                                            outP));

	return err;
}

static PF_Err
MySimpleGainFunc8 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;

	WaveInfo	*giP	= reinterpret_cast<WaveInfo*>(refcon);
	PF_FpLong	tempF	= 0;
	
	PF_Fixed	new_xFi = 0;
	PF_Fixed	new_yFi = 0;

	PF_FpLong offset_anchorX = giP->anchorX + (giP->in_data->output_origin_x << 16);
	PF_FpLong offset_anchorY = giP->anchorY + (giP->in_data->output_origin_y << 16);

	// 下采样比例
	PF_FpLong downSampleScale = (static_cast<PF_FpLong>(giP->in_data->downsample_x.num) / giP->in_data->downsample_x.den);

	CalcWavePixel(	INT2FIX(xL),
					INT2FIX(yL),
					offset_anchorX,
					offset_anchorY,
					giP->gain,
					giP->waveWidth,
					giP->offset,
					giP->reverseDirection,
					giP->waveForm,
					downSampleScale,
					&new_xFi,
					&new_yFi);
	new_xFi -= giP->in_data->output_origin_x << 16;
	new_yFi -= giP->in_data->output_origin_y << 16;
	ERR(giP->in_data->utils->subpixel_sample(giP->in_data->effect_ref,
                                            new_xFi,
                                            new_yFi,
                                            &giP->samp_pb,
                                            outP));

	return err;
}

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	WaveInfo			giP;
	AEFX_CLR_STRUCT(giP);
	A_long				linesL	= 0;

	giP.in_data = in_data;
	giP.anchorX = params[WAVESIN_ANCHOR]->u.td.x_value;
	giP.anchorY = params[WAVESIN_ANCHOR]->u.td.y_value;
	giP.samp_pb.src = &params[WAVESIN_INPUT]->u.ld;
	giP.gain = params[WAVESIN_GAIN]->u.fs_d.value;
	giP.waveWidth = params[WAVESIN_WAVE_WIDTH]->u.fs_d.value;
	giP.offset = FIX_2_FLOAT(params[WAVESIN_OFFSET]->u.ad.value);
	giP.reverseDirection = params[WAVESIN_REVERSE_DIRECTION]->u.bd.value;
	giP.waveForm = params[WAVESIN_WAVE_FORM]->u.pd.value;

	PF_Rect	src_rectR =	{0,0,0,0};
	PF_Rect	dst_rectR =	{0,0,0,0};
	
	if (in_data->appl_id != 'PrMr'){
	// 将输入复制到画面中央
		PF_LayerDef *input_layer = &params[WAVESIN_INPUT]->u.ld;
		dst_rectR.left = static_cast<A_short>(in_data->output_origin_x);
		dst_rectR.top = static_cast<A_short>(in_data->output_origin_y);
		dst_rectR.right = static_cast<short>(dst_rectR.left + input_layer->width);
		dst_rectR.bottom = static_cast<short>(dst_rectR.top + input_layer->height);
	}

	if (PF_Quality_HI == in_data->quality && in_data->appl_id != 'PrMr'){	
		ERR(suites.WorldTransformSuite1()->copy_hq(
			in_data->effect_ref,
			&params[WAVESIN_INPUT]->u.ld,
			output,
			NULL,
			&dst_rectR));
	} else if (in_data->appl_id != 'PrMr'){ 
		ERR(suites.WorldTransformSuite1()->copy(
			in_data->effect_ref,
			&params[WAVESIN_INPUT]->u.ld,
			output,
			NULL,
			&dst_rectR));
	} else {
		// For PPro
		PF_FpLong border_x, border_y = 0;
		PF_FpLong border_width = params[WAVESIN_WAVE_WIDTH]->u.fs_d.value;
		border_x = border_width * (static_cast<PF_FpLong>(in_data->downsample_x.num) / in_data->downsample_x.den);
		border_y = border_width * (static_cast<PF_FpLong>(in_data->downsample_y.num) / in_data->downsample_y.den);

		src_rectR.left = in_data->pre_effect_source_origin_x;
		src_rectR.top = in_data->pre_effect_source_origin_y;
		src_rectR.right = in_data->pre_effect_source_origin_x + in_data->width;
		src_rectR.bottom = in_data->pre_effect_source_origin_y + in_data->height;

		dst_rectR.left = in_data->pre_effect_source_origin_x + static_cast<A_long>(border_x);
		dst_rectR.top = in_data->pre_effect_source_origin_y + static_cast<A_long>(border_y);
		dst_rectR.right = in_data->pre_effect_source_origin_x + in_data->width + static_cast<A_long>(border_x);
		dst_rectR.bottom = in_data->pre_effect_source_origin_y + in_data->height + static_cast<A_long>(border_y);

		ERR(PF_COPY(&params[WAVESIN_INPUT]->u.ld,
					output,
					&src_rectR,
					&dst_rectR));
	}

	linesL = output->extent_hint.bottom - output->extent_hint.top;

	if (PF_WORLD_IS_DEEP(output)){
		ERR(suites.Iterate16Suite2()->iterate(	in_data,
												0,								// progress base
												linesL,							// progress final
												output,	// src 
												NULL,							// area - null for all pixels
												(void*)&giP,					// refcon - your custom data pointer
												MySimpleGainFunc16,				// pixel function pointer
												output));
	} else {
		ERR(suites.Iterate8Suite2()->iterate(	in_data,
												0,								// progress base
												linesL,							// progress final
												output,	// src 
												NULL,							// area - null for all pixels
												(void*)&giP,					// refcon - your custom data pointer
												MySimpleGainFunc8,				// pixel function pointer
												output));	
	}

	return err;
}

static 
PF_Err 
FrameSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	// 如果反转开关开启，那么就需要扩展边缘像素
	PF_Err	err = PF_Err_NONE;

	if(!params[WAVESIN_REVERSE_DIRECTION]->u.bd.value){
		return err;
	}

	PF_FpLong offsetX = params[WAVESIN_GAIN]->u.fs_d.value;	
	PF_FpLong offsetY = params[WAVESIN_GAIN]->u.fs_d.value;
	
    PF_LayerDef *input_layer = &params[WAVESIN_INPUT]->u.ld;
    out_data->width = input_layer->width + static_cast<A_long>(offsetX) * 2;
    out_data->height = input_layer->height + static_cast<A_long>(offsetY) * 2;

    out_data->origin.h = static_cast<A_short>(offsetX);
    out_data->origin.v = static_cast<A_short>(offsetY);

    return err;
}

extern "C" DllExport
PF_Err PluginDataEntryFunction2(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB2 inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT_EXT2(
		inPtr,
		inPluginDataCallBackPtr,
		"WaveSin", // Name
		"ADBE WaveSin", // Match Name
		"Distort", // Category
		AE_RESERVED_INFO, // Reserved Info
		"EffectMain",	// Entry point
		"https://www.otm.ink/");	// support URL

	return result;
}

PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;
			
			case PF_Cmd_FRAME_SETUP:

				err = FrameSetup(	in_data,
									out_data,
									params,
									output);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

