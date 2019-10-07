#include <Windows.h>
#include <stdbool.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#pragma comment(lib, "shell32")

#define GetArrLen(arr) (sizeof(arr) / sizeof(*(arr)))

//It can parse DxLib V8 archive only

//Assuming strdup() exists
char* strdup(const char *str);

#define CheckDXHeader(buf) (((uint8_t*)(buf))[0] == 'D' && ((uint8_t*)(buf))[1] == 'X')

#define CLR_I FOREGROUND_INTENSITY
#define CLR_R FOREGROUND_RED
#define CLR_G FOREGROUND_GREEN
#define CLR_B FOREGROUND_BLUE

#define CLR_TEXT (CLR_R | CLR_G | CLR_B)
#define CLR_ERROR (CLR_I | CLR_R)
#define CLR_SUCCESS (CLR_I | CLR_G)
#define CLR_INFO (CLR_I | CLR_B)
#define CLR_ITEXT (CLR_I | CLR_TEXT)

#define BoolErrReturn(...) return SetErrMsg(__VA_ARGS__), false
#define VoidErrReturn(...) return (void)SetErrMsg(__VA_ARGS__)

//Console IO utilities & others
typedef struct _CONSOLE_READCONSOLE_CONTROL {
	ULONG nLength;
	ULONG nInitialChars;
	ULONG dwCtrlWakeupMask;
	ULONG dwControlKeyState;
} CONSOLE_READCONSOLE_CONTROL, *PCONSOLE_READCONSOLE_CONTROL;

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

bool EnableEcho(bool bEnable) {
	bool bOldState;
	DWORD nMode;
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &nMode);
	bOldState = nMode & ENABLE_ECHO_INPUT;
	if (bEnable)
		nMode |= ENABLE_ECHO_INPUT;
	else
		nMode &= ~ENABLE_ECHO_INPUT;
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), nMode);
	return bOldState;
}
bool EnableMouse(bool bEnable) {
	bool bOldState;
	DWORD nMode;
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &nMode);
	bOldState = nMode & ENABLE_MOUSE_INPUT;
	if (bEnable)
		nMode |= ENABLE_MOUSE_INPUT;
	else
		nMode &= ~ENABLE_MOUSE_INPUT;
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), nMode);
	return bOldState;
}
bool EnableManagedInput(bool bEnable) {
	bool bOldState;
	DWORD nMode;
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &nMode);
	bOldState = nMode & ENABLE_LINE_INPUT;
	if (bEnable)
		nMode |= ENABLE_LINE_INPUT;
	else
		nMode &= ~ENABLE_LINE_INPUT;
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), nMode);
	return bOldState;
}

void gotoxy(int x, int y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD) { x, y });
}
int wherex(void) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return csbi.dwCursorPosition.X;
}
int wherey(void) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return csbi.dwCursorPosition.Y;
}
int GetConsoleWidth(void) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return csbi.dwSize.X;
}
int GetConsoleHeight(void) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return csbi.dwSize.Y;
}

char global_ErrMsg[4096];

void SetErrMsg(const char *fmt, ...) {
	char strErrMsgTemp[GetArrLen(global_ErrMsg)];
	va_list vl;
	va_start(vl, fmt);
	vsprintf(strErrMsgTemp, fmt, vl);
	va_end(vl);
	strcpy(global_ErrMsg, strErrMsgTemp);
}

const char* GetErrMsg(void) {
	return global_ErrMsg;
}

void PrintErrMsg(void) {
	PushColor(CLR_ITEXT);
	PushColor(CLR_ERROR);
	printf("Error");
	PopColor();
	printf(": %s\n", global_ErrMsg);
	PopColor();
}

void GenerateTempFilePath(char *path) {
	char strPathTmp[MAX_PATH];
	GetTempPath(MAX_PATH, strPathTmp);
	GetTempFileName(strPathTmp, "~Tmp", rand(), path);
}

bool OpenFolderAndSelectItem(const char *path) {
	//Better use SHParseDisplayName() & SHOpenFolderAndSelectItems() instead
	char *strTemp;

	if (path == NULL)
		return false;

	strTemp = (char*)malloc(strlen(path) + 32);
	if (strTemp == NULL)
		return false;
	sprintf(strTemp, "/select,\"%s\"", path);

	ShellExecute(NULL, NULL, "explorer.exe", strTemp, NULL, SW_SHOWDEFAULT);

	free(strTemp);

	return true;
}

LPSTR Win_PathCombineA(LPSTR pszDest, LPCSTR pszDir, LPCSTR pszFile) {
	LPSTR (*pFunc)(LPSTR pszDest, LPCSTR pszDir, LPCSTR pszFile);
	if (GetModuleHandle("Shlwapi.dll") == NULL)
		LoadLibrary("Shlwapi.dll");
	pFunc = (LPSTR(*)(LPSTR, LPCSTR, LPCSTR))GetProcAddress(GetModuleHandle("Shlwapi.dll"), "PathCombineA");
	if (pFunc == NULL)
		return NULL;
	return pFunc(pszDest, pszDir, pszFile);
}

char* strnchr(const char *s, size_t count, int c) {
	for (; count-- && *s != '\0'; ++s)
		if (*s == (char)c)
			return (char*)s;
	return NULL;
}

bool isslash(int ch) {
	return ch == '/' || ch == '\\';
}

//DxLib definitions & functions (mostly directly copied from DxLib source code)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DXAHEAD							*((WORD *)"DX")		// ヘッダ
#define DXAVER							(0x0008)			// バージョン
#define DXAVER_MIN						(0x0008)			// 対応している最低バージョン
#define DXA_KEY_BYTES					(7)					// 鍵のバイト数
#define DXA_KEY_STRING_LENGTH			(63)				// 鍵用文字列の長さ
#define DXA_KEY_STRING_MAXLENGTH		(2048)				// 鍵用文字列バッファのサイズ
#define DXA_DIR_MAXARCHIVENUM			(4096)				// 同時に開いておけるアーカイブファイルの数
#define DXA_DIR_MAXFILENUM				(32768)				// 同時に開いておけるファイルの数
#define DXA_MAXDRIVENUM					(64)				// 対応するドライブの最大数

#define DXARC_ID_AND_VERSION_SIZE	(sizeof( WORD ) * 2)
#define DXARC_HEAD_VER8_SIZE		(64)
#define DXARC_FILEHEAD_VER8_SIZE	(72)			// Ver0x0008 の DXARC_FILEHEAD 構造体のサイズ

// フラグ
#define DXA_FLAG_NO_KEY					(0x00000001)	// 鍵処理無し
#define DXA_FLAG_NO_HEAD_PRESS			(0x00000002)	// ヘッダの圧縮無し

// データを解凍する( 戻り値:解凍後のデータサイズ )
#define MIN_COMPRESS		(4)						// 最低圧縮バイト数
#define MAX_SEARCHLISTNUM	(64)					// 最大一致長を探す為のリストを辿る最大数
#define MAX_SUBLISTNUM		(65536)					// 圧縮時間短縮のためのサブリストの最大数
#define MAX_COPYSIZE 		(0x1fff + MIN_COMPRESS)	// 参照アドレスからコピー出切る最大サイズ( 圧縮コードが表現できるコピーサイズの最大値 + 最低圧縮バイト数 )
#define MAX_ADDRESSLISTNUM	(1024 * 1024 * 1)		// スライド辞書の最大サイズ
#define MAX_POSITION		(1 << 24)				// 参照可能な最大相対アドレス( 16MB )

// アーカイブデータの最初のヘッダ
struct DXARC_HEAD {
	WORD						Head ;							// ＩＤ
	WORD						Version ;						// バージョン
	DWORD						HeadSize ;						// ヘッダ情報の DXARC_HEAD を抜いた全サイズ
	ULONGLONG					DataStartAddress ;				// 最初のファイルのデータが格納されているデータアドレス(ファイルの先頭アドレスをアドレス０とする)
	ULONGLONG					FileNameTableStartAddress ;		// ファイル名テーブルの先頭アドレス(ファイルの先頭アドレスをアドレス０とする)
	ULONGLONG					FileTableStartAddress ;			// ファイルテーブルの先頭アドレス(メンバ変数 FileNameTableStartAddress のアドレスを０とする)
	ULONGLONG					DirectoryTableStartAddress ;	// ディレクトリテーブルの先頭アドレス(メンバ変数 FileNameTableStartAddress のアドレスを０とする)
																// アドレス０から配置されている DXARC_DIRECTORY 構造体がルートディレクトリ
	DWORD						CharCodeFormat ;				// ファイル名に使用しているコードページ番号
	DWORD						Flags ;							// フラグ( DXA_FLAG_NO_KEY 等 )
	BYTE						HuffmanEncodeKB ;				// ファイルの前後のハフマン圧縮するサイズ( 単位：キロバイト 0xff の場合はすべて圧縮する )
	BYTE						Reserve[ 15 ] ;					// 予約領域
};

// ファイルの時間情報
struct DXARC_FILETIME {
	ULONGLONG					Create ;						// 作成時間
	ULONGLONG					LastAccess ;					// 最終アクセス時間
	ULONGLONG					LastWrite ;						// 最終更新時間
};

// ファイル名データ構造体
struct DXARC_FILENAME {
	WORD						Length ;						// 文字列の長さ÷４
	WORD						Parity ;						// パリティ情報
};

// ファイル格納情報
struct DXARC_FILEHEAD {
	ULONGLONG					NameAddress ;					// ファイル名が格納されているアドレス( ARCHIVE_HEAD構造体 のメンバ変数 FileNameTableStartAddress のアドレスをアドレス０とする)
	ULONGLONG					Attributes ;					// ファイル属性
	struct DXARC_FILETIME		Time ;							// 時間情報
	ULONGLONG					DataAddress ;					// ファイルが格納されているアドレス
																//			ファイルの場合：DXARC_HEAD構造体 のメンバ変数 DataStartAddress が示すアドレスをアドレス０とする
																//			ディレクトリの場合：DXARC_HEAD構造体 のメンバ変数 DirectoryTableStartAddress のが示すアドレスをアドレス０とする
	ULONGLONG					DataSize ;						// ファイルのデータサイズ
	ULONGLONG					PressDataSize ;					// 圧縮後のデータのサイズ( 0xffffffff:圧縮されていない ) ( Ver0x0002 で追加された )
	ULONGLONG					HuffPressDataSize ;				// ハフマン圧縮後のデータのサイズ( 0xffffffffffffffff:圧縮されていない ) ( Ver0x0008 で追加された )
};

// ディレクトリ格納情報
struct DXARC_DIRECTORY {
	ULONGLONG					DirectoryAddress ;				// 自分の DXARC_FILEHEAD が格納されているアドレス( DXARC_HEAD 構造体 のメンバ変数 FileTableStartAddress が示すアドレスをアドレス０とする)
	ULONGLONG					ParentDirectoryAddress ;		// 親ディレクトリの DXARC_DIRECTORY が格納されているアドレス( DXARC_HEAD構造体 のメンバ変数 DirectoryTableStartAddress が示すアドレスをアドレス０とする)
	ULONGLONG					FileHeadNum ;					// ディレクトリ内のファイルの数
	ULONGLONG					FileHeadAddress ;				// ディレクトリ内のファイルのヘッダ列が格納されているアドレス( DXARC_HEAD構造体 のメンバ変数 FileTableStartAddress が示すアドレスをアドレス０とする)
};



// ファイル名検索用データ構造体
struct DXARC_SEARCHDATA {
	BYTE						FileName[ 1024 ] ;				// ファイル名
	WORD						Parity ;						// パリティ情報
	WORD						PackNum ;						// 文字列の長さ÷４
};

// 情報テーブル構造体
struct DXARC_TABLE {
	BYTE						*Top ;							// 情報テーブルの先頭ポインタ
	BYTE						*FileTable ;					// ファイル情報テーブルへのポインタ
	BYTE						*DirectoryTable ;				// ディレクトリ情報テーブルへのポインタ
	BYTE						*NameTable ;					// 名前情報テーブルへのポインタ
};

// アーカイブ処理用情報構造体
struct DXARC {
	struct DXARC_HEAD			Head ;							// アーカイブのヘッダ
	int							CharCodeFormat ;				// 文字コード形式
	DWORD_PTR					ReadAccessOnlyFilePointer ;		// アーカイブファイルのポインタ
	void						*MemoryImage ;					// メモリイメージを開いた場合のアドレス
	struct DXARC_TABLE			Table ;							// 各テーブルへの先頭アドレスが格納された構造体
	struct DXARC_DIRECTORY		*CurrentDirectory ;				// カレントディレクトリデータへのポインタ

	wchar_t						FilePath[ 1024 ] ;				// ファイルパス
	bool						NoKey ;							// 鍵処理を行わないかどうか
	unsigned char				Key[ DXA_KEY_BYTES ] ;			// 鍵
	char						KeyString[ DXA_KEY_STRING_LENGTH + 1 ] ;	// 鍵文字列
	size_t						KeyStringBytes ;				// 鍵文字列のバイト数
	int							MemoryOpenFlag ;				// メモリ上のファイルを開いているか、フラグ
	int							UserMemoryImageFlag ;			// ユーザーが展開したメモリイメージを使用しているか、フラグ
	LONGLONG					MemoryImageSize ;				// メモリ上のファイルから開いていた場合のイメージのサイズ
	int							MemoryImageCopyFlag ;			// メモリ上のイメージをコピーして使用しているかどうかのフラグ
	int							MemoryImageReadOnlyFlag ;		// メモリ上のイメージが読み取り専用かどうかのフラグ
	void						*MemoryImageOriginal ;			// メモリ上のイメージをコピーして使用している場合の、コピー元のデータが格納されているメモリ領域

	int							ASyncOpenFlag ;					// 非同期読み込み中かフラグ( TRUE:非同期読み込み中 FALSE:違う )
	DWORD_PTR					ASyncOpenFilePointer ;			// 非同期オープン処理に使用するファイルのポインタ

	FILE						*fpArchive;
};

// 数値ごとの出現数や算出されたエンコード後のビット列や、結合部分の情報等の構造体
struct HUFFMAN_NODE {
	ULONGLONG					Weight ;						// 出現数( 結合データでは出現数を足したモノ )
	int							BitNum ;						// 圧縮後のビット列のビット数( 結合データでは使わない )
	unsigned char				BitArray[ 32 ] ;				// 圧縮後のビット列( 結合データでは使わない )
	int							Index ;							// 結合データに割り当てられた参照インデックス( 0 or 1 )

	int							ParentNode ;					// このデータを従えている結合データの要素配列のインデックス
	int							ChildNode[ 2 ] ;				// このデータが結合させた２要素の要素配列インデックス( 結合データではない場合はどちらも -1 )
};

// ビット単位入出力用データ構造体
struct BIT_STREAM {
	BYTE						*Buffer;
	ULONGLONG					Bytes;
	DWORD						Bits;
};

// 圧縮時間短縮用リスト
typedef struct LZ_LIST {
	struct LZ_LIST *next, *prev ;
	uint32_t address ;
} LZ_LIST;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ビット単位入出力の初期化
void BitStream_Init(struct BIT_STREAM *BitStream, void *Buffer, bool IsRead) {
	BitStream->Buffer = (BYTE*)Buffer;
	BitStream->Bytes = 0;
	BitStream->Bits = 0;
	if (IsRead == false)
		BitStream->Buffer[0] = 0;
}

// ビット単位の数値の書き込みを行う
void BitStream_Write(struct BIT_STREAM *BitStream, BYTE BitNum, ULONGLONG OutputData) {
	DWORD i;
	for (i = 0; i < BitNum; i++) {
		BitStream->Buffer[BitStream->Bytes] |= ((OutputData >> (BitNum - 1 - i)) & 1) << (7 - BitStream->Bits);
		BitStream->Bits++;
		if (BitStream->Bits == 8) {
			BitStream->Bytes++;
			BitStream->Bits = 0;
			BitStream->Buffer[BitStream->Bytes] = 0;
		}
	}
}

// ビット単位の数値の読み込みを行う
ULONGLONG BitStream_Read(struct BIT_STREAM *BitStream, BYTE BitNum) {
	ULONGLONG Result = 0;
	DWORD i;
	for (i = 0; i < BitNum; i++) {
		Result |= ((ULONGLONG)((BitStream->Buffer[BitStream->Bytes] >> (7 - BitStream->Bits)) & 1)) << (BitNum - 1 - i);
		BitStream->Bits++;
		if (BitStream->Bits == 8) {
			BitStream->Bytes++;
			BitStream->Bits = 0;
		}
	}

	return Result;
}

