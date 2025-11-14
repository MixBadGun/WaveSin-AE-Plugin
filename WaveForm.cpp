#include "WaveSin.h"
#include "WaveForm.h"

#define PI 3.14159265358979323846

float sharpFunc(float angle){
	// 一个周期函数，从 0 开始，周期是 2π, 在每个周期内，前半段从-1上升到1，后半段从1下降到-1
	angle = fmod(angle, 2 * PI); // 将角度转换到 [0, 2π] 范围内
	if (angle < PI){
		return (angle / PI) * 2 - 1; // 前半段
	} else {
		return ((2 * PI - angle) / PI) * 2 - 1; // 后半段
	}
}

float sineFunc(float angle){
    // 标准正弦函数，周期是 2π
    return sin(angle);
}

float squareFunc(float angle){
    // 方波函数，周期是 2π
    angle = fmod(angle, 2 * PI); // 将角度转换到 [0, 2π] 范围内
    return (angle < PI) ? 1.0f : -1.0f;
}

float GetWaveFuncValue(int waveFormType, float angle){
    switch(waveFormType){
        case 1:
            return sineFunc(angle);
        case 2:
            return squareFunc(angle);
        case 3:
            return sharpFunc(angle);
        default:
            return sineFunc(angle); // 默认使用正弦函数
    }
}