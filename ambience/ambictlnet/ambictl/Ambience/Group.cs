using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace Ambience
{
    public abstract class Group
    {
        public readonly string Name;
        public readonly HashSet<ServiceInstance> Services;

        private DeployGroup? _deployGroup;

        public Group(string name, HashSet<ServiceInstance> services)
        {
            Name = name;
            Services = services;
            foreach (var serv in Services)
            {
                serv.AssignGroup(this);
            }
        }

        public IEnumerable<ServiceInstance> ServicesWithUnmetDeps()
        {
            return Services.Where(serv => serv.UnmetDependencies().Any());
        }

        public IEnumerable<ServiceInstance> UniqueDependencies()
        {
            return Services.SelectMany(serv => serv.Dependencies.Select(kv => kv.Value!)).Distinct();
        }

        public IEnumerable<ServiceInstance> UniqueExternalDependencies()
        {
            return UniqueDependencies().Where(x => !Services.Contains(x));
        }

        public List<ServiceInterface> OrderedInterfaceDependencies()
        {
            var interfaceDependencies = Services.Select(serv =>
                KeyValuePair.Create(serv.Implementation.Interface,
                    serv.Implementation.Dependencies.Values.Distinct().ToList())).Aggregate(
                new Dictionary<ServiceInterface, HashSet<ServiceInterface>>(),
                (cur, next) =>
                {
                    var (key, value) = next;
                    if (!cur.ContainsKey(key))
                    {
                        cur.Add(key, new HashSet<ServiceInterface>());
                    }

                    cur[key].UnionWith(value);
                    return cur;
                });

            return Toposort(interfaceDependencies);
        }

        protected List<T> Toposort<T>(Dictionary<T, HashSet<T>> elems)
        {
            var res = new Stack<T>();
            var visited = new HashSet<T>();

            Action<T>? visit_one = null;
            visit_one = elem =>
            {
                if (visited.Contains(elem))
                {
                    return;
                }

                visited.Add(elem);
                res.Push(elem);

                if (!elems.ContainsKey(elem))
                {
                    return;
                }

                foreach (var el in elems[elem])
                {
                    visit_one!(el);
                }
            };

            foreach (var el in elems)
            {
                visit_one(el.Key);
            }

            return res.ToList();
        }

        internal abstract void GenerateGroupDirectory(DeployGroup deployGroup);
    }
}