// 指定の数値のビット数を取得する
BYTE BitStream_GetBitNum(ULONGLONG Data) {
	DWORD i;
	for (i = 1; i < 64; i++) {
		if (Data < (1ULL << i))
			return (BYTE)i;
	}

	return (BYTE)i;
}

// ビット単位の入出力データのサイズ( バイト数 )を取得する
ULONGLONG BitStream_GetBytes(struct BIT_STREAM *BitStream) {
	return BitStream->Bytes + ( BitStream->Bits != 0 ? 1 : 0 );
}

DWORD DX_HashCRC32(const void *SrcData, size_t SrcDataSize) {
	static int CRC32TableInit = 0;
	static DWORD CRC32Table[256];
	BYTE *SrcByte;
	DWORD CRC;
	DWORD i;

	SrcByte = (BYTE*)SrcData;
	CRC = 0xffffffff;

	if (CRC32TableInit == 0) {
		DWORD Magic = 0xedb88320;
		DWORD j;

		for (i = 0; i < 256; i++) {
			DWORD Data = i;
			for (j = 0; j < 8; j++) {
				int b = Data & 1;
				Data >>= 1;
				if (b != 0)
					Data ^= Magic;
			}
			CRC32Table[i] = Data;
		}

		CRC32TableInit = 1;
	}

	for (i = 0; i < SrcDataSize; i++)
		CRC = CRC32Table[(BYTE)(CRC ^ SrcByte[i])] ^ (CRC >> 8);

	return CRC ^ 0xffffffff;
}

int DXA_Decode(void *Src, void *Dest) {
	uint32_t srcsize, destsize, code, indexsize, keycode, conbo, index = 0;
	uint8_t *srcp, *destp, *dp, *sp;

	destp = (uint8_t*)Dest;
	srcp = (uint8_t*)Src;

	// 解凍後のデータサイズを得る
	destsize = *((uint32_t*)&srcp[0]);

	// 圧縮データのサイズを得る
	srcsize = *((uint32_t*)&srcp[4]) - 9;

	// キーコード
	keycode = srcp[8];

	// 出力先がない場合はサイズだけ返す
	if (Dest == NULL)
		return (int)destsize;

	// 展開開始
	sp = srcp + 9;
	dp = destp;
	while (srcsize) {
		// キーコードか同かで処理を分岐
		if (sp[0] != keycode) {
			// 非圧縮コードの場合はそのまま出力
			*dp = *sp;
			dp++;
			sp++;
			srcsize--;
			continue;
		}

		// キーコードが連続していた場合はキーコード自体を出力
		if (sp[1] == keycode) {
			*dp = (uint8_t)keycode;
			dp++;
			sp += 2;
			srcsize -= 2;
			continue;
		}

		// 第一バイトを得る
		code = sp[1];

		// もしキーコードよりも大きな値だった場合はキーコード
		// とのバッティング防止の為に＋１しているので－１する
		if (code > keycode)
			code--;

		sp += 2;
		srcsize -= 2;

		// 連続長を取得する
		conbo = code >> 3;
		if (code & (0x1 << 2)) {
			conbo |= *sp << 5;
			sp++;
			srcsize--;
		}
		conbo += MIN_COMPRESS;	// 保存時に減算した最小圧縮バイト数を足す

		// 参照相対アドレスを取得する
		indexsize = code & 0x3;
		switch (indexsize) {
		case 0:
			index = *sp;
			sp++;
			srcsize--;
			break;

		case 1:
			index = *((uint16_t*)sp);
			sp += 2;
			srcsize -= 2;
			break;

		case 2:
			index = *((uint16_t*)sp) | (sp[2] << 16);
			sp += 3;
			srcsize -= 3;
			break;
		}
		index++;		// 保存時に－１しているので＋１する

		// 展開
		if (index < conbo) {
			uint32_t num;

			num = index;
			while (conbo > num) {
				memcpy(dp, dp - num, num);
				dp += num;
				conbo -= num;
				num += num;
			}
			if (conbo != 0) {
				memcpy(dp, dp - num, conbo);
				dp += conbo;
			}
		}
		else {
			memcpy(dp, dp - index, conbo);
			dp += conbo;
		}
	}

	// 解凍後のサイズを返す
	return (int)destsize;
}

