using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace cs_test
{
    struct MutableStruct
    {
        public int Value { get; set; }

        public void SetValue(int newValue)
        {
            Value = newValue;
        }
    }

    class MutableStructHolder
    {
        public MutableStruct Field;
        public MutableStruct Property { get; set; }
    }

    class Program
    {
        static void Main(string[] args)
        {
            MutableStructHolder holder = new MutableStructHolder();
            // Affects the value of holder.Field
            holder.Field.SetValue(10);
            // Retrieves holder.Property as a copy and changes the copy

            holder.Property.SetValue(10);
            holder.Property = holder.Field;
            holder.Property.SetValue(11);

            Console.WriteLine(holder.Field.Value);
            Console.WriteLine(holder.Property.Value);
            Console.WriteLine(ReferenceEquals(holder.Property, holder.Field));
            Console.WriteLine(holder.Property.GetType());
            //DateTime.Now();
            //DateTime.no
        }
    }
}
