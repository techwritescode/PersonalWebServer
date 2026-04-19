#pragma once

class CMimeTypes
{
public:
    CMimeTypes();
    
    CString Lookup(const CString& ext, BOOL* pBinary) const;
    
private:
    CMapStringToString _binMimeTypes;
    CMapStringToString _textMimeTypes;
};

extern CMimeTypes MimeTypes;
