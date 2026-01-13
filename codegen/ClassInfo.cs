using System;
using System.Collections.Generic;
using System.Text;

namespace codegen
{
    internal class FieldInfo
    {
        public string Name = string.Empty;
        public string FieldTypename = string.Empty;
        public string DefaultValue = string.Empty;
    }

    internal class ClassInfo
    {
        public string Name = string.Empty;
        public string ClassType = string.Empty;

        public Dictionary<string, string> Metadata = new Dictionary<string, string>();

        public List<FieldInfo> Fields = new List<FieldInfo>();
    }
}
