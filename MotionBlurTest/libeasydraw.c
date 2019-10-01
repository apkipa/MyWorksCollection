#include "libeasydraw.h"

#include <emmintrin.h>
#include <float.h>
#include <stdio.h>
#include <math.h>

//C(Closed): <=
//O(Open): <
#define IsInRangeOO(n, min, max) ((min) < (n) && (n) < (max))
#define IsInRangeCC(n, min, max) ((min) <= (n) && (n) <= (max))
#define IsInRangeCO(n, min, max) ((min) <= (n) && (n) < (max))
#define IsInRangeOC(n, min, max) ((min) < (n) && (n) <= (max))
#define AlignUpward(n, align) (((uintptr_t)(n) + (align) - 1) & ~(uintptr_t)(align))
#define AlignDownward(n, align) ((uintptr_t)(n) & ~(uintptr_t)(align))

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
	EasyDraw_Color *pData;
} EasyDraw_Bitmap;

typedef struct tagEasyDraw_MotionBlurData {
	//Requires 256 times of addition
	uint32_t nWidth, nHeight;
	void *pData;	//64-bit pixels
} EasyDraw_MotionBlurData, *pEasyDraw_MotionBlurData;

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
			_mm_store_si128(pmDataTemp++, mData);

		pDataRemainTemp = (uint16_t*)pmDataTemp;
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
			_mm_store_si128(pmDataTemp++, mData);

		pDataRemainTemp = (uint32_t*)pmDataTemp;
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

	return pBitmap;
}

