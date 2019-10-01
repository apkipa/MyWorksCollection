#pragma once

#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>

//Bitmap pixel is 32-bit real color by default

typedef struct tagEasyDraw_Bitmap *pEasyDraw_Bitmap;
typedef uint32_t EasyDraw_Color;

#define EasyDraw_GetLowByte(data) ((uint8_t)((uint32_t)(data) & 0xff))
#define EasyDraw_MakeArgb(a, r, g, b)	\
	((EasyDraw_Color)((((uint8_t)(b) | ((uint16_t)((uint8_t)(g)) << 8)) | (((uint32_t)(uint8_t)(r)) << 16)) | (((uint32_t)(uint8_t)(a)) << 24)))
#define EasyDraw_GetArgbA(clrArgb) (EasyDraw_GetLowByte((clrArgb) >> 24))
#define EasyDraw_GetArgbR(clrArgb) (EasyDraw_GetLowByte((clrArgb) >> 16))
#define EasyDraw_GetArgbG(clrArgb) (EasyDraw_GetLowByte(((uint16_t)(clrArgb)) >> 8))
#define EasyDraw_GetArgbB(clrArgb) (EasyDraw_GetLowByte(clrArgb))
#define EasyDraw_BlendArgb(clr1, clr2, wClr1) EasyDraw_MakeArgb(								\
		(EasyDraw_GetArgbA(clr1) * (wClr1) + EasyDraw_GetArgbA(clr2) * (255 - (wClr1))) / 255,	\
		(EasyDraw_GetArgbR(clr1) * (wClr1) + EasyDraw_GetArgbR(clr2) * (255 - (wClr1))) / 255,	\
		(EasyDraw_GetArgbG(clr1) * (wClr1) + EasyDraw_GetArgbG(clr2) * (255 - (wClr1))) / 255,	\
		(EasyDraw_GetArgbB(clr1) * (wClr1) + EasyDraw_GetArgbB(clr2) * (255 - (wClr1))) / 255	\
	)

pEasyDraw_Bitmap EasyDraw_CreateBitmap(uint32_t nWidth, uint32_t nHeight);
bool EasyDraw_DestroyBitmap(pEasyDraw_Bitmap pBitmap);
bool EasyDraw_DrawOntoDC(HDC hdc, pEasyDraw_Bitmap pBitmap, int xDest, int yDest, int xSource, int ySource, int nWidth, int nHeight);
bool EasyDraw_FillRectangle(pEasyDraw_Bitmap pBitmap, double x1, double y1, double x2, double y2, uint32_t clr);
intptr_t EasyDraw_BeginMotionBlur(pEasyDraw_Bitmap pBitmap);
bool EasyDraw_MotionBlur_AddBitmap(intptr_t nData, pEasyDraw_Bitmap pBitmap);
bool EasyDraw_FinishMotionBlur(intptr_t nData, pEasyDraw_Bitmap pBitmapOutput);