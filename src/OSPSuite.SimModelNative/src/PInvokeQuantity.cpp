#include "SimModel/PInvokeHelper.h"
#include "SimModel/PInvokeQuantity.h"
#include "SimModel/Species.h"
#include "SimModel/Observer.h"
#include "SimModel/MathHelper.h"

namespace SimModelNative
{
   using namespace std;

   void GetQuantityProperties(Quantity* quantity, char** containerPath, char** name)
   {
      *containerPath = MarshalString(quantity->GetContainerPath());
      *name = MarshalString(quantity->GetName());
   }

   bool QuantityIsSpecies(Quantity* quantity)
   {
      return (dynamic_cast<SimModelNative::Species*>(quantity) != NULL);
   }

   bool QuantityIsObserver(Quantity* quantity)
   {
      return (dynamic_cast<SimModelNative::Observer*>(quantity) != NULL);
   }

   int GetQuantityValuesSize(Quantity* quantity, bool& success, char** errorMessage)
   {
      const char* ERROR_SOURCE = "GetQuantityValuesSize";
      success = true;

      try
      {
         auto variable = dynamic_cast<Variable*>(quantity);
         if (variable == NULL)
            throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Quantity is not a variable");

         return quantity->IsConstant(false) ? 1 : variable->GetValuesSize();
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
         return 0;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("FillTimeValues");
         success = false;
         return 0;
      }
   }

   //<array> has preallocated memory for <size> elements
   void FillQuantityValues(Quantity* quantity, double* values, int size, bool& success, char** errorMessage)
   {
      const char* ERROR_SOURCE = "FillQuantityValues";
      success = true;

      try
      {
         auto variable = dynamic_cast<SimModelNative::Variable*>(quantity);
         if (variable == NULL)
            throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Quantity is not a variable");

         auto valuesSize = quantity->IsConstant(false) ? 1 : variable->GetValuesSize();
         
         //check that variable values size matches the expectation
         if(valuesSize != size)
            throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Values size does not match");

         memcpy(values, variable->GetValues(), valuesSize * sizeof(double));
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown(ERROR_SOURCE);
         success = false;
      }
   }

   bool QuantityIsConstant(Quantity* quantity)
   {
      return quantity->IsConstant(false);
   }

   double GetQuantityComparisonThreshold(Quantity* quantity, bool& success, char** errorMessage)
   {
      const char* ERROR_SOURCE = "FillQuantityValues";
      success = true;

      try
      {
         auto variable = dynamic_cast <SimModelNative::Variable*> (quantity);
         if (variable == NULL) // should never happen
            throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, quantity->GetEntityId() + " is available in model but is not an entity with values(species, observer, ...)");

         return variable->GetComparisonThreshold();
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown(ERROR_SOURCE);
         success = false;
      }

      return MathHelper::GetNaN();
   }
}//.. end "namespace SimModelNative"