// ハフマン圧縮されたデータを解凍する( 戻り値:解凍後のサイズ  0 はエラー  Dest に NULL を入れると解凍データ格納に必要なサイズが返る )
ULONGLONG Huffman_Decode(void *Press, void *Dest) {
	// 結合データと数値データ、０～２５５までが数値データ
	struct HUFFMAN_NODE Node[256 + 255];

	ULONGLONG PressSizeCounter, DestSizeCounter, DestSize;
	unsigned char *PressPoint, *DestPoint;
	ULONGLONG OriginalSize;
//	ULONGLONG PressSize;
	ULONGLONG HeadSize;
	WORD Weight[256];
	int i;

	// void 型のポインタではアドレスの操作が出来ないので unsigned char 型のポインタにする
	PressPoint = ( unsigned char * )Press ;
	DestPoint = ( unsigned char * )Dest ;

	// 圧縮データの情報を取得する
	{
		struct BIT_STREAM BitStream ;
		BYTE BitNum ;
		BYTE Minus ;
		WORD SaveData ;

		BitStream_Init( &BitStream, PressPoint, true ) ;

		OriginalSize = BitStream_Read( &BitStream, ( BYTE )( BitStream_Read( &BitStream, 6 ) + 1 ) ) ;
//		PressSize    = BitStream_Read( &BitStream, ( BYTE )( BitStream_Read( &BitStream, 6 ) + 1 ) ) ;
		BitStream_Read( &BitStream, ( BYTE )( BitStream_Read( &BitStream, 6 ) + 1 ) ) ;

		// 出現頻度のテーブルを復元する
		BitNum      = ( BYTE )( BitStream_Read( &BitStream, 3 ) + 1 ) * 2 ;
		Minus       = ( BYTE )BitStream_Read( &BitStream, 1 ) ;
		SaveData    = ( WORD )BitStream_Read( &BitStream, BitNum ) ;
		Weight[ 0 ] = SaveData ;
		for ( i = 1 ; i < 256 ; i ++ ) {
			BitNum      = ( BYTE )( BitStream_Read( &BitStream, 3 ) + 1 ) * 2 ;
			Minus       = ( BYTE )BitStream_Read( &BitStream, 1 ) ;
			SaveData    = ( WORD )BitStream_Read( &BitStream, BitNum ) ;
			Weight[ i ] = Minus == 1 ? Weight[ i - 1 ] - SaveData : Weight[ i - 1 ] + SaveData ;
		}

		HeadSize = BitStream_GetBytes( &BitStream ) ;
	}

	// Dest が NULL の場合は 解凍後のデータのサイズを返す
	if ( Dest == NULL )
		return OriginalSize ;

	// 解凍後のデータのサイズを取得する
	DestSize = OriginalSize ;

	// 各数値の結合データを構築する
	{
		int NodeIndex, MinNode1, MinNode2 ;
		int NodeNum, DataNum ;

		// 数値データを初期化する
		for ( i = 0 ; i < 256 + 255 ; i ++ ) {
			Node[i].Weight = i < 256 ? Weight[i] : 0 ;	// 出現数は保存しておいたデータからコピー
			Node[i].ChildNode[0] = -1 ;					// 数値データが終点なので -1 をセットする
			Node[i].ChildNode[1] = -1 ;					// 数値データが終点なので -1 をセットする
			Node[i].ParentNode = -1 ;					// まだどの要素とも結合されていないので -1 をセットする
		}

		// 出現数の少ない数値データ or 結合データを繋いで
		// 新しい結合データを作成、全ての要素を繋いで残り１個になるまで繰り返す
		// (圧縮時と同じコードです)
		DataNum = 256 ; // 残り要素数
		NodeNum = 256 ; // 次に新しく作る結合データの要素配列のインデックス
		while( DataNum > 1 ) {
			// 出現数値の低い要素二つを探す
			{
				MinNode1 = -1 ;
				MinNode2 = -1 ;

				// 残っている要素全てを調べるまでループ
				NodeIndex = 0 ;
				for ( i = 0 ; i < DataNum ; NodeIndex ++ ) {
					// もう既に何処かの要素と結合されている場合は対象外
					if ( Node[NodeIndex].ParentNode != -1 ) continue ;

					i ++ ;

					// まだ有効な要素をセットしていないか、より出現数値の
					// 少ない要素が見つかったら更新
					if ( MinNode1 == -1 || Node[MinNode1].Weight > Node[NodeIndex].Weight ) {
						// 今まで一番出現数値が少なかったと思われた
						// 要素は二番目に降格
						MinNode2 = MinNode1 ;

						// 新しい一番の要素の要素配列のインデックスを保存
						MinNode1 = NodeIndex ;
					}
					else {
						// 一番よりは出現数値が多くても、二番目よりは出現数値が
						// 少ないかもしれないので一応チェック(又は二番目に出現数値の
						// 少ない要素がセットされていなかった場合もセット)
						if (MinNode2 == -1 || Node[MinNode2].Weight > Node[NodeIndex].Weight)
							MinNode2 = NodeIndex;
					}
				}
			}

			// 二つの要素を繋いで新しい要素(結合データ)を作る
			Node[NodeNum].ParentNode = -1 ;  // 新しいデータは当然まだ何処とも繋がっていないので -1
			Node[NodeNum].Weight = Node[MinNode1].Weight + Node[MinNode2].Weight ;    // 出現数値は二つの数値を足したものをセットする
			Node[NodeNum].ChildNode[0] = MinNode1 ;    // この結合部で 0 を選んだら出現数値が一番少ない要素に繋がる
			Node[NodeNum].ChildNode[1] = MinNode2 ;    // この結合部で 1 を選んだら出現数値が二番目に少ない要素に繋がる

			// 結合された要素二つに、自分達に何の値が割り当てられたかをセットする
			Node[MinNode1].Index = 0 ;    // 一番出現数値が少ない要素は 0 番
			Node[MinNode2].Index = 1 ;    // 二番目に出現数値が少ない要素は 1 番

			// 結合された要素二つに、自分達を結合した結合データの要素配列インデックスをセットする
			Node[MinNode1].ParentNode = NodeNum ;
			Node[MinNode2].ParentNode = NodeNum ;

			// 要素の数を一個増やす
			NodeNum ++ ;

			// 残り要素の数は、一つ要素が新しく追加された代わりに
			// 二つの要素が結合されて検索の対象から外れたので
			// 結果 1 - 2 で -1
			DataNum -- ;
		}

		// 各数値の圧縮時のビット列を割り出す
		{
			unsigned char TempBitArray[32] ;
			int TempBitIndex, TempBitCount, BitIndex, BitCount ;

			// 数値データと結合データの数だけ繰り返す
			for ( i = 0 ; i < 256 + 254 ; i ++ ) {
				// 数値データから結合データを上へ上へと辿ってビット数を数える
				{
					// ビット数を初期化しておく
					Node[i].BitNum = 0 ;

					// 一時的に数値データから遡っていったときのビット列を保存する処理の準備
					TempBitIndex = 0 ;
					TempBitCount = 0 ;
					TempBitArray[TempBitIndex] = 0 ;

					// 何処かと結合されている限りカウントし続ける(天辺は何処とも結合されていないので終点だと分かる)
					for ( NodeIndex = ( int )i ; Node[NodeIndex].ParentNode != -1 ; NodeIndex = Node[NodeIndex].ParentNode ) {
						// 配列要素一つに入るビットデータは８個なので、同じ配列要素に
						// 既に８個保存していたら次の配列要素に保存先を変更する
						if ( TempBitCount == 8 ) {
							TempBitCount = 0 ;
							TempBitIndex ++ ;
							TempBitArray[TempBitIndex] = 0 ;
						}

						// 新しく書き込む情報で今までのデータを上書きしてしまわないように１ビット左にシフトする
						TempBitArray[TempBitIndex] <<= 1 ;

						// 結合データに割り振られたインデックスを最下位ビット(一番右側のビット)に書き込む
						TempBitArray[TempBitIndex] |= (unsigned char)Node[NodeIndex].Index ;

						// 保存したビット数を増やす
						TempBitCount ++ ;

						// ビット数を増やす
						Node[i].BitNum ++ ;
					}
				}

				// TempBitArray に溜まったデータは数値データから結合データを天辺に向かって
				// 上へ上へと遡っていった時のビット列なので、逆さまにしないと圧縮後のビット
				// 配列として使えない(展開時に天辺の結合データから数値データまで辿ることが
				// 出来ない)ので、順序を逆さまにしたものを数値データ内のビット列バッファに保存する
				{
					BitCount = 0 ;
					BitIndex = 0 ;

					// 最初のバッファを初期化しておく
					// (全部 論理和(or)演算 で書き込むので、最初から１になっている
					// ビットに０を書き込んでも１のままになってしまうため)
					Node[i].BitArray[BitIndex] = 0 ;

					// 一時的に保存しておいたビット列の最初まで遡る
					while ( TempBitIndex >= 0 ) {
						// 書き込んだビット数が一つの配列要素に入る８ビットに
						// 達してしまったら次の配列要素に移る
						if ( BitCount == 8 ) {
							BitCount = 0 ;
							BitIndex ++ ;
							Node[i].BitArray[BitIndex] = 0 ;
						}

						// まだ何も書き込まれていないビットアドレスに１ビット書き込む
						Node[i].BitArray[BitIndex] |= (unsigned char)( ( TempBitArray[TempBitIndex] & 1 ) << BitCount ) ;

						// 書き込み終わったビットはもういらないので次のビットを
						// 書き込めるように１ビット右にシフトする
						TempBitArray[TempBitIndex] >>= 1 ;

						// １ビット書き込んだので残りビット数を１個減らす
						TempBitCount -- ;

						// もし現在書き込み元となっている配列要素に書き込んでいない
						// ビット情報が無くなったら次の配列要素に移る
						if ( TempBitCount == 0 ) {
							TempBitIndex -- ;
							TempBitCount = 8 ;
						}

						// 書き込んだビット数を増やす
						BitCount++;
					}
				}
			}
		}
	}

	// 解凍処理
	{
		unsigned char *PressData ;
		int PressBitCounter, PressBitData, Index, NodeIndex ;
		int NodeIndexTable[ 512 ] ;
		int j ;

		// 各ビット配列がどのノードに繋がるかのテーブルを作成する
		{
			WORD BitMask[ 9 ] ;

			for ( i = 0 ; i < 9 ; i ++ )
				BitMask[ i ] = ( WORD )( ( 1 << ( i + 1 ) ) - 1 );

			for ( i = 0 ; i < 512 ; i ++ ) {
				NodeIndexTable[ i ] = -1 ;

				// ビット列に適合したノードを探す
				for ( j = 0 ; j < 256 + 254 ; j ++ ) {
					WORD BitArray01 ;

					if( Node[ j ].BitNum > 9 )
						continue ;

					BitArray01 = ( WORD )Node[ j ].BitArray[ 0 ] | ( Node[ j ].BitArray[ 1 ] << 8 ) ;
					if ( ( i & BitMask[ Node[ j ].BitNum - 1 ] ) == ( BitArray01 & BitMask[ Node[ j ].BitNum - 1 ] ) ) {
						NodeIndexTable[ i ] = j ;
						break ;
					}
				}
			}
		}

		// 圧縮データ本体の先頭アドレスをセット
		// (圧縮データ本体は元のサイズ、圧縮後のサイズ、各数値の出現数等を
		// 格納するデータ領域の後にある)
		PressData = PressPoint + HeadSize ;

		// 解凍したデータの格納アドレスを初期化
		DestSizeCounter = 0 ;

		// 圧縮データの参照アドレスを初期化
		PressSizeCounter = 0 ;

		// 圧縮ビットデータのカウンタを初期化
		PressBitCounter = 0 ;

		// 圧縮データの１バイト目をセット
		PressBitData = PressData[PressSizeCounter] ;

		// 圧縮前のデータサイズになるまで解凍処理を繰り返す
		for ( DestSizeCounter = 0 ; DestSizeCounter < DestSize ; DestSizeCounter ++ ) {
			// ビット列から数値データを検索する
			{
				// 最後の17byte分のデータは天辺から探す( 最後の次のバイトを読み出そうとしてメモリの不正なアクセスになる可能性があるため )
				if ( DestSizeCounter >= DestSize - 17 ) {
					// 結合データの天辺は一番最後の結合データが格納される５１０番目(０番から数える)
					// 天辺から順に下に降りていく
					NodeIndex = 510 ;
				}
				else {
					// それ以外の場合はテーブルを使用する

					// もし PressBitData に格納されている全ての
					// ビットデータを使い切ってしまった場合は次の
					// ビットデータをセットする
					if ( PressBitCounter == 8 ) {
						PressSizeCounter ++ ;
						PressBitData = PressData[PressSizeCounter] ;
						PressBitCounter = 0 ;
					}

					// 圧縮データを9bit分用意する
					PressBitData = ( PressBitData | ( PressData[ PressSizeCounter + 1 ] << ( 8 - PressBitCounter ) ) ) & 0x1ff ;

					// テーブルから最初の結合データを探す
					NodeIndex = NodeIndexTable[ PressBitData ] ;

					// 使った分圧縮データのアドレスを進める
					PressBitCounter += Node[ NodeIndex ].BitNum ;
					if ( PressBitCounter >= 16 ) {
						PressSizeCounter += 2 ;
						PressBitCounter -= 16 ;
						PressBitData = PressData[PressSizeCounter] >> PressBitCounter ;
					}
					else if ( PressBitCounter >= 8 ) {
						PressSizeCounter ++ ;
						PressBitCounter -= 8 ;
						PressBitData = PressData[PressSizeCounter] >> PressBitCounter ;
					}
					else {
						PressBitData >>= Node[ NodeIndex ].BitNum ;
					}
				}

				// 数値データに辿り着くまで結合データを下りていく
				while ( NodeIndex > 255 ) {
					// もし PressBitData に格納されている全ての
					// ビットデータを使い切ってしまった場合は次の
					// ビットデータをセットする
					if ( PressBitCounter == 8 ) {
						PressSizeCounter ++ ;
						PressBitData = PressData[PressSizeCounter] ;
						PressBitCounter = 0 ;
					}

					// １ビット取得する
					Index = PressBitData & 1 ;

					// 使用した１ビット分だけ右にシフトする
					PressBitData >>= 1 ;

					// 使用したビット数を一個増やす
					PressBitCounter ++ ;

					// 次の要素(結合データか数値データかはまだ分からない)に移る
					NodeIndex = Node[NodeIndex].ChildNode[Index] ;
				}
			}

			// 辿り着いた数値データを出力
			DestPoint[DestSizeCounter] = (unsigned char)NodeIndex ;
		}
	}

	// 解凍後のサイズを返す
	return OriginalSize ;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Custom DxLib + other definitions & functions
static char DefaultKeyString[9] = { 0x44, 0x58, 0x42, 0x44, 0x58, 0x41, 0x52, 0x43, 0x00 }; // "DXLIBARC"

typedef struct DXARC DXArchive, *pDXArchive;
typedef struct DXARC_DIRECTORY DXDir, *pDXDir;
typedef struct DXARC_FILEHEAD DXFileHead, *pDXFileHead;

typedef struct tagDXAKey {
	uint8_t key[DXA_KEY_BYTES];
} DXAKey, *pDXAKey;

void DX_CreateKey(const char *Source, size_t SourceBytes, pDXAKey pKey) {
	char SourceTempBuffer[1024];
	char WorkBuffer[1024];
	char *UseWorkBuffer;
	DWORD CRC32_0;
	DWORD CRC32_1;
	DWORD i, j;

	if (SourceBytes == 0)
		SourceBytes = strlen(Source);

	if (SourceBytes < 4) {
		strcpy(SourceTempBuffer, Source);
		strcpy(&SourceTempBuffer[SourceBytes], DefaultKeyString);
		Source = SourceTempBuffer;
		SourceBytes = strlen(Source);
	}

	if (SourceBytes / 2 > sizeof(WorkBuffer))
		UseWorkBuffer = (char*)malloc(SourceBytes / 2);
	else
		UseWorkBuffer = WorkBuffer;

	j = 0;
	for (i = 0; i < SourceBytes; i += 2, j++)
		UseWorkBuffer[j] = Source[i];
	CRC32_0 = DX_HashCRC32(UseWorkBuffer, j);

	j = 0;
	for (i = 1; i < SourceBytes; i += 2, j++)
		UseWorkBuffer[j] = Source[i];
	CRC32_1 = DX_HashCRC32(UseWorkBuffer, j);

	pKey->key[0] = (BYTE)(CRC32_0 >> 0);
	pKey->key[1] = (BYTE)(CRC32_0 >> 8);
	pKey->key[2] = (BYTE)(CRC32_0 >> 16);
	pKey->key[3] = (BYTE)(CRC32_0 >> 24);
	pKey->key[4] = (BYTE)(CRC32_1 >> 0);
	pKey->key[5] = (BYTE)(CRC32_1 >> 8);
	pKey->key[6] = (BYTE)(CRC32_1 >> 16);

	if (SourceBytes > sizeof(WorkBuffer))
		free(UseWorkBuffer);
}

void DX_CalcKey(const char *KeyString, pDXAKey pKey) {
	char KeyStringBuf[DXA_KEY_STRING_LENGTH + 1];
	int nKeyStringLen;

	if (KeyString == NULL)
		KeyString = DefaultKeyString;

	nKeyStringLen = strlen(KeyString);
	if (nKeyStringLen > DXA_KEY_STRING_LENGTH)
		nKeyStringLen = DXA_KEY_STRING_LENGTH;
	memcpy(KeyStringBuf, KeyString, nKeyStringLen);
	KeyStringBuf[nKeyStringLen] = '\0';

	DX_CreateKey(KeyString, nKeyStringLen, pKey);
}

void DX_SetKey(pDXAKey pKey, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
	pKey->key[0] = b1;
	pKey->key[1] = b2;
	pKey->key[2] = b3;
	pKey->key[3] = b4;
	pKey->key[4] = b5;
	pKey->key[5] = b6;
	pKey->key[6] = b7;
}

void DecodeData(const void *pIn, void *pOut, size_t nSize, size_t nOffset, pDXAKey pKey) {
	const uint8_t *pInBuf;
	uint32_t pKeyBuf[7];
	uint8_t *pOutBuf;
	int nLoops;

	if (pKey == NULL)	//NoKey flag is present
		return;

	pInBuf = (const uint8_t*)pIn;
	pOutBuf = (uint8_t*)pOut;

	if (!((uintptr_t)pInBuf & 3) && !((uintptr_t)pOutBuf & 3)) {
		//Both pInBuf and pOutBuf are aligned
		for (size_t i = 0; i < 4 * 7; i++)
			((uint8_t*)pKeyBuf)[i] = pKey->key[(i + nOffset) % DXA_KEY_BYTES];

		nLoops = nSize / (4 * 7);
		for (int i = 0; i < nLoops; i++) {
			((uint32_t*)pOutBuf)[i * 7 + 0] = ((const uint32_t*)pOutBuf)[i * 7 + 0] ^ pKeyBuf[0];
			((uint32_t*)pOutBuf)[i * 7 + 1] = ((const uint32_t*)pOutBuf)[i * 7 + 1] ^ pKeyBuf[1];
			((uint32_t*)pOutBuf)[i * 7 + 2] = ((const uint32_t*)pOutBuf)[i * 7 + 2] ^ pKeyBuf[2];
			((uint32_t*)pOutBuf)[i * 7 + 3] = ((const uint32_t*)pOutBuf)[i * 7 + 3] ^ pKeyBuf[3];
			((uint32_t*)pOutBuf)[i * 7 + 4] = ((const uint32_t*)pOutBuf)[i * 7 + 4] ^ pKeyBuf[4];
			((uint32_t*)pOutBuf)[i * 7 + 5] = ((const uint32_t*)pOutBuf)[i * 7 + 5] ^ pKeyBuf[5];
			((uint32_t*)pOutBuf)[i * 7 + 6] = ((const uint32_t*)pOutBuf)[i * 7 + 6] ^ pKeyBuf[6];
		}

		for (size_t i = nLoops * 4 * 7; i < nSize; i++)
			pOutBuf[i] = pInBuf[i] ^ pKey->key[(i + nOffset) % DXA_KEY_BYTES];
	}
	else {
		//At least one of pInBuf and pOutBuf is unaligned
		for (size_t i = 0; i < nSize; i++)
			pOutBuf[i] = pInBuf[i] ^ pKey->key[(i + nOffset) % DXA_KEY_BYTES];
	}
}

void DX_ReadFile(void *buffer, size_t nSize, FILE *stream, size_t nOffset, pDXAKey pKey) {
	size_t nCurPos;

	nCurPos = nOffset == -1 ? ftell(stream) : nOffset;
	fread(buffer, 1, nSize, stream);
	DecodeData(buffer, buffer, nSize, nCurPos % DXA_KEY_BYTES, pKey);
}

void DX_ReadFileRaw(void *buffer, size_t nSize, FILE *stream) {
	fread(buffer, 1, nSize, stream);
}

pDXDir DX_GetRootDirectory(pDXArchive pArc) {
	//No error detection
	return (pDXDir)pArc->Table.DirectoryTable;
}

pDXFileHead DX_DirToFile(pDXArchive pArc, pDXDir pDir) {
	//No error detection
	return (pDXFileHead)(pArc->Table.FileTable + pDir->DirectoryAddress);
}

pDXDir DX_FileToDir(pDXArchive pArc, pDXFileHead pFileHead) {
	//No error detection
	return (pDXDir)(pArc->Table.DirectoryTable + pFileHead->DataAddress);
}

pDXDir DX_GetParentDir(pDXArchive pArc, pDXDir pDir) {
	//No error detection
	if (pDir->ParentDirectoryAddress == 0xffffffffffffffff)
		return NULL;
	return (pDXDir)(pArc->Table.DirectoryTable + pDir->ParentDirectoryAddress);
}

pDXFileHead DX_GetDirFile(pDXArchive pArc, pDXDir pDir, size_t n) {
	//No error detection
	return &((pDXFileHead)(pArc->Table.FileTable + pDir->FileHeadAddress))[n];
}

const char* DX_GetOriginalFileName_Origin(uint8_t *pFileNameTable) {
	//No error detection
	return (char*)pFileNameTable + *((uint16_t*)&pFileNameTable[0]) * 4 + 4;
}

const char* DX_GetOriginalFileName(pDXArchive pArc, pDXFileHead pFileHead) {
	//No error detection
	if ((pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY) && DX_FileToDir(pArc, pFileHead) == DX_GetRootDirectory(pArc))
		return "/";
	return DX_GetOriginalFileName_Origin(pArc->Table.NameTable + pFileHead->NameAddress);
}

bool DX_GetPathFileHead_IsNameMatch(const char *pathIn, const char *fileName) {
	size_t nLen;
	nLen = strlen(fileName);
	if (strncmp(pathIn, fileName, nLen) != 0)
		return false;
	return isslash(pathIn[nLen]) || pathIn[nLen] == '\0';
}
const char* DX_GetPathFileHead_SkipSlash(const char *str) {
	while (isslash(*str))
		str++;
	return str;
}
const char* DX_GetPathFileHead_GetNextName(const char *path) {
	while (!isslash(*path) && *path != '\0')
		path++;
	return DX_GetPathFileHead_SkipSlash(path);
}
pDXFileHead DX_GetPathFileHead_Inner(pDXArchive pArc, pDXDir pCurDir, const char *path) {
	pDXFileHead pFileHead;
	pDXDir pDir;

	pDir = pCurDir;
	pFileHead = DX_DirToFile(pArc, pDir);

	for (; *path != '\0'; path = DX_GetPathFileHead_GetNextName(path)) {
		if (!(pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
			SetErrMsg("\"%s\" is a file", DX_GetOriginalFileName(pArc, pFileHead));
			return NULL;
		}

		pDir = DX_FileToDir(pArc, pFileHead);

		//Check whether folder is "." or ".."
		if (DX_GetPathFileHead_IsNameMatch(path, "."))
			continue;
		if (DX_GetPathFileHead_IsNameMatch(path, "..")) {
			pDir = DX_GetParentDir(pArc, pDir);
			if (pDir == NULL) {
				SetErrMsg("Unexpected root of file system is found");
				return NULL;
			}
			pFileHead = DX_DirToFile(pArc, pDir);
			continue;
		}

		//Search item in the folder
		for (size_t i = 0; i < pDir->FileHeadNum; i++) {
			pFileHead = DX_GetDirFile(pArc, pDir, i);
			if (DX_GetPathFileHead_IsNameMatch(path, DX_GetOriginalFileName(pArc, pFileHead)))
				goto NewCycle;
		}

		SetErrMsg("No such file or directory");
		return NULL;

NewCycle:;
	}

	if (isslash(path[-1]) && !(pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
		SetErrMsg("\"%s\" is a file", DX_GetOriginalFileName(pArc, pFileHead));
		return NULL;
	}

	return pFileHead;
}
pDXFileHead DX_GetPathFileHead(pDXArchive pArc, pDXDir pCurDir, const char *path) {
	if (pArc == NULL) {
		SetErrMsg("Archive pointer is invalid");
		return NULL;
	}

	if (pCurDir == NULL)
		pCurDir = DX_GetRootDirectory(pArc);
	if (path == NULL)
		path = "";
	if (*path == '\0')
		return DX_DirToFile(pArc, pCurDir);
	if (isslash(*path)) {
		path = DX_GetPathFileHead_SkipSlash(path);
		pCurDir = DX_GetRootDirectory(pArc);
	}

	return DX_GetPathFileHead_Inner(pArc, pCurDir, path);
}

void DX_CalcFileKey(const char *keyString, pDXArchive pArc, pDXFileHead pFileHead, pDXDir pDir, pDXAKey pKey) {
	char strFileKey[DXA_KEY_STRING_MAXLENGTH];
	size_t nCurPos;

	if (keyString == NULL) {
		strFileKey[0] = '\0';
		nCurPos = 0;
	}
	else {
		strcpy(strFileKey, keyString);
		nCurPos = strlen(strFileKey);
	}

	memset(&strFileKey[DXA_KEY_STRING_MAXLENGTH - 8], 0, 8);

	//strncat(&strFileKey[nCurPos], DX_GetOriginalFileName(pArc, pFileHead), DXA_KEY_STRING_MAXLENGTH - 8 - nCurPos);
	strncat(&strFileKey[nCurPos], (char*)(pArc->Table.NameTable + pFileHead->NameAddress + 4), DXA_KEY_STRING_MAXLENGTH - 8 - nCurPos);

	while (pDir->ParentDirectoryAddress != 0xffffffffffffffff) {
		strncat(
			&strFileKey[nCurPos],
			//DX_GetOriginalFileName(pArc, DX_DirToFile(pArc, pDir)),
			(char*)(pArc->Table.NameTable + ((pDXFileHead)(pArc->Table.FileTable + pDir->DirectoryAddress))->NameAddress + 4),
			DXA_KEY_STRING_MAXLENGTH - 8 - nCurPos
		);
		pDir = DX_GetParentDir(pArc, pDir);
	}

	DX_CreateKey(strFileKey, strlen(strFileKey), pKey);
}
























char strKeyGlobal[DXA_KEY_STRING_LENGTH + 1];
char strArcFileNameGlobal[1024];
DXAKey keyGlobal;
pDXAKey pKeyGlobal;
DXArchive arcGlobal;

#define STREAM_TYPE_STDFILE 0
#define STREAM_TYPE_STDFILE_STDIN 1
#define STREAM_TYPE_STDFILE_STDOUT 2
#define STREAM_TYPE_ARCFILE 3
#define STREAM_TYPE_AUTOMATED 4

#define STREAM_FLAG_READ 1
#define STREAM_FLAG_WRITE 2

#define STREAM_SEEK_SET 1
#define STREAM_SEEK_CUR 2
#define STREAM_SEEK_END 3

typedef struct tagStream Stream, *pStream;

bool Stream_SubStd_open(pStream pThis, const void *pSource, uint32_t nFlag);
void Stream_SubStd_close(pStream pThis);
bool Stream_SubStd_seek(pStream pThis, long nOffset, int nSeek);
size_t Stream_SubStd_tell(pStream pThis);
size_t Stream_SubStd_read(pStream pThis, void *pBuffer, size_t nSize);
size_t Stream_SubStd_write(pStream pThis, const void *pBuffer, size_t nSize);

bool Stream_SubStd_stdio_open(pStream pThis, const void *pSource, uint32_t nFlag);
void Stream_SubStd_stdio_close(pStream pThis);
bool Stream_SubStd_stdio_seek(pStream pThis, long nOffset, int nSeek);
size_t Stream_SubStd_stdio_tell(pStream pThis);
size_t Stream_SubStd_stdin_read(pStream pThis, void *pBuffer, size_t nSize);
size_t Stream_SubStd_stdin_write(pStream pThis, const void *pBuffer, size_t nSize);
size_t Stream_SubStd_stdout_read(pStream pThis, void *pBuffer, size_t nSize);
size_t Stream_SubStd_stdout_write(pStream pThis, const void *pBuffer, size_t nSize);

bool Stream_SubArc_open(pStream pThis, const void *pSource, uint32_t nFlag);
void Stream_SubArc_close(pStream pThis);
bool Stream_SubArc_seek(pStream pThis, long nOffset, int nSeek);
size_t Stream_SubArc_tell(pStream pThis);
size_t Stream_SubArc_read(pStream pThis, void *pBuffer, size_t nSize);
size_t Stream_SubArc_write(pStream pThis, const void *pBuffer, size_t nSize);

typedef struct tagStreamVirtualTable {
	bool (*open)(pStream pThis, const void *pSource, uint32_t nFlag);
	void (*close)(pStream pThis);
	bool (*seek)(pStream pThis, long nOffset, int nSeek);
	size_t (*tell)(pStream pThis);
	size_t (*read)(pStream pThis, void *pBuffer, size_t nSize);
	size_t (*write)(pStream pThis, const void *pBuffer, size_t nSize);
} StreamVirtualTable, *pStreamVirtualTable;

typedef struct tagStream {
	pStreamVirtualTable vt;
	void *pPrivate;
} Stream, *pStream;

typedef struct tagStream_SubArc_PrivateData {
	pDXFileHead pFileHead;
	void *pUncompressed;
	size_t nCurPos;
	DXAKey key;
} Stream_SubArc_PrivateData, *pStream_SubArc_PrivateData;

StreamVirtualTable svtDefault[] = {
	{
		Stream_SubStd_open,
		Stream_SubStd_close,
		Stream_SubStd_seek,
		Stream_SubStd_tell,
		Stream_SubStd_read,
		Stream_SubStd_write
	},
	{
		Stream_SubStd_stdio_open,
		Stream_SubStd_stdio_close,
		Stream_SubStd_stdio_seek,
		Stream_SubStd_stdio_tell,
		Stream_SubStd_stdin_read,
		Stream_SubStd_stdin_write
	},
	{
		Stream_SubStd_stdio_open,
		Stream_SubStd_stdio_close,
		Stream_SubStd_stdio_seek,
		Stream_SubStd_stdio_tell,
		Stream_SubStd_stdout_read,
		Stream_SubStd_stdout_write
	},
	{
		Stream_SubArc_open,
		Stream_SubArc_close,
		Stream_SubArc_seek,
		Stream_SubArc_tell,
		Stream_SubArc_read,
		Stream_SubArc_write
	}
};

void* Stream_GetPrivateData(pStream pStm) {
	if (pStm == NULL) {
		SetErrMsg("Stream pointer is invalid");
		return NULL;
	}
	return pStm->pPrivate;
}

bool Stream_SetPrivateData(pStream pStm, void *pData) {
	if (pStm == NULL)
		BoolErrReturn("Stream pointer is invalid");
	pStm->pPrivate = pData;
	return true;
}

pStream Stream_Open(const void *pSource, int nType, uint32_t nFlag) {
	pStream pStm;

	pStm = (pStream)malloc( sizeof(Stream) );
	if (pStm == NULL) {
		SetErrMsg("Memory allocation failure");
		return NULL;
	}

	switch (nType) {
	case STREAM_TYPE_STDFILE:
	case STREAM_TYPE_STDFILE_STDIN:
	case STREAM_TYPE_STDFILE_STDOUT:
	case STREAM_TYPE_ARCFILE:
		pStm->vt = &svtDefault[nType];
		break;
	case STREAM_TYPE_AUTOMATED:
		break;
	default:
		SetErrMsg("Unknown stream type");
		goto ERR;
	}

	if (nType == STREAM_TYPE_AUTOMATED) {
		// Prefer archive file
		pStm->vt = &svtDefault[STREAM_TYPE_ARCFILE];
		if ( !pStm->vt->open(pStm, pSource, nFlag) )
			pStm->vt = &svtDefault[STREAM_TYPE_STDFILE];
		else
			goto DONE;
	}

	if ( !pStm->vt->open(pStm, pSource, nFlag) )	// Error is set by vt function open()
		goto ERR;

DONE:
	return pStm;

ERR:
	free(pStm);
	return NULL;
}

bool Stream_Close(pStream pStm) {
	if (pStm == NULL)
		BoolErrReturn("Stream pointer is invalid");
	pStm->vt->close(pStm);
	free(pStm);
	return true;
}

bool Stream_Seek(pStream pStm, long nOffset, int nSeek) {
	if (pStm == NULL)
		BoolErrReturn("Stream pointer is invalid");
	return pStm->vt->seek(pStm, nOffset, nSeek);
}

size_t Stream_Tell(pStream pStm) {
	if (pStm == NULL) {
		SetErrMsg("Stream pointer is invalid");
		return -1;
	}
	return pStm->vt->tell(pStm);
}

size_t Stream_Read(pStream pStm, void *pBuffer, size_t nSize) {
	if (pStm == NULL) {
		SetErrMsg("Stream pointer is invalid");
		return 0;
	}
	return pStm->vt->read(pStm, pBuffer, nSize);
}

size_t Stream_Write(pStream pStm, const void *pBuffer, size_t nSize) {
	if (pStm == NULL) {
		SetErrMsg("Stream pointer is invalid");
		return 0;
	}
	return pStm->vt->write(pStm, pBuffer, nSize);
}

int Stream_Printf(pStream pStm, const char *fmt, ...) {
	char strBuf[2048];
	va_list vl;
	int nLen;

	va_start(vl, fmt);
	nLen = vsprintf(strBuf, fmt, vl);
	va_end(vl);

	return (int)Stream_Write(pStm, strBuf, nLen);
}

uint32_t Stream_HashCRC32(pStream pStm) {	//Copied from DX_HashCRC32()
	static bool bIsCRC32TableInited = false;
	static uint32_t CRC32Table[256];
	uint32_t nCRC;
	size_t nSize;
	size_t nPos;

	if (pStm == NULL)
		return 0;

	nCRC = 0xffffffff;

	if (!bIsCRC32TableInited) {
		uint32_t nMagic = 0xedb88320;

		for (uint32_t i = 0; i < 256; i++) {
			uint32_t nData = i;
			for (uint32_t j = 0; j < 8; j++) {
				int b = nData & 1;
				nData >>= 1;
				if (b != 0)
					nData ^= nMagic;
			}
			CRC32Table[i] = nData;
		}

		bIsCRC32TableInited = true;
	}

	//Start calculation
	nPos = pStm->vt->tell(pStm);
	if (nPos == -1)
		return 0;
	if (!pStm->vt->seek(pStm, 0, STREAM_SEEK_END))
		return 0;
	nSize = pStm->vt->tell(pStm);
	pStm->vt->seek(pStm, 0, STREAM_SEEK_SET);

	for (uint32_t i = 0; i < nSize / 2048; i++) {
		uint8_t arrBytes[2048];
		pStm->vt->read(pStm, arrBytes, 2048);
		for (int j = 0; j < 2048; j++)
			nCRC = CRC32Table[(uint8_t)(nCRC ^ arrBytes[j])] ^ (nCRC >> 8);
	}

	for (uint32_t i = nSize & ~2047; i < nSize; i++) {
		uint8_t nByte;
		pStm->vt->read(pStm, &nByte, 1);
		nCRC = CRC32Table[(uint8_t)(nCRC ^ nByte)] ^ (nCRC >> 8);
	}

	pStm->vt->seek(pStm, nPos, STREAM_SEEK_SET);

	return nCRC ^ 0xffffffff;
}

bool Stream_SubStd_open(pStream pThis, const void *pSource, uint32_t nFlag) {
	const char *strMode;
	FILE *fp;

	switch (nFlag) {
	case STREAM_FLAG_READ:
		strMode = "rb";
		break;
	case STREAM_FLAG_WRITE:
		strMode = "wb";
		break;
	case STREAM_FLAG_READ | STREAM_FLAG_WRITE:
		strMode = "rb+";
		break;
	default:
		BoolErrReturn("Unknown file mode");
	}

	fp = fopen((const char*)pSource, strMode);
	if (fp == NULL)
		BoolErrReturn("fopen: %s", strerror(errno));

	pThis->pPrivate = fp;

	return true;
}
void Stream_SubStd_close(pStream pThis) {
	fclose((FILE*)pThis->pPrivate);
}
bool Stream_SubStd_seek(pStream pThis, long nOffset, int nSeek) {
	int nStdSeek;
	switch (nSeek) {
	case STREAM_SEEK_SET:
		nStdSeek = SEEK_SET;
		break;
	case STREAM_SEEK_CUR:
		nStdSeek = SEEK_CUR;
		break;
	case STREAM_SEEK_END:
		nStdSeek = SEEK_END;
		break;
	default:
		BoolErrReturn("Unknown seek mode");
	}
	SetErrMsg("Common seek error");
	return fseek((FILE*)pThis->pPrivate, nOffset, nStdSeek) == 0;
}
size_t Stream_SubStd_tell(pStream pThis) {
	SetErrMsg("Common tell error");
	return ftell((FILE*)pThis->pPrivate);
}
size_t Stream_SubStd_read(pStream pThis, void *pBuffer, size_t nSize) {
	SetErrMsg("Common read error");
	return fread(pBuffer, 1, nSize, (FILE*)pThis->pPrivate);
}
size_t Stream_SubStd_write(pStream pThis, const void *pBuffer, size_t nSize) {
	SetErrMsg("Common write error");
	return fwrite(pBuffer, 1, nSize, (FILE*)pThis->pPrivate);
}

bool Stream_SubStd_stdio_open(pStream pThis, const void *pSource, uint32_t nFlag) {
	return true;
}
void Stream_SubStd_stdio_close(pStream pThis) {
	return;
}
bool Stream_SubStd_stdio_seek(pStream pThis, long nOffset, int nSeek) {
	BoolErrReturn("Standard stream does not support seeking");
}
size_t Stream_SubStd_stdio_tell(pStream pThis) {
	SetErrMsg("Standard stream does not support telling");
	return -1;
}
size_t Stream_SubStd_stdin_read(pStream pThis, void *pBuffer, size_t nSize) {
	uint8_t *pByteBuf;
	size_t i;
	int ch;

	if (nSize == 0)
		return 0;

	pByteBuf = (uint8_t*)pBuffer;

	i = 0;
	ch = getchar();
	while (ch != EOF) {
		pByteBuf[i++] = ch;
		if (i >= nSize)
			return nSize;
		ch = getchar();
	}

	SetErrMsg("Common read error");
	return i;
}
size_t Stream_SubStd_stdin_write(pStream pThis, const void *pBuffer, size_t nSize) {
	SetErrMsg("Standard in is read-only");
	return 0;
}
size_t Stream_SubStd_stdout_read(pStream pThis, void *pBuffer, size_t nSize) {
	SetErrMsg("Standard out is write-only");
	return 0;
}
size_t Stream_SubStd_stdout_write(pStream pThis, const void *pBuffer, size_t nSize) {
	SetErrMsg("Common write error");
	return printf("%.*s", nSize, (const char*)pBuffer);
}

bool Stream_SubArc_open(pStream pThis, const void *pSource, uint32_t nFlag) {
	pStream_SubArc_PrivateData pPrivate;
	void *pHuffBuffer, *pLzBuffer;
	size_t nHuffSize, nLzSize;
	size_t nDataSize;
	char *strTemp;
	pDXAKey pKey;
	FILE *fpArc;
	pDXDir pDir;

	if (nFlag != STREAM_FLAG_READ)
		BoolErrReturn("Archive stream is read only");

	pPrivate = (pStream_SubArc_PrivateData)malloc(sizeof(Stream_SubArc_PrivateData));
	if (pPrivate == NULL)
		BoolErrReturn("Memory allocation failure");

	pThis->pPrivate = pPrivate;

	//Get file and directory pointers
	strTemp = strdup((const char*)pSource);
	if (strTemp == NULL) {
		free(pPrivate);
		BoolErrReturn("Memory allocation failure");
	}

	pPrivate->pFileHead = DX_GetPathFileHead(&arcGlobal, NULL, strTemp);
	if (pPrivate->pFileHead == NULL) {
		free(strTemp);
		free(pPrivate);
		return false;
	}

	//There must be at least one slash in the string
	(strrchr(strTemp, '/') ? : strrchr(strTemp, '\\'))[0] = '\0';
	pDir = DX_FileToDir(&arcGlobal, DX_GetPathFileHead(&arcGlobal, NULL, strTemp));

	if (pPrivate->pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY) {
		SetErrMsg("\"%s\" is a directory", DX_GetOriginalFileName(&arcGlobal, pPrivate->pFileHead));
		free(strTemp);
		free(pPrivate);
		return false;
	}

	free(strTemp);

	pPrivate->nCurPos = 0;
	pPrivate->pUncompressed = NULL;

	if (pPrivate->pFileHead->DataSize == 0)
		return true;

	pPrivate->pUncompressed = malloc(pPrivate->pFileHead->DataSize);
	if (pPrivate->pUncompressed == NULL) {
		free(pPrivate);
		BoolErrReturn("Memory allocation failure");
	}

	//Generate key
	if (pKeyGlobal != NULL) {
		DX_CalcFileKey(strKeyGlobal, &arcGlobal, pPrivate->pFileHead, pDir, &pPrivate->key);
		pKey = &pPrivate->key;
	}
	else {
		pKey = NULL;
	}

	//Detect file compression type
	fpArc = arcGlobal.fpArchive;
	fseek(fpArc, arcGlobal.Head.DataStartAddress + pPrivate->pFileHead->DataAddress, SEEK_SET);

	nDataSize = pPrivate->pFileHead->DataSize;
	nLzSize = pPrivate->pFileHead->PressDataSize;
	nHuffSize = pPrivate->pFileHead->HuffPressDataSize;
	switch ((pPrivate->pFileHead->PressDataSize != 0xffffffffffffffff) | ((pPrivate->pFileHead->HuffPressDataSize != 0xffffffffffffffff) << 1)) {
	case 0:	//Dxa N, Huff N
		DX_ReadFile(pPrivate->pUncompressed, nDataSize, fpArc, nDataSize, pKey);
		break;
	case 1:	//Dxa Y, Huff N
		pLzBuffer = malloc(nLzSize);
		if (pLzBuffer == NULL) {
			SetErrMsg("Memory allocation failure");
			goto ERR;
		}

		DX_ReadFile(pLzBuffer, nLzSize, fpArc, nDataSize, pKey);
		if (DXA_Decode(pLzBuffer, NULL) != nDataSize) {
			free(pLzBuffer);
			SetErrMsg("Data verification failed. Please check whether the archive is corrupted or the key is wrong.");
			goto ERR;
		}

		DXA_Decode(pLzBuffer, pPrivate->pUncompressed);

		free(pLzBuffer);
		break;
	case 2:	//Dxa N, Huff Y
		pHuffBuffer = malloc(nHuffSize);
		if (pHuffBuffer == NULL) {
			SetErrMsg("Memory allocation failure");
			goto ERR;
		}

		DX_ReadFile(pHuffBuffer, nHuffSize, fpArc, nDataSize, pKey);
		/*	How to verify the data here?
		if (Huffman_Decode(pHuffBuffer, NULL) != nDataSize) {
			free(pHuffBuffer);
			SetErrMsg("Data verification failed. Please check whether the archive is corrupted or the key is wrong.");
			goto ERR;
		}
		*/

		Huffman_Decode(pHuffBuffer, pPrivate->pUncompressed);
		if (arcGlobal.Head.HuffmanEncodeKB != 0xff && nDataSize > arcGlobal.Head.HuffmanEncodeKB * 1024 * 2) {
			memmove(
				&((uint8_t*)pPrivate->pUncompressed)[nDataSize - arcGlobal.Head.HuffmanEncodeKB * 1024],
				&((uint8_t*)pPrivate->pUncompressed)[arcGlobal.Head.HuffmanEncodeKB * 1024],
				arcGlobal.Head.HuffmanEncodeKB * 1024
			);
			DX_ReadFile(
				&((uint8_t*)pPrivate->pUncompressed)[arcGlobal.Head.HuffmanEncodeKB * 1024],
				nDataSize - arcGlobal.Head.HuffmanEncodeKB * 1024 * 2,
				fpArc,
				nDataSize + nHuffSize,
				pKey
			);
		}

		free(pHuffBuffer);
		break;
	case 3:	//Dxa Y, Huff Y
		pLzBuffer = malloc(nLzSize);
		pHuffBuffer = malloc(nHuffSize);
		if (pLzBuffer == NULL || pHuffBuffer == NULL) {
			free(pLzBuffer);
			free(pHuffBuffer);
			SetErrMsg("Memory allocation failure");
			goto ERR;
		}

		DX_ReadFile(pHuffBuffer, nHuffSize, fpArc, nDataSize, pKey);
		/*	How to verify the data here?
		if (Huffman_Decode(pHuffBuffer, NULL) != nLzSize) {
			free(pHuffBuffer);
			SetErrMsg("Data verification failed. Please check whether the archive is corrupted or the key is wrong.");
			goto ERR;
		}
		*/

		Huffman_Decode(pHuffBuffer, pLzBuffer);
		if (arcGlobal.Head.HuffmanEncodeKB != 0xff && nLzSize > arcGlobal.Head.HuffmanEncodeKB * 1024 * 2) {
			memmove(
				&((uint8_t*)pLzBuffer)[nLzSize - arcGlobal.Head.HuffmanEncodeKB * 1024],
				&((uint8_t*)pLzBuffer)[arcGlobal.Head.HuffmanEncodeKB * 1024],
				arcGlobal.Head.HuffmanEncodeKB * 1024
			);
			DX_ReadFile(
				&((uint8_t*)pLzBuffer)[arcGlobal.Head.HuffmanEncodeKB * 1024],
				nLzSize - arcGlobal.Head.HuffmanEncodeKB * 1024 * 2,
				fpArc,
				nDataSize + nHuffSize,
				pKey
			);
		}

		if (DXA_Decode(pLzBuffer, NULL) != nDataSize) {
			free(pLzBuffer);
			free(pHuffBuffer);
			SetErrMsg("Data verification failed. Please check whether the archive is corrupted or the key is wrong.");
			goto ERR;
		}

		DXA_Decode(pLzBuffer, pPrivate->pUncompressed);

		free(pLzBuffer);
		free(pHuffBuffer);
		break;
	}

	return true;

ERR:
	free(pPrivate->pUncompressed);
	free(pPrivate);
	return false;
}
void Stream_SubArc_close(pStream pThis) {
	pStream_SubArc_PrivateData pPrivate;

	pPrivate = (pStream_SubArc_PrivateData)pThis->pPrivate;
	free(pPrivate->pUncompressed);
	free(pPrivate);
}
bool Stream_SubArc_seek(pStream pThis, long nOffset, int nSeek) {
	pStream_SubArc_PrivateData pPrivate;
	size_t nFinalPos;

	pPrivate = (pStream_SubArc_PrivateData)pThis->pPrivate;
	switch (nSeek) {
	case STREAM_SEEK_SET:
		nFinalPos = 0;
		break;
	case STREAM_SEEK_CUR:
		nFinalPos = pPrivate->nCurPos;
		break;
	case STREAM_SEEK_END:
		nFinalPos = pPrivate->pFileHead->DataSize;
		break;
	default:
		BoolErrReturn("Unknown seek mode");
	}

	nFinalPos += nOffset;
	if (nFinalPos < 0 || nFinalPos > pPrivate->pFileHead->DataSize)
		BoolErrReturn("File seek out of range");

	pPrivate->nCurPos = nFinalPos;

	return true;
}
size_t Stream_SubArc_tell(pStream pThis) {
	return ((pStream_SubArc_PrivateData)pThis->pPrivate)->nCurPos;
}
size_t Stream_SubArc_read(pStream pThis, void *pBuffer, size_t nSize) {
	pStream_SubArc_PrivateData pPrivate;
	size_t nCopySize;

	SetErrMsg("Encountered end of stream");

	pPrivate = (pStream_SubArc_PrivateData)pThis->pPrivate;

	nCopySize = min(nSize, pPrivate->pFileHead->DataSize - pPrivate->nCurPos);
	memcpy(pBuffer, &((uint8_t*)pPrivate->pUncompressed)[pPrivate->nCurPos], nCopySize);
	pPrivate->nCurPos += nCopySize;

	return nCopySize;
}
size_t Stream_SubArc_write(pStream pThis, const void *pBuffer, size_t nSize) {
	SetErrMsg("Archive stream is read only");
	return 0;
}
















bool DX_OpenArchive_Inner_SpecialCheckOldHeader(uint8_t byteHdr[4]) {
	static const uint8_t byteHdrOld[4] = { 0xfa, 0x1b, 0xb9, 0x5a };
	return memcmp(byteHdr, byteHdrOld, sizeof(byteHdrOld)) == 0;
}
bool DX_OpenArchive_Inner(pDXArchive pArc) {
	//Assuming pArc and the fp inside are valid
	void *pHuffHeadBuffer, *pLzHeadBuffer;
	size_t nHuffHeadSize, nLzHeadSize;
	size_t nFileSize;
	DXAKey *pKey;
	FILE *fp;

	//Preparation
	fp = pArc->fpArchive;
	rewind(fp);

	//Read header
	DX_ReadFileRaw(&pArc->Head, DXARC_ID_AND_VERSION_SIZE, fp);
	if (DX_OpenArchive_Inner_SpecialCheckOldHeader((uint8_t*)&pArc->Head.Head))
		BoolErrReturn("Older version of Rabi-Ribi archive is unsupported");
	if (!CheckDXHeader(&pArc->Head.Head))
		BoolErrReturn("Archive identifier is wrong");

	DX_ReadFileRaw((uint8_t*)&pArc->Head + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER8_SIZE - DXARC_ID_AND_VERSION_SIZE, fp);
	pArc->NoKey = pArc->Head.Flags & DXA_FLAG_NO_KEY;
	pKeyGlobal = pArc->NoKey ? NULL : &keyGlobal;

	if (pArc->Head.Version != DXAVER)
		BoolErrReturn("Archive version is wrong");

	pKey = pKeyGlobal;

	//Read file system
	pArc->Table.Top = malloc(pArc->Head.HeadSize);
	if (pArc->Table.Top == NULL)
		BoolErrReturn("Memory allocation failure");

	if (pArc->Head.Flags & DXA_FLAG_NO_HEAD_PRESS) {
		fseek(fp, (long)pArc->Head.FileNameTableStartAddress, SEEK_SET);
		DX_ReadFile(&pArc->Head, pArc->Head.HeadSize, fp, 0, pKey);
	}
	else {
		fseek(fp, 0, SEEK_END);
		nFileSize = ftell(fp);
		fseek(fp, (long)pArc->Head.FileNameTableStartAddress, SEEK_SET);
		nHuffHeadSize = (unsigned long long)(nFileSize - ftell(fp));

		pHuffHeadBuffer = malloc(nHuffHeadSize);
		if (pHuffHeadBuffer == NULL) {
			free(pArc->Table.Top);
			BoolErrReturn("Memory allocation failure");
		}

		DX_ReadFile(pHuffHeadBuffer, nHuffHeadSize, fp, 0, pKey);
		nLzHeadSize = (size_t)Huffman_Decode(pHuffHeadBuffer, NULL);

		if (nLzHeadSize > nHuffHeadSize * 4 || nLzHeadSize * 4 < nHuffHeadSize) {
			free(pHuffHeadBuffer);
			free(pArc->Table.Top);
			BoolErrReturn("Archive data verification failed. Please check whether the archive is corrupted or the key is wrong.");
		}

		pLzHeadBuffer = malloc(nLzHeadSize);
		if (pLzHeadBuffer == NULL) {
			free(pHuffHeadBuffer);
			free(pArc->Table.Top);
			BoolErrReturn("Memory allocation failure");
		}

		Huffman_Decode(pHuffHeadBuffer, pLzHeadBuffer);
		free(pHuffHeadBuffer);

		if (DXA_Decode(pLzHeadBuffer, NULL) != pArc->Head.HeadSize) {
			free(pLzHeadBuffer);
			free(pArc->Table.Top);
			BoolErrReturn("Archive data verification failed. Please check whether the archive is corrupted or the key is wrong.");
		}

		DXA_Decode(pLzHeadBuffer, pArc->Table.Top);
		free(pLzHeadBuffer);
	}

	pArc->Table.NameTable = pArc->Table.Top;
	pArc->Table.FileTable = pArc->Table.NameTable + pArc->Head.FileTableStartAddress;
	pArc->Table.DirectoryTable = pArc->Table.NameTable + pArc->Head.DirectoryTableStartAddress;

	return true;
}
bool DX_OpenArchive(pDXArchive pArc, const char *path) {
	char *strPathDup;
	size_t nLen;

	if (pArc == NULL)
		BoolErrReturn("Archive pointer is invalid");

	strPathDup = strdup(path);
	if (strPathDup == NULL)
		BoolErrReturn("Memory allocation failure");

	nLen = strlen(strPathDup);
	if (strPathDup[0] == '"' && strPathDup[nLen - 1] == '"') {
		strPathDup[nLen - 1] = '\0';
		memmove(strPathDup, &strPathDup[1], nLen - 1);
		nLen -= 2;
	}

	pArc->fpArchive = fopen(strPathDup, "rb");
	free(strPathDup);

	if (pArc->fpArchive == NULL)
		BoolErrReturn("fopen: %s", strerror(errno));

	if (!DX_OpenArchive_Inner(pArc)) {
		fclose(pArc->fpArchive);
		return false;
	}

	return true;
}

bool DX_CloseArchive(pDXArchive pArc) {
	if (pArc == NULL)
		BoolErrReturn("Archive pointer is invalid");
	free(pArc->Table.Top);
	fclose(pArc->fpArchive);
	return true;
}

void CalculateFileSystemInfo_Inner(pDXDir pDir, int *pnDirCount, int *pnFileCount) {
	//Assuming pDir, pnDirCount and pnFileCount are valid
	pDXFileHead pFileHead;

	for (size_t i = 0; i < pDir->FileHeadNum; i++) {
		pFileHead = DX_GetDirFile(&arcGlobal, pDir, i);
		if (pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY) {
			(*pnDirCount)++;
			CalculateFileSystemInfo_Inner(DX_FileToDir(&arcGlobal, pFileHead), pnDirCount, pnFileCount);
		}
		else {
			(*pnFileCount)++;
		}
	}
}
void CalculateFileSystemInfo(void) {
	int nDirCount = 0, nFileCount = 0;
	CalculateFileSystemInfo_Inner(DX_GetRootDirectory(&arcGlobal), &nDirCount, &nFileCount);
	printf(
		"There are %d folder%s and %d file%s in the archive file.\n",
		nDirCount,
		nDirCount == 1 ? "" : "s",
		nFileCount,
		nFileCount == 1 ? "" : "s"
	);
}














#define CON_CHARTOKEN_TEXT 1
#define CON_CHARTOKEN_SPACE 2
#define CON_CHARTOKEN_SYMBOL 3

typedef struct tagConsoleEnvironment {
	pStream stmIn, stmOut;
	char strWorkDir[1024];
	bool bFirstAC;
	char strACBuf[4096];	//0-3 bytes: Item count; 4-~ bytes: Auto completion items
	char strHistory[8192];	//0-3 bytes: Item count; 4-7 bytes: Current item position; 8-~ bytes: history items
	//NOTE: Since Windows has had support for command history, the history array here will not be used
	bool bACState[4096 / 2];
	char chACAutoChar;
} ConsoleEnvironment, *pConsoleEnvironment;

typedef struct tagCmdInfo {
	const char *cmd;
	bool (*pfMain)(int argc, char *argv[], pConsoleEnvironment pConEnv);
	void (*pfAC)(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv);
} CmdInfo, *pCmdInfo;

void ConAC_Default_ArcPath(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv);
void ConAC_Default_CmdName(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv);

void CmdAC_preview(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv);
void CmdAC_tree(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv);
void CmdAC_extract(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv);

bool Cmd_ls(int argc, char *argv[], pConsoleEnvironment pConEnv);
bool Cmd_cd(int argc, char *argv[], pConsoleEnvironment pConEnv);
bool Cmd_pwd(int argc, char *argv[], pConsoleEnvironment pConEnv);
bool Cmd_help(int argc, char *argv[], pConsoleEnvironment pConEnv);
bool Cmd_clear(int argc, char *argv[], pConsoleEnvironment pConEnv);
bool Cmd_preview(int argc, char *argv[], pConsoleEnvironment pConEnv);
bool Cmd_tree(int argc, char *argv[], pConsoleEnvironment pConEnv);
bool Cmd_extract(int argc, char *argv[], pConsoleEnvironment pConEnv);

CmdInfo cmds[] = {
	{ "ls", Cmd_ls, ConAC_Default_ArcPath },
	{ "cd", Cmd_cd, ConAC_Default_ArcPath },
	{ "pwd", Cmd_pwd },
	{ "help", Cmd_help },
	{ "cls", Cmd_clear },
	{ "clear", Cmd_clear },
	{ "preview", Cmd_preview, CmdAC_preview },
	{ "tree", Cmd_tree, CmdAC_tree },
	{ "extract", Cmd_extract, CmdAC_extract }
};

uint32_t ConAC_GetItemCount(pConsoleEnvironment pConEnv) {
	uint8_t *pInt;
	if (pConEnv == NULL)
		return -1;
	pInt = (uint8_t*)pConEnv->strACBuf;
	return pInt[0] | (pInt[1] << 8) | (pInt[2] << 16) | (pInt[3] << 24);
}
void ConAC_SetItemCount(pConsoleEnvironment pConEnv, uint32_t n) {
	uint8_t *pInt;
	if (pConEnv == NULL)
		return;
	pInt = (uint8_t*)pConEnv->strACBuf;
	pInt[0] = n & 255;
	pInt[1] = (n >> 8) & 255;
	pInt[2] = (n >> 16) & 255;
	pInt[3] = (n >> 24) & 255;
}
char* ConAC_GetItemStartAddress(pConsoleEnvironment pConEnv) {
	return pConEnv == NULL ? NULL : &pConEnv->strACBuf[4];
}
char* ConAC_GetItem(pConsoleEnvironment pConEnv, uint32_t n) {
	char *strItemCur;
	uint32_t nCount;
	nCount = ConAC_GetItemCount(pConEnv);
	if (n >= nCount)
		return NULL;
	strItemCur = ConAC_GetItemStartAddress(pConEnv);
	for (uint32_t i = 0; i < n; i++)
		strItemCur = &strItemCur[strlen(strItemCur) + 1];
	return strItemCur;
}
void ConAC_AddCandidate(const char *str, pConsoleEnvironment pConEnv) {
	uint32_t nItemCount;
	nItemCount = ConAC_GetItemCount(pConEnv);
	ConAC_SetItemCount(pConEnv, nItemCount + 1);
	strcpy(ConAC_GetItem(pConEnv, nItemCount), str);
}

uint32_t Con_GetHistoryCount(pConsoleEnvironment pConEnv) {
	uint8_t *pInt;
	if (pConEnv == NULL)
		return -1;
	pInt = (uint8_t*)pConEnv->strHistory;
	return pInt[0] | (pInt[1] << 8) | (pInt[2] << 16) | (pInt[3] << 24);
}
void Con_SetHistoryCount(pConsoleEnvironment pConEnv, uint32_t n) {
	uint8_t *pInt;
	if (pConEnv == NULL)
		return;
	pInt = (uint8_t*)pConEnv->strHistory;
	pInt[0] = n & 255;
	pInt[1] = (n >> 8) & 255;
	pInt[2] = (n >> 16) & 255;
	pInt[3] = (n >> 24) & 255;
}
uint32_t Con_GetCurrentHistoryPosition(pConsoleEnvironment pConEnv) {
	uint8_t *pInt;
	if (pConEnv == NULL)
		return -1;
	pInt = &((uint8_t*)pConEnv->strHistory)[4];
	return pInt[0] | (pInt[1] << 8) | (pInt[2] << 16) | (pInt[3] << 24);
}
void Con_SetCurrentHistoryPosition(pConsoleEnvironment pConEnv, uint32_t n) {
	uint8_t *pInt;
	if (pConEnv == NULL)
		return;
	pInt = &((uint8_t*)pConEnv->strHistory)[4];
	pInt[0] = n & 255;
	pInt[1] = (n >> 8) & 255;
	pInt[2] = (n >> 16) & 255;
	pInt[3] = (n >> 24) & 255;
}
char* Con_GetHistoryStartAddress(pConsoleEnvironment pConEnv) {
	return pConEnv == NULL ? NULL : &pConEnv->strHistory[8];
}
char* Con_GetHistory(pConsoleEnvironment pConEnv, uint32_t n) {
	char *strItemCur;
	uint32_t nCount;
	nCount = Con_GetHistoryCount(pConEnv);
	if (n >= nCount)
		return NULL;
	strItemCur = Con_GetHistoryStartAddress(pConEnv);
	for (uint32_t i = 0; i < n; i++)
		strItemCur = &strItemCur[strlen(strItemCur) + 1];
	return strItemCur;
}
void Con_AddHistory(const char *str, pConsoleEnvironment pConEnv) {
	char *strItemAdd, *strItemCur;
	uint32_t nSizeRequire;
	uint32_t nSizeWrite;
	uint32_t nItemCount;

	nItemCount = ConAC_GetItemCount(pConEnv);
	ConAC_SetItemCount(pConEnv, nItemCount + 1);

	nSizeWrite = (uint32_t)strlen(str) + 1;

	strItemAdd = ConAC_GetItem(pConEnv, nItemCount);
	if (nSizeWrite + (strItemAdd - pConEnv->strHistory) > sizeof(pConEnv->strHistory)) {
		nSizeRequire = nSizeWrite + (strItemAdd - pConEnv->strHistory) - sizeof(pConEnv->strHistory);

		strItemCur = Con_GetHistoryStartAddress(pConEnv);
		do {
			strItemCur = &strItemCur[strlen(strItemCur) + 1];
			nItemCount--;
		} while (strItemCur - Con_GetHistoryStartAddress(pConEnv) < nSizeRequire);

		memmove(Con_GetHistoryStartAddress(pConEnv), strItemCur, strItemAdd - strItemCur);
		ConAC_SetItemCount(pConEnv, nItemCount);

		strItemAdd -= strItemCur - Con_GetHistoryStartAddress(pConEnv);
	}

	strcpy(strItemAdd, str);
}

size_t Con_FindCommand(const char *strCmdName) {
	for (size_t i = 0; i < GetArrLen(cmds); i++) {
		if (strcmp(strCmdName, cmds[i].cmd) == 0)
			return i;
	}
	return -1;
}

void InputKey(void) {
	bool bEchoState;
	int nRet;

	printf("Before using this program, please type game archive key to proceed(leave none for default key):\n");

	bEchoState = EnableEcho(false);
	nRet = scanf("%[^\n]%*c", strKeyGlobal);
	EnableEcho(bEchoState);
	putchar('\n');

	strKeyGlobal[DXA_KEY_STRING_LENGTH] = '\0';
	if (nRet != 1 || strKeyGlobal[0] == '\0')
		strcpy(strKeyGlobal, DefaultKeyString);
	if (nRet != 1)
		getchar();

	DX_CalcKey(strKeyGlobal, &keyGlobal);
}

void CombinePath(const char *strInBase, const char *strInAdd, char *strOut) {
	//NOTE: both strInBase and strInAdd must be standard(such as "/", "/gfx/1.png"), but strInAdd can be relative
	size_t nLen1, nLen2;

	nLen1 = strlen(strInAdd);
	while ((strInAdd[nLen1 - 1] == '/' || strInAdd[nLen1 - 1] == '\\') && nLen1 > 0)
		nLen1--;

	if (nLen1 == 0) {
		strcpy(strOut, "/");
		return;
	}

	if (strInAdd[0] == '/' || strInAdd[0] == '\\') {
		strcpy(strOut, strInAdd);
	}
	else {
		strcpy(strOut, strInBase);
		if (strcmp(strOut, "/") != 0)
			strcat(strOut, "/");
		nLen2 = strlen(strOut);
		strncat(strOut, strInAdd, nLen1);
		strOut[nLen1 + nLen2] = '\0';
	}
}

void GetDxDirPath(pDXArchive pArc, pDXDir pDir, char *strOut) {
	pDXFileHead pFileHead;
	const char *filename;
	size_t nLen;

	pFileHead = DX_DirToFile(pArc, pDir);
	if (pDir == DX_GetRootDirectory(pArc)) {
		strcpy(strOut, "/");
	}
	else {
		strOut[0] = '\0';
		while (pDir != DX_GetRootDirectory(pArc)) {
			filename = DX_GetOriginalFileName(pArc, pFileHead);
			nLen = strlen(filename);
			memmove(&strOut[nLen + 1], strOut, strlen(strOut) + 1);
			strOut[0] = '/';
			memcpy(&strOut[1], filename, nLen);

			pDir = DX_GetParentDir(pArc, pDir);
			pFileHead = DX_DirToFile(pArc, pDir);
		}
	}
}

void ConAC_Default_ArcPath(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv) {
	pDXFileHead pFileHead;
	char *strTemp;
	pDXDir pDir;
	size_t nLen;

	strTemp = strrchr(argv[nArgPos], '/') ? : strrchr(argv[nArgPos], '\\');
	if (strTemp == NULL) {
		pFileHead = DX_GetPathFileHead(&arcGlobal, NULL, pConEnv->strWorkDir);
		pDir = DX_FileToDir(&arcGlobal, pFileHead);

		for (size_t i = 0; i < pDir->FileHeadNum; i++) {
			pFileHead = DX_GetDirFile(&arcGlobal, pDir, i);
			ConAC_AddCandidate(DX_GetOriginalFileName(&arcGlobal, pFileHead), pConEnv);
		}
	}
	else {
		strTemp[0] = '\0';

		strTemp = (char*)malloc(1024);
		if (strTemp == NULL)
			return;

		CombinePath(pConEnv->strWorkDir, argv[nArgPos], strTemp);
		pFileHead = DX_GetPathFileHead(&arcGlobal, NULL, strTemp);
		if (pFileHead == NULL) {
			free(strTemp);
			return;
		}
		if (!(pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
			free(strTemp);
			return;
		}
		pDir = DX_FileToDir(&arcGlobal, pFileHead);

		strcpy(strTemp, argv[nArgPos]);
		strcat(strTemp, "/");
		nLen = strlen(strTemp);

		for (size_t i = 0; i < pDir->FileHeadNum; i++) {
			pFileHead = DX_GetDirFile(&arcGlobal, pDir, i);
			strcpy(&strTemp[nLen], DX_GetOriginalFileName(&arcGlobal, pFileHead));
			ConAC_AddCandidate(strTemp, pConEnv);
		}

		free(strTemp);
	}
}
void ConAC_Default_CmdName(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv) {
	pConEnv->chACAutoChar = ' ';
	for (size_t i = 0; i < GetArrLen(cmds); i++)
		ConAC_AddCandidate(cmds[i].cmd, pConEnv);
	ConAC_AddCandidate("exit", pConEnv);
}

bool Cmd_ls(int argc, char *argv[], pConsoleEnvironment pConEnv) {
	int nDirCount, nFileCount;
	char strFinalPath[1024];
	pDXFileHead pFileHead;
	pDXDir pCurDir;
	SYSTEMTIME st;

	if (argc == 1) {
		strcpy(strFinalPath, pConEnv->strWorkDir);
	}
	else if (argc == 2) {
		CombinePath(pConEnv->strWorkDir, argv[1], strFinalPath);
	}
	else {
		BoolErrReturn("Command does not take %d parameter%s", argc - 1, argc == 2 ? "" : "s");
	}

	//Check directory
	pFileHead = DX_GetPathFileHead(&arcGlobal, NULL, strFinalPath);
	//pCurDir = DX_FileToDir(&arcGlobal, DX_GetPathFileHead(&arcGlobal, NULL, pConEnv->strWorkDir));
	//pFileHead = DX_GetPathFileHead(&arcGlobal, pCurDir, argv[1]);
	if (pFileHead == NULL)
		return false;
	if (!(pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY))
		BoolErrReturn("\"%s\" is a file", DX_GetOriginalFileName(&arcGlobal, pFileHead));
	pCurDir = DX_FileToDir(&arcGlobal, pFileHead);

	//Start listing
	nDirCount = nFileCount = 0;

	GetDxDirPath(&arcGlobal, pCurDir, strFinalPath);
	Stream_Printf(pConEnv->stmOut, "Directory of %s\n", strFinalPath);
	for (size_t i = 0; i < pCurDir->FileHeadNum; i++) {
		pFileHead = DX_GetDirFile(&arcGlobal, pCurDir, i);

		FileTimeToSystemTime((FILETIME*)&pFileHead->Time.LastWrite, &st);
		Stream_Printf(
			pConEnv->stmOut,
			"%04u/%02u/%02u  %02u:%02u    ",
			(unsigned)st.wYear,
			(unsigned)st.wMonth,
			(unsigned)st.wDay,
			(unsigned)st.wHour,
			(unsigned)st.wMinute
		);

		if (pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY) {
			Stream_Printf(pConEnv->stmOut, "<DIR>\t");
			nDirCount++;
		}
		else {
			Stream_Printf(pConEnv->stmOut, "\t\t");
			nFileCount++;
		}

		Stream_Printf(pConEnv->stmOut, "%s\n", DX_GetOriginalFileName(&arcGlobal, pFileHead));
	}

	Stream_Printf(
		pConEnv->stmOut,
		"Summary: %d file%s, %d folder%s\n",
		nFileCount, nFileCount == 1 ? "" : "s",
		nDirCount, nDirCount == 1 ? "" : "s"
	);

	return true;
}

bool Cmd_cd(int argc, char *argv[], pConsoleEnvironment pConEnv) {
	char strFinalPath[1024];
	pDXFileHead pFileHead;

	if (argc == 1)
		return Cmd_pwd(1, (char*[]) { "pwd" }, pConEnv);
	if (argc != 2)
		BoolErrReturn("Command does not take %d parameter%s", argc - 1, argc == 2 ? "" : "s");

	//Process path
	CombinePath(pConEnv->strWorkDir, argv[1], strFinalPath);
	pFileHead = DX_GetPathFileHead(&arcGlobal, NULL, strFinalPath);
	if (pFileHead == NULL)
		return false;

	if (!(pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY))
		BoolErrReturn("\"%s\" is a file", DX_GetOriginalFileName(&arcGlobal, pFileHead));

	//Set path
	GetDxDirPath(&arcGlobal, DX_FileToDir(&arcGlobal, pFileHead), pConEnv->strWorkDir);

	return true;
}

bool Cmd_pwd(int argc, char *argv[], pConsoleEnvironment pConEnv) {
	if (argc != 1)
		BoolErrReturn("Command does not take %d parameter%s", argc - 1, argc == 2 ? "" : "s");
	Stream_Printf(pConEnv->stmOut, "%s\n", pConEnv->strWorkDir);
	return true;
}

bool Cmd_help(int argc, char *argv[], pConsoleEnvironment pConEnv) {
	Stream_Printf(pConEnv->stmOut, "Registered commands list:\n");
	Stream_Printf(pConEnv->stmOut, "%s", cmds[0].cmd);
	for (size_t i = 1; i < GetArrLen(cmds); i++)
		Stream_Printf(pConEnv->stmOut, ", %s", cmds[i].cmd);
	Stream_Printf(pConEnv->stmOut, "\n");
	return true;
}

bool Cmd_clear(int argc, char *argv[], pConsoleEnvironment pConEnv) {
	system("cls");
	return true;
}

void CmdAC_preview(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv) {
	if (argv[nArgPos][0] == '-') {
		pConEnv->chACAutoChar = ' ';
		ConAC_AddCandidate("--force-write", pConEnv);	//Force program to write to disk instead of using buffered data
		return;
	}

	ConAC_Default_ArcPath(argc, argv, nArgPos, pConEnv);
}
bool Cmd_preview(int argc, char *argv[], pConsoleEnvironment pConEnv) {
	char strFinalPath[1024];
	pStream stmArc, stmOut;
	uint8_t buf[1024];
	bool bForceWrite;
	char *strPathIn;
	size_t nSize;
	size_t nRead;

	if (argc <= 1)
		BoolErrReturn("Command does not take %d parameter%s", argc - 1, argc == 2 ? "" : "s");

	bForceWrite = false;
	strPathIn = NULL;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(&argv[i][1], "-force-write") == 0)
				bForceWrite = true;
			else
				BoolErrReturn("Unrecognized option \"%s\"", argv[i]);
		}
		else {
			if (strPathIn != NULL)
				BoolErrReturn("More than one path is specified");
			strPathIn = argv[i];
		}
	}

	if (strPathIn == NULL)
		BoolErrReturn("No path is specified");

	//Open streams
	CombinePath(pConEnv->strWorkDir, strPathIn, strFinalPath);
	stmArc = Stream_Open(strFinalPath, STREAM_TYPE_ARCFILE, STREAM_FLAG_READ);
	if (stmArc == NULL)
		return false;

	Stream_Seek(stmArc, 0, STREAM_SEEK_END);
	nSize = Stream_Tell(stmArc);
	Stream_Seek(stmArc, 0, STREAM_SEEK_SET);

	GenerateTempFilePath(strFinalPath);
	sprintf(
		(strrchr(strFinalPath, '/') ? : strrchr(strFinalPath, '\\')) + 1,
		"%08x%08x.tmp%s",
		(unsigned)Stream_HashCRC32(stmArc),
		(unsigned)nSize,
		strrchr(strrchr(strPathIn, '/') ? : strrchr(strPathIn, '\\') ? : strPathIn, '.') ? : ""
	);

	//Try to use buffered file
	if (!bForceWrite) {
		stmOut = Stream_Open(strFinalPath, STREAM_TYPE_STDFILE, STREAM_FLAG_READ);
		if (stmOut != NULL) {
			Stream_Close(stmOut);
			Stream_Close(stmArc);
			goto ExtractDone;
		}
	}

	//Create a new file for writing
	stmOut = Stream_Open(strFinalPath, STREAM_TYPE_STDFILE, STREAM_FLAG_WRITE);
	if (stmOut == NULL) {
		Stream_Close(stmArc);
		return false;
	}

	//Extract data
	nRead = Stream_Read(stmArc, buf, GetArrLen(buf));
	while (nRead > 0) {
		Stream_Write(stmOut, buf, nRead);
		nRead = Stream_Read(stmArc, buf, GetArrLen(buf));
	}

	Stream_Close(stmArc);
	Stream_Close(stmOut);

ExtractDone:
	//Open file
	if ((uintptr_t)ShellExecute(NULL, "open", strFinalPath, NULL, NULL, SW_SHOWDEFAULT) <= 32) {
		//Return value of ShellExecute() that > 32 indicates success
		OpenFolderAndSelectItem(strFinalPath);
		BoolErrReturn("Your system appears unable to open this kind of file. Opening it in Windows Explorer.");
	}

	return true;
}

void CmdAC_tree(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv) {
	if (argv[nArgPos][0] == '-') {
		pConEnv->chACAutoChar = ' ';
		ConAC_AddCandidate("--ignore-file", pConEnv);	//List directories only
		return;
	}

	ConAC_Default_ArcPath(argc, argv, nArgPos, pConEnv);
}
void Cmd_tree_Inner(pDXDir pDir, char *restrict strPrefix, size_t nDepth, pConsoleEnvironment pConEnv) {
	pDXFileHead pFileHead;
	size_t nPrefixLen;

	if (pDir == NULL || pDir->FileHeadNum == 0)
		return;

	//Print directories
	nPrefixLen = strlen(strPrefix);
	strcpy(&strPrefix[nPrefixLen], "|   ");

	for (size_t i = 0; i < pDir->FileHeadNum - 1; i++) {
		pFileHead = DX_GetDirFile(&arcGlobal, pDir, i);
		Stream_Printf(pConEnv->stmOut, "%.*s+---%s\n", (int)nPrefixLen, strPrefix, DX_GetOriginalFileName(&arcGlobal, pFileHead));
		if (pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY)
			Cmd_tree_Inner(DX_FileToDir(&arcGlobal, pFileHead), strPrefix, nDepth + 1, pConEnv);
	}

	strPrefix[nPrefixLen] = ' ';

	pFileHead = DX_GetDirFile(&arcGlobal, pDir, pDir->FileHeadNum - 1);
	Stream_Printf(pConEnv->stmOut, "%.*s\\---%s\n", (int)nPrefixLen, strPrefix, DX_GetOriginalFileName(&arcGlobal, pFileHead));
	if (pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY)
		Cmd_tree_Inner(DX_FileToDir(&arcGlobal, pFileHead), strPrefix, nDepth + 1, pConEnv);

	strPrefix[nPrefixLen] = '\0';
}
void Cmd_tree_Inner_IgnoreFile(pDXDir pDir, char *restrict strPrefix, size_t nDepth, pConsoleEnvironment pConEnv) {
	pDXFileHead pFileHead, *pFileHeadTemp;
	size_t nPrefixLen;
	size_t nDirNum;

	if (pDir == NULL)
		return;

	//Collect directories
	nDirNum = 0;
	for (size_t i = 0; i < pDir->FileHeadNum; i++) {
		pFileHead = DX_GetDirFile(&arcGlobal, pDir, i);
		nDirNum += (bool)(pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	if (nDirNum == 0)
		return;

	pDXFileHead pFileHeadArr[nDirNum];
	pFileHeadTemp = pFileHeadArr;

	for (size_t i = 0; i < pDir->FileHeadNum; i++) {
		pFileHead = DX_GetDirFile(&arcGlobal, pDir, i);
		if (pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY)
			*pFileHeadTemp++ = pFileHead;
	}

	//Print directories
	nPrefixLen = strlen(strPrefix);
	strcpy(&strPrefix[nPrefixLen], "|   ");

	for (size_t i = 0; i < nDirNum - 1; i++) {
		pFileHead = pFileHeadArr[i];
		Stream_Printf(pConEnv->stmOut, "%.*s+---%s\n", (int)nPrefixLen, strPrefix, DX_GetOriginalFileName(&arcGlobal, pFileHead));
		Cmd_tree_Inner_IgnoreFile(DX_FileToDir(&arcGlobal, pFileHead), strPrefix, nDepth + 1, pConEnv);
	}

	strPrefix[nPrefixLen] = ' ';

	pFileHead = pFileHeadArr[nDirNum - 1];
	Stream_Printf(pConEnv->stmOut, "%.*s\\---%s\n", (int)nPrefixLen, strPrefix, DX_GetOriginalFileName(&arcGlobal, pFileHead));
	Cmd_tree_Inner_IgnoreFile(DX_FileToDir(&arcGlobal, pFileHead), strPrefix, nDepth + 1, pConEnv);

	strPrefix[nPrefixLen] = '\0';
}
bool Cmd_tree(int argc, char *argv[], pConsoleEnvironment pConEnv) {
	char strPrefixBuf[1024];
	char strFinalPath[1024];
	pDXFileHead pFileHead;
	bool bIgnoreFile;
	char *strPathIn;
	pDXDir pCurDir;

	strPathIn = NULL;
	bIgnoreFile = false;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(&argv[i][1], "-ignore-file") == 0)
				bIgnoreFile = true;
			else
				BoolErrReturn("Unrecognized option \"%s\"", argv[i]);
		}
		else {
			if (strPathIn != NULL)
				BoolErrReturn("More than one path is specified");
			strPathIn = argv[i];
		}
	}

	if (strPathIn == NULL) {
		strcpy(strFinalPath, pConEnv->strWorkDir);
	}
	else {
		CombinePath(pConEnv->strWorkDir, strPathIn, strFinalPath);
	}

	//Check directory
	pFileHead = DX_GetPathFileHead(&arcGlobal, NULL, strFinalPath);
	if (pFileHead == NULL)
		return false;
	if (!(pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY))
		BoolErrReturn("\"%s\" is a file", DX_GetOriginalFileName(&arcGlobal, pFileHead));
	pCurDir = DX_FileToDir(&arcGlobal, pFileHead);

	//Start listing
	GetDxDirPath(&arcGlobal, pCurDir, strFinalPath);
	Stream_Printf(pConEnv->stmOut, "Tree of %s\n", strFinalPath);
	Stream_Printf(pConEnv->stmOut, ".\n");

	strPrefixBuf[0] = '\0';
	(bIgnoreFile ? Cmd_tree_Inner_IgnoreFile : Cmd_tree_Inner)(pCurDir, strPrefixBuf, 0, pConEnv);

	return true;
}

