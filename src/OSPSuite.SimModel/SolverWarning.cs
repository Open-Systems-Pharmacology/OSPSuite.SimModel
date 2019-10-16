namespace OSPSuite.SimModel
{
   public class SolverWarning
   {
      internal SolverWarning(double outputTime,string warning)
      {
         OutputTime = outputTime;
         Warning = warning;
      }

      public double OutputTime { get; }
      public string Warning { get; }
   }
}
