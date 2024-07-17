#pragma warning disable CS1587
#pragma warning disable CS1573
#pragma warning disable CS1591

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ExpressHandle = System.Int64;

namespace RDFWrappers
{
    public class Generator
    {
        /// <summary>
        /// 
        /// </summary>
        const string KWD_PREPROC = "//##";
        const string KWD_IGNORE = "IGNORE";

        public const string KWD_NAMESPACE = "NAMESPACE_NAME";
        public const string KWD_ENTITY_NAME = "ENTITY_NAME";
        public const string KWD_BASE_CLASS = "/*PARENT_NAME*/Entity";
        public const string KWD_DEFINED_TYPE = "DEFINED_TYPE_NAME";
        public const string KWD_BaseCType = "BaseCType";
        public const string KWD_SimpleType = "SimpleType";
        public const string KWD_TextType = "TextType";
        public const string KWD_TypeNameIFC = "TypeNameIFC";
        public const string KWD_TypeNameUpper = "TypeNameUpper";
        public const string KWD_ENUMERATION_NAME = "ENUMERATION_NAME";
        public const string KWD_ENUMERATION_VALUES_ARRAY = "ENUMERATION_VALUES_ARRAY";
        public const string KWD_ENUMERATION_ELEMENT = "ENUMERATION_ELEMENT";
        public const string KWD_NUMBER = "1234";
        public const string KWD_ATTR_NAME = "ATTR_NAME";
        public const string KWD_sdaiTYPE = "sdaiTYPE";
        public const string KWD_asType = "asTYPE";
        public const string KWD_REF_ENTITY = "REF_ENTITY";
        public const string KWD_ENUM_VALUES = "\"ENUMERATION_STRING_VALUES\"";
        public const string KWD_TYPE_NAME = "GEN_TYPE_NAME";
        public const string KWD_ACCESSOR = "_accessor";
        public const string KWD_nestedSelectAccess = "nestedSelectAccess";
        public const string KWD_GETPUT = "getOrPut";
        public const string KWD_AggregationType = "AggregationType";

        /// <summary>
        /// 
        /// </summary>
        public enum Template
        { 
            None,
            BeginFile,
            ClassForwardDeclaration,
            DefinedTypesBegin,
            DefinedTypeSimple,
            DefinedTypeEntity,
            DefinedTypeEnum,
            DefinedTypeSelect,
            EnumerationsBegin,
            EnumerationBegin,
            EnumerationElement,
            EnumerationEnd,
            EnumerationNamesBegin,
            EnumerationNames,
            EnumerationsEnd,
            AggregationTypesBegin,
            AggregationOfSimple,
            AggregationOfText,
            AggregationOfInstance,
            AggregationOfEnum,
            AggregationOfAggregation,
            AggregationOfSelect,
            SelectsBegin,
            SelectAccessorBegin,
            SelectSimpleGet,
            SelectSimplePut,
            SelectTextGet,
            SelectTextPut,
            SelectEntityGet,
            SelectEntityPut,
            SelectEnumerationGet,
            SelectEnumerationPut,
            SelectAggregationGet,
            SelectAggregationPut,
            SelectAggregationPutArray,
            SelectNested,
            SelectGetAsDouble,
            SelectGetAsInt,
            SelectGetAsBool,
            SelectGetAsText,
            SelectGetAsEntity,
            SelectAccessorEnd,
            EntitiesBegin,
            EntityBegin,
            EntityCreateMethod,
            AttributeSimpleGet,
            AttributeSimplePut,
            AttributeTextGet,
            AttributeTextPut,
            AttributeEntityGet,
            AttributeEntityPut,
            AttributeEnumGet,
            AttributeEnumPut,
            AttributeSelectAccessor,
            AttributeSelectAccessorProp,
            AttributeAggregationGet,
            AttributeAggregationPut,
            AttributeAggregationPutArray,
            EntityEnd,
            SelectEntityGetImplementation,
            SelectEntityPutImplementation,
            AttributeEntityGetImplementation,
            AttributeEntityPutImplementation,
            EndFile
        }

        //
        public StringBuilder m_implementations = new StringBuilder ();

        /// <summary>
        /// 
        /// </summary>
        public bool m_cs; //cs or cpp
        string m_namespace;

        public Schema m_schema;

