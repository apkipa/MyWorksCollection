#include <inttypes.h>
#include <Windows.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#pragma comment(lib, "gdi32")
#pragma comment(lib, "user32")

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif	//!defined max
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif	//!defined min
#ifndef clamp
#define clamp(n, nMin, nMax) min(max(n, nMin), nMax)
#endif	//!defined clamp

typedef uint32_t Color32;

#define GetLowByte(data) ((uint8_t)((uint32_t)(data) & 0xff))
#define MakeArgb(a, r, g, b)	\
	((Color32)((((uint8_t)(b) | ((uint16_t)((uint8_t)(g)) << 8)) | (((uint32_t)(uint8_t)(r)) << 16)) | (((uint32_t)(uint8_t)(a)) << 24)))
#define GetArgbA(clrArgb) (GetLowByte((clrArgb) >> 24))
#define GetArgbR(clrArgb) (GetLowByte((clrArgb) >> 16))
#define GetArgbG(clrArgb) (GetLowByte(((uint16_t)(clrArgb)) >> 8))
#define GetArgbB(clrArgb) (GetLowByte(clrArgb))
#define BlendArgb(clr1, clr2, wClr1) MakeArgb(												\
		((GetArgbA(clr1) * (wClr1) + GetArgbA(clr2) * (255 - (wClr1)) + 127) * 0x8081) >> 23,	\
		((GetArgbR(clr1) * (wClr1) + GetArgbR(clr2) * (255 - (wClr1)) + 127) * 0x8081) >> 23,	\
		((GetArgbG(clr1) * (wClr1) + GetArgbG(clr2) * (255 - (wClr1)) + 127) * 0x8081) >> 23,	\
		((GetArgbB(clr1) * (wClr1) + GetArgbB(clr2) * (255 - (wClr1)) + 127) * 0x8081) >> 23	\
	)

long lfloor(double n) {
	return lround(floor(n));
}
long lffloor(double n) {
	return lround(ceil(n) - 1);
}
double ffloor(double n) {
	return ceil(n) - 1;
}
long lceil(double n) {
	return lround(ceil(n));
}
long lfceil(double n) {
	return lround(floor(n) + 1);
}
double fceil(double n) {
	return floor(n) + 1;
}

#define CLR_I FOREGROUND_INTENSITY
#define CLR_R FOREGROUND_RED
#define CLR_G FOREGROUND_GREEN
#define CLR_B FOREGROUND_BLUE

WORD nAttriQueue[1024];
int nAttriPtrFront = 0;
void PushColor(WORD nNewClr) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	nAttriQueue[nAttriPtrFront++] = csbi.wAttributes;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nNewClr);
}
void PopColor(void) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nAttriQueue[--nAttriPtrFront]);
}

void PrintInfo(const char *fmt, ...) {
	char buf[1024];
	va_list vl;

	va_start(vl, fmt);
	vsprintf(buf, fmt, vl);
	va_end(vl);

	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	PushColor(CLR_I | CLR_G | CLR_B);
	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	putchar('[');
	PopColor();
	printf("%s", "Info");
	PopColor();
	printf("] %s\n", buf);
	PopColor();
}

void PrintWarning(const char *fmt, ...) {
	char buf[1024];
	va_list vl;

	va_start(vl, fmt);
	vsprintf(buf, fmt, vl);
	va_end(vl);

	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	PushColor(CLR_I | CLR_R | CLR_G);
	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	putchar('[');
	PopColor();
	printf("%s", "Warning");
	PopColor();
	printf("] %s\n", buf);
	PopColor();
}

void PrintError(const char *fmt, ...) {
	char buf[1024];
	va_list vl;

	va_start(vl, fmt);
	vsprintf(buf, fmt, vl);
	va_end(vl);

	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	PushColor(CLR_I | CLR_R);
	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	putchar('[');
	PopColor();
	printf("%s", "Error");
	PopColor();
	printf("] %s\n", buf);
	PopColor();
}

typedef struct tagBitmap {
	BITMAPFILEHEADER bmpFile;
	BITMAPINFOHEADER bmpInfo;
	//Color32 clrPalette[256];
	Color32 *pData;
} Bitmap, *pBitmap;

