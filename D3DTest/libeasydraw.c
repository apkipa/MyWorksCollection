#define _USE_MATH_DEFINES

#include "libeasydraw.h"

#include <emmintrin.h>
#include <float.h>
#include <stdio.h>
#include <math.h>

#ifdef _MSC_VER
#define restrict __restrict
#endif

#define W "\xff"	//Build white
#define B "\0"		//Build black

//C(Closed): <=
//O(Open): <
#define IsInRangeOO(n, min, max) ((min) < (n) && (n) < (max))
#define IsInRangeCC(n, min, max) ((min) <= (n) && (n) <= (max))
#define IsInRangeCO(n, min, max) ((min) <= (n) && (n) < (max))
#define IsInRangeOC(n, min, max) ((min) < (n) && (n) <= (max))
#define AlignUpward(n, align) (((uintptr_t)(n) + (align) - 1) & ~(uintptr_t)((align) - 1))
#define AlignDownward(n, align) ((uintptr_t)(n) & ~(uintptr_t)((align) - 1))
#define IsPtInEllipse(x, y, a, b) (((x) * (x)) / ((a) * (a)) + ((y) * (y)) / ((b) * (b)) < 1)
#define SolveEllipseX(y, a, b) (((a) * sqrt((b) * (b) - (y) * (y))) / (b))
#define SolveEllipseY(x, a, b) (((b) * sqrt((a) * (a) - (x) * (x))) / (a))
#define If(cond, expTrue, expFalse) ((cond) ? (expTrue) : (expFalse))	//if with Mathematica style

#define USE_SIMD 1

#define swap(a, b) do {							\
		uint8_t __TempData[sizeof(a)];			\
		(void)(&a == &b);						\
		memcpy(__TempData, &(a), sizeof(a));	\
		memcpy(&(a), &(b), sizeof(a));			\
		memcpy(&(b), __TempData, sizeof(a));	\
	} while (0)

typedef struct tagEasyDraw_Bitmap {
	uint32_t nWidth, nHeight;
	EasyDraw_Rect rtClip;

	EasyDraw_Color *pData;
} EasyDraw_Bitmap;

//force floor a double to long long
long long llffloor(double x) {
	return llround(floor(x));
}

//force ceil a double to long long
long long llfceil(double x) {
	return llround(floor(x)) + 1;
}

//force floor a double to long
long lffloor(double x) {
	return lround(floor(x));
}

//force ceil a double to long
long lfceil(double x) {
	return lround(floor(x)) + 1;
}

long lfloor(double x) {
	return lround(floor(x));
}

long lceil(double x) {
	return lround(ceil(x));
}

bool isfltint(double x) {
	return floor(x) == ceil(x);
}

void* memset_16_SSE(void *pDes, uint16_t nValue, size_t nCount) {	//SSE optimized
	uint16_t *pDataRemainTemp;
	__m128i *pmDataTemp;
	int nRemainCount;
	int nLoopCount;
	__m128i mData;

	pDataRemainTemp = (uint16_t*)pDes;

	if (nCount < 8) {
		switch (nCount) {
		case 7:		*pDataRemainTemp++ = nValue;
		case 6:		*pDataRemainTemp++ = nValue;
		case 5:		*pDataRemainTemp++ = nValue;
		case 4:		*pDataRemainTemp++ = nValue;
		case 3:		*pDataRemainTemp++ = nValue;
		case 2:		*pDataRemainTemp++ = nValue;
		case 1:		*pDataRemainTemp = nValue;
		case 0:		break;
		default:	break;
		}
	}
	else {
		nRemainCount = (uint16_t*)(((uintptr_t)pDes + 15) & ~15) - pDataRemainTemp;
		switch (nRemainCount) {
		case 7:		*pDataRemainTemp++ = nValue; nCount--;
		case 6:		*pDataRemainTemp++ = nValue; nCount--;
		case 5:		*pDataRemainTemp++ = nValue; nCount--;
		case 4:		*pDataRemainTemp++ = nValue; nCount--;
		case 3:		*pDataRemainTemp++ = nValue; nCount--;
		case 2:		*pDataRemainTemp++ = nValue; nCount--;
		case 1:		*pDataRemainTemp++ = nValue; nCount--;
		case 0:		break;
		default:	break;
		}

		nLoopCount = nCount >> 3;
		nRemainCount = nCount & 7;

		mData = _mm_set1_epi16(nValue);
		pmDataTemp = (__m128i*)pDataRemainTemp;

		for (int i = 0; i < nLoopCount; i++)
			_mm_store_si128(pmDataTemp + i, mData);

		pDataRemainTemp = (uint16_t*)(pmDataTemp + nLoopCount);
		switch (nRemainCount) {
		case 7:		*pDataRemainTemp++ = nValue;
		case 6:		*pDataRemainTemp++ = nValue;
		case 5:		*pDataRemainTemp++ = nValue;
		case 4:		*pDataRemainTemp++ = nValue;
		case 3:		*pDataRemainTemp++ = nValue;
		case 2:		*pDataRemainTemp++ = nValue;
		case 1:		*pDataRemainTemp = nValue;
		case 0:		break;
		default:	break;
		}
	}

	return pDes;
}
void* memset_32_SSE(void *pDes, uint32_t nValue, size_t nCount) {	//SSE optimized
	uint32_t *pDataRemainTemp;
	__m128i *pmDataTemp;
	int nRemainCount;
	int nLoopCount;
	__m128i mData;

	pDataRemainTemp = (uint32_t*)pDes;

	if (nCount < 4) {
		switch (nCount) {
		case 3:		*pDataRemainTemp++ = nValue;
		case 2:		*pDataRemainTemp++ = nValue;
		case 1:		*pDataRemainTemp = nValue;
		case 0:		break;
		default:	break;
		}
	}
	else {
		nRemainCount = (uint32_t*)(((uintptr_t)pDes + 15) & ~15) - pDataRemainTemp;
		switch (nRemainCount) {
		case 3:		*pDataRemainTemp++ = nValue; nCount--;
		case 2:		*pDataRemainTemp++ = nValue; nCount--;
		case 1:		*pDataRemainTemp++ = nValue; nCount--;
		case 0:		break;
		default:	break;
		}

		nLoopCount = nCount >> 2;
		nRemainCount = nCount & 3;

		mData = _mm_set1_epi32(nValue);
		pmDataTemp = (__m128i*)pDataRemainTemp;

		for (int i = 0; i < nLoopCount; i++)
			_mm_store_si128(pmDataTemp + i, mData);

		pDataRemainTemp = (uint32_t*)(pmDataTemp + nLoopCount);
		switch (nRemainCount) {
		case 3:		*pDataRemainTemp++ = nValue;
		case 2:		*pDataRemainTemp++ = nValue;
		case 1:		*pDataRemainTemp = nValue;
		case 0:		break;
		default:	break;
		}
	}

	return pDes;
}