        public StreamWriter m_writer;

        public Dictionary<ExpressHandle, DefinedType.Foundation> m_writtenDefinedTyes = new Dictionary<ExpressHandle, DefinedType.Foundation>();

        public HashSet<string> m_writtenAggregationTypes = new HashSet<string>();

        Dictionary<Template, string> m_template = new Dictionary<Template, string>();

        public Dictionary<string, string> m_replacements = new Dictionary<string, string>();

        /// <summary>
        /// 
        /// </summary>
        /// <param name="schema"></param>
        /// <param name="cs"></param>
        public Generator (Schema schema, bool cs, string namespace_)
        {
            m_cs = cs;            
            m_namespace = namespace_;

            m_schema = schema;

            foreach (var template in Enum.GetValues(Template.BeginFile.GetType ()))
            {
                m_template.Add((Template)template, "");
            }

            ParseTemplate();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="outputFile"></param>
        public void WriteWrapper(string outputFile)
        {
            using (m_writer = new StreamWriter(outputFile))
            {
                m_replacements[KWD_NAMESPACE] = m_namespace;

                WriteByTemplate(Template.BeginFile);

                WriteForwardDeclarations();

                WriteEnumerations();

                WriteDefinedTypes();

                WriteSelects();

                Aggregation.WriteAttributesTypes(this);

                WriteEntities();

                m_writer.Write (m_implementations);

                WriteByTemplate(Template.EndFile);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteForwardDeclarations()
        {
            foreach (var cls in m_schema.m_declarations[RDF.enum_express_declaration.__ENTITY])
            {
                m_replacements[KWD_ENTITY_NAME] = ValidateIdentifier (cls.Key);
                WriteByTemplate(Template.ClassForwardDeclaration);
            }

            m_writer.WriteLine();

            foreach (var cls in m_schema.m_declarations[RDF.enum_express_declaration.__SELECT])
            {
                string name = ValidateIdentifier(cls.Key);

                m_replacements[KWD_ENTITY_NAME] = name;
                WriteByTemplate(Template.ClassForwardDeclaration);

                m_replacements[KWD_ENTITY_NAME] = name + "_get";
                WriteByTemplate(Template.ClassForwardDeclaration);

                m_replacements[KWD_ENTITY_NAME] = name + "_put";
                WriteByTemplate(Template.ClassForwardDeclaration);
            }

        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteDefinedTypes()
        {
            WriteByTemplate(Template.DefinedTypesBegin);

            var visitedTypes = new HashSet<ExpressHandle>();

            foreach (var decl in m_schema.m_declarations[RDF.enum_express_declaration.__DEFINED_TYPE])
            {
                var type = new DefinedType(decl.Value);
                type.WriteType(this, visitedTypes);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteEnumerations()
        {
            WriteByTemplate(Template.EnumerationsBegin);

            foreach (var decl in m_schema.m_declarations[RDF.enum_express_declaration.__ENUM])
            {
                var enumeration = new Enumeraion(decl.Value);
                WriteEnumerationClass(enumeration);
            }

            WriteByTemplate(Template.EnumerationNamesBegin);

            foreach (var decl in m_schema.m_declarations[RDF.enum_express_declaration.__ENUM])
            {
                var enumeration = new Enumeraion(decl.Value);
                WriteEnumerationNames(enumeration);
            }

            WriteByTemplate(Template.EnumerationsEnd);
        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteEnumerationClass(Enumeraion enumeraion)
        {
            m_replacements[KWD_ENUMERATION_NAME] = enumeraion.name;

            WriteByTemplate(Template.EnumerationBegin);

            int i = 0;
            foreach (var _e in enumeraion.GetValues())
            {               
                string e = ValidateIdentifier (_e);

                m_replacements[KWD_ENUMERATION_ELEMENT] = e;
                m_replacements[KWD_NUMBER] = i.ToString();

                WriteByTemplate(Template.EnumerationElement);

                i++;
            }

            WriteByTemplate(Template.EnumerationEnd);
        }

        private void WriteEnumerationNames(Enumeraion enumeraion)
        {
            m_replacements[KWD_ENUMERATION_NAME] = enumeraion.name;

            var values = new StringBuilder();
            bool first = true;
            foreach (var e in enumeraion.GetValues())
            {
                if (first)
                    first = false;
                else
                    values.Append(", ");

                values.Append("\"");
                values.Append(e);
                values.Append("\"");
            }

            m_replacements[KWD_ENUM_VALUES] = values.ToString();

            WriteByTemplate(Template.EnumerationNames);
        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteSelects ()
        {
            var visitedSelects = new HashSet<ExpressHandle>();

            foreach (var decl in m_schema.m_declarations[RDF.enum_express_declaration.__SELECT])
            {
                var sel = new Select(decl.Value);
                sel.WriteAccessors(this, visitedSelects);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteEntities ()
        {
            WriteByTemplate(Template.EntitiesBegin);

            HashSet<ExpressHandle> wroteEntities = new HashSet<ExpressHandle>();

            foreach (var decl in m_schema.m_declarations[RDF.enum_express_declaration.__ENTITY])
            {
                var entity = new Entity(decl.Value);
                WriteEntity (entity, wroteEntities);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteEntity(Entity entity, HashSet<ExpressHandle> wroteEntities)
        {
            if (!wroteEntities.Add(entity.inst))
            {
                return;
            }

            var superTypes = entity.GetSupertypes();

            //
            // Gather base classes and their properties

            if (m_cs) //C# allows one base class only
            {
                if (superTypes.Count > 1)
                {
                    var parentId = superTypes.FirstOrDefault();
                    superTypes.Clear();
                    if (parentId != 0)
                    {
                        superTypes.Add(parentId);
                    }
                }
            }


            string baseClass = "";
            var parentAttributes = new HashSet<string>();

            foreach (var parentId in superTypes)
            {
                var parent = new Entity(parentId);

                WriteEntity(parent, wroteEntities);

                if (baseClass.Length != 0)
                {
                    baseClass += ", public virtual ";
                }
                baseClass += ValidateIdentifier(parent.name);

                GetAttributeNames(parentAttributes, parent);
            }

            if (baseClass.Length == 0)
            {
                baseClass = "Entity";
            }

            //
            // Write this entity

            string clsName = ValidateIdentifier(entity.name);
            m_replacements[KWD_BASE_CLASS] = baseClass;
            m_replacements[KWD_ENTITY_NAME] = clsName;

            //
            WriteByTemplate(Template.EntityBegin);

            if (!entity.IsAbstract())
            {
                WriteByTemplate(Template.EntityCreateMethod);
            }
                
            WriteAttributes(entity, parentAttributes);

            WriteByTemplate(Template.EntityEnd);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="attributes"></param>
        /// <param name="entity"></param>
        private void GetAttributeNames(HashSet<string> attributes, Entity entity)
        {
            var attribs =  entity.GetAttributes();
            foreach (var a in attribs)
            {
                attributes.Add(a.name);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        public void WriteByTemplate(Template template)
        {
            string str = StringByTemplate(template);
            m_writer.Write(str);
        }

        public string StringByTemplate (Template template)
        {
            string code = m_template[template];

            foreach (var r in m_replacements)
            {
                //if (r.Key.Equals(KWD_asType))
                {
                    code = code.Replace(r.Key, r.Value, true, null); //ignore case
                }
                /*else 
                {
                    code = code.Replace(r.Key, r.Value);
                }*/
            }

            if (m_cs)
            {
                code = code.Replace("string?", "string"); //warning CS8632: The annotation for nullable reference types should only be used in code within a '#nullable' annotations context.
            }

            return code;
        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteAttributes(Entity entity, HashSet<string> exportedAttributes)
        {
            var attribs = entity.GetAttributes();
            foreach (var attr in attribs)
            {
                m_replacements[KWD_ATTR_NAME] =  ValidateIdentifier (attr.name);

                if (exportedAttributes.Add(attr.name))
                {
                    if (!attr.IsAggregation ())
                    {
                        WriteSingeAttribute(attr);
                    }
                    else
                    {
                        Aggregation.WriteAttribute(this, attr);
                    }
                }
            }
        }

        /// <summary>
        /// 
        /// </summary>
        private void WriteSingeAttribute(Attribute attr)
        {
            string expressType = null;
            string baseType = null;
            string sdaiType = null;
            if (attr.IsSimpleType(out expressType, out baseType, out sdaiType))
            {
                WriteSimpleAttribute(attr, expressType, baseType, sdaiType);
                return;
            }

            DefinedType definedType = null;
            Enumeraion enumeration = null;
            Select select = null;
            string referncedEntity = null;

            if ((definedType = attr.IsDefinedType()) != null)
            {
                definedType.WriteSingleAttribute(this, attr);
            }
            else if (attr.IsEntityReference(out referncedEntity))
            {
                WriteEntityReference(attr, referncedEntity);
            }
            else if ((enumeration = attr.IsEnumeration()) != null)
            {
                WriteEnumAttribute(attr, enumeration.name, enumeration.name, enumeration.name + "_");
            }
            else if ((select = attr.IsSelect()) != null)
            {
                WriteSelectAttribute(attr, select.name);
            }
            else if (attr.domain == 0 && attr.attrType == RDF.enum_express_attr_type.__LOGICAL)
            {
                WriteEnumAttribute(attr, "LOGICAL_VALUE", "LOGICAL_VALUE", "LOGICAL_VALUE_");
            }
            else
            {
                Console.WriteLine("Attribute is not supported: " + attr.ToString());
            }
        }

        private void WriteSimpleAttribute(Attribute attr, string definedType, string baseType, string sdaiType)
        {
            m_replacements[KWD_SimpleType] = (definedType !=null) ? definedType : baseType;
            m_replacements[KWD_TextType] = m_replacements[KWD_SimpleType]; //just different words in template
            m_replacements[KWD_sdaiTYPE] = sdaiType;
            m_replacements[KWD_BaseCType] = baseType;

            Template tplGet = baseType == "TextValue" ? Template.AttributeTextGet : Template.AttributeSimpleGet;
            Template tplPut = baseType == "TextValue" ? Template.AttributeTextPut : Template.AttributeSimplePut;

            WriteGetPut(tplGet, tplPut, attr.inverse);
        }

        public void WriteGetPut(Template tplGet, Template tplPut, bool readOnly)
        {
            var str = BuildGetPut(tplGet, tplPut, readOnly);
            m_writer.Write(str);
        }

        private string BuildGetPut (Template tplGet, Template tplPut, bool readOnly)
        {
            bool get = readOnly || !m_cs;
            bool put = !readOnly;

            StringBuilder str = new StringBuilder();

            if (get)
                {
                string s = StringByTemplate(tplGet);
                str.Append(s);
                }

            if (put)
                {
                string s = StringByTemplate(tplPut);
                str.Append(s);
                }

            return str.ToString();
        }


        private void WriteEntityReference (Attribute attr, string domain)
        {
            m_replacements[KWD_REF_ENTITY] = domain;

            WriteGetPut(Template.AttributeEntityGet, Template.AttributeEntityPut, attr.inverse);

            var impl = BuildGetPut(Template.AttributeEntityGetImplementation, Template.AttributeEntityPutImplementation, attr.inverse);
            m_implementations.Append(impl);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="attr"></param>
        /// <param name="enumName"></param>
        /// <param name="valuesArrayName"></param>
        public void WriteEnumAttribute(Attribute attr, string ifcTypeName, string enumName, string valuesArrayName)
        {
            m_replacements[Generator.KWD_ENUMERATION_NAME] = enumName;
            m_replacements[Generator.KWD_TypeNameIFC] = ifcTypeName;
            m_replacements[Generator.KWD_TypeNameUpper] = ifcTypeName.ToUpper();
            m_replacements[Generator.KWD_ENUMERATION_VALUES_ARRAY] = valuesArrayName;
            WriteGetPut(Template.AttributeEnumGet, Template.AttributeEnumPut, attr.inverse);
        }

        /// <summary>
        /// 
        /// </summary>
        public void WriteSelectAttribute(Attribute attr, string selectName)
        {
            Generator generator = this;

            generator.m_writer.WriteLine();

            generator.m_replacements[Generator.KWD_TYPE_NAME] = Generator.ValidateIdentifier(selectName);

            generator.m_replacements[Generator.KWD_GETPUT] = "get";
            generator.m_replacements[Generator.KWD_ACCESSOR] = "_get";
            generator.WriteByTemplate(Generator.Template.AttributeSelectAccessor);
            generator.WriteByTemplate(Generator.Template.AttributeSelectAccessorProp);

            if (!attr.inverse)
            {
                generator.m_replacements[Generator.KWD_GETPUT] = "put";
                generator.m_replacements[Generator.KWD_ACCESSOR] = "_put";
                generator.WriteByTemplate(Generator.Template.AttributeSelectAccessor);
            }

            generator.m_replacements.Remove(Generator.KWD_GETPUT);
            generator.m_replacements.Remove(Generator.KWD_ACCESSOR);
        }


        /// <summary>
        /// 
        /// </summary>
        private void ParseTemplate()
        {
            System.Reflection.Assembly assembly = System.Reflection.Assembly.GetCallingAssembly();

            var templateName = m_cs ? "EngineEx_Template.cs" : "EngineEx_Template.h";

            string resourceName = FormatResourceName(assembly, templateName);

            using (Stream stream = assembly.GetManifestResourceStream(resourceName))
            {
                Verify(stream != null, "Failed get resource " + resourceName);
                using (StreamReader reader = new StreamReader(stream)) //to read from file use new StreamReader(templateFile))
                {
                    Verify(reader != null, "Failed create stream reader for " + templateName);
                    Template part = Template.BeginFile;
                    int nline = 0;
                    string line;
                    while ((line = reader.ReadLine()) != null)
                    {
                        nline++;


                        if (line.Contains(KWD_PREPROC))
                        {
                            if (line.Contains(KWD_IGNORE))
                            {
                                continue;
                            }
                            else
                            {
                                part++;
                                Verify(line.Contains(part.ToString()), string.Format("Expected line {0} contains '{1}' substing while parsing template fle {2}", nline, part.ToString(), templateName));
                            }
                        }
                        else
                        {
                            string str = m_template[part];
                            str = str + line + "\r\n";
                            m_template[part] = str;
                        }
                    }
                }
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="assembly"></param>
        /// <param name="resourceName"></param>
        /// <returns></returns>
        private static string FormatResourceName(System.Reflection.Assembly assembly, string resourceName)
        {
            return assembly.GetName().Name + "." + resourceName.Replace(" ", "_").Replace("\\", ".").Replace("/", ".");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="condition"></param>
        /// <param name="exceptionMsg"></param>
        public static void Verify (bool condition, string exceptionMsg)
        {
            System.Diagnostics.Trace.Assert(condition);
            if (!condition)
            {
                throw new ApplicationException(exceptionMsg);
            }
        }


        /// <summary>
        /// 
        /// </summary>
        private static HashSet<string> m_additionalKeywords = new HashSet<string>();

        private static void InitAdditionalKeywords ()
        {
            m_additionalKeywords.Add("NULL");
            m_additionalKeywords.Add("CONST");
            m_additionalKeywords.Add("union");
        }

        public static string ValidateIdentifier(string name)
        {
            if (m_additionalKeywords.Count == 0)
            {
                InitAdditionalKeywords();
            }

            string rename = name;

            using (var codeProvider = System.CodeDom.Compiler.CodeDomProvider.CreateProvider("C#"))
            {
                if (!codeProvider.IsValidIdentifier(rename))
                {
                    //does not work as expected, var id = codeProvider.CreateValidIdentifier(name);
                    bool first = true;
                    var builder = new System.Text.StringBuilder();
                    foreach (var ch in rename)
                    {
                        if (first && char.IsLetter(ch) || !first && char.IsLetterOrDigit(ch) || ch=='_')
                        {
                            //fits
                            builder.Append(ch);
                        }
                        else
                        {
                            //replace
                            builder.Append("_");
                            builder.Append(((byte)ch).ToString());
                            builder.Append("_");
                        }

                        first = false;
                    }

                    rename = builder.ToString();

                    if (!codeProvider.IsValidIdentifier(rename))
                    {
                        rename += "_";
                        if (!codeProvider.IsValidIdentifier(rename))
                        {
                            throw new ApplicationException("Can not make a valid identifier from '" + name + "'");
                        }
                    }
                }
            }

            if (m_additionalKeywords.Contains (rename))
            {
                rename = rename + "_";
            }

            if (rename != name)
            {
                //Console.WriteLine("!!!  '{0}' is not a valid identifier, changed to '{1}'", name, rename);
            }

            return rename;
        }
    }
}
