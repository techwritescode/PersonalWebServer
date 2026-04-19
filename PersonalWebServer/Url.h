#pragma once

class CUrl
{
public:
    explicit CUrl(const CStringA& path);
    
    CStringA GetPath()
    {
        return _path;
    }
    
    CStringA GetQuery()
    {
        return _query;
    }
    
private:
    CStringA _path;
    CStringA _query;
};
