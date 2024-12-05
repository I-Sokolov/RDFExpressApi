using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RDF;

namespace RDFWrappers
{
    class Aggregation
    {
        /// <summary>
        /// 
        /// </summary>
        /// <param name="generator"></param>
        public static void WriteAttributesTypes (Generator generator)
        {
            generator.WriteByTemplate(Generator.Template.AggregationTypesBegin);

            foreach (var decl in generator.m_schema.m_declarations[enum_express_declaration.__ENTITY])
            {
                var entity = new Entity(decl.Value);

                var attrs = entity.GetAttributes();
                foreach (var attr in attrs)
                {
                    if (attr.aggregation != 0) //unnamed aggregation
                    {
                        var aggr = new Aggregation(generator, attr);
                        aggr.WriteType(null);
                    }
                }
            }
        }

        public static enum_express_aggr WriteDefinedType (Generator generator, DefinedType definedType)
        {
            var aggr = new Aggregation(generator, definedType);
            return aggr.WriteType(definedType.name);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="generator"></param>
        /// <param name="attr"></param>
        public static void WriteAttribute (Generator generator, Attribute attr)
        {
            var aggr = new Aggregation(generator, attr);
            aggr.WriteAttribute(attr.name, attr.direct);
        }

        /// <summary>
        /// 
        /// </summary>
        private Generator generator;
        private TypeDef typeDef;

        private Aggregation(Generator generator, TypeDef typeDef)
        {
            this.generator = generator;
            this.typeDef = typeDef;
        }


        enum_express_aggr WriteType (string name)
        {
            string elemIfcType;
            string elemApiType;
            string sdaiType;
            string enumValues;

            Generator.Template template = GetAggregatedType(typeDef, out elemIfcType, out elemApiType, out sdaiType, out enumValues);

            if (template != Generator.Template.None)
            {
                enum_express_aggr aggrType = enum_express_aggr.__NONE;
                var aggregation = typeDef.aggregation;
                while (aggregation!=0)
                {
                    Int64 crdMin, crdMax;
                    bool optional, unique;
                    ifcengine.engiGetAggregationDefinition(aggregation, out aggrType, out crdMin, out crdMax, out optional, out unique, out aggregation);
                    System.Diagnostics.Debug.Assert(!(aggregation != 0 && name != null), "test nested aggregation for definded type"); 

                    string aggrName = (aggregation == 0) ? name : null; //use given name for outer aggregation
                    if (aggrName == null)
                    {
                        aggrName = MakeAggregationTypeName(aggrType, elemIfcType);
                    }
                    if (aggrName == null)
                    {
                        return enum_express_aggr.__NONE;
                    }

                    if (generator.m_writtenAggregationTypes.Add(aggrName))
                    {
                        generator.m_replacements[Generator.KWD_sdaiTYPE] = sdaiType;
                        generator.m_replacements[Generator.KWD_AggregationType] = aggrName;
                        generator.m_replacements[Generator.KWD_SimpleType] = elemApiType;
                        generator.m_replacements[Generator.KWD_TextType] = elemApiType;
                        generator.m_replacements[Generator.KWD_ENUMERATION_NAME] = elemApiType;
                        generator.m_replacements[Generator.KWD_REF_ENTITY] = elemApiType;
                        generator.m_replacements[Generator.KWD_TYPE_NAME] = elemApiType;
                        generator.m_replacements[Generator.KWD_TypeNameIFC] = elemIfcType;
                        if (enumValues != null)
                            generator.m_replacements[Generator.KWD_ENUMERATION_VALUES_ARRAY] = enumValues;
                        generator.WriteByTemplate(template);
                    }

                    //for outer aggregarion
                    elemIfcType = aggrName;
                    elemApiType = aggrName;
                    name = null;
                    template = Generator.Template.AggregationOfAggregation;
                }

                return aggrType;
            }

            return enum_express_aggr.__NONE;
        }

        private string MakeAggregationTypeName (enum_express_aggr aggrType, string elemType)
        {
            System.Diagnostics.Debug.Assert(elemType != null);
            bool camelCase = Char.IsUpper(elemType.First());

            var name = new StringBuilder();

            switch (aggrType)
            {
                case enum_express_aggr.__ARRAY:
                    name.Append(camelCase ? "Array" : "array");
                    break;

                case enum_express_aggr.__LIST:
                    name.Append(camelCase ? "List" : "list");
                    break;

                case RDF.enum_express_aggr.__SET:
                    name.Append(camelCase ? "Set" : "set");
                    break;

                case RDF.enum_express_aggr.__BAG:
                    name.Append(camelCase ? "Bag" : "bag");
                    break;

                default:
                    Console.WriteLine("unsupported aggregation type " + typeDef.ToString());
                    return null;
            }

            if (camelCase)
                name.Append("Of");
            else
                name.Append("_of_");
            
            name.Append(elemType);

            return name.ToString();
        }

        private Generator.Template GetAggregatedType(TypeDef typeDef, out string elemIfcType, out string elemApiType, out string sdaiType, out string enumValues)
        {
            Generator.Template template = Generator.Template.None;

            elemIfcType = null;
            elemApiType = null;
            sdaiType = null;
            enumValues = null;

            string baseType = null;
            Select select = null;
            Enumeraion enumeration = null;
            DefinedType definedType = null;

            if (typeDef.IsSimpleType(out elemIfcType, out baseType, out sdaiType))
            {
                if (baseType != null)
                {
                    if (generator.m_cs || elemIfcType == null)
                    {
                        elemApiType = baseType;
                    }
                    else
                    {
                        elemApiType = elemIfcType;
                    }

                    if (elemIfcType == null)
                    {
                        elemIfcType = baseType;
                    }

                    if (baseType == "TextValue")
                    {
                        template = Generator.Template.AggregationOfText;
                    }
                    else
                    {
                        template = Generator.Template.AggregationOfSimple;
                    }
                }
                else
                {
                    Console.WriteLine("Unsupported type in aggregations " + elemIfcType);
                }
            }
            else if (typeDef.IsEntityReference(out elemIfcType))
            {
                template = Generator.Template.AggregationOfInstance;
                elemApiType = elemIfcType;
                sdaiType = "sdaiINSTANCE";
            }
            else if ((enumeration = typeDef.IsEnumeration()) != null)
            {
                elemIfcType = enumeration.name;
                elemApiType = elemIfcType;
                sdaiType = "sdaiENUM";
                enumValues = enumeration.name + "_";
                template = Generator.Template.AggregationOfEnum;
            }
            else if (typeDef.domain == 0 && typeDef.attrType == RDF.enum_express_attr_type.__LOGICAL)
            {
                elemIfcType = "LOGICAL_VALUE";
                elemApiType = elemIfcType;
                sdaiType = "sdaiLOGICAL";
                enumValues = "LOGICAL_VALUE_";
                template = Generator.Template.AggregationOfEnum;
            }
            else if ((select = typeDef.IsSelect()) != null)
            {
                template = Generator.Template.AggregationOfSelect;
                elemIfcType = select.name;
                elemApiType = elemIfcType;
                sdaiType = null;
            }
            else if ((definedType = typeDef.IsDefinedType()) != null)
            {
                DefinedType.Foundation foundation = null;
                if (generator.m_writtenDefinedTyes.TryGetValue(definedType.declaration, out foundation))
                {
                    switch (foundation.declarationType)
                    {
                        case enum_express_declaration.__ENTITY:
                            elemIfcType = definedType.name;
                            elemApiType = elemIfcType;
                            //???? or find name of foundation elemApiType = foundation.;
                            sdaiType = "sdaiINSTANCE";
                            template = Generator.Template.AggregationOfInstance;
                            System.Diagnostics.Debug.Assert(false, "not tested");
                            break;
                        case enum_express_declaration.__ENUM:
                            elemIfcType = definedType.name;
                            //find enum name in foundation elemApiType = foundation.;
                            sdaiType = "sdaiENUM";
                            enumValues = elemApiType + "_";
                            template = Generator.Template.AggregationOfEnum;
                            System.Diagnostics.Debug.Assert(false, "not tested");
                            break;
                        case enum_express_declaration.__SELECT:
                            elemIfcType = definedType.name;
                            elemApiType = elemIfcType;
                            sdaiType = "---- sdaiType is not set for SELECT -----";
                            template = Generator.Template.AggregationOfSelect;
                            break;
                        default:
                            switch (foundation.attrType)
                            {
                                case enum_express_attr_type.__LOGICAL:
                                    elemIfcType = definedType.name;
                                    elemApiType = "LOGICAL_VALUE";
                                    sdaiType = "sdaiLOGICAL";
                                    enumValues = "LOGICAL_VALUE_";
                                    template = Generator.Template.AggregationOfEnum;
                                    break;

                                default:
                                    Console.WriteLine("Unexpected foundation type " + foundation.declarationType.ToString() + " in aggregation of " + typeDef.ToString());
                                    System.Diagnostics.Debug.Assert(false);
                                    break;
                            }
                            break;
                    }
                }
            }

            if (template == Generator.Template.None)
            {
                Console.WriteLine("aggregation is not supported: " + typeDef.ToString());
            }

            return template;
        }

        private void WriteAttribute(string attrName, bool isDirect)
        {
            string aggrTypeName = null;
            bool nested = false;
            string elemIfcType = null;
            string elemApiType = null;
            string enumValues = null;
            string sdaiType = null;
            Generator.Template template = Generator.Template.None;

            if (typeDef.aggregation != 0)
            {
                //unnamed aggregation
                template = GetAggregatedType(typeDef, out elemIfcType, out elemApiType, out sdaiType, out enumValues);
                if (template != Generator.Template.None)
                {
                    aggrTypeName = elemIfcType;
                    for (var aggregation = typeDef.aggregation; aggregation != 0;)
                    {
                        enum_express_aggr aggrType;
                        Int64 crdMin, crdMax;
                        bool optional, unique;
                        ifcengine.engiGetAggregationDefinition(aggregation, out aggrType, out crdMin, out crdMax, out optional, out unique, out aggregation);
                        aggrTypeName = MakeAggregationTypeName(aggrType, aggrTypeName);
                        if (aggregation != 0)
                        {
                            elemApiType = aggrTypeName;
                            elemIfcType = aggrTypeName;
                            nested = true;
                        }
                    }
                }
                else
                {
                    Console.WriteLine("Aggregated attribute is not supported " + attrName + ": " + typeDef.ToString());
                }
            }
            else if (RDF.ifcengine.engiGetDeclarationType (typeDef.domain)==RDF.enum_express_declaration.__DEFINED_TYPE)
            {
                if (generator.m_writtenDefinedTyes.ContainsKey(typeDef.domain))
                {
                    var definedType = new DefinedType(typeDef.domain);
                    aggrTypeName = definedType.name;
                    template = GetAggregatedType(definedType, out elemIfcType, out elemApiType, out sdaiType, out enumValues);
                }
                else
                {
                    Console.WriteLine("Aggregated attribute is not supported (defined type is not supported) " + attrName + ": " + typeDef.ToString());
                }
            }
            else
            {
                throw new ApplicationException("unsupported aggregation " + typeDef.ToString());
            }

            if (template != Generator.Template.None)
            {
                if (generator.m_writtenAggregationTypes.Contains(aggrTypeName))
                {
                    generator.m_replacements[Generator.KWD_ATTR_NAME] = attrName;
                    generator.m_replacements[Generator.KWD_AggregationType] = aggrTypeName;
                    generator.m_replacements[Generator.KWD_SimpleType] = elemApiType;
                    generator.m_replacements[Generator.KWD_TextType] = elemApiType;
                    generator.m_replacements[Generator.KWD_REF_ENTITY] = elemApiType;
                    generator.m_replacements[Generator.KWD_TypeNameIFC] = elemIfcType;

                    generator.WriteGetPut(Generator.Template.AttributeAggregationGet, Generator.Template.AttributeAggregationPut, isDirect);
                    if (!nested && sdaiType != null && isDirect)
                    {
                        generator.WriteByTemplate(Generator.Template.AttributeAggregationPutArray);
                    }
                }
            }
        }
    }
}
