using System.Collections.Generic;

namespace Ambience
{
    public class ServiceInterface
    {
        private readonly LidlModule _module;
        private readonly string _fullServiceName;

        public ServiceInterface(LidlModule mod, string name)
        {
            _module = mod;
            _fullServiceName = name;
        }

        public string AbsoluteName()
        {
            return _fullServiceName;
        }

        public string SyncServerName()
        {
            return $"{AbsoluteName()}::sync_server";
        }

        public string AsyncServerName()
        {
            return $"{AbsoluteName()}::async_server";
        }

        public string GetInclude()
        {
            return $"{System.IO.Path.GetFileNameWithoutExtension(_module.FileName())}_generated.hpp";
        }

        public Service Implement(string name, bool sync, bool external,
            Dictionary<string, ServiceInterface>? dependencies = null, string? cmakeTarget = null)
        {
            return new Service(name, this, sync, external, dependencies, cmakeTarget);
        }
    }
}