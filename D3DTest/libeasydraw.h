#pragma once

#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>

//Bitmap pixel is 32-bit real color by default

typedef struct tagEasyDraw_Bitmap *pEasyDraw_Bitmap;
typedef uint32_t EasyDraw_Color;
typedef struct tagEasyDraw_Rect {
	int32_t left, top, right, bottom;
} *pEasyDraw_Rect, EasyDraw_Rect;
typedef struct tagEasyDraw_Brush {
	EasyDraw_Color clr;
	uint8_t nWeight;
} *pEasyDraw_Brush, EasyDraw_Brush;

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
#define EasyDraw_BlendArgb2(clr1, clr2, wClr1) EasyDraw_MakeArgb(											\
		((EasyDraw_GetArgbA(clr1) * (wClr1) + EasyDraw_GetArgbA(clr2) * (255 - (wClr1))) * 0x8081) >> 23,	\
		((EasyDraw_GetArgbR(clr1) * (wClr1) + EasyDraw_GetArgbR(clr2) * (255 - (wClr1))) * 0x8081) >> 23,	\
		((EasyDraw_GetArgbG(clr1) * (wClr1) + EasyDraw_GetArgbG(clr2) * (255 - (wClr1))) * 0x8081) >> 23,	\
		((EasyDraw_GetArgbB(clr1) * (wClr1) + EasyDraw_GetArgbB(clr2) * (255 - (wClr1))) * 0x8081) >> 23	\
	)
#define EasyDraw_BlendArgb3(clr1, clr2, wClr1) EasyDraw_MakeArgb(												\
		((EasyDraw_GetArgbA(clr1) * (wClr1) + EasyDraw_GetArgbA(clr2) * (255 - (wClr1)) + 127) * 0x8081) >> 23,	\
		((EasyDraw_GetArgbR(clr1) * (wClr1) + EasyDraw_GetArgbR(clr2) * (255 - (wClr1)) + 127) * 0x8081) >> 23,	\
		((EasyDraw_GetArgbG(clr1) * (wClr1) + EasyDraw_GetArgbG(clr2) * (255 - (wClr1)) + 127) * 0x8081) >> 23,	\
		((EasyDraw_GetArgbB(clr1) * (wClr1) + EasyDraw_GetArgbB(clr2) * (255 - (wClr1)) + 127) * 0x8081) >> 23	\
	)	//NOTE: BlendArgb3 is BlendArgb2 with rounded result
#define EasyDraw_IsRectEmpty(pRect) ((pRect)->left == (pRect)->right || (pRect)->top == (pRect)->bottom)
#define EasyDraw_Clamp(Val, Min, Max) min(max(Val, Min), Max)
#define EasyDraw_MakeBrush(clrInput, nWeightInput)	\
	((EasyDraw_Brush) { .clr = (EasyDraw_Color)clrInput, .nWeight = (uint8_t)nWeightInput })

pEasyDraw_Bitmap EasyDraw_CreateBitmap(uint32_t nWidth, uint32_t nHeight);
bool EasyDraw_DestroyBitmap(pEasyDraw_Bitmap pBitmap);
uint32_t EasyDraw_GetBitmapWidth(pEasyDraw_Bitmap pBitmap);
uint32_t EasyDraw_GetBitmapHeight(pEasyDraw_Bitmap pBitmap);
bool EasyDraw_DrawOntoDC(HDC hdc, pEasyDraw_Bitmap pBitmap, int xDest, int yDest, int xSource, int ySource, int nWidth, int nHeight);
bool EasyDraw_CopyToMemory(void *pDest, pEasyDraw_Bitmap pBitmap);
bool EasyDraw_ClipRect(pEasyDraw_Bitmap pBitmap, pEasyDraw_Rect pRectInput, pEasyDraw_Rect pOriginalRect);
bool EasyDraw_IsPtInRect(pEasyDraw_Rect pRect, int32_t x, int32_t y);
bool EasyDraw_fFillRectangle(pEasyDraw_Bitmap pBitmap, double x1, double y1, double x2, double y2, pEasyDraw_Brush pBrush);
//Warning: Some characters not implemented, use carefully
bool EasyDraw_DrawBasicTextW(pEasyDraw_Bitmap pBitmap, int32_t x, int32_t y, const wchar_t *str, uint8_t nBkWhiteWeight);