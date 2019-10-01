#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <mmsystem.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#pragma comment(lib, "Winmm.lib")

typedef uint64_t SndQword;
typedef uint32_t SndDword;
typedef uint16_t SndWord;
typedef uint8_t SndByte;

typedef struct tagWaveFileHeaderChunk {
	SndByte ChunkId[4];
	SndDword ChunkSize;	//File length minus 8
	SndByte DataFormat[4];
}WaveFileHeaderChunk;

typedef struct tagWaveFileFmtChunk {
	SndByte SubChunkId[4];
	SndDword SubchunkSize;
	SndWord AudioFormat;
	SndWord ChannelsNumber;
	SndDword SampleRate;
	SndDword SampleByteRate;
	SndWord BlockAlign;
	SndWord BitsPerSample;
}WaveFileFmtChunk;

typedef struct tagWaveFileFmtChunkExtended {	//Used when source is not PCM
	SndWord ExtraParamSize;
	SndByte ExtraParams[];
}WaveFileFmtChunkExtend;

typedef struct tagWaveFileDataChunk {
	SndByte SubChunkId[4];
	SndDword SubchunkSize;
	SndByte SoundData[];
}WaveFileDataChunk;

typedef struct tagRIFFChunkBasicInfo {
	SndByte ChunkId[4];
	SndDword ChunkSize;
}RIFFChunkBasicInfo;

bool ReadRIFFChunkId(SndByte id[4], FILE *f) {	//File pointer must be at the beginning of the chunk
	size_t countRead;
	if (id == NULL || f == NULL)
		return false;
	countRead = fread(id, sizeof(SndByte), 4, f);
	fseek(f, -(long)(sizeof(SndByte) * countRead), SEEK_CUR);
	return countRead == 4;
}

bool ReadRIFFChunkSize(SndDword *nSize, FILE *f) {	//File pointer must be at the beginning of the chunk
	size_t countRead;
	if (nSize == NULL || f == NULL)
		return false;
	if (fseek(f, sizeof(SndDword), SEEK_CUR) != 0)
		return false;
	countRead = fread(nSize, sizeof(SndDword), 1, f);
	fseek(f, -(long)(sizeof(SndByte) * countRead + sizeof(SndDword)), SEEK_CUR);
	return countRead == 1;
}

bool ReadRIFFChunkBasicInfo(RIFFChunkBasicInfo *chunk, FILE *f) {	//File pointer must be at the beginning of the chunk
	size_t countRead;
	if (chunk == NULL || f == NULL)
		return false;
	countRead = fread(chunk, sizeof(RIFFChunkBasicInfo), 1, f);
	fseek(f, -(long)(sizeof(RIFFChunkBasicInfo) * countRead), SEEK_CUR);
	return countRead == 1;
}

#define SwitchEndian_Dword(a)	\
	((((a) & 0x000000FF) << 24) | (((a) & 0x0000FF00) << 8) | (((a) & 0x00FF0000) >> 8) | (((a) & 0xFF000000) >> 24))

