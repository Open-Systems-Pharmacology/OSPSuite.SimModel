#include "ErrorData.h"

ErrorData::ErrorData ()
{
	m_Number      = ED_OK;
	m_Description = "";
	m_Source      = "";
}

ErrorData::ErrorData (const errNumber errNum, const std::string & sSource, const std::string & sDescription)
:   m_Number(errNum),
    m_Source(sSource),
    m_Description(sDescription)
{}

ErrorData::errNumber ErrorData::GetNumber () const
{
    return m_Number;
}

void ErrorData::SetNumber (errNumber p_Number)
{
    m_Number=p_Number;
}

std::string ErrorData::GetSource () const
{
    return m_Source;
}

void ErrorData::SetSource (std::string p_Source)
{
    m_Source=p_Source;
}

std::string ErrorData::GetDescription () const
{
    return m_Description;
}

void ErrorData::SetDescription (std::string p_Description)
{
    m_Description=p_Description;
}

void ErrorData::operator = (const ErrorData & ED)
{
	m_Number      = ED.GetNumber();
	m_Description = ED.GetDescription();
	m_Source      = ED.GetSource();
}

void ErrorData::Clear ()
{
	m_Number      = ED_OK;
	m_Description = "";
	m_Source      = "";
}

