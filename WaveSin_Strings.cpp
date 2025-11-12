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

#include "WaveSin.h"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;



TableString		en_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"WaveSin",
	StrID_Description,				"A wave distortion effect based on a center point, made by MixBadGun.",
	StrID_Anchor_Param_Name,		"Anchor",
	StrID_Gain_Param_Name,			"Wave Height",
	StrID_Wave_Width_Param_Name,	"Wave Width",
	StrID_Offset_Param_Name,		"Offset",
	StrID_Reverse_Direction_Param_Name,"Reverse Direction",
	StrID_Wave_Form_Param_Name,		"Wave Form",
	StrID_Wave_Form_Choices,		"Sine|Square|Triangle",
};

TableString		zh_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"WaveSin",
	StrID_Description,				"一个根据中心点进行波浪变形的插件，Made by 坏枪。",
	StrID_Anchor_Param_Name,		"锚点",
	StrID_Gain_Param_Name,			"波形高度",
	StrID_Wave_Width_Param_Name,	"波形宽度",
	StrID_Offset_Param_Name,		"偏移量",
	StrID_Reverse_Direction_Param_Name,"反转方向",
	StrID_Wave_Form_Param_Name,		"波形函数",
	StrID_Wave_Form_Choices,		"正弦波|方波|锯齿波",
};


char	*GetStringPtr(int strNum, A_char* lang_tagZ)
{
	if (lang_tagZ) {
		if (strcmp(lang_tagZ, "zh_CN") == 0){
			return zh_strs[strNum].str;
		}
		if (strcmp(lang_tagZ, "en_US") == 0) {
			return en_strs[strNum].str;
		}
		if (strcmp(lang_tagZ, "fr_FR") == 0) {
			// TODO...
		}
		if (strcmp(lang_tagZ, "de_DE") == 0) {
			// TODO...
		}
		if (strcmp(lang_tagZ, "it_IT") == 0) {
			// TODO...
		}
		if (strcmp(lang_tagZ, "ja_JP") == 0) {
			// TODO...
		}
		if (strcmp(lang_tagZ, "ko_KR") == 0) {
			// TODO...
		}
		if (strcmp(lang_tagZ, "es_ES") == 0) {
			// TODO...
		}
	}
	return en_strs[strNum].str;
}
	