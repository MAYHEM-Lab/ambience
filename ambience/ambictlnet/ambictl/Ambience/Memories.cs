namespace Ambience
{
    public record Memory(int Base, int Length);
    public record Memories(Memory Rom, Memory Ram);
}