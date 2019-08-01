using System;
using System.Collections.Generic;
using System.Text;

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

}
