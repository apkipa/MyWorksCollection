#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>

int g_nLastError;

void PrintHelp(const char *strSelfPath) {
	printf("Usage: %s <exePath>\n", strSelfPath);
}

bool DoesManifestExist(const char *strFilePath) {
	HMODULE hExeFile;
	bool bExists;

	if (strFilePath == NULL)
		return false;

	hExeFile = LoadLibraryA(strFilePath);
	if (hExeFile == NULL)
		return false;

	bExists = FindResourceA(hExeFile, MAKEINTRESOURCE(1), RT_MANIFEST) != NULL;

	FreeLibrary(hExeFile);

	return bExists;
}

bool RemoveManifest(const char *strFilePath) {
	HANDLE hExeFile;

	if (strFilePath == NULL)
		return false;

	hExeFile = BeginUpdateResourceA(strFilePath, false);
	if (hExeFile == NULL) {
		g_nLastError = GetLastError();
		return false;
	}

	if ( !UpdateResourceA(hExeFile, RT_MANIFEST, MAKEINTRESOURCE(1), 1033, NULL, 0) ) {
		g_nLastError = GetLastError();
		EndUpdateResourceA(hExeFile, true);
		return false;
	}

	if ( !EndUpdateResourceA(hExeFile, false) ) {
		g_nLastError = GetLastError();
		return false;
	}

	return true;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Error: incorrect argument count\n");
		PrintHelp(argv[0]);
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "--help") == 0) {
		PrintHelp(argv[0]);
		return EXIT_SUCCESS;
	}

	if ( !DoesManifestExist(argv[1]) ) {
		printf("The executable file contains no manifest files.\n");
		return EXIT_SUCCESS;
	}

	if ( !RemoveManifest(argv[1]) ) {
		fprintf(stderr, "Error: failed to process \"%s\"(LastError = %d)\n", argv[1], g_nLastError);
		return EXIT_FAILURE;
	}
}