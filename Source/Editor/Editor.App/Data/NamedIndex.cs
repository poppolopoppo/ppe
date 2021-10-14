using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Editor.App.Data
{
    public class NamedIndex : IComparable<NamedIndex>, IComparable<String>
    {
        public String Name { get; set; }
        public int Index { get; set; }

        public NamedIndex(string name, int index)
        {
            Name = name;
            Index = index;
        }

        public int CompareTo(NamedIndex other)
        {
            return Name.CompareTo(other.Name);
        }

        public int CompareTo(string other)
        {
            return Name.CompareTo(other);
        }
    }
}