pEasyDraw_Bitmap EasyDraw_CreateBitmap(uint32_t nWidth, uint32_t nHeight) {
	EasyDraw_Color *pBitmapData;
	pEasyDraw_Bitmap pBitmap;
	size_t nSize;

	nSize = nWidth * nHeight * 4;
	if (nSize == 0)
		return NULL;

	pBitmap = (pEasyDraw_Bitmap)malloc(sizeof(EasyDraw_Bitmap));
	pBitmapData = (EasyDraw_Color*)_mm_malloc(nSize, 32);
	if (pBitmap == NULL || pBitmapData == NULL) {
		free(pBitmap);
		_mm_free(pBitmapData);

		return NULL;
	}

	pBitmap->nWidth = nWidth;
	pBitmap->nHeight = nHeight;
	pBitmap->pData = pBitmapData;
	pBitmap->rtClip = (EasyDraw_Rect) { .left = 0, .top = 0, .right = (int32_t)nWidth, .bottom = (int32_t)nHeight };

	return pBitmap;
}

bool EasyDraw_DestroyBitmap(pEasyDraw_Bitmap pBitmap) {
	if (pBitmap == NULL)
		return false;

	_mm_free(pBitmap->pData);
	free(pBitmap);

	return true;
}

uint32_t EasyDraw_GetBitmapWidth(pEasyDraw_Bitmap pBitmap) {
	return pBitmap == NULL ? 0 : pBitmap->nWidth;
}

uint32_t EasyDraw_GetBitmapHeight(pEasyDraw_Bitmap pBitmap) {
	return pBitmap == NULL ? 0 : pBitmap->nHeight;
}

bool EasyDraw_DrawOntoDC(HDC hdc, pEasyDraw_Bitmap pBitmap, int xDest, int yDest, int xSource, int ySource, int nWidth, int nHeight) {
	int nImageWidth, nImageHeight;
	BITMAPINFO BitmapInfo;
	int nCopySize;
	SIZE sizeCopy;
	void *pData;
	RECT rtCopy;

	if (hdc == NULL || pBitmap == NULL || nWidth < 1 || nHeight < 1)
		return false;

	nImageWidth = pBitmap->nWidth;
	nImageHeight = pBitmap->nHeight;

	rtCopy.left = max(xSource, 0);
	rtCopy.top = max(ySource, 0);
	rtCopy.right = min(xSource + nWidth - 1, nImageWidth - 1);
	rtCopy.bottom = min(ySource + nHeight - 1, nImageHeight - 1);
	sizeCopy.cx = rtCopy.right - rtCopy.left + 1;
	sizeCopy.cy = rtCopy.bottom - rtCopy.top + 1;
	if (sizeCopy.cx < 1 || sizeCopy.cy < 1)
		return false;

	nCopySize = (sizeCopy.cx * sizeCopy.cy) << 2;	//(sizeCopy.cx * sizeCopy.cy) * 4

	pData = (void*)pBitmap->pData;

	BitmapInfo.bmiHeader.biClrImportant = 0;
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biCompression = 0;
	BitmapInfo.bmiHeader.biWidth = nImageWidth;
	BitmapInfo.bmiHeader.biHeight = -nImageHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	BitmapInfo.bmiHeader.biSizeImage = (nImageWidth * nImageHeight) * 4;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;

	SetDIBitsToDevice(
		hdc,
		xDest, yDest,
		sizeCopy.cx, sizeCopy.cy,
		rtCopy.left, rtCopy.top,
		rtCopy.top,
		sizeCopy.cy,
		(uint8_t*)pData + (nImageWidth * ySource) * 4,
		&BitmapInfo,
		DIB_RGB_COLORS
	);

	return true;
}

