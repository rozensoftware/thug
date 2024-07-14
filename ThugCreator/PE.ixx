module;

#include <Windows.h>

export module PE;

namespace rozen::PE
{
	export class PE
	{
	protected:

		IMAGE_NT_HEADERS ntHeaders = { 0 };
		IMAGE_SECTION_HEADER *pSectionHeaders = nullptr;

	public:

		PE() = default;
		~PE()
		{
			if (pSectionHeaders)
			{
				free(pSectionHeaders);
			}
		}

		/*!
		* \brief Load the PE header of an EXE file
		* \param szModule The path to the EXE file
		* \return true if the PE header was loaded successfully, false otherwise
		*/
		bool LoadPEHeader(const char* szModule)
		{
			if (!szModule)
			{
				return false;
			}

			HANDLE hFile = CreateFileA(szModule, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			DWORD dwRead = 0;
			IMAGE_DOS_HEADER dosHeader = { 0 };
			if (!ReadFile(hFile, &dosHeader, sizeof(IMAGE_DOS_HEADER), &dwRead, NULL))
			{
				CloseHandle(hFile);
				return false;
			}

			if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
			{
				CloseHandle(hFile);
				return false;
			}

			SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN);

			if (!ReadFile(hFile, &ntHeaders, sizeof(IMAGE_NT_HEADERS), &dwRead, NULL))
			{
				CloseHandle(hFile);
				return false;
			}

			// Calculate the size of the section headers
			DWORD dwSectionHeadersSize = sizeof(IMAGE_SECTION_HEADER) * ntHeaders.FileHeader.NumberOfSections;

			// Allocate memory for the section headers
			pSectionHeaders = (IMAGE_SECTION_HEADER*)malloc(dwSectionHeadersSize);
			if (!pSectionHeaders)
			{
				CloseHandle(hFile);
				return false;
			}

			// Read the section headers
			if (!ReadFile(hFile, pSectionHeaders, dwSectionHeadersSize, &dwRead, NULL))
			{
				free(pSectionHeaders);
				pSectionHeaders = nullptr;
				CloseHandle(hFile);
				return false;
			}

			CloseHandle(hFile);
			return true;
		}

		/*! 
		* \brief Write a buffer to a PE section
		 * \param szModule The path to the EXE file
		 * \param szSection The name of the section
		 * \param pBuffer The buffer (picture) to write
		 * \param nBufferSize The size of the buffer (picture)
		 * \return true if the buffer was written successfully, false otherwise
		*/
		bool WritePESection(const char* szModule, const char* szSection, unsigned char* pBuffer,
			DWORD nBufferSize)
		{
			if (!szSection || !pBuffer || !nBufferSize)
			{
				return false;
			}

			IMAGE_SECTION_HEADER* pSectionHeader = pSectionHeaders;

			char szSectionName[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };

			for (int i = 0; i < ntHeaders.FileHeader.NumberOfSections; i++)
			{
				memset(szSectionName, 0, sizeof(szSectionName));
				memcpy(szSectionName, pSectionHeader->Name, IMAGE_SIZEOF_SHORT_NAME);

				if (strcmp(szSectionName, szSection) == 0)
				{
					HANDLE hFile = CreateFileA(szModule, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile == INVALID_HANDLE_VALUE)
					{
						return false;
					}

					// Move the file pointer to the beginning of the section
					SetFilePointer(hFile, pSectionHeader->PointerToRawData, NULL, FILE_BEGIN);

					DWORD dwWritten = 0;
					
					// Write the size of the picture
					if(!WriteFile(hFile, &nBufferSize, sizeof(DWORD), &dwWritten, NULL))
					{
						CloseHandle(hFile);
						return false;
					}

					// Write the picture
					if (!WriteFile(hFile, pBuffer, nBufferSize, &dwWritten, NULL))
					{
						CloseHandle(hFile);
						return false;
					}

					CloseHandle(hFile);
					return true;
				}

				pSectionHeader++;
			}

			return false;
		}
	};
}