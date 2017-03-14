#include "SimModelComp/SimModelComp_HelpFunctions.h"
#include "XMLWrapper/XMLHelper.h"
#include <sys/timeb.h>
#include <time.h>

using namespace SimModelNative;
using namespace std;

double GetTime(void)
{
	double t;
#ifdef WIN32
    struct _timeb timebuffer;
    _ftime64_s( &timebuffer );

    t = timebuffer.time+timebuffer.millitm/1000.0;
#endif
#ifdef linux
    struct timeb timebuffer;
    ftime( &timebuffer );

    t = timebuffer.time+timebuffer.millitm/1000.0;
#endif
	return t;
}

void CheckColumn(const DCI::ITableHandle hTab, const string & Name, const DCI::DataType datatype)
{
	DCI::IVariableHandle hVar;
	DCI::IFieldDefHandle hFieldDef;

	if (!hTab->GetColumns()->Exists(Name.c_str()))
		throw "Missing column \""+Name+"\"";
	hVar = hTab->GetColumns()->Item(Name.c_str());
	hFieldDef = hVar->GetFieldDef();
	if (hFieldDef->GetDataType() != datatype)
		throw "Column \""+Name+"\" has invalid datatype";
}


void AddColumn(DCI::ITableHandle hTab, string Name, DCI::DataType datatype)
{
	DCI::IVariableHandle hVar;
	DCI::IFieldDefHandle hFieldDef;
    DCI::IVariableHandle dummyVar;

	hVar = hTab->GetColumns()->AddNew(Name.c_str(), dummyVar);
	hFieldDef = hVar->GetFieldDef();
	hFieldDef->SetDataType(datatype);
	hFieldDef->SetName(Name.c_str());
}




DCI::IVariableHandle GetVarHandle(DCI::ITableHandle hTab, const DCI::String & VarName,
								  const DCI::DataType VarDataType,
                                  bool Optional)
{
	DCI::IVariableHandle hVar = NULL;
	DCI::String VarDataTypeAsString;

	if (!hTab->GetColumns()->Exists(VarName))
		if(Optional)
			return hVar;
		else
			throw "Column \""+VarName+"\" in the parameter table is absent";

	hVar = hTab->GetColumns()->Item(VarName);
	if (hVar->GetFieldDef()->GetDataType() != VarDataType)
	{
		if(VarDataType == DCI::DT_DOUBLE)
			VarDataTypeAsString = "DOUBLE";
		else if(VarDataType == DCI::DT_INT)
			VarDataTypeAsString = "INT";
		else if(VarDataType == DCI::DT_STRING)
			VarDataTypeAsString = "STRING";
		else if(VarDataType == DCI::DT_DATETIME)
			VarDataTypeAsString = "DATETIME";
		else if(VarDataType == DCI::DT_ENUMERATION)
			VarDataTypeAsString = "ENUMERATION";
		else if(VarDataType == DCI::DT_BYTE)
			VarDataTypeAsString = "BYTE";
		throw "Parameter column \""+VarName+"\" must have datatype " + VarDataTypeAsString;
	}

	return hVar;
}




void ReleaseXMLDocuments(XMLDocument& pXMLDoc)
{
	try
	{
		if(!pXMLDoc.IsNull())
			pXMLDoc.Release();
	}
	catch(...){}
}

string Trim(const string & src)
{
	size_t i;
	int FirstCharPos = 0,LastCharPos = src.size() - 1;
	string LeftTrimmed;

	for(i=0; i<src.size(); i++)
	{
		if(src[i] != ' ')
			break;
		FirstCharPos++;
	}

	for(i=src.size()-1; i>=0; i--)
	{
		if(src[i] != ' ')
			break;
		LastCharPos--;
	}

	return src.substr(FirstCharPos, LastCharPos-FirstCharPos+1);
}

string StripFileName(const string & FilePath)
{
	int pos = FilePath.find_last_of(PATH_SLASH);
	if(pos<0)
		return FilePath;
	return FilePath.substr(pos+1);
}

string StripFileDir(const string & FilePath)
{
	int pos = FilePath.find_last_of(PATH_SLASH);
	if(pos<0)
		return "";
	return FilePath.substr(0,pos+1);
}

string ReplacePath(const string & FileName, const string & NewPath)
{
	string Path;
	if(NewPath.find_last_of(PATH_SLASH) < (NewPath.size()-1))
		return NewPath + PATH_SLASH + StripFileName(FileName);
	return NewPath + StripFileName(FileName);
}

