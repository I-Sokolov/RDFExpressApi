#pragma warning disable CS1587
#pragma warning disable CS1591

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

using RDF;

using ExpressHandle = System.Int64;

namespace RDFWrappers
{
    public class Select
    {
        public string name;
        public ExpressHandle inst;

        public Select(ExpressHandle inst)
        {
            this.inst = inst;
            name = Schema.GetNameOfDeclaration(inst); 
        }

        private HashSet<ExpressHandle> GetVariants()
        {
            var ret = new HashSet<ExpressHandle>();

            int i = 0;
            ExpressHandle variant;
            while (0 != (variant = ifcengine.engiGetSelectElement(inst, i++)))
            {
                if (!ret.Add(variant))
                {
                    Console.WriteLine(string.Format("duplicated type {0} in SELECT {1}", Schema.GetNameOfDeclaration(variant), name));
                    System.Diagnostics.Debug.Assert(false);
                }
            }

            return ret;
        }

        private List<ExpressHandle> GetNestedSelects(Generator generator)
        {
            var ret = new List<ExpressHandle>();

            int i = 0;
            ExpressHandle variant;
            while (0 != (variant = ifcengine.engiGetSelectElement(inst, i++)))
            {
                var decl = ifcengine.engiGetDeclarationType(variant);

                if (decl == enum_express_declaration.__DEFINED_TYPE)
                {
                    DefinedType.Foundation foundation;
                    if (generator.m_writtenDefinedTyes.TryGetValue (variant, out foundation))
                    {
                        decl = foundation.declarationType;
                    }
                }

                if (decl == enum_express_declaration.__SELECT)
                {
                    ret.Add(variant);
                }
            }

            return ret;
        }

        private HashSet<Generator.Template> CollectAsTypes ()
        {
            var ret = new HashSet<Generator.Template>();

            int i = 0;
            ExpressHandle variant;
            while (0 != (variant = ifcengine.engiGetSelectElement(inst, i++)))
            {
                enum_express_declaration declType = ifcengine.engiGetDeclarationType(variant);

                switch (declType)
                {
                    case enum_express_declaration.__ENTITY:
                        ret.Add(Generator.Template.SelectGetAsEntity);
                        break;

                    case enum_express_declaration.__ENUM:
                        //ret.Add(Generator.Template.SelectGetAsString);
                        break;

                    case enum_express_declaration.__SELECT:
                        var nestedSelect = new Select(variant);
                        foreach (var nestedType in nestedSelect.CollectAsTypes())
                        {
                            ret.Add(nestedType);
                        }
                        break;

                    case enum_express_declaration.__DEFINED_TYPE:
                        var definedType = new DefinedType(variant);
                        if (!definedType.IsAggregation () && definedType.attrType != enum_express_attr_type.__LOGICAL)
                        {
                            var cstype = definedType.GetBaseCSType();
                            if (cstype != null)
                            {
                                switch (cstype)
                                {
                                    case "double": ret.Add(Generator.Template.SelectGetAsDouble); break;
                                    case "IntValue": ret.Add(Generator.Template.SelectGetAsInt); break;
                                    case "bool": ret.Add(Generator.Template.SelectGetAsBool); break;
                                    case "TextValue": ret.Add(Generator.Template.SelectGetAsText); break;
                                    default: throw new ApplicationException("unexpected cs type " + cstype);
                                }
                            }
                        }
                        break;
                }
            }

            return ret;

        }


        private void WriteNestedSelect (Generator generator, ExpressHandle declaration, HashSet<ExpressHandle> visitedSelects)
        {
            var declType = ifcengine.engiGetDeclarationType(declaration);
            switch (declType)
            {
                case enum_express_declaration.__SELECT:
                    (new Select(declaration)).WriteAccessors(generator, visitedSelects);
                    break;

                case enum_express_declaration.__DEFINED_TYPE:
                    var definedType = new DefinedType(declaration);
                    WriteNestedSelect(generator, definedType.domain, visitedSelects);
                    break;

                default:
                    Console.WriteLine("SELECT " + name + " has unexpected mested select of type " + declType.ToString());
                    System.Diagnostics.Debug.Assert(false);
                    break;
            }

        }

        public void WriteAccessors(Generator generator, HashSet<ExpressHandle> visitedSelects)
        {
            if (!visitedSelects.Add(inst))
            {
                return;
            }

            foreach (var nested in GetNestedSelects(generator))
            {
                WriteNestedSelect(generator, nested, visitedSelects);
            }

            generator.m_replacements[Generator.KWD_TYPE_NAME] = Generator.ValidateIdentifier (name);

            foreach (var bGet in new bool?[] { null, true, false })
            {
                generator.m_replacements[Generator.KWD_ACCESSOR] = bGet.HasValue ? (bGet.Value ? "_get" : "_put") : "";

                generator.WriteByTemplate(Generator.Template.SelectAccessorBegin);

                foreach (var variant in GetVariants())
                {
                    if (!bGet.HasValue)
                        generator.m_writer.WriteLine();

                    WriteAccessorMethod(generator, variant, bGet);
                }

                if (bGet.HasValue && bGet.Value) //to implement for detached selects it needs implementation with m_adb
                {
                    var astypes = CollectAsTypes();
                    if (astypes.Count > 0)
                    {
                        generator.m_writer.WriteLine();
                        foreach (var astype in astypes)
                        {
                            generator.WriteByTemplate(astype);
                        }
                    }
                }

                generator.WriteByTemplate(Generator.Template.SelectAccessorEnd);
            }

        }

