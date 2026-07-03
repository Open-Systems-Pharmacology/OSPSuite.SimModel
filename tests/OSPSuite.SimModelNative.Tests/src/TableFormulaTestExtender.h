#ifndef _TableFormulaTestExtender_H_
#define _TableFormulaTestExtender_H_

#include "SimModel/TableFormula.h"
#include "SimModel/TObjectVector.h"

namespace SimModelNative
{
   // Provide access to protected members of TableFormula via public inheritance.
   // (Originally implemented in the C++/CLI helper TableFormulaSpecsHelper.h.)
   // Shared across the TableFormula spec files to avoid duplicating the wrapper.
   class TableFormulaTestExtender : public TableFormula
   {
   public:
      TObjectVector<ValuePoint>& ValuePoints() { return _valuePoints; }
      void CallCacheValues() { CacheValues(); }
   };
}

#endif //_TableFormulaTestExtender_H_