bool EasyDraw_DestroyBitmap(pEasyDraw_Bitmap pBitmap) {
	if (pBitmap == NULL)
		return false;

	_mm_free(pBitmap->pData);
	free(pBitmap);

	return true;
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

bool EasyDraw_FillRectangle_normal(pEasyDraw_Bitmap pBitmap, double x1, double y1, double x2, double y2, uint32_t clr) {
	EasyDraw_Color pxTemp;
	bool bXSame, bYSame;
	uint8_t nWeight;
	int32_t nOffset;
	double fTemp;

	if (isfltint(x1))
		x1 += DBL_EPSILON;
	if (isfltint(y1))
		y1 += DBL_EPSILON;

	//Special cases
	bXSame = lfloor(x1) == lfloor(x2);
	bYSame = lfloor(y1) == lfloor(y2);
	if (bXSame && bYSame) {
		//Draw one vertex
		nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y1);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)((x2 - x1) * (y2 - y1) * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		return true;
	}
	else if (bXSame) {
		//Draw two vertexes
		fTemp = x2 - x1;

		nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y1);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)(fTemp * (ceil(y1) - y1) * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y2);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)(fTemp * (y2 - floor(y2)) * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		//Then fill body(border)
		nWeight = (uint8_t)(fTemp * 255);
		for (int32_t i = lfloor(y1) + 1; i < lfloor(y2); i++) {
			nOffset = lfloor(x1) + pBitmap->nWidth * i;
			if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
				continue;
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		return true;
	}
	else if (bYSame) {
		//Draw two vertexes
		fTemp = y2 - y1;

		nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y1);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)((ceil(x1) - x1) * fTemp * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		nOffset = lfloor(x2) + pBitmap->nWidth * lfloor(y1);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)((x2 - floor(x2)) * fTemp * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		//Then fill body(border)
		nWeight = (uint8_t)(fTemp * 255);
		for (int32_t i = lfloor(x1) + 1; i < lfloor(x2); i++) {
			nOffset = i + pBitmap->nWidth * lfloor(y1);
			if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
				continue;
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		return true;
	}

	//First draw four vertexes
	//vertex (x1, y1)
	nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y1);
	if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
		nWeight = (uint8_t)((ceil(x1) - x1) * (ceil(y1) - y1) * 255);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//vertex (x2, y1)
	nOffset = lfloor(x2) + pBitmap->nWidth * lfloor(y1);
	if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
		nWeight = (uint8_t)((x2 - floor(x2)) * (ceil(y1) - y1) * 255);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//vertex (x1, y2)
	nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y2);
	if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
		nWeight = (uint8_t)((ceil(x1) - x1) * (y2 - floor(y2)) * 255);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//vertex (x2, y2)
	nOffset = lfloor(x2) + pBitmap->nWidth * lfloor(y2);
	if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
		nWeight = (uint8_t)((x2 - floor(x2)) * (y2 - floor(y2)) * 255);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	
	//Then draw four borders
	//Top
	fTemp = ceil(y1) - y1;
	nWeight = (uint8_t)(fTemp * 255);
	for (int32_t i = lfloor(x1) + 1; i < lfloor(x2); i++) {
		nOffset = i + pBitmap->nWidth * lfloor(y1);
		if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
			continue;
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//Bottom
	fTemp = y2 - floor(y2);
	nWeight = (uint8_t)(fTemp * 255);
	for (int32_t i = lfloor(x1) + 1; i < lfloor(x2); i++) {
		nOffset = i + pBitmap->nWidth * lfloor(y2);
		if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
			continue;
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//Left
	fTemp = ceil(x1) - x1;
	nWeight = (uint8_t)(fTemp * 255);
	for (int32_t i = lfloor(y1) + 1; i < lfloor(y2); i++) {
		nOffset = lfloor(x1) + pBitmap->nWidth * i;
		if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
			continue;
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//Right
	fTemp = x2 - floor(x2);
	nWeight = (uint8_t)(fTemp * 255);
	for (int32_t i = lfloor(y1) + 1; i < lfloor(y2); i++) {
		nOffset = lfloor(x2) + pBitmap->nWidth * i;
		if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
			continue;
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}

	//Finally fill the body
	for (int32_t y = lfloor(y1) + 1; y < lfloor(y2); y++) {
		for (int32_t x = lfloor(x1) + 1; x < lfloor(x2); x++) {
			nOffset = x + pBitmap->nWidth * y;
			if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
				continue;
			pBitmap->pData[nOffset] = clr;
		}
	}

	return true;
}
bool EasyDraw_FillRectangle_sse2(pEasyDraw_Bitmap pBitmap, double x1, double y1, double x2, double y2, uint32_t clr) {
	int32_t x1Dest, y1Dest, x2Dest, y2Dest;
	uintptr_t nBeginPos, nEndPos;
	EasyDraw_Color pxTemp;
	bool bXSame, bYSame;
	uint8_t nWeight;
	int32_t nOffset;
	double fTemp;

	if (isfltint(x1))
		x1 += DBL_EPSILON;
	if (isfltint(y1))
		y1 += DBL_EPSILON;

	//Special cases
	bXSame = lfloor(x1) == lfloor(x2);
	bYSame = lfloor(y1) == lfloor(y2);
	if (bXSame && bYSame) {
		//Draw one vertex
		nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y1);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)((x2 - x1) * (y2 - y1) * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		return true;
	}
	else if (bXSame) {
		//Draw two vertexes
		fTemp = x2 - x1;

		nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y1);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)(fTemp * (ceil(y1) - y1) * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y2);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)(fTemp * (y2 - floor(y2)) * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		//Then fill body(border)
		nWeight = (uint8_t)(fTemp * 255);
		for (int32_t i = lfloor(y1) + 1; i < lfloor(y2); i++) {
			nOffset = lfloor(x1) + pBitmap->nWidth * i;
			if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
				continue;
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		return true;
	}
	else if (bYSame) {
		//Draw two vertexes
		fTemp = y2 - y1;

		nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y1);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)((ceil(x1) - x1) * fTemp * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		nOffset = lfloor(x2) + pBitmap->nWidth * lfloor(y1);
		if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
			nWeight = (uint8_t)((x2 - floor(x2)) * fTemp * 255);
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		//Then fill body(border)
		nWeight = (uint8_t)(fTemp * 255);
		for (int32_t i = lfloor(x1) + 1; i < lfloor(x2); i++) {
			nOffset = i + pBitmap->nWidth * lfloor(y1);
			if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
				continue;
			pxTemp = pBitmap->pData[nOffset];
			pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
		}

		return true;
	}

	//First draw four vertexes
	//vertex (x1, y1)
	nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y1);
	if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
		nWeight = (uint8_t)((ceil(x1) - x1) * (ceil(y1) - y1) * 255);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//vertex (x2, y1)
	nOffset = lfloor(x2) + pBitmap->nWidth * lfloor(y1);
	if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
		nWeight = (uint8_t)((x2 - floor(x2)) * (ceil(y1) - y1) * 255);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//vertex (x1, y2)
	nOffset = lfloor(x1) + pBitmap->nWidth * lfloor(y2);
	if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
		nWeight = (uint8_t)((ceil(x1) - x1) * (y2 - floor(y2)) * 255);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//vertex (x2, y2)
	nOffset = lfloor(x2) + pBitmap->nWidth * lfloor(y2);
	if (IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight))) {
		nWeight = (uint8_t)((x2 - floor(x2)) * (y2 - floor(y2)) * 255);
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}

	//Then draw four borders
	//Top
	fTemp = ceil(y1) - y1;
	nWeight = (uint8_t)(fTemp * 255);
	for (int32_t i = lfloor(x1) + 1; i < lfloor(x2); i++) {
		nOffset = i + pBitmap->nWidth * lfloor(y1);
		if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
			continue;
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//Bottom
	fTemp = y2 - floor(y2);
	nWeight = (uint8_t)(fTemp * 255);
	for (int32_t i = lfloor(x1) + 1; i < lfloor(x2); i++) {
		nOffset = i + pBitmap->nWidth * lfloor(y2);
		if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
			continue;
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//Left
	fTemp = ceil(x1) - x1;
	nWeight = (uint8_t)(fTemp * 255);
	for (int32_t i = lfloor(y1) + 1; i < lfloor(y2); i++) {
		nOffset = lfloor(x1) + pBitmap->nWidth * i;
		if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
			continue;
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}
	//Right
	fTemp = x2 - floor(x2);
	nWeight = (uint8_t)(fTemp * 255);
	for (int32_t i = lfloor(y1) + 1; i < lfloor(y2); i++) {
		nOffset = lfloor(x2) + pBitmap->nWidth * i;
		if (!IsInRangeCO(nOffset, 0, (int32_t)(pBitmap->nWidth * pBitmap->nHeight)))
			continue;
		pxTemp = pBitmap->pData[nOffset];
		pBitmap->pData[nOffset] = EasyDraw_BlendArgb(clr, pxTemp, nWeight);
	}

	//Finally fill the body
	x1Dest = max(lfloor(x1) + 1, 0);
	x2Dest = min(lfloor(x2), (int32_t)(pBitmap->nWidth - 1));
	y1Dest = max(lfloor(y1) + 1, 0);
	y2Dest = min(lfloor(y2), (int32_t)(pBitmap->nHeight - 1));
	if (x1Dest >= x2Dest || y1Dest >= y2Dest)
		return true;

	if (x1Dest == 0 && x2Dest == pBitmap->nWidth - 1) {
		//Pixels are connected, fill directly
		nOffset = x1Dest + pBitmap->nWidth * y1Dest;
		memset_32_SSE(&pBitmap->pData[nOffset], clr, (x2Dest - x1Dest + 1) * (y2Dest - y1Dest + 1));
	}
	else {
		//Pixels are not connected, fill every line
		nBeginPos = x1Dest + pBitmap->nWidth * y1Dest;
		memset_32_SSE(&pBitmap->pData[nBeginPos], clr, x2Dest - x1Dest + 1);
		for (int32_t y = y1Dest + 1; y <= y2Dest; y++) {
			nOffset = x1Dest + pBitmap->nWidth * y;
			memcpy(&pBitmap->pData[nOffset], &pBitmap->pData[nBeginPos], (x2Dest - x1Dest + 1) * 4);
		}
	}

	return true;
}
bool EasyDraw_FillRectangle(pEasyDraw_Bitmap pBitmap, double x1, double y1, double x2, double y2, uint32_t clr) {
	if (pBitmap == NULL)
		return false;
	if (x1 > x2)
		swap(x1, x2);
	if (y1 > y2)
		swap(y1, y2);
#if USE_SIMD
	return EasyDraw_FillRectangle_sse2(pBitmap, x1, y1, x2, y2, clr);
#else
	return EasyDraw_FillRectangle_normal(pBitmap, x1, y1, x2, y2, clr);
#endif
}

intptr_t EasyDraw_BeginMotionBlur(pEasyDraw_Bitmap pBitmap) {
	pEasyDraw_MotionBlurData pBlurData;
	void *pBlurPixelData;
	size_t nSize;

	if (pBitmap == NULL)
		return 0;

	nSize = pBitmap->nWidth * pBitmap->nHeight * 8;

	pBlurData = (pEasyDraw_MotionBlurData)malloc(sizeof(EasyDraw_MotionBlurData));
	pBlurPixelData = _mm_malloc(nSize, 32);
	if (pBlurData == NULL || pBlurPixelData == NULL) {
		free(pBlurData);
		_mm_free(pBlurPixelData);

		return 0;
	}

	pBlurData->nWidth = pBitmap->nWidth;
	pBlurData->nHeight = pBitmap->nHeight;
	pBlurData->pData = pBlurPixelData;
	//memset(pBlurPixelData, 0, nSize);	//Zero data
	memset_16_SSE(pBlurPixelData, 127, nSize / 2);	//Set 127 for rounding

	return (intptr_t)pBlurData;
}

bool EasyDraw_MotionBlur_AddBitmap_normal(intptr_t nData, pEasyDraw_Bitmap pBitmap) {
	pEasyDraw_MotionBlurData pBlurData;
	size_t nOffset;

	pBlurData = (pEasyDraw_MotionBlurData)nData;

	for (size_t y = 0; y < pBlurData->nHeight; y++) {
		for (size_t x = 0; x < pBlurData->nWidth; x++) {
			nOffset = (x + pBlurData->nWidth * y) * 4;
			((uint16_t*)pBlurData->pData)[nOffset + 0] += ((uint8_t*)pBitmap->pData)[nOffset + 0];
			((uint16_t*)pBlurData->pData)[nOffset + 1] += ((uint8_t*)pBitmap->pData)[nOffset + 1];
			((uint16_t*)pBlurData->pData)[nOffset + 2] += ((uint8_t*)pBitmap->pData)[nOffset + 2];
			((uint16_t*)pBlurData->pData)[nOffset + 3] += ((uint8_t*)pBitmap->pData)[nOffset + 3];
		}
	}

	return true;
}
bool EasyDraw_MotionBlur_AddBitmap_sse2(intptr_t nData, pEasyDraw_Bitmap pBitmap) {
	pEasyDraw_MotionBlurData pBlurData;
	size_t nPixelCount, nLoopCount;
	uint16_t *pDest;
	size_t nOffset;
	uint8_t *pSrc;

	pBlurData = (pEasyDraw_MotionBlurData)nData;
	pDest = (uint16_t*)pBlurData->pData;
	pSrc = (uint8_t*)pBitmap->pData;
	nPixelCount = pBlurData->nWidth * pBlurData->nHeight;
	nLoopCount = nPixelCount / 4;

	for (size_t i = 0; i < nLoopCount; i++) {
		__m128i xmm0, xmm1, xmm2, xmm3;
		xmm0 = _mm_setzero_si128();
		xmm1 = _mm_load_si128((__m128i*)pSrc + i);
		xmm2 = _mm_unpacklo_epi8(xmm1, xmm0);
		xmm3 = _mm_unpackhi_epi8(xmm1, xmm0);
		xmm0 = _mm_load_si128((__m128i*)pDest + i * 2 + 0);
		xmm1 = _mm_load_si128((__m128i*)pDest + i * 2 + 1);
		xmm0 = _mm_add_epi16(xmm0, xmm2);
		xmm1 = _mm_add_epi16(xmm1, xmm3);
		_mm_store_si128((__m128i*)pDest + i * 2 + 0, xmm0);
		_mm_store_si128((__m128i*)pDest + i * 2 + 1, xmm1);
	}

	for (size_t i = nLoopCount * 4; i < nPixelCount; i++) {
		nOffset = i * 4;
		((uint16_t*)pBlurData->pData)[nOffset + 0] += ((uint8_t*)pBitmap->pData)[nOffset + 0];
		((uint16_t*)pBlurData->pData)[nOffset + 1] += ((uint8_t*)pBitmap->pData)[nOffset + 1];
		((uint16_t*)pBlurData->pData)[nOffset + 2] += ((uint8_t*)pBitmap->pData)[nOffset + 2];
		((uint16_t*)pBlurData->pData)[nOffset + 3] += ((uint8_t*)pBitmap->pData)[nOffset + 3];
	}

	return true;
}
bool EasyDraw_MotionBlur_AddBitmap(intptr_t nData, pEasyDraw_Bitmap pBitmap) {
	if (nData == 0 || pBitmap == NULL)
		return false;
#if USE_SIMD
	return EasyDraw_MotionBlur_AddBitmap_sse2(nData, pBitmap);
#else
	return EasyDraw_MotionBlur_AddBitmap_normal(nData, pBitmap);
#endif
}

bool EasyDraw_FinishMotionBlur_normal(intptr_t nData, pEasyDraw_Bitmap pBitmapOutput) {
	pEasyDraw_MotionBlurData pBlurData;
	size_t nOffset;

	pBlurData = (pEasyDraw_MotionBlurData)nData;

	for (size_t y = 0; y < pBlurData->nHeight; y++) {
		for (size_t x = 0; x < pBlurData->nWidth; x++) {
			nOffset = (x + pBlurData->nWidth * y) * 4;
			((uint8_t*)pBitmapOutput->pData)[nOffset + 0] = (uint8_t)(((uint16_t*)pBlurData->pData)[nOffset + 0] / 256);
			((uint8_t*)pBitmapOutput->pData)[nOffset + 1] = (uint8_t)(((uint16_t*)pBlurData->pData)[nOffset + 1] / 256);
			((uint8_t*)pBitmapOutput->pData)[nOffset + 2] = (uint8_t)(((uint16_t*)pBlurData->pData)[nOffset + 2] / 256);
			((uint8_t*)pBitmapOutput->pData)[nOffset + 3] = (uint8_t)(((uint16_t*)pBlurData->pData)[nOffset + 3] / 256);
		}
	}

	_mm_free(pBlurData->pData);
	free(pBlurData);

	return true;
}
bool EasyDraw_FinishMotionBlur_sse2(intptr_t nData, pEasyDraw_Bitmap pBitmapOutput) {
	pEasyDraw_MotionBlurData pBlurData;
	size_t nPixelCount, nLoopCount;
	size_t nOffset;
	uint16_t *pSrc;
	uint8_t *pDest;

	pBlurData = (pEasyDraw_MotionBlurData)nData;
	pSrc = (uint16_t*)pBlurData->pData;
	pDest = (uint8_t*)pBitmapOutput->pData;
	nPixelCount = pBlurData->nWidth * pBlurData->nHeight;
	nLoopCount = nPixelCount / 4;

	for (size_t i = 0; i < nLoopCount; i++) {
		__m128i xmm0, xmm1, xmm2;
		xmm0 = _mm_load_si128((__m128i*)pSrc + i * 2 + 0);
		xmm1 = _mm_load_si128((__m128i*)pSrc + i * 2 + 1);
		xmm0 = _mm_srli_epi16(xmm0, 8);
		xmm1 = _mm_srli_epi16(xmm1, 8);
		xmm2 = _mm_packus_epi16(xmm0, xmm1);
		_mm_store_si128((__m128i*)pDest + i, xmm2);
	}

	for (size_t i = nLoopCount * 4; i < nPixelCount; i++) {
		nOffset = i * 4;
		((uint8_t*)pBitmapOutput->pData)[nOffset + 0] = (uint8_t)(((uint16_t*)pBlurData->pData)[nOffset + 0] / 256);
		((uint8_t*)pBitmapOutput->pData)[nOffset + 1] = (uint8_t)(((uint16_t*)pBlurData->pData)[nOffset + 1] / 256);
		((uint8_t*)pBitmapOutput->pData)[nOffset + 2] = (uint8_t)(((uint16_t*)pBlurData->pData)[nOffset + 2] / 256);
		((uint8_t*)pBitmapOutput->pData)[nOffset + 3] = (uint8_t)(((uint16_t*)pBlurData->pData)[nOffset + 3] / 256);
	}

	return true;
}
bool EasyDraw_FinishMotionBlur(intptr_t nData, pEasyDraw_Bitmap pBitmapOutput) {
	pEasyDraw_MotionBlurData pBlurData;
	pBlurData = (pEasyDraw_MotionBlurData)nData;
	if (nData == 0 || pBitmapOutput == NULL)
		return false;
	if (pBlurData->nWidth != pBitmapOutput->nWidth || pBlurData->nHeight != pBitmapOutput->nHeight)
		return false;
#if USE_SIMD
	return EasyDraw_FinishMotionBlur_sse2(nData, pBitmapOutput);
#else
	return EasyDraw_FinishMotionBlur_normal(nData, pBitmapOutput);
#endif
}