Color32 Compatible_GetColor32FromColor24Src(const void *pSrc, uint32_t nWidth, uint32_t x, uint32_t y) {
	const uint8_t *pClrBegin;
	pClrBegin = &((const uint8_t*)pSrc)[x * 3 + y * ((nWidth * 3 + 3) & ~3)];
	return MakeArgb(0, pClrBegin[0], pClrBegin[1], pClrBegin[2]);
}

void Compatible_SetColor32ToColor24Dest(void *pDes, uint32_t nWidth, uint32_t x, uint32_t y, Color32 clr) {
	uint8_t *pClrBegin;
	pClrBegin = &((uint8_t*)pDes)[x * 3 + y * ((nWidth * 3 + 3) & ~3)];
	pClrBegin[0] = GetArgbR(clr);
	pClrBegin[1] = GetArgbG(clr);
	pClrBegin[2] = GetArgbB(clr);
}

pBitmap CreateEmptyBitmap(uint32_t nWidth, uint32_t nHeight) {
	Color32 *pClrData;
	pBitmap pBmp;

	pBmp = (pBitmap)malloc(sizeof(Bitmap));
	pClrData = (Color32*)malloc(sizeof(Color32) * nWidth * nHeight);
	if (pBmp == NULL || pClrData == NULL) {
		free(pBmp);
		free(pClrData);
		return NULL;
	}

	memset(pClrData, 0, sizeof(Color32) * nWidth * nHeight);

	((uint8_t*)&pBmp->bmpFile.bfType)[0] = 'B';
	((uint8_t*)&pBmp->bmpFile.bfType)[1] = 'M';
	pBmp->bmpFile.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(Color32) * nWidth * nHeight;
	pBmp->bmpFile.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	pBmp->bmpInfo.biClrImportant = 0;
	pBmp->bmpInfo.biClrUsed = 0;
	pBmp->bmpInfo.biCompression = 0;
	pBmp->bmpInfo.biWidth = nWidth;
	pBmp->bmpInfo.biHeight = nHeight;
	pBmp->bmpInfo.biPlanes = 1;
	pBmp->bmpInfo.biBitCount = 32;
	pBmp->bmpInfo.biSize = sizeof(BITMAPINFOHEADER);
	pBmp->bmpInfo.biSizeImage = sizeof(Color32) * nWidth * nHeight;
	pBmp->bmpInfo.biXPelsPerMeter = 0;
	pBmp->bmpInfo.biYPelsPerMeter = 0;

	pBmp->pData = pClrData;

	return pBmp;
}

void DestroyBitmap(pBitmap pBmp) {
	if (pBmp == NULL)
		return;
	free(pBmp->pData);
	free(pBmp);
}

uint32_t GetBitmapWidth(pBitmap pBmp) {
	return pBmp == NULL ? -1 : pBmp->bmpInfo.biWidth;
}

uint32_t GetBitmapHeight(pBitmap pBmp) {
	return pBmp == NULL ? -1 : pBmp->bmpInfo.biHeight;
}

Color32 GetBitmapPixel(pBitmap pBmp, uint32_t x, uint32_t y) {
	if (pBmp == NULL)
		return 0;
#if 0	//Return black
	if (x < 0 || x >= pBmp->bmpInfo.biWidth)
		return 0;
	if (y < 0 || y >= pBmp->bmpInfo.biHeight)
		return 0;
#else	//Return boundary color
	x = clamp(x, 0, pBmp->bmpInfo.biWidth - 1);
	y = clamp(y, 0, pBmp->bmpInfo.biHeight - 1);
#endif
	return pBmp->pData[x + y * pBmp->bmpInfo.biWidth];
}

bool SetBitmapPixel(pBitmap pBmp, uint32_t x, uint32_t y, Color32 clr) {
	if (pBmp == NULL)
		return false;
	if (x < 0 || x >= pBmp->bmpInfo.biWidth)
		return false;
	if (y < 0 || y >= pBmp->bmpInfo.biHeight)
		return false;
	pBmp->pData[x + y * pBmp->bmpInfo.biWidth] = clr;
	return true;
}

