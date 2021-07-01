using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using RazorLight;

namespace Ambience.Groups
{
    public class UserGroup : Group
    {
        public UserGroup(string name, HashSet<ServiceInstance> services) : base(name, services)
        {
        }

        private static IEnumerable<(int index, T value)> Enumerate<T>(IEnumerable<T> coll)
            => coll.Select((i, val) => (val, i));

        private IEnumerable<KeyValuePair<ServiceInstance, int>> AssignNumsToExternalDeps()
        {
            Debug.Assert(!ServicesWithUnmetDeps().Any());

            return Enumerate(UniqueExternalDependencies()).Select(el => KeyValuePair.Create(el.value, el.index + 1));
        }

        public string GenerateExternalDepsSection()
        {
            var numbering = AssignNumsToExternalDeps();
            return string.Join("\n",
                numbering.Select(kv =>
                    $"auto ext_dep{kv.Value} = transport.get_service<{kv.Key.Implementation.Interface.AbsoluteName()}, {kv.Value}>();"));
        }
        //def generateInitSigSection(self):
        //unique_services = set(s.impl for s in self.servs)
        //return "\n".join(s.getInitSignature() for s in unique_services)

        public string GenerateInitSigSection()
        {
            var uniqueServices = Services.Select(serv => serv.Implementation).Distinct();
            return string.Join("\n", uniqueServices.Select(serv => serv.GetCxxInitSignature()));
        }
            
        public List<string> GenerateInitSection()
        {
            var serv_name_mapping = new Dictionary<ServiceInstance, string>();
            foreach (var (serv, num) in AssignNumsToExternalDeps())
            {
                serv_name_mapping[serv] = $"ext_dep{num}";
            }

            foreach (var serv in Services)
            {
                serv_name_mapping[serv] = serv.Name;
            }

            var ext_deps = UniqueExternalDependencies();
            var in_deps = Services.Select(serv =>
                    KeyValuePair.Create(serv,
                        serv.Dependencies.Where(dep => !ext_deps.Contains(dep.Value!)).Select(kv => kv.Value)
                            .ToHashSet()))
                .ToDictionary(x => x.Key, x => x.Value);
            var sorted = Toposort<ServiceInstance>(in_deps);

            var res = new List<string>();
            foreach (var serv in sorted)
            {
                var args = string.Join(", ", serv.Dependencies.Values.Select(x => serv_name_mapping[x!]));
                res.Add($"auto {serv.Name} = co_await init_{serv.Implementation.Name}({args});");
            }

            return res;
        }

        public Task<string> GenerateBody()
        {
            var engine = new RazorLightEngineBuilder()
                .UseEmbeddedResourcesProject(Assembly.GetAssembly(typeof(UserGroup)))
                .SetOperatingAssembly(typeof(UserGroup).Assembly)
                .UseMemoryCachingProvider()
                .Build();

            var groupInit =
                $"::g = new tos::ae::group<{Services.Count}>(tos::ae::group<{Services.Count}>::make({string.Join(", ", Services.Select(serv => serv.Name))}));";
            
            return engine.CompileRenderAsync("Ambience.Templates.Node.Groups.UserGroup.group.cpp",
                new {Group = this, ExternalDeps = GenerateExternalDepsSection(), Inits = GenerateInitSection(), GroupInit = groupInit, Signatures = GenerateInitSigSection()});
        }

        internal override void GenerateGroupDirectory(DeployGroup deployGroup)
        {
            var subDir = Path.Join(deployGroup.Node.Deployment.BuildDirectory, Name);
            Directory.CreateDirectory(subDir);

            using (var writer = new StreamWriter(Path.Join(subDir, "null.cpp")))
            {
                writer.WriteLine("");
            }

            using (var writer = new StreamWriter(Path.Join(subDir, "linker.ld")))
            {
                writer.WriteLine("");
            }
        }
    }
}