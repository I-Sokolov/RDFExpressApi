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
    public class TypeDef
    {
        public enum_express_data_type dataType;
        public ExpressHandle domain;

        public Int64 aggregation;

        public bool IsAggregation()
        {
            if (aggregation != 0)
            {
                return true;
            }

            if (domain != 0)
            {
                if (ifcengine.engiGetDeclarationType(domain) == enum_express_declaration.__DEFINED_TYPE)
                {
                    DefinedType refer = new DefinedType(domain);
                    return refer.IsAggregation();
                }
            }

            return false;
        }

        public bool IsSimpleType(out string domainType, out string baseType, out string sdaiType)
        {
            return IsSimpleType(dataType, domain, out domainType, out baseType, out sdaiType);
        }

        private bool IsSimpleType(enum_express_data_type dataType, ExpressHandle domainEntity, out string domainTypeName, out string baseType, out string sdaiType)
        {
            domainTypeName = null;
            baseType = null;
            sdaiType = null;

            if (domainEntity != 0)
            {
                domainTypeName = Schema.GetNameOfDeclaration(domainEntity);
            }

            baseType = Schema.GetPrimitiveType(dataType);

            switch (dataType)
            {
                case enum_express_data_type.__NONE: //attribute type is defined by reference domain entity
                    if (enum_express_declaration.__DEFINED_TYPE == ifcengine.engiGetDeclarationType(domainEntity))
                    {
                        var domainType = new DefinedType(domainEntity);
                        string skip;
                        bool ret = IsSimpleType(domainType.dataType, domainType.domain, out skip, out baseType, out sdaiType);
                        return ret;
                    }
                    else
                    {
                        return false;
                    }

                case enum_express_data_type.__LOGICAL:
                case enum_express_data_type.__ENUMERATION:
                case enum_express_data_type.__SELECT:
                    return false;

                case enum_express_data_type.__BINARY:
                case enum_express_data_type.__BINARY_32:
                    sdaiType = "sdaiBINARY";
                    return true;

                case enum_express_data_type.__BOOLEAN:
                    sdaiType = "sdaiBOOLEAN";
                    return true;

                case enum_express_data_type.__INTEGER:
                    sdaiType = "sdaiINTEGER";
                    return true;

                case enum_express_data_type.__REAL:
                case enum_express_data_type.__NUMBER:
                    sdaiType = "sdaiREAL";
                    return true;

                case enum_express_data_type.__STRING:
                    sdaiType = "sdaiSTRING";
                    return true;

                default:
                    System.Diagnostics.Debug.Assert(false);
                    return false;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="entityName"></param>
        /// <returns></returns>
        public bool IsEntityReference(out string entityName)
        {
            entityName = null;

            if (dataType == enum_express_data_type.__NONE)
            {
                if (enum_express_declaration.__ENTITY == ifcengine.engiGetDeclarationType(domain))
                {
                    entityName = Schema.GetNameOfDeclaration(domain);
                    return true;
                }
            }
            return false;
        }

        public DefinedType IsDefinedType ()
        {
            if (domain != 0)
            {
                if (enum_express_declaration.__DEFINED_TYPE == ifcengine.engiGetDeclarationType(domain))
                {
                    var dt = new DefinedType(domain);
                    return dt;
                }
            }
            return null;
        }

        public Enumeraion IsEnumeration()
        {
            if (dataType == enum_express_data_type.__NONE || dataType == enum_express_data_type.__ENUMERATION)
            {
                if (enum_express_declaration.__ENUM == ifcengine.engiGetDeclarationType(domain))
                {
                    var en = new Enumeraion(domain);
                    return en;
                }
            }

            return null;
        }

        public Select IsSelect()
        {
            if (dataType == enum_express_data_type.__NONE || dataType == enum_express_data_type.__SELECT)
            {
                if (enum_express_declaration.__SELECT == ifcengine.engiGetDeclarationType(domain))
                {
                    var sel = new Select(domain);
                    return sel;
                }
            }

            return null;
        }

        public override string ToString()
        {
            var str = new StringBuilder();

            Int64 aggr = aggregation;
            while (aggr != 0)
            {
                enum_express_aggr aggrType;
                Int64 cardMin, cardMax;
                bool optional, unique;
                ifcengine.engiGetAggregationDefinition(aggr, out aggrType, out cardMin, out cardMax, out optional, out unique, out aggr);

                str.Append(aggrType.ToString() + "[" + cardMin.ToString() + ".." + cardMax.ToString() + "] OF ");
                if (optional)
                    {
                    str.Append("OPTIONAL ");
                    }
                if (unique)
                    {
                    str.Append("UNIQUE ");
                    }
            }

            switch (dataType)
            {
                case enum_express_data_type.__NONE: //attribute type is defined by reference domain entity
                    if (domain != 0)
                    {
                        str.Append("REF:");
                        str.Append(ifcengine.engiGetDeclarationType(domain));
                        str.Append(':');
                        str.Append (Schema.GetNameOfDeclaration(domain));
                    }
                    else
                    {
                        System.Diagnostics.Debug.Assert(false);
                    }
                    break;

                case enum_express_data_type.__ENUMERATION:
                    if (domain != 0)
                    {
                        str.Append ("ENUM:" + Schema.GetNameOfDeclaration(domain));
                    }
                    else
                    {
                        System.Diagnostics.Debug.Assert(false);
                    }
                    break;

                case enum_express_data_type.__SELECT:
                    if (domain != 0)
                    {
                        str.Append ("SELECT:" + Schema.GetNameOfDeclaration(domain));
                    }
                    else
                    {
                        System.Diagnostics.Debug.Assert(false);
                    }
                    break;

                case enum_express_data_type.__BINARY:
                case enum_express_data_type.__BINARY_32:
                case enum_express_data_type.__BOOLEAN:
                case enum_express_data_type.__INTEGER:
                case enum_express_data_type.__LOGICAL:
                case enum_express_data_type.__NUMBER:
                case enum_express_data_type.__REAL:
                case enum_express_data_type.__STRING:
                    str.Append (dataType.ToString());
                    if(domain != 0)
                    {
                        str.Append("<-");
                        str.Append(Schema.GetNameOfDeclaration(domain));
                    }
                    break;

                default:
                    System.Diagnostics.Debug.Assert(false);
                    break;
            }

            return str.ToString();
        }

    }
}
