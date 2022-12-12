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
    public class Schema
    {
        /// <summary>
        /// 
        /// </summary>

        static public string GetNameOfDeclaration(ExpressHandle clsid)
        {
            if (clsid!=0)
            {
                var ptrName = IntPtr.Zero;
                ifcengine.engiGetEntityName(clsid, ifcengine.sdaiSTRING, out ptrName);
                var name = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ptrName);

                return name;
            }
            else
            {
                return "";
            }
        }

        static public string GetPrimitiveType(enum_express_attr_type attr_Type)
        {
            switch (attr_Type)
            {
                case enum_express_attr_type.__BOOLEAN:
                    return "bool";

                case enum_express_attr_type.__INTEGER:
                    return "IntValue";

                case enum_express_attr_type.__NUMBER:
                    return "double";

                case enum_express_attr_type.__REAL:
                    return "double";

                case enum_express_attr_type.__STRING:
                case enum_express_attr_type.__BINARY:
                case enum_express_attr_type.__BINARY_32:
                    return "TextValue";

                case enum_express_attr_type.__NONE:
                case enum_express_attr_type.__LOGICAL:
                case enum_express_attr_type.__ENUMERATION:
                case enum_express_attr_type.__SELECT:
                    return null;

                default:
                    System.Diagnostics.Debug.Assert(false);
                    return null;
            }
        }

        static public string GetSdaiType(enum_express_attr_type attr_Type)
        {
            switch (attr_Type)
            {
                case enum_express_attr_type.__BOOLEAN:
                    return "sdaiBOOLEAN";

                case enum_express_attr_type.__INTEGER:
                    return "sdaiINTEGER";

                case enum_express_attr_type.__LOGICAL:
                    return "sdaiLOGICAL";

                case enum_express_attr_type.__NUMBER:
                case enum_express_attr_type.__REAL:
                    return "sdaiREAL";

                case enum_express_attr_type.__STRING:
                    return "sdaiSTRING";

                case enum_express_attr_type.__BINARY:
                case enum_express_attr_type.__BINARY_32:
                    return "sdaiBINARY";

                case enum_express_attr_type.__NONE:
                case enum_express_attr_type.__ENUMERATION:
                case enum_express_attr_type.__SELECT:
                default:
                    System.Diagnostics.Debug.Assert(false);
                    return null;
            }
        }

        public class DefinitionsList : SortedList<string, ExpressHandle> { }

        public class Definitions : Dictionary <enum_express_declaration, DefinitionsList> { }

        /// <summary>
        /// 
        /// </summary>
        public Definitions m_declarations = new Definitions();

        private Int64 m_model = 0;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="schemaName"></param>
        public Schema (string schemaName)
        {
            m_model = ifcengine.sdaiCreateModelBN(1, "", schemaName);

            m_declarations.Add(enum_express_declaration.__DEFINED_TYPE, new DefinitionsList());
            m_declarations.Add(enum_express_declaration.__ENUM, new DefinitionsList());
            m_declarations.Add(enum_express_declaration.__SELECT, new DefinitionsList());
            m_declarations.Add(enum_express_declaration.__ENTITY, new DefinitionsList());

            CollectDefinitions();
        }

        /// <summary>
        /// 
        /// </summary>
        private void CollectDefinitions()
        {
            Int64 it = 0;
            while (0 != (it = ifcengine.engiGetNextDeclarationIterator(m_model, it)))
            {
                ExpressHandle defenition = ifcengine.engiGetDeclarationFromIterator(m_model, it);

                var name = GetNameOfDeclaration(defenition);
                var type = ifcengine.engiGetDeclarationType(defenition);

                var defs = m_declarations[type];
                defs.Add(name, defenition);
            }
        }

        public void ToConsole()
        {
            Console.WriteLine("-------- Extracted shcema ----------------");

            Console.WriteLine("============= Defined Types ====================");
            var definedTypes = m_declarations[enum_express_declaration.__DEFINED_TYPE];
            if (definedTypes != null)
                foreach (var def in definedTypes)
                {
                    var type = new DefinedType(def.Value);
                    Console.WriteLine (type.ToString());
                }

            Console.WriteLine("============= Enumerations ====================");
            var enums = m_declarations[enum_express_declaration.__ENUM];
            if (enums != null)
                foreach (var enm in enums)
                {
                    var e = new Enumeraion(enm.Value);
                    Console.WriteLine (e.ToString());
                }

            Console.WriteLine("============= Selects ====================");
            var sels = m_declarations[enum_express_declaration.__SELECT];
            if (sels != null)
                foreach (var sel in sels)
                {
                    var s = new Select(sel.Value); ;
                    Console.WriteLine(s.ToString());
                }

            Console.WriteLine("============= Entities ====================");
            var entities = m_declarations[enum_express_declaration.__ENTITY];
            if (entities != null)
                foreach (var entity in entities)
                {
                    var e = new Entity(entity.Value); ;
                    Console.WriteLine(e.ToString());
                }
        }
    
    }
}
