#include "SimModel/PInvokeHelper.h"
#include "SimModel/PInvokeQuantity.h"
#include "SimModel/Species.h"
#include "SimModel/Observer.h"
#include "SimModel/MathHelper.h"

#if defined(linux) || defined (__APPLE__)
#include <string.h> //for memcpy
#endif

namespace SimModelNative
{
   using namespace std;

   void GetQuantityProperties(Quantity* quantity, char** containerPath, char** name, char** entityId)
   {
      *containerPath = MarshalString(quantity->GetContainerPath());
      *name = MarshalString(quantity->GetName());
      *entityId = MarshalString(quantity->GetEntityId());
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

   void FillSensitivityValues(Quantity* quantity, double* values, int size, const char* parameterPath, bool& success, char** errorMessage)
   {
      const char* ERROR_SOURCE = "FillSensitivityValues";
      success = false;

      try
      {
         if (!quantity->IsPersistable())
            throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Cannot retrieve sensitivity information for " + quantity->GetPathWithoutRoot() + ": variable is declared as nonpersistable");

         auto variableWithParameterSensitivity = dynamic_cast<VariableWithParameterSensitivity*>(quantity);
         if (variableWithParameterSensitivity == NULL)
            throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Quantity " + quantity->GetPathWithoutRoot() + " is not a variable");

         auto sizeToFill = quantity->IsConstant(false) ? 1 : variableWithParameterSensitivity->GetValuesSize();

         //check that array to fill has correct length
         if (sizeToFill != size)
            throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Expected number of values does not match");

         auto parameterSensitivities = variableWithParameterSensitivity->ParameterSensitivities();
         SimModelNative::ParameterSensitivity* parameterSensitivity = NULL;

         for (auto idx = 0; idx < parameterSensitivities.size(); idx++)
         {
            if (parameterSensitivities[idx]->GetParameter()->GetPathWithoutRoot() == parameterPath)
            {
               parameterSensitivity = parameterSensitivities[idx];
               break;
            }
         }

         if (parameterSensitivity == NULL)
            throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, string(parameterPath) + " is not a valid path of a sensitivity parameter");

         memcpy(values, parameterSensitivity->GetValues(), sizeToFill * sizeof(double));

         success = true;
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

}//.. end "namespace SimModelNative"
