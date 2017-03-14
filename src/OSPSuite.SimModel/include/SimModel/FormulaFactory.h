#ifndef _FormulaFactory_H_
#define _FormulaFactory_H_

#include <string>
#include "SimModel/Formula.h"

namespace SimModelNative
{

class FormulaFactory
{
public:
	FormulaFactory(void);
	virtual ~FormulaFactory(void);

	static Formula * CreateFormula(std::string formulaName);
};

}//.. end "namespace SimModelNative"


#endif //_FormulaFactory_H_
