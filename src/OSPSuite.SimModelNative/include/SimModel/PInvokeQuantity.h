#ifndef _PInvokeQuantity_H_
#define _PInvokeQuantity_H_

#include "SimModel/Quantity.h"

namespace SimModelNative
{
   //-------------- C interface for PInvoke -----------------------------------------
   extern "C"
   {
      SIM_EXPORT void GetQuantityProperties(Quantity* quantity, char** containerPath, char** name);
      SIM_EXPORT bool QuantityIsSpecies(Quantity* quantity);
      SIM_EXPORT bool QuantityIsObserver(Quantity* quantity);
      SIM_EXPORT int GetQuantityValuesSize(Quantity* quantity, bool& success, char** errorMessage);
      SIM_EXPORT bool QuantityIsConstant(Quantity* quantity);
      
      //<array> has preallocated memory for <size> elements
      SIM_EXPORT void FillQuantityValues(Quantity* quantity, double* values, int size, bool& success, char** errorMessage);
      SIM_EXPORT double GetQuantityComparisonThreshold(Quantity* quantity, bool& success, char** errorMessage);
   }
}//.. end "namespace SimModelNative"


#endif //_PInvokeQuantity_H_

