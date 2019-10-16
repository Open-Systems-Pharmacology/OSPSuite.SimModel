using System;
using System.Collections.Generic;
using System.Text;

namespace OSPSuite.SimModel
{
   public class SpeciesProperties : EntityProperties
   {
      private readonly IntPtr _allSpecies; //pointer to the C++ native vector with species infos
      private readonly int _speciesIndex;   //current object stores/passes infos for _allSpecies[_speciesIndex]

      private double _initialValue;
      private double _scaleFactor;

      internal SpeciesProperties(IntPtr allSpecies, int speciesIndex, string entityId, string pathWithoutRoot, string fullName, 
                                 double initialValue, double scaleFactor)
      : base(entityId, pathWithoutRoot, fullName)
      {
         _allSpecies = allSpecies;
         _speciesIndex = speciesIndex;

         _initialValue = initialValue;
         _scaleFactor = scaleFactor;
      }

      internal int SpeciesIndex => _speciesIndex;

      public double InitialValue
      {
         get => _initialValue;
         set
         {
            _initialValue = value;
            SimulationImports.SetSpeciesInitialValue(_allSpecies, _speciesIndex, _initialValue, out var success, out var errorMessage);
            PInvokeHelper.EvaluateCppCallResult(success, errorMessage);
         }
      }

      public double ScaleFactor
      {
         get => _scaleFactor;
         set
         {
            _scaleFactor = value;
            SimulationImports.SetSpeciesScaleFactor(_allSpecies, _speciesIndex, _scaleFactor, out var success, out var errorMessage);
            PInvokeHelper.EvaluateCppCallResult(success, errorMessage);
         }
      }

      public bool IsUsedInSimulation
      {
         get
         {
            var used = SimulationImports.SpeciesIsUsedInSimulation(_allSpecies, _speciesIndex, out var success, out var errorMessage);
            PInvokeHelper.EvaluateCppCallResult(success, errorMessage);

            return used;
         }
      }
   }
}