bool EasyDraw_CopyToMemory(void *pDest, pEasyDraw_Bitmap pBitmap) {
	if (pDest == NULL || pBitmap == NULL)
		return false;
	memcpy(pDest, pBitmap->pData, sizeof(EasyDraw_Color) * (pBitmap->nWidth * pBitmap->nHeight));
	return true;
}

//ClipRect only affects functions which write to the bitmap, not the ones which read from the bitmap
bool EasyDraw_ClipRect(pEasyDraw_Bitmap pBitmap, pEasyDraw_Rect pRectInput, pEasyDraw_Rect pOriginalRect) {
	EasyDraw_Rect rtOriginalCopy;
	EasyDraw_Rect rtInputCopy;

	if (pBitmap == NULL)
		return false;
	if (pRectInput == NULL && pOriginalRect == NULL)
		return false;

	rtOriginalCopy = pBitmap->rtClip;

	if (pRectInput != NULL) {
		rtInputCopy = *pRectInput;
		if (rtInputCopy.left > rtInputCopy.right)
			swap(rtInputCopy.left, rtInputCopy.right);
		if (rtInputCopy.top > rtInputCopy.bottom)
			swap(rtInputCopy.top, rtInputCopy.bottom);

		if (rtInputCopy.left == rtInputCopy.right || rtInputCopy.top == rtInputCopy.bottom) {
			pBitmap->rtClip = (EasyDraw_Rect) { .left = 0, .top = 0, .right = 0, .bottom = 0 };
		}
		else if (rtInputCopy.right <= 0 || rtInputCopy.bottom <= 0) {
			pBitmap->rtClip = (EasyDraw_Rect) { .left = 0, .top = 0, .right = 0, .bottom = 0 };
		}
		else if ((uint32_t)rtInputCopy.left >= pBitmap->nWidth || (uint32_t)rtInputCopy.top >= pBitmap->nHeight) {
			pBitmap->rtClip = (EasyDraw_Rect) { .left = 0, .top = 0, .right = 0, .bottom = 0 };
		}
		else {
			pBitmap->rtClip.left = rtInputCopy.left;
			pBitmap->rtClip.top = rtInputCopy.top;
			pBitmap->rtClip.right = min((uint32_t)rtInputCopy.right, pBitmap->nWidth);
			pBitmap->rtClip.bottom = min((uint32_t)rtInputCopy.bottom, pBitmap->nHeight);
		}
	}

	if (pOriginalRect != NULL)
		*pOriginalRect = rtOriginalCopy;

	return true;
}

bool EasyDraw_IsPtInRect(pEasyDraw_Rect pRect, int32_t x, int32_t y) {
	if (pRect == NULL)
		return false;
	return (pRect->left <= x && x < pRect->right) && (pRect->top <= y && y < pRect->bottom);
}

