//
// Early-binding C++ API for SDAI (C++ wrappers)
//
#ifndef __RDF_LTD__NAMESPACE_NAME_H
#define __RDF_LTD__NAMESPACE_NAME_H

#include    <assert.h>
#include    <list>
#include    <string>

#include	"ifcengine.h"

namespace NAMESPACE_NAME
{
    ///
    typedef int_t SdaiModel;
    typedef int_t SdaiInstance;

    typedef const char* TextValue;
    typedef int_t       IntValue;

    class StringValue : public std::string
    {
    public:
        StringValue(TextValue str) : std::string(str) {}
        operator const char* () const { return c_str(); }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename T> class Nullable
    {
    protected:
        T* m_value;

    public:
        Nullable<T>() : m_value(NULL) {}
        Nullable<T>(T value) { m_value = new T(value); }
        Nullable<T>(const Nullable<T>& src) { if (src.m_value) m_value = new T(*src.m_value); else m_value = NULL; }

        virtual ~Nullable<T>() { if (m_value) { delete m_value; } };

        bool IsNull() const { return !m_value; }
        T Value() const { assert(m_value); if (m_value) return *m_value; else return (T) 0; }

        virtual Nullable<T>& operator=(const Nullable<T>& src)
        {
            if (m_value) { delete m_value; }
            m_value = NULL;
            if (src.m_value) { m_value = new T(*(src.m_value)); }
            return *this;
        }
    };

    //
    //
    enum class LOGICAL_VALUE { False = 0, True, Unknown };
    static TextValue LOGICAL_VALUE_[] = {"F", "T", "U", NULL};

    //
    //
    static int EnumerationNameToIndex(TextValue rEnumValues[], TextValue value)
    {
        if (value) {
            for (int i = 0; rEnumValues[i]; i++) {
                if (0 == _stricmp(value, rEnumValues[i])) {
                    return i;
                }
            }
        }
        return -1;
    }

    /// <summary>
    /// Helper class to handle and access SELECT instance data
    /// </summary>
    class Select
    {
    protected:
        SdaiInstance m_instance;
        TextValue m_attrName;

    private:
        void* m_adb;
        Select* m_outerSelect;

    public:
        void* ADB()
        {
            if (m_outerSelect) {
                return m_outerSelect->ADB();
            }

            if (!m_adb && m_instance && m_attrName) {
                if (!sdaiGetAttrBN(m_instance, m_attrName, sdaiADB, &m_adb)) {
                    sdaiDeleteADB(m_adb);
                    m_adb = NULL;
                }
            }

            return m_adb;
        }

    protected:
        Select(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL)
            : m_instance(instance), m_attrName(attrName), m_adb(adb), m_outerSelect(NULL)
        {
            assert(instance);
        }

        Select(Select* outer)
            : m_instance(NULL), m_attrName(NULL), m_adb(NULL), m_outerSelect(outer)
        {
            assert(outer);
            if (m_outerSelect) {
                m_instance = m_outerSelect->m_instance;
            }
        }

        ~Select()
        {
            sdaiDeleteADB(m_adb);
        }

        void SetADB(void* adb)
        {
            if (m_outerSelect) {
                m_outerSelect->SetADB(adb);
            }
            else {
                //???sdaiDeleteADB(m_adb);
                m_adb = adb;

                if (m_instance && m_attrName) {
                    sdaiPutAttrBN(m_instance, m_attrName, sdaiADB, m_adb);
                }
            }
        }

        //
        template <typename T> Nullable<T> getSimpleValue(TextValue typeName, IntValue sdaiType)
        {
            Nullable<T> ret;
            if (void* adb = ADB()) {
                char* path = sdaiGetADBTypePath(adb, 0);
                if (typeName == NULL || path && 0 == _stricmp(path, typeName)) {
                    T val = (T) 0;
                    if (sdaiGetADBValue(adb, sdaiType, &val)) {
                        ret = val;
                    }
                }
            }
            return ret;
        }

