namespace Ambience
{
    public class LidlModule
    {
        private readonly string _absPath;
        public readonly string CmakeTarget;


        public LidlModule(string absPath, string cmakeTarget)
        {
            _absPath = absPath;
            CmakeTarget = cmakeTarget;
        }

        public string AbsoluteImport()
        {
            return _absPath;
        }

        public string FileName()
        {
            return System.IO.Path.GetFileName(_absPath);
        }

        public ServiceInterface GetService(string name)
        {
            return new ServiceInterface(this, name);
        }
    }
}