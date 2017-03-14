#ifndef _ErrorData_H_
#define _ErrorData_H_

#include "SysToolConfig.h"
#include <string>

class SYSTOOL_EXPORT ErrorData
{
	public:

		//Enumeration with  all errors  types
		enum errNumber
		{
			ED_OK=0,
			ED_CRITICAL_ERROR=1,
			ED_ERROR=2
		};

	private:
		errNumber m_Number;
        std::string m_Source;
        std::string m_Description;

	public:
		ErrorData (const errNumber errNum, const std::string & sSource, const std::string & sDescription);
		ErrorData ();
		errNumber GetNumber () const;
		void SetNumber (errNumber p_Number);
		std::string GetSource () const;
		void SetSource (std::string p_Source);
		std::string GetDescription () const;
		void SetDescription (std::string p_Description);
		void operator = (const ErrorData & ED);
		void Clear ();
};

#endif //_ErrorData_H_

