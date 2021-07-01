using System.Collections.Generic;

namespace Ambience
{
    public class Deployment
    {
        public readonly string BuildDirectory;
        public readonly List<DeployNode> Nodes;
    }
}