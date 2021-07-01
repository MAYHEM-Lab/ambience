namespace Ambience
{
    public class DeployGroup
    {
        public readonly DeployNode Node;
        public readonly Group Group;

        public DeployGroup(DeployNode node, Group group)
        {
            Node = node;
            Group = group;
        }
    }
}