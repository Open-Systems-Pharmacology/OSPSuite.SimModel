using System;
using System.Runtime.InteropServices;

namespace OSPSuite.SimModel
{
   internal class QuantityImports
   {
      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern bool QuantityIsConstant(IntPtr quantity);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetQuantityValuesSize(IntPtr quantity, out bool success, out string errorMessage);

      //<array> has preallocated memory for <size> elements
      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillQuantityValues(IntPtr quantity, [In, Out] double[] values, int size, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern double GetQuantityComparisonThreshold(IntPtr quantity, out bool success, out string errorMessage);
   }

   public class VariableValues : EntityProperties
   {
      //pointer to C++ quantity (species or observer)
      private readonly IntPtr _quantity;

      public enum VariableTypes
      {
         Species = 1,
         Observer = 2
      };

      public VariableValues(IntPtr quantity, VariableTypes variableType, string entityId, string path, string name) :
         base(entityId, path, name)
      {
         _quantity = quantity;
         VariableType = variableType;
      }

      public VariableTypes VariableType { get; }

      public double[] Values
      {
         get
         {
            var size = QuantityImports.GetQuantityValuesSize(_quantity, out var success, out var errorMessage);
            evaluateCppCallResult(success, errorMessage);

            var values=new double[size];
            QuantityImports.FillQuantityValues(_quantity,values,size,out success, out errorMessage);
            evaluateCppCallResult(success, errorMessage);

            return values;
         }
      }

      public bool IsConstant => QuantityImports.QuantityIsConstant(_quantity);

      public double ComparisonThreshold
      {
         get
         {
            double threshold =
               QuantityImports.GetQuantityComparisonThreshold(_quantity, out var success, out var errorMessage);
            evaluateCppCallResult(success,errorMessage);

            return threshold;
         }
      }

      private void evaluateCppCallResult(bool success, string errorMessage)
      {
         PInvokeHelper.EvaluateCppCallResult(success, errorMessage);
      }
   }
}
