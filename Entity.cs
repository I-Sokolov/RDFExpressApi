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
        public ExpressHandle sdaiEntity;

        public Entity(ExpressHandle sdaiEntity)
        {
            this.name = Schema.GetNameOfDeclaration (sdaiEntity);
            this.sdaiEntity = sdaiEntity;
        }

        
        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public List<Attribute> GetAttributes()
        {
            var ret = new List<Attribute>();

            var model = ifcengine.engiGetEntityModel(sdaiEntity);
            var modelName = ifcengine.GetSchemaName (model);
            var isIFC = modelName.StartsWith("ifc", StringComparison.OrdinalIgnoreCase);

            ExpressHandle attribute = 0;
            while (0 != (attribute = ifcengine.engiGetEntityAttributeByIterator(sdaiEntity, attribute)))
            {
                IntPtr ptrName = IntPtr.Zero;
                Int64 definingEntity, domainEntity, aggregation;
                enum_express_attr_type attrType;
                bool inverse, direct, optional;

                ifcengine.engiGetAttrTraits
                                (attribute,
                                out ptrName,
                                out definingEntity, out direct, out inverse,
                                out attrType, out domainEntity,
                                out aggregation,
                                out optional
                                );

                if (!isIFC && !inverse && !direct)
                {
                    break;
                }

                var prop = new Attribute
                {
                    name = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ptrName),
                    definingEntity = definingEntity,
                    direct = direct,
                    attrType = attrType,
                    domain = domainEntity,
                    aggregation = aggregation,
                    optional = optional
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

            return ret;
        }

        public List<ExpressHandle> GetSupertypes ()
        {
            var ret = new List<ExpressHandle>();

            int ind = 0;
            while (true)
            {
                var parentId = ifcengine.engiGetEntityParentEx(sdaiEntity, ind++);
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
            return 0!=ifcengine.engiGetEntityIsAbstract(sdaiEntity);
        }
    }
}
