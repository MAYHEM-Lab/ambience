using System.Collections.Generic;

namespace Ambience
{
    public class DeployNode
    {
        public readonly Deployment Deployment;
        public readonly Node Node;
        public readonly List<DeployGroup> Groups;
    }
}