pBitmap CreateBitmapFromFile(const char *path) {
	BITMAPINFOHEADER bmpInfo;
	pBitmap pBmp, pBmpTmp;
	void *pDataTemp;
	FILE *fp;

	fp = fopen(path, "rb");
	if (fp == NULL)
		return NULL;

	fseek(fp, sizeof(BITMAPFILEHEADER), SEEK_SET);
	fread(&bmpInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	rewind(fp);

	pBmp = CreateEmptyBitmap(bmpInfo.biWidth, bmpInfo.biHeight);
	if (pBmp == NULL) {
		fclose(fp);
		return NULL;
	}

	if (bmpInfo.biSizeImage == 0) {
		PrintWarning("bmpInfo.biSizeImage is zero!");

		switch (bmpInfo.biBitCount) {
		case 32:
			bmpInfo.biSizeImage = sizeof(Color32) * bmpInfo.biWidth * bmpInfo.biHeight;
			break;
		case 24:
			bmpInfo.biSizeImage = ((bmpInfo.biWidth * 3 + 3) & ~3) * bmpInfo.biHeight;
			break;
		}
	}

	if (bmpInfo.biBitCount == 32) {
		fread(&pBmp->bmpFile, sizeof(BITMAPFILEHEADER), 1, fp);
		fread(&pBmp->bmpInfo, sizeof(BITMAPINFOHEADER), 1, fp);
		//fseek(fp, 11, SEEK_CUR);	// ???
		fread(pBmp->pData, 1, bmpInfo.biSizeImage, fp);

		for (int y = 0; y < bmpInfo.biHeight; y++) {
			for (int x = 0; x < bmpInfo.biWidth; x++) {
				uint8_t *pBuf = (uint8_t*)&pBmp->pData[x + y * bmpInfo.biWidth];

				pBmp->pData[x + y * bmpInfo.biWidth] = MakeArgb(pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
			}
		}
	}
	else if (bmpInfo.biBitCount == 24) {
		pDataTemp = malloc(bmpInfo.biSizeImage);
		PrintInfo("Bitmap size: %d", bmpInfo.biSizeImage);
		if (pDataTemp == NULL) {
			DestroyBitmap(pBmp);
			fclose(fp);
			return NULL;
		}

		fread(&pBmp->bmpFile, sizeof(BITMAPFILEHEADER), 1, fp);
		fread(&pBmp->bmpInfo, sizeof(BITMAPINFOHEADER), 1, fp);
		fread(pDataTemp, 1, bmpInfo.biSizeImage, fp);

		for (uint32_t y = 0; y < bmpInfo.biHeight; y++) {
			for (uint32_t x = 0; x < bmpInfo.biWidth; x++)
				pBmp->pData[x + y * bmpInfo.biWidth] = Compatible_GetColor32FromColor24Src(pDataTemp, bmpInfo.biWidth, x, y);
		}

		free(pDataTemp);
	}
	else {
		//Unsupported pixel width
		DestroyBitmap(pBmp);
		fclose(fp);
		return NULL;
	}

	fclose(fp);

	return pBmp;
}

bool SaveBitmapToFile(pBitmap pBmp, const char *path) {	//Save 32-bit bitmaps as 24-bit bitmaps
	BITMAPFILEHEADER bmpFileTmp;
	BITMAPINFOHEADER bmpInfoTmp;
	void *pDataTemp;
	FILE *fp;

	fp = fopen(path, "wb");
	if (fp == NULL)
		return false;

	bmpFileTmp = pBmp->bmpFile;
	bmpInfoTmp = pBmp->bmpInfo;
	bmpFileTmp.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpInfoTmp.biHeight * ((bmpInfoTmp.biWidth * 3 + 3) & ~3);
	bmpInfoTmp.biBitCount = 24;
	bmpInfoTmp.biSizeImage = bmpInfoTmp.biHeight * ((bmpInfoTmp.biWidth * 3 + 3) & ~3);

	pDataTemp = malloc(bmpInfoTmp.biSizeImage);
	if (pDataTemp == NULL) {
		fclose(fp);
		return false;
	}

	for (uint32_t y = 0; y < bmpInfoTmp.biHeight; y++) {
		for (uint32_t x = 0; x < bmpInfoTmp.biWidth; x++)
			Compatible_SetColor32ToColor24Dest(pDataTemp, bmpInfoTmp.biWidth, x, y, pBmp->pData[x + y * bmpInfoTmp.biWidth]);
	}

	fwrite(&bmpFileTmp, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&bmpInfoTmp, sizeof(BITMAPINFOHEADER), 1, fp);
	fwrite(pDataTemp, 1, bmpInfoTmp.biSizeImage, fp);

	fclose(fp);

	return true;
}

char* TrimStringQuotes(char *str) {
	int nStrLen;
	int nOffset;
	nOffset = 0;
	nStrLen = strlen(str);
	if (str[0] == '"' && str[nStrLen - 1] == '"') {
		str[nStrLen - 1] = '\0';
		nOffset++;
	}
	return &str[nOffset];
}

static int nDepth = 0;

bool ClipNextImage_SetMark(pBitmap pBmpSrc, pBitmap pBmpMask, Color32 clrBorder, int nThreshold, int x, int y, RECT *pRt) {
	bool bOutOfBounds = false;
	bool bSuccess;

	if (x < 0 || x >= GetBitmapWidth(pBmpSrc))
		return false;
	if (y < 0 || y >= GetBitmapHeight(pBmpSrc))
		return false;
	if (nThreshold <= 0)
		return false;
	if (GetBitmapPixel(pBmpMask, x, y) == 1)
		return false;
	if (GetBitmapPixel(pBmpSrc, x, y) == clrBorder) {
		if (nThreshold > 0)
			nThreshold--, bOutOfBounds = true;
		else
			return false;
	}

	//printf("Step into (%d, %d, threshold %d, color 0x%08x)...\n", x, y, nThreshold, GetBitmapPixel(pBmpSrc, x, y));

	nDepth++;
	if (nDepth > 3000) {
		nDepth--;
		PrintError("Too deep call");
		return false;
	}

	bSuccess = !bOutOfBounds;

	SetBitmapPixel(pBmpMask, x, y, 1);
	if (!bOutOfBounds) {
		pRt->left = min(pRt->left, x);
		pRt->right = max(pRt->right, x + 1);
		pRt->top = min(pRt->top, y);
		pRt->bottom = max(pRt->bottom, y + 1);
	}
	bSuccess = ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x - 1, y, pRt) || bSuccess;
	bSuccess = ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x + 1, y, pRt) || bSuccess;
	bSuccess = ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x, y - 1, pRt) || bSuccess;
	bSuccess = ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x, y + 1, pRt) || bSuccess;
	bSuccess = ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x - 1, y - 1, pRt) || bSuccess;
	bSuccess = ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x - 1, y + 1, pRt) || bSuccess;
	bSuccess = ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x + 1, y - 1, pRt) || bSuccess;
	bSuccess = ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x + 1, y + 1, pRt) || bSuccess;

	if (bOutOfBounds)
		SetBitmapPixel(pBmpMask, x, y, 0);

	nDepth--;

	return bSuccess;
}

