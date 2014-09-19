////////////////////////////////////////////////////////////////////////////
// InterProcessShare.hpp -- Win32 interprocess shared data
// This file is part of MZC3.  See file "ReadMe.txt" and "License.txt".
////////////////////////////////////////////////////////////////////////////

#ifndef __MZC3_INTERPROCESSSHARE__
#define __MZC3_INTERPROCESSSHARE__

////////////////////////////////////////////////////////////////////////////

template <typename T_DATA>
class MInterProcessShare
{
public:
    MInterProcessShare();
    MInterProcessShare(LPCTSTR pszName, LPBOOL pfAlreadyExists = NULL);
    virtual ~MInterProcessShare();

    BOOL Create(LPCTSTR pszName, LPBOOL pfAlreadyExists = NULL);
    void Close();

    operator bool() const;
    bool operator!() const;

    T_DATA* Lock(DWORD dwTimeout = 1500);
    void    Unlock();

    HANDLE GetMutexHandle();
    HANDLE GetFileMappingHandle();

protected:
    HANDLE m_hMutex;
    HANDLE m_hFileMapping;
    T_DATA *m_pData;
};

////////////////////////////////////////////////////////////////////////////

#undef MZC_INLINE
#define MZC_INLINE inline

template <typename T_DATA>
MZC_INLINE MInterProcessShare<T_DATA>::MInterProcessShare() :
    m_hMutex(NULL), m_hFileMapping(NULL), m_pData(NULL)
{
}

template <typename T_DATA>
MZC_INLINE MInterProcessShare<T_DATA>::MInterProcessShare(
    LPCTSTR pszName, LPBOOL pfAlreadyExists/* = NULL*/
) : m_hMutex(NULL), m_hFileMapping(NULL), m_pData(NULL)
{
    Create(pszName, pfAlreadyExists);
}

template <typename T_DATA>
MZC_INLINE /*virtual*/ MInterProcessShare<T_DATA>::~MInterProcessShare()
{
    if (m_hMutex)
        Close();
}

template <typename T_DATA>
MZC_INLINE HANDLE MInterProcessShare<T_DATA>::GetMutexHandle()
{
    return m_hMutex;
}

template <typename T_DATA>
MZC_INLINE HANDLE MInterProcessShare<T_DATA>::GetFileMappingHandle()
{
    return m_hFileMapping;
}

template <typename T_DATA>
MZC_INLINE MInterProcessShare<T_DATA>::operator bool() const
{
    return m_hMutex != NULL;
}

template <typename T_DATA>
MZC_INLINE bool MInterProcessShare<T_DATA>::operator!() const
{
    return m_hMutex == NULL;
}

template <typename T_DATA>
BOOL MInterProcessShare<T_DATA>::Create(
    LPCTSTR pszName, LPBOOL pfAlreadyExists/* = NULL*/)
{
    assert(m_hMutex == NULL);
    assert(m_hFileMapping == NULL);
    assert(m_pData == NULL);

    TCHAR szName[MAX_PATH];
    lstrcpyn(szName, pszName, MAX_PATH);
    lstrcat(szName, TEXT(" Mutex"));

    if (pfAlreadyExists)
        *pfAlreadyExists = FALSE;
    m_hMutex = ::CreateMutex(NULL, TRUE, szName);
    if (m_hMutex == NULL)
        return FALSE;

    lstrcpyn(szName, pszName, MAX_PATH);
    lstrcat(szName, TEXT(" FileMapping"));

    m_hFileMapping = ::CreateFileMapping(INVALID_HANDLE_VALUE,
        NULL, PAGE_READWRITE, 0, sizeof(T_DATA), szName);
    if (m_hFileMapping == NULL)
    {
        BOOL bOK;
        bOK = ::ReleaseMutex(m_hMutex);
        assert(bOK);

        bOK = ::CloseHandle(m_hMutex);
        m_hMutex = NULL;
        assert(bOK);

        return FALSE;
    }
    DWORD dwError = ::GetLastError();
    ::ReleaseMutex(m_hMutex);

    if (dwError == ERROR_ALREADY_EXISTS && pfAlreadyExists)
        *pfAlreadyExists = TRUE;

    return TRUE;
}

template <typename T_DATA>
void MInterProcessShare<T_DATA>::Close()
{
    if (m_pData != NULL)
        Unlock();

    if (m_hFileMapping != NULL)
    {
        BOOL bOK = ::CloseHandle(m_hFileMapping);
        assert(bOK);
        m_hFileMapping = NULL;
    }

    if (m_hMutex != NULL)
    {
        BOOL bOK = ::CloseHandle(m_hMutex);
        assert(bOK);
        m_hMutex = NULL;
    }
}

template <typename T_DATA>
MZC_INLINE T_DATA* MInterProcessShare<T_DATA>::Lock(DWORD dwTimeout/* = 1500*/)
{
    assert(m_pData == NULL);
    if (::WaitForSingleObject(m_hMutex, dwTimeout) != WAIT_OBJECT_0)
        return NULL;
    m_pData = reinterpret_cast<T_DATA *>(::MapViewOfFile(
        m_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T_DATA)));
    assert(m_pData);
    return m_pData;
}

template <typename T_DATA>
MZC_INLINE void MInterProcessShare<T_DATA>::Unlock()
{
    BOOL bOK;
    assert(m_pData);
    if (m_pData != NULL)
    {
        bOK = ::FlushViewOfFile(m_pData, sizeof(T_DATA));
        assert(bOK);
        bOK = ::UnmapViewOfFile(m_pData);
        assert(bOK);
        m_pData = NULL;
    }
    bOK = ::ReleaseMutex(m_hMutex);
    assert(bOK);
}

////////////////////////////////////////////////////////////////////////////

#endif  // ndef __MZC3_INTERPROCESSSHARE__
