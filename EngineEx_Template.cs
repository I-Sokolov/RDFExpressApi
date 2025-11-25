//
// Early-binding C# API for SDAI (CE wrappers)
//

#pragma warning disable CS1587
#pragma warning disable CS1591

using System;
using System.Diagnostics;
using System.Collections;
using System.Collections.Generic;
using RDF;

#if _WIN64
        using int_t = System.Int64;
#else
		using int_t = System.Int32;
#endif

namespace NAMESPACE_NAME
    {
    using SdaiModel = int_t;
    using SdaiInstance = int_t;
    using SdaiAggr = int_t;

    using IntValue = int_t;
    using TextValue = String;
    using SimpleType = Double; //## IGNORE
    using BaseCType = Double; //## IGNORE
    public class SIMpleType : List<SimpleType> { }  //## IGNORE
    public class REF_ENTITY : Entity { public REF_ENTITY(SdaiInstance ints) : base(0) { } public REF_ENTITY() : base(0) { } protected override string WrapperEntityName() { return null; } } //## IGNORE

    public class SImpleType : Select                        //##IGNORE
        {                                                       //##IGNORE
        public SImpleType() : base(null) { }                //##IGNORE
        public SImpleType(Select outer) : base(outer) { }   //## IGNORE
        public SImpleType(SdaiInstance instance, TextValue attrName = null, IntValue adb = 0) : base(instance, attrName, adb) { }//##IGNORE
        }                                                       //##IGNORE
    /// <summary>
    /// 
    /// </summary>
    class EnumValue<TEnum> where TEnum : struct, Enum
        {
        static public TEnum? FromIndex(int index)
            {
            var values = System.Enum.GetValues<TEnum>();
            if (index >= 0 && index < values.Length)
                {
                return values[index];
                }
            else
                {
                return null;
                }
            }
        }

    class EnumIndex
        {
        static public int FromString(TextValue value, TextValue[] allStrings)
            {
            for (int i = 0; i < allStrings.Length; i++)
                {
                if (value == allStrings[i])
                    return i;
                }
            return -1;
            }
        }

    class EnumString<TEnum> where TEnum : struct, Enum, IComparable
        {
        public static TextValue FromValue(TEnum value, TextValue[] allStrings)
            {
            var values = System.Enum.GetValues<TEnum>();

            for (int i = 0; i < values.Length; i++)
                {
                if (values[i].Equals(value))
                    {
                    if (i < allStrings.Length)
                        {
                        return allStrings[i];
                        }
                    else
                        {
                        Debug.Assert(false);
                        return null;
                        }
                    }
                }

            Debug.Assert(false);
            return null;
            }
        }

    /// <summary>
    /// Helper class to handle and access SELECT instance data
    /// </summary>
    public class Select
        {
        protected SdaiInstance m_instance;
        protected TextValue m_attrName;

        private IntValue m_adb;
        private Select m_outerSelect;

        public IntValue ADB()
            {
            if (m_outerSelect != null)
                {
                return m_outerSelect.ADB();
                }

            if (m_adb == 0 && m_instance != 0 && m_attrName != null)
                {
                if (0 == ifcengine.sdaiGetAttrBN(m_instance, m_attrName, ifcengine.sdaiADB, out m_adb))
                    {
                    ifcengine.sdaiDeleteADB(m_adb);
                    m_adb = 0;
                    }
                }

            return m_adb;
            }

        protected Select(SdaiInstance instance, TextValue attrName = null, IntValue adb = 0)
            {
            Init(instance, attrName, adb);
            }

        protected Select(Select outer)
            {
            m_instance = 0;
            m_attrName = null;
            m_adb = 0;
            m_outerSelect = outer;
            if (m_outerSelect != null)
                {
                m_instance = m_outerSelect.m_instance;
                }
            }

        public void Init(SdaiInstance instance, TextValue attrName = null, IntValue adb = 0)
            {
            Debug.Assert(instance != 0);
            m_instance = instance;
            m_attrName = attrName;
            m_adb = adb;
            m_outerSelect = null;
            }

        protected void SetADB(IntValue adb)
            {
            if (m_outerSelect != null)
                {
                m_outerSelect.SetADB(adb);
                }
            else
                {
                //???sdaiDeleteADB(m_adb);
                m_adb = adb;

                if (m_instance != 0 && m_attrName != null)
                    {
                    ifcengine.sdaiPutAttrBN(m_instance, m_attrName, ifcengine.sdaiADB, m_adb);
                    }
                }
            }

        private bool CheckADBType(IntValue adb, TextValue typeName)
            {
            if (adb == 0)
                {
                return false;
                }

            if (typeName == null)
                {
                return true; //any suitable
                }

            var pPath = ifcengine.sdaiGetADBTypePath(adb, 0);
            var path = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(pPath);

            return path != null && path == typeName;
            }

        public TextValue TypePath
            { get
                {
                var adb = ADB();
                if (adb == 0)
                    {
                    return null;
                    }
                else
                    {
                    return ifcengine.sdaiGetADBTypePathx(adb, 0);
                    }
                }
            }

        //
        //
        protected BaseCType? get_BaseCType(TextValue typeName, IntValue sdaiType) { return null; } //## IGNORE
        protected void put_BaseCType(TextValue typeName, IntValue sdaiType, BaseCType? value) { } //## IGNORE
        protected IntValue? get_IntValue(TextValue typeName, IntValue sdaiType)
            {
            IntValue? ret = null;
            var adb = ADB();
            if (CheckADBType(adb, typeName))
                {
                IntValue val = 0;
                if (ifcengine.sdaiGetADBValue(adb, sdaiType, out val) != 0)
                    {
                    ret = val;
                    }
                }
            return ret;
            }

        protected void put_IntValue(TextValue typeName, IntValue sdaiType, IntValue? value)
            {
            if (value.HasValue)
                {
                IntValue v = value.Value;
                var adb = ifcengine.sdaiCreateADB(sdaiType, ref v);
                ifcengine.sdaiPutADBTypePath(adb, 1, typeName);
                SetADB(adb);
                }
            else
                {
                Debug.Assert(false);
                }
            }

        //
        protected double? get_double(TextValue typeName, IntValue sdaiType)
            {
            double? ret = null;
            var adb = ADB();
            if (CheckADBType(adb, typeName))
                {
                double val = 0;
                if (ifcengine.sdaiGetADBValue(adb, sdaiType, out val) != 0)
                    {
                    ret = val;
                    }
                }
            return ret;
            }
        protected bool? get_bool(TextValue typeName, IntValue sdaiType)
            {
            Debug.Assert(sdaiType == ifcengine.sdaiBOOLEAN);
            bool? ret = null;
            var adb = ADB();
            if (CheckADBType(adb, typeName))
                {
                bool val = false;
                if (ifcengine.sdaiGetADBValue(adb, sdaiType, out val) != 0)
                    {
                    ret = val;
                    }
                }
            return ret;
            }

        //
        protected void put_double(TextValue typeName, IntValue sdaiType, double? value)
            {
            if (value.HasValue)
                {
                double v = value.Value;
                var adb = ifcengine.sdaiCreateADB(sdaiType, ref v);
                ifcengine.sdaiPutADBTypePath(adb, 1, typeName);
                SetADB(adb);
                }
            else
                {
                Debug.Assert(false);
                }
            }
        protected void put_bool(TextValue typeName, IntValue sdaiType, bool? value)
            {
            if (value.HasValue)
                {
                bool v = value.Value;
                Debug.Assert(sdaiType == ifcengine.sdaiBOOLEAN);
                var adb = ifcengine.sdaiCreateADB(sdaiType, ref v);
                ifcengine.sdaiPutADBTypePath(adb, 1, typeName);
                SetADB(adb);
                }
            else
                {
                Debug.Assert(false);
                }
            }

        //
        protected TextValue getTextValue(TextValue typeName, IntValue sdaiType)
            {
            TextValue ret = null;
            var adb = ADB();
            if (CheckADBType(adb, typeName))
                {
                string val;
                if (ifcengine.sdaiGetADBValue(adb, sdaiType, out val) != 0)
                    {
                    ret = val;
                    }
                }
            return ret;
            }

        //
        protected void putTextValue(TextValue typeName, IntValue sdaiType, TextValue value)
            {
            var adb = ifcengine.sdaiCreateADB(sdaiType, value);
            ifcengine.sdaiPutADBTypePath(adb, 1, typeName);
            SetADB(adb);
            }

        //
        protected int getEnumerationIndex(TextValue typeName, TextValue[] rEnumValues)
            {
            int ret = -1;
            var adb = ADB();
            if (CheckADBType(adb, typeName))
                {
                string value;
                if (0 != ifcengine.sdaiGetADBValue(adb, ifcengine.sdaiENUM, out value))
                    {
                    ret = EnumIndex.FromString(value, rEnumValues);
                    }
                }
            return ret;
            }

        //
        protected void putEnumerationValue(TextValue typeName, TextValue value)
            {
            var adb = ifcengine.sdaiCreateADB(ifcengine.sdaiENUM, value);
            ifcengine.sdaiPutADBTypePath(adb, 1, typeName);
            SetADB(adb);
            }

        //
        protected SdaiInstance getEntityInstance(TextValue typeName)
            {
            SdaiInstance ret = 0;
            var adb = ADB();
            if (adb != 0)
                {
                SdaiInstance inst = 0;
                if (ifcengine.sdaiGetADBValue(adb, ifcengine.sdaiINSTANCE, out inst) != 0)
                    {
                    if (typeName == null || ifcengine.sdaiIsKindOfBN(inst, typeName) != 0)
                        {
                        ret = inst;
                        }
                    }
                }
            return ret;
            }

        //
        protected void putEntityInstance(TextValue typeName, SdaiInstance inst)
            {
            if (inst == 0 || ifcengine.sdaiIsKindOfBN(inst, typeName) != 0)
                {
                var adb = ifcengine.sdaiCreateADB(ifcengine.sdaiINSTANCE, inst);
                SetADB(adb);
                }
            else
                {
                Debug.Assert(false);
                }
            }

        //
        protected SdaiAggr getAggrValue(TextValue typeName)
            {
            SdaiAggr ret = 0;
            var adb = ADB();
            if (CheckADBType(adb, typeName))
                {
                if (ifcengine.sdaiGetADBValue(adb, ifcengine.sdaiAGGR, out ret) == 0)
                    {
                    ret = 0;
                    }
                }
            return ret;
            }

        //
        protected void putAggrValue(TextValue typeName, SdaiAggr value)
            {
            var adb = ifcengine.sdaiCreateADB(ifcengine.sdaiAGGR, value);
            ifcengine.sdaiPutADBTypePath(adb, 1, typeName);
            SetADB(adb);
            }

        //
        protected bool IsADBType(TextValue typeName)
            {
            var adb = ADB();
            return CheckADBType(adb, typeName);
            }

        protected bool IsADBEntity(TextValue typeName)
            {
            var adb = ADB();
            if (adb != 0)
                {
                SdaiInstance inst = 0;
                if (ifcengine.sdaiGetADBValue(adb, ifcengine.sdaiINSTANCE, out inst) != 0)
                    {
                    if (ifcengine.sdaiIsKindOfBN(inst, typeName) != 0)
                        {
                        return true;
                        }
                    }
                }
            return false;
            }
        };

    /// <summary>
    /// Aggregations templates
    /// </summary>
    /// 
    interface IAggrSerializerObj
        {
        public abstract IList FromSdaiAggrUntyped(SdaiInstance inst, SdaiAggr aggr);
        public abstract SdaiAggr ToSdaiAggr(IEnumerable lst, SdaiInstance instance, TextValue attrName);

        }

    abstract class AggrSerializer<TElem, TList> : IAggrSerializerObj
        where TList : List<TElem>, new()
        {
        //
        public TList FromAttr(SdaiInstance instance, TextValue attrName)
            {
            SdaiAggr aggr = 0;
            ifcengine.sdaiGetAttrBN(instance, attrName, ifcengine.sdaiAGGR, out aggr);
            return FromSdaiAggr(instance, aggr);
            }

        //
        public TList FromSdaiAggr(SdaiInstance inst, SdaiAggr aggr)
            {
            var ret = new TList();
            IntValue cnt = ifcengine.sdaiGetMemberCount(aggr);
            for (IntValue i = 0; i < cnt; i++)
                {
                TElem elem;
                if (GetAggrElement(inst, aggr, i, out elem))
                    {
                    ret.Add(elem);
                    }
                }
            return ret;
            }

        public SdaiAggr ToSdaiAggr(IEnumerable<TElem> lst, SdaiInstance instance, TextValue attrName)
            {
            SdaiAggr aggr = ifcengine.sdaiCreateAggrBN(instance, attrName);
            foreach (var v in lst)
                {
                AppendAggrElement(instance, aggr, v);
                }
            return aggr;
            }

        public SdaiAggr ToSdaiAggr(IEnumerable lst, SdaiInstance instance, TextValue attrName)
            {
            SdaiAggr aggr = ifcengine.sdaiCreateAggrBN(instance, attrName);
            foreach (var v in lst)
                {
                AppendAggrElement(instance, aggr, (TElem)v);
                }
            return aggr;
            }

        protected abstract bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out TElem elem);
        protected abstract void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, TElem elem);

        IList IAggrSerializerObj.FromSdaiAggrUntyped(SdaiInstance inst, SdaiAggr aggr)
            {
            return FromSdaiAggr(inst, aggr);
            }
        }

    /// <summary>
    /// 
    /// </summary>
    class AggrSerializer_IntValue<TElem, TList> : AggrSerializer<IntValue, TList>
        where TList : List<IntValue>, new()
        {
        protected override bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out IntValue elem)
            {
            ifcengine.sdaiGetAggrByIndex(aggr, i, ifcengine.sdaiINTEGER, out elem);
            return true;
            }
        protected override void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue elem)
            {
            ifcengine.sdaiAppend(aggr, ifcengine.sdaiINTEGER, ref elem);
            }
        };

    /// <summary>
    /// 
    /// </summary>
    class AggrSerializer_double<TElem, TList> : AggrSerializer<double, TList>
        where TList : List<double>, new()
        {
        protected override bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out double elem)
            {
            ifcengine.sdaiGetAggrByIndex(aggr, i, ifcengine.sdaiREAL, out elem);
            return true;
            }
        protected override void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, double elem)
            {
            ifcengine.sdaiAppend(aggr, ifcengine.sdaiREAL, ref elem);
            }
        };

    /// <summary>
    /// 
    /// </summary>
    class AggrSerializer_bool<TElem, TList> : AggrSerializer<bool, TList>
        where TList : List<bool>, new()
        {
        protected override bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out bool elem)
            {
            ifcengine.sdaiGetAggrByIndex(aggr, i, ifcengine.sdaiBOOLEAN, out elem);
            return true;
            }
        protected override void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, bool elem)
            {
            ifcengine.sdaiAppend(aggr, ifcengine.sdaiBOOLEAN, ref elem);
            }
        };

    class AggrSerializerText<TElem, TList> : AggrSerializer<TextValue, TList>
        where TList : List<TextValue>, new()
        {
        private IntValue m_sdaiType;

        public AggrSerializerText(IntValue sdaiType)
            {
            Debug.Assert(sdaiType == ifcengine.sdaiSTRING || sdaiType == ifcengine.sdaiBINARY);
            m_sdaiType = sdaiType;
            }
        protected override bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out TextValue elem)
            {
            ifcengine.sdaiGetAggrByIndex(aggr, i, m_sdaiType, out elem);
            return (elem != null);
            }
        protected override void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, TextValue elem)
            {
            ifcengine.sdaiAppend(aggr, m_sdaiType, elem);
            }
        };

    class AggrSerializerInstance<TElem, TList> : AggrSerializer<TElem, TList>
        where TElem : Entity, new()
        where TList : List<TElem>, new()
        {
        protected override bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out TElem elem)
            {
            SdaiInstance val = 0;
            ifcengine.sdaiGetAggrByIndex(aggr, i, ifcengine.sdaiINSTANCE, out val);
            elem = new TElem();
            elem.Set(val);
            return (elem != 0);
            }
        protected override void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, TElem elem)
            {
            SdaiInstance v = elem;
            ifcengine.sdaiAppend(aggr, ifcengine.sdaiINSTANCE, v);
            }
        };

    class AggrSerializerEnum<TEnum, TList> : AggrSerializer<TEnum, TList>
        where TEnum : struct, Enum
        where TList : List<TEnum>, new()
        {
        private IntValue m_sdaiType;
        private TextValue[] m_EnumValues;

        public AggrSerializerEnum(TextValue[] enumValues, IntValue sdaiType)
            {
            Debug.Assert(sdaiType == ifcengine.sdaiENUM || sdaiType == ifcengine.sdaiLOGICAL);
            m_EnumValues = enumValues;
            m_sdaiType = sdaiType;
            }

        //
        protected override bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out TEnum elem)
            {
            string value;
            ifcengine.sdaiGetAggrByIndex(aggr, i, m_sdaiType, out value);
            var ind = EnumIndex.FromString(value, m_EnumValues);
            var val = EnumValue<TEnum>.FromIndex(ind);
            if (val.HasValue)
                {
                elem = val.Value;
                return true;
                }
            else
                {
                elem = EnumValue<TEnum>.FromIndex(0).Value;
                return false;
                }
            }
        protected override void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, TEnum elem)
            {
            var value = EnumString<TEnum>.FromValue(elem, m_EnumValues);
            ifcengine.sdaiAppend(aggr, m_sdaiType, value);
            }
        }
    /// <summary>
    /// 
    /// </summary>
    class AggrSerializerAggr<TNestedAggr, TNestedSerializer, TList> : AggrSerializer<TNestedAggr, TList>
                    where TNestedAggr : IEnumerable
                    where TNestedSerializer : IAggrSerializerObj, new()
                    where TList : List<TNestedAggr>, new()
        {
        protected override bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out TNestedAggr elem)
            {
            SdaiAggr nested = 0;
            ifcengine.sdaiGetAggrByIndex(aggr, i, ifcengine.sdaiAGGR, out nested);
            if (nested != 0)
                {
                var nestedSerializer = new TNestedSerializer();
                elem = (TNestedAggr)nestedSerializer.FromSdaiAggrUntyped(inst, nested);
                return true;
                }
            else
                {
                elem = default(TNestedAggr);
                return false;
                }
            }
        protected override void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, TNestedAggr elem)
            {
            var nestedSerializer = new TNestedSerializer();
            SdaiAggr nested = nestedSerializer.ToSdaiAggr(elem, inst, null);
            ifcengine.sdaiAppend(aggr, ifcengine.sdaiAGGR, nested);
            }
        };

    class AggrSerializerSelect<TSelect, TList> : AggrSerializer<TSelect, TList>
        where TSelect : Select, new()
        where TList : List<TSelect>, new()
        {
        protected override bool GetAggrElement(SdaiInstance inst, SdaiAggr aggr, IntValue i, out TSelect elem)
            {
            IntValue adb = 0;
            ifcengine.sdaiGetAggrByIndex(aggr, i, ifcengine.sdaiADB, out adb);
            if (adb != 0)
                {
                elem = new TSelect();
                elem.Init(inst, null, adb);
                return true;
                }
            else
                {
                elem = null;
                return false;
                }
            }
        protected override void AppendAggrElement(SdaiInstance inst, SdaiAggr aggr, TSelect elem)
            {
            var adb = elem.ADB();
            if (adb != 0)
                {
                ifcengine.sdaiAppend((IntValue)aggr, ifcengine.sdaiADB, adb);
                }
            }
        };

    /// <summary>
    /// Provides utility methods to interact with a generic SDAI instance
    /// You also can use object of this class instead of SdaiInstance handle in any place where the handle is required
    /// </summary>
    public abstract class Entity : IEquatable<Entity>, IComparable, IComparable<Entity>
        {
        public static Entity Create(SdaiModel model) { System.Diagnostics.Debug.Assert(false); return null; }

        //
        public Int64 StepID { get { return m_instance != 0 ? ifcengine.internalGetP21Line(m_instance) : 0; } }

        public bool IsNull { get { return m_instance == 0; } }

        public string EntityName { get
                {
                if (m_instance != 0)
                    {
                    var type = ifcengine.sdaiGetInstanceType(m_instance);
                    if (type != 0)
                        return ifcengine.engiGetEntityName(type);
                    }
                return null;
                } }

        //
        protected SdaiInstance m_instance = 0;

        public Entity(SdaiInstance instance)
            {
            Set(instance);
            }

        public void Set(SdaiInstance instance)
            {
            m_instance = instance;

            if (m_instance != 0)
                {
                if (ifcengine.sdaiIsKindOfBN(m_instance, WrapperEntityName()) == 0)
                    {
                    m_instance = 0;
                    }
                }
            }

        protected abstract TextValue WrapperEntityName();

        /// <summary>
        /// Conversion to instance handle, so the object of the class can be used anywhere where a handle required
        /// </summary>
        public static implicit operator SdaiInstance(Entity instance) => instance.m_instance;

        protected BaseCType? get_BaseCType(TextValue attrName, IntValue sdaiType) { return null; } //##IGNORE
        protected TextValue get_string(TextValue attrName, IntValue sdaiType)
            {
            string value;
            if (0 != ifcengine.sdaiGetAttrBN(m_instance, attrName, sdaiType, out value))
                {
                return value;
                }
            else
                {
                return null;
                }
            }
        public double? get_double(TextValue attrName, IntValue sdaiType)
            {
            double val = 0;
            if (ifcengine.sdaiGetAttrBN(m_instance, attrName, sdaiType, out val) != 0)
                return val;
            else
                return null;
            }
        public IntValue? get_IntValue(TextValue attrName, IntValue sdaiType)
            {
            IntValue val = 0;
            if (ifcengine.sdaiGetAttrBN(m_instance, attrName, sdaiType, out val) != 0)
                return val;
            else
                return null;
            }
        public bool? get_bool(TextValue attrName, IntValue sdaiType)
            {
            bool val = false;
            if (ifcengine.sdaiGetAttrBN(m_instance, attrName, sdaiType, out val) != 0)
                return val;
            else
                return null;
            }

        /// <summary>
        /// 
        /// </summary>
        public static bool operator ==(Entity i1, Entity i2) => (Equals(i1, i2));
        public static bool operator !=(Entity i1, Entity i2) => (!(i1 == i2));
        public override bool Equals(Object obj)
            {
            return Equals(obj as Entity);
            }
        public bool Equals(Entity other)
            {
            return (other == null) ? false : (other.m_instance == m_instance);
            }
        public int CompareTo(object obj)
            {
            return CompareTo(obj as Entity);
            }
        public int CompareTo(Entity other)
            {
            return (other == null) ? 1 : m_instance.CompareTo(other.m_instance);
            }
        public override int GetHashCode()
            {
            return m_instance.GetHashCode();
            }
        }
    //## TEMPLATE: ClassForwardDeclaration
    //## TEMPLATE: DefinedTypesBegin

    //
    // Defined types
    // 
    //## TEMPLATE: DefinedTypeSimple
    //## TEMPLATE: DefinedTypeEntity
    public class DEFINED_TYPE_NAMe : ENTITY_NAME
        {
        public DEFINED_TYPE_NAMe(SdaiInstance instance) : base(instance) { }
        public DEFINED_TYPE_NAMe() : base(0) { }
        public static implicit operator DEFINED_TYPE_NAMe(SdaiInstance instance) => new DEFINED_TYPE_NAMe(instance);
        }
    //## TEMPLATE: DefinedTypeEnum
    //## TEMPLATE: DefinedTypeSelect

    public class DEFINED_TYPE_NAME : SImpleType
        {
        /// <summary>
        /// Use this constructor if you want to put value for attribute (scalar or aggregation)
        /// </summary>
        /// <param name="instance">instance you intent to modify</param>
        /// <param name="attrName">scalar attribute you intent to modify, leave null for aggregations</param>
        /// <param name="adb">leave null, it is for internal workflow</param>
        public DEFINED_TYPE_NAME(SdaiInstance instance, TextValue attrName = null, IntValue adb = 0) : base(instance, attrName, adb) { }

        /// <summary>
        /// Use this constructor to put nested select value
        /// </summary>
        /// <param name="outer">outer select</param>
        public DEFINED_TYPE_NAME(Select outer) : base(outer) { }

        /// <summary>
        /// !!! do not use this constructor, it is for internal workflow
        /// </summary>
        public DEFINED_TYPE_NAME() : base(null) { }
        }
    //## TEMPLATE: EnumerationsBegin

    //
    // Enumerations
    //
    public enum LOGICAL_VALUE { False = 0, True = 1, Unknown = 2 };
    //## TEMPLATE: EnumerationBegin

    public enum ENUMERATION_NAME
        {
        //## EnumerationElement
        ENUMERATION_ELEMENT = 1234,
        //## EnumerationEnd
        };
    //## EnumerationNamesBegin
    //
    class EnumNames
        {
        public static TextValue[] ENUMERATION_VALUES_ARRAY = null; //## IGNORE
        public static TextValue[] LOGICAL_VALUE_ = { "F", "T", "U" };
        //## EnumerationNames
        public static TextValue[] ENUMERATION_NAME_ = { "ENUMERATION_STRING_VALUES" };
        //## TEMPLATE: EnumerationsEnd
        }

    //## TEMPLATE: AggregationTypesBegin
    //
    // Unnamed aggregations
    //
    class AggrSerializer_SimpleType<SimpleType, ListType> : IAggrSerializerObj //## IGNORE
        { //## IGNORE
        public IList FromSdaiAggrUntyped(long inst, long aggr) { throw new NotImplementedException(); } //## IGNORE
        public long ToSdaiAggr(IEnumerable lst, long instance, string attrName) { throw new NotImplementedException(); } //## IGNORE 
        }  //## IGNORE
    //## AggregationOfSimple
    public class AggregationTYpe : List<SimpleType> { }
    class AggregationTYpeSerializer : AggrSerializer_SimpleType<SimpleType, AggregationTYpe> { }
    //## AggregationOfText
    public class Aggregationtype : List<TextValue> { }
    class AggregationtypeSerializer : AggrSerializerText<TextValue, Aggregationtype> { public AggregationtypeSerializer() : base(ifcengine.sdaiTYPE) { } }
    //## AggregationOfInstance
    public class AggregationType : List<REF_ENTITY> { }
    class AggregationTypeSerializer : AggrSerializerInstance<REF_ENTITY, AggregationType> { }
    //## AggregationOfEnum
    public class AggregationTyPe : List<ENUMERATION_NAME> { }
    class AggregationTyPeSerializer : AggrSerializerEnum<ENUMERATION_NAME, AggregationTyPe> { public AggregationTyPeSerializer() : base(EnumNames.ENUMERATION_VALUES_ARRAY, ifcengine.sdaiTYPE) { } };
    //class AggregationTyPeSerializer : AggrSerializerEnum<ENUMERATION_NAME, AggregationTyPe> { public AggregationTyPeSerializer() : base(EnumNames.ENUMERATION_VALUES_ARRAY, ifcengine.sdaiENUM) { } };
    //## AggregationOfAggregation
    class SIMpleTypeSerializer : AggrSerializer_SimpleType<SimpleType, AggregationTYpe> { } //## IGNORE
    public class AggregationTYPe : List<SIMpleType> { }
    class AggregationTYPeSerializer : AggrSerializerAggr<SIMpleType, SIMpleTypeSerializer, AggregationTYPe> { }
    //## AggregationOfSelect
    public class AggregationTYPE : List<SImpleType> { }
    class AggregationTYPESerializer : AggrSerializerSelect<SImpleType, AggregationTYPE> { }
    //## SelectsBegin

    //
    // SELECT TYPES
    // 
    //## TEMPLATE: SelectAccessorBegin

    public class GEN_TYPE_NAME_accessor : Select
        {
        /// <summary>
        /// Use this constructor if you want to put value for attribute (scalar or aggregation)
        /// </summary>
        /// <param name="instance">instance you intent to modify</param>
        /// <param name="attrName">scalar attribute you intent to modify, leave null for aggregations</param>
        /// <param name="adb">leave null, it is for internal workflow</param>
        public GEN_TYPE_NAME_accessor(SdaiInstance instance, TextValue attrName = null, IntValue adb = 0) : base(instance, attrName, adb) { }

        /// <summary>
        /// Use this constructor to put nested select value
        /// </summary>
        /// <param name="outer">outer select</param>
        public GEN_TYPE_NAME_accessor(Select outer) : base(outer) { }

        /// <summary>
        /// !!! do not use this constructor, it is for internal workflow
        /// </summary>
        public GEN_TYPE_NAME_accessor() : base(null) { }

        //## SelectSimpleGet
        public bool is_TypeNameIfc { get { return IsADBType("TypeNameUpper"); } }
        public BaseCType? TypeNameIfc
            {
            get { return get_BaseCType("TypeNameUpper", ifcengine.sdaiTYPE); }
            }
        //## SelectSimplePut
        public bool is_typeNameIfc { get { return IsADBType("TypeNameUpper"); } }
        public BaseCType? TypeNameifc
            {
            set { put_BaseCType("TypeNameUpper", ifcengine.sdaiTYPE, value); }
            get { return get_BaseCType("TypeNameUpper", ifcengine.sdaiTYPE); }
            }
        //## SelectTextGet
        public bool is_TypeNameIFC { get { return IsADBType("TypeNameUpper"); } }
        public TextValue TypeNameIFc
            {
            get { return getTextValue("TypeNameUpper", ifcengine.sdaiTYPE); }
            }
        //## SelectTextPut
        public bool is_typeNameIFC { get { return IsADBType("TypeNameUpper"); } }
        public TextValue TypeNameIFC { 
            set { putTextValue("TypeNameUpper", ifcengine.sdaiTYPE, value); }
            get { return getTextValue("TypeNameUpper", ifcengine.sdaiTYPE); } 
            }
        //## SelectEntityGet
        public bool is_REF_ENTITY { get { return IsADBEntity("REF_ENTITY"); } }
        public REF_ENTITY REF_ENTITy_suffix
            {
            get { return new REF_ENTITY(getEntityInstance("TypeNameUpper")); }
            }
        //## SelectEntityPut
        public bool is_rEF_ENTITY { get { return IsADBEntity("REF_ENTITY"); } }
        public REF_ENTITY REF_ENTITY_suffix
            {
            set { putEntityInstance("TypeNameUpper", value); }
            get { return new REF_ENTITY(getEntityInstance("TypeNameUpper")); } 
            }
        //## SelectEnumerationGet
        public bool is_TypeNAmeIFC { get { return IsADBType("TypeNameUpper"); } }
        public ENUMERATION_NAME? TypeNAmeIFc
            {
            get
                {
                int ind = getEnumerationIndex("TypeNameUpper", EnumNames.ENUMERATION_VALUES_ARRAY);
                return EnumValue<ENUMERATION_NAME>.FromIndex(ind);
                }
            }
        //## SelectEnumerationPut
        public bool is_typeNAmeIFC { get { return IsADBType("TypeNameUpper"); } }
        public ENUMERATION_NAME? TypeNAmeIFC 
            { 
            set {
                if (value.HasValue)
                    {
                    TextValue val = EnumString<ENUMERATION_NAME>.FromValue(value.Value, EnumNames.ENUMERATION_VALUES_ARRAY); putEnumerationValue("TypeNameUpper", val);
                    }
                else
                    {
                    Debug.Assert(false);
                    }
                }
            get { 
                int ind = getEnumerationIndex("TypeNameUpper", EnumNames.ENUMERATION_VALUES_ARRAY); 
                return EnumValue<ENUMERATION_NAME>.FromIndex(ind); 
                } 
            }
        //## SelectAggregationGet
        public bool is_AggregationType { get { return IsADBType("TypeNameUpper"); } }
        public AggregationType AggregationType { get { SdaiAggr aggr = getAggrValue("TypeNameUpper"); return (new AggregationTypeSerializer()).FromSdaiAggr(m_instance, aggr); } }
        //## SelectAggregationPut
        public bool is_aggregationType { get { return IsADBType("TypeNameUpper"); } }
        public AggregationType AGgregationType { get { SdaiAggr aggr = getAggrValue("TypeNameUpper"); return (new AggregationTypeSerializer()).FromSdaiAggr(m_instance, aggr); } }
        public void put_AggregationType(IEnumerable<REF_ENTITY> lst) { SdaiAggr aggr = (new AggregationTypeSerializer()).ToSdaiAggr(lst, m_instance, null); putAggrValue("TypeNameUpper", aggr); }
        public void put_AggregationType(IEnumerable lst) { SdaiAggr aggr = (new AggregationTypeSerializer()).ToSdaiAggr(lst, m_instance, null); putAggrValue("TypeNameUpper", aggr); }
        //## SelectAggregationPutArray
        //## SelectNested
        public GEN_TYPE_NAME_accessor GEN_TYPE_NAME { get { return new GEN_TYPE_NAME_accessor(this); } }
        //## SelectGetAsDouble
        public double? as_double { get { double val = 0; if (ifcengine.sdaiGetAttrBN(m_instance, m_attrName, ifcengine.sdaiREAL, out val) != 0) return val; else return null; } }
        //## SelectGetAsInt
        public IntValue? as_int { get { IntValue val = 0; if (ifcengine.sdaiGetAttrBN(m_instance, m_attrName, ifcengine.sdaiINTEGER, out val) != 0) return val; else return null; } }
        //## SelectGetAsBool
        public bool? as_bool { get { bool val = false; if (ifcengine.sdaiGetAttrBN(m_instance, m_attrName, ifcengine.sdaiBOOLEAN, out val) != 0) return val; else return null; } }
        //## SelectGetAsText
        public TextValue as_text { get { string val = null; if (ifcengine.sdaiGetAttrBN(m_instance, m_attrName, ifcengine.sdaiSTRING, out val) != 0) return val; else return null; } }
        //## SelectGetAsEntity
        public SdaiInstance as_instance { get { return getEntityInstance(null); } }
        //## SelectAccessorEnd
        };

    //## TEMPLATE: EntitiesBegin

    //
    // Entities
    // 

    //## TEMPLATE: EntityBegin

    /// <summary>
    /// Provides utility methods to interact with an instance of ENTITY_NAME
    /// You also can use object of this C++ class instead of IntValue handle of the OWL instance in any place where the handle is required
    /// </summary>
    public class ENTITY_NAME : /*PARENT_NAME*/Entity
        {
        /// <summary>
        /// Constructs object of this C# class that wraps existing SdaiInstance of ENTITY_NAME
        /// </summary>
        /// <param name="instance">An instance to interact with</param>
        public ENTITY_NAME(SdaiInstance instance) : base(instance) { }
        public ENTITY_NAME() : base(0) { }

        public static implicit operator ENTITY_NAME(SdaiInstance instance) => new ENTITY_NAME(instance);

        //## EntityCreateMethod
        /// <summary>
        /// Create new instance of ENTITY_NAME and returns object of this class to interact with
        /// </summary>
        public static new ENTITY_NAME Create(SdaiModel model) { SdaiInstance inst = ifcengine.sdaiCreateInstanceBN(model, "ENTITY_NAME"); Debug.Assert(inst != 0); return inst; }

        //## AttributeSimpleGet
        public BaseCType? aTTR_NAME { get { return get_BaseCType("ATTR_NAME", ifcengine.sdaiTYPE); } }
        //## AttributeSimplePut
        public BaseCType? ATTR_NAME
            {
            get { return get_BaseCType("ATTR_NAME", ifcengine.sdaiTYPE); }
            set { if (value.HasValue) { BaseCType v = value.Value; ifcengine.sdaiPutAttrBN(m_instance, "ATTR_NAME", ifcengine.sdaiTYPE, ref v); } else Debug.Assert(false); }
            }
        //## AttributeTextGet
        public TextValue attr_NAME { get { return get_string("ATTR_NAME", ifcengine.sdaiTYPE); } }
        //## AttributeTextPut
        public TextValue aTtr_NAME
            {
            get { return get_string("ATTR_NAME", ifcengine.sdaiTYPE); }
            set { ifcengine.sdaiPutAttrBN(m_instance, "ATTR_NAME", ifcengine.sdaiTYPE, value); }
            }
        //## AttributeEntityGet
        public REF_ENTITY Attr_nAME { get { SdaiInstance inst = 0; ifcengine.sdaiGetAttrBN(m_instance, "ATTR_NAME", ifcengine.sdaiINSTANCE, out inst); return new REF_ENTITY(inst); } }
        //## AttributeEntityPut
        public REF_ENTITY Attr_NAME 
            {
            get { SdaiInstance inst = 0; ifcengine.sdaiGetAttrBN(m_instance, "ATTR_NAME", ifcengine.sdaiINSTANCE, out inst); return new REF_ENTITY(inst); } 
            set { SdaiInstance i = value; ifcengine.sdaiPutAttrBN(m_instance, "ATTR_NAME", ifcengine.sdaiINSTANCE, i); }
            }
        //## AttributeEnumGet
        public ENUMERATION_NAME? ATtr_nAME { get { var str = get_string("ATTR_NAME", ifcengine.sdaiENUM); var ind = EnumIndex.FromString(str, EnumNames.ENUMERATION_VALUES_ARRAY); return EnumValue<ENUMERATION_NAME>.FromIndex(ind); } }
        //## AttributeEnumPut
        public ENUMERATION_NAME? ATtr_NAME 
            { 
            get { var str = get_string("ATTR_NAME", ifcengine.sdaiENUM); var ind = EnumIndex.FromString(str, EnumNames.ENUMERATION_VALUES_ARRAY); return EnumValue<ENUMERATION_NAME>.FromIndex(ind); } 
            set { if (value.HasValue) { var str = EnumString<ENUMERATION_NAME>.FromValue(value.Value, EnumNames.ENUMERATION_VALUES_ARRAY); ifcengine.sdaiPutAttrBN(m_instance, "ATTR_NAME", ifcengine.sdaiENUM, str); } else Debug.Assert(false); }
            }
        //## AttributeSelectAccessor        
        public GEN_TYPE_NAME_accessor ATtR_NAME
            {
            get { return new GEN_TYPE_NAME_accessor(m_instance, "ATTR_NAME", 0); }
            }
        //## AttributeAggregationGet
        public AggregationType ATTr_NAMe { get { return (new AggregationTypeSerializer()).FromAttr(m_instance, "ATTR_NAME"); } }
        //## AttributeAggregationPut
        public AggregationType ATTr_NAME { get { return (new AggregationTypeSerializer()).FromAttr(m_instance, "ATTR_NAME"); } }
        public void put_ATTr_NAME(IEnumerable<SimpleType> lst) { (new AggregationTypeSerializer()).ToSdaiAggr(lst, m_instance, "ATTR_NAME"); }
        public void put_ATTr_NAME_untyped(IEnumerable lst) { (new AggregationTypeSerializer()).ToSdaiAggr(lst, m_instance, "ATTR_NAME"); }
        //## AttributeAggregationPutArray
        //## EntityEnd

        protected override TextValue WrapperEntityName() { return "ENTITY_NAME"; }
    };

    //## SelectEntityGetImplementation
    //## SelectEntityPutImplementation
    //## AttributeEntityGetImplementation
    //## AttributeEntityPutImplementation
    //## TEMPLATE: EndFile template part
}

