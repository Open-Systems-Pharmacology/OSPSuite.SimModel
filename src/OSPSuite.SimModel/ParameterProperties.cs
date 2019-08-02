using System;
using System.Collections;
using System.Collections.Generic;

namespace OSPSuite.SimModel
{
   public class ValuePoint
   {
      public ValuePoint(double x, double y, bool restartSolver)
      {
         X = x;
         Y = y;
         RestartSolver = restartSolver;
      }

      public ValuePoint(ValuePoint srcValuePoint)
         : this(srcValuePoint.X, srcValuePoint.Y, srcValuePoint.RestartSolver) { }

      public double X { get; set; }

      public double Y { get; set; }

      public bool RestartSolver { get; set; }
   }

   public class ParameterProperties : AdjustableEntityProperties
   {
      private IntPtr _parameterInfo;
      private IList<ValuePoint> _tablePoints;

      internal ParameterProperties(IntPtr parameterInfo, long id, string entityId, string pathWithoutRoot, string fullName, string description, string unit,
         bool isFormula, string formulaEquation, double value)
      : base(id, entityId, pathWithoutRoot, fullName, description,  unit, isFormula, formulaEquation, value)
      {
         _parameterInfo = parameterInfo;
         _tablePoints = new List<ValuePoint>();

         //CanBeVaried = parameterInfo.CanBeVaried(); //TODO
         //CalculateSensitivity = parameterInfo.CalculateSensitivity(); //TODO
         //IsUsedInSimulation = parameterInfo.IsUsedInSimulation(); //TODO

         IEnumerable<ValuePoint> tablePoints=new List<ValuePoint>();// = parameterInfo.GetTablePoints(); //TODO

         foreach (var tablePoint in tablePoints)
         {
            _tablePoints.Add(new ValuePoint(tablePoint));
         }
      }


      public bool CanBeVaried { get; }

      public bool CalculateSensitivity { get; set; }

      public bool IsTable { get; }

      private IEnumerable<ValuePoint> TablePoints { get; set; }

      public bool IsUsedInSimulation { get; }

   }
}