typedef struct tagClipNextImageData_t {
	int xCurSearch, yCurSearch;
	pBitmap pBmpOut;
} ClipNextImageData_t;

bool ClipNextImage(pBitmap pBmpSrc, pBitmap pBmpMask, Color32 clrBorder, int nThreshold, ClipNextImageData_t *pData) {
	int nWidth = GetBitmapWidth(pBmpSrc), nHeight = GetBitmapHeight(pBmpSrc);
	RECT rt;

	for (int y = pData->yCurSearch; y < nHeight; y++) {
		for (int x = pData->xCurSearch; x < nWidth; x++) {
			SetRect(&rt, x, y, x, y);
			pData->xCurSearch = x;
			pData->yCurSearch = y;
			//printf("Search (%d, %d)...\n", x, y);
			if (GetBitmapPixel(pBmpSrc, x, y) == clrBorder)
				continue;

			if (ClipNextImage_SetMark(pBmpSrc, pBmpMask, clrBorder, nThreshold, x, y, &rt)) {
				PrintInfo("Cliped area: (%d, %d), (%d, %d)", rt.left, rt.top, rt.right, rt.bottom);
				if (rt.right - rt.left <= nThreshold && rt.bottom - rt.top <= nThreshold) {
					PrintWarning("Ignoring too small image");
					continue;
				}

				pData->pBmpOut = CreateEmptyBitmap(rt.right - rt.left, rt.bottom - rt.top);
				if (pData->pBmpOut == NULL)
					return true;

				for (int j = rt.top; j < rt.bottom; j++) {
					for (int i = rt.left; i < rt.right; i++)
						SetBitmapPixel(pData->pBmpOut, i - rt.left, j - rt.top, GetBitmapPixel(pBmpSrc, i, j));
				}

				return true;
			}
		}
		pData->xCurSearch = 0;
	}

	return false;
}

