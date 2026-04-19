#include "stdafx.h"
#include "UriCoder.h"

CString CUriCoder::Encode(const CString& input)
{
    CString output;
    DWORD len = INTERNET_MAX_URL_LENGTH;
    HRESULT ok = UrlEscape(input, output.GetBuffer(INTERNET_MAX_URL_LENGTH), &len,
                           URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8 | URL_ESCAPE_ASCII_URI_COMPONENT);
    output.ReleaseBufferSetLength(static_cast<int>(len));
    return output;
}

CString CUriCoder::Decode(const CString& input)
{
    CString output;
    DWORD len = INTERNET_MAX_URL_LENGTH;
    HRESULT ok = UrlUnescape(const_cast<PTSTR>(input.GetString()), output.GetBuffer(INTERNET_MAX_URL_LENGTH), &len,
                             URL_UNESCAPE_AS_UTF8);
    output.ReleaseBufferSetLength(static_cast<int>(len));
    return output;
}

CString CUriCoder::Canonicalize(const CString& input)
{
    CString output;
    DWORD len = INTERNET_MAX_URL_LENGTH;
    HRESULT ok = UrlCanonicalize(input.GetString(), output.GetBuffer(INTERNET_MAX_URL_LENGTH), &len,
                                 URL_UNESCAPE | URL_UNESCAPE_AS_UTF8);
    output.ReleaseBufferSetLength(static_cast<int>(len));
    return output;
}
