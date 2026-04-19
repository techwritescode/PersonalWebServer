#include "stdafx.h"
#include "Url.h"

CUrl::CUrl(const CStringA& path)
{
    if (const int idx = path.Find("?"); idx == -1)
    {
        _path = path;
    } else
    {
        _path = path.Mid(0, idx);
        _query = path.Mid(idx);
    }
}