int main(void) {
	pBitmap pBmpIn, pBmpOut, pBmpMask;
	ClipNextImageData_t cniData;
	int cxOrigin, cyOrigin;
	char strPath2[1024];
	int cxDest, cyDest;
	char strPath[1024];
	Color32 clrBorder;
	int nThreshold;
	char *str;
	int i;

	PrintWarning("Bmp read function is experimental, use cautiously");

	printf("This program can split image into smaller ones.\n");

	printf("Enter file path (bmp only): ");
	scanf("%[^\n]", strPath);

	pBmpIn = CreateBitmapFromFile(TrimStringQuotes(strPath));
	if (pBmpIn == NULL) {
		PrintError("Cannot open bitmap");
		return EXIT_FAILURE;
	}

	cxOrigin = GetBitmapWidth(pBmpIn);
	cyOrigin = GetBitmapHeight(pBmpIn);
	PrintInfo("Bitmap dimension: (%d, %d)", cxOrigin, cyOrigin);

#if 0
	printf("Drawing image...\n");
	HDC hdc = GetDC(NULL);
	for (int y = 0; y < cyOrigin; y++) {
		for (int x = 0; x < cxOrigin; x++) {
			printf("0x%08x\r", GetBitmapPixel(pBmpIn, x, y));
			SetPixelV(hdc, x, y, GetBitmapPixel(pBmpIn, x, y) & 0x00ffffff);
		}
	}
	ReleaseDC(NULL, hdc);
#endif

	pBmpMask = CreateEmptyBitmap(cxOrigin, cyOrigin);
	if (pBmpMask == NULL) {
		PrintError("Cannot create mask bitmap");
		return EXIT_FAILURE;
	}

	printf("Input color as border: ");
	scanf("%" PRIx32, &clrBorder);

	printf("Input maximum clip threshold: ");
	scanf("%d", &nThreshold);

	printf("Input destination path: ");
	scanf(" %[^\n]", strPath);
	str = TrimStringQuotes(strPath);

	i = 1;
	cniData.xCurSearch = 0;
	cniData.yCurSearch = 0;
	while (ClipNextImage(pBmpIn, pBmpMask, clrBorder, nThreshold, &cniData)) {
		printf(" => Saving to image %d...\n", i);
		sprintf(strPath2, "%s\\out_%d.bmp", str, i);
		SaveBitmapToFile(cniData.pBmpOut, strPath2);
		DestroyBitmap(cniData.pBmpOut);
		i++;
	}

#if 0
	printf("Drawing mask...\n");
	HDC hdc = GetDC(NULL);
	for (int y = 0; y < cyOrigin; y++) {
		for (int x = 0; x < cxOrigin; x++) {
			printf("0x%08x\r", GetBitmapPixel(pBmpMask, x, y));
			SetPixelV(hdc, x, y, GetBitmapPixel(pBmpMask, x, y) * 0x00ffffff);
		}
	}
	ReleaseDC(NULL, hdc);
#endif

#if 1
	printf("Saving mask...\n");
	for (int y = 0; y < cyOrigin; y++) {
		for (int x = 0; x < cxOrigin; x++)
			SetBitmapPixel(pBmpMask, x, y, GetBitmapPixel(pBmpMask, x, y) * 0x00ffffff);
	}
	sprintf(strPath2, "%s\\out_mask.bmp", strPath);
	SaveBitmapToFile(pBmpMask, strPath2);
#endif

	DestroyBitmap(pBmpIn);
	DestroyBitmap(pBmpMask);

	printf("\nTotal %d images cliped.\n", i - 1);
}