bool MyPlaySound_wav(const wchar_t *path) {
#define ReturnFalseWithFileClosedAndMemoryFreed(f, mem) do { fclose(f); free(mem); return false; } while (false)
#define ReturnFalseWithMemoryFreedDeviceClosedAndHeaderUnprepared(mem, dev, hdr)	\
	do { free(mem); waveOutClose(dev); waveOutUnprepareHeader(dev, hdr, sizeof(WAVEHDR)); return false; } while (false)
#define ReturnFalseWithMemoryFreedAndDeviceClosed(mem, dev) do { free(mem); waveOutClose(dev); return false; } while (false)
#define ReturnFalseWithMemoryFreed(mem) do { free(mem); return false; } while (false)
#define ReturnFalseWithFileClosed(f) do { fclose(f); return false; } while (false)
	SndDword nDataChunkStartAddress;
	WaveFileHeaderChunk wavHeader;
	WaveFileDataChunk wavData;
	WaveFileFmtChunk wavInfo;
	WAVEFORMATEX wavFormatEx;
	HWAVEOUT wavOutDevice;
	WAVEHDR wavHdr;
	FILE *fileWave;
	SndByte *data;

	RIFFChunkBasicInfo chunkInfo;
	
	//Open file
	if ((fileWave = _wfopen(path, L"rb")) == NULL)
		return false;	//Failed: access denied or file not exist

	//Check whether it is a wave file
	if (fread(&wavHeader, sizeof(WaveFileHeaderChunk), 1, fileWave) != 1)
		ReturnFalseWithFileClosed(fileWave);	//Failed: unable to read RIFF header
	if (*(SndDword*)wavHeader.ChunkId != SwitchEndian_Dword('RIFF'))
		ReturnFalseWithFileClosed(fileWave);	//Failed: not an RIFF file
	if (*(SndDword*)wavHeader.DataFormat != SwitchEndian_Dword('WAVE'))
		ReturnFalseWithFileClosed(fileWave);	//Failed: not a wave file

	//Clear chunk id
	memset(wavInfo.SubChunkId, 0, sizeof(SndByte) * 4);
	memset(wavData.SubChunkId, 0, sizeof(SndByte) * 4);

	//Is a wave file, continue reading
	while (*(SndDword*)wavInfo.SubChunkId != SwitchEndian_Dword('fmt ') || *(SndDword*)wavData.SubChunkId != SwitchEndian_Dword('data')) {
		if (ReadRIFFChunkBasicInfo(&chunkInfo, fileWave) == false)
			ReturnFalseWithFileClosed(fileWave);	//Failed: unable to read chunk or no enough necessary chunks
		switch (*(SndDword*)chunkInfo.ChunkId) {
		case SwitchEndian_Dword('fmt '):	//Found fmt chunk
			if (fread(&wavInfo, sizeof(WaveFileFmtChunk), 1, fileWave) != 1)
				ReturnFalseWithFileClosed(fileWave);	//Failed: unable to read chunk
			if (fseek(fileWave, wavInfo.SubchunkSize - 16, SEEK_CUR) != 0)	//Skip extended information if exists
				ReturnFalseWithFileClosed(fileWave);	//Failed: unable to seek chunk
			break;
		case SwitchEndian_Dword('data'):	//Found data chunk
			nDataChunkStartAddress = (SndDword)ftell(fileWave);	//Record the start address of the data chunk
			if (fread(&wavData, sizeof(WaveFileDataChunk), 1, fileWave) != 1)
				ReturnFalseWithFileClosed(fileWave);	//Failed: unable to read chunk
			if (fseek(fileWave, wavData.SubchunkSize, SEEK_CUR) != 0)
				ReturnFalseWithFileClosed(fileWave);	//Failed: unable to seek chunk
			break;
		default:	//Found other chunk that we don't care, just skip
			if (fseek(fileWave, chunkInfo.ChunkSize + sizeof(RIFFChunkBasicInfo), SEEK_CUR) != 0)
				ReturnFalseWithFileClosed(fileWave);	//Failed: unable to seek chunk
			continue;
		}
	}

	//Allocate memory for data
	if ((data = (SndByte*)malloc(wavData.SubchunkSize)) == NULL)
		ReturnFalseWithFileClosed(fileWave);	//Failed: unable to allocate memory

	//Load data into memory
	if (fseek(fileWave, (long)nDataChunkStartAddress + sizeof(WaveFileDataChunk), SEEK_SET) != 0)
		ReturnFalseWithFileClosed(fileWave);	//Failed: unable to seek chunk
	if (fread(data, sizeof(SndByte), wavData.SubchunkSize, fileWave) != wavData.SubchunkSize)
		ReturnFalseWithFileClosedAndMemoryFreed(fileWave, data);	//Failed: unable to read audio data

	//Close file
	fclose(fileWave);

	//Open wave device
	wavFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	wavFormatEx.nChannels = wavInfo.ChannelsNumber;
	wavFormatEx.nSamplesPerSec = wavInfo.SampleRate;
	wavFormatEx.nAvgBytesPerSec = wavInfo.SampleByteRate;
	wavFormatEx.nBlockAlign = wavInfo.BlockAlign;
	wavFormatEx.wBitsPerSample = wavInfo.BitsPerSample;
	wavFormatEx.cbSize = 0;
	if (waveOutOpen(&wavOutDevice, WAVE_MAPPER, &wavFormatEx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		ReturnFalseWithMemoryFreed(data);	//Failed: unable to open wave device

	//Prepare header
	wavHdr.lpData = data;
	wavHdr.dwBufferLength = wavData.SubchunkSize;
	wavHdr.dwBytesRecorded = 0;
	wavHdr.dwUser = 0;
	wavHdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	wavHdr.dwLoops = 1;
	wavHdr.lpNext = NULL;
	wavHdr.reserved = 0;
	if (waveOutPrepareHeader(wavOutDevice, &wavHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		ReturnFalseWithMemoryFreedAndDeviceClosed(data, wavOutDevice);	//Failed: unable to prepare header

	//Write data
	if (waveOutWrite(wavOutDevice, &wavHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		ReturnFalseWithMemoryFreedDeviceClosedAndHeaderUnprepared(data, wavOutDevice, &wavHdr);	//Failed: unable to prepare header

	//Unprepare header
	while (waveOutUnprepareHeader(wavOutDevice, &wavHdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		Sleep(100);

	//Close device
	waveOutClose(wavOutDevice);

	//Free data
	free(data);

	return true;	//Succeeded
}

int wmain(int argc, wchar_t *argv[]) {
	wchar_t path[1024];
	wchar_t *str;

	if (argc == 1) {
		str = path;
		printf("Enter path: ");
		wscanf(L"%[^\n]%*c", path);
	}
	else {
		str = argv[1];
	}

	if (MyPlaySound_wav(str)) {
		printf("Succeeded.\n");
	}
	else {
		printf("Failed.\n");
	}

	printf("Press Enter to exit.\n");
	getchar();
}
