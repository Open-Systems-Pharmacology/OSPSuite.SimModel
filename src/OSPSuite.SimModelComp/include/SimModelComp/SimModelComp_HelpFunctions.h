#ifndef SimModelComp_HelpFunctions__H_
#define SimModelComp_HelpFunctions__H_

#include "SimModel/Simulation.h"
#include "DCI/DCI.h"
#include "DCI/Manager.h"
#include <string>

double GetTime(void);

void CheckColumn(const DCI::ITableHandle hTab, const std::string & Name, const DCI::DataType datatype);

void AddColumn(DCI::ITableHandle hTab, std::string Name, DCI::DataType datatype);

DCI::IVariableHandle GetVarHandle(DCI::ITableHandle hTab, const DCI::String & VarName,
								  const DCI::DataType VarDataType,
                                  bool Optional = false);

void ReleaseXMLDocuments(XMLDocument& pXMLDoc);

std::string Trim(const std::string & src);

std::string StripFileName(const std::string & FilePath);

std::string StripFileDir(const std::string & FilePath);

std::string ReplacePath(const std::string & FileName, const std::string & NewPath);

#endif
