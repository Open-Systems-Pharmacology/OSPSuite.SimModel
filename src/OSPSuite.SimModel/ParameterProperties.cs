using System;
using System.Collections.Generic;
using System.Linq;

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

   public class ParameterProperties : EntityProperties
   {
      private readonly IntPtr _allParameters; //pointer to the C++ native vector with parameter infos
      private readonly int _parameterIndex;   //current object stores/passes infos for _allParameters[_parameterIndex]

      private readonly IList<ValuePoint> _tablePoints;
      private double _value;
      private bool _calculateSensitivity;

      internal ParameterProperties(IntPtr allParameters, int parameterIndex, string entityId, string pathWithoutRoot, 
                                   string fullName, double value, double[] tablePointsX, double[] tablePointsY, bool[] tablePointsRestartSolver)
      : base(entityId, pathWithoutRoot, fullName)
      {
         _allParameters = allParameters;
         _parameterIndex = parameterIndex;

         _value = value;

         _tablePoints = new List<ValuePoint>();

         for (var idx = 0; idx < tablePointsX.Length; idx++)
         {
            _tablePoints.Add(new ValuePoint(tablePointsX[idx], tablePointsY[idx], tablePointsRestartSolver[idx]));
         }

         _calculateSensitivity = false;
      }

      internal int ParameterIndex => _parameterIndex;

      public double Value
      {
         get => _value;
         set
         {
            _value = value;
            SimulationImports.SetParameterValue(_allParameters,_parameterIndex,_value,out var success,out var errorMessage);
            PInvokeHelper.EvaluateCppCallResult(success,errorMessage);
         }
      }

      public bool CalculateSensitivity
      {
         get => _calculateSensitivity;
         set
         {
            _calculateSensitivity = value;
            SimulationImports.SetParameterCalculateSensitivity(_allParameters, _parameterIndex, _calculateSensitivity, out var success, out var errorMessage);
            PInvokeHelper.EvaluateCppCallResult(success, errorMessage);
         }
      }

      public IEnumerable<ValuePoint> TablePoints
      {
         get => _tablePoints;
         set
         {
            _tablePoints.Clear();
            foreach (var valuePoint in value)
            {
               _tablePoints.Add(new ValuePoint(valuePoint));
            }

            var xValues = _tablePoints.Select(p => p.X).ToArray();
            var yValues = _tablePoints.Select(p => p.Y).ToArray();
            var restartSolver = _tablePoints.Select(p => p.RestartSolver).ToArray();

            SimulationImports.SetParameterTablePoints(_allParameters,_parameterIndex, xValues, yValues, restartSolver,
                                                      _tablePoints.Count,out var success,out var errorMessage);
            PInvokeHelper.EvaluateCppCallResult(success, errorMessage);
         }
      }

      public bool IsUsedInSimulation
      {
         get
         {
            var used = SimulationImports.ParameterIsUsedInSimulation(_allParameters, _parameterIndex, out var success, out var errorMessage);
            PInvokeHelper.EvaluateCppCallResult(success, errorMessage);

            return used;
         }
      }

   }
}
