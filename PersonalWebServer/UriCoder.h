#pragma once

class CUriCoder
{
public:
    static CString Encode(const CString& input);
    static CString Decode(const CString& input);
    static CString Canonicalize(const CString& input);
};
