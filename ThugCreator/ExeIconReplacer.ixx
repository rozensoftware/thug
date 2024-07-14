module;

#include <windows.h>


export module ExeIconReplacer;

namespace rozen::iconreplacer
{
    typedef struct
    {
        WORD           idReserved;  // Reserved (must be 0)
        WORD           idType;      // Resource Type (1 for icons)
        WORD           idCount;     // How many images?
    } ICONDIR, * LPICONDIR;

    typedef struct
    {
        BYTE        bWidth;         // Width, in pixels, of the image
        BYTE        bHeight;        // Height, in pixels, of the image
        BYTE        bColorCount;    // Number of colors in image (0 if >=8bpp)
        BYTE        bReserved;      // Reserved ( must be 0)
        WORD        wPlanes;        // Color Planes
        WORD        wBitCount;      // Bits per pixel
        DWORD       dwBytesInRes;   // How many bytes in this resource?
        DWORD       dwImageOffset;  // Where in the file is this image?
    } ICONDIRENTRY, * LPICONDIRENTRY;

    typedef struct
    {
        BYTE   bWidth;               // Width, in pixels, of the image
        BYTE   bHeight;              // Height, in pixels, of the image
        BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
        BYTE   bReserved;            // Reserved
        WORD   wPlanes;              // Color Planes
        WORD   wBitCount;            // Bits per pixel
        DWORD   dwBytesInRes;        // how many bytes in this resource?
        WORD   nID;                  // the ID
    } GRPICONDIRENTRY, * LPGRPICONDIRENTRY;

    export class ExeIconReplacer
    {
    public:
        ExeIconReplacer()
        {
            ZeroMemory(&m_dir, sizeof(m_dir));

            m_entry = nullptr;
            m_image = nullptr;
            m_group = nullptr;
        }

        ~ExeIconReplacer() 
        {
            if (m_image) 
            {
                for (int i = 0; i < m_dir.idCount; i++) 
                {
                    delete m_image[i];
                }

                delete m_image;
            }

            if (m_entry)
			{
				delete m_entry;
			}

            if (m_group)
            {
                delete m_group;
			}
        }

        int GetImageCount() const 
        {
            return m_dir.idCount;
        }

        LPBYTE GetImageData(int index) const 
        {
            return m_image[index];
        }

        DWORD GetImageSize(int index) const 
        {
            return m_entry[index].dwBytesInRes;
        }

        BOOL IsIconDirOK() const 
        {
            return m_dir.idReserved == 0 &&
                m_dir.idType == 1 && m_dir.idCount > 0;
        }

        int SizeOfIconGroupData() const 
        {
            return sizeof(ICONDIR) +
                sizeof(GRPICONDIRENTRY) * GetImageCount();
        }

        bool LoadIconFile(LPCTSTR pszFileName) 
        {
            bool bOK = false;

            HANDLE hFile = ::CreateFile(pszFileName, GENERIC_READ,
                0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile == INVALID_HANDLE_VALUE) 
            {
                return false;
            }

            DWORD dwRead;

            if (::ReadFile(hFile, &m_dir, sizeof(ICONDIR), &dwRead, NULL)) 
            {
                if (IsIconDirOK()) 
                {
                    m_entry = new ICONDIRENTRY[m_dir.idCount];
                    bOK = true;
                    
                    for (int i = 0; i < m_dir.idCount; i++) 
                    {
                        if (!::ReadFile(hFile, &m_entry[i],
                            sizeof(ICONDIRENTRY), &dwRead, NULL))
                        {
                            bOK = false;
                            break;
                        }
                    }

                    if (bOK) 
                    {
                        m_image = new LPBYTE[m_dir.idCount];
                        bOK = true;
                        for (int i = 0; i < m_dir.idCount; i++) 
                        {
                            ::SetFilePointer(hFile, m_entry[i].dwImageOffset,
                                NULL, FILE_BEGIN);

                            m_image[i] = new BYTE[m_entry[i].dwBytesInRes];
                            if (!::ReadFile(hFile, m_image[i],
                                m_entry[i].dwBytesInRes,
                                &dwRead, NULL))
                            {
                                for (int j = i; j >= 0; --j) 
                                {
                                    delete[] m_image[i];
                                    m_image[i] = NULL;
                                }
                                bOK = false;
                                break;
                            }
                        }
                        if (!bOK) 
                        {
                            delete[] m_image;
                            m_image = NULL;
                        }
                    }
                    
                    if (!bOK) 
                    {
                        delete m_entry;
                        m_entry = NULL;
                    }
                }
            }

            ::CloseHandle(hFile);
            return bOK;
        }

        LPBYTE CreateIconGroupData(int nBaseID) 
        {
            delete m_group;
            m_group = new BYTE[SizeOfIconGroupData()];
            CopyMemory(m_group, &m_dir, sizeof(ICONDIR));

            int offset = sizeof(ICONDIR);
            for (int i = 0; i < GetImageCount(); i++) {
                BITMAPINFOHEADER    bmih;
                CopyMemory(&bmih, GetImageData(i),
                    sizeof(BITMAPINFOHEADER));

                GRPICONDIRENTRY grpEntry;
                grpEntry.bWidth = m_entry[i].bWidth;
                grpEntry.bHeight = m_entry[i].bHeight;
                grpEntry.bColorCount = m_entry[i].bColorCount;
                grpEntry.bReserved = m_entry[i].bReserved;
                grpEntry.wPlanes = bmih.biPlanes;
                grpEntry.wBitCount = bmih.biBitCount;
                grpEntry.dwBytesInRes = m_entry[i].dwBytesInRes;
                grpEntry.nID = nBaseID + i;

                CopyMemory(m_group + offset, &grpEntry,
                    sizeof(GRPICONDIRENTRY));

                offset += sizeof(GRPICONDIRENTRY);
            }

            return m_group;
        } // ExeIconReplacer::CreateIconGroupData

    protected:

        ICONDIR         m_dir;
        ICONDIRENTRY* m_entry;
        LPBYTE* m_image;
        LPBYTE          m_group;

    };

    export BOOL ReplaceIconOfExeFile(LPCTSTR pszExeFile, LPCTSTR pszIconFile,
        UINT nIconGroupID, UINT nIconBaseID)
    {
        ExeIconReplacer replacer;

        if (replacer.LoadIconFile(pszIconFile)) 
        {
            HANDLE hUpdate = ::BeginUpdateResource(pszExeFile, FALSE);
            if (hUpdate) 
            {
                // RT_GROUP_ICON
                BOOL bOK = ::UpdateResource(hUpdate, RT_GROUP_ICON,
                    MAKEINTRESOURCE(nIconGroupID), 0,
                    replacer.CreateIconGroupData(nIconBaseID),
                    replacer.SizeOfIconGroupData()
                );

                // RT_ICON
                if (bOK) 
                {
                    for (int i = 0; i < replacer.GetImageCount(); i++) 
                    {
                        bOK = ::UpdateResource(hUpdate, RT_ICON,
                            MAKEINTRESOURCE(nIconBaseID + i), 0,
                            replacer.GetImageData(i),
                            replacer.GetImageSize(i)
                        );

                        if (!bOK) 
                        {
                            break;
                        }
                    }
                }

                // finish
                if (bOK) 
                {
                    return ::EndUpdateResource(hUpdate, FALSE);
                }
                
                ::EndUpdateResource(hUpdate, TRUE);
            }
        }
        return FALSE;
    }
}