        private void WriteAccessorMethod(Generator generator, ExpressHandle selectVariant, bool? bGet)
        {
            var type = ifcengine.engiGetDeclarationType(selectVariant);

            switch (type)
            {
                case enum_express_declaration.__DEFINED_TYPE:
                    var definedType = new DefinedType(selectVariant);
                    WriteAccessorMethod(generator, definedType, bGet);
                    break;

                case enum_express_declaration.__SELECT:
                    var selectName = Schema.GetNameOfDeclaration(selectVariant);
                    WriteSelectAccessorMethod(generator, selectName, bGet);
                    break;

                case enum_express_declaration.__ENTITY:
                    var entityType = new Entity(selectVariant);
                    WriteAccessorMethod(generator, entityType, bGet);
                    break;

                case enum_express_declaration.__ENUM:
                    var enumType = new Enumeraion(selectVariant);
                    WriteAccessorMethod(generator, enumType, bGet);
                    break;

                default:
                    System.Diagnostics.Debug.Assert(false);
                    break;
            }
        }

        private void WriteAccessorMethod(Generator generator, Enumeraion enumType, bool? bGet)
        {
            var name = Generator.ValidateIdentifier(enumType.name);
            WriteAccessorEnumMethod(generator, name, name, name + "_", bGet);
        }

        private void WriteAccessorEnumMethod(Generator generator, string ifcTypeName, string enumName, string enumValuesArray, bool? bGet)
        { 
            generator.m_replacements[Generator.KWD_ENUMERATION_NAME] = enumName;
            generator.m_replacements[Generator.KWD_TypeNameIFC] = ifcTypeName;
            generator.m_replacements[Generator.KWD_TypeNameUpper] = ifcTypeName.ToUpper();
            generator.m_replacements[Generator.KWD_ENUMERATION_VALUES_ARRAY] = enumValuesArray;

            if (bGet.HasValue)
            {
                generator.WriteByTemplate(bGet.Value ? Generator.Template.SelectEnumerationGet : Generator.Template.SelectEnumerationPut);
            }
            else
            {
                generator.WriteByTemplate(Generator.Template.SelectEnumerationGet);
                generator.WriteByTemplate(Generator.Template.SelectEnumerationPut);
            }
        }


        private void WriteAccessorMethod(Generator generator, Entity entityType, bool? bGet)
        {
            generator.m_replacements[Generator.KWD_REF_ENTITY] = Generator.ValidateIdentifier (entityType.name);
            generator.m_replacements[Generator.KWD_TypeNameIFC] = entityType.name;
            generator.m_replacements[Generator.KWD_TypeNameUpper] = entityType.name.ToUpper();

            if (bGet.HasValue)
            {
                generator.WriteByTemplate(bGet.Value ? Generator.Template.SelectEntityGet : Generator.Template.SelectEntityPut);

                var impl = generator.StringByTemplate(bGet.Value ? Generator.Template.SelectEntityGetImplementation : Generator.Template.SelectEntityPutImplementation);
                generator.m_implementations.Append(impl);
            }
            else
            {
                generator.WriteByTemplate(Generator.Template.SelectEntityGet);
                generator.WriteByTemplate(Generator.Template.SelectEntityPut);

                var impl = generator.StringByTemplate(Generator.Template.SelectEntityGetImplementation);
                generator.m_implementations.Append(impl);

                impl = generator.StringByTemplate(Generator.Template.SelectEntityPutImplementation);
                generator.m_implementations.Append(impl);
            }
        }


        private void WriteAccessorMethod(Generator generator, DefinedType definedType, bool? bGet)
        {
            DefinedType.Foundation foundation = null;
            if (!generator.m_writtenDefinedTyes.TryGetValue(definedType.declaration, out foundation))
            {
                Console.WriteLine("SLECT " + name + " - DefinedType is not supported: " + definedType.name);
                return; //>>>>>>>>>>>>>>>>>>
            }

            if (foundation.aggrType != enum_express_aggr.__NONE)
            {
                WriteAggrAccessorMethod(generator, definedType, foundation, bGet);
            }
            else
            {
                switch (foundation.declarationType)
                {
                    case enum_express_declaration.__UNDEF: //based on primitive
                        switch (foundation.attrType)
                        {
                            case enum_express_attr_type.__LOGICAL:
                                WriteAccessorEnumMethod(generator, definedType.name, "LOGICAL_VALUE", "LOGICAL_VALUE_", bGet);
                                break;

                            default:
                                WriteSimpleAccessorMethod(generator, definedType, bGet);
                                break;
                        }
                        break;

                    case enum_express_declaration.__SELECT:
                        WriteSelectAccessorMethod(generator, definedType.name, bGet);
                        break;

                    default:
                        Console.WriteLine("SLECT " + name + " - DefinedType " + definedType.name + " is " + foundation.declarationType.ToString());
                        break;
                }
            }
        }

