using System.Collections.Generic;
using System.Text.Json;

namespace rex
{
  // This class descibes a module of rex. 
  // A module is equivalent to a Visual Studio project
  // We have this class so we can save module information to disk post generation
  // so the engine can load it at runtime and use it for initialization
  public class RexModule
  {
    // The properties to be filled in to the module file.
    // The module file gets saved after sharpmake linking
    private Dictionary<string, object> _ModuleFields = new Dictionary<string, object>();

    // The properties to be filled in to the module file for a specific configuration
    private Dictionary<RexConfiguration, Dictionary<string, object>> _ModuleFieldsForConfig = new Dictionary<RexConfiguration, Dictionary<string, object>>();

    // Add or update a property of the module
    // all module properties will be serialized to json after sharpmake linking
    // This allows the engine to load these files at runtime to get information about its modules
    public void SetModuleProperty(string name, object value)
    {
      string nameLower = name.ToLower();

      if (!_ModuleFields.ContainsKey(nameLower))
      {
        _ModuleFields.Add(nameLower, value);
      }
      else
      {
        _ModuleFields[nameLower] = value;
      }
    }
    // Add or update a property of the module
    // all module properties will be serialized to json after sharpmake linking
    // This allows the engine to load these files at runtime to get information about its modules
    public void SetModulePropertyForConfig(RexConfiguration conf, string name, object value)
    {
      if (!_ModuleFieldsForConfig.ContainsKey(conf))
      {
        _ModuleFieldsForConfig[conf] = new Dictionary<string, object>();
      }
      Dictionary<string, object> moduleFiledsOfConfig = _ModuleFieldsForConfig[conf];
      string nameLower = name.ToLower();

      if (!moduleFiledsOfConfig.ContainsKey(nameLower))
      {
        moduleFiledsOfConfig.Add(nameLower, value);
      }
      else
      {
        moduleFiledsOfConfig[nameLower] = value;
      }
    }

    // Serialize the module to json format
    // Because a module can be configured different based on the configuration
    // we need to serialize it based on configuration as well
    public string SerializeForConfig(RexConfiguration conf)
    {
      // Merge the dictionary of generic fields and config based fields together
      Dictionary<string, object> fields = new Dictionary<string, object>();
      fields.AddRange(_ModuleFields);

      if (_ModuleFieldsForConfig.ContainsKey(conf))
      {
        fields.AddRange(_ModuleFieldsForConfig[conf]);
      }

      string jsonString = JsonSerializer.Serialize(fields, new JsonSerializerOptions()
      {
        WriteIndented = true
      });

      return jsonString;
    }
  }
}