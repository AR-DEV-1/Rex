using System.Collections.Generic;

namespace rex
{
  // This class descibes a module of rex. 
  // A module is equivalent to a Visual Studio project
  // We have this class so we can save module information to disk post generation
  // so the engine can load it at runtime and use it for initialization
  public class RexModule
  {
    // The name of the modules
    public string Name { get; set; }
    // The directory where all data of this module is stored
    public string DataPath { get; set; }
    // The dependencies of a module
    public List<string> Dependencies { get; set; }
  }
}