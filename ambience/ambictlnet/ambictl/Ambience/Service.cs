using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace Ambience
{
    public class Service
    {
        public readonly ServiceInterface Interface;
        public readonly string Name;
        public readonly string? CmakeTarget;
        public readonly bool Sync;
        public readonly bool External;
        public readonly Dictionary<string, ServiceInterface> Dependencies;

        public Service(string name, ServiceInterface iface, bool sync, bool external,
            Dictionary<string, ServiceInterface>? deps = null, string? cmakeTarget = null)
        {
            Interface = iface;
            Name = name;
            CmakeTarget = cmakeTarget;
            Sync = sync;
            External = external;
            Dependencies = deps ?? new Dictionary<string, ServiceInterface>();
        }

        public string GetCxxType()
        {
            return Sync ? Interface.SyncServerName() : Interface.AsyncServerName();
        }

        public string GetCxxInitSignature()
        {
            Debug.Assert(!External);
            var args = String.Join(", ", Dependencies.Select(dep => dep.Value)
                .Select(iface => Sync ? iface.SyncServerName() : iface.AsyncServerName() + "*"));
            if (Sync)
            {
                return $"auto init_{Name}({args}) -> {GetCxxType()}*;";
            }
            return $"auto init_{Name}({args}) -> tos::Task<{GetCxxType()}*>;";
        }

        public ServiceInstance Instantiate(string name, Dictionary<string, ServiceInstance?>? dependencies = null)
        {
            return new ServiceInstance(name, this, dependencies ?? new Dictionary<string, ServiceInstance?>());
        }
    }
}