#include "dragdropwrapper.h"
#include "ole2.h"
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <cstring>
#include <stdio.h>
#include <iostream>


class DataObject : public IDataObject
{
public:
    DataObject(FORMATETC format_etc, STGMEDIUM storage_medium)
        : refCount(1), formatEtc(format_etc), storageMedium(storage_medium)
    {}

    // Standard IUnknown methods
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override
    {
        if (iid == IID_IUnknown || iid == IID_IDataObject)
        {
            *ppvObject = static_cast<IDataObject*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef() override
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG __stdcall Release() override
    {
        ULONG count = InterlockedDecrement(&refCount);
        if (count == 0)
            delete this;
        return count;
    }

    // IDataObject methods
    HRESULT __stdcall GetData(FORMATETC* format, STGMEDIUM* medium) override
    {
        if ((format->tymed & formatEtc.tymed) != 0 &&
            format->cfFormat == formatEtc.cfFormat &&
            format->dwAspect == formatEtc.dwAspect)
        {
            medium->tymed = formatEtc.tymed;
            medium->pUnkForRelease = nullptr;

            if (formatEtc.tymed == TYMED_HGLOBAL)
            {
                auto len = GlobalSize(storageMedium.hGlobal);
                void* src = GlobalLock(storageMedium.hGlobal);
                void* dst = GlobalAlloc(GMEM_FIXED, len);

                if (src != nullptr && dst != nullptr)
                    memcpy(dst, src, len);

                GlobalUnlock(storageMedium.hGlobal);

                medium->hGlobal = dst;
                return S_OK;
            }
        }

        return DV_E_FORMATETC;
    }

    HRESULT __stdcall QueryGetData(FORMATETC* f) override
    {
        if (f == nullptr)
            return E_INVALIDARG;

        if (f->tymed == formatEtc.tymed &&
            f->cfFormat == formatEtc.cfFormat &&
            f->dwAspect == formatEtc.dwAspect)
            return S_OK;

        return DV_E_FORMATETC;
    }

    HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC*, FORMATETC* pFormatEtcOut) override
    {
        pFormatEtcOut->ptd = nullptr;
        return E_NOTIMPL;
    }

    HRESULT __stdcall EnumFormatEtc(DWORD direction, IEnumFORMATETC** result) override
    {
        *result = nullptr;
        return E_NOTIMPL;
    }

    HRESULT __stdcall GetDataHere(FORMATETC*, STGMEDIUM*) override { 
        return DATA_E_FORMATETC; 
    }

    HRESULT __stdcall SetData(FORMATETC*, STGMEDIUM*, BOOL) override { 
        return E_NOTIMPL; 
    }

    HRESULT __stdcall DAdvise(FORMATETC*, DWORD, IAdviseSink*, DWORD*) override { 
        return OLE_E_ADVISENOTSUPPORTED; 
    }

    HRESULT __stdcall DUnadvise(DWORD) override { 
        return E_NOTIMPL; 
    }

    HRESULT __stdcall EnumDAdvise(IEnumSTATDATA**) override { 
        return OLE_E_ADVISENOTSUPPORTED; 
    }
    
private:
    LONG refCount;
    FORMATETC formatEtc;
    STGMEDIUM storageMedium;

    DataObject(const DataObject&) = delete;
    DataObject& operator=(const DataObject&) = delete;
};

class DropSource : public IDropSource
{
public:
    DropSource() : refCount(1) {}

    // IUnknown methods
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override
    {
        if (iid == IID_IUnknown || iid == IID_IDropSource)
        {
            *ppvObject = static_cast<IDropSource*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef() override
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG __stdcall Release() override
    {
        ULONG count = InterlockedDecrement(&refCount);
        if (count == 0)
            delete this;
        return count;
    }

    HRESULT __stdcall QueryContinueDrag(BOOL escapePressed, DWORD keys) override
    {
        if (escapePressed)
            return DRAGDROP_S_CANCEL;
        
        if ((keys & (MK_LBUTTON | MK_RBUTTON)) == 0)
            return DRAGDROP_S_DROP;

        return S_OK;
    }

    HRESULT __stdcall GiveFeedback(DWORD dwEffect) override
    {
        UNREFERENCED_PARAMETER(dwEffect);
        return DRAGDROP_S_USEDEFAULTCURSORS;
    }

private:
    LONG refCount;

    // Prevent copying
    DropSource(const DropSource&) = delete;
    DropSource& operator=(const DropSource&) = delete;
};


extern "C" {
    void SendFileAsDragDrop(void* handle, const char* file_path, std::function<void(void)> callback) {
        // Ensure the file path is converted to Unicode
        std::wstring wideFilePath;
        int sizeRequired = MultiByteToWideChar(CP_UTF8, 0, file_path, -1, nullptr, 0);
        if (sizeRequired > 0)
        {
            wideFilePath.resize(sizeRequired - 1);
            MultiByteToWideChar(CP_UTF8, 0, file_path, -1, &wideFilePath[0], sizeRequired);
        }
        
        const size_t pathLength = wideFilePath.length() + 1;  // Include null terminator
        const size_t bufferSize = sizeof(DROPFILES) + (pathLength + 1) * sizeof(wchar_t);  // Additional NULL for final delimiter

        HGLOBAL hDrop = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bufferSize);
        if (!hDrop)
            return;

        DROPFILES* dropFiles = static_cast<DROPFILES*>(GlobalLock(hDrop));
        dropFiles->pFiles = sizeof(DROPFILES);
        dropFiles->fWide = TRUE; // Use wide character

        // Copy the file path
        wchar_t* dest = reinterpret_cast<wchar_t*>((char*)dropFiles + sizeof(DROPFILES));
        std::wcscpy(dest, wideFilePath.c_str());

        // Complete dropFiles structure
        GlobalUnlock(hDrop);

        FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM med = { TYMED_HGLOBAL };
        med.hGlobal = hDrop;

        OleInitialize(nullptr);

        // Create DataObject and DropSource objects
        auto dataObject = new DataObject(fmt, med);
        auto dropSource = new DropSource();

        DWORD effect;

        // Perform drag and drop
        HRESULT hr = DoDragDrop(dataObject, dropSource, DROPEFFECT_COPY | DROPEFFECT_MOVE, &effect);
        if (hr == DRAGDROP_S_DROP)
        {
            // Handle successful drop if needed
            std::cout << "File dropped successfully on Windows!" << std::endl;
        }

        // Cleanup
        dataObject->Release();
        dropSource->Release();

        OleUninitialize();

        GlobalFree(hDrop);

        // Call the provided callback
        if (callback)
            callback();
    }
}