#include "stdafx.h"
#include "MimeTypes.h"

CMimeTypes::CMimeTypes()
{
    _textMimeTypes.SetAt(L".css", L"text/css");
    _textMimeTypes.SetAt(L".csv", L"text/csv");
    _textMimeTypes.SetAt(L".html", L"text/html");
    _textMimeTypes.SetAt(L".htm", L"text/html");
    _textMimeTypes.SetAt(L".ics", L"text/calendar");
    _textMimeTypes.SetAt(L".js", L"text/javascript");
    _textMimeTypes.SetAt(L".json", L"application/json");
    _textMimeTypes.SetAt(L".jsonld", L"application/ld+json");
    _textMimeTypes.SetAt(L".md", L"text/markdown");
    _textMimeTypes.SetAt(L".mjs", L"text/javascript");
    _textMimeTypes.SetAt(L".rtf", L"application/rtf");
    _textMimeTypes.SetAt(L".txt", L"text/plain");
    _textMimeTypes.SetAt(L".xhtml", L"application/xhtml+xml");
    _textMimeTypes.SetAt(L".xml", L"application/xml");
    _textMimeTypes.SetAt(L".webmanifest", L"application/manifest+json");

    _binMimeTypes.SetAt(L".png", L"image/png");
    _binMimeTypes.SetAt(L".jpg", L"image/jpeg");
    _binMimeTypes.SetAt(L".jpeg", L"image/jpeg");
    _binMimeTypes.SetAt(L".gif", L"image/gif");
    _binMimeTypes.SetAt(L".webp", L"image/webp");
    _binMimeTypes.SetAt(L".svg", L"image/svg+xml");
    _binMimeTypes.SetAt(L".ico", L"image/vnd.microsoft.icon");
    _binMimeTypes.SetAt(L".mp4", L"video/mp4");
    _binMimeTypes.SetAt(L".webm", L"video/webm");
    _binMimeTypes.SetAt(L".mp3", L"audio/mpeg");
    _binMimeTypes.SetAt(L".wav", L"audio/wav");
    _binMimeTypes.SetAt(L".woff", L"font/woff");
    _binMimeTypes.SetAt(L".woff2", L"font/woff2");
    _binMimeTypes.SetAt(L".ttf", L"font/ttf");
    _binMimeTypes.SetAt(L".otf", L"font/otf");
    _binMimeTypes.SetAt(L".pdf", L"application/pdf");
    _binMimeTypes.SetAt(L".zip", L"application/zip");
    _binMimeTypes.SetAt(L".gz", L"application/gzip");
}

CString CMimeTypes::Lookup(const CString& ext, BOOL* pBinary) const
{
    CString extension = ext;
    extension.MakeLower();

    CString mime;
    if (_textMimeTypes.Lookup(extension, mime))
    {
        *pBinary = FALSE;
        mime += "; charset=utf-8";
        return mime;
    }
    
    _binMimeTypes.Lookup(extension, mime);
    *pBinary = TRUE;
    return mime;
}

CMimeTypes MimeTypes;
