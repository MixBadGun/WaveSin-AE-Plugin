#pragma once

typedef enum {
	StrID_NONE, 
	StrID_Name,
	StrID_Description,
	StrID_Anchor_Param_Name,
	StrID_Gain_Param_Name,
	StrID_Wave_Width_Param_Name,
	StrID_Offset_Param_Name,
	StrID_Reverse_Direction_Param_Name,
	StrID_Wave_Form_Param_Name,
	StrID_Wave_Form_Choices,
	StrID_NUMTYPES
} StrIDType;

char	*GetStringPtr(int strNum, A_char* lang_tagZ);
