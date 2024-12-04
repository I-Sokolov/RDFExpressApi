#pragma warning disable CS1587
#pragma warning disable CS1573
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
    public class DefinedType : TypeDef
    {
        public class Foundation
            {
            public enum_express_declaration declarationType = enum_express_declaration.__NONE; //Undef for primitive types, see attrType 
            public enum_express_attr_type attrType = enum_express_attr_type.__NONE;        
            public enum_express_aggr aggrType = enum_express_aggr.__NONE;
            };

        public string                     name;
        public ExpressHandle              declaration;

        public DefinedType (ExpressHandle declaration)
        {
            this.declaration = declaration;

            System.Diagnostics.Debug.Assert(ifcengine.engiGetDeclarationType(declaration) == enum_express_declaration.__DEFINED_TYPE);

            name = Schema.GetNameOfDeclaration(declaration);

            attrType = ifcengine.engiGetDefinedType(declaration, out domain, out aggregation);
        }

        public string GetBaseCSType()
        {
            if (domain != 0)
            {
                var domainType = ifcengine.engiGetDeclarationType(domain);
                switch (domainType)
                {
                    case enum_express_declaration.__DEFINED_TYPE:
                        var refType = new DefinedType(domain);
                        return refType.GetBaseCSType();

                    case enum_express_declaration.__ENTITY:
                        return "IntValue";

                    default:
                        return null;
                }
            }
            else
            {
                return Schema.GetPrimitiveType(attrType);
            }
        }

        public string GetSdaiType()
        {
            if (domain != 0)
            {
                var domainType = ifcengine.engiGetDeclarationType(domain);
                switch (domainType)
                {
                    case enum_express_declaration.__ENTITY:
                        return "sdaiINSTANCE";

                    case enum_express_declaration.__ENUM:
                        return "sdaiENUM";

                    case enum_express_declaration.__SELECT:
                        return null;

                    case enum_express_declaration.__DEFINED_TYPE:
                        var refType = new DefinedType(domain);
                        return refType.GetSdaiType();

                    default:
                        throw new ApplicationException("unknonw express type " + domainType.ToString());
                }
            }
            else
            {
                return Schema.GetSdaiType(attrType);
            }
        }

        public Foundation WriteType(Generator generator, HashSet<ExpressHandle> visitedTypes)
        {
            if (!visitedTypes.Add(declaration))
            {
                Foundation f = null;
                generator.m_writtenDefinedTyes.TryGetValue(declaration, out f);
                return f; //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            }

            var foundation = new Foundation();
            foundation.attrType = attrType;

            var referTypeName = "";

            if (domain != 0)
            {
                referTypeName = Schema.GetNameOfDeclaration(domain);
                generator.m_replacements[Generator.KWD_SimpleType] = referTypeName;

                var referType = RDF.ifcengine.engiGetDeclarationType(domain);
                foundation.declarationType = referType;

                if (referType == RDF.enum_express_declaration.__DEFINED_TYPE)
                {
                    var referencedType = new DefinedType(domain);
                    var baseFoundation = referencedType.WriteType(generator, visitedTypes);
                    foundation.aggrType = baseFoundation.aggrType;
                    foundation.attrType = baseFoundation.attrType;
                    foundation.declarationType = baseFoundation.declarationType;
                }
            }
            else if (attrType == RDF.enum_express_attr_type.__LOGICAL)
            {
                generator.m_replacements[Generator.KWD_SimpleType] = "LOGICAL_VALUE";
            }
            else
            {
                var csType = Schema.GetPrimitiveType(attrType);
                if (csType == null)
                {
                    Console.WriteLine("Defined type {0} is not supproted (primitive type is {1})", name, attrType.ToString());
                    return null;
                }

                generator.m_replacements[Generator.KWD_SimpleType] = csType;
            }

            //
            //
            if (aggregation == 0)
            {
                Generator.Template template;
                switch (foundation.declarationType)
                {
                    case enum_express_declaration.__NONE:
                        template = Generator.Template.DefinedTypeSimple;
                        break;
                    case enum_express_declaration.__ENTITY:
                        System.Diagnostics.Debug.Assert(false); //not tested
                        template = Generator.Template.DefinedTypeEntity;
                        break;
                    case enum_express_declaration.__ENUM:
                        template = Generator.Template.DefinedTypeEnum;
                        break;
                    case enum_express_declaration.__SELECT:
                        template = Generator.Template.DefinedTypeSelect;
                        break;
                    default:
                        throw new System.ApplicationException(foundation.declarationType.ToString() + ": unexpected declaration type (defined type should not be here)");
                }


                generator.m_replacements[Generator.KWD_DEFINED_TYPE] = name;
                generator.WriteByTemplate(template);

                if (foundation.declarationType == enum_express_declaration.__SELECT)
                {
                    if (!generator.m_cs)
                        {
                        generator.m_replacements[Generator.KWD_DEFINED_TYPE] = name + "_put";
                        generator.m_replacements[Generator.KWD_SimpleType] = referTypeName + "_put";
                        generator.WriteByTemplate(template);
                        }

                    generator.m_replacements[Generator.KWD_DEFINED_TYPE] = name + "_get";
                    generator.m_replacements[Generator.KWD_SimpleType] = referTypeName + "_get";
                    generator.WriteByTemplate(template);
                }

            }
            else
            {
                System.Diagnostics.Debug.Assert(foundation.declarationType != enum_express_declaration.__ENUM); //not tested
                foundation.aggrType = Aggregation.WriteDefinedType(generator, this);
            }

            generator.m_writtenDefinedTyes.Add(declaration, foundation);

            return foundation;
        }

        public void WriteSingleAttribute(Generator generator, Attribute attr)
        {
            Foundation foundation;
            if (!generator.m_writtenDefinedTyes.TryGetValue(declaration, out foundation))
            {
                return; //defined type is not supported, message for definded type already done
            }

            switch (attrType)
            {
                case RDF.enum_express_attr_type.__LOGICAL:
                    generator.WriteEnumAttribute(attr, name, "LOGICAL_VALUE", "LOGICAL_VALUE_");
                    return;

                case enum_express_attr_type.__SELECT:
                    generator.WriteSelectAttribute(attr, name);
                    return;
            }

            Console.WriteLine("Attribute '" + attr.name + "' is not supported. Defined type: " + ToString() + " based on " + foundation.declarationType.ToString() + ", defining entity " + Schema.GetNameOfDeclaration(attr.definingEntity));
        }

        public override string ToString()
        {
            var str = new StringBuilder();

            str.Append(string.Format("{0}: {1}", name, base.ToString ()));

            return str.ToString();

        }
    }
}