        private void WriteSimpleAccessorMethod(Generator generator, DefinedType definedType, bool? bGet)
        {
            string sdaiType = definedType.GetSdaiType();
            string baseType = definedType.GetBaseCSType();

            generator.m_replacements[Generator.KWD_SimpleType] = definedType.name;
            generator.m_replacements[Generator.KWD_TextType] = definedType.name;
            generator.m_replacements[Generator.KWD_sdaiTYPE] = sdaiType;
            generator.m_replacements[Generator.KWD_TypeNameIFC] = definedType.name;
            generator.m_replacements[Generator.KWD_TypeNameUpper] = definedType.name.ToUpper();
            generator.m_replacements[Generator.KWD_BaseCType] = baseType;

            Generator.Template tplGet;
            Generator.Template tplPut;
            if (baseType == "TextValue")
            {
                tplGet = Generator.Template.SelectTextGet;
                tplPut = Generator.Template.SelectTextPut;
            }
            else 
            {
                tplGet = Generator.Template.SelectSimpleGet;
                tplPut = Generator.Template.SelectSimplePut;
            }

            if (bGet.HasValue)
            {
                generator.WriteByTemplate(bGet.Value ? tplGet : tplPut);
            }
            else
            {
                generator.WriteByTemplate(tplGet);
                generator.WriteByTemplate(tplPut);
            }
        }

        private void WriteAggrAccessorMethod(Generator generator, DefinedType definedType, DefinedType.Foundation foundation, bool? bGet)
        {                
            string sdaiType = definedType.GetSdaiType();
            string baseType = definedType.GetBaseCSType();

            string elemType = baseType;
            if (definedType.domain != 0)
            {
                if (!generator.m_cs || ifcengine.engiGetDeclarationType(definedType.domain) != enum_express_declaration.__DEFINED_TYPE)
                {
                    elemType = Schema.GetNameOfDeclaration(definedType.domain);
                }
            }

            generator.m_replacements[Generator.KWD_AggregationType] = definedType.name;
            generator.m_replacements[Generator.KWD_SimpleType] = elemType;
            generator.m_replacements[Generator.KWD_TextType] = elemType;
            generator.m_replacements[Generator.KWD_REF_ENTITY] = elemType;
            generator.m_replacements[Generator.KWD_sdaiTYPE] = sdaiType;
            generator.m_replacements[Generator.KWD_TypeNameIFC] = definedType.name;
            generator.m_replacements[Generator.KWD_TypeNameUpper] = definedType.name.ToUpper();

            if (bGet.HasValue)
            {
                var tpl = bGet.Value ? Generator.Template.SelectAggregationGet : Generator.Template.SelectAggregationPut;
                generator.WriteByTemplate(tpl);
            }
            else
            {
                generator.WriteByTemplate(Generator.Template.SelectAggregationGet);
                generator.WriteByTemplate(Generator.Template.SelectAggregationPut);
            }

            //Put array methods
            if (!(bGet.HasValue && bGet.Value))
            {
                enum_express_aggr aggr;
                Int64 crdMin, crdMax, nestedAggr;
                byte optional, unique;
                ifcengine.engiGetAggregation(definedType.aggregation, out aggr, out crdMin, out crdMax, out optional, out unique, out nestedAggr);
                System.Diagnostics.Debug.Assert(nestedAggr == 0); //to test
                if (nestedAggr == 0 && sdaiType != null)
                {
                    var tpl = Generator.Template.SelectAggregationPutArray;
                    generator.WriteByTemplate(tpl);
                }
            }
        }

        private void WriteSelectAccessorMethod(Generator generator, string selectName, bool? bGet)
        {
            var saveSelect = generator.m_replacements[Generator.KWD_TYPE_NAME];
            generator.m_replacements[Generator.KWD_TYPE_NAME] = selectName;

            generator.m_replacements[Generator.KWD_nestedSelectAccess] = bGet.HasValue ? (bGet.Value ? "get" : "put") : "";

            generator.WriteByTemplate(Generator.Template.SelectNested);

            generator.m_replacements[Generator.KWD_TYPE_NAME] = saveSelect;
        }


        public override string ToString()
        {
            var str = new StringBuilder();

            str.AppendLine(string.Format("{0}:", name));

            foreach (var variant in GetVariants())
            {
                var name = Schema.GetNameOfDeclaration(variant);
                var type = ifcengine.engiGetDeclarationType(variant);

                str.AppendLine(string.Format("        {0} {1}", name, type.ToString()));
            }

            return str.ToString();

        }

    }
}