//NOTE: This function rounds the blending result
static inline void EasyDraw_FillHLine_Blend_sse2(
	EasyDraw_Bitmap *restrict pBitmap,
	uint32_t y,
	uint32_t x1, uint32_t x2,
	EasyDraw_Brush *restrict pBrush
) {
	EasyDraw_Color *pDataTempBegin, *pDataTempEnd;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
	EasyDraw_Color pxTemp;
	uint32_t nLoopCount;
	size_t nOffset;

	if (x2 - x1 < 4) {
		nOffset = y * pBitmap->nWidth + x1;
		pDataTempBegin = &pBitmap->pData[nOffset];
		switch (x2 - x1) {
		case 3:
			pxTemp = *pDataTempBegin;
			*pDataTempBegin = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
			pDataTempBegin++;
		case 2:
			pxTemp = *pDataTempBegin;
			*pDataTempBegin = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
			pDataTempBegin++;
		case 1:
			pxTemp = *pDataTempBegin;
			*pDataTempBegin = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
		}
		return;
	}

	//Blend front pixels
	nOffset = y * pBitmap->nWidth + x1;
	pDataTempBegin = &pBitmap->pData[nOffset];
	switch (AlignUpward(nOffset, 4) - nOffset) {	//We align nOffset here because dest is already aligned
	case 3:
		pxTemp = *pDataTempBegin;
		*pDataTempBegin = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
		pDataTempBegin++;
	case 2:
		pxTemp = *pDataTempBegin;
		*pDataTempBegin = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
		pDataTempBegin++;
	case 1:
		pxTemp = *pDataTempBegin;
		*pDataTempBegin = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
		pDataTempBegin++;
	}

	//Blend back pixels
	nOffset = y * pBitmap->nWidth + x2;
	pDataTempEnd = &pBitmap->pData[nOffset];
	switch (nOffset - AlignDownward(nOffset, 4)) {	//We align nOffset here because dest is already aligned
	case 3:
		pDataTempEnd--;
		pxTemp = *pDataTempEnd;
		*pDataTempEnd = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
	case 2:
		pDataTempEnd--;
		pxTemp = *pDataTempEnd;
		*pDataTempEnd = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
	case 1:
		pDataTempEnd--;
		pxTemp = *pDataTempEnd;
		*pDataTempEnd = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
	}

	//Blend middle pixels
	nLoopCount = (pDataTempEnd - pDataTempBegin) >> 2;	//We divide the result by 4 because sse2 can process 128 bits at a time

	xmm0 = _mm_setzero_si128();	//Zero
	xmm1 = _mm_set1_epi16(0x8081);	//Magic number for division by 255
	xmm2 = _mm_set1_epi16(pBrush->nWeight);	//Weight
	xmm3 = _mm_set1_epi32(pBrush->clr);	//Color to fill
	xmm3 = _mm_unpacklo_epi8(xmm3, xmm0);	//Expand 32-bit color to 64-bit
	xmm3 = _mm_mullo_epi16(xmm3, xmm2);	//Pre-multiply color by weight
	xmm3 = _mm_add_epi16(xmm3, _mm_set1_epi16(127));	//Add result with 127 in order to round final pixels
	xmm2 = _mm_set1_epi16(255 - pBrush->nWeight);	//Invert weight (255 - weight)

	for (uint32_t i = 0; i < nLoopCount; i++) {
		xmm4 = _mm_load_si128((__m128i*)pDataTempBegin + i);	//Load pixels
		xmm5 = _mm_unpackhi_epi8(xmm4, xmm0);	//Expand high part of 32-bit pixels to 64-bit
		xmm4 = _mm_unpacklo_epi8(xmm4, xmm0);	//Expand low part of 32-bit pixels to 64-bit
		xmm4 = _mm_mullo_epi16(xmm4, xmm2);	//Multiply low part of pixels by weight
		xmm5 = _mm_mullo_epi16(xmm5, xmm2);	//Multiply high part of pixels by weight
		xmm4 = _mm_add_epi16(xmm4, xmm3);	//Add low part of pixels with color
		xmm5 = _mm_add_epi16(xmm5, xmm3);	//Add high part of pixels with color
		xmm4 = _mm_srli_epi16(_mm_mulhi_epu16(xmm4, xmm1), 7);	//Divide low part of result by 255
		xmm5 = _mm_srli_epi16(_mm_mulhi_epu16(xmm5, xmm1), 7);	//Divide high part of result by 255
		xmm4 = _mm_packus_epi16(xmm4, xmm5);	//Compress 64-bit pixels to 32-bit
		_mm_store_si128((__m128i*)pDataTempBegin + i, xmm4);	//Store result
	}
}
static inline void EasyDraw_FillHLine(
	EasyDraw_Bitmap *restrict pBitmap,
	uint32_t y,
	uint32_t x1, uint32_t x2,
	EasyDraw_Brush *restrict pBrush
) {
	//Assuming brush is not hollow (weight > 0), fill range [x1, x2)
	//x2 must be within [0, nWidth] except for being called by FillRect
	//Fill area is not limited by clip rect

#if USE_SIMD
	if (pBrush->nWeight == 255) {
		memset_32_SSE(&pBitmap->pData[x1 + y * pBitmap->nWidth], pBrush->clr, x2 - x1);
	}
	else {
		EasyDraw_FillHLine_Blend_sse2(pBitmap, y, x1, x2, pBrush);
	}
#else
	if (pBrush->nWeight == 255) {
		size_t nOffset = y * pBitmap->nWidth;
		for (uint32_t x = x1; x < x2; x++)
			pBitmap->pData[nOffset + x] = pBrush->clr;
	}
	else {
		size_t nOffset = y * pBitmap->nWidth;
		EasyDraw_Color pxTemp;
		for (uint32_t x = x1; x < x2; x++) {
			pxTemp = pBitmap->pData[nOffset + x];
			pBitmap->pData[nOffset + x] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
		}
	}
#endif
}
static inline void EasyDraw_FillHLineClip(
	EasyDraw_Bitmap *restrict pBitmap,
	int32_t y,
	int32_t x1, int32_t x2,
	EasyDraw_Brush *restrict pBrush
) {
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, pBitmap->rtClip.left, y)) {
		EasyDraw_FillHLine(
			pBitmap,
			y,
			EasyDraw_Clamp(x1, pBitmap->rtClip.left, pBitmap->rtClip.right),
			EasyDraw_Clamp(x2, pBitmap->rtClip.left, pBitmap->rtClip.right),
			pBrush
		);
	}
}

static inline void EasyDraw_FillVLine(
	EasyDraw_Bitmap *restrict pBitmap,
	uint32_t x,
	uint32_t y1, uint32_t y2,
	EasyDraw_Brush *restrict pBrush
) {
	//Assuming brush is not hollow (weight > 0), fill range [y1, y2)
	//y2 must be within [0, nHeight]
	//Fill area is not limited by clip rect

	if (pBrush->nWeight == 255) {
		for (uint32_t y = y1; y < y2; y++)
			pBitmap->pData[x + y * pBitmap->nWidth] = pBrush->clr;
	}
	else {
		EasyDraw_Color pxTemp;
		for (uint32_t y = y1; y < y2; y++) {
			size_t nOffset = x + y * pBitmap->nWidth;
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, pBrush->nWeight);
		}
	}
}
static inline void EasyDraw_FillVLineClip(
	EasyDraw_Bitmap *restrict pBitmap,
	int32_t x,
	int32_t y1, int32_t y2,
	EasyDraw_Brush *restrict pBrush
) {
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x, pBitmap->rtClip.top)) {
		EasyDraw_FillVLine(
			pBitmap,
			x,
			EasyDraw_Clamp(y1, pBitmap->rtClip.top, pBitmap->rtClip.bottom),
			EasyDraw_Clamp(y2, pBitmap->rtClip.top, pBitmap->rtClip.bottom),
			pBrush
		);
	}
}