        //
        template <typename T> void putSimpleValue(TextValue typeName, IntValue sdaiType, T value)
        {
            void* adb = sdaiCreateADB(sdaiType, &value);
            sdaiPutADBTypePath(adb, 1, typeName);
            SetADB(adb);
        }

        //
        TextValue getTextValue(TextValue typeName, IntValue sdaiType)
        {
            TextValue ret = NULL;
            if (void* adb = ADB()) {
                char* path = sdaiGetADBTypePath(adb, 0);
                if (typeName == NULL || path && 0 == _stricmp(path, typeName)) {
                    if (!sdaiGetADBValue(adb, sdaiType, &ret)) {
                        ret = NULL;
                    }
                }
            }
            return ret;
        }

        //
        void putTextValue(TextValue typeName, IntValue sdaiType, TextValue value)
        {
            void* adb = sdaiCreateADB(sdaiType, value);
            sdaiPutADBTypePath(adb, 1, typeName);
            SetADB(adb);
        }

        //
        int getEnumerationValue(TextValue typeName, TextValue rEnumValues[])
        {
            int ret = -1;
            if (void* adb = ADB()) {
                char* path = sdaiGetADBTypePath(adb, 0);
                if (typeName == NULL || path && 0 == _stricmp(path, typeName)) {
                    TextValue value = NULL;
                    if (sdaiGetADBValue(adb, sdaiENUM, &value)) {
                        ret = EnumerationNameToIndex(rEnumValues, value);
                    }
                }
            }
            return ret;
        }

        //
        void putEnumerationValue(TextValue typeName, TextValue value)
        {
            void* adb = sdaiCreateADB(sdaiENUM, value);
            sdaiPutADBTypePath(adb, 1, typeName);
            SetADB(adb);
        }

        //
        SdaiInstance getEntityInstance(TextValue typeName)
        {
            SdaiInstance ret = 0;
            if (auto adb = ADB()) {
                SdaiInstance inst = 0;
                if (sdaiGetADBValue(adb, sdaiINSTANCE, &inst)) {
                    if (typeName == NULL || sdaiIsKindOfBN(inst, typeName)) {
                        ret = inst;
                    }
                }
            }
            return ret;
        }

        //
        void putEntityInstance(TextValue typeName, SdaiInstance inst)
        {
            if (inst == 0 || sdaiIsKindOfBN(inst, typeName)) {
                auto adb = sdaiCreateADB(sdaiINSTANCE, (void*) inst);
                SetADB(adb);
            }
            else {
                assert(0);
            }
        }

        //
        SdaiAggr getAggrValue(TextValue typeName)
        {
            SdaiAggr ret = NULL;
            if (void* adb = ADB()) {
                char* path = sdaiGetADBTypePath(adb, 0);
                if (typeName == NULL || path && 0 == _stricmp(path, typeName)) {
                    if (!sdaiGetADBValue(adb, sdaiAGGR, &ret)) {
                        ret = NULL;
                    }
                }
            }
            return ret;
        }

        //
        void putAggrValue(TextValue typeName, SdaiAggr value)
        {
            void* adb = sdaiCreateADB(sdaiAGGR, value);
            sdaiPutADBTypePath(adb, 1, typeName);
            SetADB(adb);
        }

        //
        bool IsADBType(TextValue typeName)
        {
            if (void* adb = ADB()) {
                char* path = sdaiGetADBTypePath(adb, 0);
                if (0 == _stricmp(path, typeName)) {
                    return true;
                }
            }
            return false;
        }

