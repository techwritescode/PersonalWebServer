#include "stdafx.h"
#include "HttpParser.h"

#include "HttpRequest.h"

static void ParseAddressLine(const CStringA& line, CStringA* method, CStringA* path)
{
    int curPos = 0;
    *method = line.Tokenize(" ", curPos);
    if (*method == "") AfxThrowInvalidArgException();
    *path = line.Tokenize(" ", curPos);
    if (*path == "") AfxThrowInvalidArgException();
}

static void ParseHeaderLine(const char* line, const int lineLen, CStringA& key, CStringA& value)
{
    bool isValue = false;

    for (int i = 0; i < lineLen; i++)
    {
        if (!isValue)
        {
            if (line[i] == ':')
            {
                isValue = true;
                if (i + 1 < lineLen && line[i + 1] == ' ') i++;
            }
            else
            {
                key += line[i];
            }
        }
        else
        {
            value += line[i];
        }
    }
}

void CHttpParser::ParseRequest(const CStringA& request, CHttpRequest* req)
{
    bool pathReceived = false;

    char c = request[0];
    int idx = 0;

    char line[256] = {};
    int lineIdx = 0;

    while (c != 0)
    {
        if (c == '\r' && request[idx + 1] == '\n')
        {
            if (lineIdx == 0)
            {
            }
            else
            {
                idx++;
                if (!pathReceived)
                {
                    ParseAddressLine(line, &req->Method, &req->Path);
                    pathReceived = true;
                }
                else
                {
                    CStringA key;
                    CStringA value;
                    ParseHeaderLine(line, lineIdx, key, value);
                    
                    req->Headers.SetAt(key, value);
                }
                ZeroMemory(line, sizeof(line));
                lineIdx = 0;
            }
        }
        else
        {
            line[lineIdx++] = c;
            if (lineIdx == sizeof(line))
            {
                AfxThrowMemoryException();
            }
        }

        c = request[++idx];
    }
}