static inline void EasyDraw_FillRect(
	EasyDraw_Bitmap *restrict pBitmap,
	uint32_t x1, uint32_t y1,
	uint32_t x2, uint32_t y2,
	EasyDraw_Brush *restrict pBrush
) {
	//Assuming brush is not hollow (weight > 0), fill range [x1, x2)
	//x2 must be within [0, nWidth]
	//Fill area is not limited by clip rect

	if (x1 == 0 && x2 == pBitmap->nWidth) {
		EasyDraw_FillHLine(pBitmap, y1, 0, pBitmap->nWidth * (y2 - y1), pBrush);
	}
	else {
		for (uint32_t y = y1; y < y2; y++)
			EasyDraw_FillHLine(pBitmap, y, x1, x2, pBrush);
	}
}

static inline bool EasyDraw_fFillRectangle_normal_Special(
	EasyDraw_Bitmap *restrict pBitmap,
	double x1, double y1,
	double x2, double y2,
	EasyDraw_Brush *restrict pBrush
) {
	int32_t x1floor, x1ceil;
	int32_t y1floor, y1ceil;
	int32_t x2floor, x2ceil;
	int32_t y2floor, y2ceil;
	uint32_t xBegin, xEnd;
	uint32_t yBegin, yEnd;
	EasyDraw_Color pxTemp;
	uint32_t nOffset;
	uint8_t nWeight;

	x1floor = lffloor(x1); x1ceil = lfceil(x1);
	y1floor = lffloor(y1); y1ceil = lfceil(y1);
	x2floor = lffloor(x2); x2ceil = lfceil(x2);
	y2floor = lffloor(y2); y2ceil = lfceil(y2);

	if (x1floor == x2floor && y1floor == y2floor) {
		if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x1floor, y1floor)) {	//(x1, y1)
			nOffset = x1floor + pBitmap->nWidth * y1floor;
			nWeight = (uint8_t)lround((x2 - x1) * (y2 - y1) * pBrush->nWeight);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
		}
	}
	else if (x1floor == x2floor) {
		if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x1floor, y1floor)) {
			nOffset = x1floor + pBitmap->nWidth * y1floor;
			nWeight = (uint8_t)lround((x2 - x1) * (y1ceil - y1) * pBrush->nWeight);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
		}
		if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x1floor, y2floor)) {
			nOffset = x1floor + pBitmap->nWidth * y2floor;
			nWeight = (uint8_t)lround((x2 - x1) * (y2 - y2floor) * pBrush->nWeight);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
		}
		if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x1floor, pBitmap->rtClip.top)) {
			yBegin = EasyDraw_Clamp(y1floor + 1, pBitmap->rtClip.top, pBitmap->rtClip.bottom);
			yEnd = EasyDraw_Clamp(y2floor, pBitmap->rtClip.top, pBitmap->rtClip.bottom);
			EasyDraw_FillVLine(pBitmap, x1floor, yBegin, yEnd, &EasyDraw_MakeBrush(pBrush->clr, lround((x2 - x1) * pBrush->nWeight)));
		}
	}
	else if (y1floor == y2floor) {
		if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x1floor, y1floor)) {
			nOffset = x1floor + pBitmap->nWidth * y1floor;
			nWeight = (uint8_t)lround((x1ceil - x1) * (y2 - y1) * pBrush->nWeight);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
		}
		if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x2floor, y1floor)) {
			nOffset = x2floor + pBitmap->nWidth * y1floor;
			nWeight = (uint8_t)lround((x2 - x2floor) * (y2 - y1) * pBrush->nWeight);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
		}
		if (EasyDraw_IsPtInRect(&pBitmap->rtClip, pBitmap->rtClip.left, y1floor)) {
			xBegin = EasyDraw_Clamp(x1floor + 1, pBitmap->rtClip.left, pBitmap->rtClip.right);
			xEnd = EasyDraw_Clamp(x2floor, pBitmap->rtClip.left, pBitmap->rtClip.right);
			EasyDraw_FillHLine(pBitmap, y1floor, xBegin, xEnd, &EasyDraw_MakeBrush(pBrush->clr, lround((y2 - y1) * pBrush->nWeight)));
		}
	}
	else {
		return false;
	}

	return true;
}
bool EasyDraw_fFillRectangle_normal(
	pEasyDraw_Bitmap pBitmap,
	double x1, double y1,
	double x2, double y2,
	pEasyDraw_Brush pBrush
) {
	int32_t x1floor, x1ceil;
	int32_t y1floor, y1ceil;
	int32_t x2floor, x2ceil;
	int32_t y2floor, y2ceil;
	uint32_t xBegin, xEnd;
	uint32_t yBegin, yEnd;
	EasyDraw_Color pxTemp;
	uint32_t nOffset;
	uint8_t nWeight;

	if (EasyDraw_IsRectEmpty(&pBitmap->rtClip))
		return true;
	if (pBrush == NULL || pBrush->nWeight == 0)
		return true;

	//Special cases
	if (EasyDraw_fFillRectangle_normal_Special(pBitmap, x1, y1, x2, y2, pBrush))
		return true;

	x1floor = lffloor(x1); x1ceil = lfceil(x1);
	y1floor = lffloor(y1); y1ceil = lfceil(y1);
	x2floor = lffloor(x2); x2ceil = lfceil(x2);
	y2floor = lffloor(y2); y2ceil = lfceil(y2);

	xBegin = EasyDraw_Clamp(x1floor + 1, pBitmap->rtClip.left, pBitmap->rtClip.right);
	xEnd = EasyDraw_Clamp(x2floor, pBitmap->rtClip.left, pBitmap->rtClip.right);
	yBegin = EasyDraw_Clamp(y1floor + 1, pBitmap->rtClip.top, pBitmap->rtClip.bottom);
	yEnd = EasyDraw_Clamp(y2floor, pBitmap->rtClip.top, pBitmap->rtClip.bottom);

	//Draw 4 vertexes
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x1floor, y1floor)) {	//(x1, y1)
		nOffset = x1floor + pBitmap->nWidth * y1floor;
		nWeight = (uint8_t)lround((x1ceil - x1) * (y1ceil - y1) * pBrush->nWeight);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
	}
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x2floor, y1floor)) {	//(x2, y1)
		nOffset = x2floor + pBitmap->nWidth * y1floor;
		nWeight = (uint8_t)lround((x2 - x2floor) * (y1ceil - y1) * pBrush->nWeight);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
	}
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x1floor, y2floor)) {	//(x1, y2)
		nOffset = x1floor + pBitmap->nWidth * y2floor;
		nWeight = (uint8_t)lround((x1ceil - x1) * (y2 - y2floor) * pBrush->nWeight);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
	}
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x2floor, y2floor)) {	//(x2, y2)
		nOffset = x2floor + pBitmap->nWidth * y2floor;
		nWeight = (uint8_t)lround((x2 - x2floor) * (y2 - y2floor) * pBrush->nWeight);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb3(pBrush->clr, pxTemp, nWeight);
	}

	//Draw border lines
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, pBitmap->rtClip.left, y1floor))	//Top
		EasyDraw_FillHLine(pBitmap, y1floor, xBegin, xEnd, &EasyDraw_MakeBrush(pBrush->clr, lround((y1ceil - y1) * pBrush->nWeight)));
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, pBitmap->rtClip.left, y2floor))	//Bottom
		EasyDraw_FillHLine(pBitmap, y2floor, xBegin, xEnd, &EasyDraw_MakeBrush(pBrush->clr, lround((y2 - y2floor) * pBrush->nWeight)));
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x1floor, pBitmap->rtClip.top))	//Left
		EasyDraw_FillVLine(pBitmap, x1floor, yBegin, yEnd, &EasyDraw_MakeBrush(pBrush->clr, lround((x1ceil - x1) * pBrush->nWeight)));
	if (EasyDraw_IsPtInRect(&pBitmap->rtClip, x2floor, pBitmap->rtClip.top))	//Right
		EasyDraw_FillVLine(pBitmap, x2floor, yBegin, yEnd, &EasyDraw_MakeBrush(pBrush->clr, lround((x2 - x2floor) * pBrush->nWeight)));

	//Fill body
	EasyDraw_FillRect(pBitmap, xBegin, yBegin, xEnd, yEnd, pBrush);

	return true;
}
bool EasyDraw_fFillRectangle_sse2(
	pEasyDraw_Bitmap pBitmap,
	double x1, double y1,
	double x2, double y2,
	pEasyDraw_Brush pBrush
) {
	return EasyDraw_fFillRectangle_normal(pBitmap, x1, y1, x2, y2, pBrush);
}
bool EasyDraw_fFillRectangle(
	pEasyDraw_Bitmap pBitmap,
	double x1, double y1,
	double x2, double y2,
	pEasyDraw_Brush pBrush
) {
	if (pBitmap == NULL)
		return false;
	if (x1 > x2)
		swap(x1, x2);
	if (y1 > y2)
		swap(y1, y2);
#if USE_SIMD
	return EasyDraw_fFillRectangle_sse2(pBitmap, x1, y1, x2, y2, pBrush);
#else
	return EasyDraw_fFillRectangle_normal(pBitmap, x1, y1, x2, y2, pBrush);
#endif
}