        bool IsADBEntity(TextValue typeName)
        {
            if (void* adb = ADB()) {
                IntValue inst = 0;
                if (sdaiGetADBValue(adb, sdaiINSTANCE, &inst)) {
                    if (sdaiIsKindOfBN(inst, typeName)) {
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

    template <typename TArrayElem, typename TList> void ArrayToList(TArrayElem arrayElems[], IntValue numOfElems, TList& lst)
    {
        for (IntValue i = 0; i < numOfElems; i++) {
            lst.push_back(arrayElems[i]);
        }
    }

    template <typename TList> class AggrSerializer
    {
    public:
        //
        void FromAttr(TList& lst, SdaiInstance instance, TextValue attrName)
        {
            SdaiAggr aggr = NULL;
            sdaiGetAttrBN(instance, attrName, sdaiAGGR, &aggr);
            if (aggr) {
                FromSdaiAggr(lst, instance, aggr);
            }
        }

        //
        virtual void FromSdaiAggr(TList& lst, SdaiInstance inst, SdaiAggr aggr) = 0; 
        virtual SdaiAggr ToSdaiAggr(TList& lst, SdaiInstance instance, TextValue attrName) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TList, typename TElem, IntValue sdaiType> class AggrSerializerSimple : public AggrSerializer<TList>
    {
    public:
        AggrSerializerSimple() { assert(sdaiType == sdaiINTEGER || sdaiType == sdaiREAL || sdaiType == sdaiBOOLEAN); }

        //
        virtual void FromSdaiAggr(TList& lst, SdaiInstance /*unused*/, SdaiAggr aggr) override
        {
            IntValue  cnt = sdaiGetMemberCount(aggr);
            for (IntValue i = 0; i < cnt; i++) {
                TElem val = 0;
                engiGetAggrElement(aggr, i, sdaiType, &val);
                lst.push_back(val);
            }
        }

        //
        virtual SdaiAggr ToSdaiAggr(TList& lst, SdaiInstance instance, TextValue attrName) override
        {
            SdaiAggr aggr = sdaiCreateAggrBN(instance, attrName);
            for (auto const& v : lst) {
                TElem val = v;
                sdaiAppend((IntValue) aggr, sdaiType, &val);
            }
            return aggr;
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TList, typename TElem, IntValue sdaiType> class AggrSerializerText : public AggrSerializer<TList>
    {
    public:
        AggrSerializerText() { assert(sdaiType == sdaiSTRING || sdaiType == sdaiBINARY); }

        virtual void FromSdaiAggr(TList& lst, SdaiInstance /*unused*/, SdaiAggr aggr) override
        {
            IntValue  cnt = sdaiGetMemberCount(aggr);
            for (IntValue i = 0; i < cnt; i++) {
                TextValue val;
                engiGetAggrElement(aggr, i, sdaiType, &val);
                lst.push_back(val);
            }
        }

        virtual SdaiAggr ToSdaiAggr(TList& lst, SdaiInstance instance, TextValue attrName) override
        {
            SdaiAggr aggr = sdaiCreateAggrBN(instance, attrName);
            for (auto& val : lst) {
                TextValue v = val;
                sdaiAppend((IntValue) aggr, sdaiType, v);
            }
            return aggr;
        }

    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TList, typename TElem> class AggrSerializerInstance : public AggrSerializer <TList>
    {
    public:
        //
        virtual void FromSdaiAggr(TList& lst, SdaiInstance /*unused*/, SdaiAggr aggr) override
        {
            auto  cnt = sdaiGetMemberCount(aggr);
            for (IntValue i = 0; i < cnt; i++) {
                SdaiInstance val = 0;
                engiGetAggrElement(aggr, i, sdaiINSTANCE, &val);
                TElem elem(val);
                if (val) {
                    lst.push_back(val);
                }
            }
        }

        //
        virtual SdaiAggr ToSdaiAggr(TList& lst, SdaiInstance instance, TextValue attrName) override
        {
            auto aggr = sdaiCreateAggrBN(instance, attrName);
            for (auto& val : lst) {
                SdaiInstance v = val;
                sdaiAppend((IntValue) aggr, sdaiINSTANCE, (void*) v);
            }
            return aggr;
        }
    };


    /// <summary>
    /// 
    /// </summary>
    template <typename TList, typename TElem, TextValue* rEnumValues, IntValue sdaiType> class AggrSerializerEnum : public AggrSerializer<TList>
    {
    public:
        AggrSerializerEnum() { assert(sdaiType == sdaiENUM || sdaiType == sdaiLOGICAL); }

        //
        virtual void FromSdaiAggr(TList& lst, SdaiInstance /*instance*/, SdaiAggr aggr) override
        {
            IntValue  cnt = sdaiGetMemberCount(aggr);
            for (IntValue i = 0; i < cnt; i++) {
                TextValue value = NULL;
                engiGetAggrElement(aggr, i, sdaiType, &value);
                int val = EnumerationNameToIndex(rEnumValues, value);
                if (val >= 0) {
                    lst.push_back((TElem) val);
                }
            }
        }

        //
        virtual SdaiAggr ToSdaiAggr(TList& lst, SdaiInstance instance, TextValue attrName) override
        {
            SdaiAggr aggr = sdaiCreateAggrBN(instance, attrName);
            for (auto const& val : lst) {
                TextValue value = rEnumValues[(IntValue) val];
                sdaiAppend((IntValue) aggr, sdaiType, value);
            }
            return aggr;
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TList, typename TNestedAggr, typename TNestedSerializer> class AggrSerializerAggr : public AggrSerializer<TList>
    {
    public:
        //
        virtual void FromSdaiAggr(TList& lst, SdaiInstance instance, SdaiAggr aggr) override
        {
            IntValue  cnt = sdaiGetMemberCount(aggr);
            for (IntValue i = 0; i < cnt; i++) {
                SdaiAggr nested = 0;
                engiGetAggrElement(aggr, i, sdaiAGGR, &nested);
                if (nested) {
                    lst.push_back(TNestedAggr());
                    TNestedSerializer nestedSerializer;
                    nestedSerializer.FromSdaiAggr(lst.back(), instance, nested);
                }
            }
        }

        //
        virtual SdaiAggr ToSdaiAggr(TList& lst, SdaiInstance instance, TextValue attrName) override
        {
            SdaiAggr aggr = sdaiCreateAggrBN(instance, attrName);
            for (TNestedAggr& val : lst) {
                TNestedSerializer nestedSerializer;
                SdaiAggr nested = nestedSerializer.ToSdaiAggr(val, instance, NULL);
                sdaiAppend((IntValue) aggr, sdaiAGGR, nested);
            }
            return aggr;
        }
    };

    template<typename TList, typename TElem> class AggrSerializerSelect : public AggrSerializer<TList>
    {
    public:
        //
        virtual void FromSdaiAggr(TList& lst, SdaiInstance instance, SdaiAggr aggr) override
        {
            IntValue  cnt = sdaiGetMemberCount(aggr);
            for (IntValue i = 0; i < cnt; i++) {
                void* adb = 0;
                engiGetAggrElement(aggr, i, sdaiADB, &adb);
                if (adb) {
                    lst.push_back(TElem(instance, NULL, adb));
                }
            }
        }

        //
        virtual SdaiAggr ToSdaiAggr(TList& lst, SdaiInstance instance, TextValue attrName) override
        {
            SdaiAggr aggr = sdaiCreateAggrBN(instance, attrName);
            for (auto& val : lst) {
                void* adb = val.ADB();
                if (adb) {
                    sdaiAppend((IntValue) aggr, sdaiADB, adb);
                }
            }
            return aggr;
        }
    };


    /// <summary>
    /// Provides utility methods to interact with a generic SDAI instnace
    /// You also can use object of this class instead of SdaiInstance handle in any place where the handle is required
    /// </summary>
    class Entity
    {
    protected:
        SdaiInstance m_instance;

    public:
        Entity(SdaiInstance instance, TextValue entityName)
        {
            m_instance = instance;

            if (m_instance != 0 && entityName != NULL) {
                if (!sdaiIsKindOfBN(m_instance, entityName)) {
                    m_instance = 0;
                }
            }
        }


        /// <summary>
        /// Conversion  to instance handle, so the object of the class can be used anywhere where a handle required
        /// </summary>
        operator SdaiInstance() const { return m_instance; }

    protected:
        //
        //
        int getENUM(TextValue attrName, TextValue rEnumValues[])
        {
            TextValue value = NULL;
            sdaiGetAttrBN(m_instance, attrName, sdaiENUM, (void*) &value);
            return EnumerationNameToIndex(rEnumValues, value);
        }
    };


    //
    // Entities forward declarations
    //

//## TEMPLATE: ClassForwardDeclaration
    class ENTITY_NAME;
    //## TEMPLATE: DefinedTypesBegin
#define sdaiTYPE  (-1)                                          //## IGNORE
#define ENUMERATION_VALUES_ARRAY ENUMERATION_NAME_              //## IGNORE
    typedef double        SimpleType;                           //## IGNORE
    typedef TextValue     TextType;                             //## IGNORE
    typedef int           SelectType;                           //## IGNORE
    typedef int_t         REF_ENTITY;                           //## IGNORE
    typedef int_t         TypeNameIFC;                          //## IGNORE
    template <typename TList> class SimpleTypeSerializer {};    //## IGNORE

    //
    // Defined types
    // 
//## TEMPLATE: DefinedTypeSimple
    typedef SimpleType DEFINED_TYPE_NAME;
    //## TEMPLATE: DefinedTypeEntity
    typedef SimpleType DEFINED_TYPE_NAME;
    //## TEMPLATE: DefinedTypeEnum
    typedef SimpleType DEFINED_TYPE_NAME;
    //## TEMPLATE: DefinedTypeSelect
    typedef SimpleType DEFINED_TYPE_NAME;
    //## TEMPLATE: EnumerationsBegin

        //
        // Enumerations
        //
    //## EnumerationBegin

    enum class ENUMERATION_NAME
    {
        //## EnumerationElement
        ENUMERATION_ELEMENT = 1234,
        //## EnumerationEnd
        ___unk = -1
    };
    //## EnumerationNamesBegin
    //
            //## EnumerationNames
    static TextValue ENUMERATION_NAME_[] = {"ENUMERATION_STRING_VALUES", NULL};
    //## TEMPLATE: EnumerationsEnd
    //## TEMPLATE: AggregationTypesBegin

        //
        // Unnamed aggregations
        //
    //## AggregationOfSimple
    typedef std::list<SimpleType> AggregationType;
    template <typename TList> class AggregationTypeSerializer : public AggrSerializerSimple<TList, SimpleType, sdaiTYPE> {};
    //## AggregationOfText
    typedef std::list<StringValue> Aggregationtype;
    template <typename TList> class AggregationtypeSerializer : public AggrSerializerText<Aggregationtype, TextType, sdaiTYPE> {};
    //## AggregationOfInstance
    typedef std::list<SimpleType> AggregationTYpe;
    template <typename TList> class AggregationTYpeSerializer : public AggrSerializerInstance<TList, SimpleType> {};
    //## AggregationOfEnum
    typedef std::list<TypeNameIFC> AggregationTyPe;
    template <typename TList> class AggregationTyPeSerializer : public AggrSerializerEnum<TList, ENUMERATION_NAME, ENUMERATION_VALUES_ARRAY, sdaiTYPE> {};
    //## AggregationOfAggregation
    typedef std::list<SimpleType> AggregationTYPe;
    template <typename TList> class AggregationTYPeSerializer : public AggrSerializerAggr<TList, SimpleType, SimpleTypeSerializer<SimpleType>> {};
    //## AggregationOfSelect
    typedef std::list<SimpleType> AggregationTYPE;
    template <typename TList> class AggregationTYPESerializer : public AggrSerializerSelect<TList, SimpleType> {};
    //## SelectsBegin

        //
        // SELECT TYPES
        // 
    //## TEMPLATE: SelectAccessorBegin

    class GEN_TYPE_NAME_accessor : public Select
    {
    public:
        GEN_TYPE_NAME_accessor(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        GEN_TYPE_NAME_accessor(Select* outer) : Select(outer) {}
        //## SelectSimpleGet
        bool is_SimpleType() { return IsADBType("TypeNameUpper"); }
        Nullable<SimpleType> get_SimpleType() { return getSimpleValue<SimpleType>("TypeNameUpper", sdaiTYPE); }
        //## SelectSimplePut
        void put_SimpleType(SimpleType value) { putSimpleValue("TypeNameUpper", sdaiTYPE, value); }
        //## SelectTextGet
        bool is_TextType() { return IsADBType("TypeNameUpper"); }
        TextType get_TextType() { return getTextValue("TypeNameUpper", sdaiTYPE); }
        //## SelectTextPut
        void put_TextType(TextType value) { putTextValue("TypeNameUpper", sdaiTYPE, value); }
        //## SelectEntityGet
        bool is_REF_ENTITY() { return IsADBEntity("REF_ENTITY"); }
        REF_ENTITY get_REF_ENTITY();
        //## SelectEntityPut
        void put_REF_ENTITY(REF_ENTITY inst);
        //## SelectEnumerationGet
        bool is_TypeNameIFC() { return IsADBType("TypeNameUpper"); }
        Nullable<TypeNameIFC> get_TypeNameIFC() { int v = getEnumerationValue("TypeNameUpper", ENUMERATION_VALUES_ARRAY); if (v >= 0) return (TypeNameIFC) v; else return Nullable<TypeNameIFC>(); }
        //## SelectEnumerationPut
        void put_TypeNameIFC(TypeNameIFC value) { TextValue val = ENUMERATION_VALUES_ARRAY[(int) value]; putEnumerationValue("TypeNameUpper", val); }
        //## SelectAggregationGet
        bool is_AggregationType() { return IsADBType("TypeNameUpper"); }

        //TList may be AggregationType or list of converible elements
        template <typename TList> void get_AggregationType(TList& lst) { SdaiAggr aggr = getAggrValue("TypeNameUpper"); AggregationTypeSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }
        //## SelectAggregationPut

                //TList may be AggregationType or list of converible elements
        template <typename TList> void put_AggregationType(TList& lst) { AggregationTypeSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("TypeNameUpper", aggr); }
        //## SelectAggregationPutArray

                //TArrayElem[] may be SimpleType[] or array of convertible elements
        template <typename TArrayElem> void put_AggregationType(TArrayElem arr[], size_t n) { AggregationType lst; ArrayToList(arr, n, lst); put_AggregationType(lst); }
        //## SelectNested
        GEN_TYPE_NAME_accessor nestedSelectAccess_GEN_TYPE_NAME() { return GEN_TYPE_NAME_accessor(this); }
        //## SelectGetAsDouble
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
        //## SelectGetAsInt
        Nullable<IntValue> as_int() { IntValue val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiINTEGER, &val)) return val; else return Nullable<IntValue>(); }
        //## SelectGetAsBool
        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
        //## SelectGetAsText
        TextValue as_text() { TextValue val = NULL; sdaiGetAttrBN(m_instance, m_attrName, sdaiSTRING, &val); return val; }
        //## SelectGetAsEntity
        SdaiInstance as_instance() { return getEntityInstance(NULL); }
        //## SelectAccessorEnd
    };

    //## TEMPLATE: EntitiesBegin

        //
        // Entities
        // 

    //## TEMPLATE: EntityBegin

        /// <summary>
        /// Provides utility methods to interact with an instnace of ENTITY_NAME
        /// You also can use object of this C++ class instead of IntValue handle of the OWL instance in any place where the handle is required
        /// </summary>
    class ENTITY_NAME : public virtual /*PARENT_NAME*/Entity
    {
    public:
        /// <summary>
        /// Constructs object of this C++ class that wraps existing SdaiInstance of ENTITY_NAME
        /// </summary>
        /// <param name="instance">An instance to interact with</param>
        ENTITY_NAME(SdaiInstance instance = NULL, TextValue entityName = NULL)
            : Entity(instance, entityName ? entityName : "ENTITY_NAME")
        {}

        //## EntityCreateMethod
                /// <summary>
                /// Create new instace of ENTITY_NAME and returns object of this C++ class to interact with
                /// </summary>
        static ENTITY_NAME Create(SdaiModel model) { SdaiInstance inst = sdaiCreateInstanceBN(model, "ENTITY_NAME"); assert(inst); return inst; }
        //## AttributeSimpleGet

        Nullable<SimpleType> get_ATTR_NAME() { SimpleType val = (SimpleType) 0; if (sdaiGetAttrBN(m_instance, "ATTR_NAME", sdaiTYPE, &val)) return val; else return Nullable<SimpleType>(); }
        //## AttributeSimplePut
        void put_ATTR_NAME(SimpleType value) { sdaiPutAttrBN(m_instance, "ATTR_NAME", sdaiTYPE, &value); }
        //## AttributeTextGet

        TextType get_attr_NAME() { TextType val = NULL; if (sdaiGetAttrBN(m_instance, "ATTR_NAME", sdaiTYPE, &val)) return val; else return NULL; }
        //## AttributeTextPut
        void put_ATTR_NAME(TextType value) { sdaiPutAttrBN(m_instance, "ATTR_NAME", sdaiTYPE, value); }
        //## AttributeEntityGet

        REF_ENTITY get_Attr_NAME();
        //## AttributeEntityPut
        void put_Attr_NAME(REF_ENTITY inst);
        //## AttributeEnumGet

        Nullable<TypeNameIFC> get_ATtr_NAME() { int v = getENUM("ATTR_NAME", ENUMERATION_VALUES_ARRAY); if (v >= 0) return (TypeNameIFC) v; else return Nullable<TypeNameIFC>(); }
        //## AttributeEnumPut
        void put_ATTR_NAME(TypeNameIFC value) { TextValue val = ENUMERATION_VALUES_ARRAY[(int) value]; sdaiPutAttrBN(m_instance, "ATTR_NAME", sdaiENUM, val); }
        //## AttributeSelectAccessor
        GEN_TYPE_NAME_accessor getOrPut_ATTR_NAME() { return GEN_TYPE_NAME_accessor(m_instance, "ATTR_NAME", NULL); }
        //## AttributeAggregationGet

        //TList may be AggregationType or list of converible elements
        template <typename TList> void get_ATTr_NAME(TList& lst) { AggregationTypeSerializer<TList> sr; sr.FromAttr(lst, m_instance, "ATTR_NAME"); }
        //## AttributeAggregationPut

        //TList may be AggregationType or list of converible elements
        template <typename TList> void put_ATTr_NAME(TList& lst) { AggregationTypeSerializer<TList> sr;  sr.ToSdaiAggr(lst, m_instance, "ATTR_NAME"); }
        //## AttributeAggregationPutArray

        //TArrayElem[] may be SimpleType[] or array of convertible elements
        template <typename TArrayElem> void put_ATTr_NAME(TArrayElem arr[], size_t n) { AggregationType lst; ArrayToList(arr, n, lst); put_ATTr_NAME(lst); }
        //## EntityEnd
    };

    //## SelectEntityGetImplementation
    inline REF_ENTITY GEN_TYPE_NAME_accessor::get_REF_ENTITY() { return getEntityInstance("TypeNameUpper"); }
    //## SelectEntityPutImplementation
    inline void GEN_TYPE_NAME_accessor::put_REF_ENTITY(REF_ENTITY inst) { putEntityInstance("TypeNameUpper", inst); }
    //## AttributeEntityGetImplementation
    inline REF_ENTITY ENTITY_NAME::get_Attr_NAME() { SdaiInstance inst = 0; sdaiGetAttrBN(m_instance, "ATTR_NAME", sdaiINSTANCE, &inst); return inst; }
    //## AttributeEntityPutImplementation
    inline void ENTITY_NAME::put_Attr_NAME(REF_ENTITY inst) { SdaiInstance i = inst;  sdaiPutAttrBN(m_instance, "ATTR_NAME", sdaiINSTANCE, (void*) i); }
    //## TEMPLATE: EndFile template part

}

#endif //__RDF_LTD__NAMESPACE_NAME_H
