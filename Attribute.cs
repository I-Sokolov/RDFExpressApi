#pragma warning disable CS1587
#pragma warning disable CS1591

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RDF;

using ExpressHandle = System.Int64;

namespace RDFWrappers
{
    public class Attribute : TypeDef
    {
        public string name;
        public ExpressHandle definingEntity;
        public bool explicit_;
        public bool optional;

        private string DefiningEntity { get { return Schema.GetNameOfDeclaration(definingEntity); } }
        //private string Domain { get { return ExpressSchema.GetNameOfDeclaration(domain); } }

        override public string ToString()
        {
            var str = new StringBuilder();

            str.Append(name + ": ");

            if (explicit_)
            {
                str.Append("explicit ");
            }

            str.Append(base.ToString());

            System.Diagnostics.Debug.Assert(definingEntity != 0);
            str.Append(" defined by " + DefiningEntity);

            return str.ToString();
        }

    }
}
