module;

#include <Windows.h>
#include <stdio.h>

export module FileLoader;

namespace rozen::FileLoader
{
	constexpr auto MAX_JPG_SIZE = 1024 * 1024;

	export class FileLoader
	{
	protected:

		void *pBuffer = nullptr;
		DWORD dwFileSize = 0;

	public:

		FileLoader() = default;
		~FileLoader()
		{
			if (pBuffer)
			{
				free(pBuffer);
			}
		}

		void *GetBuffer() const
		{
			return pBuffer;
		}

		DWORD GetPicSize() const
		{
			return dwFileSize;
		}

		/*!
		* \brief Load a file into memory
		* \param szModule The path to the file
		* \return true if the file was loaded successfully, false otherwise
		*/
		bool LoadFile(const char* szModule)
		{
			if (!szModule || pBuffer)
			{
				return false;
			}

			HANDLE hFile = CreateFileA(szModule, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			//Get size of the file
			dwFileSize = GetFileSize(hFile, NULL);

			if(dwFileSize >= MAX_JPG_SIZE)
			{
				printf("File size is too long. Max. 1MB");
				CloseHandle(hFile);
				return false;
			}

			//Allocate memory for the file
			pBuffer = malloc(dwFileSize);
			if (!pBuffer)
			{
				CloseHandle(hFile);
				return false;
			}

			DWORD dwRead = 0;
			
			if (!ReadFile(hFile, pBuffer, dwFileSize, &dwRead, NULL))
			{
				CloseHandle(hFile);
				return false;
			}

			CloseHandle(hFile);

			return true;
		}
	};
}