void EasyDraw_DrawBasicTextW_RenderBitmap(
	EasyDraw_Bitmap *restrict pBitmap,
	int32_t xIn, int32_t yIn,
	uint32_t nCharWidth, uint32_t nCharHeight,
	uint8_t *pData
) {
	EasyDraw_Color pxTemp;
	int32_t xPen, yPen;
	for (uint32_t y = 0; y < nCharHeight; y++) {
		for (uint32_t x = 0; x < nCharWidth; x++) {
			xPen = x + xIn;
			yPen = y + yIn;
			if (EasyDraw_IsPtInRect(&pBitmap->rtClip, xPen, yPen)) {
				//pxTemp = EasyDraw_BlendArgb3(
				//	EasyDraw_MakeArgb(255, 255, 255, 255),
				//	pBitmap->pData[xPen + yPen * pBitmap->nWidth],
				//	nBkWhiteWeight
				//);
				pxTemp = pBitmap->pData[xPen + yPen * pBitmap->nWidth];
				pBitmap->pData[xPen + yPen * pBitmap->nWidth] = EasyDraw_BlendArgb3(
					pxTemp,
					EasyDraw_MakeArgb(255, 0, 0, 0),
					pData[x + y * nCharWidth]
				);
			}
		}
	}
}
bool EasyDraw_DrawBasicTextW(pEasyDraw_Bitmap pBitmap, int32_t x, int32_t y, const wchar_t *str, uint8_t nBkWhiteWeight) {
	struct {
		uint32_t nWidth, nHeight;
		uint32_t xPenOffset, yPenOffset;
		uint32_t xMoveOffset, yMoveOffset;
		uint8_t *pData;
	} CharacterInfo[256] = {	//Characters are copied from Fixedsys
		[L'\n'] = { 0, 0, 0, 0, 8, 15, NULL },
		[L' '] = { 0, 0, 0, 0, 8, 15, NULL },
		[L'('] = {
			4, 11, 3, 2, 8, 15, (uint8_t*)
			W W B B	\
			W B B W	\
			W B B W	\
			B B W W	\
			B B W W	\
			B B W W	\
			B B W W	\
			B B W W	\
			W B B W	\
			W B B W	\
			W W B B	\
		},
		[L')'] = {
			4, 11, 3, 2, 8, 15, (uint8_t*)
			B B W W	\
			W B B W	\
			W B B W	\
			W W B B	\
			W W B B	\
			W W B B	\
			W W B B	\
			W W B B	\
			W B B W	\
			W B B W	\
			B B W W	\
		},
		[L','] = {
			3, 4, 4, 9, 8, 15, (uint8_t*)
			B B B	\
			B B B	\
			W B B	\
			B B W	\
		},
		[L'.'] = {
			3, 2, 4, 9, 8, 15, (uint8_t*)
			B B B	\
			B B B	\
		},
		[L'0'] = {
			6, 9, 3, 2, 8, 15, (uint8_t*)
			W B B B B W	\
			B B W W B B	\
			B B W B B B	\
			B B W B B B	\
			B B W W B B	\
			B B B W B B	\
			B B B W B B	\
			B B W W B B	\
			W B B B B W	\
		},
		[L'1'] = {
			5, 9, 2, 2, 8, 15, (uint8_t*)
			W W W B B	\
			W W B B B	\
			B B B B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
		},
		[L'2'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			W B B B B W	\
			B B W W B B	\
			B B W W B B	\
			W W W W B B	\
			W W W B B W	\
			W W B B W W	\
			W B B W W W	\
			B B W W W W	\
			B B B B B B	\
		},
		[L'3'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			W B B B B W	\
			B B W W B B	\
			B B W W B B	\
			W W W W B B	\
			W W B B B W	\
			W W W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B W	\
		},
		[L'4'] = {
			7, 9, 2, 2, 8, 15, (uint8_t*)
			W B B W W W W	\
			W B B W W W W	\
			W B B W B B W	\
			W B B W B B W	\
			W B B W B B W	\
			B B W W B B W	\
			B B B B B B B	\
			W W W W B B W	\
			W W W W B B W	\
		},
		[L'5'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			B B B B B B	\
			B B W W W W	\
			B B W W W W	\
			B B W W W W	\
			B B B B B W	\
			W W W W B B	\
			W W W W B B	\
			W W W B B W	\
			B B B B W W	\
		},
		[L'6'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			W W B B B W	\
			W W B B W W	\
			W B B W W W	\
			B B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B W	\
		},
		[L'7'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			B B B B B B	\
			W W W W B B	\
			W W W B B W	\
			W W W B B W	\
			W W B B W W	\
			W W B B W W	\
			W B B W W W	\
			W B B W W W	\
			W B B W W W	\
		},
		[L'8'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			W B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B B W B B	\
			W B B B B W	\
			B B W B B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B W	\
		},
		[L'9'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			W B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B B	\
			W W W B B W	\
			W W B B W W	\
			W B B B W W	\
		},	
		[L'='] = {
			6, 3, 2, 5, 8, 15, (uint8_t*)
			B B B B B B	\
			W W W W W W	\
			B B B B B B	\
		},
		[L'H'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B B B B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
		},
		[L'a'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			W B B B B W	\
			W W W W B B	\
			W W W W B B	\
			W B B B B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B B	\
		},
		[L'b'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			B B W W W W	\
			B B W W W W	\
			B B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B B B B W	\
		},
		[L'c'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			W B B B B W	\
			B B W W B B	\
			B B W W W W	\
			B B W W W W	\
			B B W W W W	\
			B B W W B B	\
			W B B B B W	\
		},
		[L'd'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			W W W W B B	\
			W W W W B B	\
			W B B B B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B B	\
		},
		[L'e'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			W B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B B B B B	\
			B B W W W W	\
			B B W W W W	\
			W B B B B W	\
		},
		[L'f'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			W W B B B B	\
			W B B W W W	\
			W B B W W W	\
			W B B W W W	\
			B B B B B B	\
			W B B W W W	\
			W B B W W W	\
			W B B W W W	\
			W B B W W W	\
		},
		[L'g'] = {
			6, 10, 2, 4, 8, 15, (uint8_t*)
			W B B B B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B B	\
			W W W W B B	\
			W W W W B B	\
			B B B B B W	\
		},
		[L'h'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			B B W W W W	\
			B B W W W W	\
			B B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
		},
		[L'i'] = {
			6, 10, 2, 1, 8, 15, (uint8_t*)
			W W B B W W	\
			W W B B W W	\
			W W W W W W	\
			B B B B W W	\
			W W B B W W	\
			W W B B W W	\
			W W B B W W	\
			W W B B W W	\
			W W B B W W	\
			B B B B B B	\
		},
		[L'j'] = {
			5, 13, 2, 1, 8, 15, (uint8_t*)
			W W W B B	\
			W W W B B	\
			W W W W W	\
			W B B B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			W W W B B	\
			B B B B W	\
		},
		[L'k'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			B B W W W W	\
			B B W W W W	\
			B B W W B B	\
			B B W W B B	\
			B B W B B W	\
			B B B B W W	\
			B B W B B W	\
			B B W W B B	\
			B B W W B B	\
		},
		[L'l'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			B B B B W W	\
			W W B B W W	\
			W W B B W W	\
			W W B B W W	\
			W W B B W W	\
			W W B B W W	\
			W W B B W W	\
			W W B B W W	\
			B B B B B B	\
		},
		[L'm'] = {
			7, 7, 2, 4, 8, 15, (uint8_t*)
			B B B B B B W	\
			B B W B W B B	\
			B B W B W B B	\
			B B W B W B B	\
			B B W B W B B	\
			B B W B W B B	\
			B B W W W B B	\
		},
		[L'n'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			B B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
		},
		[L'o'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			W B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B W	\
		},
		[L'p'] = {
			6, 10, 2, 4, 8, 15, (uint8_t*)
			B B B B B W	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B B B B W	\
			B B W W W W	\
			B B W W W W	\
			B B W W W W	\
		},
		[L'q'] = {
			6, 10, 2, 4, 8, 15, (uint8_t*)
			W B B B B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B B	\
			W W W W B B	\
			W W W W B B	\
			W W W W B B	\
		},
		[L'r'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			B B W W B B	\
			B B W B B B	\
			B B B W W W	\
			B B W W W W	\
			B B W W W W	\
			B B W W W W	\
			B B W W W W	\
		},
		[L's'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			W B B B B B	\
			B B W W W W	\
			B B W W W W	\
			W B B B B W	\
			W W W W B B	\
			W W W W B B	\
			B B B B B W	\
		},
		[L't'] = {
			6, 9, 2, 2, 8, 15, (uint8_t*)
			W B B W W W	\
			W B B W W W	\
			B B B B B B	\
			W B B W W W	\
			W B B W W W	\
			W B B W W W	\
			W B B W W W	\
			W B B W W W	\
			W W B B B B	\
		},
		[L'u'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B B	\
		},
		[L'v'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			B B W W B B	\
			W B B B B W	\
			W W B B W W	\
		},
		[L'w'] = {
			7, 7, 2, 4, 8, 15, (uint8_t*)
			B B W W W B B	\
			B B W B W B B	\
			B B W B W B B	\
			B B W B W B B	\
			B B W B W B B	\
			W B B W B B W	\
			W B B W B B W	\
		},
		[L'x'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			B B W W B B	\
			B B W W B B	\
			W B B B B W	\
			W W B B W W	\
			W B B B B W	\
			B B W W B B	\
			B B W W B B	\
		},
		[L'y'] = {
			7, 10, 1, 4, 8, 15, (uint8_t*)
			W B B W W B B	\
			W B B W W B B	\
			W B B W W B B	\
			W B B W W B B	\
			W B B W W B B	\
			W B B W W B B	\
			W W B B B B W	\
			W W W W B B W	\
			W W W B B W W	\
			B B B B W W W	\
		},
		[L'z'] = {
			6, 7, 2, 4, 8, 15, (uint8_t*)
			B B B B B B	\
			W W W W B B	\
			W W W B B W	\
			W W B B W W	\
			W B B W W W	\
			B B W W W W	\
			B B B B B B	\
		}
	};
	int32_t xReachMax, yReachMax;
	const wchar_t *strTemp;
	int32_t xPen, yPen;
	if (pBitmap == NULL || str == NULL)
		return false;

	xPen = x;
	yPen = y;
	xReachMax = x;
	for (strTemp = str; *strTemp != L'\0'; strTemp++) {
		if (*strTemp == L'\n') {
			xReachMax = max(xReachMax, xPen);
			xPen = x;
			yPen += CharacterInfo[*strTemp].yMoveOffset;
			continue;
		}

		xPen += CharacterInfo[*strTemp].xMoveOffset;
	}
	xReachMax = max(xReachMax, xPen);
	yReachMax = strTemp == str ? 0 : CharacterInfo[strTemp[-1]].yMoveOffset;

	EasyDraw_FillRect(
		pBitmap,
		max(x, 0), max(y, 0),
		min(x + xReachMax, (int32_t)(pBitmap->nWidth - 1)), min(y + yReachMax, (int32_t)(pBitmap->nHeight - 1)),
		&EasyDraw_MakeBrush(EasyDraw_MakeArgb(255, 255, 255, 255), nBkWhiteWeight)
	);

	xPen = x;
	yPen = y;
	for (strTemp = str; *strTemp != L'\0'; strTemp++) {
		if (*strTemp == L'\n') {
			xPen = x;
			yPen += CharacterInfo[*strTemp].yMoveOffset;
			continue;
		}

		EasyDraw_DrawBasicTextW_RenderBitmap(
			pBitmap,
			xPen + CharacterInfo[*strTemp].xPenOffset, yPen + CharacterInfo[*strTemp].yPenOffset,
			CharacterInfo[*strTemp].nWidth, CharacterInfo[*strTemp].nHeight,
			CharacterInfo[*strTemp].pData
		);

		xPen += CharacterInfo[*strTemp].xMoveOffset;
	}

	return true;
}