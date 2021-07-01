using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Ambience
{
    public class ServiceInstance
    {
        public readonly string Name;
        public readonly Service Implementation;
        public readonly Dictionary<string, ServiceInstance?> Dependencies;

        private Group? _assignedGroup;

        public ServiceInstance(string name, Service impl, Dictionary<string, ServiceInstance?> deps)
        {
            Name = name;
            Implementation = impl;
            Dependencies = deps;

            foreach (var nm in Implementation.Dependencies.Keys.Where(nm => !Dependencies.ContainsKey(nm)))
            {
                Dependencies.Add(nm, null);
            }
        }

        public string CxxRegistryType()
        {
            /*        # if not self.assigned_group.privileged:
        #     return self.impl.iface.async_server_name()
*/
            return Implementation.GetCxxType();
        }

        public void AssignGroup(Group g)
        {
            _assignedGroup = g;
        }

        public IEnumerable<string> UnmetDependencies()
        {
            return Dependencies.Where(kv => kv.Value == null).Select(kv => kv.Key);
        }
    }
}