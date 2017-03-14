#ifndef _WindowsHelper_H_
#define _WindowsHelper_H_

#include "StdAfx.h"
#include <string>
#include <atlbase.h>
#include <comdef.h>

const std::string BStrToSTDString (const BSTR bstr) ;

HRESULT STDStringToBStr (const std::string s, BSTR & bstr) ;

const std::string SourceFromComError(const _com_error & e);
const std::string DescriptionFromComError(const _com_error & e);
const std::string DescriptionFromHResult(HRESULT hr);

#endif //_WindowsHelper_H_

