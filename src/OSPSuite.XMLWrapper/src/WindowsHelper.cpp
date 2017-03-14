#ifdef WIN32

#include "StdAfx.h"
#include <string>
#include "XMLWrapper/WindowsHelper.h"

const std::string BStrToSTDString (const BSTR bstr)
{
	std::string s;
   	char *  sTmp;
	if (!bstr || !*bstr) {
        s = "";
   } else {
        int slen = WideCharToMultiByte(CP_ACP, 0, bstr, -1, 0, 0, 0, 0);
        if (!slen) return "";
        sTmp = new char[slen];
        if (!sTmp) return "";
        WideCharToMultiByte(CP_ACP, 0, bstr, -1, sTmp, slen, 0, 0);
		  s=sTmp;
			delete [] sTmp;
    }
    return s;
}

HRESULT STDStringToBStr (const std::string s, BSTR & bstr)
{
    int slen = MultiByteToWideChar(CP_ACP, 0, s.data(), -1, 0, 0);
    if (!slen) return E_FAIL;
    bstr = SysAllocStringLen(0, slen-1);
    if (!bstr) return E_OUTOFMEMORY;
    if (!MultiByteToWideChar(CP_ACP, 0, s.data(), -1, bstr, slen)) return E_FAIL;
    return S_OK;
}

const std::string SourceFromComError(const _com_error & e)
{
     _bstr_t bstrSource(e.Source());
	 return std::string((const char *) bstrSource);
}

const std::string DescriptionFromComError(const _com_error & e)
{
	LPCTSTR errMsg = e.ErrorMessage();
	return std::string(CT2A(errMsg));
}

const std::string DescriptionFromHResult(HRESULT hr)
{
	_com_error err(hr);
	return DescriptionFromComError(err);
}

#endif
