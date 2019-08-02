namespace OSPSuite.SimModel
{
   public class EntityProperties
   {
      internal EntityProperties(string entityId, string path, string name)
      {
         EntityId = entityId;
         Path = path;
         Name = name;
      }

      public string EntityId { get; }
      public string Path { get; }
      public string Name { get; }
   }

   public class AdjustableEntityProperties : EntityProperties
   {
      internal AdjustableEntityProperties(long id, string entityId, string path, string name, string description, 
                                        string unit, bool isFormula, string formulaEquation, double value) 
         : base(entityId, path, name)
      {
         Id = id;
         Description = description;
         Value = value;
         Unit = unit;
         IsFormula = isFormula;
         FormulaEquation = formulaEquation;
      }

      public long Id { get; }

      public string Description { get; }

      public double Value { get; set; }

      public string Unit { get; }

      public bool IsFormula { get; }

      public string FormulaEquation { get; }

   }
}
