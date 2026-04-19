#pragma once

class CHttpResponse
{
public:
    static CHttpResponse* BuildDirectoryList(const CString& path);
    static CHttpResponse* Build404();
    
    UINT Status = 200;
    CByteArray Body;
    CMap<CStringA, LPCSTR, CStringA, LPCSTR> Headers;
    
    CByteArray* BuildResponse();

private:
    void BuildHeaders();
};
