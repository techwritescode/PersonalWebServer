#pragma once
#include "HttpRequest.h"

class CHttpParser
{
public:
    static void ParseRequest(const CStringA& request, CHttpRequest* req);
};