void CmdAC_extract(int argc, char *argv[], int nArgPos, pConsoleEnvironment pConEnv) {
	if (argv[nArgPos][0] == '-') {
		pConEnv->chACAutoChar = ' ';
		if (argv[nArgPos][1] == '-') {
			ConAC_AddCandidate("--no-original-filetime", pConEnv);	//Don't respect original file time stored in the archive
		}
		else {
			ConAC_AddCandidate("-i", pConEnv);	//Input path
			ConAC_AddCandidate("-o", pConEnv);	//Output path
		}
		return;
	}

	ConAC_Default_ArcPath(argc, argv, nArgPos, pConEnv);
}
bool Cmd_extract_SetFileTime(char *strFilePath, pDXFileHead pFileHead) {
	HANDLE hFile;

	hFile = CreateFile(
		strFilePath,
		GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	SetFileTime(hFile, (FILETIME*)&pFileHead->Time.Create, (FILETIME*)&pFileHead->Time.LastAccess, (FILETIME*)&pFileHead->Time.LastWrite);

	CloseHandle(hFile);

	return true;
}
void Cmd_extract_Inner(pDXDir pDir, char *strDirDest, size_t *pnCount, bool bNoOriginalFileTime, pConsoleEnvironment pConEnv) {
	pStream stmArc, stmOut;
	pDXFileHead pFileHead;
	size_t nStrLen;
	size_t nRead;

	nStrLen = strlen(strDirDest);
	strcpy(&strDirDest[nStrLen], "\\");

	//This function ignores errors by default

	for (size_t i = 0; i < pDir->FileHeadNum; i++) {
		pFileHead = DX_GetDirFile(&arcGlobal, pDir, i);
		strcpy(&strDirDest[nStrLen + 1], DX_GetOriginalFileName(&arcGlobal, pFileHead));

		if (pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY) {
			//Directory
			if (CreateDirectory(strDirDest, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
				SetErrMsg("Cannot create directory \"%s\"", strDirDest);
				PrintErrMsg();
				continue;
			}

			Cmd_extract_Inner(DX_FileToDir(&arcGlobal, pFileHead), strDirDest, pnCount, bNoOriginalFileTime, pConEnv);
		}
		else {
			//File
			char strBuf[1024];

			GetDxDirPath(&arcGlobal, pDir, strBuf);
			if (strcmp(strBuf, "/") != 0)
				strcat(strBuf, "/");
			strcat(strBuf, DX_GetOriginalFileName(&arcGlobal, pFileHead));

			Stream_Printf(pConEnv->stmOut, "%d: %s\n", (int)++(*pnCount), strBuf);

			stmArc = Stream_Open(strBuf, STREAM_TYPE_ARCFILE, STREAM_FLAG_READ);
			if (stmArc == NULL) {
				SetErrMsg("Cannot read archive file \"%s\" (%s)", strBuf, GetErrMsg());
				PrintErrMsg();
				continue;
			}

			stmOut = Stream_Open(strDirDest, STREAM_TYPE_STDFILE, STREAM_FLAG_WRITE);
			if (stmOut == NULL) {
				Stream_Close(stmArc);

				SetErrMsg("Cannot create file \"%s\" (%s)", strDirDest, GetErrMsg());
				PrintErrMsg();
				continue;
			}

			nRead = Stream_Read(stmArc, strBuf, GetArrLen(strBuf));
			while (nRead > 0) {
				Stream_Write(stmOut, strBuf, nRead);
				nRead = Stream_Read(stmArc, strBuf, GetArrLen(strBuf));
			}

			Stream_Close(stmArc);
			Stream_Close(stmOut);
		}

		if (!bNoOriginalFileTime) {
			if (!Cmd_extract_SetFileTime(strDirDest, pFileHead)) {
				SetErrMsg("Cannot set file time for \"%s\"", strDirDest);
				PrintErrMsg();
			}
		}
	}

	strDirDest[nStrLen] = '\0';
}
bool Cmd_extract(int argc, char *argv[], pConsoleEnvironment pConEnv) {
	char strFinalPathIn[1024], strFinalPathOut[1024], strPhyWorkDir[1024], buf[1024];
	char *strPathIn, *strPathOut;
	bool bNoOriginalFileTime;
	pStream stmArc, stmOut;
	pDXFileHead pFileHead;
	size_t nCount;
	size_t nRead;

	nCount = 0;

	strPathIn = strPathOut = NULL;
	bNoOriginalFileTime = false;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			BoolErrReturn("Unexpected argument \"%s\"", argv[i]);

		if (argv[i][1] == '-') {
			if (strcmp(&argv[i][2], "no-original-filetime") == 0)
				bNoOriginalFileTime = true;
			else
				BoolErrReturn("Unrecognized option \"%s\"", argv[i]);
		}
		else {
			if (strlen(&argv[i][1]) > 1)
				BoolErrReturn("Unrecognized option \"%s\"", argv[i]);

			switch (argv[i][1]) {
			case 'i':
				if (i + 1 >= argc)
					BoolErrReturn("Missing operand for \"%s\"", argv[i]);
				i++;
				strPathIn = argv[i];
				break;
			case 'o':
				if (i + 1 >= argc)
					BoolErrReturn("Missing operand for \"%s\"", argv[i]);
				i++;
				strPathOut = argv[i];
				break;
			default:
				BoolErrReturn("Unrecognized option \"%s\"", argv[i]);
			}
		}
	}

	if (strPathIn == NULL) {
		strcpy(strFinalPathIn, pConEnv->strWorkDir);
	}
	else {
		CombinePath(pConEnv->strWorkDir, strPathIn, strFinalPathIn);
	}

	GetCurrentDirectory(GetArrLen(strPhyWorkDir), strPhyWorkDir);
	if (strPathOut == NULL) {
		strcpy(strFinalPathOut, strPhyWorkDir);
	}
	else {
		Win_PathCombineA(strFinalPathOut, strPhyWorkDir, strPathOut);
	}

	//Check file
	pFileHead = DX_GetPathFileHead(&arcGlobal, NULL, strFinalPathIn);
	if (pFileHead == NULL)
		return false;

	//Start extracting
	if (pFileHead->Attributes & FILE_ATTRIBUTE_DIRECTORY) {
		//Directory
		//1) Create a folder first
		if (strcmp(DX_GetOriginalFileName(&arcGlobal, pFileHead), "/") == 0) {
			strcat(strFinalPathOut, "\\");
			strcat(strFinalPathOut, strArcFileNameGlobal);
		}
		else {
			strcat(strFinalPathOut, "\\");
			strcat(strFinalPathOut, DX_GetOriginalFileName(&arcGlobal, pFileHead));
		}

		if (CreateDirectory(strFinalPathOut, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
			BoolErrReturn("Cannot create directory \"%s\"", strFinalPathOut);

		//2) Call Cmd_extract_Inner()
		Cmd_extract_Inner(DX_FileToDir(&arcGlobal, pFileHead), strFinalPathOut, &nCount, bNoOriginalFileTime, pConEnv);
	}
	else {
		//File
		stmArc = Stream_Open(strFinalPathIn, STREAM_TYPE_ARCFILE, STREAM_FLAG_READ);
		if (stmArc == NULL)
			BoolErrReturn("Cannot read archive file \"%s\" (%s)", strFinalPathIn, GetErrMsg());

		strcat(strFinalPathOut, "\\");
		strcat(strFinalPathOut, DX_GetOriginalFileName(&arcGlobal, pFileHead));

		stmOut = Stream_Open(strFinalPathOut, STREAM_TYPE_STDFILE, STREAM_FLAG_WRITE);
		if (stmOut == NULL) {
			Stream_Close(stmArc);
			BoolErrReturn("Cannot create file \"%s\" (%s)", strFinalPathOut, GetErrMsg());
		}

		nRead = Stream_Read(stmArc, buf, GetArrLen(buf));
		while (nRead > 0) {
			Stream_Write(stmOut, buf, nRead);
			nRead = Stream_Read(stmArc, buf, GetArrLen(buf));
		}

		Stream_Close(stmArc);
		Stream_Close(stmOut);
	}

	if (!bNoOriginalFileTime) {
		if (!Cmd_extract_SetFileTime(strFinalPathOut, pFileHead)) {
			SetErrMsg("Cannot set file time for \"%s\"", strFinalPathOut);
			PrintErrMsg();
		}
	}

	return true;
}

int Con_ParseCmdLine_GetCharToken(int ch) {
	switch (ch) {
	case ' ':
	case '\t':
	case '\r':
	case '\n':
	case '\0':
		return CON_CHARTOKEN_SPACE;
	case '|':
	case '&':
	case '<':
	case '>':
		return CON_CHARTOKEN_SYMBOL;
	default:
		return CON_CHARTOKEN_TEXT;
	}
}
size_t Con_ParseCmdLine_FindMatchQuote(const char *str, size_t nBeginPos) {
	const char *strTemp;
	strTemp = strchr(&str[nBeginPos + 1], '"');
	if (strTemp == NULL)
		return -1;
	return strTemp - str;
}
void* Con_ParseCmdLine(const char *str, int *argc, char *argv[]) {
	//It will return a buffer which needs to be freed after using and let argv point to it
	//Initial value of argc represents max arguments count that argv can hold
	//It will not accept an empty string
	int nLastCharToken, nCurCharToken;
	size_t nMaxArgs;
	size_t nLenTmp;
	bool bInQuote;
	char *strTemp;
	size_t nLen;

	if (str == NULL || argc == NULL || argv == NULL) {
		SetErrMsg("Pointer is null");
		return NULL;
	}
	if (*str == '\0') {
		SetErrMsg("Command string is empty");
		return NULL;
	}

	nLen = strlen(str);

	strTemp = (char*)malloc(nLen * 2 + 2);
	if (strTemp == NULL) {
		SetErrMsg("Memory allocation failure");
		return NULL;
	}

	nMaxArgs = *argc;
	*argc = 0;

	//Add space between close tokens
	nLenTmp = 0;
	bInQuote = false;
	nLastCharToken = CON_CHARTOKEN_SPACE;
	for (size_t i = 0; i < nLen; i++) {
		if (str[i] == '"')
			bInQuote = !bInQuote;

		nCurCharToken = Con_ParseCmdLine_GetCharToken(str[i]);
		if (nCurCharToken != nLastCharToken) {
			//Char token type changed
			if (!bInQuote && (nLastCharToken != CON_CHARTOKEN_SPACE && nCurCharToken != CON_CHARTOKEN_SPACE)) {
				//No space between tokens, add one
				strTemp[nLenTmp++] = ' ';
			}
			nLastCharToken = nCurCharToken;
		}

		strTemp[nLenTmp++] = str[i];
	}
	strTemp[nLenTmp] = '\0';

	//Prase command line
	bInQuote = false;
	nLastCharToken = CON_CHARTOKEN_SPACE;
	for (size_t i = 0; i < nLenTmp; i++) {
		if (strTemp[i] == '"') {
			//Erase current " and move string (& translate double quotes to one)
			if (strTemp[i + 1] != '"')
				bInQuote = !bInQuote;
			memmove(&strTemp[i], &strTemp[i + 1], nLenTmp - (i + 1) + 1);
		}

		//Detect token change
		nCurCharToken = bInQuote ? CON_CHARTOKEN_TEXT : Con_ParseCmdLine_GetCharToken(strTemp[i]);
		if (nCurCharToken != nLastCharToken) {
			switch (nCurCharToken) {
			case CON_CHARTOKEN_SPACE:
				strTemp[i] = '\0';
				break;
			case CON_CHARTOKEN_SYMBOL:
			case CON_CHARTOKEN_TEXT:
				if (*argc >= nMaxArgs)
					return strTemp;
				argv[(*argc)++] = &strTemp[i];
				break;
			}
			nLastCharToken = nCurCharToken;
		}
	}

	return strTemp;
}

size_t Con_GetLine_TryAutoComplete_GetFrontPublicStrLen(const char *str1, const char *str2, size_t nMaxLen) {
	size_t i;
	for (i = 0; i < nMaxLen; i++) {
		if (str1[i] != str2[i])
			break;
	}
	return i;
}
bool Con_GetLine_TryAutoComplete(char *str, size_t nMaxLen, pConsoleEnvironment pConEnv) {
	uint32_t nCandidateCount;
	size_t nLenPublic;
	void *pParseTemp;
	char *argv[128];
	char *strTemp;
	size_t nCmdId;
	size_t nLen;
	int nCount;
	int argc;

	ConAC_SetItemCount(pConEnv, 0);
	pConEnv->chACAutoChar = '\0';

	//Parse command
	argc = GetArrLen(argv);
	pParseTemp = Con_ParseCmdLine(str, &argc, argv);
	if (pParseTemp == NULL)
		return false;
	if (isspace(str[strlen(str) - 1]))
		argv[argc++] = "";

#warning TODO: Add support for restarting AC when encountered &&, ||, etc...

	//Get auto completion list from command or native function
	if (argc == 1) {
		ConAC_Default_CmdName(argc, argv, 0, pConEnv);
	}
	else {
		nCmdId = Con_FindCommand(argv[0]);
		if (nCmdId != -1 && cmds[nCmdId].pfAC != NULL)
			cmds[nCmdId].pfAC(argc, argv, argc - 1, pConEnv);
	}

	//Since sub AC function may modify argv, we need to regenerate it
	free(pParseTemp);
	pParseTemp = Con_ParseCmdLine(str, &argc, argv);
	if (pParseTemp == NULL)
		return false;
	if (isspace(str[strlen(str) - 1]))
		argv[argc++] = "";

	//Detect match items from list
	nCandidateCount = ConAC_GetItemCount(pConEnv);
	if (nCandidateCount == 0)
		return false;

	//1) Early search
	nLen = strlen(argv[argc - 1]);
	for (size_t i = 0; i < nCandidateCount; i++) {
		pConEnv->bACState[i] = strncmp(argv[argc - 1], ConAC_GetItem(pConEnv, i), nLen) == 0;
		if (!pConEnv->bACState[i]) {
			if (i + 1 < nCandidateCount) {
				memmove(
					ConAC_GetItem(pConEnv, i),
					ConAC_GetItem(pConEnv, i + 1),
					GetArrLen(pConEnv->strACBuf) - (ConAC_GetItem(pConEnv, i + 1) - ConAC_GetItemStartAddress(pConEnv))
				);
			}
			nCandidateCount--;
			i--;
		}
	}

	free(pParseTemp);

	//2) Re-apply modified candidates count
	ConAC_SetItemCount(pConEnv, nCandidateCount);
	if (nCandidateCount == 0)
		return false;

	//3) Try to complete public front part with valid candidates
	nLenPublic = strlen(ConAC_GetItem(pConEnv, 0));
	for (size_t i = 1; i < nCandidateCount; i++)
		nLenPublic = Con_GetLine_TryAutoComplete_GetFrontPublicStrLen(ConAC_GetItem(pConEnv, 0), ConAC_GetItem(pConEnv, i), nLenPublic);

	//Write result back to str
	//1) Get quotes count
	nCount = 0;
	nLen = strlen(str);
	for (size_t i = 0; i < nLen; i++) {
		if (str[i] == '"') {
			nCount++;
			strTemp = &str[i];
		}
	}

	if (nCount & 1) {
		strTemp--;
		nCount--;
	}
	else {
		strTemp = &str[nLen - 1];
	}

	//2) Find position at which ought to be written
	while (strTemp > str) {
		if (*strTemp == '"') {
			nCount--;
			strTemp--;
			continue;
		}

		if (!(nCount & 1)) {
			if (isspace(*strTemp)) {
				strTemp++;
				break;
			}
		}

		strTemp--;
	}
	if (strTemp < str)
		strTemp = str;

	//3) Write back
	if (strnchr(ConAC_GetItem(pConEnv, 0), nLenPublic, ' ') != NULL) {	//Check whether string contains white spaces
		//Add quotes
		if ((strTemp - str) + nLenPublic + 2 > nMaxLen)
			return false;

		strTemp[0] = '"';
		strncpy(&strTemp[1], ConAC_GetItem(pConEnv, 0), nLenPublic);
		strTemp[nLenPublic + 1] = '"';
		strTemp[nLenPublic + 2] = '\0';

		//Smart add character when there is only one candidate
		if (pConEnv->chACAutoChar != '\0' && nCandidateCount == 1 && (strTemp - str) + nLenPublic + 3 <= nMaxLen) {
			strTemp[nLenPublic + 2] = pConEnv->chACAutoChar;
			strTemp[nLenPublic + 3] = '\0';
		}
	}
	else {
		//Directly copy
		if ((strTemp - str) + nLenPublic > nMaxLen)
			return false;

		strncpy(strTemp, ConAC_GetItem(pConEnv, 0), nLenPublic);
		strTemp[nLenPublic] = '\0';

		//Smart add character when there is only one candidate
		if (pConEnv->chACAutoChar != '\0' && nCandidateCount == 1 && (strTemp - str) + nLenPublic + 1 <= nMaxLen) {
			strTemp[nLenPublic] = pConEnv->chACAutoChar;
			strTemp[nLenPublic + 1] = '\0';
		}
	}

	return true;
}
DWORD Con_GetLine_ReadString(char *str, size_t nMaxLen, HANDLE hConIn) {
	CONSOLE_READCONSOLE_CONTROL crc;
	wchar_t wstrBuf[nMaxLen];
	DWORD dwRead;
	size_t nLen;

	nLen = strlen(str);
	crc.nLength = sizeof(CONSOLE_READCONSOLE_CONTROL);
	crc.nInitialChars = nLen;
	crc.dwCtrlWakeupMask = 1 << '\t';
	MultiByteToWideChar(CP_ACP, 0, str, nLen, wstrBuf, nLen);	//Fill the buffer so ReadConsoleW() will not be confused

	ReadConsoleW(hConIn, wstrBuf, nMaxLen - 1, &dwRead, &crc);
	WideCharToMultiByte(CP_ACP, 0, wstrBuf, dwRead, str, nMaxLen - 1, NULL, NULL);

	str[dwRead] = '\0';

	return dwRead;
}
DWORD Con_GetLine_WriteString(const char *str, size_t nLen, HANDLE hConOut) {
	wchar_t wstrBuf[nLen];
	DWORD dwWritten;
	MultiByteToWideChar(CP_ACP, 0, str, nLen, wstrBuf, nLen);
	WriteConsoleW(hConOut, wstrBuf, nLen, &dwWritten, NULL);
	return dwWritten;
}
size_t Con_GetLine(char *str, size_t nMaxLen, pConsoleEnvironment pConEnv) {	//nMaxLen includes '\0'
	uint32_t nLastInputCRC, nCurInputCRC;
	DWORD dwRead, dwWritten;
	HANDLE hConIn, hConOut;
	int xOrigin, yOrigin;
	uint32_t nCount;
	char *strTemp;
	size_t nLen;
	int ch;

	hConIn = GetStdHandle(STD_INPUT_HANDLE);
	hConOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if (str == NULL || pConEnv == NULL)
		return -1;
	if (nMaxLen == 0)
		return 0;

	str[0] = '\0';

	nLastInputCRC = DX_HashCRC32("\t", 1);

	if ((GetFileType(hConIn) & ~FILE_TYPE_REMOTE) == FILE_TYPE_CHAR) {
		//Console handle
		xOrigin = wherex();
		yOrigin = wherey();

		for (;;) {
			dwRead = Con_GetLine_ReadString(str, nMaxLen, hConIn);

			//After reading from console, there may be 2 results
			//1)	The input is terminated with \r\n and contains no \t
			//2)	The input contains \t and will not contain \r\n
			strTemp = strchr(str, '\t');
			if (strTemp == NULL) {
				//Enter is pressed, end input
				nLen = strlen(str);
				if (str[nLen - 1] == '\n' || str[nLen - 1] == '\r')
					str[nLen - 1] = '\0';
				if (str[nLen - 2] == '\r')
					str[nLen - 2] = '\0';
				break;
			}

			//Then do auto completion
			//Due to the design of ReadConsoleW(), the char after \t will be overlapped,
			//so we can only use '\t' as the end of the input
			strTemp[0] = '\0';

			nLen = strlen(str);
			FillConsoleOutputCharacter(hConOut, ' ', dwRead - nLen, (COORD) { wherex(), wherey() }, &dwWritten);

			nCurInputCRC = DX_HashCRC32(str, nLen);
			pConEnv->bFirstAC = nCurInputCRC != nLastInputCRC;
			nLastInputCRC = nCurInputCRC;
			if (pConEnv->bFirstAC || ConAC_GetItemCount(pConEnv) == 1) {
				//Try to auto complete
				if (Con_GetLine_TryAutoComplete(str, nMaxLen, pConEnv)) {
					nLen = strlen(str);

					gotoxy(xOrigin, yOrigin);
					Con_GetLine_WriteString(str, nLen, hConOut);

					if (ConAC_GetItemCount(pConEnv) > 1)	//Check whether there is more than one candidate
						nLastInputCRC = DX_HashCRC32(str, nLen);
				}
				else {
					putchar('\a');
				}
			}
			else {
				//Just print the candidates
				nCount = ConAC_GetItemCount(pConEnv);
				if (nCount == 0) {
					putchar('\a');
				}
				else {
					putchar('\n');
					for (uint32_t i = 0; i < nCount; i++)
						printf("%s\n", ConAC_GetItem(pConEnv, i));
					printf("> ");

					xOrigin = wherex();
					yOrigin = wherey();

					Con_GetLine_WriteString(str, nLen, hConOut);
				}
			}
		}
	}
	else {
		//Pure text handle
		//ReadFile() can be used here
		size_t i;

		i = 0;
		ch = getchar();
		while (ch != EOF && ch != '\n') {
			str[i++] = ch;
			if (i >= nMaxLen - 1)
				break;
			ch = getchar();
		}

		if (i == 0 && ch == EOF)
			return -1;

		str[i] = '\0';

		return i;
	}

	return strlen(str);
}
bool ConsoleMain(void) {
	char strCommandBuffer[1024];
	ConsoleEnvironment ConEnv;
	void *pParseTemp;
	char *argv[128];
	size_t nCmdId;
	int argc;

	//Prepare streams
	ConEnv.stmIn = Stream_Open(NULL, STREAM_TYPE_STDFILE_STDIN, 0);
	if (ConEnv.stmIn == NULL)
		return false;
	ConEnv.stmOut = Stream_Open(NULL, STREAM_TYPE_STDFILE_STDOUT, 0);
	if (ConEnv.stmOut == NULL) {
		Stream_Close(ConEnv.stmIn);
		return false;
	}

	strcpy(ConEnv.strWorkDir, "/");

	printf("\nNow you are in the console mode. Enter commands to do whatever you want to do.\n");

	for (;;) {
		printf("> ");
		if (Con_GetLine(strCommandBuffer, GetArrLen(strCommandBuffer), &ConEnv) == -1)
			break;
		if (strCommandBuffer[0] == '\0')
			continue;

		//Parse command line
		argc = GetArrLen(argv);
		pParseTemp = Con_ParseCmdLine(strCommandBuffer, &argc, argv);
		if (pParseTemp == NULL) {
			PrintErrMsg();
			continue;
		}

		//Detect command
		if (strcmp(argv[0], "exit") == 0)
			break;
		nCmdId = Con_FindCommand(argv[0]);
		if (nCmdId == -1) {
			SetErrMsg("Unknown command \"%s\"", argv[0]);
			PrintErrMsg();
		}
		else if (!cmds[nCmdId].pfMain(argc, argv, &ConEnv)) {
			PrintErrMsg();
		}

		free(pParseTemp);
	}

	Stream_Close(ConEnv.stmIn);
	Stream_Close(ConEnv.stmOut);
	return true;

ERR:
	Stream_Close(ConEnv.stmIn);
	Stream_Close(ConEnv.stmOut);
	return false;
}

bool GetSteamInstallPath(char *strBuf) {
	char buf[1024];
	DWORD dwSize;
	LONG nRet;

	if (strBuf == NULL)
		return false;

	dwSize = 1024;
	nRet = SHGetValueA(
		HKEY_LOCAL_MACHINE,
		"SOFTWARE\\WOW6432Node\\Valve\\Steam",
		"InstallPath",
		NULL,
		buf,
		&dwSize
	);

	if (nRet != ERROR_SUCCESS) {
		dwSize = 1024;
		nRet = SHGetValueA(
			HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Valve\\Steam",
			"InstallPath",
			NULL,
			buf,
			&dwSize
		);

		if (nRet != ERROR_SUCCESS)
			return false;
	}

	strcpy(strBuf, buf);

	return true;
}

bool GetRabiRibiGamePath(char *strBuf) {
	char strSteamPath[1024];

	if ( !GetSteamInstallPath(strSteamPath) )
		return false;

	sprintf(strBuf, "%s%s", strSteamPath, "\\steamapps\\common\\Rabi-Ribi");

	return true;
}

int main(void) {
	char strGameInstallPath[1024];
	char filename[1024];
	char chPreserved;
	char *strTemp;
	size_t nLen;

	srand((unsigned)time(NULL));
	EnableMouse(false);

	printf("Rabi-Ribi resource(DxLib) unpacker for ver1.99l (or maybe higher)\n");
	printf("Powered from GARbro and DxLib.\n");
	printf("\n");
	printf("This tool is used to view in-game resources packed in the .kanobi files.\n");
	printf("\n");

	InputKey();

	filename[0] = '\0';
	printf("Enter archive file path: ");
	scanf("%[^\n]%*c", filename);

	nLen = strlen(filename);

	// Replace patterns
	if ( GetRabiRibiGamePath(strGameInstallPath) ) {
		strTemp = StrStrIA(filename, "%GAME%");
		while (strTemp != NULL) {
			char strBuf[1024];

			sprintf(
				strBuf,
				"%.*s%s%s",
				(int)(strTemp - filename), filename,
				strGameInstallPath,
				strTemp + strlen("%GAME%")
			);

			strcpy(filename, strBuf);

			strTemp = StrStrIA(filename, "%GAME%");
		}
	}

	// Change path & store archive file name
	strTemp = strrchr(filename, '/') ? : strrchr(filename, '\\');
	if (strTemp != NULL) {
		chPreserved = *strTemp;
		*strTemp = '\0';

		if (filename[nLen - 1] == '"') {
			filename[nLen - 1] = '\0';
			SetCurrentDirectory(filename);
			filename[nLen - 1] = '"';
		}
		else {
			SetCurrentDirectory(filename);
		}

		*strTemp = chPreserved;

		strTemp++;
	}
	else {
		strTemp = filename;
	}

	strcpy(strArcFileNameGlobal, strTemp);

	nLen = strlen(strArcFileNameGlobal);
	if (strArcFileNameGlobal[nLen - 1] == '"')
		strArcFileNameGlobal[nLen - 1] = '\0';

	for (size_t i = 0; i < nLen; i++) {
		if (strArcFileNameGlobal[i] == '.')
			strArcFileNameGlobal[i] = '_';
	}

	// Do prasing
	printf("\n");
	printf("Parsing archive file \"%s\"...\n", filename);
	if (DX_OpenArchive(&arcGlobal, filename)) {
		printf("Preparing file system...\n");
		CalculateFileSystemInfo();
		if (!ConsoleMain())
			PrintErrMsg();
		DX_CloseArchive(&arcGlobal);
	}
	else {
		PrintErrMsg();
	}

	system("pause");
}
