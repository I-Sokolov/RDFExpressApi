using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RDF;

using ExpressHandle = System.Int64;

namespace RDFWrappers
{
    class Entity
    {
        public string name;
        public ExpressHandle inst;

        public Entity(ExpressHandle inst)
        {
            this.name = Schema.GetNameOfDeclaration (inst);
            this.inst = inst;
        }

        
        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public List<Attribute> GetAttributes()
        {
            var ret = new List<Attribute>();

            var nattr = ifcengine.engiGetEntityNoAttributes(inst);
            for (int i = 0; i < nattr; i++)
            {
                IntPtr ptrName = IntPtr.Zero;
                Int64 definingEntity, domainEntity, aggregation;
                enum_express_attr_type attrType;
                byte inverse, optional, unique;

                byte ok = ifcengine.engiGetEntityAttribute
                                (inst, i,
                                out ptrName,
                                out definingEntity, out inverse,
                                out attrType, out domainEntity,
                                out aggregation,
                                out optional, out unique
                                );
                System.Diagnostics.Debug.Assert(ok != 0);

                if (ok != 0)
                {
                    var prop = new Attribute
                    {
                        name = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ptrName),
                        definingEntity = definingEntity,
                        inverse = inverse != 0 ? true : false,
                        attrType = attrType,
                        domain = domainEntity,
                        aggregation = aggregation,
                        optional = optional != 0 ? true : false,
                        unique = unique != 0 ? true : false
                    };

                    //check duplications
                    bool add = true;
                    foreach (var a in ret)
                    { 
                        if (a.name == prop.name)
                        {
                            //see AP242 abstract_variable  System.Diagnostics.Debug.Assert(false);
                            add = false;
                            break;
                        }
                    }

                    //
                    if (add)
                        ret.Add(prop);
                }
            }

            return ret;
        }

        public List<ExpressHandle> GetSupertypes ()
        {
            var ret = new List<ExpressHandle>();

            int ind = 0;
            while (true)
            {
                var parentId = ifcengine.engiGetEntityParentEx(inst, ind++);
                if (parentId == 0)
                    break;
                ret.Add(parentId);
            }

            return ret;
        }

        public override string ToString()
        {
            var str = new StringBuilder();

            str.Append(string.Format("{0}:", name));

            foreach (var parent in GetSupertypes())
            {
                str.Append (string.Format(" {0}", Schema.GetNameOfDeclaration(parent)));
            }
            str.AppendLine() ;

            foreach (var attr in GetAttributes())
            {
                str.AppendLine(string.Format ("    {0}", attr.ToString()));
            }

            return str.ToString();
        }

        public bool IsAbstract()
        {
            return 0!=ifcengine.engiGetEntityIsAbstract(inst);
        }
    }
}
