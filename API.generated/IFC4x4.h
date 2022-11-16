//
// Early-binding C++ API for SDAI (C++ wrappers)
//
#ifndef __RDF_LTD__IFC4x4_H
#define __RDF_LTD__IFC4x4_H

#include    <assert.h>
#include    <list>
#include    <string>

#include	"ifcengine.h"

namespace IFC4x4
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
                m_adb = sdaiCreateEmptyADB();
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

    class IfcActionRequest;
    class IfcActor;
    class IfcActorRole;
    class IfcActuator;
    class IfcActuatorType;
    class IfcAddress;
    class IfcAdvancedBrep;
    class IfcAdvancedBrepWithVoids;
    class IfcAdvancedFace;
    class IfcAirTerminal;
    class IfcAirTerminalBox;
    class IfcAirTerminalBoxType;
    class IfcAirTerminalType;
    class IfcAirToAirHeatRecovery;
    class IfcAirToAirHeatRecoveryType;
    class IfcAlarm;
    class IfcAlarmType;
    class IfcAlignment;
    class IfcAlignmentCant;
    class IfcAlignmentCantSegment;
    class IfcAlignmentHorizontal;
    class IfcAlignmentHorizontalSegment;
    class IfcAlignmentParameterSegment;
    class IfcAlignmentSegment;
    class IfcAlignmentVertical;
    class IfcAlignmentVerticalSegment;
    class IfcAnnotation;
    class IfcAnnotationFillArea;
    class IfcApplication;
    class IfcAppliedValue;
    class IfcApproval;
    class IfcApprovalRelationship;
    class IfcArbitraryClosedProfileDef;
    class IfcArbitraryOpenProfileDef;
    class IfcArbitraryProfileDefWithVoids;
    class IfcAsset;
    class IfcAsymmetricIShapeProfileDef;
    class IfcAudioVisualAppliance;
    class IfcAudioVisualApplianceType;
    class IfcAxis1Placement;
    class IfcAxis2Placement2D;
    class IfcAxis2Placement3D;
    class IfcAxis2PlacementLinear;
    class IfcBeam;
    class IfcBeamType;
    class IfcBearing;
    class IfcBearingType;
    class IfcBlobTexture;
    class IfcBlock;
    class IfcBoiler;
    class IfcBoilerType;
    class IfcBooleanClippingResult;
    class IfcBooleanResult;
    class IfcBorehole;
    class IfcBoundaryCondition;
    class IfcBoundaryCurve;
    class IfcBoundaryEdgeCondition;
    class IfcBoundaryFaceCondition;
    class IfcBoundaryNodeCondition;
    class IfcBoundaryNodeConditionWarping;
    class IfcBoundedCurve;
    class IfcBoundedSurface;
    class IfcBoundingBox;
    class IfcBoxedHalfSpace;
    class IfcBridge;
    class IfcBridgePart;
    class IfcBSplineCurve;
    class IfcBSplineCurveWithKnots;
    class IfcBSplineSurface;
    class IfcBSplineSurfaceWithKnots;
    class IfcBuilding;
    class IfcBuildingElementPart;
    class IfcBuildingElementPartType;
    class IfcBuildingElementProxy;
    class IfcBuildingElementProxyType;
    class IfcBuildingStorey;
    class IfcBuildingSystem;
    class IfcBuiltElement;
    class IfcBuiltElementType;
    class IfcBuiltSystem;
    class IfcBurner;
    class IfcBurnerType;
    class IfcCableCarrierFitting;
    class IfcCableCarrierFittingType;
    class IfcCableCarrierSegment;
    class IfcCableCarrierSegmentType;
    class IfcCableFitting;
    class IfcCableFittingType;
    class IfcCableSegment;
    class IfcCableSegmentType;
    class IfcCaissonFoundation;
    class IfcCaissonFoundationType;
    class IfcCartesianPoint;
    class IfcCartesianPointList;
    class IfcCartesianPointList2D;
    class IfcCartesianPointList3D;
    class IfcCartesianTransformationOperator;
    class IfcCartesianTransformationOperator2D;
    class IfcCartesianTransformationOperator2DnonUniform;
    class IfcCartesianTransformationOperator3D;
    class IfcCartesianTransformationOperator3DnonUniform;
    class IfcCenterLineProfileDef;
    class IfcChiller;
    class IfcChillerType;
    class IfcChimney;
    class IfcChimneyType;
    class IfcCircle;
    class IfcCircleHollowProfileDef;
    class IfcCircleProfileDef;
    class IfcCivilElement;
    class IfcCivilElementType;
    class IfcClassification;
    class IfcClassificationReference;
    class IfcClosedShell;
    class IfcClothoid;
    class IfcCoil;
    class IfcCoilType;
    class IfcColourRgb;
    class IfcColourRgbList;
    class IfcColourSpecification;
    class IfcColumn;
    class IfcColumnType;
    class IfcCommunicationsAppliance;
    class IfcCommunicationsApplianceType;
    class IfcComplementaryData;
    class IfcComplexProperty;
    class IfcComplexPropertyTemplate;
    class IfcCompositeCurve;
    class IfcCompositeCurveOnSurface;
    class IfcCompositeCurveSegment;
    class IfcCompositeProfileDef;
    class IfcCompressor;
    class IfcCompressorType;
    class IfcCondenser;
    class IfcCondenserType;
    class IfcConic;
    class IfcConnectedFaceSet;
    class IfcConnectionCurveGeometry;
    class IfcConnectionGeometry;
    class IfcConnectionPointEccentricity;
    class IfcConnectionPointGeometry;
    class IfcConnectionSurfaceGeometry;
    class IfcConnectionVolumeGeometry;
    class IfcConstraint;
    class IfcConstructionEquipmentResource;
    class IfcConstructionEquipmentResourceType;
    class IfcConstructionMaterialResource;
    class IfcConstructionMaterialResourceType;
    class IfcConstructionProductResource;
    class IfcConstructionProductResourceType;
    class IfcConstructionResource;
    class IfcConstructionResourceType;
    class IfcContext;
    class IfcContextDependentUnit;
    class IfcControl;
    class IfcController;
    class IfcControllerType;
    class IfcConversionBasedUnit;
    class IfcConversionBasedUnitWithOffset;
    class IfcConveyorSegment;
    class IfcConveyorSegmentType;
    class IfcCooledBeam;
    class IfcCooledBeamType;
    class IfcCoolingTower;
    class IfcCoolingTowerType;
    class IfcCoordinateOperation;
    class IfcCoordinateReferenceSystem;
    class IfcCosineSpiral;
    class IfcCostItem;
    class IfcCostSchedule;
    class IfcCostValue;
    class IfcCourse;
    class IfcCourseType;
    class IfcCovering;
    class IfcCoveringType;
    class IfcCrewResource;
    class IfcCrewResourceType;
    class IfcCsgPrimitive3D;
    class IfcCsgSolid;
    class IfcCShapeProfileDef;
    class IfcCurrencyRelationship;
    class IfcCurtainWall;
    class IfcCurtainWallType;
    class IfcCurve;
    class IfcCurveBoundedPlane;
    class IfcCurveBoundedSurface;
    class IfcCurveSegment;
    class IfcCurveStyle;
    class IfcCurveStyleFont;
    class IfcCurveStyleFontAndScaling;
    class IfcCurveStyleFontPattern;
    class IfcCylindricalSurface;
    class IfcDamper;
    class IfcDamperType;
    class IfcDatasetInformation;
    class IfcDatasetReference;
    class IfcDeepFoundation;
    class IfcDeepFoundationType;
    class IfcDerivedProfileDef;
    class IfcDerivedUnit;
    class IfcDerivedUnitElement;
    class IfcDimensionalExponents;
    class IfcDirection;
    class IfcDirectrixCurveSweptAreaSolid;
    class IfcDirectrixDerivedReferenceSweptAreaSolid;
    class IfcDiscreteAccessory;
    class IfcDiscreteAccessoryType;
    class IfcDistributionBoard;
    class IfcDistributionBoardType;
    class IfcDistributionChamberElement;
    class IfcDistributionChamberElementType;
    class IfcDistributionCircuit;
    class IfcDistributionControlElement;
    class IfcDistributionControlElementType;
    class IfcDistributionElement;
    class IfcDistributionElementType;
    class IfcDistributionFlowElement;
    class IfcDistributionFlowElementType;
    class IfcDistributionPort;
    class IfcDistributionSystem;
    class IfcDocumentInformation;
    class IfcDocumentInformationRelationship;
    class IfcDocumentReference;
    class IfcDoor;
    class IfcDoorLiningProperties;
    class IfcDoorPanelProperties;
    class IfcDoorStyle;
    class IfcDoorType;
    class IfcDraughtingPreDefinedColour;
    class IfcDraughtingPreDefinedCurveFont;
    class IfcDuctFitting;
    class IfcDuctFittingType;
    class IfcDuctSegment;
    class IfcDuctSegmentType;
    class IfcDuctSilencer;
    class IfcDuctSilencerType;
    class IfcEarthingElement;
    class IfcEarthingElementType;
    class IfcEarthworksCut;
    class IfcEarthworksElement;
    class IfcEarthworksFill;
    class IfcEdge;
    class IfcEdgeCurve;
    class IfcEdgeLoop;
    class IfcElectricAppliance;
    class IfcElectricApplianceType;
    class IfcElectricDistributionBoard;
    class IfcElectricDistributionBoardType;
    class IfcElectricFlowStorageDevice;
    class IfcElectricFlowStorageDeviceType;
    class IfcElectricFlowTreatmentDevice;
    class IfcElectricFlowTreatmentDeviceType;
    class IfcElectricGenerator;
    class IfcElectricGeneratorType;
    class IfcElectricMotor;
    class IfcElectricMotorType;
    class IfcElectricTimeControl;
    class IfcElectricTimeControlType;
    class IfcElement;
    class IfcElementarySurface;
    class IfcElementAssembly;
    class IfcElementAssemblyType;
    class IfcElementComponent;
    class IfcElementComponentType;
    class IfcElementQuantity;
    class IfcElementType;
    class IfcEllipse;
    class IfcEllipseProfileDef;
    class IfcEnergyConversionDevice;
    class IfcEnergyConversionDeviceType;
    class IfcEngine;
    class IfcEngineType;
    class IfcEvaporativeCooler;
    class IfcEvaporativeCoolerType;
    class IfcEvaporator;
    class IfcEvaporatorType;
    class IfcEvent;
    class IfcEventTime;
    class IfcEventType;
    class IfcExcavation;
    class IfcExtendedProperties;
    class IfcExternalInformation;
    class IfcExternallyDefinedHatchStyle;
    class IfcExternallyDefinedSurfaceStyle;
    class IfcExternallyDefinedTextFont;
    class IfcExternalReference;
    class IfcExternalReferenceRelationship;
    class IfcExternalSpatialElement;
    class IfcExternalSpatialStructureElement;
    class IfcExtrudedAreaSolid;
    class IfcExtrudedAreaSolidTapered;
    class IfcFace;
    class IfcFaceBasedSurfaceModel;
    class IfcFaceBound;
    class IfcFaceOuterBound;
    class IfcFaceSurface;
    class IfcFacetedBrep;
    class IfcFacetedBrepWithVoids;
    class IfcFacility;
    class IfcFacilityPart;
    class IfcFacilityPartCommon;
    class IfcFailureConnectionCondition;
    class IfcFan;
    class IfcFanType;
    class IfcFastener;
    class IfcFastenerType;
    class IfcFeatureElement;
    class IfcFeatureElementAddition;
    class IfcFeatureElementSubtraction;
    class IfcFillAreaStyle;
    class IfcFillAreaStyleHatching;
    class IfcFillAreaStyleTiles;
    class IfcFillElement;
    class IfcFillElementType;
    class IfcFilter;
    class IfcFilterType;
    class IfcFireSuppressionTerminal;
    class IfcFireSuppressionTerminalType;
    class IfcFixedReferenceSweptAreaSolid;
    class IfcFlowController;
    class IfcFlowControllerType;
    class IfcFlowFitting;
    class IfcFlowFittingType;
    class IfcFlowInstrument;
    class IfcFlowInstrumentType;
    class IfcFlowMeter;
    class IfcFlowMeterType;
    class IfcFlowMovingDevice;
    class IfcFlowMovingDeviceType;
    class IfcFlowSegment;
    class IfcFlowSegmentType;
    class IfcFlowStorageDevice;
    class IfcFlowStorageDeviceType;
    class IfcFlowTerminal;
    class IfcFlowTerminalType;
    class IfcFlowTreatmentDevice;
    class IfcFlowTreatmentDeviceType;
    class IfcFooting;
    class IfcFootingType;
    class IfcFurnishingElement;
    class IfcFurnishingElementType;
    class IfcFurniture;
    class IfcFurnitureType;
    class IfcGeographicCRS;
    class IfcGeographicElement;
    class IfcGeographicElementType;
    class IfcGeometricCurveSet;
    class IfcGeometricRepresentationContext;
    class IfcGeometricRepresentationItem;
    class IfcGeometricRepresentationSubContext;
    class IfcGeometricSet;
    class IfcGeomodel;
    class IfcGeoScienceElement;
    class IfcGeoScienceFeature;
    class IfcGeoScienceModel;
    class IfcGeoScienceObservation;
    class IfcGeoslice;
    class IfcGeotechnicalAssembly;
    class IfcGeotechnicalElement;
    class IfcGeotechnicalStratum;
    class IfcGeotechTypicalSection;
    class IfcGradientCurve;
    class IfcGrid;
    class IfcGridAxis;
    class IfcGridPlacement;
    class IfcGroundReinforcementElement;
    class IfcGroup;
    class IfcHalfSpaceSolid;
    class IfcHeatExchanger;
    class IfcHeatExchangerType;
    class IfcHumidifier;
    class IfcHumidifierType;
    class IfcImageTexture;
    class IfcImpactProtectionDevice;
    class IfcImpactProtectionDeviceType;
    class IfcImprovedGround;
    class IfcIndexedColourMap;
    class IfcIndexedPolyCurve;
    class IfcIndexedPolygonalFace;
    class IfcIndexedPolygonalFaceWithVoids;
    class IfcIndexedTextureMap;
    class IfcIndexedTriangleTextureMap;
    class IfcIntegerVoxelData;
    class IfcInterceptor;
    class IfcInterceptorType;
    class IfcIntersectionCurve;
    class IfcInventory;
    class IfcIrregularTimeSeries;
    class IfcIrregularTimeSeriesValue;
    class IfcIShapeProfileDef;
    class IfcJunctionBox;
    class IfcJunctionBoxType;
    class IfcKerb;
    class IfcKerbType;
    class IfcLabelVoxelData;
    class IfcLaborResource;
    class IfcLaborResourceType;
    class IfcLagTime;
    class IfcLamp;
    class IfcLampType;
    class IfcLibraryInformation;
    class IfcLibraryReference;
    class IfcLightDistributionData;
    class IfcLightFixture;
    class IfcLightFixtureType;
    class IfcLightIntensityDistribution;
    class IfcLightSource;
    class IfcLightSourceAmbient;
    class IfcLightSourceDirectional;
    class IfcLightSourceGoniometric;
    class IfcLightSourcePositional;
    class IfcLightSourceSpot;
    class IfcLine;
    class IfcLinearElement;
    class IfcLinearPlacement;
    class IfcLinearPositioningElement;
    class IfcLinearZone;
    class IfcLiquidTerminal;
    class IfcLiquidTerminalType;
    class IfcLocalPlacement;
    class IfcLogicalVoxelData;
    class IfcLoop;
    class IfcLShapeProfileDef;
    class IfcManifoldSolidBrep;
    class IfcMapConversion;
    class IfcMapConversionScaled;
    class IfcMappedItem;
    class IfcMarineFacility;
    class IfcMarinePart;
    class IfcMaterial;
    class IfcMaterialClassificationRelationship;
    class IfcMaterialConstituent;
    class IfcMaterialConstituentSet;
    class IfcMaterialDefinition;
    class IfcMaterialDefinitionRepresentation;
    class IfcMaterialLayer;
    class IfcMaterialLayerSet;
    class IfcMaterialLayerSetUsage;
    class IfcMaterialLayerWithOffsets;
    class IfcMaterialList;
    class IfcMaterialProfile;
    class IfcMaterialProfileSet;
    class IfcMaterialProfileSetUsage;
    class IfcMaterialProfileSetUsageTapering;
    class IfcMaterialProfileWithOffsets;
    class IfcMaterialProperties;
    class IfcMaterialRelationship;
    class IfcMaterialUsageDefinition;
    class IfcMeasureWithUnit;
    class IfcMechanicalFastener;
    class IfcMechanicalFastenerType;
    class IfcMedicalDevice;
    class IfcMedicalDeviceType;
    class IfcMember;
    class IfcMemberType;
    class IfcMetric;
    class IfcMirroredProfileDef;
    class IfcMobileTelecommunicationsAppliance;
    class IfcMobileTelecommunicationsApplianceType;
    class IfcMonetaryUnit;
    class IfcMooringDevice;
    class IfcMooringDeviceType;
    class IfcMotorConnection;
    class IfcMotorConnectionType;
    class IfcNamedUnit;
    class IfcNavigationElement;
    class IfcNavigationElementType;
    class IfcObject;
    class IfcObjectDefinition;
    class IfcObjective;
    class IfcObjectPlacement;
    class IfcObservation;
    class IfcOccupant;
    class IfcOffsetCurve;
    class IfcOffsetCurve2D;
    class IfcOffsetCurve3D;
    class IfcOffsetCurveByDistances;
    class IfcOpenCrossProfileDef;
    class IfcOpeningElement;
    class IfcOpenShell;
    class IfcOrganization;
    class IfcOrganizationRelationship;
    class IfcOrientedEdge;
    class IfcOuterBoundaryCurve;
    class IfcOutlet;
    class IfcOutletType;
    class IfcOwnerHistory;
    class IfcParameterizedProfileDef;
    class IfcPath;
    class IfcPavement;
    class IfcPavementType;
    class IfcPcurve;
    class IfcPerformanceHistory;
    class IfcPermeableCoveringProperties;
    class IfcPermit;
    class IfcPerson;
    class IfcPersonAndOrganization;
    class IfcPhysicalComplexQuantity;
    class IfcPhysicalQuantity;
    class IfcPhysicalSimpleQuantity;
    class IfcPile;
    class IfcPileType;
    class IfcPipeFitting;
    class IfcPipeFittingType;
    class IfcPipeSegment;
    class IfcPipeSegmentType;
    class IfcPixelTexture;
    class IfcPlacement;
    class IfcPlanarBox;
    class IfcPlanarExtent;
    class IfcPlane;
    class IfcPlate;
    class IfcPlateType;
    class IfcPoint;
    class IfcPointByDistanceExpression;
    class IfcPointOnCurve;
    class IfcPointOnSurface;
    class IfcPolygonalBoundedHalfSpace;
    class IfcPolygonalFaceSet;
    class IfcPolyline;
    class IfcPolyLoop;
    class IfcPolynomialCurve;
    class IfcPort;
    class IfcPositioningElement;
    class IfcPostalAddress;
    class IfcPreDefinedColour;
    class IfcPreDefinedCurveFont;
    class IfcPreDefinedItem;
    class IfcPreDefinedProperties;
    class IfcPreDefinedPropertySet;
    class IfcPreDefinedTextFont;
    class IfcPresentationItem;
    class IfcPresentationLayerAssignment;
    class IfcPresentationLayerWithStyle;
    class IfcPresentationStyle;
    class IfcProcedure;
    class IfcProcedureType;
    class IfcProcess;
    class IfcProduct;
    class IfcProductDefinitionShape;
    class IfcProductRepresentation;
    class IfcProfileDef;
    class IfcProfileProperties;
    class IfcProject;
    class IfcProjectedCRS;
    class IfcProjectionElement;
    class IfcProjectLibrary;
    class IfcProjectOrder;
    class IfcProperty;
    class IfcPropertyAbstraction;
    class IfcPropertyBoundedValue;
    class IfcPropertyDefinition;
    class IfcPropertyDependencyRelationship;
    class IfcPropertyEnumeratedValue;
    class IfcPropertyEnumeration;
    class IfcPropertyListValue;
    class IfcPropertyReferenceValue;
    class IfcPropertySet;
    class IfcPropertySetDefinition;
    class IfcPropertySetTemplate;
    class IfcPropertySingleValue;
    class IfcPropertyTableValue;
    class IfcPropertyTemplate;
    class IfcPropertyTemplateDefinition;
    class IfcProtectiveDevice;
    class IfcProtectiveDeviceTrippingUnit;
    class IfcProtectiveDeviceTrippingUnitType;
    class IfcProtectiveDeviceType;
    class IfcProxy;
    class IfcPump;
    class IfcPumpType;
    class IfcQuantityArea;
    class IfcQuantityCount;
    class IfcQuantityLength;
    class IfcQuantitySet;
    class IfcQuantityTime;
    class IfcQuantityVolume;
    class IfcQuantityWeight;
    class IfcRail;
    class IfcRailing;
    class IfcRailingType;
    class IfcRailType;
    class IfcRailway;
    class IfcRailwayPart;
    class IfcRamp;
    class IfcRampFlight;
    class IfcRampFlightType;
    class IfcRampType;
    class IfcRationalBSplineCurveWithKnots;
    class IfcRationalBSplineSurfaceWithKnots;
    class IfcRealVoxelData;
    class IfcRectangleHollowProfileDef;
    class IfcRectangleProfileDef;
    class IfcRectangularPyramid;
    class IfcRectangularTrimmedSurface;
    class IfcRecurrencePattern;
    class IfcReference;
    class IfcReferent;
    class IfcRegularTimeSeries;
    class IfcReinforcementBarProperties;
    class IfcReinforcementDefinitionProperties;
    class IfcReinforcingBar;
    class IfcReinforcingBarType;
    class IfcReinforcingElement;
    class IfcReinforcingElementType;
    class IfcReinforcingMesh;
    class IfcReinforcingMeshType;
    class IfcRelAdheresToElement;
    class IfcRelAggregates;
    class IfcRelAssigns;
    class IfcRelAssignsToActor;
    class IfcRelAssignsToControl;
    class IfcRelAssignsToGroup;
    class IfcRelAssignsToGroupByFactor;
    class IfcRelAssignsToProcess;
    class IfcRelAssignsToProduct;
    class IfcRelAssignsToResource;
    class IfcRelAssociates;
    class IfcRelAssociatesApproval;
    class IfcRelAssociatesClassification;
    class IfcRelAssociatesConstraint;
    class IfcRelAssociatesDataset;
    class IfcRelAssociatesDocument;
    class IfcRelAssociatesLibrary;
    class IfcRelAssociatesMaterial;
    class IfcRelAssociatesProfileDef;
    class IfcRelationship;
    class IfcRelConnects;
    class IfcRelConnectsElements;
    class IfcRelConnectsPathElements;
    class IfcRelConnectsPorts;
    class IfcRelConnectsPortToElement;
    class IfcRelConnectsStructuralActivity;
    class IfcRelConnectsStructuralMember;
    class IfcRelConnectsWithEccentricity;
    class IfcRelConnectsWithRealizingElements;
    class IfcRelContainedInSpatialStructure;
    class IfcRelCoversBldgElements;
    class IfcRelCoversSpaces;
    class IfcRelDeclares;
    class IfcRelDecomposes;
    class IfcRelDefines;
    class IfcRelDefinesByObject;
    class IfcRelDefinesByProperties;
    class IfcRelDefinesByTemplate;
    class IfcRelDefinesByType;
    class IfcRelFillsElement;
    class IfcRelFlowControlElements;
    class IfcRelInterferesElements;
    class IfcRelNests;
    class IfcRelPositions;
    class IfcRelProjectsElement;
    class IfcRelReferencedInSpatialStructure;
    class IfcRelSequence;
    class IfcRelServicesBuildings;
    class IfcRelSpaceBoundary;
    class IfcRelSpaceBoundary1stLevel;
    class IfcRelSpaceBoundary2ndLevel;
    class IfcRelVoidsElement;
    class IfcReparametrisedCompositeCurveSegment;
    class IfcRepresentation;
    class IfcRepresentationContext;
    class IfcRepresentationItem;
    class IfcRepresentationMap;
    class IfcResource;
    class IfcResourceApprovalRelationship;
    class IfcResourceConstraintRelationship;
    class IfcResourceLevelRelationship;
    class IfcResourceTime;
    class IfcRevolvedAreaSolid;
    class IfcRevolvedAreaSolidTapered;
    class IfcRightCircularCone;
    class IfcRightCircularCylinder;
    class IfcRigidOperation;
    class IfcRoad;
    class IfcRoadPart;
    class IfcRoof;
    class IfcRoofType;
    class IfcRoot;
    class IfcRoundedRectangleProfileDef;
    class IfcSanitaryTerminal;
    class IfcSanitaryTerminalType;
    class IfcSchedulingTime;
    class IfcSeamCurve;
    class IfcSecondOrderPolynomialSpiral;
    class IfcSectionedSolid;
    class IfcSectionedSolidHorizontal;
    class IfcSectionedSpine;
    class IfcSectionedSurface;
    class IfcSectionProperties;
    class IfcSectionReinforcementProperties;
    class IfcSegment;
    class IfcSegmentedReferenceCurve;
    class IfcSensor;
    class IfcSensorType;
    class IfcSeventhOrderPolynomialSpiral;
    class IfcShadingDevice;
    class IfcShadingDeviceType;
    class IfcShapeAspect;
    class IfcShapeModel;
    class IfcShapeRepresentation;
    class IfcShellBasedSurfaceModel;
    class IfcSign;
    class IfcSignal;
    class IfcSignalType;
    class IfcSignType;
    class IfcSimpleProperty;
    class IfcSimplePropertyTemplate;
    class IfcSineSpiral;
    class IfcSite;
    class IfcSIUnit;
    class IfcSlab;
    class IfcSlabType;
    class IfcSlippageConnectionCondition;
    class IfcSolarDevice;
    class IfcSolarDeviceType;
    class IfcSolidModel;
    class IfcSolidStratum;
    class IfcSpace;
    class IfcSpaceHeater;
    class IfcSpaceHeaterType;
    class IfcSpaceType;
    class IfcSpatialElement;
    class IfcSpatialElementType;
    class IfcSpatialStructureElement;
    class IfcSpatialStructureElementType;
    class IfcSpatialZone;
    class IfcSpatialZoneType;
    class IfcSphere;
    class IfcSphericalSurface;
    class IfcSpiral;
    class IfcStackTerminal;
    class IfcStackTerminalType;
    class IfcStair;
    class IfcStairFlight;
    class IfcStairFlightType;
    class IfcStairType;
    class IfcStructuralAction;
    class IfcStructuralActivity;
    class IfcStructuralAnalysisModel;
    class IfcStructuralConnection;
    class IfcStructuralConnectionCondition;
    class IfcStructuralCurveAction;
    class IfcStructuralCurveConnection;
    class IfcStructuralCurveMember;
    class IfcStructuralCurveMemberVarying;
    class IfcStructuralCurveReaction;
    class IfcStructuralItem;
    class IfcStructuralLinearAction;
    class IfcStructuralLoad;
    class IfcStructuralLoadCase;
    class IfcStructuralLoadConfiguration;
    class IfcStructuralLoadGroup;
    class IfcStructuralLoadLinearForce;
    class IfcStructuralLoadOrResult;
    class IfcStructuralLoadPlanarForce;
    class IfcStructuralLoadSingleDisplacement;
    class IfcStructuralLoadSingleDisplacementDistortion;
    class IfcStructuralLoadSingleForce;
    class IfcStructuralLoadSingleForceWarping;
    class IfcStructuralLoadStatic;
    class IfcStructuralLoadTemperature;
    class IfcStructuralMember;
    class IfcStructuralPlanarAction;
    class IfcStructuralPointAction;
    class IfcStructuralPointConnection;
    class IfcStructuralPointReaction;
    class IfcStructuralReaction;
    class IfcStructuralResultGroup;
    class IfcStructuralSurfaceAction;
    class IfcStructuralSurfaceConnection;
    class IfcStructuralSurfaceMember;
    class IfcStructuralSurfaceMemberVarying;
    class IfcStructuralSurfaceReaction;
    class IfcStyledItem;
    class IfcStyledRepresentation;
    class IfcStyleModel;
    class IfcSubContractResource;
    class IfcSubContractResourceType;
    class IfcSubedge;
    class IfcSurface;
    class IfcSurfaceCurve;
    class IfcSurfaceCurveSweptAreaSolid;
    class IfcSurfaceFeature;
    class IfcSurfaceOfLinearExtrusion;
    class IfcSurfaceOfRevolution;
    class IfcSurfaceReinforcementArea;
    class IfcSurfaceStyle;
    class IfcSurfaceStyleLighting;
    class IfcSurfaceStyleRefraction;
    class IfcSurfaceStyleRendering;
    class IfcSurfaceStyleShading;
    class IfcSurfaceStyleWithTextures;
    class IfcSurfaceTexture;
    class IfcSweptAreaSolid;
    class IfcSweptDiskSolid;
    class IfcSweptDiskSolidPolygonal;
    class IfcSweptSurface;
    class IfcSwitchingDevice;
    class IfcSwitchingDeviceType;
    class IfcSystem;
    class IfcSystemFurnitureElement;
    class IfcSystemFurnitureElementType;
    class IfcTable;
    class IfcTableColumn;
    class IfcTableRow;
    class IfcTank;
    class IfcTankType;
    class IfcTask;
    class IfcTaskTime;
    class IfcTaskTimeRecurring;
    class IfcTaskType;
    class IfcTelecomAddress;
    class IfcTendon;
    class IfcTendonAnchor;
    class IfcTendonAnchorType;
    class IfcTendonConduit;
    class IfcTendonConduitType;
    class IfcTendonType;
    class IfcTessellatedFaceSet;
    class IfcTessellatedItem;
    class IfcTextLiteral;
    class IfcTextLiteralWithExtent;
    class IfcTextStyle;
    class IfcTextStyleFontModel;
    class IfcTextStyleForDefinedFont;
    class IfcTextStyleTextModel;
    class IfcTextureCoordinate;
    class IfcTextureCoordinateGenerator;
    class IfcTextureMap;
    class IfcTextureVertex;
    class IfcTextureVertexList;
    class IfcThirdOrderPolynomialSpiral;
    class IfcTimePeriod;
    class IfcTimeSeries;
    class IfcTimeSeriesValue;
    class IfcTopologicalRepresentationItem;
    class IfcTopologyRepresentation;
    class IfcToroidalSurface;
    class IfcTrackElement;
    class IfcTrackElementType;
    class IfcTransformer;
    class IfcTransformerType;
    class IfcTransportationDevice;
    class IfcTransportationDeviceType;
    class IfcTransportElement;
    class IfcTransportElementType;
    class IfcTrapeziumProfileDef;
    class IfcTriangulatedFaceSet;
    class IfcTriangulatedIrregularNetwork;
    class IfcTrimmedCurve;
    class IfcTShapeProfileDef;
    class IfcTubeBundle;
    class IfcTubeBundleType;
    class IfcTunnel;
    class IfcTunnelPart;
    class IfcTypeObject;
    class IfcTypeProcess;
    class IfcTypeProduct;
    class IfcTypeResource;
    class IfcUndergroundExcavation;
    class IfcUnitaryControlElement;
    class IfcUnitaryControlElementType;
    class IfcUnitaryEquipment;
    class IfcUnitaryEquipmentType;
    class IfcUnitAssignment;
    class IfcUShapeProfileDef;
    class IfcValve;
    class IfcValveType;
    class IfcVector;
    class IfcVectorVoxelData;
    class IfcVehicle;
    class IfcVehicleType;
    class IfcVertex;
    class IfcVertexLoop;
    class IfcVertexPoint;
    class IfcVibrationDamper;
    class IfcVibrationDamperType;
    class IfcVibrationIsolator;
    class IfcVibrationIsolatorType;
    class IfcVirtualElement;
    class IfcVirtualGridIntersection;
    class IfcVoidingFeature;
    class IfcVoidStratum;
    class IfcVoxelData;
    class IfcVoxelGrid;
    class IfcWall;
    class IfcWallStandardCase;
    class IfcWallType;
    class IfcWasteTerminal;
    class IfcWasteTerminalType;
    class IfcWaterStratum;
    class IfcWellKnownText;
    class IfcWindow;
    class IfcWindowLiningProperties;
    class IfcWindowPanelProperties;
    class IfcWindowStyle;
    class IfcWindowType;
    class IfcWorkCalendar;
    class IfcWorkControl;
    class IfcWorkPlan;
    class IfcWorkSchedule;
    class IfcWorkTime;
    class IfcZone;
    class IfcZShapeProfileDef;

    class IfcActorSelect;
    class IfcActorSelect_get;
    class IfcActorSelect_put;
    class IfcAppliedValueSelect;
    class IfcAppliedValueSelect_get;
    class IfcAppliedValueSelect_put;
    class IfcAxis2Placement;
    class IfcAxis2Placement_get;
    class IfcAxis2Placement_put;
    class IfcBendingParameterSelect;
    class IfcBendingParameterSelect_get;
    class IfcBendingParameterSelect_put;
    class IfcBooleanOperand;
    class IfcBooleanOperand_get;
    class IfcBooleanOperand_put;
    class IfcClassificationReferenceSelect;
    class IfcClassificationReferenceSelect_get;
    class IfcClassificationReferenceSelect_put;
    class IfcClassificationSelect;
    class IfcClassificationSelect_get;
    class IfcClassificationSelect_put;
    class IfcColour;
    class IfcColour_get;
    class IfcColour_put;
    class IfcColourOrFactor;
    class IfcColourOrFactor_get;
    class IfcColourOrFactor_put;
    class IfcCoordinateReferenceSystemSelect;
    class IfcCoordinateReferenceSystemSelect_get;
    class IfcCoordinateReferenceSystemSelect_put;
    class IfcCsgSelect;
    class IfcCsgSelect_get;
    class IfcCsgSelect_put;
    class IfcCurveFontOrScaledCurveFontSelect;
    class IfcCurveFontOrScaledCurveFontSelect_get;
    class IfcCurveFontOrScaledCurveFontSelect_put;
    class IfcCurveMeasureSelect;
    class IfcCurveMeasureSelect_get;
    class IfcCurveMeasureSelect_put;
    class IfcCurveOnSurface;
    class IfcCurveOnSurface_get;
    class IfcCurveOnSurface_put;
    class IfcCurveOrEdgeCurve;
    class IfcCurveOrEdgeCurve_get;
    class IfcCurveOrEdgeCurve_put;
    class IfcCurveStyleFontSelect;
    class IfcCurveStyleFontSelect_get;
    class IfcCurveStyleFontSelect_put;
    class IfcDatasetSelect;
    class IfcDatasetSelect_get;
    class IfcDatasetSelect_put;
    class IfcDefinitionSelect;
    class IfcDefinitionSelect_get;
    class IfcDefinitionSelect_put;
    class IfcDerivedMeasureValue;
    class IfcDerivedMeasureValue_get;
    class IfcDerivedMeasureValue_put;
    class IfcDocumentSelect;
    class IfcDocumentSelect_get;
    class IfcDocumentSelect_put;
    class IfcFillStyleSelect;
    class IfcFillStyleSelect_get;
    class IfcFillStyleSelect_put;
    class IfcGeometricSetSelect;
    class IfcGeometricSetSelect_get;
    class IfcGeometricSetSelect_put;
    class IfcGridPlacementDirectionSelect;
    class IfcGridPlacementDirectionSelect_get;
    class IfcGridPlacementDirectionSelect_put;
    class IfcHatchLineDistanceSelect;
    class IfcHatchLineDistanceSelect_get;
    class IfcHatchLineDistanceSelect_put;
    class IfcInterferenceSelect;
    class IfcInterferenceSelect_get;
    class IfcInterferenceSelect_put;
    class IfcLayeredItem;
    class IfcLayeredItem_get;
    class IfcLayeredItem_put;
    class IfcLibrarySelect;
    class IfcLibrarySelect_get;
    class IfcLibrarySelect_put;
    class IfcLightDistributionDataSourceSelect;
    class IfcLightDistributionDataSourceSelect_get;
    class IfcLightDistributionDataSourceSelect_put;
    class IfcMaterialSelect;
    class IfcMaterialSelect_get;
    class IfcMaterialSelect_put;
    class IfcMeasureValue;
    class IfcMeasureValue_get;
    class IfcMeasureValue_put;
    class IfcMetricValueSelect;
    class IfcMetricValueSelect_get;
    class IfcMetricValueSelect_put;
    class IfcModulusOfRotationalSubgradeReactionSelect;
    class IfcModulusOfRotationalSubgradeReactionSelect_get;
    class IfcModulusOfRotationalSubgradeReactionSelect_put;
    class IfcModulusOfSubgradeReactionSelect;
    class IfcModulusOfSubgradeReactionSelect_get;
    class IfcModulusOfSubgradeReactionSelect_put;
    class IfcModulusOfTranslationalSubgradeReactionSelect;
    class IfcModulusOfTranslationalSubgradeReactionSelect_get;
    class IfcModulusOfTranslationalSubgradeReactionSelect_put;
    class IfcObjectReferenceSelect;
    class IfcObjectReferenceSelect_get;
    class IfcObjectReferenceSelect_put;
    class IfcPointOrVertexPoint;
    class IfcPointOrVertexPoint_get;
    class IfcPointOrVertexPoint_put;
    class IfcProcessSelect;
    class IfcProcessSelect_get;
    class IfcProcessSelect_put;
    class IfcProductRepresentationSelect;
    class IfcProductRepresentationSelect_get;
    class IfcProductRepresentationSelect_put;
    class IfcProductSelect;
    class IfcProductSelect_get;
    class IfcProductSelect_put;
    class IfcPropertySetDefinitionSelect;
    class IfcPropertySetDefinitionSelect_get;
    class IfcPropertySetDefinitionSelect_put;
    class IfcResourceObjectSelect;
    class IfcResourceObjectSelect_get;
    class IfcResourceObjectSelect_put;
    class IfcResourceSelect;
    class IfcResourceSelect_get;
    class IfcResourceSelect_put;
    class IfcRotationalStiffnessSelect;
    class IfcRotationalStiffnessSelect_get;
    class IfcRotationalStiffnessSelect_put;
    class IfcSegmentIndexSelect;
    class IfcSegmentIndexSelect_get;
    class IfcSegmentIndexSelect_put;
    class IfcShell;
    class IfcShell_get;
    class IfcShell_put;
    class IfcSimpleValue;
    class IfcSimpleValue_get;
    class IfcSimpleValue_put;
    class IfcSizeSelect;
    class IfcSizeSelect_get;
    class IfcSizeSelect_put;
    class IfcSolidOrShell;
    class IfcSolidOrShell_get;
    class IfcSolidOrShell_put;
    class IfcSpaceBoundarySelect;
    class IfcSpaceBoundarySelect_get;
    class IfcSpaceBoundarySelect_put;
    class IfcSpatialReferenceSelect;
    class IfcSpatialReferenceSelect_get;
    class IfcSpatialReferenceSelect_put;
    class IfcSpecularHighlightSelect;
    class IfcSpecularHighlightSelect_get;
    class IfcSpecularHighlightSelect_put;
    class IfcStructuralActivityAssignmentSelect;
    class IfcStructuralActivityAssignmentSelect_get;
    class IfcStructuralActivityAssignmentSelect_put;
    class IfcSurfaceOrFaceSurface;
    class IfcSurfaceOrFaceSurface_get;
    class IfcSurfaceOrFaceSurface_put;
    class IfcSurfaceStyleElementSelect;
    class IfcSurfaceStyleElementSelect_get;
    class IfcSurfaceStyleElementSelect_put;
    class IfcTextFontSelect;
    class IfcTextFontSelect_get;
    class IfcTextFontSelect_put;
    class IfcTimeOrRatioSelect;
    class IfcTimeOrRatioSelect_get;
    class IfcTimeOrRatioSelect_put;
    class IfcTranslationalStiffnessSelect;
    class IfcTranslationalStiffnessSelect_get;
    class IfcTranslationalStiffnessSelect_put;
    class IfcTrimmingSelect;
    class IfcTrimmingSelect_get;
    class IfcTrimmingSelect_put;
    class IfcUnit;
    class IfcUnit_get;
    class IfcUnit_put;
    class IfcValue;
    class IfcValue_get;
    class IfcValue_put;
    class IfcVectorOrDirection;
    class IfcVectorOrDirection_get;
    class IfcVectorOrDirection_put;
    class IfcWarpingStiffnessSelect;
    class IfcWarpingStiffnessSelect_get;
    class IfcWarpingStiffnessSelect_put;

        //
        // Enumerations
        //

    enum class IfcActionRequestTypeEnum
    {
        EMAIL = 0,
        FAX = 1,
        PHONE = 2,
        POST = 3,
        VERBAL = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcActionSourceTypeEnum
    {
        DEAD_LOAD_G = 0,
        COMPLETION_G1 = 1,
        LIVE_LOAD_Q = 2,
        SNOW_S = 3,
        WIND_W = 4,
        PRESTRESSING_P = 5,
        SETTLEMENT_U = 6,
        TEMPERATURE_T = 7,
        EARTHQUAKE_E = 8,
        FIRE = 9,
        IMPULSE = 10,
        IMPACT = 11,
        TRANSPORT = 12,
        ERECTION = 13,
        PROPPING = 14,
        SYSTEM_IMPERFECTION = 15,
        SHRINKAGE = 16,
        CREEP = 17,
        LACK_OF_FIT = 18,
        BUOYANCY = 19,
        ICE = 20,
        CURRENT = 21,
        WAVE = 22,
        RAIN = 23,
        BRAKES = 24,
        USERDEFINED = 25,
        NOTDEFINED = 26,
        ___unk = -1
    };

    enum class IfcActionTypeEnum
    {
        PERMANENT_G = 0,
        VARIABLE_Q = 1,
        EXTRAORDINARY_A = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcActuatorTypeEnum
    {
        ELECTRICACTUATOR = 0,
        HANDOPERATEDACTUATOR = 1,
        HYDRAULICACTUATOR = 2,
        PNEUMATICACTUATOR = 3,
        THERMOSTATICACTUATOR = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcAddressTypeEnum
    {
        OFFICE = 0,
        SITE = 1,
        HOME = 2,
        DISTRIBUTIONPOINT = 3,
        USERDEFINED = 4,
        ___unk = -1
    };

    enum class IfcAirTerminalBoxTypeEnum
    {
        CONSTANTFLOW = 0,
        VARIABLEFLOWPRESSUREDEPENDANT = 1,
        VARIABLEFLOWPRESSUREINDEPENDANT = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcAirTerminalTypeEnum
    {
        DIFFUSER = 0,
        GRILLE = 1,
        LOUVRE = 2,
        REGISTER = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcAirToAirHeatRecoveryTypeEnum
    {
        FIXEDPLATECOUNTERFLOWEXCHANGER = 0,
        FIXEDPLATECROSSFLOWEXCHANGER = 1,
        FIXEDPLATEPARALLELFLOWEXCHANGER = 2,
        ROTARYWHEEL = 3,
        RUNAROUNDCOILLOOP = 4,
        HEATPIPE = 5,
        TWINTOWERENTHALPYRECOVERYLOOPS = 6,
        THERMOSIPHONSEALEDTUBEHEATEXCHANGERS = 7,
        THERMOSIPHONCOILTYPEHEATEXCHANGERS = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcAlarmTypeEnum
    {
        BELL = 0,
        BREAKGLASSBUTTON = 1,
        LIGHT = 2,
        MANUALPULLBOX = 3,
        SIREN = 4,
        WHISTLE = 5,
        RAILWAYCROCODILE = 6,
        RAILWAYDETONATOR = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcAlignmentCantSegmentTypeEnum
    {
        BLOSSCURVE = 0,
        CONSTANTCANT = 1,
        COSINECURVE = 2,
        HELMERTCURVE = 3,
        LINEARTRANSITION = 4,
        SINECURVE = 5,
        VIENNESEBEND = 6,
        ___unk = -1
    };

    enum class IfcAlignmentHorizontalSegmentTypeEnum
    {
        LINE = 0,
        CIRCULARARC = 1,
        CLOTHOID = 2,
        CUBIC = 3,
        HELMERTCURVE = 4,
        BLOSSCURVE = 5,
        COSINECURVE = 6,
        SINECURVE = 7,
        VIENNESEBEND = 8,
        ___unk = -1
    };

    enum class IfcAlignmentTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcAlignmentVerticalSegmentTypeEnum
    {
        CONSTANTGRADIENT = 0,
        CIRCULARARC = 1,
        PARABOLICARC = 2,
        CLOTHOID = 3,
        ___unk = -1
    };

    enum class IfcAnalysisModelTypeEnum
    {
        IN_PLANE_LOADING_2D = 0,
        OUT_PLANE_LOADING_2D = 1,
        LOADING_3D = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcAnalysisTheoryTypeEnum
    {
        FIRST_ORDER_THEORY = 0,
        SECOND_ORDER_THEORY = 1,
        THIRD_ORDER_THEORY = 2,
        FULL_NONLINEAR_THEORY = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcAnnotationTypeEnum
    {
        ASSUMEDPOINT = 0,
        ASBUILTAREA = 1,
        ASBUILTLINE = 2,
        NON_PHYSICAL_SIGNAL = 3,
        ASSUMEDLINE = 4,
        WIDTHEVENT = 5,
        ASSUMEDAREA = 6,
        SUPERELEVATIONEVENT = 7,
        ASBUILTPOINT = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcArithmeticOperatorEnum
    {
        ADD = 0,
        DIVIDE = 1,
        MULTIPLY = 2,
        SUBTRACT = 3,
        ___unk = -1
    };

    enum class IfcAssemblyPlaceEnum
    {
        SITE = 0,
        FACTORY = 1,
        NOTDEFINED = 2,
        ___unk = -1
    };

    enum class IfcAudioVisualApplianceTypeEnum
    {
        AMPLIFIER = 0,
        CAMERA = 1,
        DISPLAY = 2,
        MICROPHONE = 3,
        PLAYER = 4,
        PROJECTOR = 5,
        RECEIVER = 6,
        SPEAKER = 7,
        SWITCHER = 8,
        TELEPHONE = 9,
        TUNER = 10,
        COMMUNICATIONTERMINAL = 11,
        RECORDINGEQUIPMENT = 12,
        USERDEFINED = 13,
        NOTDEFINED = 14,
        SIREN = 15,
        BEACON = 16,
        ___unk = -1
    };

    enum class IfcBeamTypeEnum
    {
        BEAM = 0,
        JOIST = 1,
        HOLLOWCORE = 2,
        LINTEL = 3,
        SPANDREL = 4,
        T_BEAM = 5,
        GIRDER_SEGMENT = 6,
        DIAPHRAGM = 7,
        PIERCAP = 8,
        HATSTONE = 9,
        CORNICE = 10,
        EDGEBEAM = 11,
        USERDEFINED = 12,
        NOTDEFINED = 13,
        ___unk = -1
    };

    enum class IfcBearingTypeDisplacementEnum
    {
        FIXED_MOVEMENT = 0,
        GUIDED_LONGITUDINAL = 1,
        GUIDED_TRANSVERSAL = 2,
        FREE_MOVEMENT = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcBearingTypeEnum
    {
        CYLINDRICAL = 0,
        SPHERICAL = 1,
        ELASTOMERIC = 2,
        POT = 3,
        GUIDE = 4,
        ROCKER = 5,
        ROLLER = 6,
        DISK = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcBenchmarkEnum
    {
        GREATERTHAN = 0,
        GREATERTHANOREQUALTO = 1,
        LESSTHAN = 2,
        LESSTHANOREQUALTO = 3,
        EQUALTO = 4,
        NOTEQUALTO = 5,
        INCLUDES = 6,
        NOTINCLUDES = 7,
        INCLUDEDIN = 8,
        NOTINCLUDEDIN = 9,
        ___unk = -1
    };

    enum class IfcBoilerTypeEnum
    {
        WATER = 0,
        STEAM = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcBooleanOperator
    {
        UNION = 0,
        INTERSECTION = 1,
        DIFFERENCE = 2,
        ___unk = -1
    };

    enum class IfcBoreholeTypeEnum
    {
        COREDRILLING = 0,
        DESTRUCTIVEDRILLING = 1,
        TRIALPIT = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcBridgePartTypeEnum
    {
        ABUTMENT = 0,
        DECK = 1,
        DECK_SEGMENT = 2,
        FOUNDATION = 3,
        PIER = 4,
        PIER_SEGMENT = 5,
        PYLON = 6,
        SUBSTRUCTURE = 7,
        SUPERSTRUCTURE = 8,
        SURFACESTRUCTURE = 9,
        USERDEFINED = 10,
        NOTDEFINED = 11,
        ___unk = -1
    };

    enum class IfcBridgeTypeEnum
    {
        ARCHED = 0,
        CABLE_STAYED = 1,
        CANTILEVER = 2,
        CULVERT = 3,
        FRAMEWORK = 4,
        GIRDER = 5,
        SUSPENSION = 6,
        TRUSS = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcBSplineCurveForm
    {
        POLYLINE_FORM = 0,
        CIRCULAR_ARC = 1,
        ELLIPTIC_ARC = 2,
        PARABOLIC_ARC = 3,
        HYPERBOLIC_ARC = 4,
        UNSPECIFIED = 5,
        ___unk = -1
    };

    enum class IfcBSplineSurfaceForm
    {
        PLANE_SURF = 0,
        CYLINDRICAL_SURF = 1,
        CONICAL_SURF = 2,
        SPHERICAL_SURF = 3,
        TOROIDAL_SURF = 4,
        SURF_OF_REVOLUTION = 5,
        RULED_SURF = 6,
        GENERALISED_CONE = 7,
        QUADRIC_SURF = 8,
        SURF_OF_LINEAR_EXTRUSION = 9,
        UNSPECIFIED = 10,
        ___unk = -1
    };

    enum class IfcBuildingElementPartTypeEnum
    {
        INSULATION = 0,
        PRECASTPANEL = 1,
        APRON = 2,
        ARMOURUNIT = 3,
        SAFETYCAGE = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcBuildingElementProxyTypeEnum
    {
        COMPLEX = 0,
        ELEMENT = 1,
        PARTIAL = 2,
        PROVISIONFORVOID = 3,
        PROVISIONFORSPACE = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcBuildingSystemTypeEnum
    {
        FENESTRATION = 0,
        FOUNDATION = 1,
        LOADBEARING = 2,
        OUTERSHELL = 3,
        SHADING = 4,
        TRANSPORT = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcBuiltSystemTypeEnum
    {
        REINFORCING = 0,
        MOORING = 1,
        OUTERSHELL = 2,
        TRACKCIRCUIT = 3,
        EROSIONPREVENTION = 4,
        FOUNDATION = 5,
        LOADBEARING = 6,
        SHADING = 7,
        FENESTRATION = 8,
        TRANSPORT = 9,
        PRESTRESSING = 10,
        RAILWAYLINE = 11,
        RAILWAYTRACK = 12,
        TUNNEL_PRESUPPORT = 13,
        TUNNEL_SUPPORT = 14,
        TUNNEL_LINING = 15,
        WATERPROOFING = 16,
        FIREPROTECTION = 17,
        USERDEFINED = 18,
        NOTDEFINED = 19,
        ___unk = -1
    };

    enum class IfcBurnerTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcCableCarrierFittingTypeEnum
    {
        BEND = 0,
        CROSS = 1,
        REDUCER = 2,
        TEE = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcCableCarrierSegmentTypeEnum
    {
        CABLELADDERSEGMENT = 0,
        CABLETRAYSEGMENT = 1,
        CABLETRUNKINGSEGMENT = 2,
        CONDUITSEGMENT = 3,
        CABLEBRACKET = 4,
        CATENARYWIRE = 5,
        DROPPER = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcCableFittingTypeEnum
    {
        CONNECTOR = 0,
        ENTRY = 1,
        EXIT = 2,
        JUNCTION = 3,
        TRANSITION = 4,
        FANOUT = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcCableSegmentTypeEnum
    {
        BUSBARSEGMENT = 0,
        CABLESEGMENT = 1,
        CONDUCTORSEGMENT = 2,
        CORESEGMENT = 3,
        CONTACTWIRESEGMENT = 4,
        FIBERSEGMENT = 5,
        FIBERTUBE = 6,
        OPTICALCABLESEGMENT = 7,
        STITCHWIRE = 8,
        WIREPAIRSEGMENT = 9,
        USERDEFINED = 10,
        NOTDEFINED = 11,
        ___unk = -1
    };

    enum class IfcCaissonFoundationTypeEnum
    {
        WELL = 0,
        CAISSON = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcChangeActionEnum
    {
        NOCHANGE = 0,
        MODIFIED = 1,
        ADDED = 2,
        DELETED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcChillerTypeEnum
    {
        AIRCOOLED = 0,
        WATERCOOLED = 1,
        HEATRECOVERY = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcChimneyTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcCoilTypeEnum
    {
        DXCOOLINGCOIL = 0,
        ELECTRICHEATINGCOIL = 1,
        GASHEATINGCOIL = 2,
        HYDRONICCOIL = 3,
        STEAMHEATINGCOIL = 4,
        WATERCOOLINGCOIL = 5,
        WATERHEATINGCOIL = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcColumnTypeEnum
    {
        COLUMN = 0,
        PILASTER = 1,
        PIERSTEM = 2,
        PIERSTEM_SEGMENT = 3,
        STANDCOLUMN = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcCommunicationsApplianceTypeEnum
    {
        ANTENNA = 0,
        COMPUTER = 1,
        FAX = 2,
        GATEWAY = 3,
        MODEM = 4,
        NETWORKAPPLIANCE = 5,
        NETWORKBRIDGE = 6,
        NETWORKHUB = 7,
        PRINTER = 8,
        REPEATER = 9,
        ROUTER = 10,
        SCANNER = 11,
        AUTOMATON = 12,
        INTELLIGENTPERIPHERAL = 13,
        IPNETWORKEQUIPMENT = 14,
        OPTICALNETWORKUNIT = 15,
        TELECOMMAND = 16,
        TELEPHONYEXCHANGE = 17,
        TRANSITIONCOMPONENT = 18,
        TRANSPONDER = 19,
        TRANSPORTEQUIPMENT = 20,
        OPTICALLINETERMINAL = 21,
        LINESIDEELECTRONICUNIT = 22,
        RADIOBLOCKCENTER = 23,
        USERDEFINED = 24,
        NOTDEFINED = 25,
        ___unk = -1
    };

    enum class IfcComplexPropertyTemplateTypeEnum
    {
        P_COMPLEX = 0,
        Q_COMPLEX = 1,
        ___unk = -1
    };

    enum class IfcCompressorTypeEnum
    {
        DYNAMIC = 0,
        RECIPROCATING = 1,
        ROTARY = 2,
        SCROLL = 3,
        TROCHOIDAL = 4,
        SINGLESTAGE = 5,
        BOOSTER = 6,
        OPENTYPE = 7,
        HERMETIC = 8,
        SEMIHERMETIC = 9,
        WELDEDSHELLHERMETIC = 10,
        ROLLINGPISTON = 11,
        ROTARYVANE = 12,
        SINGLESCREW = 13,
        TWINSCREW = 14,
        USERDEFINED = 15,
        NOTDEFINED = 16,
        ___unk = -1
    };

    enum class IfcCondenserTypeEnum
    {
        AIRCOOLED = 0,
        EVAPORATIVECOOLED = 1,
        WATERCOOLED = 2,
        WATERCOOLEDBRAZEDPLATE = 3,
        WATERCOOLEDSHELLCOIL = 4,
        WATERCOOLEDSHELLTUBE = 5,
        WATERCOOLEDTUBEINTUBE = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcConnectionTypeEnum
    {
        ATPATH = 0,
        ATSTART = 1,
        ATEND = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcConstraintEnum
    {
        HARD = 0,
        SOFT = 1,
        ADVISORY = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcConstructionEquipmentResourceTypeEnum
    {
        DEMOLISHING = 0,
        EARTHMOVING = 1,
        ERECTING = 2,
        HEATING = 3,
        LIGHTING = 4,
        PAVING = 5,
        PUMPING = 6,
        TRANSPORTING = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcConstructionMaterialResourceTypeEnum
    {
        AGGREGATES = 0,
        CONCRETE = 1,
        DRYWALL = 2,
        FUEL = 3,
        GYPSUM = 4,
        MASONRY = 5,
        METAL = 6,
        PLASTIC = 7,
        WOOD = 8,
        NOTDEFINED = 9,
        USERDEFINED = 10,
        ___unk = -1
    };

    enum class IfcConstructionProductResourceTypeEnum
    {
        ASSEMBLY = 0,
        FORMWORK = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcControllerTypeEnum
    {
        FLOATING = 0,
        PROGRAMMABLE = 1,
        PROPORTIONAL = 2,
        MULTIPOSITION = 3,
        TWOPOSITION = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcConveyorSegmentTypeEnum
    {
        CHUTECONVEYOR = 0,
        BELTCONVEYOR = 1,
        SCREWCONVEYOR = 2,
        BUCKETCONVEYOR = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcCooledBeamTypeEnum
    {
        ACTIVE = 0,
        PASSIVE = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcCoolingTowerTypeEnum
    {
        NATURALDRAFT = 0,
        MECHANICALINDUCEDDRAFT = 1,
        MECHANICALFORCEDDRAFT = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcCostItemTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcCostScheduleTypeEnum
    {
        BUDGET = 0,
        COSTPLAN = 1,
        ESTIMATE = 2,
        TENDER = 3,
        PRICEDBILLOFQUANTITIES = 4,
        UNPRICEDBILLOFQUANTITIES = 5,
        SCHEDULEOFRATES = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcCourseTypeEnum
    {
        ARMOUR = 0,
        FILTER = 1,
        BALLASTBED = 2,
        CORE = 3,
        PAVEMENT = 4,
        PROTECTION = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcCoveringTypeEnum
    {
        CEILING = 0,
        FLOORING = 1,
        CLADDING = 2,
        ROOFING = 3,
        MOLDING = 4,
        SKIRTINGBOARD = 5,
        INSULATION = 6,
        MEMBRANE = 7,
        SLEEVING = 8,
        TOPPING = 9,
        WRAPPING = 10,
        COPING = 11,
        USERDEFINED = 12,
        NOTDEFINED = 13,
        ___unk = -1
    };

    enum class IfcCrewResourceTypeEnum
    {
        OFFICE = 0,
        SITE = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcCurtainWallTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcCurveInterpolationEnum
    {
        LINEAR = 0,
        LOG_LINEAR = 1,
        LOG_LOG = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcDamperTypeEnum
    {
        BACKDRAFTDAMPER = 0,
        BALANCINGDAMPER = 1,
        BLASTDAMPER = 2,
        CONTROLDAMPER = 3,
        FIREDAMPER = 4,
        FIRESMOKEDAMPER = 5,
        FUMEHOODEXHAUST = 6,
        GRAVITYDAMPER = 7,
        GRAVITYRELIEFDAMPER = 8,
        RELIEFDAMPER = 9,
        SMOKEDAMPER = 10,
        USERDEFINED = 11,
        NOTDEFINED = 12,
        ___unk = -1
    };

    enum class IfcDataOriginEnum
    {
        MEASURED = 0,
        PREDICTED = 1,
        SIMULATED = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcDerivedUnitEnum
    {
        ANGULARVELOCITYUNIT = 0,
        AREADENSITYUNIT = 1,
        COMPOUNDPLANEANGLEUNIT = 2,
        DYNAMICVISCOSITYUNIT = 3,
        HEATFLUXDENSITYUNIT = 4,
        INTEGERCOUNTRATEUNIT = 5,
        ISOTHERMALMOISTURECAPACITYUNIT = 6,
        KINEMATICVISCOSITYUNIT = 7,
        LINEARVELOCITYUNIT = 8,
        MASSDENSITYUNIT = 9,
        MASSFLOWRATEUNIT = 10,
        MOISTUREDIFFUSIVITYUNIT = 11,
        MOLECULARWEIGHTUNIT = 12,
        SPECIFICHEATCAPACITYUNIT = 13,
        THERMALADMITTANCEUNIT = 14,
        THERMALCONDUCTANCEUNIT = 15,
        THERMALRESISTANCEUNIT = 16,
        THERMALTRANSMITTANCEUNIT = 17,
        VAPORPERMEABILITYUNIT = 18,
        VOLUMETRICFLOWRATEUNIT = 19,
        ROTATIONALFREQUENCYUNIT = 20,
        TORQUEUNIT = 21,
        MOMENTOFINERTIAUNIT = 22,
        LINEARMOMENTUNIT = 23,
        LINEARFORCEUNIT = 24,
        PLANARFORCEUNIT = 25,
        MODULUSOFELASTICITYUNIT = 26,
        SHEARMODULUSUNIT = 27,
        LINEARSTIFFNESSUNIT = 28,
        ROTATIONALSTIFFNESSUNIT = 29,
        MODULUSOFSUBGRADEREACTIONUNIT = 30,
        ACCELERATIONUNIT = 31,
        CURVATUREUNIT = 32,
        HEATINGVALUEUNIT = 33,
        IONCONCENTRATIONUNIT = 34,
        LUMINOUSINTENSITYDISTRIBUTIONUNIT = 35,
        MASSPERLENGTHUNIT = 36,
        MODULUSOFLINEARSUBGRADEREACTIONUNIT = 37,
        MODULUSOFROTATIONALSUBGRADEREACTIONUNIT = 38,
        PHUNIT = 39,
        ROTATIONALMASSUNIT = 40,
        SECTIONAREAINTEGRALUNIT = 41,
        SECTIONMODULUSUNIT = 42,
        SOUNDPOWERLEVELUNIT = 43,
        SOUNDPOWERUNIT = 44,
        SOUNDPRESSURELEVELUNIT = 45,
        SOUNDPRESSUREUNIT = 46,
        TEMPERATUREGRADIENTUNIT = 47,
        TEMPERATURERATEOFCHANGEUNIT = 48,
        THERMALEXPANSIONCOEFFICIENTUNIT = 49,
        WARPINGCONSTANTUNIT = 50,
        WARPINGMOMENTUNIT = 51,
        USERDEFINED = 52,
        ___unk = -1
    };

    enum class IfcDirectionSenseEnum
    {
        POSITIVE = 0,
        NEGATIVE = 1,
        ___unk = -1
    };

    enum class IfcDiscreteAccessoryTypeEnum
    {
        ANCHORPLATE = 0,
        BRACKET = 1,
        SHOE = 2,
        EXPANSION_JOINT_DEVICE = 3,
        CABLEARRANGER = 4,
        FILLER = 5,
        FLASHING = 6,
        INSULATOR = 7,
        LOCK = 8,
        TENSIONINGEQUIPMENT = 9,
        RAILPAD = 10,
        SLIDINGCHAIR = 11,
        RAIL_LUBRICATION = 12,
        PANEL_STRENGTHENING = 13,
        RAILBRACE = 14,
        ELASTIC_CUSHION = 15,
        SOUNDABSORPTION = 16,
        POINTMACHINEMOUNTINGDEVICE = 17,
        POINT_MACHINE_LOCKING_DEVICE = 18,
        RAIL_MECHANICAL_EQUIPMENT = 19,
        BIRDPROTECTION = 20,
        WATER_BARRIER = 21,
        STRUCTURAL_SEALING = 22,
        USERDEFINED = 23,
        NOTDEFINED = 24,
        ___unk = -1
    };

    enum class IfcDistributionBoardTypeEnum
    {
        SWITCHBOARD = 0,
        CONSUMERUNIT = 1,
        MOTORCONTROLCENTRE = 2,
        DISTRIBUTIONFRAME = 3,
        DISTRIBUTIONBOARD = 4,
        DISPATCHINGBOARD = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcDistributionChamberElementTypeEnum
    {
        FORMEDDUCT = 0,
        INSPECTIONCHAMBER = 1,
        INSPECTIONPIT = 2,
        MANHOLE = 3,
        METERCHAMBER = 4,
        SUMP = 5,
        TRENCH = 6,
        VALVECHAMBER = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcDistributionPortTypeEnum
    {
        CABLE = 0,
        CABLECARRIER = 1,
        DUCT = 2,
        PIPE = 3,
        WIRELESS = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcDistributionSystemEnum
    {
        AIRCONDITIONING = 0,
        AUDIOVISUAL = 1,
        CHEMICAL = 2,
        CHILLEDWATER = 3,
        COMMUNICATION = 4,
        COMPRESSEDAIR = 5,
        CONDENSERWATER = 6,
        CONTROL = 7,
        CONVEYING = 8,
        DATA = 9,
        DISPOSAL = 10,
        DOMESTICCOLDWATER = 11,
        DOMESTICHOTWATER = 12,
        DRAINAGE = 13,
        EARTHING = 14,
        ELECTRICAL = 15,
        ELECTROACOUSTIC = 16,
        EXHAUST = 17,
        FIREPROTECTION = 18,
        FUEL = 19,
        GAS = 20,
        HAZARDOUS = 21,
        HEATING = 22,
        LIGHTING = 23,
        LIGHTNINGPROTECTION = 24,
        MUNICIPALSOLIDWASTE = 25,
        OIL = 26,
        OPERATIONAL = 27,
        POWERGENERATION = 28,
        RAINWATER = 29,
        REFRIGERATION = 30,
        SECURITY = 31,
        SEWAGE = 32,
        SIGNAL = 33,
        STORMWATER = 34,
        TELEPHONE = 35,
        TV = 36,
        VACUUM = 37,
        VENT = 38,
        VENTILATION = 39,
        WASTEWATER = 40,
        WATERSUPPLY = 41,
        CATENARY_SYSTEM = 42,
        OVERHEAD_CONTACTLINE_SYSTEM = 43,
        RETURN_CIRCUIT = 44,
        FIXEDTRANSMISSIONNETWORK = 45,
        OPERATIONALTELEPHONYSYSTEM = 46,
        MOBILENETWORK = 47,
        MONITORINGSYSTEM = 48,
        SAFETY = 49,
        USERDEFINED = 50,
        NOTDEFINED = 51,
        ___unk = -1
    };

    enum class IfcDocumentConfidentialityEnum
    {
        PUBLIC = 0,
        RESTRICTED = 1,
        CONFIDENTIAL = 2,
        PERSONAL = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcDocumentStatusEnum
    {
        DRAFT = 0,
        FINALDRAFT = 1,
        FINAL = 2,
        REVISION = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcDoorPanelOperationEnum
    {
        SWINGING = 0,
        DOUBLE_ACTING = 1,
        SLIDING = 2,
        FOLDING = 3,
        REVOLVING = 4,
        ROLLINGUP = 5,
        FIXEDPANEL = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcDoorPanelPositionEnum
    {
        LEFT = 0,
        MIDDLE = 1,
        RIGHT = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcDoorStyleConstructionEnum
    {
        ALUMINIUM = 0,
        HIGH_GRADE_STEEL = 1,
        STEEL = 2,
        WOOD = 3,
        ALUMINIUM_WOOD = 4,
        ALUMINIUM_PLASTIC = 5,
        PLASTIC = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcDoorStyleOperationEnum
    {
        SINGLE_SWING_LEFT = 0,
        SINGLE_SWING_RIGHT = 1,
        DOUBLE_DOOR_SINGLE_SWING = 2,
        DOUBLE_DOOR_SINGLE_SWING_OPPOSITE_LEFT = 3,
        DOUBLE_DOOR_SINGLE_SWING_OPPOSITE_RIGHT = 4,
        DOUBLE_SWING_LEFT = 5,
        DOUBLE_SWING_RIGHT = 6,
        DOUBLE_DOOR_DOUBLE_SWING = 7,
        SLIDING_TO_LEFT = 8,
        SLIDING_TO_RIGHT = 9,
        DOUBLE_DOOR_SLIDING = 10,
        FOLDING_TO_LEFT = 11,
        FOLDING_TO_RIGHT = 12,
        DOUBLE_DOOR_FOLDING = 13,
        REVOLVING = 14,
        ROLLINGUP = 15,
        USERDEFINED = 16,
        NOTDEFINED = 17,
        ___unk = -1
    };

    enum class IfcDoorTypeEnum
    {
        DOOR = 0,
        GATE = 1,
        TRAPDOOR = 2,
        BOOM_BARRIER = 3,
        TURNSTILE = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcDoorTypeOperationEnum
    {
        SINGLE_SWING_LEFT = 0,
        SINGLE_SWING_RIGHT = 1,
        DOUBLE_PANEL_SINGLE_SWING = 2,
        DOUBLE_PANEL_SINGLE_SWING_OPPOSITE_LEFT = 3,
        DOUBLE_PANEL_SINGLE_SWING_OPPOSITE_RIGHT = 4,
        DOUBLE_SWING_LEFT = 5,
        DOUBLE_SWING_RIGHT = 6,
        DOUBLE_PANEL_DOUBLE_SWING = 7,
        SLIDING_TO_LEFT = 8,
        SLIDING_TO_RIGHT = 9,
        DOUBLE_PANEL_SLIDING = 10,
        FOLDING_TO_LEFT = 11,
        FOLDING_TO_RIGHT = 12,
        DOUBLE_PANEL_FOLDING = 13,
        REVOLVING_HORIZONTAL = 14,
        ROLLINGUP = 15,
        SWING_FIXED_LEFT = 16,
        SWING_FIXED_RIGHT = 17,
        DOUBLE_PANEL_LIFTING_VERTICAL = 18,
        LIFTING_HORIZONTAL = 19,
        LIFTING_VERTICAL_LEFT = 20,
        LIFTING_VERTICAL_RIGHT = 21,
        REVOLVING_VERTICAL = 22,
        USERDEFINED = 23,
        NOTDEFINED = 24,
        ___unk = -1
    };

    enum class IfcDuctFittingTypeEnum
    {
        BEND = 0,
        CONNECTOR = 1,
        ENTRY = 2,
        EXIT = 3,
        JUNCTION = 4,
        OBSTRUCTION = 5,
        TRANSITION = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcDuctSegmentTypeEnum
    {
        RIGIDSEGMENT = 0,
        FLEXIBLESEGMENT = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcDuctSilencerTypeEnum
    {
        FLATOVAL = 0,
        RECTANGULAR = 1,
        ROUND = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcEarthingElementTypeEnum
    {
        EARTHINGSTRIP = 0,
        GROUNDINGPLATE = 1,
        GROUNDINGROD = 2,
        FIXEDTERMINAL = 3,
        GROUNDINGMESH = 4,
        NOTDEFINED = 5,
        USERDEFINED = 6,
        ___unk = -1
    };

    enum class IfcEarthworksCutTypeEnum
    {
        TRENCH = 0,
        DREDGING = 1,
        EXCAVATION = 2,
        OVEREXCAVATION = 3,
        TOPSOILREMOVAL = 4,
        STEPEXCAVATION = 5,
        PAVEMENTMILLING = 6,
        CUT = 7,
        BASE_EXCAVATION = 8,
        CONFINEDOPENEXCAVATION = 9,
        ANCHOREDOPENEXCAVATION = 10,
        BRACEDOPENEXCAVATION = 11,
        USERDEFINED = 12,
        NOTDEFINED = 13,
        ___unk = -1
    };

    enum class IfcEarthworksFillTypeEnum
    {
        BACKFILL = 0,
        COUNTERWEIGHT = 1,
        SUBGRADE = 2,
        EMBANKMENT = 3,
        TRANSITIONSECTION = 4,
        SUBGRADEBED = 5,
        SLOPEFILL = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcElectricApplianceTypeEnum
    {
        DISHWASHER = 0,
        ELECTRICCOOKER = 1,
        FREESTANDINGELECTRICHEATER = 2,
        FREESTANDINGFAN = 3,
        FREESTANDINGWATERHEATER = 4,
        FREESTANDINGWATERCOOLER = 5,
        FREEZER = 6,
        FRIDGE_FREEZER = 7,
        HANDDRYER = 8,
        KITCHENMACHINE = 9,
        MICROWAVE = 10,
        PHOTOCOPIER = 11,
        REFRIGERATOR = 12,
        TUMBLEDRYER = 13,
        VENDINGMACHINE = 14,
        WASHINGMACHINE = 15,
        USERDEFINED = 16,
        NOTDEFINED = 17,
        ___unk = -1
    };

    enum class IfcElectricDistributionBoardTypeEnum
    {
        CONSUMERUNIT = 0,
        DISTRIBUTIONBOARD = 1,
        MOTORCONTROLCENTRE = 2,
        SWITCHBOARD = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcElectricFlowStorageDeviceTypeEnum
    {
        BATTERY = 0,
        CAPACITORBANK = 1,
        HARMONICFILTER = 2,
        INDUCTORBANK = 3,
        UPS = 4,
        CAPACITOR = 5,
        COMPENSATOR = 6,
        INDUCTOR = 7,
        RECHARGER = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcElectricFlowTreatmentDeviceTypeEnum
    {
        ELECTRONICFILTER = 0,
        USERDEFINED = 1,
        NOTDEFINED = 2,
        ___unk = -1
    };

    enum class IfcElectricGeneratorTypeEnum
    {
        CHP = 0,
        ENGINEGENERATOR = 1,
        STANDALONE = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcElectricMotorTypeEnum
    {
        DC = 0,
        INDUCTION = 1,
        POLYPHASE = 2,
        RELUCTANCESYNCHRONOUS = 3,
        SYNCHRONOUS = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcElectricTimeControlTypeEnum
    {
        TIMECLOCK = 0,
        TIMEDELAY = 1,
        RELAY = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcElementAssemblyTypeEnum
    {
        ACCESSORY_ASSEMBLY = 0,
        ARCH = 1,
        BEAM_GRID = 2,
        BRACED_FRAME = 3,
        GIRDER = 4,
        REINFORCEMENT_UNIT = 5,
        RIGID_FRAME = 6,
        SLAB_FIELD = 7,
        TRUSS = 8,
        ABUTMENT = 9,
        PIER = 10,
        PYLON = 11,
        CROSS_BRACING = 12,
        DECK = 13,
        MAST = 14,
        SIGNALASSEMBLY = 15,
        GRID = 16,
        SHELTER = 17,
        SUPPORTINGASSEMBLY = 18,
        SUSPENSIONASSEMBLY = 19,
        TRACTION_SWITCHING_ASSEMBLY = 20,
        TRACKPANEL = 21,
        TURNOUTPANEL = 22,
        DILATATIONPANEL = 23,
        RAIL_MECHANICAL_EQUIPMENT_ASSEMBLY = 24,
        ENTRANCEWORKS = 25,
        SUMPBUSTER = 26,
        TRAFFIC_CALMING_DEVICE = 27,
        DUCTBANK = 28,
        UMBRELLAVAULT = 29,
        USERDEFINED = 30,
        NOTDEFINED = 31,
        ___unk = -1
    };

    enum class IfcElementCompositionEnum
    {
        COMPLEX = 0,
        ELEMENT = 1,
        PARTIAL = 2,
        ___unk = -1
    };

    enum class IfcEngineTypeEnum
    {
        EXTERNALCOMBUSTION = 0,
        INTERNALCOMBUSTION = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcEvaporativeCoolerTypeEnum
    {
        DIRECTEVAPORATIVERANDOMMEDIAAIRCOOLER = 0,
        DIRECTEVAPORATIVERIGIDMEDIAAIRCOOLER = 1,
        DIRECTEVAPORATIVESLINGERSPACKAGEDAIRCOOLER = 2,
        DIRECTEVAPORATIVEPACKAGEDROTARYAIRCOOLER = 3,
        DIRECTEVAPORATIVEAIRWASHER = 4,
        INDIRECTEVAPORATIVEPACKAGEAIRCOOLER = 5,
        INDIRECTEVAPORATIVEWETCOIL = 6,
        INDIRECTEVAPORATIVECOOLINGTOWERORCOILCOOLER = 7,
        INDIRECTDIRECTCOMBINATION = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcEvaporatorTypeEnum
    {
        DIRECTEXPANSION = 0,
        DIRECTEXPANSIONSHELLANDTUBE = 1,
        DIRECTEXPANSIONTUBEINTUBE = 2,
        DIRECTEXPANSIONBRAZEDPLATE = 3,
        FLOODEDSHELLANDTUBE = 4,
        SHELLANDCOIL = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcEventTriggerTypeEnum
    {
        EVENTRULE = 0,
        EVENTMESSAGE = 1,
        EVENTTIME = 2,
        EVENTCOMPLEX = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcEventTypeEnum
    {
        STARTEVENT = 0,
        ENDEVENT = 1,
        INTERMEDIATEEVENT = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcExternalSpatialElementTypeEnum
    {
        EXTERNAL = 0,
        EXTERNAL_EARTH = 1,
        EXTERNAL_WATER = 2,
        EXTERNAL_FIRE = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcFacilityPartCommonTypeEnum
    {
        SEGMENT = 0,
        ABOVEGROUND = 1,
        JUNCTION = 2,
        LEVELCROSSING = 3,
        BELOWGROUND = 4,
        SUBSTRUCTURE = 5,
        TERMINAL = 6,
        SUPERSTRUCTURE = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcFacilityUsageEnum
    {
        LATERAL = 0,
        REGION = 1,
        VERTICAL = 2,
        LONGITUDINAL = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcFanTypeEnum
    {
        CENTRIFUGALFORWARDCURVED = 0,
        CENTRIFUGALRADIAL = 1,
        CENTRIFUGALBACKWARDINCLINEDCURVED = 2,
        CENTRIFUGALAIRFOIL = 3,
        TUBEAXIAL = 4,
        VANEAXIAL = 5,
        PROPELLORAXIAL = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        JET = 9,
        ___unk = -1
    };

    enum class IfcFastenerTypeEnum
    {
        GLUE = 0,
        MORTAR = 1,
        WELD = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcFillElementTypeEnum
    {
        INVERTFILL = 0,
        ANNULARGAPFILL = 1,
        NOTDEFINED = 2,
        USERDEFINED = 3,
        ___unk = -1
    };

    enum class IfcFilterTypeEnum
    {
        AIRPARTICLEFILTER = 0,
        COMPRESSEDAIRFILTER = 1,
        ODORFILTER = 2,
        OILFILTER = 3,
        STRAINER = 4,
        WATERFILTER = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcFireSuppressionTerminalTypeEnum
    {
        BREECHINGINLET = 0,
        FIREHYDRANT = 1,
        HOSEREEL = 2,
        SPRINKLER = 3,
        SPRINKLERDEFLECTOR = 4,
        FIREMONITOR = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcFlowDirectionEnum
    {
        SOURCE = 0,
        SINK = 1,
        SOURCEANDSINK = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcFlowInstrumentTypeEnum
    {
        PRESSUREGAUGE = 0,
        THERMOMETER = 1,
        AMMETER = 2,
        FREQUENCYMETER = 3,
        POWERFACTORMETER = 4,
        PHASEANGLEMETER = 5,
        VOLTMETER_PEAK = 6,
        VOLTMETER_RMS = 7,
        COMBINED = 8,
        VOLTMETER = 9,
        USERDEFINED = 10,
        NOTDEFINED = 11,
        ___unk = -1
    };

    enum class IfcFlowMeterTypeEnum
    {
        ENERGYMETER = 0,
        GASMETER = 1,
        OILMETER = 2,
        WATERMETER = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcFootingTypeEnum
    {
        CAISSON_FOUNDATION = 0,
        FOOTING_BEAM = 1,
        PAD_FOOTING = 2,
        PILE_CAP = 3,
        STRIP_FOOTING = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcFurnitureTypeEnum
    {
        CHAIR = 0,
        TABLE = 1,
        DESK = 2,
        BED = 3,
        FILECABINET = 4,
        SHELF = 5,
        SOFA = 6,
        TECHNICALCABINET = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcGeographicElementTypeEnum
    {
        TERRAIN = 0,
        SOIL_BORING_POINT = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcGeometricProjectionEnum
    {
        GRAPH_VIEW = 0,
        SKETCH_VIEW = 1,
        MODEL_VIEW = 2,
        PLAN_VIEW = 3,
        REFLECTED_PLAN_VIEW = 4,
        SECTION_VIEW = 5,
        ELEVATION_VIEW = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcGeoScienceFeatureTypeEnum
    {
        DISCRETEDISCONTINUITY = 0,
        FOLD = 1,
        FLUIDBODY = 2,
        PIEZOMETRICWATERLEVEL = 3,
        VOIDBODY = 4,
        GEOLOGICUNIT = 5,
        GEOTECHNICALUNIT = 6,
        HAZARDAREA = 7,
        HYDROGEOUNIT = 8,
        FAULT = 9,
        CONTACT = 10,
        PHYSICALPROPERTYDISTRIBUTION = 11,
        USERDEFINED = 12,
        NOTDEFINED = 13,
        ___unk = -1
    };

    enum class IfcGeoScienceModelTypeEnum
    {
        GEOTECHMODEL = 0,
        HYDROGEOMODEL = 1,
        GEOLOGYMODEL = 2,
        GEOTECHSYNTHESISMODEL = 3,
        PHYSICALPROPERTYDISTIBUTIONMODEL = 4,
        GEOHAZARDMODEL = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcGeoScienceObservationTypeEnum
    {
        INSITUTESTRESULT = 0,
        LABTESTRESULT = 1,
        BOREHOLELOG = 2,
        MAPPEDFEATURE = 3,
        LOCALINFORMATION = 4,
        GEOPHYSICALSURVEYRESULT = 5,
        NOTDEFINED = 6,
        USERDEFINED = 7,
        ___unk = -1
    };

    enum class IfcGeotechTypicalSectionTypeEnum
    {
        NOTDEFINED = 0,
        USERDEFINED = 1,
        ___unk = -1
    };

    enum class IfcGlobalOrLocalEnum
    {
        GLOBAL_COORDS = 0,
        LOCAL_COORDS = 1,
        ___unk = -1
    };

    enum class IfcGridTypeEnum
    {
        RECTANGULAR = 0,
        RADIAL = 1,
        TRIANGULAR = 2,
        IRREGULAR = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcGroundReinforcementElementTypeEnum
    {
        SPILINGBOLT = 0,
        ROCKSUPPORTBOLT = 1,
        NOTDEFINED = 2,
        USERDEFINED = 3,
        ___unk = -1
    };

    enum class IfcHeatExchangerTypeEnum
    {
        PLATE = 0,
        SHELLANDTUBE = 1,
        TURNOUTHEATING = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcHumidifierTypeEnum
    {
        STEAMINJECTION = 0,
        ADIABATICAIRWASHER = 1,
        ADIABATICPAN = 2,
        ADIABATICWETTEDELEMENT = 3,
        ADIABATICATOMIZING = 4,
        ADIABATICULTRASONIC = 5,
        ADIABATICRIGIDMEDIA = 6,
        ADIABATICCOMPRESSEDAIRNOZZLE = 7,
        ASSISTEDELECTRIC = 8,
        ASSISTEDNATURALGAS = 9,
        ASSISTEDPROPANE = 10,
        ASSISTEDBUTANE = 11,
        ASSISTEDSTEAM = 12,
        USERDEFINED = 13,
        NOTDEFINED = 14,
        ___unk = -1
    };

    enum class IfcImpactProtectionDeviceTypeEnum
    {
        CRASHCUSHION = 0,
        DAMPINGSYSTEM = 1,
        FENDER = 2,
        BUMPER = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcImprovedGroundTypeEnum
    {
        SURCHARGEPRELOADED = 0,
        VERTICALLYDRAINED = 1,
        DYNAMICALLYCOMPACTED = 2,
        REPLACED = 3,
        ROLLERCOMPACTED = 4,
        GROUTED = 5,
        DEEPMIXED = 6,
        LATERALLYDRAINED = 7,
        FROZEN = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcInterceptorTypeEnum
    {
        CYCLONIC = 0,
        GREASE = 1,
        OIL = 2,
        PETROL = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcInternalOrExternalEnum
    {
        INTERNAL = 0,
        EXTERNAL = 1,
        EXTERNAL_EARTH = 2,
        EXTERNAL_WATER = 3,
        EXTERNAL_FIRE = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcInventoryTypeEnum
    {
        ASSETINVENTORY = 0,
        SPACEINVENTORY = 1,
        FURNITUREINVENTORY = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcJunctionBoxTypeEnum
    {
        DATA = 0,
        POWER = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcKerbTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcKnotType
    {
        UNIFORM_KNOTS = 0,
        QUASI_UNIFORM_KNOTS = 1,
        PIECEWISE_BEZIER_KNOTS = 2,
        UNSPECIFIED = 3,
        ___unk = -1
    };

    enum class IfcLaborResourceTypeEnum
    {
        ADMINISTRATION = 0,
        CARPENTRY = 1,
        CLEANING = 2,
        CONCRETE = 3,
        DRYWALL = 4,
        ELECTRIC = 5,
        FINISHING = 6,
        FLOORING = 7,
        GENERAL = 8,
        HVAC = 9,
        LANDSCAPING = 10,
        MASONRY = 11,
        PAINTING = 12,
        PAVING = 13,
        PLUMBING = 14,
        ROOFING = 15,
        SITEGRADING = 16,
        STEELWORK = 17,
        SURVEYING = 18,
        USERDEFINED = 19,
        NOTDEFINED = 20,
        ___unk = -1
    };

    enum class IfcLampTypeEnum
    {
        COMPACTFLUORESCENT = 0,
        FLUORESCENT = 1,
        HALOGEN = 2,
        HIGHPRESSUREMERCURY = 3,
        HIGHPRESSURESODIUM = 4,
        LED = 5,
        METALHALIDE = 6,
        OLED = 7,
        TUNGSTENFILAMENT = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcLayerSetDirectionEnum
    {
        AXIS1 = 0,
        AXIS2 = 1,
        AXIS3 = 2,
        ___unk = -1
    };

    enum class IfcLightDistributionCurveEnum
    {
        TYPE_A = 0,
        TYPE_B = 1,
        TYPE_C = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcLightEmissionSourceEnum
    {
        COMPACTFLUORESCENT = 0,
        FLUORESCENT = 1,
        HIGHPRESSUREMERCURY = 2,
        HIGHPRESSURESODIUM = 3,
        LIGHTEMITTINGDIODE = 4,
        LOWPRESSURESODIUM = 5,
        LOWVOLTAGEHALOGEN = 6,
        MAINVOLTAGEHALOGEN = 7,
        METALHALIDE = 8,
        TUNGSTENFILAMENT = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcLightFixtureTypeEnum
    {
        POINTSOURCE = 0,
        DIRECTIONSOURCE = 1,
        SECURITYLIGHTING = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcLiquidTerminalTypeEnum
    {
        LOADINGARM = 0,
        HOSEREEL = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcLoadGroupTypeEnum
    {
        LOAD_GROUP = 0,
        LOAD_CASE = 1,
        LOAD_COMBINATION = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcLogicalOperatorEnum
    {
        LOGICALAND = 0,
        LOGICALOR = 1,
        LOGICALXOR = 2,
        LOGICALNOTAND = 3,
        LOGICALNOTOR = 4,
        ___unk = -1
    };

    enum class IfcMarineFacilityTypeEnum
    {
        CANAL = 0,
        WATERWAYSHIPLIFT = 1,
        REVETMENT = 2,
        LAUNCHRECOVERY = 3,
        MARINEDEFENCE = 4,
        HYDROLIFT = 5,
        SHIPYARD = 6,
        SHIPLIFT = 7,
        PORT = 8,
        QUAY = 9,
        FLOATINGDOCK = 10,
        NAVIGATIONALCHANNEL = 11,
        BREAKWATER = 12,
        DRYDOCK = 13,
        JETTY = 14,
        SHIPLOCK = 15,
        BARRIERBEACH = 16,
        SLIPWAY = 17,
        WATERWAY = 18,
        USERDEFINED = 19,
        NOTDEFINED = 20,
        ___unk = -1
    };

    enum class IfcMarinePartTypeEnum
    {
        CREST = 0,
        MANUFACTURING = 1,
        LOWWATERLINE = 2,
        CORE = 3,
        WATERFIELD = 4,
        CILL_LEVEL = 5,
        BERTHINGSTRUCTURE = 6,
        COPELEVEL = 7,
        CHAMBER = 8,
        STORAGEAREA = 9,
        APPROACHCHANNEL = 10,
        VEHICLESERVICING = 11,
        SHIPTRANSFER = 12,
        GATEHEAD = 13,
        GUDINGSTRUCTURE = 14,
        BELOWWATERLINE = 15,
        WEATHERSIDE = 16,
        LANDFIELD = 17,
        PROTECTION = 18,
        LEEWARDSIDE = 19,
        ABOVEWATERLINE = 20,
        ANCHORAGE = 21,
        NAVIGATIONALAREA = 22,
        HIGHWATERLINE = 23,
        USERDEFINED = 24,
        NOTDEFINED = 25,
        ___unk = -1
    };

    enum class IfcMechanicalFastenerTypeEnum
    {
        ANCHORBOLT = 0,
        BOLT = 1,
        DOWEL = 2,
        NAIL = 3,
        NAILPLATE = 4,
        RIVET = 5,
        SCREW = 6,
        SHEARCONNECTOR = 7,
        STAPLE = 8,
        STUDSHEARCONNECTOR = 9,
        COUPLER = 10,
        RAILJOINT = 11,
        RAILFASTENING = 12,
        CHAIN = 13,
        ROPE = 14,
        USERDEFINED = 15,
        NOTDEFINED = 16,
        ___unk = -1
    };

    enum class IfcMedicalDeviceTypeEnum
    {
        AIRSTATION = 0,
        FEEDAIRUNIT = 1,
        OXYGENGENERATOR = 2,
        OXYGENPLANT = 3,
        VACUUMSTATION = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcMemberTypeEnum
    {
        BRACE = 0,
        CHORD = 1,
        COLLAR = 2,
        MEMBER = 3,
        MULLION = 4,
        PLATE = 5,
        POST = 6,
        PURLIN = 7,
        RAFTER = 8,
        STRINGER = 9,
        STRUT = 10,
        STUD = 11,
        STIFFENING_RIB = 12,
        ARCH_SEGMENT = 13,
        SUSPENSION_CABLE = 14,
        SUSPENDER = 15,
        STAY_CABLE = 16,
        STRUCTURALCABLE = 17,
        TIEBAR = 18,
        USERDEFINED = 19,
        NOTDEFINED = 20,
        ___unk = -1
    };

    enum class IfcMobileTelecommunicationsApplianceTypeEnum
    {
        E_UTRAN_NODE_B = 0,
        REMOTERADIOUNIT = 1,
        ACCESSPOINT = 2,
        BASETRANSCEIVERSTATION = 3,
        REMOTEUNIT = 4,
        BASEBANDUNIT = 5,
        MASTERUNIT = 6,
        GATEWAY_GPRS_SUPPORT_NODE = 7,
        SUBSCRIBERSERVER = 8,
        MOBILESWITCHINGCENTER = 9,
        MSCSERVER = 10,
        PACKETCONTROLUNIT = 11,
        SERVICE_GPRS_SUPPORT_NODE = 12,
        USERDEFINED = 13,
        NOTDEFINED = 14,
        ___unk = -1
    };

    enum class IfcMooringDeviceTypeEnum
    {
        LINETENSIONER = 0,
        MAGNETICDEVICE = 1,
        MOORINGHOOKS = 2,
        VACUUMDEVICE = 3,
        BOLLARD = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcMotorConnectionTypeEnum
    {
        BELTDRIVE = 0,
        COUPLING = 1,
        DIRECTDRIVE = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcNavigationElementTypeEnum
    {
        BEACON = 0,
        BUOY = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcObjectiveEnum
    {
        CODECOMPLIANCE = 0,
        CODEWAIVER = 1,
        DESIGNINTENT = 2,
        EXTERNAL = 3,
        HEALTHANDSAFETY = 4,
        MERGECONFLICT = 5,
        MODELVIEW = 6,
        PARAMETER = 7,
        REQUIREMENT = 8,
        SPECIFICATION = 9,
        TRIGGERCONDITION = 10,
        USERDEFINED = 11,
        NOTDEFINED = 12,
        ___unk = -1
    };

    enum class IfcObjectTypeEnum
    {
        PRODUCT = 0,
        PROCESS = 1,
        CONTROL = 2,
        RESOURCE = 3,
        ACTOR = 4,
        GROUP = 5,
        PROJECT = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcOccupantTypeEnum
    {
        ASSIGNEE = 0,
        ASSIGNOR = 1,
        LESSEE = 2,
        LESSOR = 3,
        LETTINGAGENT = 4,
        OWNER = 5,
        TENANT = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcOpeningElementTypeEnum
    {
        OPENING = 0,
        RECESS = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcOutletTypeEnum
    {
        AUDIOVISUALOUTLET = 0,
        COMMUNICATIONSOUTLET = 1,
        POWEROUTLET = 2,
        DATAOUTLET = 3,
        TELEPHONEOUTLET = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcPavementTypeEnum
    {
        FLEXIBLE = 0,
        RIGID = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcPerformanceHistoryTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcPermeableCoveringOperationEnum
    {
        GRILL = 0,
        LOUVER = 1,
        SCREEN = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcPermitTypeEnum
    {
        ACCESS = 0,
        BUILDING = 1,
        WORK = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcPhysicalOrVirtualEnum
    {
        PHYSICAL = 0,
        VIRTUAL = 1,
        NOTDEFINED = 2,
        ___unk = -1
    };

    enum class IfcPileConstructionEnum
    {
        CAST_IN_PLACE = 0,
        COMPOSITE = 1,
        PRECAST_CONCRETE = 2,
        PREFAB_STEEL = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcPileTypeEnum
    {
        BORED = 0,
        DRIVEN = 1,
        JETGROUTING = 2,
        COHESION = 3,
        FRICTION = 4,
        SUPPORT = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcPipeFittingTypeEnum
    {
        BEND = 0,
        CONNECTOR = 1,
        ENTRY = 2,
        EXIT = 3,
        JUNCTION = 4,
        OBSTRUCTION = 5,
        TRANSITION = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcPipeSegmentTypeEnum
    {
        CULVERT = 0,
        FLEXIBLESEGMENT = 1,
        RIGIDSEGMENT = 2,
        GUTTER = 3,
        SPOOL = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcPlateTypeEnum
    {
        CURTAIN_PANEL = 0,
        SHEET = 1,
        FLANGE_PLATE = 2,
        WEB_PLATE = 3,
        STIFFENER_PLATE = 4,
        GUSSET_PLATE = 5,
        COVER_PLATE = 6,
        SPLICE_PLATE = 7,
        BASE_PLATE = 8,
        LAGGING = 9,
        USERDEFINED = 10,
        NOTDEFINED = 11,
        ___unk = -1
    };

    enum class IfcPreferredSurfaceCurveRepresentation
    {
        CURVE3D = 0,
        PCURVE_S1 = 1,
        PCURVE_S2 = 2,
        ___unk = -1
    };

    enum class IfcProcedureTypeEnum
    {
        ADVICE_CAUTION = 0,
        ADVICE_NOTE = 1,
        ADVICE_WARNING = 2,
        CALIBRATION = 3,
        DIAGNOSTIC = 4,
        SHUTDOWN = 5,
        STARTUP = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcProfileTypeEnum
    {
        CURVE = 0,
        AREA = 1,
        ___unk = -1
    };

    enum class IfcProjectedOrTrueLengthEnum
    {
        PROJECTED_LENGTH = 0,
        TRUE_LENGTH = 1,
        ___unk = -1
    };

    enum class IfcProjectionElementTypeEnum
    {
        BLISTER = 0,
        DEVIATOR = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcProjectOrderTypeEnum
    {
        CHANGEORDER = 0,
        MAINTENANCEWORKORDER = 1,
        MOVEORDER = 2,
        PURCHASEORDER = 3,
        WORKORDER = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcPropertySetTemplateTypeEnum
    {
        PSET_MATERIALDRIVEN = 0,
        PSET_TYPEDRIVENONLY = 1,
        PSET_TYPEDRIVENOVERRIDE = 2,
        PSET_OCCURRENCEDRIVEN = 3,
        PSET_PERFORMANCEDRIVEN = 4,
        PSET_PROFILEDRIVEN = 5,
        QTO_TYPEDRIVENONLY = 6,
        QTO_TYPEDRIVENOVERRIDE = 7,
        QTO_OCCURRENCEDRIVEN = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcProtectiveDeviceTrippingUnitTypeEnum
    {
        ELECTRONIC = 0,
        ELECTROMAGNETIC = 1,
        RESIDUALCURRENT = 2,
        THERMAL = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcProtectiveDeviceTypeEnum
    {
        CIRCUITBREAKER = 0,
        EARTHLEAKAGECIRCUITBREAKER = 1,
        EARTHINGSWITCH = 2,
        FUSEDISCONNECTOR = 3,
        RESIDUALCURRENTCIRCUITBREAKER = 4,
        RESIDUALCURRENTSWITCH = 5,
        VARISTOR = 6,
        ANTI_ARCING_DEVICE = 7,
        SPARKGAP = 8,
        VOLTAGELIMITER = 9,
        USERDEFINED = 10,
        NOTDEFINED = 11,
        ___unk = -1
    };

    enum class IfcPumpTypeEnum
    {
        CIRCULATOR = 0,
        ENDSUCTION = 1,
        SPLITCASE = 2,
        SUBMERSIBLEPUMP = 3,
        SUMPPUMP = 4,
        VERTICALINLINE = 5,
        VERTICALTURBINE = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcRailingTypeEnum
    {
        HANDRAIL = 0,
        GUARDRAIL = 1,
        BALUSTRADE = 2,
        FENCE = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcRailTypeEnum
    {
        RACKRAIL = 0,
        BLADE = 1,
        GUARDRAIL = 2,
        STOCKRAIL = 3,
        CHECKRAIL = 4,
        RAIL = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcRailwayPartTypeEnum
    {
        TRACKSTRUCTURE = 0,
        TRACKSTRUCTUREPART = 1,
        LINESIDESTRUCTUREPART = 2,
        DILATATIONSUPERSTRUCTURE = 3,
        PLAINTRACKSUPERSTRUCTURE = 4,
        LINESIDESTRUCTURE = 5,
        SUPERSTRUCTURE = 6,
        TURNOUTSUPERSTRUCTURE = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcRailwayTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcRampFlightTypeEnum
    {
        STRAIGHT = 0,
        SPIRAL = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcRampTypeEnum
    {
        STRAIGHT_RUN_RAMP = 0,
        TWO_STRAIGHT_RUN_RAMP = 1,
        QUARTER_TURN_RAMP = 2,
        TWO_QUARTER_TURN_RAMP = 3,
        HALF_TURN_RAMP = 4,
        SPIRAL_RAMP = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcRecurrenceTypeEnum
    {
        DAILY = 0,
        WEEKLY = 1,
        MONTHLY_BY_DAY_OF_MONTH = 2,
        MONTHLY_BY_POSITION = 3,
        BY_DAY_COUNT = 4,
        BY_WEEKDAY_COUNT = 5,
        YEARLY_BY_DAY_OF_MONTH = 6,
        YEARLY_BY_POSITION = 7,
        ___unk = -1
    };

    enum class IfcReferentTypeEnum
    {
        STATION = 0,
        REFERENCEMARKER = 1,
        LANDMARK = 2,
        BOUNDARY = 3,
        INTERSECTION = 4,
        POSITION = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcReflectanceMethodEnum
    {
        BLINN = 0,
        FLAT = 1,
        GLASS = 2,
        MATT = 3,
        METAL = 4,
        MIRROR = 5,
        PHONG = 6,
        PHYSICAL = 7,
        PLASTIC = 8,
        STRAUSS = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcReinforcingBarRoleEnum
    {
        MAIN = 0,
        SHEAR = 1,
        LIGATURE = 2,
        STUD = 3,
        PUNCHING = 4,
        EDGE = 5,
        RING = 6,
        ANCHORING = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcReinforcingBarSurfaceEnum
    {
        PLAIN = 0,
        TEXTURED = 1,
        ___unk = -1
    };

    enum class IfcReinforcingBarTypeEnum
    {
        ANCHORING = 0,
        EDGE = 1,
        LIGATURE = 2,
        MAIN = 3,
        PUNCHING = 4,
        RING = 5,
        SHEAR = 6,
        STUD = 7,
        SPACEBAR = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcReinforcingMeshTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcRoadPartTypeEnum
    {
        ROADSIDEPART = 0,
        BUS_STOP = 1,
        HARDSHOULDER = 2,
        INTERSECTION = 3,
        PASSINGBAY = 4,
        ROADWAYPLATEAU = 5,
        ROADSIDE = 6,
        REFUGEISLAND = 7,
        TOLLPLAZA = 8,
        CENTRALRESERVE = 9,
        SIDEWALK = 10,
        PARKINGBAY = 11,
        RAILWAYCROSSING = 12,
        PEDESTRIAN_CROSSING = 13,
        SOFTSHOULDER = 14,
        BICYCLECROSSING = 15,
        CENTRALISLAND = 16,
        SHOULDER = 17,
        TRAFFICLANE = 18,
        ROADSEGMENT = 19,
        ROUNDABOUT = 20,
        LAYBY = 21,
        CARRIAGEWAY = 22,
        TRAFFICISLAND = 23,
        USERDEFINED = 24,
        NOTDEFINED = 25,
        ___unk = -1
    };

    enum class IfcRoadTypeEnum
    {
        USERDEFINED = 0,
        NOTDEFINED = 1,
        ___unk = -1
    };

    enum class IfcRoleEnum
    {
        SUPPLIER = 0,
        MANUFACTURER = 1,
        CONTRACTOR = 2,
        SUBCONTRACTOR = 3,
        ARCHITECT = 4,
        STRUCTURALENGINEER = 5,
        COSTENGINEER = 6,
        CLIENT = 7,
        BUILDINGOWNER = 8,
        BUILDINGOPERATOR = 9,
        MECHANICALENGINEER = 10,
        ELECTRICALENGINEER = 11,
        PROJECTMANAGER = 12,
        FACILITIESMANAGER = 13,
        CIVILENGINEER = 14,
        COMMISSIONINGENGINEER = 15,
        ENGINEER = 16,
        OWNER = 17,
        CONSULTANT = 18,
        CONSTRUCTIONMANAGER = 19,
        FIELDCONSTRUCTIONMANAGER = 20,
        RESELLER = 21,
        USERDEFINED = 22,
        ___unk = -1
    };

    enum class IfcRoofTypeEnum
    {
        FLAT_ROOF = 0,
        SHED_ROOF = 1,
        GABLE_ROOF = 2,
        HIP_ROOF = 3,
        HIPPED_GABLE_ROOF = 4,
        GAMBREL_ROOF = 5,
        MANSARD_ROOF = 6,
        BARREL_ROOF = 7,
        RAINBOW_ROOF = 8,
        BUTTERFLY_ROOF = 9,
        PAVILION_ROOF = 10,
        DOME_ROOF = 11,
        FREEFORM = 12,
        USERDEFINED = 13,
        NOTDEFINED = 14,
        ___unk = -1
    };

    enum class IfcSanitaryTerminalTypeEnum
    {
        BATH = 0,
        BIDET = 1,
        CISTERN = 2,
        SHOWER = 3,
        SINK = 4,
        SANITARYFOUNTAIN = 5,
        TOILETPAN = 6,
        URINAL = 7,
        WASHHANDBASIN = 8,
        WCSEAT = 9,
        USERDEFINED = 10,
        NOTDEFINED = 11,
        ___unk = -1
    };

    enum class IfcSectionTypeEnum
    {
        UNIFORM = 0,
        TAPERED = 1,
        ___unk = -1
    };

    enum class IfcSensorTypeEnum
    {
        COSENSOR = 0,
        CO2SENSOR = 1,
        CONDUCTANCESENSOR = 2,
        CONTACTSENSOR = 3,
        FIRESENSOR = 4,
        FLOWSENSOR = 5,
        FROSTSENSOR = 6,
        GASSENSOR = 7,
        HEATSENSOR = 8,
        HUMIDITYSENSOR = 9,
        IDENTIFIERSENSOR = 10,
        IONCONCENTRATIONSENSOR = 11,
        LEVELSENSOR = 12,
        LIGHTSENSOR = 13,
        MOISTURESENSOR = 14,
        MOVEMENTSENSOR = 15,
        PHSENSOR = 16,
        PRESSURESENSOR = 17,
        RADIATIONSENSOR = 18,
        RADIOACTIVITYSENSOR = 19,
        SMOKESENSOR = 20,
        SOUNDSENSOR = 21,
        TEMPERATURESENSOR = 22,
        WINDSENSOR = 23,
        EARTHQUAKESENSOR = 24,
        FOREIGNOBJECTDETECTIONSENSOR = 25,
        OBSTACLESENSOR = 26,
        RAINSENSOR = 27,
        SNOWDEPTHSENSOR = 28,
        TRAINSENSOR = 29,
        TURNOUTCLOSURESENSOR = 30,
        WHEELSENSOR = 31,
        USERDEFINED = 32,
        NOTDEFINED = 33,
        ___unk = -1
    };

    enum class IfcSequenceEnum
    {
        START_START = 0,
        START_FINISH = 1,
        FINISH_START = 2,
        FINISH_FINISH = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcShadingDeviceTypeEnum
    {
        JALOUSIE = 0,
        SHUTTER = 1,
        AWNING = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcSignalTypeEnum
    {
        VISUAL = 0,
        AUDIO = 1,
        MIXED = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcSignTypeEnum
    {
        MARKER = 0,
        PICTORAL = 1,
        MIRROR = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcSimplePropertyTemplateTypeEnum
    {
        P_SINGLEVALUE = 0,
        P_ENUMERATEDVALUE = 1,
        P_BOUNDEDVALUE = 2,
        P_LISTVALUE = 3,
        P_TABLEVALUE = 4,
        P_REFERENCEVALUE = 5,
        Q_LENGTH = 6,
        Q_AREA = 7,
        Q_VOLUME = 8,
        Q_COUNT = 9,
        Q_WEIGHT = 10,
        Q_TIME = 11,
        ___unk = -1
    };

    enum class IfcSIPrefix
    {
        EXA = 0,
        PETA = 1,
        TERA = 2,
        GIGA = 3,
        MEGA = 4,
        KILO = 5,
        HECTO = 6,
        DECA = 7,
        DECI = 8,
        CENTI = 9,
        MILLI = 10,
        MICRO = 11,
        NANO = 12,
        PICO = 13,
        FEMTO = 14,
        ATTO = 15,
        ___unk = -1
    };

    enum class IfcSIUnitName
    {
        AMPERE = 0,
        BECQUEREL = 1,
        CANDELA = 2,
        COULOMB = 3,
        CUBIC_METRE = 4,
        DEGREE_CELSIUS = 5,
        FARAD = 6,
        GRAM = 7,
        GRAY = 8,
        HENRY = 9,
        HERTZ = 10,
        JOULE = 11,
        KELVIN = 12,
        LUMEN = 13,
        LUX = 14,
        METRE = 15,
        MOLE = 16,
        NEWTON = 17,
        OHM = 18,
        PASCAL = 19,
        RADIAN = 20,
        SECOND = 21,
        SIEMENS = 22,
        SIEVERT = 23,
        SQUARE_METRE = 24,
        STERADIAN = 25,
        TESLA = 26,
        VOLT = 27,
        WATT = 28,
        WEBER = 29,
        ___unk = -1
    };

    enum class IfcSlabTypeEnum
    {
        FLOOR = 0,
        ROOF = 1,
        LANDING = 2,
        BASESLAB = 3,
        APPROACH_SLAB = 4,
        PAVING = 5,
        WEARING = 6,
        SIDEWALK = 7,
        TRACKSLAB = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcSolarDeviceTypeEnum
    {
        SOLARCOLLECTOR = 0,
        SOLARPANEL = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcSpaceHeaterTypeEnum
    {
        CONVECTOR = 0,
        RADIATOR = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcSpaceTypeEnum
    {
        SPACE = 0,
        PARKING = 1,
        GFA = 2,
        INTERNAL = 3,
        EXTERNAL = 4,
        BERTH = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcSpatialZoneTypeEnum
    {
        CONSTRUCTION = 0,
        FIRESAFETY = 1,
        LIGHTING = 2,
        OCCUPANCY = 3,
        SECURITY = 4,
        THERMAL = 5,
        TRANSPORT = 6,
        VENTILATION = 7,
        RESERVATION = 8,
        INTERFERENCE = 9,
        MAPPEDZONE = 10,
        TESTEDZONE = 11,
        COMPARTMENT = 12,
        ANNULARGAP = 13,
        CLEARANCE = 14,
        INSTALLATION = 15,
        INTERIOR = 16,
        INVERT = 17,
        LINING = 18,
        SERVICE = 19,
        USERDEFINED = 20,
        NOTDEFINED = 21,
        ___unk = -1
    };

    enum class IfcStackTerminalTypeEnum
    {
        BIRDCAGE = 0,
        COWL = 1,
        RAINWATERHOPPER = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcStairFlightTypeEnum
    {
        STRAIGHT = 0,
        WINDER = 1,
        SPIRAL = 2,
        CURVED = 3,
        FREEFORM = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcStairTypeEnum
    {
        STRAIGHT_RUN_STAIR = 0,
        TWO_STRAIGHT_RUN_STAIR = 1,
        QUARTER_WINDING_STAIR = 2,
        QUARTER_TURN_STAIR = 3,
        HALF_WINDING_STAIR = 4,
        HALF_TURN_STAIR = 5,
        TWO_QUARTER_WINDING_STAIR = 6,
        TWO_QUARTER_TURN_STAIR = 7,
        THREE_QUARTER_WINDING_STAIR = 8,
        THREE_QUARTER_TURN_STAIR = 9,
        SPIRAL_STAIR = 10,
        DOUBLE_RETURN_STAIR = 11,
        CURVED_RUN_STAIR = 12,
        TWO_CURVED_RUN_STAIR = 13,
        LADDER = 14,
        USERDEFINED = 15,
        NOTDEFINED = 16,
        ___unk = -1
    };

    enum class IfcStateEnum
    {
        READWRITE = 0,
        READONLY = 1,
        LOCKED = 2,
        READWRITELOCKED = 3,
        READONLYLOCKED = 4,
        ___unk = -1
    };

    enum class IfcStructuralCurveActivityTypeEnum
    {
        CONST_ = 0,
        LINEAR = 1,
        POLYGONAL = 2,
        EQUIDISTANT = 3,
        SINUS = 4,
        PARABOLA = 5,
        DISCRETE = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcStructuralCurveMemberTypeEnum
    {
        RIGID_JOINED_MEMBER = 0,
        PIN_JOINED_MEMBER = 1,
        CABLE = 2,
        TENSION_MEMBER = 3,
        COMPRESSION_MEMBER = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcStructuralSurfaceActivityTypeEnum
    {
        CONST_ = 0,
        BILINEAR = 1,
        DISCRETE = 2,
        ISOCONTOUR = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcStructuralSurfaceMemberTypeEnum
    {
        BENDING_ELEMENT = 0,
        MEMBRANE_ELEMENT = 1,
        SHELL = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcSubContractResourceTypeEnum
    {
        PURCHASE = 0,
        WORK = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcSurfaceFeatureTypeEnum
    {
        MARK = 0,
        TAG = 1,
        TREATMENT = 2,
        DEFECT = 3,
        HATCHMARKING = 4,
        LINEMARKING = 5,
        PAVEMENTSURFACEMARKING = 6,
        SYMBOLMARKING = 7,
        NONSKIDSURFACING = 8,
        RUMBLESTRIP = 9,
        TRANSVERSERUMBLESTRIP = 10,
        USERDEFINED = 11,
        NOTDEFINED = 12,
        ___unk = -1
    };

    enum class IfcSurfaceSide
    {
        POSITIVE = 0,
        NEGATIVE = 1,
        BOTH = 2,
        ___unk = -1
    };

    enum class IfcSwitchingDeviceTypeEnum
    {
        CONTACTOR = 0,
        DIMMERSWITCH = 1,
        EMERGENCYSTOP = 2,
        KEYPAD = 3,
        MOMENTARYSWITCH = 4,
        SELECTORSWITCH = 5,
        STARTER = 6,
        SWITCHDISCONNECTOR = 7,
        TOGGLESWITCH = 8,
        RELAY = 9,
        START_AND_STOP_EQUIPMENT = 10,
        USERDEFINED = 11,
        NOTDEFINED = 12,
        ___unk = -1
    };

    enum class IfcSystemFurnitureElementTypeEnum
    {
        PANEL = 0,
        WORKSURFACE = 1,
        SUBRACK = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcTankTypeEnum
    {
        BASIN = 0,
        BREAKPRESSURE = 1,
        EXPANSION = 2,
        FEEDANDEXPANSION = 3,
        PRESSUREVESSEL = 4,
        STORAGE = 5,
        VESSEL = 6,
        OILRETENTIONTRAY = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcTaskDurationEnum
    {
        ELAPSEDTIME = 0,
        WORKTIME = 1,
        NOTDEFINED = 2,
        ___unk = -1
    };

    enum class IfcTaskTypeEnum
    {
        ADJUSTMENT = 0,
        ATTENDANCE = 1,
        CALIBRATION = 2,
        CONSTRUCTION = 3,
        DEMOLITION = 4,
        DISMANTLE = 5,
        DISPOSAL = 6,
        EMERGENCY = 7,
        INSPECTION = 8,
        INSTALLATION = 9,
        LOGISTIC = 10,
        MAINTENANCE = 11,
        MOVE = 12,
        OPERATION = 13,
        REMOVAL = 14,
        RENOVATION = 15,
        SAFETY = 16,
        SHUTDOWN = 17,
        STARTUP = 18,
        TESTING = 19,
        TROUBLESHOOTING = 20,
        USERDEFINED = 21,
        NOTDEFINED = 22,
        ___unk = -1
    };

    enum class IfcTendonAnchorTypeEnum
    {
        COUPLER = 0,
        FIXED_END = 1,
        TENSIONING_END = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcTendonConduitTypeEnum
    {
        DUCT = 0,
        COUPLER = 1,
        GROUTING_DUCT = 2,
        TRUMPET = 3,
        DIABOLO = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcTendonTypeEnum
    {
        BAR = 0,
        COATED = 1,
        STRAND = 2,
        WIRE = 3,
        USERDEFINED = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcTextPath
    {
        LEFT = 0,
        RIGHT = 1,
        UP = 2,
        DOWN = 3,
        ___unk = -1
    };

    enum class IfcTimeSeriesDataTypeEnum
    {
        CONTINUOUS = 0,
        DISCRETE = 1,
        DISCRETEBINARY = 2,
        PIECEWISEBINARY = 3,
        PIECEWISECONSTANT = 4,
        PIECEWISECONTINUOUS = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcTrackElementTypeEnum
    {
        TRACKENDOFALIGNMENT = 0,
        BLOCKINGDEVICE = 1,
        VEHICLESTOP = 2,
        SLEEPER = 3,
        HALF_SET_OF_BLADES = 4,
        SPEEDREGULATOR = 5,
        DERAILER = 6,
        FROG = 7,
        USERDEFINED = 8,
        NOTDEFINED = 9,
        ___unk = -1
    };

    enum class IfcTransformerTypeEnum
    {
        CURRENT = 0,
        FREQUENCY = 1,
        INVERTER = 2,
        RECTIFIER = 3,
        VOLTAGE = 4,
        CHOPPER = 5,
        COMBINED = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcTransitionCode
    {
        DISCONTINUOUS = 0,
        CONTINUOUS = 1,
        CONTSAMEGRADIENT = 2,
        CONTSAMEGRADIENTSAMECURVATURE = 3,
        ___unk = -1
    };

    enum class IfcTransportElementTypeEnum
    {
        ELEVATOR = 0,
        ESCALATOR = 1,
        MOVINGWALKWAY = 2,
        CRANEWAY = 3,
        LIFTINGGEAR = 4,
        HAULINGGEAR = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcTrimmingPreference
    {
        CARTESIAN = 0,
        PARAMETER = 1,
        UNSPECIFIED = 2,
        ___unk = -1
    };

    enum class IfcTubeBundleTypeEnum
    {
        FINNED = 0,
        USERDEFINED = 1,
        NOTDEFINED = 2,
        ___unk = -1
    };

    enum class IfcTunnelPartTypeEnum
    {
        TUNNELSECTION = 0,
        CROSSWAY = 1,
        RINGSECTION = 2,
        PORTAL = 3,
        NOTDEFINED = 4,
        USERDEFINED = 5,
        ___unk = -1
    };

    enum class IfcTunnelTypeEnum
    {
        ACCESSTUNNEL = 0,
        SHAFT = 1,
        UTILITIES = 2,
        RAILWAY = 3,
        ROAD = 4,
        PEDESTRIAN = 5,
        METRO = 6,
        BICYCLE = 7,
        BYPASS = 8,
        MAINTENANCE = 9,
        UNDERGROUND_FACILITIES = 10,
        RAMP = 11,
        NOTDEFINED = 12,
        USERDEFINED = 13,
        ___unk = -1
    };

    enum class IfcUndergroundExcavationTypeEnum
    {
        FACEEXCAVATION = 0,
        RADIALEXCAVATION = 1,
        USERDEFINED = 2,
        NOTDEFINED = 3,
        ___unk = -1
    };

    enum class IfcUnitaryControlElementTypeEnum
    {
        ALARMPANEL = 0,
        CONTROLPANEL = 1,
        GASDETECTIONPANEL = 2,
        INDICATORPANEL = 3,
        MIMICPANEL = 4,
        HUMIDISTAT = 5,
        THERMOSTAT = 6,
        WEATHERSTATION = 7,
        COMBINED = 8,
        BASESTATIONCONTROLLER = 9,
        USERDEFINED = 10,
        NOTDEFINED = 11,
        ___unk = -1
    };

    enum class IfcUnitaryEquipmentTypeEnum
    {
        AIRHANDLER = 0,
        AIRCONDITIONINGUNIT = 1,
        DEHUMIDIFIER = 2,
        SPLITSYSTEM = 3,
        ROOFTOPUNIT = 4,
        USERDEFINED = 5,
        NOTDEFINED = 6,
        ___unk = -1
    };

    enum class IfcUnitEnum
    {
        ABSORBEDDOSEUNIT = 0,
        AMOUNTOFSUBSTANCEUNIT = 1,
        AREAUNIT = 2,
        DOSEEQUIVALENTUNIT = 3,
        ELECTRICCAPACITANCEUNIT = 4,
        ELECTRICCHARGEUNIT = 5,
        ELECTRICCONDUCTANCEUNIT = 6,
        ELECTRICCURRENTUNIT = 7,
        ELECTRICRESISTANCEUNIT = 8,
        ELECTRICVOLTAGEUNIT = 9,
        ENERGYUNIT = 10,
        FORCEUNIT = 11,
        FREQUENCYUNIT = 12,
        ILLUMINANCEUNIT = 13,
        INDUCTANCEUNIT = 14,
        LENGTHUNIT = 15,
        LUMINOUSFLUXUNIT = 16,
        LUMINOUSINTENSITYUNIT = 17,
        MAGNETICFLUXDENSITYUNIT = 18,
        MAGNETICFLUXUNIT = 19,
        MASSUNIT = 20,
        PLANEANGLEUNIT = 21,
        POWERUNIT = 22,
        PRESSUREUNIT = 23,
        RADIOACTIVITYUNIT = 24,
        SOLIDANGLEUNIT = 25,
        THERMODYNAMICTEMPERATUREUNIT = 26,
        TIMEUNIT = 27,
        VOLUMEUNIT = 28,
        USERDEFINED = 29,
        ___unk = -1
    };

    enum class IfcValveTypeEnum
    {
        AIRRELEASE = 0,
        ANTIVACUUM = 1,
        CHANGEOVER = 2,
        CHECK = 3,
        COMMISSIONING = 4,
        DIVERTING = 5,
        DRAWOFFCOCK = 6,
        DOUBLECHECK = 7,
        DOUBLEREGULATING = 8,
        FAUCET = 9,
        FLUSHING = 10,
        GASCOCK = 11,
        GASTAP = 12,
        ISOLATING = 13,
        MIXING = 14,
        PRESSUREREDUCING = 15,
        PRESSURERELIEF = 16,
        REGULATING = 17,
        SAFETYCUTOFF = 18,
        STEAMTRAP = 19,
        STOPCOCK = 20,
        USERDEFINED = 21,
        NOTDEFINED = 22,
        ___unk = -1
    };

    enum class IfcVehicleTypeEnum
    {
        VEHICLE = 0,
        VEHICLETRACKED = 1,
        ROLLINGSTOCK = 2,
        VEHICLEWHEELED = 3,
        VEHICLEAIR = 4,
        CARGO = 5,
        VEHICLEMARINE = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcVibrationDamperTypeEnum
    {
        BENDING_YIELD = 0,
        SHEAR_YIELD = 1,
        AXIAL_YIELD = 2,
        FRICTION = 3,
        VISCOUS = 4,
        RUBBER = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcVibrationIsolatorTypeEnum
    {
        COMPRESSION = 0,
        SPRING = 1,
        BASE = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcVoidingFeatureTypeEnum
    {
        CUTOUT = 0,
        NOTCH = 1,
        HOLE = 2,
        MITER = 3,
        CHAMFER = 4,
        EDGE = 5,
        USERDEFINED = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcWallTypeEnum
    {
        MOVABLE = 0,
        PARAPET = 1,
        PARTITIONING = 2,
        PLUMBINGWALL = 3,
        SHEAR = 4,
        SOLIDWALL = 5,
        STANDARD = 6,
        POLYGONAL = 7,
        ELEMENTEDWALL = 8,
        RETAININGWALL = 9,
        WAVEWALL = 10,
        USERDEFINED = 11,
        NOTDEFINED = 12,
        ___unk = -1
    };

    enum class IfcWasteTerminalTypeEnum
    {
        FLOORTRAP = 0,
        FLOORWASTE = 1,
        GULLYSUMP = 2,
        GULLYTRAP = 3,
        ROOFDRAIN = 4,
        WASTEDISPOSALUNIT = 5,
        WASTETRAP = 6,
        USERDEFINED = 7,
        NOTDEFINED = 8,
        ___unk = -1
    };

    enum class IfcWindowPanelOperationEnum
    {
        SIDEHUNGRIGHTHAND = 0,
        SIDEHUNGLEFTHAND = 1,
        TILTANDTURNRIGHTHAND = 2,
        TILTANDTURNLEFTHAND = 3,
        TOPHUNG = 4,
        BOTTOMHUNG = 5,
        PIVOTHORIZONTAL = 6,
        PIVOTVERTICAL = 7,
        SLIDINGHORIZONTAL = 8,
        SLIDINGVERTICAL = 9,
        REMOVABLECASEMENT = 10,
        FIXEDCASEMENT = 11,
        OTHEROPERATION = 12,
        NOTDEFINED = 13,
        ___unk = -1
    };

    enum class IfcWindowPanelPositionEnum
    {
        LEFT = 0,
        MIDDLE = 1,
        RIGHT = 2,
        BOTTOM = 3,
        TOP = 4,
        NOTDEFINED = 5,
        ___unk = -1
    };

    enum class IfcWindowStyleConstructionEnum
    {
        ALUMINIUM = 0,
        HIGH_GRADE_STEEL = 1,
        STEEL = 2,
        WOOD = 3,
        ALUMINIUM_WOOD = 4,
        PLASTIC = 5,
        OTHER_CONSTRUCTION = 6,
        NOTDEFINED = 7,
        ___unk = -1
    };

    enum class IfcWindowStyleOperationEnum
    {
        SINGLE_PANEL = 0,
        DOUBLE_PANEL_VERTICAL = 1,
        DOUBLE_PANEL_HORIZONTAL = 2,
        TRIPLE_PANEL_VERTICAL = 3,
        TRIPLE_PANEL_BOTTOM = 4,
        TRIPLE_PANEL_TOP = 5,
        TRIPLE_PANEL_LEFT = 6,
        TRIPLE_PANEL_RIGHT = 7,
        TRIPLE_PANEL_HORIZONTAL = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcWindowTypeEnum
    {
        WINDOW = 0,
        SKYLIGHT = 1,
        LIGHTDOME = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcWindowTypePartitioningEnum
    {
        SINGLE_PANEL = 0,
        DOUBLE_PANEL_VERTICAL = 1,
        DOUBLE_PANEL_HORIZONTAL = 2,
        TRIPLE_PANEL_VERTICAL = 3,
        TRIPLE_PANEL_BOTTOM = 4,
        TRIPLE_PANEL_TOP = 5,
        TRIPLE_PANEL_LEFT = 6,
        TRIPLE_PANEL_RIGHT = 7,
        TRIPLE_PANEL_HORIZONTAL = 8,
        USERDEFINED = 9,
        NOTDEFINED = 10,
        ___unk = -1
    };

    enum class IfcWorkCalendarTypeEnum
    {
        FIRSTSHIFT = 0,
        SECONDSHIFT = 1,
        THIRDSHIFT = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcWorkPlanTypeEnum
    {
        ACTUAL = 0,
        BASELINE = 1,
        PLANNED = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };

    enum class IfcWorkScheduleTypeEnum
    {
        ACTUAL = 0,
        BASELINE = 1,
        PLANNED = 2,
        USERDEFINED = 3,
        NOTDEFINED = 4,
        ___unk = -1
    };
    //
    static TextValue IfcActionRequestTypeEnum_[] = {"EMAIL", "FAX", "PHONE", "POST", "VERBAL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcActionSourceTypeEnum_[] = {"DEAD_LOAD_G", "COMPLETION_G1", "LIVE_LOAD_Q", "SNOW_S", "WIND_W", "PRESTRESSING_P", "SETTLEMENT_U", "TEMPERATURE_T", "EARTHQUAKE_E", "FIRE", "IMPULSE", "IMPACT", "TRANSPORT", "ERECTION", "PROPPING", "SYSTEM_IMPERFECTION", "SHRINKAGE", "CREEP", "LACK_OF_FIT", "BUOYANCY", "ICE", "CURRENT", "WAVE", "RAIN", "BRAKES", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcActionTypeEnum_[] = {"PERMANENT_G", "VARIABLE_Q", "EXTRAORDINARY_A", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcActuatorTypeEnum_[] = {"ELECTRICACTUATOR", "HANDOPERATEDACTUATOR", "HYDRAULICACTUATOR", "PNEUMATICACTUATOR", "THERMOSTATICACTUATOR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcAddressTypeEnum_[] = {"OFFICE", "SITE", "HOME", "DISTRIBUTIONPOINT", "USERDEFINED", NULL};
    static TextValue IfcAirTerminalBoxTypeEnum_[] = {"CONSTANTFLOW", "VARIABLEFLOWPRESSUREDEPENDANT", "VARIABLEFLOWPRESSUREINDEPENDANT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcAirTerminalTypeEnum_[] = {"DIFFUSER", "GRILLE", "LOUVRE", "REGISTER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcAirToAirHeatRecoveryTypeEnum_[] = {"FIXEDPLATECOUNTERFLOWEXCHANGER", "FIXEDPLATECROSSFLOWEXCHANGER", "FIXEDPLATEPARALLELFLOWEXCHANGER", "ROTARYWHEEL", "RUNAROUNDCOILLOOP", "HEATPIPE", "TWINTOWERENTHALPYRECOVERYLOOPS", "THERMOSIPHONSEALEDTUBEHEATEXCHANGERS", "THERMOSIPHONCOILTYPEHEATEXCHANGERS", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcAlarmTypeEnum_[] = {"BELL", "BREAKGLASSBUTTON", "LIGHT", "MANUALPULLBOX", "SIREN", "WHISTLE", "RAILWAYCROCODILE", "RAILWAYDETONATOR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcAlignmentCantSegmentTypeEnum_[] = {"BLOSSCURVE", "CONSTANTCANT", "COSINECURVE", "HELMERTCURVE", "LINEARTRANSITION", "SINECURVE", "VIENNESEBEND", NULL};
    static TextValue IfcAlignmentHorizontalSegmentTypeEnum_[] = {"LINE", "CIRCULARARC", "CLOTHOID", "CUBIC", "HELMERTCURVE", "BLOSSCURVE", "COSINECURVE", "SINECURVE", "VIENNESEBEND", NULL};
    static TextValue IfcAlignmentTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcAlignmentVerticalSegmentTypeEnum_[] = {"CONSTANTGRADIENT", "CIRCULARARC", "PARABOLICARC", "CLOTHOID", NULL};
    static TextValue IfcAnalysisModelTypeEnum_[] = {"IN_PLANE_LOADING_2D", "OUT_PLANE_LOADING_2D", "LOADING_3D", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcAnalysisTheoryTypeEnum_[] = {"FIRST_ORDER_THEORY", "SECOND_ORDER_THEORY", "THIRD_ORDER_THEORY", "FULL_NONLINEAR_THEORY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcAnnotationTypeEnum_[] = {"ASSUMEDPOINT", "ASBUILTAREA", "ASBUILTLINE", "NON_PHYSICAL_SIGNAL", "ASSUMEDLINE", "WIDTHEVENT", "ASSUMEDAREA", "SUPERELEVATIONEVENT", "ASBUILTPOINT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcArithmeticOperatorEnum_[] = {"ADD", "DIVIDE", "MULTIPLY", "SUBTRACT", NULL};
    static TextValue IfcAssemblyPlaceEnum_[] = {"SITE", "FACTORY", "NOTDEFINED", NULL};
    static TextValue IfcAudioVisualApplianceTypeEnum_[] = {"AMPLIFIER", "CAMERA", "DISPLAY", "MICROPHONE", "PLAYER", "PROJECTOR", "RECEIVER", "SPEAKER", "SWITCHER", "TELEPHONE", "TUNER", "COMMUNICATIONTERMINAL", "RECORDINGEQUIPMENT", "USERDEFINED", "NOTDEFINED", "SIREN", "BEACON", NULL};
    static TextValue IfcBeamTypeEnum_[] = {"BEAM", "JOIST", "HOLLOWCORE", "LINTEL", "SPANDREL", "T_BEAM", "GIRDER_SEGMENT", "DIAPHRAGM", "PIERCAP", "HATSTONE", "CORNICE", "EDGEBEAM", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBearingTypeDisplacementEnum_[] = {"FIXED_MOVEMENT", "GUIDED_LONGITUDINAL", "GUIDED_TRANSVERSAL", "FREE_MOVEMENT", "NOTDEFINED", NULL};
    static TextValue IfcBearingTypeEnum_[] = {"CYLINDRICAL", "SPHERICAL", "ELASTOMERIC", "POT", "GUIDE", "ROCKER", "ROLLER", "DISK", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBenchmarkEnum_[] = {"GREATERTHAN", "GREATERTHANOREQUALTO", "LESSTHAN", "LESSTHANOREQUALTO", "EQUALTO", "NOTEQUALTO", "INCLUDES", "NOTINCLUDES", "INCLUDEDIN", "NOTINCLUDEDIN", NULL};
    static TextValue IfcBoilerTypeEnum_[] = {"WATER", "STEAM", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBooleanOperator_[] = {"UNION", "INTERSECTION", "DIFFERENCE", NULL};
    static TextValue IfcBoreholeTypeEnum_[] = {"COREDRILLING", "DESTRUCTIVEDRILLING", "TRIALPIT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBridgePartTypeEnum_[] = {"ABUTMENT", "DECK", "DECK_SEGMENT", "FOUNDATION", "PIER", "PIER_SEGMENT", "PYLON", "SUBSTRUCTURE", "SUPERSTRUCTURE", "SURFACESTRUCTURE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBridgeTypeEnum_[] = {"ARCHED", "CABLE_STAYED", "CANTILEVER", "CULVERT", "FRAMEWORK", "GIRDER", "SUSPENSION", "TRUSS", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBSplineCurveForm_[] = {"POLYLINE_FORM", "CIRCULAR_ARC", "ELLIPTIC_ARC", "PARABOLIC_ARC", "HYPERBOLIC_ARC", "UNSPECIFIED", NULL};
    static TextValue IfcBSplineSurfaceForm_[] = {"PLANE_SURF", "CYLINDRICAL_SURF", "CONICAL_SURF", "SPHERICAL_SURF", "TOROIDAL_SURF", "SURF_OF_REVOLUTION", "RULED_SURF", "GENERALISED_CONE", "QUADRIC_SURF", "SURF_OF_LINEAR_EXTRUSION", "UNSPECIFIED", NULL};
    static TextValue IfcBuildingElementPartTypeEnum_[] = {"INSULATION", "PRECASTPANEL", "APRON", "ARMOURUNIT", "SAFETYCAGE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBuildingElementProxyTypeEnum_[] = {"COMPLEX", "ELEMENT", "PARTIAL", "PROVISIONFORVOID", "PROVISIONFORSPACE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBuildingSystemTypeEnum_[] = {"FENESTRATION", "FOUNDATION", "LOADBEARING", "OUTERSHELL", "SHADING", "TRANSPORT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBuiltSystemTypeEnum_[] = {"REINFORCING", "MOORING", "OUTERSHELL", "TRACKCIRCUIT", "EROSIONPREVENTION", "FOUNDATION", "LOADBEARING", "SHADING", "FENESTRATION", "TRANSPORT", "PRESTRESSING", "RAILWAYLINE", "RAILWAYTRACK", "TUNNEL_PRESUPPORT", "TUNNEL_SUPPORT", "TUNNEL_LINING", "WATERPROOFING", "FIREPROTECTION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcBurnerTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCableCarrierFittingTypeEnum_[] = {"BEND", "CROSS", "REDUCER", "TEE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCableCarrierSegmentTypeEnum_[] = {"CABLELADDERSEGMENT", "CABLETRAYSEGMENT", "CABLETRUNKINGSEGMENT", "CONDUITSEGMENT", "CABLEBRACKET", "CATENARYWIRE", "DROPPER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCableFittingTypeEnum_[] = {"CONNECTOR", "ENTRY", "EXIT", "JUNCTION", "TRANSITION", "FANOUT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCableSegmentTypeEnum_[] = {"BUSBARSEGMENT", "CABLESEGMENT", "CONDUCTORSEGMENT", "CORESEGMENT", "CONTACTWIRESEGMENT", "FIBERSEGMENT", "FIBERTUBE", "OPTICALCABLESEGMENT", "STITCHWIRE", "WIREPAIRSEGMENT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCaissonFoundationTypeEnum_[] = {"WELL", "CAISSON", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcChangeActionEnum_[] = {"NOCHANGE", "MODIFIED", "ADDED", "DELETED", "NOTDEFINED", NULL};
    static TextValue IfcChillerTypeEnum_[] = {"AIRCOOLED", "WATERCOOLED", "HEATRECOVERY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcChimneyTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCoilTypeEnum_[] = {"DXCOOLINGCOIL", "ELECTRICHEATINGCOIL", "GASHEATINGCOIL", "HYDRONICCOIL", "STEAMHEATINGCOIL", "WATERCOOLINGCOIL", "WATERHEATINGCOIL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcColumnTypeEnum_[] = {"COLUMN", "PILASTER", "PIERSTEM", "PIERSTEM_SEGMENT", "STANDCOLUMN", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCommunicationsApplianceTypeEnum_[] = {"ANTENNA", "COMPUTER", "FAX", "GATEWAY", "MODEM", "NETWORKAPPLIANCE", "NETWORKBRIDGE", "NETWORKHUB", "PRINTER", "REPEATER", "ROUTER", "SCANNER", "AUTOMATON", "INTELLIGENTPERIPHERAL", "IPNETWORKEQUIPMENT", "OPTICALNETWORKUNIT", "TELECOMMAND", "TELEPHONYEXCHANGE", "TRANSITIONCOMPONENT", "TRANSPONDER", "TRANSPORTEQUIPMENT", "OPTICALLINETERMINAL", "LINESIDEELECTRONICUNIT", "RADIOBLOCKCENTER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcComplexPropertyTemplateTypeEnum_[] = {"P_COMPLEX", "Q_COMPLEX", NULL};
    static TextValue IfcCompressorTypeEnum_[] = {"DYNAMIC", "RECIPROCATING", "ROTARY", "SCROLL", "TROCHOIDAL", "SINGLESTAGE", "BOOSTER", "OPENTYPE", "HERMETIC", "SEMIHERMETIC", "WELDEDSHELLHERMETIC", "ROLLINGPISTON", "ROTARYVANE", "SINGLESCREW", "TWINSCREW", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCondenserTypeEnum_[] = {"AIRCOOLED", "EVAPORATIVECOOLED", "WATERCOOLED", "WATERCOOLEDBRAZEDPLATE", "WATERCOOLEDSHELLCOIL", "WATERCOOLEDSHELLTUBE", "WATERCOOLEDTUBEINTUBE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcConnectionTypeEnum_[] = {"ATPATH", "ATSTART", "ATEND", "NOTDEFINED", NULL};
    static TextValue IfcConstraintEnum_[] = {"HARD", "SOFT", "ADVISORY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcConstructionEquipmentResourceTypeEnum_[] = {"DEMOLISHING", "EARTHMOVING", "ERECTING", "HEATING", "LIGHTING", "PAVING", "PUMPING", "TRANSPORTING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcConstructionMaterialResourceTypeEnum_[] = {"AGGREGATES", "CONCRETE", "DRYWALL", "FUEL", "GYPSUM", "MASONRY", "METAL", "PLASTIC", "WOOD", "NOTDEFINED", "USERDEFINED", NULL};
    static TextValue IfcConstructionProductResourceTypeEnum_[] = {"ASSEMBLY", "FORMWORK", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcControllerTypeEnum_[] = {"FLOATING", "PROGRAMMABLE", "PROPORTIONAL", "MULTIPOSITION", "TWOPOSITION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcConveyorSegmentTypeEnum_[] = {"CHUTECONVEYOR", "BELTCONVEYOR", "SCREWCONVEYOR", "BUCKETCONVEYOR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCooledBeamTypeEnum_[] = {"ACTIVE", "PASSIVE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCoolingTowerTypeEnum_[] = {"NATURALDRAFT", "MECHANICALINDUCEDDRAFT", "MECHANICALFORCEDDRAFT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCostItemTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCostScheduleTypeEnum_[] = {"BUDGET", "COSTPLAN", "ESTIMATE", "TENDER", "PRICEDBILLOFQUANTITIES", "UNPRICEDBILLOFQUANTITIES", "SCHEDULEOFRATES", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCourseTypeEnum_[] = {"ARMOUR", "FILTER", "BALLASTBED", "CORE", "PAVEMENT", "PROTECTION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCoveringTypeEnum_[] = {"CEILING", "FLOORING", "CLADDING", "ROOFING", "MOLDING", "SKIRTINGBOARD", "INSULATION", "MEMBRANE", "SLEEVING", "TOPPING", "WRAPPING", "COPING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCrewResourceTypeEnum_[] = {"OFFICE", "SITE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCurtainWallTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcCurveInterpolationEnum_[] = {"LINEAR", "LOG_LINEAR", "LOG_LOG", "NOTDEFINED", NULL};
    static TextValue IfcDamperTypeEnum_[] = {"BACKDRAFTDAMPER", "BALANCINGDAMPER", "BLASTDAMPER", "CONTROLDAMPER", "FIREDAMPER", "FIRESMOKEDAMPER", "FUMEHOODEXHAUST", "GRAVITYDAMPER", "GRAVITYRELIEFDAMPER", "RELIEFDAMPER", "SMOKEDAMPER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDataOriginEnum_[] = {"MEASURED", "PREDICTED", "SIMULATED", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDerivedUnitEnum_[] = {"ANGULARVELOCITYUNIT", "AREADENSITYUNIT", "COMPOUNDPLANEANGLEUNIT", "DYNAMICVISCOSITYUNIT", "HEATFLUXDENSITYUNIT", "INTEGERCOUNTRATEUNIT", "ISOTHERMALMOISTURECAPACITYUNIT", "KINEMATICVISCOSITYUNIT", "LINEARVELOCITYUNIT", "MASSDENSITYUNIT", "MASSFLOWRATEUNIT", "MOISTUREDIFFUSIVITYUNIT", "MOLECULARWEIGHTUNIT", "SPECIFICHEATCAPACITYUNIT", "THERMALADMITTANCEUNIT", "THERMALCONDUCTANCEUNIT", "THERMALRESISTANCEUNIT", "THERMALTRANSMITTANCEUNIT", "VAPORPERMEABILITYUNIT", "VOLUMETRICFLOWRATEUNIT", "ROTATIONALFREQUENCYUNIT", "TORQUEUNIT", "MOMENTOFINERTIAUNIT", "LINEARMOMENTUNIT", "LINEARFORCEUNIT", "PLANARFORCEUNIT", "MODULUSOFELASTICITYUNIT", "SHEARMODULUSUNIT", "LINEARSTIFFNESSUNIT", "ROTATIONALSTIFFNESSUNIT", "MODULUSOFSUBGRADEREACTIONUNIT", "ACCELERATIONUNIT", "CURVATUREUNIT", "HEATINGVALUEUNIT", "IONCONCENTRATIONUNIT", "LUMINOUSINTENSITYDISTRIBUTIONUNIT", "MASSPERLENGTHUNIT", "MODULUSOFLINEARSUBGRADEREACTIONUNIT", "MODULUSOFROTATIONALSUBGRADEREACTIONUNIT", "PHUNIT", "ROTATIONALMASSUNIT", "SECTIONAREAINTEGRALUNIT", "SECTIONMODULUSUNIT", "SOUNDPOWERLEVELUNIT", "SOUNDPOWERUNIT", "SOUNDPRESSURELEVELUNIT", "SOUNDPRESSUREUNIT", "TEMPERATUREGRADIENTUNIT", "TEMPERATURERATEOFCHANGEUNIT", "THERMALEXPANSIONCOEFFICIENTUNIT", "WARPINGCONSTANTUNIT", "WARPINGMOMENTUNIT", "USERDEFINED", NULL};
    static TextValue IfcDirectionSenseEnum_[] = {"POSITIVE", "NEGATIVE", NULL};
    static TextValue IfcDiscreteAccessoryTypeEnum_[] = {"ANCHORPLATE", "BRACKET", "SHOE", "EXPANSION_JOINT_DEVICE", "CABLEARRANGER", "FILLER", "FLASHING", "INSULATOR", "LOCK", "TENSIONINGEQUIPMENT", "RAILPAD", "SLIDINGCHAIR", "RAIL_LUBRICATION", "PANEL_STRENGTHENING", "RAILBRACE", "ELASTIC_CUSHION", "SOUNDABSORPTION", "POINTMACHINEMOUNTINGDEVICE", "POINT_MACHINE_LOCKING_DEVICE", "RAIL_MECHANICAL_EQUIPMENT", "BIRDPROTECTION", "WATER_BARRIER", "STRUCTURAL_SEALING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDistributionBoardTypeEnum_[] = {"SWITCHBOARD", "CONSUMERUNIT", "MOTORCONTROLCENTRE", "DISTRIBUTIONFRAME", "DISTRIBUTIONBOARD", "DISPATCHINGBOARD", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDistributionChamberElementTypeEnum_[] = {"FORMEDDUCT", "INSPECTIONCHAMBER", "INSPECTIONPIT", "MANHOLE", "METERCHAMBER", "SUMP", "TRENCH", "VALVECHAMBER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDistributionPortTypeEnum_[] = {"CABLE", "CABLECARRIER", "DUCT", "PIPE", "WIRELESS", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDistributionSystemEnum_[] = {"AIRCONDITIONING", "AUDIOVISUAL", "CHEMICAL", "CHILLEDWATER", "COMMUNICATION", "COMPRESSEDAIR", "CONDENSERWATER", "CONTROL", "CONVEYING", "DATA", "DISPOSAL", "DOMESTICCOLDWATER", "DOMESTICHOTWATER", "DRAINAGE", "EARTHING", "ELECTRICAL", "ELECTROACOUSTIC", "EXHAUST", "FIREPROTECTION", "FUEL", "GAS", "HAZARDOUS", "HEATING", "LIGHTING", "LIGHTNINGPROTECTION", "MUNICIPALSOLIDWASTE", "OIL", "OPERATIONAL", "POWERGENERATION", "RAINWATER", "REFRIGERATION", "SECURITY", "SEWAGE", "SIGNAL", "STORMWATER", "TELEPHONE", "TV", "VACUUM", "VENT", "VENTILATION", "WASTEWATER", "WATERSUPPLY", "CATENARY_SYSTEM", "OVERHEAD_CONTACTLINE_SYSTEM", "RETURN_CIRCUIT", "FIXEDTRANSMISSIONNETWORK", "OPERATIONALTELEPHONYSYSTEM", "MOBILENETWORK", "MONITORINGSYSTEM", "SAFETY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDocumentConfidentialityEnum_[] = {"PUBLIC", "RESTRICTED", "CONFIDENTIAL", "PERSONAL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDocumentStatusEnum_[] = {"DRAFT", "FINALDRAFT", "FINAL", "REVISION", "NOTDEFINED", NULL};
    static TextValue IfcDoorPanelOperationEnum_[] = {"SWINGING", "DOUBLE_ACTING", "SLIDING", "FOLDING", "REVOLVING", "ROLLINGUP", "FIXEDPANEL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDoorPanelPositionEnum_[] = {"LEFT", "MIDDLE", "RIGHT", "NOTDEFINED", NULL};
    static TextValue IfcDoorStyleConstructionEnum_[] = {"ALUMINIUM", "HIGH_GRADE_STEEL", "STEEL", "WOOD", "ALUMINIUM_WOOD", "ALUMINIUM_PLASTIC", "PLASTIC", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDoorStyleOperationEnum_[] = {"SINGLE_SWING_LEFT", "SINGLE_SWING_RIGHT", "DOUBLE_DOOR_SINGLE_SWING", "DOUBLE_DOOR_SINGLE_SWING_OPPOSITE_LEFT", "DOUBLE_DOOR_SINGLE_SWING_OPPOSITE_RIGHT", "DOUBLE_SWING_LEFT", "DOUBLE_SWING_RIGHT", "DOUBLE_DOOR_DOUBLE_SWING", "SLIDING_TO_LEFT", "SLIDING_TO_RIGHT", "DOUBLE_DOOR_SLIDING", "FOLDING_TO_LEFT", "FOLDING_TO_RIGHT", "DOUBLE_DOOR_FOLDING", "REVOLVING", "ROLLINGUP", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDoorTypeEnum_[] = {"DOOR", "GATE", "TRAPDOOR", "BOOM_BARRIER", "TURNSTILE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDoorTypeOperationEnum_[] = {"SINGLE_SWING_LEFT", "SINGLE_SWING_RIGHT", "DOUBLE_PANEL_SINGLE_SWING", "DOUBLE_PANEL_SINGLE_SWING_OPPOSITE_LEFT", "DOUBLE_PANEL_SINGLE_SWING_OPPOSITE_RIGHT", "DOUBLE_SWING_LEFT", "DOUBLE_SWING_RIGHT", "DOUBLE_PANEL_DOUBLE_SWING", "SLIDING_TO_LEFT", "SLIDING_TO_RIGHT", "DOUBLE_PANEL_SLIDING", "FOLDING_TO_LEFT", "FOLDING_TO_RIGHT", "DOUBLE_PANEL_FOLDING", "REVOLVING_HORIZONTAL", "ROLLINGUP", "SWING_FIXED_LEFT", "SWING_FIXED_RIGHT", "DOUBLE_PANEL_LIFTING_VERTICAL", "LIFTING_HORIZONTAL", "LIFTING_VERTICAL_LEFT", "LIFTING_VERTICAL_RIGHT", "REVOLVING_VERTICAL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDuctFittingTypeEnum_[] = {"BEND", "CONNECTOR", "ENTRY", "EXIT", "JUNCTION", "OBSTRUCTION", "TRANSITION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDuctSegmentTypeEnum_[] = {"RIGIDSEGMENT", "FLEXIBLESEGMENT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcDuctSilencerTypeEnum_[] = {"FLATOVAL", "RECTANGULAR", "ROUND", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcEarthingElementTypeEnum_[] = {"EARTHINGSTRIP", "GROUNDINGPLATE", "GROUNDINGROD", "FIXEDTERMINAL", "GROUNDINGMESH", "NOTDEFINED", "USERDEFINED", NULL};
    static TextValue IfcEarthworksCutTypeEnum_[] = {"TRENCH", "DREDGING", "EXCAVATION", "OVEREXCAVATION", "TOPSOILREMOVAL", "STEPEXCAVATION", "PAVEMENTMILLING", "CUT", "BASE_EXCAVATION", "CONFINEDOPENEXCAVATION", "ANCHOREDOPENEXCAVATION", "BRACEDOPENEXCAVATION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcEarthworksFillTypeEnum_[] = {"BACKFILL", "COUNTERWEIGHT", "SUBGRADE", "EMBANKMENT", "TRANSITIONSECTION", "SUBGRADEBED", "SLOPEFILL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElectricApplianceTypeEnum_[] = {"DISHWASHER", "ELECTRICCOOKER", "FREESTANDINGELECTRICHEATER", "FREESTANDINGFAN", "FREESTANDINGWATERHEATER", "FREESTANDINGWATERCOOLER", "FREEZER", "FRIDGE_FREEZER", "HANDDRYER", "KITCHENMACHINE", "MICROWAVE", "PHOTOCOPIER", "REFRIGERATOR", "TUMBLEDRYER", "VENDINGMACHINE", "WASHINGMACHINE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElectricDistributionBoardTypeEnum_[] = {"CONSUMERUNIT", "DISTRIBUTIONBOARD", "MOTORCONTROLCENTRE", "SWITCHBOARD", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElectricFlowStorageDeviceTypeEnum_[] = {"BATTERY", "CAPACITORBANK", "HARMONICFILTER", "INDUCTORBANK", "UPS", "CAPACITOR", "COMPENSATOR", "INDUCTOR", "RECHARGER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElectricFlowTreatmentDeviceTypeEnum_[] = {"ELECTRONICFILTER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElectricGeneratorTypeEnum_[] = {"CHP", "ENGINEGENERATOR", "STANDALONE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElectricMotorTypeEnum_[] = {"DC", "INDUCTION", "POLYPHASE", "RELUCTANCESYNCHRONOUS", "SYNCHRONOUS", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElectricTimeControlTypeEnum_[] = {"TIMECLOCK", "TIMEDELAY", "RELAY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElementAssemblyTypeEnum_[] = {"ACCESSORY_ASSEMBLY", "ARCH", "BEAM_GRID", "BRACED_FRAME", "GIRDER", "REINFORCEMENT_UNIT", "RIGID_FRAME", "SLAB_FIELD", "TRUSS", "ABUTMENT", "PIER", "PYLON", "CROSS_BRACING", "DECK", "MAST", "SIGNALASSEMBLY", "GRID", "SHELTER", "SUPPORTINGASSEMBLY", "SUSPENSIONASSEMBLY", "TRACTION_SWITCHING_ASSEMBLY", "TRACKPANEL", "TURNOUTPANEL", "DILATATIONPANEL", "RAIL_MECHANICAL_EQUIPMENT_ASSEMBLY", "ENTRANCEWORKS", "SUMPBUSTER", "TRAFFIC_CALMING_DEVICE", "DUCTBANK", "UMBRELLAVAULT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcElementCompositionEnum_[] = {"COMPLEX", "ELEMENT", "PARTIAL", NULL};
    static TextValue IfcEngineTypeEnum_[] = {"EXTERNALCOMBUSTION", "INTERNALCOMBUSTION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcEvaporativeCoolerTypeEnum_[] = {"DIRECTEVAPORATIVERANDOMMEDIAAIRCOOLER", "DIRECTEVAPORATIVERIGIDMEDIAAIRCOOLER", "DIRECTEVAPORATIVESLINGERSPACKAGEDAIRCOOLER", "DIRECTEVAPORATIVEPACKAGEDROTARYAIRCOOLER", "DIRECTEVAPORATIVEAIRWASHER", "INDIRECTEVAPORATIVEPACKAGEAIRCOOLER", "INDIRECTEVAPORATIVEWETCOIL", "INDIRECTEVAPORATIVECOOLINGTOWERORCOILCOOLER", "INDIRECTDIRECTCOMBINATION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcEvaporatorTypeEnum_[] = {"DIRECTEXPANSION", "DIRECTEXPANSIONSHELLANDTUBE", "DIRECTEXPANSIONTUBEINTUBE", "DIRECTEXPANSIONBRAZEDPLATE", "FLOODEDSHELLANDTUBE", "SHELLANDCOIL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcEventTriggerTypeEnum_[] = {"EVENTRULE", "EVENTMESSAGE", "EVENTTIME", "EVENTCOMPLEX", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcEventTypeEnum_[] = {"STARTEVENT", "ENDEVENT", "INTERMEDIATEEVENT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcExternalSpatialElementTypeEnum_[] = {"EXTERNAL", "EXTERNAL_EARTH", "EXTERNAL_WATER", "EXTERNAL_FIRE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFacilityPartCommonTypeEnum_[] = {"SEGMENT", "ABOVEGROUND", "JUNCTION", "LEVELCROSSING", "BELOWGROUND", "SUBSTRUCTURE", "TERMINAL", "SUPERSTRUCTURE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFacilityUsageEnum_[] = {"LATERAL", "REGION", "VERTICAL", "LONGITUDINAL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFanTypeEnum_[] = {"CENTRIFUGALFORWARDCURVED", "CENTRIFUGALRADIAL", "CENTRIFUGALBACKWARDINCLINEDCURVED", "CENTRIFUGALAIRFOIL", "TUBEAXIAL", "VANEAXIAL", "PROPELLORAXIAL", "USERDEFINED", "NOTDEFINED", "JET", NULL};
    static TextValue IfcFastenerTypeEnum_[] = {"GLUE", "MORTAR", "WELD", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFillElementTypeEnum_[] = {"INVERTFILL", "ANNULARGAPFILL", "NOTDEFINED", "USERDEFINED", NULL};
    static TextValue IfcFilterTypeEnum_[] = {"AIRPARTICLEFILTER", "COMPRESSEDAIRFILTER", "ODORFILTER", "OILFILTER", "STRAINER", "WATERFILTER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFireSuppressionTerminalTypeEnum_[] = {"BREECHINGINLET", "FIREHYDRANT", "HOSEREEL", "SPRINKLER", "SPRINKLERDEFLECTOR", "FIREMONITOR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFlowDirectionEnum_[] = {"SOURCE", "SINK", "SOURCEANDSINK", "NOTDEFINED", NULL};
    static TextValue IfcFlowInstrumentTypeEnum_[] = {"PRESSUREGAUGE", "THERMOMETER", "AMMETER", "FREQUENCYMETER", "POWERFACTORMETER", "PHASEANGLEMETER", "VOLTMETER_PEAK", "VOLTMETER_RMS", "COMBINED", "VOLTMETER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFlowMeterTypeEnum_[] = {"ENERGYMETER", "GASMETER", "OILMETER", "WATERMETER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFootingTypeEnum_[] = {"CAISSON_FOUNDATION", "FOOTING_BEAM", "PAD_FOOTING", "PILE_CAP", "STRIP_FOOTING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcFurnitureTypeEnum_[] = {"CHAIR", "TABLE", "DESK", "BED", "FILECABINET", "SHELF", "SOFA", "TECHNICALCABINET", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcGeographicElementTypeEnum_[] = {"TERRAIN", "SOIL_BORING_POINT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcGeometricProjectionEnum_[] = {"GRAPH_VIEW", "SKETCH_VIEW", "MODEL_VIEW", "PLAN_VIEW", "REFLECTED_PLAN_VIEW", "SECTION_VIEW", "ELEVATION_VIEW", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcGeoScienceFeatureTypeEnum_[] = {"DISCRETEDISCONTINUITY", "FOLD", "FLUIDBODY", "PIEZOMETRICWATERLEVEL", "VOIDBODY", "GEOLOGICUNIT", "GEOTECHNICALUNIT", "HAZARDAREA", "HYDROGEOUNIT", "FAULT", "CONTACT", "PHYSICALPROPERTYDISTRIBUTION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcGeoScienceModelTypeEnum_[] = {"GEOTECHMODEL", "HYDROGEOMODEL", "GEOLOGYMODEL", "GEOTECHSYNTHESISMODEL", "PHYSICALPROPERTYDISTIBUTIONMODEL", "GEOHAZARDMODEL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcGeoScienceObservationTypeEnum_[] = {"INSITUTESTRESULT", "LABTESTRESULT", "BOREHOLELOG", "MAPPEDFEATURE", "LOCALINFORMATION", "GEOPHYSICALSURVEYRESULT", "NOTDEFINED", "USERDEFINED", NULL};
    static TextValue IfcGeotechTypicalSectionTypeEnum_[] = {"NOTDEFINED", "USERDEFINED", NULL};
    static TextValue IfcGlobalOrLocalEnum_[] = {"GLOBAL_COORDS", "LOCAL_COORDS", NULL};
    static TextValue IfcGridTypeEnum_[] = {"RECTANGULAR", "RADIAL", "TRIANGULAR", "IRREGULAR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcGroundReinforcementElementTypeEnum_[] = {"SPILINGBOLT", "ROCKSUPPORTBOLT", "NOTDEFINED", "USERDEFINED", NULL};
    static TextValue IfcHeatExchangerTypeEnum_[] = {"PLATE", "SHELLANDTUBE", "TURNOUTHEATING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcHumidifierTypeEnum_[] = {"STEAMINJECTION", "ADIABATICAIRWASHER", "ADIABATICPAN", "ADIABATICWETTEDELEMENT", "ADIABATICATOMIZING", "ADIABATICULTRASONIC", "ADIABATICRIGIDMEDIA", "ADIABATICCOMPRESSEDAIRNOZZLE", "ASSISTEDELECTRIC", "ASSISTEDNATURALGAS", "ASSISTEDPROPANE", "ASSISTEDBUTANE", "ASSISTEDSTEAM", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcImpactProtectionDeviceTypeEnum_[] = {"CRASHCUSHION", "DAMPINGSYSTEM", "FENDER", "BUMPER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcImprovedGroundTypeEnum_[] = {"SURCHARGEPRELOADED", "VERTICALLYDRAINED", "DYNAMICALLYCOMPACTED", "REPLACED", "ROLLERCOMPACTED", "GROUTED", "DEEPMIXED", "LATERALLYDRAINED", "FROZEN", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcInterceptorTypeEnum_[] = {"CYCLONIC", "GREASE", "OIL", "PETROL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcInternalOrExternalEnum_[] = {"INTERNAL", "EXTERNAL", "EXTERNAL_EARTH", "EXTERNAL_WATER", "EXTERNAL_FIRE", "NOTDEFINED", NULL};
    static TextValue IfcInventoryTypeEnum_[] = {"ASSETINVENTORY", "SPACEINVENTORY", "FURNITUREINVENTORY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcJunctionBoxTypeEnum_[] = {"DATA", "POWER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcKerbTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcKnotType_[] = {"UNIFORM_KNOTS", "QUASI_UNIFORM_KNOTS", "PIECEWISE_BEZIER_KNOTS", "UNSPECIFIED", NULL};
    static TextValue IfcLaborResourceTypeEnum_[] = {"ADMINISTRATION", "CARPENTRY", "CLEANING", "CONCRETE", "DRYWALL", "ELECTRIC", "FINISHING", "FLOORING", "GENERAL", "HVAC", "LANDSCAPING", "MASONRY", "PAINTING", "PAVING", "PLUMBING", "ROOFING", "SITEGRADING", "STEELWORK", "SURVEYING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcLampTypeEnum_[] = {"COMPACTFLUORESCENT", "FLUORESCENT", "HALOGEN", "HIGHPRESSUREMERCURY", "HIGHPRESSURESODIUM", "LED", "METALHALIDE", "OLED", "TUNGSTENFILAMENT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcLayerSetDirectionEnum_[] = {"AXIS1", "AXIS2", "AXIS3", NULL};
    static TextValue IfcLightDistributionCurveEnum_[] = {"TYPE_A", "TYPE_B", "TYPE_C", "NOTDEFINED", NULL};
    static TextValue IfcLightEmissionSourceEnum_[] = {"COMPACTFLUORESCENT", "FLUORESCENT", "HIGHPRESSUREMERCURY", "HIGHPRESSURESODIUM", "LIGHTEMITTINGDIODE", "LOWPRESSURESODIUM", "LOWVOLTAGEHALOGEN", "MAINVOLTAGEHALOGEN", "METALHALIDE", "TUNGSTENFILAMENT", "NOTDEFINED", NULL};
    static TextValue IfcLightFixtureTypeEnum_[] = {"POINTSOURCE", "DIRECTIONSOURCE", "SECURITYLIGHTING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcLiquidTerminalTypeEnum_[] = {"LOADINGARM", "HOSEREEL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcLoadGroupTypeEnum_[] = {"LOAD_GROUP", "LOAD_CASE", "LOAD_COMBINATION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcLogicalOperatorEnum_[] = {"LOGICALAND", "LOGICALOR", "LOGICALXOR", "LOGICALNOTAND", "LOGICALNOTOR", NULL};
    static TextValue IfcMarineFacilityTypeEnum_[] = {"CANAL", "WATERWAYSHIPLIFT", "REVETMENT", "LAUNCHRECOVERY", "MARINEDEFENCE", "HYDROLIFT", "SHIPYARD", "SHIPLIFT", "PORT", "QUAY", "FLOATINGDOCK", "NAVIGATIONALCHANNEL", "BREAKWATER", "DRYDOCK", "JETTY", "SHIPLOCK", "BARRIERBEACH", "SLIPWAY", "WATERWAY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcMarinePartTypeEnum_[] = {"CREST", "MANUFACTURING", "LOWWATERLINE", "CORE", "WATERFIELD", "CILL_LEVEL", "BERTHINGSTRUCTURE", "COPELEVEL", "CHAMBER", "STORAGEAREA", "APPROACHCHANNEL", "VEHICLESERVICING", "SHIPTRANSFER", "GATEHEAD", "GUDINGSTRUCTURE", "BELOWWATERLINE", "WEATHERSIDE", "LANDFIELD", "PROTECTION", "LEEWARDSIDE", "ABOVEWATERLINE", "ANCHORAGE", "NAVIGATIONALAREA", "HIGHWATERLINE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcMechanicalFastenerTypeEnum_[] = {"ANCHORBOLT", "BOLT", "DOWEL", "NAIL", "NAILPLATE", "RIVET", "SCREW", "SHEARCONNECTOR", "STAPLE", "STUDSHEARCONNECTOR", "COUPLER", "RAILJOINT", "RAILFASTENING", "CHAIN", "ROPE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcMedicalDeviceTypeEnum_[] = {"AIRSTATION", "FEEDAIRUNIT", "OXYGENGENERATOR", "OXYGENPLANT", "VACUUMSTATION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcMemberTypeEnum_[] = {"BRACE", "CHORD", "COLLAR", "MEMBER", "MULLION", "PLATE", "POST", "PURLIN", "RAFTER", "STRINGER", "STRUT", "STUD", "STIFFENING_RIB", "ARCH_SEGMENT", "SUSPENSION_CABLE", "SUSPENDER", "STAY_CABLE", "STRUCTURALCABLE", "TIEBAR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcMobileTelecommunicationsApplianceTypeEnum_[] = {"E_UTRAN_NODE_B", "REMOTERADIOUNIT", "ACCESSPOINT", "BASETRANSCEIVERSTATION", "REMOTEUNIT", "BASEBANDUNIT", "MASTERUNIT", "GATEWAY_GPRS_SUPPORT_NODE", "SUBSCRIBERSERVER", "MOBILESWITCHINGCENTER", "MSCSERVER", "PACKETCONTROLUNIT", "SERVICE_GPRS_SUPPORT_NODE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcMooringDeviceTypeEnum_[] = {"LINETENSIONER", "MAGNETICDEVICE", "MOORINGHOOKS", "VACUUMDEVICE", "BOLLARD", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcMotorConnectionTypeEnum_[] = {"BELTDRIVE", "COUPLING", "DIRECTDRIVE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcNavigationElementTypeEnum_[] = {"BEACON", "BUOY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcObjectiveEnum_[] = {"CODECOMPLIANCE", "CODEWAIVER", "DESIGNINTENT", "EXTERNAL", "HEALTHANDSAFETY", "MERGECONFLICT", "MODELVIEW", "PARAMETER", "REQUIREMENT", "SPECIFICATION", "TRIGGERCONDITION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcObjectTypeEnum_[] = {"PRODUCT", "PROCESS", "CONTROL", "RESOURCE", "ACTOR", "GROUP", "PROJECT", "NOTDEFINED", NULL};
    static TextValue IfcOccupantTypeEnum_[] = {"ASSIGNEE", "ASSIGNOR", "LESSEE", "LESSOR", "LETTINGAGENT", "OWNER", "TENANT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcOpeningElementTypeEnum_[] = {"OPENING", "RECESS", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcOutletTypeEnum_[] = {"AUDIOVISUALOUTLET", "COMMUNICATIONSOUTLET", "POWEROUTLET", "DATAOUTLET", "TELEPHONEOUTLET", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPavementTypeEnum_[] = {"FLEXIBLE", "RIGID", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPerformanceHistoryTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPermeableCoveringOperationEnum_[] = {"GRILL", "LOUVER", "SCREEN", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPermitTypeEnum_[] = {"ACCESS", "BUILDING", "WORK", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPhysicalOrVirtualEnum_[] = {"PHYSICAL", "VIRTUAL", "NOTDEFINED", NULL};
    static TextValue IfcPileConstructionEnum_[] = {"CAST_IN_PLACE", "COMPOSITE", "PRECAST_CONCRETE", "PREFAB_STEEL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPileTypeEnum_[] = {"BORED", "DRIVEN", "JETGROUTING", "COHESION", "FRICTION", "SUPPORT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPipeFittingTypeEnum_[] = {"BEND", "CONNECTOR", "ENTRY", "EXIT", "JUNCTION", "OBSTRUCTION", "TRANSITION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPipeSegmentTypeEnum_[] = {"CULVERT", "FLEXIBLESEGMENT", "RIGIDSEGMENT", "GUTTER", "SPOOL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPlateTypeEnum_[] = {"CURTAIN_PANEL", "SHEET", "FLANGE_PLATE", "WEB_PLATE", "STIFFENER_PLATE", "GUSSET_PLATE", "COVER_PLATE", "SPLICE_PLATE", "BASE_PLATE", "LAGGING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPreferredSurfaceCurveRepresentation_[] = {"CURVE3D", "PCURVE_S1", "PCURVE_S2", NULL};
    static TextValue IfcProcedureTypeEnum_[] = {"ADVICE_CAUTION", "ADVICE_NOTE", "ADVICE_WARNING", "CALIBRATION", "DIAGNOSTIC", "SHUTDOWN", "STARTUP", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcProfileTypeEnum_[] = {"CURVE", "AREA", NULL};
    static TextValue IfcProjectedOrTrueLengthEnum_[] = {"PROJECTED_LENGTH", "TRUE_LENGTH", NULL};
    static TextValue IfcProjectionElementTypeEnum_[] = {"BLISTER", "DEVIATOR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcProjectOrderTypeEnum_[] = {"CHANGEORDER", "MAINTENANCEWORKORDER", "MOVEORDER", "PURCHASEORDER", "WORKORDER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPropertySetTemplateTypeEnum_[] = {"PSET_MATERIALDRIVEN", "PSET_TYPEDRIVENONLY", "PSET_TYPEDRIVENOVERRIDE", "PSET_OCCURRENCEDRIVEN", "PSET_PERFORMANCEDRIVEN", "PSET_PROFILEDRIVEN", "QTO_TYPEDRIVENONLY", "QTO_TYPEDRIVENOVERRIDE", "QTO_OCCURRENCEDRIVEN", "NOTDEFINED", NULL};
    static TextValue IfcProtectiveDeviceTrippingUnitTypeEnum_[] = {"ELECTRONIC", "ELECTROMAGNETIC", "RESIDUALCURRENT", "THERMAL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcProtectiveDeviceTypeEnum_[] = {"CIRCUITBREAKER", "EARTHLEAKAGECIRCUITBREAKER", "EARTHINGSWITCH", "FUSEDISCONNECTOR", "RESIDUALCURRENTCIRCUITBREAKER", "RESIDUALCURRENTSWITCH", "VARISTOR", "ANTI_ARCING_DEVICE", "SPARKGAP", "VOLTAGELIMITER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcPumpTypeEnum_[] = {"CIRCULATOR", "ENDSUCTION", "SPLITCASE", "SUBMERSIBLEPUMP", "SUMPPUMP", "VERTICALINLINE", "VERTICALTURBINE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRailingTypeEnum_[] = {"HANDRAIL", "GUARDRAIL", "BALUSTRADE", "FENCE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRailTypeEnum_[] = {"RACKRAIL", "BLADE", "GUARDRAIL", "STOCKRAIL", "CHECKRAIL", "RAIL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRailwayPartTypeEnum_[] = {"TRACKSTRUCTURE", "TRACKSTRUCTUREPART", "LINESIDESTRUCTUREPART", "DILATATIONSUPERSTRUCTURE", "PLAINTRACKSUPERSTRUCTURE", "LINESIDESTRUCTURE", "SUPERSTRUCTURE", "TURNOUTSUPERSTRUCTURE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRailwayTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRampFlightTypeEnum_[] = {"STRAIGHT", "SPIRAL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRampTypeEnum_[] = {"STRAIGHT_RUN_RAMP", "TWO_STRAIGHT_RUN_RAMP", "QUARTER_TURN_RAMP", "TWO_QUARTER_TURN_RAMP", "HALF_TURN_RAMP", "SPIRAL_RAMP", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRecurrenceTypeEnum_[] = {"DAILY", "WEEKLY", "MONTHLY_BY_DAY_OF_MONTH", "MONTHLY_BY_POSITION", "BY_DAY_COUNT", "BY_WEEKDAY_COUNT", "YEARLY_BY_DAY_OF_MONTH", "YEARLY_BY_POSITION", NULL};
    static TextValue IfcReferentTypeEnum_[] = {"STATION", "REFERENCEMARKER", "LANDMARK", "BOUNDARY", "INTERSECTION", "POSITION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcReflectanceMethodEnum_[] = {"BLINN", "FLAT", "GLASS", "MATT", "METAL", "MIRROR", "PHONG", "PHYSICAL", "PLASTIC", "STRAUSS", "NOTDEFINED", NULL};
    static TextValue IfcReinforcingBarRoleEnum_[] = {"MAIN", "SHEAR", "LIGATURE", "STUD", "PUNCHING", "EDGE", "RING", "ANCHORING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcReinforcingBarSurfaceEnum_[] = {"PLAIN", "TEXTURED", NULL};
    static TextValue IfcReinforcingBarTypeEnum_[] = {"ANCHORING", "EDGE", "LIGATURE", "MAIN", "PUNCHING", "RING", "SHEAR", "STUD", "SPACEBAR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcReinforcingMeshTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRoadPartTypeEnum_[] = {"ROADSIDEPART", "BUS_STOP", "HARDSHOULDER", "INTERSECTION", "PASSINGBAY", "ROADWAYPLATEAU", "ROADSIDE", "REFUGEISLAND", "TOLLPLAZA", "CENTRALRESERVE", "SIDEWALK", "PARKINGBAY", "RAILWAYCROSSING", "PEDESTRIAN_CROSSING", "SOFTSHOULDER", "BICYCLECROSSING", "CENTRALISLAND", "SHOULDER", "TRAFFICLANE", "ROADSEGMENT", "ROUNDABOUT", "LAYBY", "CARRIAGEWAY", "TRAFFICISLAND", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRoadTypeEnum_[] = {"USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcRoleEnum_[] = {"SUPPLIER", "MANUFACTURER", "CONTRACTOR", "SUBCONTRACTOR", "ARCHITECT", "STRUCTURALENGINEER", "COSTENGINEER", "CLIENT", "BUILDINGOWNER", "BUILDINGOPERATOR", "MECHANICALENGINEER", "ELECTRICALENGINEER", "PROJECTMANAGER", "FACILITIESMANAGER", "CIVILENGINEER", "COMMISSIONINGENGINEER", "ENGINEER", "OWNER", "CONSULTANT", "CONSTRUCTIONMANAGER", "FIELDCONSTRUCTIONMANAGER", "RESELLER", "USERDEFINED", NULL};
    static TextValue IfcRoofTypeEnum_[] = {"FLAT_ROOF", "SHED_ROOF", "GABLE_ROOF", "HIP_ROOF", "HIPPED_GABLE_ROOF", "GAMBREL_ROOF", "MANSARD_ROOF", "BARREL_ROOF", "RAINBOW_ROOF", "BUTTERFLY_ROOF", "PAVILION_ROOF", "DOME_ROOF", "FREEFORM", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSanitaryTerminalTypeEnum_[] = {"BATH", "BIDET", "CISTERN", "SHOWER", "SINK", "SANITARYFOUNTAIN", "TOILETPAN", "URINAL", "WASHHANDBASIN", "WCSEAT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSectionTypeEnum_[] = {"UNIFORM", "TAPERED", NULL};
    static TextValue IfcSensorTypeEnum_[] = {"COSENSOR", "CO2SENSOR", "CONDUCTANCESENSOR", "CONTACTSENSOR", "FIRESENSOR", "FLOWSENSOR", "FROSTSENSOR", "GASSENSOR", "HEATSENSOR", "HUMIDITYSENSOR", "IDENTIFIERSENSOR", "IONCONCENTRATIONSENSOR", "LEVELSENSOR", "LIGHTSENSOR", "MOISTURESENSOR", "MOVEMENTSENSOR", "PHSENSOR", "PRESSURESENSOR", "RADIATIONSENSOR", "RADIOACTIVITYSENSOR", "SMOKESENSOR", "SOUNDSENSOR", "TEMPERATURESENSOR", "WINDSENSOR", "EARTHQUAKESENSOR", "FOREIGNOBJECTDETECTIONSENSOR", "OBSTACLESENSOR", "RAINSENSOR", "SNOWDEPTHSENSOR", "TRAINSENSOR", "TURNOUTCLOSURESENSOR", "WHEELSENSOR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSequenceEnum_[] = {"START_START", "START_FINISH", "FINISH_START", "FINISH_FINISH", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcShadingDeviceTypeEnum_[] = {"JALOUSIE", "SHUTTER", "AWNING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSignalTypeEnum_[] = {"VISUAL", "AUDIO", "MIXED", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSignTypeEnum_[] = {"MARKER", "PICTORAL", "MIRROR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSimplePropertyTemplateTypeEnum_[] = {"P_SINGLEVALUE", "P_ENUMERATEDVALUE", "P_BOUNDEDVALUE", "P_LISTVALUE", "P_TABLEVALUE", "P_REFERENCEVALUE", "Q_LENGTH", "Q_AREA", "Q_VOLUME", "Q_COUNT", "Q_WEIGHT", "Q_TIME", NULL};
    static TextValue IfcSIPrefix_[] = {"EXA", "PETA", "TERA", "GIGA", "MEGA", "KILO", "HECTO", "DECA", "DECI", "CENTI", "MILLI", "MICRO", "NANO", "PICO", "FEMTO", "ATTO", NULL};
    static TextValue IfcSIUnitName_[] = {"AMPERE", "BECQUEREL", "CANDELA", "COULOMB", "CUBIC_METRE", "DEGREE_CELSIUS", "FARAD", "GRAM", "GRAY", "HENRY", "HERTZ", "JOULE", "KELVIN", "LUMEN", "LUX", "METRE", "MOLE", "NEWTON", "OHM", "PASCAL", "RADIAN", "SECOND", "SIEMENS", "SIEVERT", "SQUARE_METRE", "STERADIAN", "TESLA", "VOLT", "WATT", "WEBER", NULL};
    static TextValue IfcSlabTypeEnum_[] = {"FLOOR", "ROOF", "LANDING", "BASESLAB", "APPROACH_SLAB", "PAVING", "WEARING", "SIDEWALK", "TRACKSLAB", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSolarDeviceTypeEnum_[] = {"SOLARCOLLECTOR", "SOLARPANEL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSpaceHeaterTypeEnum_[] = {"CONVECTOR", "RADIATOR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSpaceTypeEnum_[] = {"SPACE", "PARKING", "GFA", "INTERNAL", "EXTERNAL", "BERTH", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSpatialZoneTypeEnum_[] = {"CONSTRUCTION", "FIRESAFETY", "LIGHTING", "OCCUPANCY", "SECURITY", "THERMAL", "TRANSPORT", "VENTILATION", "RESERVATION", "INTERFERENCE", "MAPPEDZONE", "TESTEDZONE", "COMPARTMENT", "ANNULARGAP", "CLEARANCE", "INSTALLATION", "INTERIOR", "INVERT", "LINING", "SERVICE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcStackTerminalTypeEnum_[] = {"BIRDCAGE", "COWL", "RAINWATERHOPPER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcStairFlightTypeEnum_[] = {"STRAIGHT", "WINDER", "SPIRAL", "CURVED", "FREEFORM", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcStairTypeEnum_[] = {"STRAIGHT_RUN_STAIR", "TWO_STRAIGHT_RUN_STAIR", "QUARTER_WINDING_STAIR", "QUARTER_TURN_STAIR", "HALF_WINDING_STAIR", "HALF_TURN_STAIR", "TWO_QUARTER_WINDING_STAIR", "TWO_QUARTER_TURN_STAIR", "THREE_QUARTER_WINDING_STAIR", "THREE_QUARTER_TURN_STAIR", "SPIRAL_STAIR", "DOUBLE_RETURN_STAIR", "CURVED_RUN_STAIR", "TWO_CURVED_RUN_STAIR", "LADDER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcStateEnum_[] = {"READWRITE", "READONLY", "LOCKED", "READWRITELOCKED", "READONLYLOCKED", NULL};
    static TextValue IfcStructuralCurveActivityTypeEnum_[] = {"CONST", "LINEAR", "POLYGONAL", "EQUIDISTANT", "SINUS", "PARABOLA", "DISCRETE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcStructuralCurveMemberTypeEnum_[] = {"RIGID_JOINED_MEMBER", "PIN_JOINED_MEMBER", "CABLE", "TENSION_MEMBER", "COMPRESSION_MEMBER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcStructuralSurfaceActivityTypeEnum_[] = {"CONST", "BILINEAR", "DISCRETE", "ISOCONTOUR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcStructuralSurfaceMemberTypeEnum_[] = {"BENDING_ELEMENT", "MEMBRANE_ELEMENT", "SHELL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSubContractResourceTypeEnum_[] = {"PURCHASE", "WORK", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSurfaceFeatureTypeEnum_[] = {"MARK", "TAG", "TREATMENT", "DEFECT", "HATCHMARKING", "LINEMARKING", "PAVEMENTSURFACEMARKING", "SYMBOLMARKING", "NONSKIDSURFACING", "RUMBLESTRIP", "TRANSVERSERUMBLESTRIP", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSurfaceSide_[] = {"POSITIVE", "NEGATIVE", "BOTH", NULL};
    static TextValue IfcSwitchingDeviceTypeEnum_[] = {"CONTACTOR", "DIMMERSWITCH", "EMERGENCYSTOP", "KEYPAD", "MOMENTARYSWITCH", "SELECTORSWITCH", "STARTER", "SWITCHDISCONNECTOR", "TOGGLESWITCH", "RELAY", "START_AND_STOP_EQUIPMENT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcSystemFurnitureElementTypeEnum_[] = {"PANEL", "WORKSURFACE", "SUBRACK", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTankTypeEnum_[] = {"BASIN", "BREAKPRESSURE", "EXPANSION", "FEEDANDEXPANSION", "PRESSUREVESSEL", "STORAGE", "VESSEL", "OILRETENTIONTRAY", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTaskDurationEnum_[] = {"ELAPSEDTIME", "WORKTIME", "NOTDEFINED", NULL};
    static TextValue IfcTaskTypeEnum_[] = {"ADJUSTMENT", "ATTENDANCE", "CALIBRATION", "CONSTRUCTION", "DEMOLITION", "DISMANTLE", "DISPOSAL", "EMERGENCY", "INSPECTION", "INSTALLATION", "LOGISTIC", "MAINTENANCE", "MOVE", "OPERATION", "REMOVAL", "RENOVATION", "SAFETY", "SHUTDOWN", "STARTUP", "TESTING", "TROUBLESHOOTING", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTendonAnchorTypeEnum_[] = {"COUPLER", "FIXED_END", "TENSIONING_END", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTendonConduitTypeEnum_[] = {"DUCT", "COUPLER", "GROUTING_DUCT", "TRUMPET", "DIABOLO", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTendonTypeEnum_[] = {"BAR", "COATED", "STRAND", "WIRE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTextPath_[] = {"LEFT", "RIGHT", "UP", "DOWN", NULL};
    static TextValue IfcTimeSeriesDataTypeEnum_[] = {"CONTINUOUS", "DISCRETE", "DISCRETEBINARY", "PIECEWISEBINARY", "PIECEWISECONSTANT", "PIECEWISECONTINUOUS", "NOTDEFINED", NULL};
    static TextValue IfcTrackElementTypeEnum_[] = {"TRACKENDOFALIGNMENT", "BLOCKINGDEVICE", "VEHICLESTOP", "SLEEPER", "HALF_SET_OF_BLADES", "SPEEDREGULATOR", "DERAILER", "FROG", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTransformerTypeEnum_[] = {"CURRENT", "FREQUENCY", "INVERTER", "RECTIFIER", "VOLTAGE", "CHOPPER", "COMBINED", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTransitionCode_[] = {"DISCONTINUOUS", "CONTINUOUS", "CONTSAMEGRADIENT", "CONTSAMEGRADIENTSAMECURVATURE", NULL};
    static TextValue IfcTransportElementTypeEnum_[] = {"ELEVATOR", "ESCALATOR", "MOVINGWALKWAY", "CRANEWAY", "LIFTINGGEAR", "HAULINGGEAR", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTrimmingPreference_[] = {"CARTESIAN", "PARAMETER", "UNSPECIFIED", NULL};
    static TextValue IfcTubeBundleTypeEnum_[] = {"FINNED", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcTunnelPartTypeEnum_[] = {"TUNNELSECTION", "CROSSWAY", "RINGSECTION", "PORTAL", "NOTDEFINED", "USERDEFINED", NULL};
    static TextValue IfcTunnelTypeEnum_[] = {"ACCESSTUNNEL", "SHAFT", "UTILITIES", "RAILWAY", "ROAD", "PEDESTRIAN", "METRO", "BICYCLE", "BYPASS", "MAINTENANCE", "UNDERGROUND_FACILITIES", "RAMP", "NOTDEFINED", "USERDEFINED", NULL};
    static TextValue IfcUndergroundExcavationTypeEnum_[] = {"FACEEXCAVATION", "RADIALEXCAVATION", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcUnitaryControlElementTypeEnum_[] = {"ALARMPANEL", "CONTROLPANEL", "GASDETECTIONPANEL", "INDICATORPANEL", "MIMICPANEL", "HUMIDISTAT", "THERMOSTAT", "WEATHERSTATION", "COMBINED", "BASESTATIONCONTROLLER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcUnitaryEquipmentTypeEnum_[] = {"AIRHANDLER", "AIRCONDITIONINGUNIT", "DEHUMIDIFIER", "SPLITSYSTEM", "ROOFTOPUNIT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcUnitEnum_[] = {"ABSORBEDDOSEUNIT", "AMOUNTOFSUBSTANCEUNIT", "AREAUNIT", "DOSEEQUIVALENTUNIT", "ELECTRICCAPACITANCEUNIT", "ELECTRICCHARGEUNIT", "ELECTRICCONDUCTANCEUNIT", "ELECTRICCURRENTUNIT", "ELECTRICRESISTANCEUNIT", "ELECTRICVOLTAGEUNIT", "ENERGYUNIT", "FORCEUNIT", "FREQUENCYUNIT", "ILLUMINANCEUNIT", "INDUCTANCEUNIT", "LENGTHUNIT", "LUMINOUSFLUXUNIT", "LUMINOUSINTENSITYUNIT", "MAGNETICFLUXDENSITYUNIT", "MAGNETICFLUXUNIT", "MASSUNIT", "PLANEANGLEUNIT", "POWERUNIT", "PRESSUREUNIT", "RADIOACTIVITYUNIT", "SOLIDANGLEUNIT", "THERMODYNAMICTEMPERATUREUNIT", "TIMEUNIT", "VOLUMEUNIT", "USERDEFINED", NULL};
    static TextValue IfcValveTypeEnum_[] = {"AIRRELEASE", "ANTIVACUUM", "CHANGEOVER", "CHECK", "COMMISSIONING", "DIVERTING", "DRAWOFFCOCK", "DOUBLECHECK", "DOUBLEREGULATING", "FAUCET", "FLUSHING", "GASCOCK", "GASTAP", "ISOLATING", "MIXING", "PRESSUREREDUCING", "PRESSURERELIEF", "REGULATING", "SAFETYCUTOFF", "STEAMTRAP", "STOPCOCK", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcVehicleTypeEnum_[] = {"VEHICLE", "VEHICLETRACKED", "ROLLINGSTOCK", "VEHICLEWHEELED", "VEHICLEAIR", "CARGO", "VEHICLEMARINE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcVibrationDamperTypeEnum_[] = {"BENDING_YIELD", "SHEAR_YIELD", "AXIAL_YIELD", "FRICTION", "VISCOUS", "RUBBER", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcVibrationIsolatorTypeEnum_[] = {"COMPRESSION", "SPRING", "BASE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcVoidingFeatureTypeEnum_[] = {"CUTOUT", "NOTCH", "HOLE", "MITER", "CHAMFER", "EDGE", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcWallTypeEnum_[] = {"MOVABLE", "PARAPET", "PARTITIONING", "PLUMBINGWALL", "SHEAR", "SOLIDWALL", "STANDARD", "POLYGONAL", "ELEMENTEDWALL", "RETAININGWALL", "WAVEWALL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcWasteTerminalTypeEnum_[] = {"FLOORTRAP", "FLOORWASTE", "GULLYSUMP", "GULLYTRAP", "ROOFDRAIN", "WASTEDISPOSALUNIT", "WASTETRAP", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcWindowPanelOperationEnum_[] = {"SIDEHUNGRIGHTHAND", "SIDEHUNGLEFTHAND", "TILTANDTURNRIGHTHAND", "TILTANDTURNLEFTHAND", "TOPHUNG", "BOTTOMHUNG", "PIVOTHORIZONTAL", "PIVOTVERTICAL", "SLIDINGHORIZONTAL", "SLIDINGVERTICAL", "REMOVABLECASEMENT", "FIXEDCASEMENT", "OTHEROPERATION", "NOTDEFINED", NULL};
    static TextValue IfcWindowPanelPositionEnum_[] = {"LEFT", "MIDDLE", "RIGHT", "BOTTOM", "TOP", "NOTDEFINED", NULL};
    static TextValue IfcWindowStyleConstructionEnum_[] = {"ALUMINIUM", "HIGH_GRADE_STEEL", "STEEL", "WOOD", "ALUMINIUM_WOOD", "PLASTIC", "OTHER_CONSTRUCTION", "NOTDEFINED", NULL};
    static TextValue IfcWindowStyleOperationEnum_[] = {"SINGLE_PANEL", "DOUBLE_PANEL_VERTICAL", "DOUBLE_PANEL_HORIZONTAL", "TRIPLE_PANEL_VERTICAL", "TRIPLE_PANEL_BOTTOM", "TRIPLE_PANEL_TOP", "TRIPLE_PANEL_LEFT", "TRIPLE_PANEL_RIGHT", "TRIPLE_PANEL_HORIZONTAL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcWindowTypeEnum_[] = {"WINDOW", "SKYLIGHT", "LIGHTDOME", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcWindowTypePartitioningEnum_[] = {"SINGLE_PANEL", "DOUBLE_PANEL_VERTICAL", "DOUBLE_PANEL_HORIZONTAL", "TRIPLE_PANEL_VERTICAL", "TRIPLE_PANEL_BOTTOM", "TRIPLE_PANEL_TOP", "TRIPLE_PANEL_LEFT", "TRIPLE_PANEL_RIGHT", "TRIPLE_PANEL_HORIZONTAL", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcWorkCalendarTypeEnum_[] = {"FIRSTSHIFT", "SECONDSHIFT", "THIRDSHIFT", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcWorkPlanTypeEnum_[] = {"ACTUAL", "BASELINE", "PLANNED", "USERDEFINED", "NOTDEFINED", NULL};
    static TextValue IfcWorkScheduleTypeEnum_[] = {"ACTUAL", "BASELINE", "PLANNED", "USERDEFINED", "NOTDEFINED", NULL};

    //
    // Defined types
    // 
    typedef double IfcAbsorbedDoseMeasure;
    typedef double IfcAccelerationMeasure;
    typedef double IfcAmountOfSubstanceMeasure;
    typedef double IfcAngularVelocityMeasure;
    typedef IntValue IfcInteger;
    typedef IntValue IfcPositiveInteger;
    typedef std::list<IfcPositiveInteger> IfcArcIndex;
    template <typename TList> class IfcArcIndexSerializer : public AggrSerializerSimple<TList, IfcPositiveInteger, sdaiINTEGER> {};
    typedef double IfcAreaDensityMeasure;
    typedef double IfcAreaMeasure;
    typedef TextValue IfcBinary;
    typedef bool IfcBoolean;
    typedef TextValue IfcLabel;
    typedef TextValue IfcBoxAlignment;
    typedef IntValue IfcCardinalPointReference;
    typedef std::list<double> IfcComplexNumber;
    template <typename TList> class IfcComplexNumberSerializer : public AggrSerializerSimple<TList, double, sdaiREAL> {};
    typedef std::list<IntValue> IfcCompoundPlaneAngleMeasure;
    template <typename TList> class IfcCompoundPlaneAngleMeasureSerializer : public AggrSerializerSimple<TList, IntValue, sdaiINTEGER> {};
    typedef double IfcContextDependentMeasure;
    typedef double IfcCountMeasure;
    typedef double IfcCurvatureMeasure;
    typedef TextValue IfcDate;
    typedef TextValue IfcDateTime;
    typedef IntValue IfcDayInMonthNumber;
    typedef IntValue IfcDayInWeekNumber;
    typedef TextValue IfcDescriptiveMeasure;
    typedef IntValue IfcDimensionCount;
    typedef double IfcDoseEquivalentMeasure;
    typedef TextValue IfcDuration;
    typedef double IfcDynamicViscosityMeasure;
    typedef double IfcElectricCapacitanceMeasure;
    typedef double IfcElectricChargeMeasure;
    typedef double IfcElectricConductanceMeasure;
    typedef double IfcElectricCurrentMeasure;
    typedef double IfcElectricResistanceMeasure;
    typedef double IfcElectricVoltageMeasure;
    typedef double IfcEnergyMeasure;
    typedef TextValue IfcFontStyle;
    typedef TextValue IfcFontVariant;
    typedef TextValue IfcFontWeight;
    typedef double IfcForceMeasure;
    typedef double IfcFrequencyMeasure;
    typedef TextValue IfcGloballyUniqueId;
    typedef double IfcHeatFluxDensityMeasure;
    typedef double IfcHeatingValueMeasure;
    typedef TextValue IfcIdentifier;
    typedef double IfcIlluminanceMeasure;
    typedef double IfcInductanceMeasure;
    typedef IntValue IfcIntegerCountRateMeasure;
    typedef double IfcIonConcentrationMeasure;
    typedef double IfcIsothermalMoistureCapacityMeasure;
    typedef double IfcKinematicViscosityMeasure;
    typedef IfcIdentifier IfcLanguageId;
    typedef double IfcLengthMeasure;
    typedef double IfcLinearForceMeasure;
    typedef double IfcLinearMomentMeasure;
    typedef double IfcLinearStiffnessMeasure;
    typedef double IfcLinearVelocityMeasure;
    typedef std::list<IfcPositiveInteger> IfcLineIndex;
    template <typename TList> class IfcLineIndexSerializer : public AggrSerializerSimple<TList, IfcPositiveInteger, sdaiINTEGER> {};
    typedef LOGICAL_VALUE IfcLogical;
    typedef double IfcLuminousFluxMeasure;
    typedef double IfcLuminousIntensityDistributionMeasure;
    typedef double IfcLuminousIntensityMeasure;
    typedef double IfcMagneticFluxDensityMeasure;
    typedef double IfcMagneticFluxMeasure;
    typedef double IfcMassDensityMeasure;
    typedef double IfcMassFlowRateMeasure;
    typedef double IfcMassMeasure;
    typedef double IfcMassPerLengthMeasure;
    typedef double IfcModulusOfElasticityMeasure;
    typedef double IfcModulusOfLinearSubgradeReactionMeasure;
    typedef double IfcModulusOfRotationalSubgradeReactionMeasure;
    typedef double IfcModulusOfSubgradeReactionMeasure;
    typedef double IfcMoistureDiffusivityMeasure;
    typedef double IfcMolecularWeightMeasure;
    typedef double IfcMomentOfInertiaMeasure;
    typedef double IfcMonetaryMeasure;
    typedef IntValue IfcMonthInYearNumber;
    typedef IfcLengthMeasure IfcNonNegativeLengthMeasure;
    typedef double IfcRatioMeasure;
    typedef double IfcNormalisedRatioMeasure;
    typedef double IfcNumericMeasure;
    typedef double IfcParameterValue;
    typedef double IfcPHMeasure;
    typedef double IfcPlanarForceMeasure;
    typedef double IfcPlaneAngleMeasure;
    typedef IfcLengthMeasure IfcPositiveLengthMeasure;
    typedef IfcPlaneAngleMeasure IfcPositivePlaneAngleMeasure;
    typedef IfcRatioMeasure IfcPositiveRatioMeasure;
    typedef double IfcPowerMeasure;
    typedef TextValue IfcPresentableText;
    typedef double IfcPressureMeasure;
    typedef std::list<IfcPropertySetDefinition> IfcPropertySetDefinitionSet;
    template <typename TList> class IfcPropertySetDefinitionSetSerializer : public AggrSerializerInstance<TList, IfcPropertySetDefinition> {};
    typedef double IfcRadioActivityMeasure;
    typedef double IfcReal;
    typedef double IfcRotationalFrequencyMeasure;
    typedef double IfcRotationalMassMeasure;
    typedef double IfcRotationalStiffnessMeasure;
    typedef double IfcSectionalAreaIntegralMeasure;
    typedef double IfcSectionModulusMeasure;
    typedef double IfcShearModulusMeasure;
    typedef double IfcSolidAngleMeasure;
    typedef double IfcSoundPowerLevelMeasure;
    typedef double IfcSoundPowerMeasure;
    typedef double IfcSoundPressureLevelMeasure;
    typedef double IfcSoundPressureMeasure;
    typedef double IfcSpecificHeatCapacityMeasure;
    typedef double IfcSpecularExponent;
    typedef double IfcSpecularRoughness;
    typedef double IfcTemperatureGradientMeasure;
    typedef double IfcTemperatureRateOfChangeMeasure;
    typedef TextValue IfcText;
    typedef TextValue IfcTextAlignment;
    typedef TextValue IfcTextDecoration;
    typedef TextValue IfcTextFontName;
    typedef TextValue IfcTextTransformation;
    typedef double IfcThermalAdmittanceMeasure;
    typedef double IfcThermalConductivityMeasure;
    typedef double IfcThermalExpansionCoefficientMeasure;
    typedef double IfcThermalResistanceMeasure;
    typedef double IfcThermalTransmittanceMeasure;
    typedef double IfcThermodynamicTemperatureMeasure;
    typedef TextValue IfcTime;
    typedef double IfcTimeMeasure;
    typedef IntValue IfcTimeStamp;
    typedef double IfcTorqueMeasure;
    typedef TextValue IfcURIReference;
    typedef double IfcVaporPermeabilityMeasure;
    typedef double IfcVolumeMeasure;
    typedef double IfcVolumetricFlowRateMeasure;
    typedef double IfcWarpingConstantMeasure;
    typedef double IfcWarpingMomentMeasure;
    typedef TextValue IfcWellKnownTextLiteral;

    class IfcActorSelect : public Select
    {
    public:
        IfcActorSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcActorSelect(Select* outer) : Select(outer) {}

        bool is_IfcOrganization() { return IsADBEntity("IfcOrganization"); }
        IfcOrganization get_IfcOrganization();
        void put_IfcOrganization(IfcOrganization inst);

        bool is_IfcPerson() { return IsADBEntity("IfcPerson"); }
        IfcPerson get_IfcPerson();
        void put_IfcPerson(IfcPerson inst);

        bool is_IfcPersonAndOrganization() { return IsADBEntity("IfcPersonAndOrganization"); }
        IfcPersonAndOrganization get_IfcPersonAndOrganization();
        void put_IfcPersonAndOrganization(IfcPersonAndOrganization inst);
    };


    class IfcActorSelect_get : public Select
    {
    public:
        IfcActorSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcActorSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcOrganization() { return IsADBEntity("IfcOrganization"); }
        IfcOrganization get_IfcOrganization();
        bool is_IfcPerson() { return IsADBEntity("IfcPerson"); }
        IfcPerson get_IfcPerson();
        bool is_IfcPersonAndOrganization() { return IsADBEntity("IfcPersonAndOrganization"); }
        IfcPersonAndOrganization get_IfcPersonAndOrganization();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcActorSelect_put : public Select
    {
    public:
        IfcActorSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcActorSelect_put(Select* outer) : Select(outer) {}
        void put_IfcOrganization(IfcOrganization inst);
        void put_IfcPerson(IfcPerson inst);
        void put_IfcPersonAndOrganization(IfcPersonAndOrganization inst);
    };


    class IfcDerivedMeasureValue : public Select
    {
    public:
        IfcDerivedMeasureValue(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDerivedMeasureValue(Select* outer) : Select(outer) {}

        bool is_IfcAbsorbedDoseMeasure() { return IsADBType("IFCABSORBEDDOSEMEASURE"); }
        Nullable<IfcAbsorbedDoseMeasure> get_IfcAbsorbedDoseMeasure() { return getSimpleValue<IfcAbsorbedDoseMeasure>("IFCABSORBEDDOSEMEASURE", sdaiREAL); }
        void put_IfcAbsorbedDoseMeasure(IfcAbsorbedDoseMeasure value) { putSimpleValue("IFCABSORBEDDOSEMEASURE", sdaiREAL, value); }

        bool is_IfcAccelerationMeasure() { return IsADBType("IFCACCELERATIONMEASURE"); }
        Nullable<IfcAccelerationMeasure> get_IfcAccelerationMeasure() { return getSimpleValue<IfcAccelerationMeasure>("IFCACCELERATIONMEASURE", sdaiREAL); }
        void put_IfcAccelerationMeasure(IfcAccelerationMeasure value) { putSimpleValue("IFCACCELERATIONMEASURE", sdaiREAL, value); }

        bool is_IfcAngularVelocityMeasure() { return IsADBType("IFCANGULARVELOCITYMEASURE"); }
        Nullable<IfcAngularVelocityMeasure> get_IfcAngularVelocityMeasure() { return getSimpleValue<IfcAngularVelocityMeasure>("IFCANGULARVELOCITYMEASURE", sdaiREAL); }
        void put_IfcAngularVelocityMeasure(IfcAngularVelocityMeasure value) { putSimpleValue("IFCANGULARVELOCITYMEASURE", sdaiREAL, value); }

        bool is_IfcAreaDensityMeasure() { return IsADBType("IFCAREADENSITYMEASURE"); }
        Nullable<IfcAreaDensityMeasure> get_IfcAreaDensityMeasure() { return getSimpleValue<IfcAreaDensityMeasure>("IFCAREADENSITYMEASURE", sdaiREAL); }
        void put_IfcAreaDensityMeasure(IfcAreaDensityMeasure value) { putSimpleValue("IFCAREADENSITYMEASURE", sdaiREAL, value); }

        bool is_IfcCompoundPlaneAngleMeasure() { return IsADBType("IFCCOMPOUNDPLANEANGLEMEASURE"); }

        //TList may be IfcCompoundPlaneAngleMeasure or list of converible elements
        template <typename TList> void get_IfcCompoundPlaneAngleMeasure(TList& lst) { SdaiAggr aggr = getAggrValue("IFCCOMPOUNDPLANEANGLEMEASURE"); IfcCompoundPlaneAngleMeasureSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }

                //TList may be IfcCompoundPlaneAngleMeasure or list of converible elements
        template <typename TList> void put_IfcCompoundPlaneAngleMeasure(TList& lst) { IfcCompoundPlaneAngleMeasureSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCCOMPOUNDPLANEANGLEMEASURE", aggr); }

                //TArrayElem[] may be IntValue[] or array of convertible elements
        template <typename TArrayElem> void put_IfcCompoundPlaneAngleMeasure(TArrayElem arr[], size_t n) { IfcCompoundPlaneAngleMeasure lst; ArrayToList(arr, n, lst); put_IfcCompoundPlaneAngleMeasure(lst); }

        bool is_IfcCurvatureMeasure() { return IsADBType("IFCCURVATUREMEASURE"); }
        Nullable<IfcCurvatureMeasure> get_IfcCurvatureMeasure() { return getSimpleValue<IfcCurvatureMeasure>("IFCCURVATUREMEASURE", sdaiREAL); }
        void put_IfcCurvatureMeasure(IfcCurvatureMeasure value) { putSimpleValue("IFCCURVATUREMEASURE", sdaiREAL, value); }

        bool is_IfcDoseEquivalentMeasure() { return IsADBType("IFCDOSEEQUIVALENTMEASURE"); }
        Nullable<IfcDoseEquivalentMeasure> get_IfcDoseEquivalentMeasure() { return getSimpleValue<IfcDoseEquivalentMeasure>("IFCDOSEEQUIVALENTMEASURE", sdaiREAL); }
        void put_IfcDoseEquivalentMeasure(IfcDoseEquivalentMeasure value) { putSimpleValue("IFCDOSEEQUIVALENTMEASURE", sdaiREAL, value); }

        bool is_IfcDynamicViscosityMeasure() { return IsADBType("IFCDYNAMICVISCOSITYMEASURE"); }
        Nullable<IfcDynamicViscosityMeasure> get_IfcDynamicViscosityMeasure() { return getSimpleValue<IfcDynamicViscosityMeasure>("IFCDYNAMICVISCOSITYMEASURE", sdaiREAL); }
        void put_IfcDynamicViscosityMeasure(IfcDynamicViscosityMeasure value) { putSimpleValue("IFCDYNAMICVISCOSITYMEASURE", sdaiREAL, value); }

        bool is_IfcElectricCapacitanceMeasure() { return IsADBType("IFCELECTRICCAPACITANCEMEASURE"); }
        Nullable<IfcElectricCapacitanceMeasure> get_IfcElectricCapacitanceMeasure() { return getSimpleValue<IfcElectricCapacitanceMeasure>("IFCELECTRICCAPACITANCEMEASURE", sdaiREAL); }
        void put_IfcElectricCapacitanceMeasure(IfcElectricCapacitanceMeasure value) { putSimpleValue("IFCELECTRICCAPACITANCEMEASURE", sdaiREAL, value); }

        bool is_IfcElectricChargeMeasure() { return IsADBType("IFCELECTRICCHARGEMEASURE"); }
        Nullable<IfcElectricChargeMeasure> get_IfcElectricChargeMeasure() { return getSimpleValue<IfcElectricChargeMeasure>("IFCELECTRICCHARGEMEASURE", sdaiREAL); }
        void put_IfcElectricChargeMeasure(IfcElectricChargeMeasure value) { putSimpleValue("IFCELECTRICCHARGEMEASURE", sdaiREAL, value); }

        bool is_IfcElectricConductanceMeasure() { return IsADBType("IFCELECTRICCONDUCTANCEMEASURE"); }
        Nullable<IfcElectricConductanceMeasure> get_IfcElectricConductanceMeasure() { return getSimpleValue<IfcElectricConductanceMeasure>("IFCELECTRICCONDUCTANCEMEASURE", sdaiREAL); }
        void put_IfcElectricConductanceMeasure(IfcElectricConductanceMeasure value) { putSimpleValue("IFCELECTRICCONDUCTANCEMEASURE", sdaiREAL, value); }

        bool is_IfcElectricResistanceMeasure() { return IsADBType("IFCELECTRICRESISTANCEMEASURE"); }
        Nullable<IfcElectricResistanceMeasure> get_IfcElectricResistanceMeasure() { return getSimpleValue<IfcElectricResistanceMeasure>("IFCELECTRICRESISTANCEMEASURE", sdaiREAL); }
        void put_IfcElectricResistanceMeasure(IfcElectricResistanceMeasure value) { putSimpleValue("IFCELECTRICRESISTANCEMEASURE", sdaiREAL, value); }

        bool is_IfcElectricVoltageMeasure() { return IsADBType("IFCELECTRICVOLTAGEMEASURE"); }
        Nullable<IfcElectricVoltageMeasure> get_IfcElectricVoltageMeasure() { return getSimpleValue<IfcElectricVoltageMeasure>("IFCELECTRICVOLTAGEMEASURE", sdaiREAL); }
        void put_IfcElectricVoltageMeasure(IfcElectricVoltageMeasure value) { putSimpleValue("IFCELECTRICVOLTAGEMEASURE", sdaiREAL, value); }

        bool is_IfcEnergyMeasure() { return IsADBType("IFCENERGYMEASURE"); }
        Nullable<IfcEnergyMeasure> get_IfcEnergyMeasure() { return getSimpleValue<IfcEnergyMeasure>("IFCENERGYMEASURE", sdaiREAL); }
        void put_IfcEnergyMeasure(IfcEnergyMeasure value) { putSimpleValue("IFCENERGYMEASURE", sdaiREAL, value); }

        bool is_IfcForceMeasure() { return IsADBType("IFCFORCEMEASURE"); }
        Nullable<IfcForceMeasure> get_IfcForceMeasure() { return getSimpleValue<IfcForceMeasure>("IFCFORCEMEASURE", sdaiREAL); }
        void put_IfcForceMeasure(IfcForceMeasure value) { putSimpleValue("IFCFORCEMEASURE", sdaiREAL, value); }

        bool is_IfcFrequencyMeasure() { return IsADBType("IFCFREQUENCYMEASURE"); }
        Nullable<IfcFrequencyMeasure> get_IfcFrequencyMeasure() { return getSimpleValue<IfcFrequencyMeasure>("IFCFREQUENCYMEASURE", sdaiREAL); }
        void put_IfcFrequencyMeasure(IfcFrequencyMeasure value) { putSimpleValue("IFCFREQUENCYMEASURE", sdaiREAL, value); }

        bool is_IfcHeatFluxDensityMeasure() { return IsADBType("IFCHEATFLUXDENSITYMEASURE"); }
        Nullable<IfcHeatFluxDensityMeasure> get_IfcHeatFluxDensityMeasure() { return getSimpleValue<IfcHeatFluxDensityMeasure>("IFCHEATFLUXDENSITYMEASURE", sdaiREAL); }
        void put_IfcHeatFluxDensityMeasure(IfcHeatFluxDensityMeasure value) { putSimpleValue("IFCHEATFLUXDENSITYMEASURE", sdaiREAL, value); }

        bool is_IfcHeatingValueMeasure() { return IsADBType("IFCHEATINGVALUEMEASURE"); }
        Nullable<IfcHeatingValueMeasure> get_IfcHeatingValueMeasure() { return getSimpleValue<IfcHeatingValueMeasure>("IFCHEATINGVALUEMEASURE", sdaiREAL); }
        void put_IfcHeatingValueMeasure(IfcHeatingValueMeasure value) { putSimpleValue("IFCHEATINGVALUEMEASURE", sdaiREAL, value); }

        bool is_IfcIlluminanceMeasure() { return IsADBType("IFCILLUMINANCEMEASURE"); }
        Nullable<IfcIlluminanceMeasure> get_IfcIlluminanceMeasure() { return getSimpleValue<IfcIlluminanceMeasure>("IFCILLUMINANCEMEASURE", sdaiREAL); }
        void put_IfcIlluminanceMeasure(IfcIlluminanceMeasure value) { putSimpleValue("IFCILLUMINANCEMEASURE", sdaiREAL, value); }

        bool is_IfcInductanceMeasure() { return IsADBType("IFCINDUCTANCEMEASURE"); }
        Nullable<IfcInductanceMeasure> get_IfcInductanceMeasure() { return getSimpleValue<IfcInductanceMeasure>("IFCINDUCTANCEMEASURE", sdaiREAL); }
        void put_IfcInductanceMeasure(IfcInductanceMeasure value) { putSimpleValue("IFCINDUCTANCEMEASURE", sdaiREAL, value); }

        bool is_IfcIntegerCountRateMeasure() { return IsADBType("IFCINTEGERCOUNTRATEMEASURE"); }
        Nullable<IfcIntegerCountRateMeasure> get_IfcIntegerCountRateMeasure() { return getSimpleValue<IfcIntegerCountRateMeasure>("IFCINTEGERCOUNTRATEMEASURE", sdaiINTEGER); }
        void put_IfcIntegerCountRateMeasure(IfcIntegerCountRateMeasure value) { putSimpleValue("IFCINTEGERCOUNTRATEMEASURE", sdaiINTEGER, value); }

        bool is_IfcIonConcentrationMeasure() { return IsADBType("IFCIONCONCENTRATIONMEASURE"); }
        Nullable<IfcIonConcentrationMeasure> get_IfcIonConcentrationMeasure() { return getSimpleValue<IfcIonConcentrationMeasure>("IFCIONCONCENTRATIONMEASURE", sdaiREAL); }
        void put_IfcIonConcentrationMeasure(IfcIonConcentrationMeasure value) { putSimpleValue("IFCIONCONCENTRATIONMEASURE", sdaiREAL, value); }

        bool is_IfcIsothermalMoistureCapacityMeasure() { return IsADBType("IFCISOTHERMALMOISTURECAPACITYMEASURE"); }
        Nullable<IfcIsothermalMoistureCapacityMeasure> get_IfcIsothermalMoistureCapacityMeasure() { return getSimpleValue<IfcIsothermalMoistureCapacityMeasure>("IFCISOTHERMALMOISTURECAPACITYMEASURE", sdaiREAL); }
        void put_IfcIsothermalMoistureCapacityMeasure(IfcIsothermalMoistureCapacityMeasure value) { putSimpleValue("IFCISOTHERMALMOISTURECAPACITYMEASURE", sdaiREAL, value); }

        bool is_IfcKinematicViscosityMeasure() { return IsADBType("IFCKINEMATICVISCOSITYMEASURE"); }
        Nullable<IfcKinematicViscosityMeasure> get_IfcKinematicViscosityMeasure() { return getSimpleValue<IfcKinematicViscosityMeasure>("IFCKINEMATICVISCOSITYMEASURE", sdaiREAL); }
        void put_IfcKinematicViscosityMeasure(IfcKinematicViscosityMeasure value) { putSimpleValue("IFCKINEMATICVISCOSITYMEASURE", sdaiREAL, value); }

        bool is_IfcLinearForceMeasure() { return IsADBType("IFCLINEARFORCEMEASURE"); }
        Nullable<IfcLinearForceMeasure> get_IfcLinearForceMeasure() { return getSimpleValue<IfcLinearForceMeasure>("IFCLINEARFORCEMEASURE", sdaiREAL); }
        void put_IfcLinearForceMeasure(IfcLinearForceMeasure value) { putSimpleValue("IFCLINEARFORCEMEASURE", sdaiREAL, value); }

        bool is_IfcLinearMomentMeasure() { return IsADBType("IFCLINEARMOMENTMEASURE"); }
        Nullable<IfcLinearMomentMeasure> get_IfcLinearMomentMeasure() { return getSimpleValue<IfcLinearMomentMeasure>("IFCLINEARMOMENTMEASURE", sdaiREAL); }
        void put_IfcLinearMomentMeasure(IfcLinearMomentMeasure value) { putSimpleValue("IFCLINEARMOMENTMEASURE", sdaiREAL, value); }

        bool is_IfcLinearStiffnessMeasure() { return IsADBType("IFCLINEARSTIFFNESSMEASURE"); }
        Nullable<IfcLinearStiffnessMeasure> get_IfcLinearStiffnessMeasure() { return getSimpleValue<IfcLinearStiffnessMeasure>("IFCLINEARSTIFFNESSMEASURE", sdaiREAL); }
        void put_IfcLinearStiffnessMeasure(IfcLinearStiffnessMeasure value) { putSimpleValue("IFCLINEARSTIFFNESSMEASURE", sdaiREAL, value); }

        bool is_IfcLinearVelocityMeasure() { return IsADBType("IFCLINEARVELOCITYMEASURE"); }
        Nullable<IfcLinearVelocityMeasure> get_IfcLinearVelocityMeasure() { return getSimpleValue<IfcLinearVelocityMeasure>("IFCLINEARVELOCITYMEASURE", sdaiREAL); }
        void put_IfcLinearVelocityMeasure(IfcLinearVelocityMeasure value) { putSimpleValue("IFCLINEARVELOCITYMEASURE", sdaiREAL, value); }

        bool is_IfcLuminousFluxMeasure() { return IsADBType("IFCLUMINOUSFLUXMEASURE"); }
        Nullable<IfcLuminousFluxMeasure> get_IfcLuminousFluxMeasure() { return getSimpleValue<IfcLuminousFluxMeasure>("IFCLUMINOUSFLUXMEASURE", sdaiREAL); }
        void put_IfcLuminousFluxMeasure(IfcLuminousFluxMeasure value) { putSimpleValue("IFCLUMINOUSFLUXMEASURE", sdaiREAL, value); }

        bool is_IfcLuminousIntensityDistributionMeasure() { return IsADBType("IFCLUMINOUSINTENSITYDISTRIBUTIONMEASURE"); }
        Nullable<IfcLuminousIntensityDistributionMeasure> get_IfcLuminousIntensityDistributionMeasure() { return getSimpleValue<IfcLuminousIntensityDistributionMeasure>("IFCLUMINOUSINTENSITYDISTRIBUTIONMEASURE", sdaiREAL); }
        void put_IfcLuminousIntensityDistributionMeasure(IfcLuminousIntensityDistributionMeasure value) { putSimpleValue("IFCLUMINOUSINTENSITYDISTRIBUTIONMEASURE", sdaiREAL, value); }

        bool is_IfcMagneticFluxDensityMeasure() { return IsADBType("IFCMAGNETICFLUXDENSITYMEASURE"); }
        Nullable<IfcMagneticFluxDensityMeasure> get_IfcMagneticFluxDensityMeasure() { return getSimpleValue<IfcMagneticFluxDensityMeasure>("IFCMAGNETICFLUXDENSITYMEASURE", sdaiREAL); }
        void put_IfcMagneticFluxDensityMeasure(IfcMagneticFluxDensityMeasure value) { putSimpleValue("IFCMAGNETICFLUXDENSITYMEASURE", sdaiREAL, value); }

        bool is_IfcMagneticFluxMeasure() { return IsADBType("IFCMAGNETICFLUXMEASURE"); }
        Nullable<IfcMagneticFluxMeasure> get_IfcMagneticFluxMeasure() { return getSimpleValue<IfcMagneticFluxMeasure>("IFCMAGNETICFLUXMEASURE", sdaiREAL); }
        void put_IfcMagneticFluxMeasure(IfcMagneticFluxMeasure value) { putSimpleValue("IFCMAGNETICFLUXMEASURE", sdaiREAL, value); }

        bool is_IfcMassDensityMeasure() { return IsADBType("IFCMASSDENSITYMEASURE"); }
        Nullable<IfcMassDensityMeasure> get_IfcMassDensityMeasure() { return getSimpleValue<IfcMassDensityMeasure>("IFCMASSDENSITYMEASURE", sdaiREAL); }
        void put_IfcMassDensityMeasure(IfcMassDensityMeasure value) { putSimpleValue("IFCMASSDENSITYMEASURE", sdaiREAL, value); }

        bool is_IfcMassFlowRateMeasure() { return IsADBType("IFCMASSFLOWRATEMEASURE"); }
        Nullable<IfcMassFlowRateMeasure> get_IfcMassFlowRateMeasure() { return getSimpleValue<IfcMassFlowRateMeasure>("IFCMASSFLOWRATEMEASURE", sdaiREAL); }
        void put_IfcMassFlowRateMeasure(IfcMassFlowRateMeasure value) { putSimpleValue("IFCMASSFLOWRATEMEASURE", sdaiREAL, value); }

        bool is_IfcMassPerLengthMeasure() { return IsADBType("IFCMASSPERLENGTHMEASURE"); }
        Nullable<IfcMassPerLengthMeasure> get_IfcMassPerLengthMeasure() { return getSimpleValue<IfcMassPerLengthMeasure>("IFCMASSPERLENGTHMEASURE", sdaiREAL); }
        void put_IfcMassPerLengthMeasure(IfcMassPerLengthMeasure value) { putSimpleValue("IFCMASSPERLENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcModulusOfElasticityMeasure() { return IsADBType("IFCMODULUSOFELASTICITYMEASURE"); }
        Nullable<IfcModulusOfElasticityMeasure> get_IfcModulusOfElasticityMeasure() { return getSimpleValue<IfcModulusOfElasticityMeasure>("IFCMODULUSOFELASTICITYMEASURE", sdaiREAL); }
        void put_IfcModulusOfElasticityMeasure(IfcModulusOfElasticityMeasure value) { putSimpleValue("IFCMODULUSOFELASTICITYMEASURE", sdaiREAL, value); }

        bool is_IfcModulusOfLinearSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfLinearSubgradeReactionMeasure> get_IfcModulusOfLinearSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfLinearSubgradeReactionMeasure>("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE", sdaiREAL); }
        void put_IfcModulusOfLinearSubgradeReactionMeasure(IfcModulusOfLinearSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE", sdaiREAL, value); }

        bool is_IfcModulusOfRotationalSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfRotationalSubgradeReactionMeasure> get_IfcModulusOfRotationalSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfRotationalSubgradeReactionMeasure>("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE", sdaiREAL); }
        void put_IfcModulusOfRotationalSubgradeReactionMeasure(IfcModulusOfRotationalSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE", sdaiREAL, value); }

        bool is_IfcModulusOfSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfSubgradeReactionMeasure> get_IfcModulusOfSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfSubgradeReactionMeasure>("IFCMODULUSOFSUBGRADEREACTIONMEASURE", sdaiREAL); }
        void put_IfcModulusOfSubgradeReactionMeasure(IfcModulusOfSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFSUBGRADEREACTIONMEASURE", sdaiREAL, value); }

        bool is_IfcMoistureDiffusivityMeasure() { return IsADBType("IFCMOISTUREDIFFUSIVITYMEASURE"); }
        Nullable<IfcMoistureDiffusivityMeasure> get_IfcMoistureDiffusivityMeasure() { return getSimpleValue<IfcMoistureDiffusivityMeasure>("IFCMOISTUREDIFFUSIVITYMEASURE", sdaiREAL); }
        void put_IfcMoistureDiffusivityMeasure(IfcMoistureDiffusivityMeasure value) { putSimpleValue("IFCMOISTUREDIFFUSIVITYMEASURE", sdaiREAL, value); }

        bool is_IfcMolecularWeightMeasure() { return IsADBType("IFCMOLECULARWEIGHTMEASURE"); }
        Nullable<IfcMolecularWeightMeasure> get_IfcMolecularWeightMeasure() { return getSimpleValue<IfcMolecularWeightMeasure>("IFCMOLECULARWEIGHTMEASURE", sdaiREAL); }
        void put_IfcMolecularWeightMeasure(IfcMolecularWeightMeasure value) { putSimpleValue("IFCMOLECULARWEIGHTMEASURE", sdaiREAL, value); }

        bool is_IfcMomentOfInertiaMeasure() { return IsADBType("IFCMOMENTOFINERTIAMEASURE"); }
        Nullable<IfcMomentOfInertiaMeasure> get_IfcMomentOfInertiaMeasure() { return getSimpleValue<IfcMomentOfInertiaMeasure>("IFCMOMENTOFINERTIAMEASURE", sdaiREAL); }
        void put_IfcMomentOfInertiaMeasure(IfcMomentOfInertiaMeasure value) { putSimpleValue("IFCMOMENTOFINERTIAMEASURE", sdaiREAL, value); }

        bool is_IfcMonetaryMeasure() { return IsADBType("IFCMONETARYMEASURE"); }
        Nullable<IfcMonetaryMeasure> get_IfcMonetaryMeasure() { return getSimpleValue<IfcMonetaryMeasure>("IFCMONETARYMEASURE", sdaiREAL); }
        void put_IfcMonetaryMeasure(IfcMonetaryMeasure value) { putSimpleValue("IFCMONETARYMEASURE", sdaiREAL, value); }

        bool is_IfcPHMeasure() { return IsADBType("IFCPHMEASURE"); }
        Nullable<IfcPHMeasure> get_IfcPHMeasure() { return getSimpleValue<IfcPHMeasure>("IFCPHMEASURE", sdaiREAL); }
        void put_IfcPHMeasure(IfcPHMeasure value) { putSimpleValue("IFCPHMEASURE", sdaiREAL, value); }

        bool is_IfcPlanarForceMeasure() { return IsADBType("IFCPLANARFORCEMEASURE"); }
        Nullable<IfcPlanarForceMeasure> get_IfcPlanarForceMeasure() { return getSimpleValue<IfcPlanarForceMeasure>("IFCPLANARFORCEMEASURE", sdaiREAL); }
        void put_IfcPlanarForceMeasure(IfcPlanarForceMeasure value) { putSimpleValue("IFCPLANARFORCEMEASURE", sdaiREAL, value); }

        bool is_IfcPowerMeasure() { return IsADBType("IFCPOWERMEASURE"); }
        Nullable<IfcPowerMeasure> get_IfcPowerMeasure() { return getSimpleValue<IfcPowerMeasure>("IFCPOWERMEASURE", sdaiREAL); }
        void put_IfcPowerMeasure(IfcPowerMeasure value) { putSimpleValue("IFCPOWERMEASURE", sdaiREAL, value); }

        bool is_IfcPressureMeasure() { return IsADBType("IFCPRESSUREMEASURE"); }
        Nullable<IfcPressureMeasure> get_IfcPressureMeasure() { return getSimpleValue<IfcPressureMeasure>("IFCPRESSUREMEASURE", sdaiREAL); }
        void put_IfcPressureMeasure(IfcPressureMeasure value) { putSimpleValue("IFCPRESSUREMEASURE", sdaiREAL, value); }

        bool is_IfcRadioActivityMeasure() { return IsADBType("IFCRADIOACTIVITYMEASURE"); }
        Nullable<IfcRadioActivityMeasure> get_IfcRadioActivityMeasure() { return getSimpleValue<IfcRadioActivityMeasure>("IFCRADIOACTIVITYMEASURE", sdaiREAL); }
        void put_IfcRadioActivityMeasure(IfcRadioActivityMeasure value) { putSimpleValue("IFCRADIOACTIVITYMEASURE", sdaiREAL, value); }

        bool is_IfcRotationalFrequencyMeasure() { return IsADBType("IFCROTATIONALFREQUENCYMEASURE"); }
        Nullable<IfcRotationalFrequencyMeasure> get_IfcRotationalFrequencyMeasure() { return getSimpleValue<IfcRotationalFrequencyMeasure>("IFCROTATIONALFREQUENCYMEASURE", sdaiREAL); }
        void put_IfcRotationalFrequencyMeasure(IfcRotationalFrequencyMeasure value) { putSimpleValue("IFCROTATIONALFREQUENCYMEASURE", sdaiREAL, value); }

        bool is_IfcRotationalMassMeasure() { return IsADBType("IFCROTATIONALMASSMEASURE"); }
        Nullable<IfcRotationalMassMeasure> get_IfcRotationalMassMeasure() { return getSimpleValue<IfcRotationalMassMeasure>("IFCROTATIONALMASSMEASURE", sdaiREAL); }
        void put_IfcRotationalMassMeasure(IfcRotationalMassMeasure value) { putSimpleValue("IFCROTATIONALMASSMEASURE", sdaiREAL, value); }

        bool is_IfcRotationalStiffnessMeasure() { return IsADBType("IFCROTATIONALSTIFFNESSMEASURE"); }
        Nullable<IfcRotationalStiffnessMeasure> get_IfcRotationalStiffnessMeasure() { return getSimpleValue<IfcRotationalStiffnessMeasure>("IFCROTATIONALSTIFFNESSMEASURE", sdaiREAL); }
        void put_IfcRotationalStiffnessMeasure(IfcRotationalStiffnessMeasure value) { putSimpleValue("IFCROTATIONALSTIFFNESSMEASURE", sdaiREAL, value); }

        bool is_IfcSectionModulusMeasure() { return IsADBType("IFCSECTIONMODULUSMEASURE"); }
        Nullable<IfcSectionModulusMeasure> get_IfcSectionModulusMeasure() { return getSimpleValue<IfcSectionModulusMeasure>("IFCSECTIONMODULUSMEASURE", sdaiREAL); }
        void put_IfcSectionModulusMeasure(IfcSectionModulusMeasure value) { putSimpleValue("IFCSECTIONMODULUSMEASURE", sdaiREAL, value); }

        bool is_IfcSectionalAreaIntegralMeasure() { return IsADBType("IFCSECTIONALAREAINTEGRALMEASURE"); }
        Nullable<IfcSectionalAreaIntegralMeasure> get_IfcSectionalAreaIntegralMeasure() { return getSimpleValue<IfcSectionalAreaIntegralMeasure>("IFCSECTIONALAREAINTEGRALMEASURE", sdaiREAL); }
        void put_IfcSectionalAreaIntegralMeasure(IfcSectionalAreaIntegralMeasure value) { putSimpleValue("IFCSECTIONALAREAINTEGRALMEASURE", sdaiREAL, value); }

        bool is_IfcShearModulusMeasure() { return IsADBType("IFCSHEARMODULUSMEASURE"); }
        Nullable<IfcShearModulusMeasure> get_IfcShearModulusMeasure() { return getSimpleValue<IfcShearModulusMeasure>("IFCSHEARMODULUSMEASURE", sdaiREAL); }
        void put_IfcShearModulusMeasure(IfcShearModulusMeasure value) { putSimpleValue("IFCSHEARMODULUSMEASURE", sdaiREAL, value); }

        bool is_IfcSoundPowerLevelMeasure() { return IsADBType("IFCSOUNDPOWERLEVELMEASURE"); }
        Nullable<IfcSoundPowerLevelMeasure> get_IfcSoundPowerLevelMeasure() { return getSimpleValue<IfcSoundPowerLevelMeasure>("IFCSOUNDPOWERLEVELMEASURE", sdaiREAL); }
        void put_IfcSoundPowerLevelMeasure(IfcSoundPowerLevelMeasure value) { putSimpleValue("IFCSOUNDPOWERLEVELMEASURE", sdaiREAL, value); }

        bool is_IfcSoundPowerMeasure() { return IsADBType("IFCSOUNDPOWERMEASURE"); }
        Nullable<IfcSoundPowerMeasure> get_IfcSoundPowerMeasure() { return getSimpleValue<IfcSoundPowerMeasure>("IFCSOUNDPOWERMEASURE", sdaiREAL); }
        void put_IfcSoundPowerMeasure(IfcSoundPowerMeasure value) { putSimpleValue("IFCSOUNDPOWERMEASURE", sdaiREAL, value); }

        bool is_IfcSoundPressureLevelMeasure() { return IsADBType("IFCSOUNDPRESSURELEVELMEASURE"); }
        Nullable<IfcSoundPressureLevelMeasure> get_IfcSoundPressureLevelMeasure() { return getSimpleValue<IfcSoundPressureLevelMeasure>("IFCSOUNDPRESSURELEVELMEASURE", sdaiREAL); }
        void put_IfcSoundPressureLevelMeasure(IfcSoundPressureLevelMeasure value) { putSimpleValue("IFCSOUNDPRESSURELEVELMEASURE", sdaiREAL, value); }

        bool is_IfcSoundPressureMeasure() { return IsADBType("IFCSOUNDPRESSUREMEASURE"); }
        Nullable<IfcSoundPressureMeasure> get_IfcSoundPressureMeasure() { return getSimpleValue<IfcSoundPressureMeasure>("IFCSOUNDPRESSUREMEASURE", sdaiREAL); }
        void put_IfcSoundPressureMeasure(IfcSoundPressureMeasure value) { putSimpleValue("IFCSOUNDPRESSUREMEASURE", sdaiREAL, value); }

        bool is_IfcSpecificHeatCapacityMeasure() { return IsADBType("IFCSPECIFICHEATCAPACITYMEASURE"); }
        Nullable<IfcSpecificHeatCapacityMeasure> get_IfcSpecificHeatCapacityMeasure() { return getSimpleValue<IfcSpecificHeatCapacityMeasure>("IFCSPECIFICHEATCAPACITYMEASURE", sdaiREAL); }
        void put_IfcSpecificHeatCapacityMeasure(IfcSpecificHeatCapacityMeasure value) { putSimpleValue("IFCSPECIFICHEATCAPACITYMEASURE", sdaiREAL, value); }

        bool is_IfcTemperatureGradientMeasure() { return IsADBType("IFCTEMPERATUREGRADIENTMEASURE"); }
        Nullable<IfcTemperatureGradientMeasure> get_IfcTemperatureGradientMeasure() { return getSimpleValue<IfcTemperatureGradientMeasure>("IFCTEMPERATUREGRADIENTMEASURE", sdaiREAL); }
        void put_IfcTemperatureGradientMeasure(IfcTemperatureGradientMeasure value) { putSimpleValue("IFCTEMPERATUREGRADIENTMEASURE", sdaiREAL, value); }

        bool is_IfcTemperatureRateOfChangeMeasure() { return IsADBType("IFCTEMPERATURERATEOFCHANGEMEASURE"); }
        Nullable<IfcTemperatureRateOfChangeMeasure> get_IfcTemperatureRateOfChangeMeasure() { return getSimpleValue<IfcTemperatureRateOfChangeMeasure>("IFCTEMPERATURERATEOFCHANGEMEASURE", sdaiREAL); }
        void put_IfcTemperatureRateOfChangeMeasure(IfcTemperatureRateOfChangeMeasure value) { putSimpleValue("IFCTEMPERATURERATEOFCHANGEMEASURE", sdaiREAL, value); }

        bool is_IfcThermalAdmittanceMeasure() { return IsADBType("IFCTHERMALADMITTANCEMEASURE"); }
        Nullable<IfcThermalAdmittanceMeasure> get_IfcThermalAdmittanceMeasure() { return getSimpleValue<IfcThermalAdmittanceMeasure>("IFCTHERMALADMITTANCEMEASURE", sdaiREAL); }
        void put_IfcThermalAdmittanceMeasure(IfcThermalAdmittanceMeasure value) { putSimpleValue("IFCTHERMALADMITTANCEMEASURE", sdaiREAL, value); }

        bool is_IfcThermalConductivityMeasure() { return IsADBType("IFCTHERMALCONDUCTIVITYMEASURE"); }
        Nullable<IfcThermalConductivityMeasure> get_IfcThermalConductivityMeasure() { return getSimpleValue<IfcThermalConductivityMeasure>("IFCTHERMALCONDUCTIVITYMEASURE", sdaiREAL); }
        void put_IfcThermalConductivityMeasure(IfcThermalConductivityMeasure value) { putSimpleValue("IFCTHERMALCONDUCTIVITYMEASURE", sdaiREAL, value); }

        bool is_IfcThermalExpansionCoefficientMeasure() { return IsADBType("IFCTHERMALEXPANSIONCOEFFICIENTMEASURE"); }
        Nullable<IfcThermalExpansionCoefficientMeasure> get_IfcThermalExpansionCoefficientMeasure() { return getSimpleValue<IfcThermalExpansionCoefficientMeasure>("IFCTHERMALEXPANSIONCOEFFICIENTMEASURE", sdaiREAL); }
        void put_IfcThermalExpansionCoefficientMeasure(IfcThermalExpansionCoefficientMeasure value) { putSimpleValue("IFCTHERMALEXPANSIONCOEFFICIENTMEASURE", sdaiREAL, value); }

        bool is_IfcThermalResistanceMeasure() { return IsADBType("IFCTHERMALRESISTANCEMEASURE"); }
        Nullable<IfcThermalResistanceMeasure> get_IfcThermalResistanceMeasure() { return getSimpleValue<IfcThermalResistanceMeasure>("IFCTHERMALRESISTANCEMEASURE", sdaiREAL); }
        void put_IfcThermalResistanceMeasure(IfcThermalResistanceMeasure value) { putSimpleValue("IFCTHERMALRESISTANCEMEASURE", sdaiREAL, value); }

        bool is_IfcThermalTransmittanceMeasure() { return IsADBType("IFCTHERMALTRANSMITTANCEMEASURE"); }
        Nullable<IfcThermalTransmittanceMeasure> get_IfcThermalTransmittanceMeasure() { return getSimpleValue<IfcThermalTransmittanceMeasure>("IFCTHERMALTRANSMITTANCEMEASURE", sdaiREAL); }
        void put_IfcThermalTransmittanceMeasure(IfcThermalTransmittanceMeasure value) { putSimpleValue("IFCTHERMALTRANSMITTANCEMEASURE", sdaiREAL, value); }

        bool is_IfcTorqueMeasure() { return IsADBType("IFCTORQUEMEASURE"); }
        Nullable<IfcTorqueMeasure> get_IfcTorqueMeasure() { return getSimpleValue<IfcTorqueMeasure>("IFCTORQUEMEASURE", sdaiREAL); }
        void put_IfcTorqueMeasure(IfcTorqueMeasure value) { putSimpleValue("IFCTORQUEMEASURE", sdaiREAL, value); }

        bool is_IfcVaporPermeabilityMeasure() { return IsADBType("IFCVAPORPERMEABILITYMEASURE"); }
        Nullable<IfcVaporPermeabilityMeasure> get_IfcVaporPermeabilityMeasure() { return getSimpleValue<IfcVaporPermeabilityMeasure>("IFCVAPORPERMEABILITYMEASURE", sdaiREAL); }
        void put_IfcVaporPermeabilityMeasure(IfcVaporPermeabilityMeasure value) { putSimpleValue("IFCVAPORPERMEABILITYMEASURE", sdaiREAL, value); }

        bool is_IfcVolumetricFlowRateMeasure() { return IsADBType("IFCVOLUMETRICFLOWRATEMEASURE"); }
        Nullable<IfcVolumetricFlowRateMeasure> get_IfcVolumetricFlowRateMeasure() { return getSimpleValue<IfcVolumetricFlowRateMeasure>("IFCVOLUMETRICFLOWRATEMEASURE", sdaiREAL); }
        void put_IfcVolumetricFlowRateMeasure(IfcVolumetricFlowRateMeasure value) { putSimpleValue("IFCVOLUMETRICFLOWRATEMEASURE", sdaiREAL, value); }

        bool is_IfcWarpingConstantMeasure() { return IsADBType("IFCWARPINGCONSTANTMEASURE"); }
        Nullable<IfcWarpingConstantMeasure> get_IfcWarpingConstantMeasure() { return getSimpleValue<IfcWarpingConstantMeasure>("IFCWARPINGCONSTANTMEASURE", sdaiREAL); }
        void put_IfcWarpingConstantMeasure(IfcWarpingConstantMeasure value) { putSimpleValue("IFCWARPINGCONSTANTMEASURE", sdaiREAL, value); }

        bool is_IfcWarpingMomentMeasure() { return IsADBType("IFCWARPINGMOMENTMEASURE"); }
        Nullable<IfcWarpingMomentMeasure> get_IfcWarpingMomentMeasure() { return getSimpleValue<IfcWarpingMomentMeasure>("IFCWARPINGMOMENTMEASURE", sdaiREAL); }
        void put_IfcWarpingMomentMeasure(IfcWarpingMomentMeasure value) { putSimpleValue("IFCWARPINGMOMENTMEASURE", sdaiREAL, value); }
    };


    class IfcDerivedMeasureValue_get : public Select
    {
    public:
        IfcDerivedMeasureValue_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDerivedMeasureValue_get(Select* outer) : Select(outer) {}
        bool is_IfcAbsorbedDoseMeasure() { return IsADBType("IFCABSORBEDDOSEMEASURE"); }
        Nullable<IfcAbsorbedDoseMeasure> get_IfcAbsorbedDoseMeasure() { return getSimpleValue<IfcAbsorbedDoseMeasure>("IFCABSORBEDDOSEMEASURE", sdaiREAL); }
        bool is_IfcAccelerationMeasure() { return IsADBType("IFCACCELERATIONMEASURE"); }
        Nullable<IfcAccelerationMeasure> get_IfcAccelerationMeasure() { return getSimpleValue<IfcAccelerationMeasure>("IFCACCELERATIONMEASURE", sdaiREAL); }
        bool is_IfcAngularVelocityMeasure() { return IsADBType("IFCANGULARVELOCITYMEASURE"); }
        Nullable<IfcAngularVelocityMeasure> get_IfcAngularVelocityMeasure() { return getSimpleValue<IfcAngularVelocityMeasure>("IFCANGULARVELOCITYMEASURE", sdaiREAL); }
        bool is_IfcAreaDensityMeasure() { return IsADBType("IFCAREADENSITYMEASURE"); }
        Nullable<IfcAreaDensityMeasure> get_IfcAreaDensityMeasure() { return getSimpleValue<IfcAreaDensityMeasure>("IFCAREADENSITYMEASURE", sdaiREAL); }
        bool is_IfcCompoundPlaneAngleMeasure() { return IsADBType("IFCCOMPOUNDPLANEANGLEMEASURE"); }

        //TList may be IfcCompoundPlaneAngleMeasure or list of converible elements
        template <typename TList> void get_IfcCompoundPlaneAngleMeasure(TList& lst) { SdaiAggr aggr = getAggrValue("IFCCOMPOUNDPLANEANGLEMEASURE"); IfcCompoundPlaneAngleMeasureSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }
        bool is_IfcCurvatureMeasure() { return IsADBType("IFCCURVATUREMEASURE"); }
        Nullable<IfcCurvatureMeasure> get_IfcCurvatureMeasure() { return getSimpleValue<IfcCurvatureMeasure>("IFCCURVATUREMEASURE", sdaiREAL); }
        bool is_IfcDoseEquivalentMeasure() { return IsADBType("IFCDOSEEQUIVALENTMEASURE"); }
        Nullable<IfcDoseEquivalentMeasure> get_IfcDoseEquivalentMeasure() { return getSimpleValue<IfcDoseEquivalentMeasure>("IFCDOSEEQUIVALENTMEASURE", sdaiREAL); }
        bool is_IfcDynamicViscosityMeasure() { return IsADBType("IFCDYNAMICVISCOSITYMEASURE"); }
        Nullable<IfcDynamicViscosityMeasure> get_IfcDynamicViscosityMeasure() { return getSimpleValue<IfcDynamicViscosityMeasure>("IFCDYNAMICVISCOSITYMEASURE", sdaiREAL); }
        bool is_IfcElectricCapacitanceMeasure() { return IsADBType("IFCELECTRICCAPACITANCEMEASURE"); }
        Nullable<IfcElectricCapacitanceMeasure> get_IfcElectricCapacitanceMeasure() { return getSimpleValue<IfcElectricCapacitanceMeasure>("IFCELECTRICCAPACITANCEMEASURE", sdaiREAL); }
        bool is_IfcElectricChargeMeasure() { return IsADBType("IFCELECTRICCHARGEMEASURE"); }
        Nullable<IfcElectricChargeMeasure> get_IfcElectricChargeMeasure() { return getSimpleValue<IfcElectricChargeMeasure>("IFCELECTRICCHARGEMEASURE", sdaiREAL); }
        bool is_IfcElectricConductanceMeasure() { return IsADBType("IFCELECTRICCONDUCTANCEMEASURE"); }
        Nullable<IfcElectricConductanceMeasure> get_IfcElectricConductanceMeasure() { return getSimpleValue<IfcElectricConductanceMeasure>("IFCELECTRICCONDUCTANCEMEASURE", sdaiREAL); }
        bool is_IfcElectricResistanceMeasure() { return IsADBType("IFCELECTRICRESISTANCEMEASURE"); }
        Nullable<IfcElectricResistanceMeasure> get_IfcElectricResistanceMeasure() { return getSimpleValue<IfcElectricResistanceMeasure>("IFCELECTRICRESISTANCEMEASURE", sdaiREAL); }
        bool is_IfcElectricVoltageMeasure() { return IsADBType("IFCELECTRICVOLTAGEMEASURE"); }
        Nullable<IfcElectricVoltageMeasure> get_IfcElectricVoltageMeasure() { return getSimpleValue<IfcElectricVoltageMeasure>("IFCELECTRICVOLTAGEMEASURE", sdaiREAL); }
        bool is_IfcEnergyMeasure() { return IsADBType("IFCENERGYMEASURE"); }
        Nullable<IfcEnergyMeasure> get_IfcEnergyMeasure() { return getSimpleValue<IfcEnergyMeasure>("IFCENERGYMEASURE", sdaiREAL); }
        bool is_IfcForceMeasure() { return IsADBType("IFCFORCEMEASURE"); }
        Nullable<IfcForceMeasure> get_IfcForceMeasure() { return getSimpleValue<IfcForceMeasure>("IFCFORCEMEASURE", sdaiREAL); }
        bool is_IfcFrequencyMeasure() { return IsADBType("IFCFREQUENCYMEASURE"); }
        Nullable<IfcFrequencyMeasure> get_IfcFrequencyMeasure() { return getSimpleValue<IfcFrequencyMeasure>("IFCFREQUENCYMEASURE", sdaiREAL); }
        bool is_IfcHeatFluxDensityMeasure() { return IsADBType("IFCHEATFLUXDENSITYMEASURE"); }
        Nullable<IfcHeatFluxDensityMeasure> get_IfcHeatFluxDensityMeasure() { return getSimpleValue<IfcHeatFluxDensityMeasure>("IFCHEATFLUXDENSITYMEASURE", sdaiREAL); }
        bool is_IfcHeatingValueMeasure() { return IsADBType("IFCHEATINGVALUEMEASURE"); }
        Nullable<IfcHeatingValueMeasure> get_IfcHeatingValueMeasure() { return getSimpleValue<IfcHeatingValueMeasure>("IFCHEATINGVALUEMEASURE", sdaiREAL); }
        bool is_IfcIlluminanceMeasure() { return IsADBType("IFCILLUMINANCEMEASURE"); }
        Nullable<IfcIlluminanceMeasure> get_IfcIlluminanceMeasure() { return getSimpleValue<IfcIlluminanceMeasure>("IFCILLUMINANCEMEASURE", sdaiREAL); }
        bool is_IfcInductanceMeasure() { return IsADBType("IFCINDUCTANCEMEASURE"); }
        Nullable<IfcInductanceMeasure> get_IfcInductanceMeasure() { return getSimpleValue<IfcInductanceMeasure>("IFCINDUCTANCEMEASURE", sdaiREAL); }
        bool is_IfcIntegerCountRateMeasure() { return IsADBType("IFCINTEGERCOUNTRATEMEASURE"); }
        Nullable<IfcIntegerCountRateMeasure> get_IfcIntegerCountRateMeasure() { return getSimpleValue<IfcIntegerCountRateMeasure>("IFCINTEGERCOUNTRATEMEASURE", sdaiINTEGER); }
        bool is_IfcIonConcentrationMeasure() { return IsADBType("IFCIONCONCENTRATIONMEASURE"); }
        Nullable<IfcIonConcentrationMeasure> get_IfcIonConcentrationMeasure() { return getSimpleValue<IfcIonConcentrationMeasure>("IFCIONCONCENTRATIONMEASURE", sdaiREAL); }
        bool is_IfcIsothermalMoistureCapacityMeasure() { return IsADBType("IFCISOTHERMALMOISTURECAPACITYMEASURE"); }
        Nullable<IfcIsothermalMoistureCapacityMeasure> get_IfcIsothermalMoistureCapacityMeasure() { return getSimpleValue<IfcIsothermalMoistureCapacityMeasure>("IFCISOTHERMALMOISTURECAPACITYMEASURE", sdaiREAL); }
        bool is_IfcKinematicViscosityMeasure() { return IsADBType("IFCKINEMATICVISCOSITYMEASURE"); }
        Nullable<IfcKinematicViscosityMeasure> get_IfcKinematicViscosityMeasure() { return getSimpleValue<IfcKinematicViscosityMeasure>("IFCKINEMATICVISCOSITYMEASURE", sdaiREAL); }
        bool is_IfcLinearForceMeasure() { return IsADBType("IFCLINEARFORCEMEASURE"); }
        Nullable<IfcLinearForceMeasure> get_IfcLinearForceMeasure() { return getSimpleValue<IfcLinearForceMeasure>("IFCLINEARFORCEMEASURE", sdaiREAL); }
        bool is_IfcLinearMomentMeasure() { return IsADBType("IFCLINEARMOMENTMEASURE"); }
        Nullable<IfcLinearMomentMeasure> get_IfcLinearMomentMeasure() { return getSimpleValue<IfcLinearMomentMeasure>("IFCLINEARMOMENTMEASURE", sdaiREAL); }
        bool is_IfcLinearStiffnessMeasure() { return IsADBType("IFCLINEARSTIFFNESSMEASURE"); }
        Nullable<IfcLinearStiffnessMeasure> get_IfcLinearStiffnessMeasure() { return getSimpleValue<IfcLinearStiffnessMeasure>("IFCLINEARSTIFFNESSMEASURE", sdaiREAL); }
        bool is_IfcLinearVelocityMeasure() { return IsADBType("IFCLINEARVELOCITYMEASURE"); }
        Nullable<IfcLinearVelocityMeasure> get_IfcLinearVelocityMeasure() { return getSimpleValue<IfcLinearVelocityMeasure>("IFCLINEARVELOCITYMEASURE", sdaiREAL); }
        bool is_IfcLuminousFluxMeasure() { return IsADBType("IFCLUMINOUSFLUXMEASURE"); }
        Nullable<IfcLuminousFluxMeasure> get_IfcLuminousFluxMeasure() { return getSimpleValue<IfcLuminousFluxMeasure>("IFCLUMINOUSFLUXMEASURE", sdaiREAL); }
        bool is_IfcLuminousIntensityDistributionMeasure() { return IsADBType("IFCLUMINOUSINTENSITYDISTRIBUTIONMEASURE"); }
        Nullable<IfcLuminousIntensityDistributionMeasure> get_IfcLuminousIntensityDistributionMeasure() { return getSimpleValue<IfcLuminousIntensityDistributionMeasure>("IFCLUMINOUSINTENSITYDISTRIBUTIONMEASURE", sdaiREAL); }
        bool is_IfcMagneticFluxDensityMeasure() { return IsADBType("IFCMAGNETICFLUXDENSITYMEASURE"); }
        Nullable<IfcMagneticFluxDensityMeasure> get_IfcMagneticFluxDensityMeasure() { return getSimpleValue<IfcMagneticFluxDensityMeasure>("IFCMAGNETICFLUXDENSITYMEASURE", sdaiREAL); }
        bool is_IfcMagneticFluxMeasure() { return IsADBType("IFCMAGNETICFLUXMEASURE"); }
        Nullable<IfcMagneticFluxMeasure> get_IfcMagneticFluxMeasure() { return getSimpleValue<IfcMagneticFluxMeasure>("IFCMAGNETICFLUXMEASURE", sdaiREAL); }
        bool is_IfcMassDensityMeasure() { return IsADBType("IFCMASSDENSITYMEASURE"); }
        Nullable<IfcMassDensityMeasure> get_IfcMassDensityMeasure() { return getSimpleValue<IfcMassDensityMeasure>("IFCMASSDENSITYMEASURE", sdaiREAL); }
        bool is_IfcMassFlowRateMeasure() { return IsADBType("IFCMASSFLOWRATEMEASURE"); }
        Nullable<IfcMassFlowRateMeasure> get_IfcMassFlowRateMeasure() { return getSimpleValue<IfcMassFlowRateMeasure>("IFCMASSFLOWRATEMEASURE", sdaiREAL); }
        bool is_IfcMassPerLengthMeasure() { return IsADBType("IFCMASSPERLENGTHMEASURE"); }
        Nullable<IfcMassPerLengthMeasure> get_IfcMassPerLengthMeasure() { return getSimpleValue<IfcMassPerLengthMeasure>("IFCMASSPERLENGTHMEASURE", sdaiREAL); }
        bool is_IfcModulusOfElasticityMeasure() { return IsADBType("IFCMODULUSOFELASTICITYMEASURE"); }
        Nullable<IfcModulusOfElasticityMeasure> get_IfcModulusOfElasticityMeasure() { return getSimpleValue<IfcModulusOfElasticityMeasure>("IFCMODULUSOFELASTICITYMEASURE", sdaiREAL); }
        bool is_IfcModulusOfLinearSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfLinearSubgradeReactionMeasure> get_IfcModulusOfLinearSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfLinearSubgradeReactionMeasure>("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE", sdaiREAL); }
        bool is_IfcModulusOfRotationalSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfRotationalSubgradeReactionMeasure> get_IfcModulusOfRotationalSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfRotationalSubgradeReactionMeasure>("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE", sdaiREAL); }
        bool is_IfcModulusOfSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfSubgradeReactionMeasure> get_IfcModulusOfSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfSubgradeReactionMeasure>("IFCMODULUSOFSUBGRADEREACTIONMEASURE", sdaiREAL); }
        bool is_IfcMoistureDiffusivityMeasure() { return IsADBType("IFCMOISTUREDIFFUSIVITYMEASURE"); }
        Nullable<IfcMoistureDiffusivityMeasure> get_IfcMoistureDiffusivityMeasure() { return getSimpleValue<IfcMoistureDiffusivityMeasure>("IFCMOISTUREDIFFUSIVITYMEASURE", sdaiREAL); }
        bool is_IfcMolecularWeightMeasure() { return IsADBType("IFCMOLECULARWEIGHTMEASURE"); }
        Nullable<IfcMolecularWeightMeasure> get_IfcMolecularWeightMeasure() { return getSimpleValue<IfcMolecularWeightMeasure>("IFCMOLECULARWEIGHTMEASURE", sdaiREAL); }
        bool is_IfcMomentOfInertiaMeasure() { return IsADBType("IFCMOMENTOFINERTIAMEASURE"); }
        Nullable<IfcMomentOfInertiaMeasure> get_IfcMomentOfInertiaMeasure() { return getSimpleValue<IfcMomentOfInertiaMeasure>("IFCMOMENTOFINERTIAMEASURE", sdaiREAL); }
        bool is_IfcMonetaryMeasure() { return IsADBType("IFCMONETARYMEASURE"); }
        Nullable<IfcMonetaryMeasure> get_IfcMonetaryMeasure() { return getSimpleValue<IfcMonetaryMeasure>("IFCMONETARYMEASURE", sdaiREAL); }
        bool is_IfcPHMeasure() { return IsADBType("IFCPHMEASURE"); }
        Nullable<IfcPHMeasure> get_IfcPHMeasure() { return getSimpleValue<IfcPHMeasure>("IFCPHMEASURE", sdaiREAL); }
        bool is_IfcPlanarForceMeasure() { return IsADBType("IFCPLANARFORCEMEASURE"); }
        Nullable<IfcPlanarForceMeasure> get_IfcPlanarForceMeasure() { return getSimpleValue<IfcPlanarForceMeasure>("IFCPLANARFORCEMEASURE", sdaiREAL); }
        bool is_IfcPowerMeasure() { return IsADBType("IFCPOWERMEASURE"); }
        Nullable<IfcPowerMeasure> get_IfcPowerMeasure() { return getSimpleValue<IfcPowerMeasure>("IFCPOWERMEASURE", sdaiREAL); }
        bool is_IfcPressureMeasure() { return IsADBType("IFCPRESSUREMEASURE"); }
        Nullable<IfcPressureMeasure> get_IfcPressureMeasure() { return getSimpleValue<IfcPressureMeasure>("IFCPRESSUREMEASURE", sdaiREAL); }
        bool is_IfcRadioActivityMeasure() { return IsADBType("IFCRADIOACTIVITYMEASURE"); }
        Nullable<IfcRadioActivityMeasure> get_IfcRadioActivityMeasure() { return getSimpleValue<IfcRadioActivityMeasure>("IFCRADIOACTIVITYMEASURE", sdaiREAL); }
        bool is_IfcRotationalFrequencyMeasure() { return IsADBType("IFCROTATIONALFREQUENCYMEASURE"); }
        Nullable<IfcRotationalFrequencyMeasure> get_IfcRotationalFrequencyMeasure() { return getSimpleValue<IfcRotationalFrequencyMeasure>("IFCROTATIONALFREQUENCYMEASURE", sdaiREAL); }
        bool is_IfcRotationalMassMeasure() { return IsADBType("IFCROTATIONALMASSMEASURE"); }
        Nullable<IfcRotationalMassMeasure> get_IfcRotationalMassMeasure() { return getSimpleValue<IfcRotationalMassMeasure>("IFCROTATIONALMASSMEASURE", sdaiREAL); }
        bool is_IfcRotationalStiffnessMeasure() { return IsADBType("IFCROTATIONALSTIFFNESSMEASURE"); }
        Nullable<IfcRotationalStiffnessMeasure> get_IfcRotationalStiffnessMeasure() { return getSimpleValue<IfcRotationalStiffnessMeasure>("IFCROTATIONALSTIFFNESSMEASURE", sdaiREAL); }
        bool is_IfcSectionModulusMeasure() { return IsADBType("IFCSECTIONMODULUSMEASURE"); }
        Nullable<IfcSectionModulusMeasure> get_IfcSectionModulusMeasure() { return getSimpleValue<IfcSectionModulusMeasure>("IFCSECTIONMODULUSMEASURE", sdaiREAL); }
        bool is_IfcSectionalAreaIntegralMeasure() { return IsADBType("IFCSECTIONALAREAINTEGRALMEASURE"); }
        Nullable<IfcSectionalAreaIntegralMeasure> get_IfcSectionalAreaIntegralMeasure() { return getSimpleValue<IfcSectionalAreaIntegralMeasure>("IFCSECTIONALAREAINTEGRALMEASURE", sdaiREAL); }
        bool is_IfcShearModulusMeasure() { return IsADBType("IFCSHEARMODULUSMEASURE"); }
        Nullable<IfcShearModulusMeasure> get_IfcShearModulusMeasure() { return getSimpleValue<IfcShearModulusMeasure>("IFCSHEARMODULUSMEASURE", sdaiREAL); }
        bool is_IfcSoundPowerLevelMeasure() { return IsADBType("IFCSOUNDPOWERLEVELMEASURE"); }
        Nullable<IfcSoundPowerLevelMeasure> get_IfcSoundPowerLevelMeasure() { return getSimpleValue<IfcSoundPowerLevelMeasure>("IFCSOUNDPOWERLEVELMEASURE", sdaiREAL); }
        bool is_IfcSoundPowerMeasure() { return IsADBType("IFCSOUNDPOWERMEASURE"); }
        Nullable<IfcSoundPowerMeasure> get_IfcSoundPowerMeasure() { return getSimpleValue<IfcSoundPowerMeasure>("IFCSOUNDPOWERMEASURE", sdaiREAL); }
        bool is_IfcSoundPressureLevelMeasure() { return IsADBType("IFCSOUNDPRESSURELEVELMEASURE"); }
        Nullable<IfcSoundPressureLevelMeasure> get_IfcSoundPressureLevelMeasure() { return getSimpleValue<IfcSoundPressureLevelMeasure>("IFCSOUNDPRESSURELEVELMEASURE", sdaiREAL); }
        bool is_IfcSoundPressureMeasure() { return IsADBType("IFCSOUNDPRESSUREMEASURE"); }
        Nullable<IfcSoundPressureMeasure> get_IfcSoundPressureMeasure() { return getSimpleValue<IfcSoundPressureMeasure>("IFCSOUNDPRESSUREMEASURE", sdaiREAL); }
        bool is_IfcSpecificHeatCapacityMeasure() { return IsADBType("IFCSPECIFICHEATCAPACITYMEASURE"); }
        Nullable<IfcSpecificHeatCapacityMeasure> get_IfcSpecificHeatCapacityMeasure() { return getSimpleValue<IfcSpecificHeatCapacityMeasure>("IFCSPECIFICHEATCAPACITYMEASURE", sdaiREAL); }
        bool is_IfcTemperatureGradientMeasure() { return IsADBType("IFCTEMPERATUREGRADIENTMEASURE"); }
        Nullable<IfcTemperatureGradientMeasure> get_IfcTemperatureGradientMeasure() { return getSimpleValue<IfcTemperatureGradientMeasure>("IFCTEMPERATUREGRADIENTMEASURE", sdaiREAL); }
        bool is_IfcTemperatureRateOfChangeMeasure() { return IsADBType("IFCTEMPERATURERATEOFCHANGEMEASURE"); }
        Nullable<IfcTemperatureRateOfChangeMeasure> get_IfcTemperatureRateOfChangeMeasure() { return getSimpleValue<IfcTemperatureRateOfChangeMeasure>("IFCTEMPERATURERATEOFCHANGEMEASURE", sdaiREAL); }
        bool is_IfcThermalAdmittanceMeasure() { return IsADBType("IFCTHERMALADMITTANCEMEASURE"); }
        Nullable<IfcThermalAdmittanceMeasure> get_IfcThermalAdmittanceMeasure() { return getSimpleValue<IfcThermalAdmittanceMeasure>("IFCTHERMALADMITTANCEMEASURE", sdaiREAL); }
        bool is_IfcThermalConductivityMeasure() { return IsADBType("IFCTHERMALCONDUCTIVITYMEASURE"); }
        Nullable<IfcThermalConductivityMeasure> get_IfcThermalConductivityMeasure() { return getSimpleValue<IfcThermalConductivityMeasure>("IFCTHERMALCONDUCTIVITYMEASURE", sdaiREAL); }
        bool is_IfcThermalExpansionCoefficientMeasure() { return IsADBType("IFCTHERMALEXPANSIONCOEFFICIENTMEASURE"); }
        Nullable<IfcThermalExpansionCoefficientMeasure> get_IfcThermalExpansionCoefficientMeasure() { return getSimpleValue<IfcThermalExpansionCoefficientMeasure>("IFCTHERMALEXPANSIONCOEFFICIENTMEASURE", sdaiREAL); }
        bool is_IfcThermalResistanceMeasure() { return IsADBType("IFCTHERMALRESISTANCEMEASURE"); }
        Nullable<IfcThermalResistanceMeasure> get_IfcThermalResistanceMeasure() { return getSimpleValue<IfcThermalResistanceMeasure>("IFCTHERMALRESISTANCEMEASURE", sdaiREAL); }
        bool is_IfcThermalTransmittanceMeasure() { return IsADBType("IFCTHERMALTRANSMITTANCEMEASURE"); }
        Nullable<IfcThermalTransmittanceMeasure> get_IfcThermalTransmittanceMeasure() { return getSimpleValue<IfcThermalTransmittanceMeasure>("IFCTHERMALTRANSMITTANCEMEASURE", sdaiREAL); }
        bool is_IfcTorqueMeasure() { return IsADBType("IFCTORQUEMEASURE"); }
        Nullable<IfcTorqueMeasure> get_IfcTorqueMeasure() { return getSimpleValue<IfcTorqueMeasure>("IFCTORQUEMEASURE", sdaiREAL); }
        bool is_IfcVaporPermeabilityMeasure() { return IsADBType("IFCVAPORPERMEABILITYMEASURE"); }
        Nullable<IfcVaporPermeabilityMeasure> get_IfcVaporPermeabilityMeasure() { return getSimpleValue<IfcVaporPermeabilityMeasure>("IFCVAPORPERMEABILITYMEASURE", sdaiREAL); }
        bool is_IfcVolumetricFlowRateMeasure() { return IsADBType("IFCVOLUMETRICFLOWRATEMEASURE"); }
        Nullable<IfcVolumetricFlowRateMeasure> get_IfcVolumetricFlowRateMeasure() { return getSimpleValue<IfcVolumetricFlowRateMeasure>("IFCVOLUMETRICFLOWRATEMEASURE", sdaiREAL); }
        bool is_IfcWarpingConstantMeasure() { return IsADBType("IFCWARPINGCONSTANTMEASURE"); }
        Nullable<IfcWarpingConstantMeasure> get_IfcWarpingConstantMeasure() { return getSimpleValue<IfcWarpingConstantMeasure>("IFCWARPINGCONSTANTMEASURE", sdaiREAL); }
        bool is_IfcWarpingMomentMeasure() { return IsADBType("IFCWARPINGMOMENTMEASURE"); }
        Nullable<IfcWarpingMomentMeasure> get_IfcWarpingMomentMeasure() { return getSimpleValue<IfcWarpingMomentMeasure>("IFCWARPINGMOMENTMEASURE", sdaiREAL); }

        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
        Nullable<IntValue> as_int() { IntValue val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiINTEGER, &val)) return val; else return Nullable<IntValue>(); }
    };


    class IfcDerivedMeasureValue_put : public Select
    {
    public:
        IfcDerivedMeasureValue_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDerivedMeasureValue_put(Select* outer) : Select(outer) {}
        void put_IfcAbsorbedDoseMeasure(IfcAbsorbedDoseMeasure value) { putSimpleValue("IFCABSORBEDDOSEMEASURE", sdaiREAL, value); }
        void put_IfcAccelerationMeasure(IfcAccelerationMeasure value) { putSimpleValue("IFCACCELERATIONMEASURE", sdaiREAL, value); }
        void put_IfcAngularVelocityMeasure(IfcAngularVelocityMeasure value) { putSimpleValue("IFCANGULARVELOCITYMEASURE", sdaiREAL, value); }
        void put_IfcAreaDensityMeasure(IfcAreaDensityMeasure value) { putSimpleValue("IFCAREADENSITYMEASURE", sdaiREAL, value); }

                //TList may be IfcCompoundPlaneAngleMeasure or list of converible elements
        template <typename TList> void put_IfcCompoundPlaneAngleMeasure(TList& lst) { IfcCompoundPlaneAngleMeasureSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCCOMPOUNDPLANEANGLEMEASURE", aggr); }

                //TArrayElem[] may be IntValue[] or array of convertible elements
        template <typename TArrayElem> void put_IfcCompoundPlaneAngleMeasure(TArrayElem arr[], size_t n) { IfcCompoundPlaneAngleMeasure lst; ArrayToList(arr, n, lst); put_IfcCompoundPlaneAngleMeasure(lst); }
        void put_IfcCurvatureMeasure(IfcCurvatureMeasure value) { putSimpleValue("IFCCURVATUREMEASURE", sdaiREAL, value); }
        void put_IfcDoseEquivalentMeasure(IfcDoseEquivalentMeasure value) { putSimpleValue("IFCDOSEEQUIVALENTMEASURE", sdaiREAL, value); }
        void put_IfcDynamicViscosityMeasure(IfcDynamicViscosityMeasure value) { putSimpleValue("IFCDYNAMICVISCOSITYMEASURE", sdaiREAL, value); }
        void put_IfcElectricCapacitanceMeasure(IfcElectricCapacitanceMeasure value) { putSimpleValue("IFCELECTRICCAPACITANCEMEASURE", sdaiREAL, value); }
        void put_IfcElectricChargeMeasure(IfcElectricChargeMeasure value) { putSimpleValue("IFCELECTRICCHARGEMEASURE", sdaiREAL, value); }
        void put_IfcElectricConductanceMeasure(IfcElectricConductanceMeasure value) { putSimpleValue("IFCELECTRICCONDUCTANCEMEASURE", sdaiREAL, value); }
        void put_IfcElectricResistanceMeasure(IfcElectricResistanceMeasure value) { putSimpleValue("IFCELECTRICRESISTANCEMEASURE", sdaiREAL, value); }
        void put_IfcElectricVoltageMeasure(IfcElectricVoltageMeasure value) { putSimpleValue("IFCELECTRICVOLTAGEMEASURE", sdaiREAL, value); }
        void put_IfcEnergyMeasure(IfcEnergyMeasure value) { putSimpleValue("IFCENERGYMEASURE", sdaiREAL, value); }
        void put_IfcForceMeasure(IfcForceMeasure value) { putSimpleValue("IFCFORCEMEASURE", sdaiREAL, value); }
        void put_IfcFrequencyMeasure(IfcFrequencyMeasure value) { putSimpleValue("IFCFREQUENCYMEASURE", sdaiREAL, value); }
        void put_IfcHeatFluxDensityMeasure(IfcHeatFluxDensityMeasure value) { putSimpleValue("IFCHEATFLUXDENSITYMEASURE", sdaiREAL, value); }
        void put_IfcHeatingValueMeasure(IfcHeatingValueMeasure value) { putSimpleValue("IFCHEATINGVALUEMEASURE", sdaiREAL, value); }
        void put_IfcIlluminanceMeasure(IfcIlluminanceMeasure value) { putSimpleValue("IFCILLUMINANCEMEASURE", sdaiREAL, value); }
        void put_IfcInductanceMeasure(IfcInductanceMeasure value) { putSimpleValue("IFCINDUCTANCEMEASURE", sdaiREAL, value); }
        void put_IfcIntegerCountRateMeasure(IfcIntegerCountRateMeasure value) { putSimpleValue("IFCINTEGERCOUNTRATEMEASURE", sdaiINTEGER, value); }
        void put_IfcIonConcentrationMeasure(IfcIonConcentrationMeasure value) { putSimpleValue("IFCIONCONCENTRATIONMEASURE", sdaiREAL, value); }
        void put_IfcIsothermalMoistureCapacityMeasure(IfcIsothermalMoistureCapacityMeasure value) { putSimpleValue("IFCISOTHERMALMOISTURECAPACITYMEASURE", sdaiREAL, value); }
        void put_IfcKinematicViscosityMeasure(IfcKinematicViscosityMeasure value) { putSimpleValue("IFCKINEMATICVISCOSITYMEASURE", sdaiREAL, value); }
        void put_IfcLinearForceMeasure(IfcLinearForceMeasure value) { putSimpleValue("IFCLINEARFORCEMEASURE", sdaiREAL, value); }
        void put_IfcLinearMomentMeasure(IfcLinearMomentMeasure value) { putSimpleValue("IFCLINEARMOMENTMEASURE", sdaiREAL, value); }
        void put_IfcLinearStiffnessMeasure(IfcLinearStiffnessMeasure value) { putSimpleValue("IFCLINEARSTIFFNESSMEASURE", sdaiREAL, value); }
        void put_IfcLinearVelocityMeasure(IfcLinearVelocityMeasure value) { putSimpleValue("IFCLINEARVELOCITYMEASURE", sdaiREAL, value); }
        void put_IfcLuminousFluxMeasure(IfcLuminousFluxMeasure value) { putSimpleValue("IFCLUMINOUSFLUXMEASURE", sdaiREAL, value); }
        void put_IfcLuminousIntensityDistributionMeasure(IfcLuminousIntensityDistributionMeasure value) { putSimpleValue("IFCLUMINOUSINTENSITYDISTRIBUTIONMEASURE", sdaiREAL, value); }
        void put_IfcMagneticFluxDensityMeasure(IfcMagneticFluxDensityMeasure value) { putSimpleValue("IFCMAGNETICFLUXDENSITYMEASURE", sdaiREAL, value); }
        void put_IfcMagneticFluxMeasure(IfcMagneticFluxMeasure value) { putSimpleValue("IFCMAGNETICFLUXMEASURE", sdaiREAL, value); }
        void put_IfcMassDensityMeasure(IfcMassDensityMeasure value) { putSimpleValue("IFCMASSDENSITYMEASURE", sdaiREAL, value); }
        void put_IfcMassFlowRateMeasure(IfcMassFlowRateMeasure value) { putSimpleValue("IFCMASSFLOWRATEMEASURE", sdaiREAL, value); }
        void put_IfcMassPerLengthMeasure(IfcMassPerLengthMeasure value) { putSimpleValue("IFCMASSPERLENGTHMEASURE", sdaiREAL, value); }
        void put_IfcModulusOfElasticityMeasure(IfcModulusOfElasticityMeasure value) { putSimpleValue("IFCMODULUSOFELASTICITYMEASURE", sdaiREAL, value); }
        void put_IfcModulusOfLinearSubgradeReactionMeasure(IfcModulusOfLinearSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
        void put_IfcModulusOfRotationalSubgradeReactionMeasure(IfcModulusOfRotationalSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
        void put_IfcModulusOfSubgradeReactionMeasure(IfcModulusOfSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
        void put_IfcMoistureDiffusivityMeasure(IfcMoistureDiffusivityMeasure value) { putSimpleValue("IFCMOISTUREDIFFUSIVITYMEASURE", sdaiREAL, value); }
        void put_IfcMolecularWeightMeasure(IfcMolecularWeightMeasure value) { putSimpleValue("IFCMOLECULARWEIGHTMEASURE", sdaiREAL, value); }
        void put_IfcMomentOfInertiaMeasure(IfcMomentOfInertiaMeasure value) { putSimpleValue("IFCMOMENTOFINERTIAMEASURE", sdaiREAL, value); }
        void put_IfcMonetaryMeasure(IfcMonetaryMeasure value) { putSimpleValue("IFCMONETARYMEASURE", sdaiREAL, value); }
        void put_IfcPHMeasure(IfcPHMeasure value) { putSimpleValue("IFCPHMEASURE", sdaiREAL, value); }
        void put_IfcPlanarForceMeasure(IfcPlanarForceMeasure value) { putSimpleValue("IFCPLANARFORCEMEASURE", sdaiREAL, value); }
        void put_IfcPowerMeasure(IfcPowerMeasure value) { putSimpleValue("IFCPOWERMEASURE", sdaiREAL, value); }
        void put_IfcPressureMeasure(IfcPressureMeasure value) { putSimpleValue("IFCPRESSUREMEASURE", sdaiREAL, value); }
        void put_IfcRadioActivityMeasure(IfcRadioActivityMeasure value) { putSimpleValue("IFCRADIOACTIVITYMEASURE", sdaiREAL, value); }
        void put_IfcRotationalFrequencyMeasure(IfcRotationalFrequencyMeasure value) { putSimpleValue("IFCROTATIONALFREQUENCYMEASURE", sdaiREAL, value); }
        void put_IfcRotationalMassMeasure(IfcRotationalMassMeasure value) { putSimpleValue("IFCROTATIONALMASSMEASURE", sdaiREAL, value); }
        void put_IfcRotationalStiffnessMeasure(IfcRotationalStiffnessMeasure value) { putSimpleValue("IFCROTATIONALSTIFFNESSMEASURE", sdaiREAL, value); }
        void put_IfcSectionModulusMeasure(IfcSectionModulusMeasure value) { putSimpleValue("IFCSECTIONMODULUSMEASURE", sdaiREAL, value); }
        void put_IfcSectionalAreaIntegralMeasure(IfcSectionalAreaIntegralMeasure value) { putSimpleValue("IFCSECTIONALAREAINTEGRALMEASURE", sdaiREAL, value); }
        void put_IfcShearModulusMeasure(IfcShearModulusMeasure value) { putSimpleValue("IFCSHEARMODULUSMEASURE", sdaiREAL, value); }
        void put_IfcSoundPowerLevelMeasure(IfcSoundPowerLevelMeasure value) { putSimpleValue("IFCSOUNDPOWERLEVELMEASURE", sdaiREAL, value); }
        void put_IfcSoundPowerMeasure(IfcSoundPowerMeasure value) { putSimpleValue("IFCSOUNDPOWERMEASURE", sdaiREAL, value); }
        void put_IfcSoundPressureLevelMeasure(IfcSoundPressureLevelMeasure value) { putSimpleValue("IFCSOUNDPRESSURELEVELMEASURE", sdaiREAL, value); }
        void put_IfcSoundPressureMeasure(IfcSoundPressureMeasure value) { putSimpleValue("IFCSOUNDPRESSUREMEASURE", sdaiREAL, value); }
        void put_IfcSpecificHeatCapacityMeasure(IfcSpecificHeatCapacityMeasure value) { putSimpleValue("IFCSPECIFICHEATCAPACITYMEASURE", sdaiREAL, value); }
        void put_IfcTemperatureGradientMeasure(IfcTemperatureGradientMeasure value) { putSimpleValue("IFCTEMPERATUREGRADIENTMEASURE", sdaiREAL, value); }
        void put_IfcTemperatureRateOfChangeMeasure(IfcTemperatureRateOfChangeMeasure value) { putSimpleValue("IFCTEMPERATURERATEOFCHANGEMEASURE", sdaiREAL, value); }
        void put_IfcThermalAdmittanceMeasure(IfcThermalAdmittanceMeasure value) { putSimpleValue("IFCTHERMALADMITTANCEMEASURE", sdaiREAL, value); }
        void put_IfcThermalConductivityMeasure(IfcThermalConductivityMeasure value) { putSimpleValue("IFCTHERMALCONDUCTIVITYMEASURE", sdaiREAL, value); }
        void put_IfcThermalExpansionCoefficientMeasure(IfcThermalExpansionCoefficientMeasure value) { putSimpleValue("IFCTHERMALEXPANSIONCOEFFICIENTMEASURE", sdaiREAL, value); }
        void put_IfcThermalResistanceMeasure(IfcThermalResistanceMeasure value) { putSimpleValue("IFCTHERMALRESISTANCEMEASURE", sdaiREAL, value); }
        void put_IfcThermalTransmittanceMeasure(IfcThermalTransmittanceMeasure value) { putSimpleValue("IFCTHERMALTRANSMITTANCEMEASURE", sdaiREAL, value); }
        void put_IfcTorqueMeasure(IfcTorqueMeasure value) { putSimpleValue("IFCTORQUEMEASURE", sdaiREAL, value); }
        void put_IfcVaporPermeabilityMeasure(IfcVaporPermeabilityMeasure value) { putSimpleValue("IFCVAPORPERMEABILITYMEASURE", sdaiREAL, value); }
        void put_IfcVolumetricFlowRateMeasure(IfcVolumetricFlowRateMeasure value) { putSimpleValue("IFCVOLUMETRICFLOWRATEMEASURE", sdaiREAL, value); }
        void put_IfcWarpingConstantMeasure(IfcWarpingConstantMeasure value) { putSimpleValue("IFCWARPINGCONSTANTMEASURE", sdaiREAL, value); }
        void put_IfcWarpingMomentMeasure(IfcWarpingMomentMeasure value) { putSimpleValue("IFCWARPINGMOMENTMEASURE", sdaiREAL, value); }
    };


    class IfcMeasureValue : public Select
    {
    public:
        IfcMeasureValue(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMeasureValue(Select* outer) : Select(outer) {}

        bool is_IfcAmountOfSubstanceMeasure() { return IsADBType("IFCAMOUNTOFSUBSTANCEMEASURE"); }
        Nullable<IfcAmountOfSubstanceMeasure> get_IfcAmountOfSubstanceMeasure() { return getSimpleValue<IfcAmountOfSubstanceMeasure>("IFCAMOUNTOFSUBSTANCEMEASURE", sdaiREAL); }
        void put_IfcAmountOfSubstanceMeasure(IfcAmountOfSubstanceMeasure value) { putSimpleValue("IFCAMOUNTOFSUBSTANCEMEASURE", sdaiREAL, value); }

        bool is_IfcAreaMeasure() { return IsADBType("IFCAREAMEASURE"); }
        Nullable<IfcAreaMeasure> get_IfcAreaMeasure() { return getSimpleValue<IfcAreaMeasure>("IFCAREAMEASURE", sdaiREAL); }
        void put_IfcAreaMeasure(IfcAreaMeasure value) { putSimpleValue("IFCAREAMEASURE", sdaiREAL, value); }

        bool is_IfcComplexNumber() { return IsADBType("IFCCOMPLEXNUMBER"); }

        //TList may be IfcComplexNumber or list of converible elements
        template <typename TList> void get_IfcComplexNumber(TList& lst) { SdaiAggr aggr = getAggrValue("IFCCOMPLEXNUMBER"); IfcComplexNumberSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }

                //TList may be IfcComplexNumber or list of converible elements
        template <typename TList> void put_IfcComplexNumber(TList& lst) { IfcComplexNumberSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCCOMPLEXNUMBER", aggr); }

                //TArrayElem[] may be double[] or array of convertible elements
        template <typename TArrayElem> void put_IfcComplexNumber(TArrayElem arr[], size_t n) { IfcComplexNumber lst; ArrayToList(arr, n, lst); put_IfcComplexNumber(lst); }

        bool is_IfcContextDependentMeasure() { return IsADBType("IFCCONTEXTDEPENDENTMEASURE"); }
        Nullable<IfcContextDependentMeasure> get_IfcContextDependentMeasure() { return getSimpleValue<IfcContextDependentMeasure>("IFCCONTEXTDEPENDENTMEASURE", sdaiREAL); }
        void put_IfcContextDependentMeasure(IfcContextDependentMeasure value) { putSimpleValue("IFCCONTEXTDEPENDENTMEASURE", sdaiREAL, value); }

        bool is_IfcCountMeasure() { return IsADBType("IFCCOUNTMEASURE"); }
        Nullable<IfcCountMeasure> get_IfcCountMeasure() { return getSimpleValue<IfcCountMeasure>("IFCCOUNTMEASURE", sdaiREAL); }
        void put_IfcCountMeasure(IfcCountMeasure value) { putSimpleValue("IFCCOUNTMEASURE", sdaiREAL, value); }

        bool is_IfcDescriptiveMeasure() { return IsADBType("IFCDESCRIPTIVEMEASURE"); }
        IfcDescriptiveMeasure get_IfcDescriptiveMeasure() { return getTextValue("IFCDESCRIPTIVEMEASURE", sdaiSTRING); }
        void put_IfcDescriptiveMeasure(IfcDescriptiveMeasure value) { putTextValue("IFCDESCRIPTIVEMEASURE", sdaiSTRING, value); }

        bool is_IfcElectricCurrentMeasure() { return IsADBType("IFCELECTRICCURRENTMEASURE"); }
        Nullable<IfcElectricCurrentMeasure> get_IfcElectricCurrentMeasure() { return getSimpleValue<IfcElectricCurrentMeasure>("IFCELECTRICCURRENTMEASURE", sdaiREAL); }
        void put_IfcElectricCurrentMeasure(IfcElectricCurrentMeasure value) { putSimpleValue("IFCELECTRICCURRENTMEASURE", sdaiREAL, value); }

        bool is_IfcLengthMeasure() { return IsADBType("IFCLENGTHMEASURE"); }
        Nullable<IfcLengthMeasure> get_IfcLengthMeasure() { return getSimpleValue<IfcLengthMeasure>("IFCLENGTHMEASURE", sdaiREAL); }
        void put_IfcLengthMeasure(IfcLengthMeasure value) { putSimpleValue("IFCLENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcLuminousIntensityMeasure() { return IsADBType("IFCLUMINOUSINTENSITYMEASURE"); }
        Nullable<IfcLuminousIntensityMeasure> get_IfcLuminousIntensityMeasure() { return getSimpleValue<IfcLuminousIntensityMeasure>("IFCLUMINOUSINTENSITYMEASURE", sdaiREAL); }
        void put_IfcLuminousIntensityMeasure(IfcLuminousIntensityMeasure value) { putSimpleValue("IFCLUMINOUSINTENSITYMEASURE", sdaiREAL, value); }

        bool is_IfcMassMeasure() { return IsADBType("IFCMASSMEASURE"); }
        Nullable<IfcMassMeasure> get_IfcMassMeasure() { return getSimpleValue<IfcMassMeasure>("IFCMASSMEASURE", sdaiREAL); }
        void put_IfcMassMeasure(IfcMassMeasure value) { putSimpleValue("IFCMASSMEASURE", sdaiREAL, value); }

        bool is_IfcNonNegativeLengthMeasure() { return IsADBType("IFCNONNEGATIVELENGTHMEASURE"); }
        Nullable<IfcNonNegativeLengthMeasure> get_IfcNonNegativeLengthMeasure() { return getSimpleValue<IfcNonNegativeLengthMeasure>("IFCNONNEGATIVELENGTHMEASURE", sdaiREAL); }
        void put_IfcNonNegativeLengthMeasure(IfcNonNegativeLengthMeasure value) { putSimpleValue("IFCNONNEGATIVELENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcNormalisedRatioMeasure() { return IsADBType("IFCNORMALISEDRATIOMEASURE"); }
        Nullable<IfcNormalisedRatioMeasure> get_IfcNormalisedRatioMeasure() { return getSimpleValue<IfcNormalisedRatioMeasure>("IFCNORMALISEDRATIOMEASURE", sdaiREAL); }
        void put_IfcNormalisedRatioMeasure(IfcNormalisedRatioMeasure value) { putSimpleValue("IFCNORMALISEDRATIOMEASURE", sdaiREAL, value); }

        bool is_IfcNumericMeasure() { return IsADBType("IFCNUMERICMEASURE"); }
        Nullable<IfcNumericMeasure> get_IfcNumericMeasure() { return getSimpleValue<IfcNumericMeasure>("IFCNUMERICMEASURE", sdaiREAL); }
        void put_IfcNumericMeasure(IfcNumericMeasure value) { putSimpleValue("IFCNUMERICMEASURE", sdaiREAL, value); }

        bool is_IfcParameterValue() { return IsADBType("IFCPARAMETERVALUE"); }
        Nullable<IfcParameterValue> get_IfcParameterValue() { return getSimpleValue<IfcParameterValue>("IFCPARAMETERVALUE", sdaiREAL); }
        void put_IfcParameterValue(IfcParameterValue value) { putSimpleValue("IFCPARAMETERVALUE", sdaiREAL, value); }

        bool is_IfcPlaneAngleMeasure() { return IsADBType("IFCPLANEANGLEMEASURE"); }
        Nullable<IfcPlaneAngleMeasure> get_IfcPlaneAngleMeasure() { return getSimpleValue<IfcPlaneAngleMeasure>("IFCPLANEANGLEMEASURE", sdaiREAL); }
        void put_IfcPlaneAngleMeasure(IfcPlaneAngleMeasure value) { putSimpleValue("IFCPLANEANGLEMEASURE", sdaiREAL, value); }

        bool is_IfcPositiveLengthMeasure() { return IsADBType("IFCPOSITIVELENGTHMEASURE"); }
        Nullable<IfcPositiveLengthMeasure> get_IfcPositiveLengthMeasure() { return getSimpleValue<IfcPositiveLengthMeasure>("IFCPOSITIVELENGTHMEASURE", sdaiREAL); }
        void put_IfcPositiveLengthMeasure(IfcPositiveLengthMeasure value) { putSimpleValue("IFCPOSITIVELENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcPositivePlaneAngleMeasure() { return IsADBType("IFCPOSITIVEPLANEANGLEMEASURE"); }
        Nullable<IfcPositivePlaneAngleMeasure> get_IfcPositivePlaneAngleMeasure() { return getSimpleValue<IfcPositivePlaneAngleMeasure>("IFCPOSITIVEPLANEANGLEMEASURE", sdaiREAL); }
        void put_IfcPositivePlaneAngleMeasure(IfcPositivePlaneAngleMeasure value) { putSimpleValue("IFCPOSITIVEPLANEANGLEMEASURE", sdaiREAL, value); }

        bool is_IfcPositiveRatioMeasure() { return IsADBType("IFCPOSITIVERATIOMEASURE"); }
        Nullable<IfcPositiveRatioMeasure> get_IfcPositiveRatioMeasure() { return getSimpleValue<IfcPositiveRatioMeasure>("IFCPOSITIVERATIOMEASURE", sdaiREAL); }
        void put_IfcPositiveRatioMeasure(IfcPositiveRatioMeasure value) { putSimpleValue("IFCPOSITIVERATIOMEASURE", sdaiREAL, value); }

        bool is_IfcRatioMeasure() { return IsADBType("IFCRATIOMEASURE"); }
        Nullable<IfcRatioMeasure> get_IfcRatioMeasure() { return getSimpleValue<IfcRatioMeasure>("IFCRATIOMEASURE", sdaiREAL); }
        void put_IfcRatioMeasure(IfcRatioMeasure value) { putSimpleValue("IFCRATIOMEASURE", sdaiREAL, value); }

        bool is_IfcSolidAngleMeasure() { return IsADBType("IFCSOLIDANGLEMEASURE"); }
        Nullable<IfcSolidAngleMeasure> get_IfcSolidAngleMeasure() { return getSimpleValue<IfcSolidAngleMeasure>("IFCSOLIDANGLEMEASURE", sdaiREAL); }
        void put_IfcSolidAngleMeasure(IfcSolidAngleMeasure value) { putSimpleValue("IFCSOLIDANGLEMEASURE", sdaiREAL, value); }

        bool is_IfcThermodynamicTemperatureMeasure() { return IsADBType("IFCTHERMODYNAMICTEMPERATUREMEASURE"); }
        Nullable<IfcThermodynamicTemperatureMeasure> get_IfcThermodynamicTemperatureMeasure() { return getSimpleValue<IfcThermodynamicTemperatureMeasure>("IFCTHERMODYNAMICTEMPERATUREMEASURE", sdaiREAL); }
        void put_IfcThermodynamicTemperatureMeasure(IfcThermodynamicTemperatureMeasure value) { putSimpleValue("IFCTHERMODYNAMICTEMPERATUREMEASURE", sdaiREAL, value); }

        bool is_IfcTimeMeasure() { return IsADBType("IFCTIMEMEASURE"); }
        Nullable<IfcTimeMeasure> get_IfcTimeMeasure() { return getSimpleValue<IfcTimeMeasure>("IFCTIMEMEASURE", sdaiREAL); }
        void put_IfcTimeMeasure(IfcTimeMeasure value) { putSimpleValue("IFCTIMEMEASURE", sdaiREAL, value); }

        bool is_IfcVolumeMeasure() { return IsADBType("IFCVOLUMEMEASURE"); }
        Nullable<IfcVolumeMeasure> get_IfcVolumeMeasure() { return getSimpleValue<IfcVolumeMeasure>("IFCVOLUMEMEASURE", sdaiREAL); }
        void put_IfcVolumeMeasure(IfcVolumeMeasure value) { putSimpleValue("IFCVOLUMEMEASURE", sdaiREAL, value); }
    };


    class IfcMeasureValue_get : public Select
    {
    public:
        IfcMeasureValue_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMeasureValue_get(Select* outer) : Select(outer) {}
        bool is_IfcAmountOfSubstanceMeasure() { return IsADBType("IFCAMOUNTOFSUBSTANCEMEASURE"); }
        Nullable<IfcAmountOfSubstanceMeasure> get_IfcAmountOfSubstanceMeasure() { return getSimpleValue<IfcAmountOfSubstanceMeasure>("IFCAMOUNTOFSUBSTANCEMEASURE", sdaiREAL); }
        bool is_IfcAreaMeasure() { return IsADBType("IFCAREAMEASURE"); }
        Nullable<IfcAreaMeasure> get_IfcAreaMeasure() { return getSimpleValue<IfcAreaMeasure>("IFCAREAMEASURE", sdaiREAL); }
        bool is_IfcComplexNumber() { return IsADBType("IFCCOMPLEXNUMBER"); }

        //TList may be IfcComplexNumber or list of converible elements
        template <typename TList> void get_IfcComplexNumber(TList& lst) { SdaiAggr aggr = getAggrValue("IFCCOMPLEXNUMBER"); IfcComplexNumberSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }
        bool is_IfcContextDependentMeasure() { return IsADBType("IFCCONTEXTDEPENDENTMEASURE"); }
        Nullable<IfcContextDependentMeasure> get_IfcContextDependentMeasure() { return getSimpleValue<IfcContextDependentMeasure>("IFCCONTEXTDEPENDENTMEASURE", sdaiREAL); }
        bool is_IfcCountMeasure() { return IsADBType("IFCCOUNTMEASURE"); }
        Nullable<IfcCountMeasure> get_IfcCountMeasure() { return getSimpleValue<IfcCountMeasure>("IFCCOUNTMEASURE", sdaiREAL); }
        bool is_IfcDescriptiveMeasure() { return IsADBType("IFCDESCRIPTIVEMEASURE"); }
        IfcDescriptiveMeasure get_IfcDescriptiveMeasure() { return getTextValue("IFCDESCRIPTIVEMEASURE", sdaiSTRING); }
        bool is_IfcElectricCurrentMeasure() { return IsADBType("IFCELECTRICCURRENTMEASURE"); }
        Nullable<IfcElectricCurrentMeasure> get_IfcElectricCurrentMeasure() { return getSimpleValue<IfcElectricCurrentMeasure>("IFCELECTRICCURRENTMEASURE", sdaiREAL); }
        bool is_IfcLengthMeasure() { return IsADBType("IFCLENGTHMEASURE"); }
        Nullable<IfcLengthMeasure> get_IfcLengthMeasure() { return getSimpleValue<IfcLengthMeasure>("IFCLENGTHMEASURE", sdaiREAL); }
        bool is_IfcLuminousIntensityMeasure() { return IsADBType("IFCLUMINOUSINTENSITYMEASURE"); }
        Nullable<IfcLuminousIntensityMeasure> get_IfcLuminousIntensityMeasure() { return getSimpleValue<IfcLuminousIntensityMeasure>("IFCLUMINOUSINTENSITYMEASURE", sdaiREAL); }
        bool is_IfcMassMeasure() { return IsADBType("IFCMASSMEASURE"); }
        Nullable<IfcMassMeasure> get_IfcMassMeasure() { return getSimpleValue<IfcMassMeasure>("IFCMASSMEASURE", sdaiREAL); }
        bool is_IfcNonNegativeLengthMeasure() { return IsADBType("IFCNONNEGATIVELENGTHMEASURE"); }
        Nullable<IfcNonNegativeLengthMeasure> get_IfcNonNegativeLengthMeasure() { return getSimpleValue<IfcNonNegativeLengthMeasure>("IFCNONNEGATIVELENGTHMEASURE", sdaiREAL); }
        bool is_IfcNormalisedRatioMeasure() { return IsADBType("IFCNORMALISEDRATIOMEASURE"); }
        Nullable<IfcNormalisedRatioMeasure> get_IfcNormalisedRatioMeasure() { return getSimpleValue<IfcNormalisedRatioMeasure>("IFCNORMALISEDRATIOMEASURE", sdaiREAL); }
        bool is_IfcNumericMeasure() { return IsADBType("IFCNUMERICMEASURE"); }
        Nullable<IfcNumericMeasure> get_IfcNumericMeasure() { return getSimpleValue<IfcNumericMeasure>("IFCNUMERICMEASURE", sdaiREAL); }
        bool is_IfcParameterValue() { return IsADBType("IFCPARAMETERVALUE"); }
        Nullable<IfcParameterValue> get_IfcParameterValue() { return getSimpleValue<IfcParameterValue>("IFCPARAMETERVALUE", sdaiREAL); }
        bool is_IfcPlaneAngleMeasure() { return IsADBType("IFCPLANEANGLEMEASURE"); }
        Nullable<IfcPlaneAngleMeasure> get_IfcPlaneAngleMeasure() { return getSimpleValue<IfcPlaneAngleMeasure>("IFCPLANEANGLEMEASURE", sdaiREAL); }
        bool is_IfcPositiveLengthMeasure() { return IsADBType("IFCPOSITIVELENGTHMEASURE"); }
        Nullable<IfcPositiveLengthMeasure> get_IfcPositiveLengthMeasure() { return getSimpleValue<IfcPositiveLengthMeasure>("IFCPOSITIVELENGTHMEASURE", sdaiREAL); }
        bool is_IfcPositivePlaneAngleMeasure() { return IsADBType("IFCPOSITIVEPLANEANGLEMEASURE"); }
        Nullable<IfcPositivePlaneAngleMeasure> get_IfcPositivePlaneAngleMeasure() { return getSimpleValue<IfcPositivePlaneAngleMeasure>("IFCPOSITIVEPLANEANGLEMEASURE", sdaiREAL); }
        bool is_IfcPositiveRatioMeasure() { return IsADBType("IFCPOSITIVERATIOMEASURE"); }
        Nullable<IfcPositiveRatioMeasure> get_IfcPositiveRatioMeasure() { return getSimpleValue<IfcPositiveRatioMeasure>("IFCPOSITIVERATIOMEASURE", sdaiREAL); }
        bool is_IfcRatioMeasure() { return IsADBType("IFCRATIOMEASURE"); }
        Nullable<IfcRatioMeasure> get_IfcRatioMeasure() { return getSimpleValue<IfcRatioMeasure>("IFCRATIOMEASURE", sdaiREAL); }
        bool is_IfcSolidAngleMeasure() { return IsADBType("IFCSOLIDANGLEMEASURE"); }
        Nullable<IfcSolidAngleMeasure> get_IfcSolidAngleMeasure() { return getSimpleValue<IfcSolidAngleMeasure>("IFCSOLIDANGLEMEASURE", sdaiREAL); }
        bool is_IfcThermodynamicTemperatureMeasure() { return IsADBType("IFCTHERMODYNAMICTEMPERATUREMEASURE"); }
        Nullable<IfcThermodynamicTemperatureMeasure> get_IfcThermodynamicTemperatureMeasure() { return getSimpleValue<IfcThermodynamicTemperatureMeasure>("IFCTHERMODYNAMICTEMPERATUREMEASURE", sdaiREAL); }
        bool is_IfcTimeMeasure() { return IsADBType("IFCTIMEMEASURE"); }
        Nullable<IfcTimeMeasure> get_IfcTimeMeasure() { return getSimpleValue<IfcTimeMeasure>("IFCTIMEMEASURE", sdaiREAL); }
        bool is_IfcVolumeMeasure() { return IsADBType("IFCVOLUMEMEASURE"); }
        Nullable<IfcVolumeMeasure> get_IfcVolumeMeasure() { return getSimpleValue<IfcVolumeMeasure>("IFCVOLUMEMEASURE", sdaiREAL); }

        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
        TextValue as_text() { TextValue val = NULL; sdaiGetAttrBN(m_instance, m_attrName, sdaiSTRING, &val); return val; }
    };


    class IfcMeasureValue_put : public Select
    {
    public:
        IfcMeasureValue_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMeasureValue_put(Select* outer) : Select(outer) {}
        void put_IfcAmountOfSubstanceMeasure(IfcAmountOfSubstanceMeasure value) { putSimpleValue("IFCAMOUNTOFSUBSTANCEMEASURE", sdaiREAL, value); }
        void put_IfcAreaMeasure(IfcAreaMeasure value) { putSimpleValue("IFCAREAMEASURE", sdaiREAL, value); }

                //TList may be IfcComplexNumber or list of converible elements
        template <typename TList> void put_IfcComplexNumber(TList& lst) { IfcComplexNumberSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCCOMPLEXNUMBER", aggr); }

                //TArrayElem[] may be double[] or array of convertible elements
        template <typename TArrayElem> void put_IfcComplexNumber(TArrayElem arr[], size_t n) { IfcComplexNumber lst; ArrayToList(arr, n, lst); put_IfcComplexNumber(lst); }
        void put_IfcContextDependentMeasure(IfcContextDependentMeasure value) { putSimpleValue("IFCCONTEXTDEPENDENTMEASURE", sdaiREAL, value); }
        void put_IfcCountMeasure(IfcCountMeasure value) { putSimpleValue("IFCCOUNTMEASURE", sdaiREAL, value); }
        void put_IfcDescriptiveMeasure(IfcDescriptiveMeasure value) { putTextValue("IFCDESCRIPTIVEMEASURE", sdaiSTRING, value); }
        void put_IfcElectricCurrentMeasure(IfcElectricCurrentMeasure value) { putSimpleValue("IFCELECTRICCURRENTMEASURE", sdaiREAL, value); }
        void put_IfcLengthMeasure(IfcLengthMeasure value) { putSimpleValue("IFCLENGTHMEASURE", sdaiREAL, value); }
        void put_IfcLuminousIntensityMeasure(IfcLuminousIntensityMeasure value) { putSimpleValue("IFCLUMINOUSINTENSITYMEASURE", sdaiREAL, value); }
        void put_IfcMassMeasure(IfcMassMeasure value) { putSimpleValue("IFCMASSMEASURE", sdaiREAL, value); }
        void put_IfcNonNegativeLengthMeasure(IfcNonNegativeLengthMeasure value) { putSimpleValue("IFCNONNEGATIVELENGTHMEASURE", sdaiREAL, value); }
        void put_IfcNormalisedRatioMeasure(IfcNormalisedRatioMeasure value) { putSimpleValue("IFCNORMALISEDRATIOMEASURE", sdaiREAL, value); }
        void put_IfcNumericMeasure(IfcNumericMeasure value) { putSimpleValue("IFCNUMERICMEASURE", sdaiREAL, value); }
        void put_IfcParameterValue(IfcParameterValue value) { putSimpleValue("IFCPARAMETERVALUE", sdaiREAL, value); }
        void put_IfcPlaneAngleMeasure(IfcPlaneAngleMeasure value) { putSimpleValue("IFCPLANEANGLEMEASURE", sdaiREAL, value); }
        void put_IfcPositiveLengthMeasure(IfcPositiveLengthMeasure value) { putSimpleValue("IFCPOSITIVELENGTHMEASURE", sdaiREAL, value); }
        void put_IfcPositivePlaneAngleMeasure(IfcPositivePlaneAngleMeasure value) { putSimpleValue("IFCPOSITIVEPLANEANGLEMEASURE", sdaiREAL, value); }
        void put_IfcPositiveRatioMeasure(IfcPositiveRatioMeasure value) { putSimpleValue("IFCPOSITIVERATIOMEASURE", sdaiREAL, value); }
        void put_IfcRatioMeasure(IfcRatioMeasure value) { putSimpleValue("IFCRATIOMEASURE", sdaiREAL, value); }
        void put_IfcSolidAngleMeasure(IfcSolidAngleMeasure value) { putSimpleValue("IFCSOLIDANGLEMEASURE", sdaiREAL, value); }
        void put_IfcThermodynamicTemperatureMeasure(IfcThermodynamicTemperatureMeasure value) { putSimpleValue("IFCTHERMODYNAMICTEMPERATUREMEASURE", sdaiREAL, value); }
        void put_IfcTimeMeasure(IfcTimeMeasure value) { putSimpleValue("IFCTIMEMEASURE", sdaiREAL, value); }
        void put_IfcVolumeMeasure(IfcVolumeMeasure value) { putSimpleValue("IFCVOLUMEMEASURE", sdaiREAL, value); }
    };


    class IfcSimpleValue : public Select
    {
    public:
        IfcSimpleValue(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSimpleValue(Select* outer) : Select(outer) {}

        bool is_IfcBinary() { return IsADBType("IFCBINARY"); }
        IfcBinary get_IfcBinary() { return getTextValue("IFCBINARY", sdaiBINARY); }
        void put_IfcBinary(IfcBinary value) { putTextValue("IFCBINARY", sdaiBINARY, value); }

        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }

        bool is_IfcDate() { return IsADBType("IFCDATE"); }
        IfcDate get_IfcDate() { return getTextValue("IFCDATE", sdaiSTRING); }
        void put_IfcDate(IfcDate value) { putTextValue("IFCDATE", sdaiSTRING, value); }

        bool is_IfcDateTime() { return IsADBType("IFCDATETIME"); }
        IfcDateTime get_IfcDateTime() { return getTextValue("IFCDATETIME", sdaiSTRING); }
        void put_IfcDateTime(IfcDateTime value) { putTextValue("IFCDATETIME", sdaiSTRING, value); }

        bool is_IfcDuration() { return IsADBType("IFCDURATION"); }
        IfcDuration get_IfcDuration() { return getTextValue("IFCDURATION", sdaiSTRING); }
        void put_IfcDuration(IfcDuration value) { putTextValue("IFCDURATION", sdaiSTRING, value); }

        bool is_IfcIdentifier() { return IsADBType("IFCIDENTIFIER"); }
        IfcIdentifier get_IfcIdentifier() { return getTextValue("IFCIDENTIFIER", sdaiSTRING); }
        void put_IfcIdentifier(IfcIdentifier value) { putTextValue("IFCIDENTIFIER", sdaiSTRING, value); }

        bool is_IfcInteger() { return IsADBType("IFCINTEGER"); }
        Nullable<IfcInteger> get_IfcInteger() { return getSimpleValue<IfcInteger>("IFCINTEGER", sdaiINTEGER); }
        void put_IfcInteger(IfcInteger value) { putSimpleValue("IFCINTEGER", sdaiINTEGER, value); }

        bool is_IfcLabel() { return IsADBType("IFCLABEL"); }
        IfcLabel get_IfcLabel() { return getTextValue("IFCLABEL", sdaiSTRING); }
        void put_IfcLabel(IfcLabel value) { putTextValue("IFCLABEL", sdaiSTRING, value); }

        bool is_IfcLogical() { return IsADBType("IFCLOGICAL"); }
        Nullable<IfcLogical> get_IfcLogical() { int v = getEnumerationValue("IFCLOGICAL", LOGICAL_VALUE_); if (v >= 0) return (IfcLogical) v; else return Nullable<IfcLogical>(); }
        void put_IfcLogical(IfcLogical value) { TextValue val = LOGICAL_VALUE_[(int) value]; putEnumerationValue("IFCLOGICAL", val); }

        bool is_IfcPositiveInteger() { return IsADBType("IFCPOSITIVEINTEGER"); }
        Nullable<IfcPositiveInteger> get_IfcPositiveInteger() { return getSimpleValue<IfcPositiveInteger>("IFCPOSITIVEINTEGER", sdaiINTEGER); }
        void put_IfcPositiveInteger(IfcPositiveInteger value) { putSimpleValue("IFCPOSITIVEINTEGER", sdaiINTEGER, value); }

        bool is_IfcReal() { return IsADBType("IFCREAL"); }
        Nullable<IfcReal> get_IfcReal() { return getSimpleValue<IfcReal>("IFCREAL", sdaiREAL); }
        void put_IfcReal(IfcReal value) { putSimpleValue("IFCREAL", sdaiREAL, value); }

        bool is_IfcText() { return IsADBType("IFCTEXT"); }
        IfcText get_IfcText() { return getTextValue("IFCTEXT", sdaiSTRING); }
        void put_IfcText(IfcText value) { putTextValue("IFCTEXT", sdaiSTRING, value); }

        bool is_IfcTime() { return IsADBType("IFCTIME"); }
        IfcTime get_IfcTime() { return getTextValue("IFCTIME", sdaiSTRING); }
        void put_IfcTime(IfcTime value) { putTextValue("IFCTIME", sdaiSTRING, value); }

        bool is_IfcTimeStamp() { return IsADBType("IFCTIMESTAMP"); }
        Nullable<IfcTimeStamp> get_IfcTimeStamp() { return getSimpleValue<IfcTimeStamp>("IFCTIMESTAMP", sdaiINTEGER); }
        void put_IfcTimeStamp(IfcTimeStamp value) { putSimpleValue("IFCTIMESTAMP", sdaiINTEGER, value); }
    };


    class IfcSimpleValue_get : public Select
    {
    public:
        IfcSimpleValue_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSimpleValue_get(Select* outer) : Select(outer) {}
        bool is_IfcBinary() { return IsADBType("IFCBINARY"); }
        IfcBinary get_IfcBinary() { return getTextValue("IFCBINARY", sdaiBINARY); }
        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        bool is_IfcDate() { return IsADBType("IFCDATE"); }
        IfcDate get_IfcDate() { return getTextValue("IFCDATE", sdaiSTRING); }
        bool is_IfcDateTime() { return IsADBType("IFCDATETIME"); }
        IfcDateTime get_IfcDateTime() { return getTextValue("IFCDATETIME", sdaiSTRING); }
        bool is_IfcDuration() { return IsADBType("IFCDURATION"); }
        IfcDuration get_IfcDuration() { return getTextValue("IFCDURATION", sdaiSTRING); }
        bool is_IfcIdentifier() { return IsADBType("IFCIDENTIFIER"); }
        IfcIdentifier get_IfcIdentifier() { return getTextValue("IFCIDENTIFIER", sdaiSTRING); }
        bool is_IfcInteger() { return IsADBType("IFCINTEGER"); }
        Nullable<IfcInteger> get_IfcInteger() { return getSimpleValue<IfcInteger>("IFCINTEGER", sdaiINTEGER); }
        bool is_IfcLabel() { return IsADBType("IFCLABEL"); }
        IfcLabel get_IfcLabel() { return getTextValue("IFCLABEL", sdaiSTRING); }
        bool is_IfcLogical() { return IsADBType("IFCLOGICAL"); }
        Nullable<IfcLogical> get_IfcLogical() { int v = getEnumerationValue("IFCLOGICAL", LOGICAL_VALUE_); if (v >= 0) return (IfcLogical) v; else return Nullable<IfcLogical>(); }
        bool is_IfcPositiveInteger() { return IsADBType("IFCPOSITIVEINTEGER"); }
        Nullable<IfcPositiveInteger> get_IfcPositiveInteger() { return getSimpleValue<IfcPositiveInteger>("IFCPOSITIVEINTEGER", sdaiINTEGER); }
        bool is_IfcReal() { return IsADBType("IFCREAL"); }
        Nullable<IfcReal> get_IfcReal() { return getSimpleValue<IfcReal>("IFCREAL", sdaiREAL); }
        bool is_IfcText() { return IsADBType("IFCTEXT"); }
        IfcText get_IfcText() { return getTextValue("IFCTEXT", sdaiSTRING); }
        bool is_IfcTime() { return IsADBType("IFCTIME"); }
        IfcTime get_IfcTime() { return getTextValue("IFCTIME", sdaiSTRING); }
        bool is_IfcTimeStamp() { return IsADBType("IFCTIMESTAMP"); }
        Nullable<IfcTimeStamp> get_IfcTimeStamp() { return getSimpleValue<IfcTimeStamp>("IFCTIMESTAMP", sdaiINTEGER); }

        TextValue as_text() { TextValue val = NULL; sdaiGetAttrBN(m_instance, m_attrName, sdaiSTRING, &val); return val; }
        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
        Nullable<IntValue> as_int() { IntValue val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiINTEGER, &val)) return val; else return Nullable<IntValue>(); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcSimpleValue_put : public Select
    {
    public:
        IfcSimpleValue_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSimpleValue_put(Select* outer) : Select(outer) {}
        void put_IfcBinary(IfcBinary value) { putTextValue("IFCBINARY", sdaiBINARY, value); }
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }
        void put_IfcDate(IfcDate value) { putTextValue("IFCDATE", sdaiSTRING, value); }
        void put_IfcDateTime(IfcDateTime value) { putTextValue("IFCDATETIME", sdaiSTRING, value); }
        void put_IfcDuration(IfcDuration value) { putTextValue("IFCDURATION", sdaiSTRING, value); }
        void put_IfcIdentifier(IfcIdentifier value) { putTextValue("IFCIDENTIFIER", sdaiSTRING, value); }
        void put_IfcInteger(IfcInteger value) { putSimpleValue("IFCINTEGER", sdaiINTEGER, value); }
        void put_IfcLabel(IfcLabel value) { putTextValue("IFCLABEL", sdaiSTRING, value); }
        void put_IfcLogical(IfcLogical value) { TextValue val = LOGICAL_VALUE_[(int) value]; putEnumerationValue("IFCLOGICAL", val); }
        void put_IfcPositiveInteger(IfcPositiveInteger value) { putSimpleValue("IFCPOSITIVEINTEGER", sdaiINTEGER, value); }
        void put_IfcReal(IfcReal value) { putSimpleValue("IFCREAL", sdaiREAL, value); }
        void put_IfcText(IfcText value) { putTextValue("IFCTEXT", sdaiSTRING, value); }
        void put_IfcTime(IfcTime value) { putTextValue("IFCTIME", sdaiSTRING, value); }
        void put_IfcTimeStamp(IfcTimeStamp value) { putSimpleValue("IFCTIMESTAMP", sdaiINTEGER, value); }
    };


    class IfcValue : public Select
    {
    public:
        IfcValue(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcValue(Select* outer) : Select(outer) {}

        IfcDerivedMeasureValue _IfcDerivedMeasureValue() { return IfcDerivedMeasureValue(this); }

        IfcMeasureValue _IfcMeasureValue() { return IfcMeasureValue(this); }

        IfcSimpleValue _IfcSimpleValue() { return IfcSimpleValue(this); }
    };


    class IfcValue_get : public Select
    {
    public:
        IfcValue_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcValue_get(Select* outer) : Select(outer) {}
        IfcDerivedMeasureValue_get get_IfcDerivedMeasureValue() { return IfcDerivedMeasureValue_get(this); }
        IfcMeasureValue_get get_IfcMeasureValue() { return IfcMeasureValue_get(this); }
        IfcSimpleValue_get get_IfcSimpleValue() { return IfcSimpleValue_get(this); }

        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
        Nullable<IntValue> as_int() { IntValue val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiINTEGER, &val)) return val; else return Nullable<IntValue>(); }
        TextValue as_text() { TextValue val = NULL; sdaiGetAttrBN(m_instance, m_attrName, sdaiSTRING, &val); return val; }
        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
    };


    class IfcValue_put : public Select
    {
    public:
        IfcValue_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcValue_put(Select* outer) : Select(outer) {}
        IfcDerivedMeasureValue_put put_IfcDerivedMeasureValue() { return IfcDerivedMeasureValue_put(this); }
        IfcMeasureValue_put put_IfcMeasureValue() { return IfcMeasureValue_put(this); }
        IfcSimpleValue_put put_IfcSimpleValue() { return IfcSimpleValue_put(this); }
    };


    class IfcAppliedValueSelect : public Select
    {
    public:
        IfcAppliedValueSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcAppliedValueSelect(Select* outer) : Select(outer) {}

        bool is_IfcMeasureWithUnit() { return IsADBEntity("IfcMeasureWithUnit"); }
        IfcMeasureWithUnit get_IfcMeasureWithUnit();
        void put_IfcMeasureWithUnit(IfcMeasureWithUnit inst);

        bool is_IfcReference() { return IsADBEntity("IfcReference"); }
        IfcReference get_IfcReference();
        void put_IfcReference(IfcReference inst);

        IfcValue _IfcValue() { return IfcValue(this); }
    };


    class IfcAppliedValueSelect_get : public Select
    {
    public:
        IfcAppliedValueSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcAppliedValueSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcMeasureWithUnit() { return IsADBEntity("IfcMeasureWithUnit"); }
        IfcMeasureWithUnit get_IfcMeasureWithUnit();
        bool is_IfcReference() { return IsADBEntity("IfcReference"); }
        IfcReference get_IfcReference();
        IfcValue_get get_IfcValue() { return IfcValue_get(this); }

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
        Nullable<IntValue> as_int() { IntValue val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiINTEGER, &val)) return val; else return Nullable<IntValue>(); }
        TextValue as_text() { TextValue val = NULL; sdaiGetAttrBN(m_instance, m_attrName, sdaiSTRING, &val); return val; }
        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
    };


    class IfcAppliedValueSelect_put : public Select
    {
    public:
        IfcAppliedValueSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcAppliedValueSelect_put(Select* outer) : Select(outer) {}
        void put_IfcMeasureWithUnit(IfcMeasureWithUnit inst);
        void put_IfcReference(IfcReference inst);
        IfcValue_put put_IfcValue() { return IfcValue_put(this); }
    };


    class IfcAxis2Placement : public Select
    {
    public:
        IfcAxis2Placement(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcAxis2Placement(Select* outer) : Select(outer) {}

        bool is_IfcAxis2Placement2D() { return IsADBEntity("IfcAxis2Placement2D"); }
        IfcAxis2Placement2D get_IfcAxis2Placement2D();
        void put_IfcAxis2Placement2D(IfcAxis2Placement2D inst);

        bool is_IfcAxis2Placement3D() { return IsADBEntity("IfcAxis2Placement3D"); }
        IfcAxis2Placement3D get_IfcAxis2Placement3D();
        void put_IfcAxis2Placement3D(IfcAxis2Placement3D inst);
    };


    class IfcAxis2Placement_get : public Select
    {
    public:
        IfcAxis2Placement_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcAxis2Placement_get(Select* outer) : Select(outer) {}
        bool is_IfcAxis2Placement2D() { return IsADBEntity("IfcAxis2Placement2D"); }
        IfcAxis2Placement2D get_IfcAxis2Placement2D();
        bool is_IfcAxis2Placement3D() { return IsADBEntity("IfcAxis2Placement3D"); }
        IfcAxis2Placement3D get_IfcAxis2Placement3D();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcAxis2Placement_put : public Select
    {
    public:
        IfcAxis2Placement_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcAxis2Placement_put(Select* outer) : Select(outer) {}
        void put_IfcAxis2Placement2D(IfcAxis2Placement2D inst);
        void put_IfcAxis2Placement3D(IfcAxis2Placement3D inst);
    };


    class IfcBendingParameterSelect : public Select
    {
    public:
        IfcBendingParameterSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcBendingParameterSelect(Select* outer) : Select(outer) {}

        bool is_IfcLengthMeasure() { return IsADBType("IFCLENGTHMEASURE"); }
        Nullable<IfcLengthMeasure> get_IfcLengthMeasure() { return getSimpleValue<IfcLengthMeasure>("IFCLENGTHMEASURE", sdaiREAL); }
        void put_IfcLengthMeasure(IfcLengthMeasure value) { putSimpleValue("IFCLENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcPlaneAngleMeasure() { return IsADBType("IFCPLANEANGLEMEASURE"); }
        Nullable<IfcPlaneAngleMeasure> get_IfcPlaneAngleMeasure() { return getSimpleValue<IfcPlaneAngleMeasure>("IFCPLANEANGLEMEASURE", sdaiREAL); }
        void put_IfcPlaneAngleMeasure(IfcPlaneAngleMeasure value) { putSimpleValue("IFCPLANEANGLEMEASURE", sdaiREAL, value); }
    };


    class IfcBendingParameterSelect_get : public Select
    {
    public:
        IfcBendingParameterSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcBendingParameterSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcLengthMeasure() { return IsADBType("IFCLENGTHMEASURE"); }
        Nullable<IfcLengthMeasure> get_IfcLengthMeasure() { return getSimpleValue<IfcLengthMeasure>("IFCLENGTHMEASURE", sdaiREAL); }
        bool is_IfcPlaneAngleMeasure() { return IsADBType("IFCPLANEANGLEMEASURE"); }
        Nullable<IfcPlaneAngleMeasure> get_IfcPlaneAngleMeasure() { return getSimpleValue<IfcPlaneAngleMeasure>("IFCPLANEANGLEMEASURE", sdaiREAL); }

        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcBendingParameterSelect_put : public Select
    {
    public:
        IfcBendingParameterSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcBendingParameterSelect_put(Select* outer) : Select(outer) {}
        void put_IfcLengthMeasure(IfcLengthMeasure value) { putSimpleValue("IFCLENGTHMEASURE", sdaiREAL, value); }
        void put_IfcPlaneAngleMeasure(IfcPlaneAngleMeasure value) { putSimpleValue("IFCPLANEANGLEMEASURE", sdaiREAL, value); }
    };


    class IfcBooleanOperand : public Select
    {
    public:
        IfcBooleanOperand(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcBooleanOperand(Select* outer) : Select(outer) {}

        bool is_IfcBooleanResult() { return IsADBEntity("IfcBooleanResult"); }
        IfcBooleanResult get_IfcBooleanResult();
        void put_IfcBooleanResult(IfcBooleanResult inst);

        bool is_IfcCsgPrimitive3D() { return IsADBEntity("IfcCsgPrimitive3D"); }
        IfcCsgPrimitive3D get_IfcCsgPrimitive3D();
        void put_IfcCsgPrimitive3D(IfcCsgPrimitive3D inst);

        bool is_IfcHalfSpaceSolid() { return IsADBEntity("IfcHalfSpaceSolid"); }
        IfcHalfSpaceSolid get_IfcHalfSpaceSolid();
        void put_IfcHalfSpaceSolid(IfcHalfSpaceSolid inst);

        bool is_IfcSolidModel() { return IsADBEntity("IfcSolidModel"); }
        IfcSolidModel get_IfcSolidModel();
        void put_IfcSolidModel(IfcSolidModel inst);

        bool is_IfcTessellatedFaceSet() { return IsADBEntity("IfcTessellatedFaceSet"); }
        IfcTessellatedFaceSet get_IfcTessellatedFaceSet();
        void put_IfcTessellatedFaceSet(IfcTessellatedFaceSet inst);
    };


    class IfcBooleanOperand_get : public Select
    {
    public:
        IfcBooleanOperand_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcBooleanOperand_get(Select* outer) : Select(outer) {}
        bool is_IfcBooleanResult() { return IsADBEntity("IfcBooleanResult"); }
        IfcBooleanResult get_IfcBooleanResult();
        bool is_IfcCsgPrimitive3D() { return IsADBEntity("IfcCsgPrimitive3D"); }
        IfcCsgPrimitive3D get_IfcCsgPrimitive3D();
        bool is_IfcHalfSpaceSolid() { return IsADBEntity("IfcHalfSpaceSolid"); }
        IfcHalfSpaceSolid get_IfcHalfSpaceSolid();
        bool is_IfcSolidModel() { return IsADBEntity("IfcSolidModel"); }
        IfcSolidModel get_IfcSolidModel();
        bool is_IfcTessellatedFaceSet() { return IsADBEntity("IfcTessellatedFaceSet"); }
        IfcTessellatedFaceSet get_IfcTessellatedFaceSet();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcBooleanOperand_put : public Select
    {
    public:
        IfcBooleanOperand_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcBooleanOperand_put(Select* outer) : Select(outer) {}
        void put_IfcBooleanResult(IfcBooleanResult inst);
        void put_IfcCsgPrimitive3D(IfcCsgPrimitive3D inst);
        void put_IfcHalfSpaceSolid(IfcHalfSpaceSolid inst);
        void put_IfcSolidModel(IfcSolidModel inst);
        void put_IfcTessellatedFaceSet(IfcTessellatedFaceSet inst);
    };


    class IfcClassificationReferenceSelect : public Select
    {
    public:
        IfcClassificationReferenceSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcClassificationReferenceSelect(Select* outer) : Select(outer) {}

        bool is_IfcClassification() { return IsADBEntity("IfcClassification"); }
        IfcClassification get_IfcClassification();
        void put_IfcClassification(IfcClassification inst);

        bool is_IfcClassificationReference() { return IsADBEntity("IfcClassificationReference"); }
        IfcClassificationReference get_IfcClassificationReference();
        void put_IfcClassificationReference(IfcClassificationReference inst);
    };


    class IfcClassificationReferenceSelect_get : public Select
    {
    public:
        IfcClassificationReferenceSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcClassificationReferenceSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcClassification() { return IsADBEntity("IfcClassification"); }
        IfcClassification get_IfcClassification();
        bool is_IfcClassificationReference() { return IsADBEntity("IfcClassificationReference"); }
        IfcClassificationReference get_IfcClassificationReference();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcClassificationReferenceSelect_put : public Select
    {
    public:
        IfcClassificationReferenceSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcClassificationReferenceSelect_put(Select* outer) : Select(outer) {}
        void put_IfcClassification(IfcClassification inst);
        void put_IfcClassificationReference(IfcClassificationReference inst);
    };


    class IfcClassificationSelect : public Select
    {
    public:
        IfcClassificationSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcClassificationSelect(Select* outer) : Select(outer) {}

        bool is_IfcClassification() { return IsADBEntity("IfcClassification"); }
        IfcClassification get_IfcClassification();
        void put_IfcClassification(IfcClassification inst);

        bool is_IfcClassificationReference() { return IsADBEntity("IfcClassificationReference"); }
        IfcClassificationReference get_IfcClassificationReference();
        void put_IfcClassificationReference(IfcClassificationReference inst);
    };


    class IfcClassificationSelect_get : public Select
    {
    public:
        IfcClassificationSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcClassificationSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcClassification() { return IsADBEntity("IfcClassification"); }
        IfcClassification get_IfcClassification();
        bool is_IfcClassificationReference() { return IsADBEntity("IfcClassificationReference"); }
        IfcClassificationReference get_IfcClassificationReference();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcClassificationSelect_put : public Select
    {
    public:
        IfcClassificationSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcClassificationSelect_put(Select* outer) : Select(outer) {}
        void put_IfcClassification(IfcClassification inst);
        void put_IfcClassificationReference(IfcClassificationReference inst);
    };


    class IfcColour : public Select
    {
    public:
        IfcColour(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcColour(Select* outer) : Select(outer) {}

        bool is_IfcColourSpecification() { return IsADBEntity("IfcColourSpecification"); }
        IfcColourSpecification get_IfcColourSpecification();
        void put_IfcColourSpecification(IfcColourSpecification inst);

        bool is_IfcPreDefinedColour() { return IsADBEntity("IfcPreDefinedColour"); }
        IfcPreDefinedColour get_IfcPreDefinedColour();
        void put_IfcPreDefinedColour(IfcPreDefinedColour inst);
    };


    class IfcColour_get : public Select
    {
    public:
        IfcColour_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcColour_get(Select* outer) : Select(outer) {}
        bool is_IfcColourSpecification() { return IsADBEntity("IfcColourSpecification"); }
        IfcColourSpecification get_IfcColourSpecification();
        bool is_IfcPreDefinedColour() { return IsADBEntity("IfcPreDefinedColour"); }
        IfcPreDefinedColour get_IfcPreDefinedColour();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcColour_put : public Select
    {
    public:
        IfcColour_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcColour_put(Select* outer) : Select(outer) {}
        void put_IfcColourSpecification(IfcColourSpecification inst);
        void put_IfcPreDefinedColour(IfcPreDefinedColour inst);
    };


    class IfcColourOrFactor : public Select
    {
    public:
        IfcColourOrFactor(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcColourOrFactor(Select* outer) : Select(outer) {}

        bool is_IfcColourRgb() { return IsADBEntity("IfcColourRgb"); }
        IfcColourRgb get_IfcColourRgb();
        void put_IfcColourRgb(IfcColourRgb inst);

        bool is_IfcNormalisedRatioMeasure() { return IsADBType("IFCNORMALISEDRATIOMEASURE"); }
        Nullable<IfcNormalisedRatioMeasure> get_IfcNormalisedRatioMeasure() { return getSimpleValue<IfcNormalisedRatioMeasure>("IFCNORMALISEDRATIOMEASURE", sdaiREAL); }
        void put_IfcNormalisedRatioMeasure(IfcNormalisedRatioMeasure value) { putSimpleValue("IFCNORMALISEDRATIOMEASURE", sdaiREAL, value); }
    };


    class IfcColourOrFactor_get : public Select
    {
    public:
        IfcColourOrFactor_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcColourOrFactor_get(Select* outer) : Select(outer) {}
        bool is_IfcColourRgb() { return IsADBEntity("IfcColourRgb"); }
        IfcColourRgb get_IfcColourRgb();
        bool is_IfcNormalisedRatioMeasure() { return IsADBType("IFCNORMALISEDRATIOMEASURE"); }
        Nullable<IfcNormalisedRatioMeasure> get_IfcNormalisedRatioMeasure() { return getSimpleValue<IfcNormalisedRatioMeasure>("IFCNORMALISEDRATIOMEASURE", sdaiREAL); }

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcColourOrFactor_put : public Select
    {
    public:
        IfcColourOrFactor_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcColourOrFactor_put(Select* outer) : Select(outer) {}
        void put_IfcColourRgb(IfcColourRgb inst);
        void put_IfcNormalisedRatioMeasure(IfcNormalisedRatioMeasure value) { putSimpleValue("IFCNORMALISEDRATIOMEASURE", sdaiREAL, value); }
    };


    class IfcCoordinateReferenceSystemSelect : public Select
    {
    public:
        IfcCoordinateReferenceSystemSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCoordinateReferenceSystemSelect(Select* outer) : Select(outer) {}

        bool is_IfcCoordinateReferenceSystem() { return IsADBEntity("IfcCoordinateReferenceSystem"); }
        IfcCoordinateReferenceSystem get_IfcCoordinateReferenceSystem();
        void put_IfcCoordinateReferenceSystem(IfcCoordinateReferenceSystem inst);

        bool is_IfcGeometricRepresentationContext() { return IsADBEntity("IfcGeometricRepresentationContext"); }
        IfcGeometricRepresentationContext get_IfcGeometricRepresentationContext();
        void put_IfcGeometricRepresentationContext(IfcGeometricRepresentationContext inst);
    };


    class IfcCoordinateReferenceSystemSelect_get : public Select
    {
    public:
        IfcCoordinateReferenceSystemSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCoordinateReferenceSystemSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcCoordinateReferenceSystem() { return IsADBEntity("IfcCoordinateReferenceSystem"); }
        IfcCoordinateReferenceSystem get_IfcCoordinateReferenceSystem();
        bool is_IfcGeometricRepresentationContext() { return IsADBEntity("IfcGeometricRepresentationContext"); }
        IfcGeometricRepresentationContext get_IfcGeometricRepresentationContext();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcCoordinateReferenceSystemSelect_put : public Select
    {
    public:
        IfcCoordinateReferenceSystemSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCoordinateReferenceSystemSelect_put(Select* outer) : Select(outer) {}
        void put_IfcCoordinateReferenceSystem(IfcCoordinateReferenceSystem inst);
        void put_IfcGeometricRepresentationContext(IfcGeometricRepresentationContext inst);
    };


    class IfcCsgSelect : public Select
    {
    public:
        IfcCsgSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCsgSelect(Select* outer) : Select(outer) {}

        bool is_IfcBooleanResult() { return IsADBEntity("IfcBooleanResult"); }
        IfcBooleanResult get_IfcBooleanResult();
        void put_IfcBooleanResult(IfcBooleanResult inst);

        bool is_IfcCsgPrimitive3D() { return IsADBEntity("IfcCsgPrimitive3D"); }
        IfcCsgPrimitive3D get_IfcCsgPrimitive3D();
        void put_IfcCsgPrimitive3D(IfcCsgPrimitive3D inst);
    };


    class IfcCsgSelect_get : public Select
    {
    public:
        IfcCsgSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCsgSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcBooleanResult() { return IsADBEntity("IfcBooleanResult"); }
        IfcBooleanResult get_IfcBooleanResult();
        bool is_IfcCsgPrimitive3D() { return IsADBEntity("IfcCsgPrimitive3D"); }
        IfcCsgPrimitive3D get_IfcCsgPrimitive3D();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcCsgSelect_put : public Select
    {
    public:
        IfcCsgSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCsgSelect_put(Select* outer) : Select(outer) {}
        void put_IfcBooleanResult(IfcBooleanResult inst);
        void put_IfcCsgPrimitive3D(IfcCsgPrimitive3D inst);
    };


    class IfcCurveStyleFontSelect : public Select
    {
    public:
        IfcCurveStyleFontSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveStyleFontSelect(Select* outer) : Select(outer) {}

        bool is_IfcCurveStyleFont() { return IsADBEntity("IfcCurveStyleFont"); }
        IfcCurveStyleFont get_IfcCurveStyleFont();
        void put_IfcCurveStyleFont(IfcCurveStyleFont inst);

        bool is_IfcPreDefinedCurveFont() { return IsADBEntity("IfcPreDefinedCurveFont"); }
        IfcPreDefinedCurveFont get_IfcPreDefinedCurveFont();
        void put_IfcPreDefinedCurveFont(IfcPreDefinedCurveFont inst);
    };


    class IfcCurveStyleFontSelect_get : public Select
    {
    public:
        IfcCurveStyleFontSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveStyleFontSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcCurveStyleFont() { return IsADBEntity("IfcCurveStyleFont"); }
        IfcCurveStyleFont get_IfcCurveStyleFont();
        bool is_IfcPreDefinedCurveFont() { return IsADBEntity("IfcPreDefinedCurveFont"); }
        IfcPreDefinedCurveFont get_IfcPreDefinedCurveFont();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcCurveStyleFontSelect_put : public Select
    {
    public:
        IfcCurveStyleFontSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveStyleFontSelect_put(Select* outer) : Select(outer) {}
        void put_IfcCurveStyleFont(IfcCurveStyleFont inst);
        void put_IfcPreDefinedCurveFont(IfcPreDefinedCurveFont inst);
    };


    class IfcCurveFontOrScaledCurveFontSelect : public Select
    {
    public:
        IfcCurveFontOrScaledCurveFontSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveFontOrScaledCurveFontSelect(Select* outer) : Select(outer) {}

        bool is_IfcCurveStyleFontAndScaling() { return IsADBEntity("IfcCurveStyleFontAndScaling"); }
        IfcCurveStyleFontAndScaling get_IfcCurveStyleFontAndScaling();
        void put_IfcCurveStyleFontAndScaling(IfcCurveStyleFontAndScaling inst);

        IfcCurveStyleFontSelect _IfcCurveStyleFontSelect() { return IfcCurveStyleFontSelect(this); }
    };


    class IfcCurveFontOrScaledCurveFontSelect_get : public Select
    {
    public:
        IfcCurveFontOrScaledCurveFontSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveFontOrScaledCurveFontSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcCurveStyleFontAndScaling() { return IsADBEntity("IfcCurveStyleFontAndScaling"); }
        IfcCurveStyleFontAndScaling get_IfcCurveStyleFontAndScaling();
        IfcCurveStyleFontSelect_get get_IfcCurveStyleFontSelect() { return IfcCurveStyleFontSelect_get(this); }

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcCurveFontOrScaledCurveFontSelect_put : public Select
    {
    public:
        IfcCurveFontOrScaledCurveFontSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveFontOrScaledCurveFontSelect_put(Select* outer) : Select(outer) {}
        void put_IfcCurveStyleFontAndScaling(IfcCurveStyleFontAndScaling inst);
        IfcCurveStyleFontSelect_put put_IfcCurveStyleFontSelect() { return IfcCurveStyleFontSelect_put(this); }
    };


    class IfcCurveMeasureSelect : public Select
    {
    public:
        IfcCurveMeasureSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveMeasureSelect(Select* outer) : Select(outer) {}

        bool is_IfcNonNegativeLengthMeasure() { return IsADBType("IFCNONNEGATIVELENGTHMEASURE"); }
        Nullable<IfcNonNegativeLengthMeasure> get_IfcNonNegativeLengthMeasure() { return getSimpleValue<IfcNonNegativeLengthMeasure>("IFCNONNEGATIVELENGTHMEASURE", sdaiREAL); }
        void put_IfcNonNegativeLengthMeasure(IfcNonNegativeLengthMeasure value) { putSimpleValue("IFCNONNEGATIVELENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcParameterValue() { return IsADBType("IFCPARAMETERVALUE"); }
        Nullable<IfcParameterValue> get_IfcParameterValue() { return getSimpleValue<IfcParameterValue>("IFCPARAMETERVALUE", sdaiREAL); }
        void put_IfcParameterValue(IfcParameterValue value) { putSimpleValue("IFCPARAMETERVALUE", sdaiREAL, value); }
    };


    class IfcCurveMeasureSelect_get : public Select
    {
    public:
        IfcCurveMeasureSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveMeasureSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcNonNegativeLengthMeasure() { return IsADBType("IFCNONNEGATIVELENGTHMEASURE"); }
        Nullable<IfcNonNegativeLengthMeasure> get_IfcNonNegativeLengthMeasure() { return getSimpleValue<IfcNonNegativeLengthMeasure>("IFCNONNEGATIVELENGTHMEASURE", sdaiREAL); }
        bool is_IfcParameterValue() { return IsADBType("IFCPARAMETERVALUE"); }
        Nullable<IfcParameterValue> get_IfcParameterValue() { return getSimpleValue<IfcParameterValue>("IFCPARAMETERVALUE", sdaiREAL); }

        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcCurveMeasureSelect_put : public Select
    {
    public:
        IfcCurveMeasureSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveMeasureSelect_put(Select* outer) : Select(outer) {}
        void put_IfcNonNegativeLengthMeasure(IfcNonNegativeLengthMeasure value) { putSimpleValue("IFCNONNEGATIVELENGTHMEASURE", sdaiREAL, value); }
        void put_IfcParameterValue(IfcParameterValue value) { putSimpleValue("IFCPARAMETERVALUE", sdaiREAL, value); }
    };


    class IfcCurveOnSurface : public Select
    {
    public:
        IfcCurveOnSurface(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveOnSurface(Select* outer) : Select(outer) {}

        bool is_IfcCompositeCurveOnSurface() { return IsADBEntity("IfcCompositeCurveOnSurface"); }
        IfcCompositeCurveOnSurface get_IfcCompositeCurveOnSurface();
        void put_IfcCompositeCurveOnSurface(IfcCompositeCurveOnSurface inst);

        bool is_IfcPcurve() { return IsADBEntity("IfcPcurve"); }
        IfcPcurve get_IfcPcurve();
        void put_IfcPcurve(IfcPcurve inst);

        bool is_IfcSurfaceCurve() { return IsADBEntity("IfcSurfaceCurve"); }
        IfcSurfaceCurve get_IfcSurfaceCurve();
        void put_IfcSurfaceCurve(IfcSurfaceCurve inst);
    };


    class IfcCurveOnSurface_get : public Select
    {
    public:
        IfcCurveOnSurface_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveOnSurface_get(Select* outer) : Select(outer) {}
        bool is_IfcCompositeCurveOnSurface() { return IsADBEntity("IfcCompositeCurveOnSurface"); }
        IfcCompositeCurveOnSurface get_IfcCompositeCurveOnSurface();
        bool is_IfcPcurve() { return IsADBEntity("IfcPcurve"); }
        IfcPcurve get_IfcPcurve();
        bool is_IfcSurfaceCurve() { return IsADBEntity("IfcSurfaceCurve"); }
        IfcSurfaceCurve get_IfcSurfaceCurve();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcCurveOnSurface_put : public Select
    {
    public:
        IfcCurveOnSurface_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveOnSurface_put(Select* outer) : Select(outer) {}
        void put_IfcCompositeCurveOnSurface(IfcCompositeCurveOnSurface inst);
        void put_IfcPcurve(IfcPcurve inst);
        void put_IfcSurfaceCurve(IfcSurfaceCurve inst);
    };


    class IfcCurveOrEdgeCurve : public Select
    {
    public:
        IfcCurveOrEdgeCurve(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveOrEdgeCurve(Select* outer) : Select(outer) {}

        bool is_IfcBoundedCurve() { return IsADBEntity("IfcBoundedCurve"); }
        IfcBoundedCurve get_IfcBoundedCurve();
        void put_IfcBoundedCurve(IfcBoundedCurve inst);

        bool is_IfcEdgeCurve() { return IsADBEntity("IfcEdgeCurve"); }
        IfcEdgeCurve get_IfcEdgeCurve();
        void put_IfcEdgeCurve(IfcEdgeCurve inst);
    };


    class IfcCurveOrEdgeCurve_get : public Select
    {
    public:
        IfcCurveOrEdgeCurve_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveOrEdgeCurve_get(Select* outer) : Select(outer) {}
        bool is_IfcBoundedCurve() { return IsADBEntity("IfcBoundedCurve"); }
        IfcBoundedCurve get_IfcBoundedCurve();
        bool is_IfcEdgeCurve() { return IsADBEntity("IfcEdgeCurve"); }
        IfcEdgeCurve get_IfcEdgeCurve();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcCurveOrEdgeCurve_put : public Select
    {
    public:
        IfcCurveOrEdgeCurve_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcCurveOrEdgeCurve_put(Select* outer) : Select(outer) {}
        void put_IfcBoundedCurve(IfcBoundedCurve inst);
        void put_IfcEdgeCurve(IfcEdgeCurve inst);
    };


    class IfcDatasetSelect : public Select
    {
    public:
        IfcDatasetSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDatasetSelect(Select* outer) : Select(outer) {}

        bool is_IfcDatasetInformation() { return IsADBEntity("IfcDatasetInformation"); }
        IfcDatasetInformation get_IfcDatasetInformation();
        void put_IfcDatasetInformation(IfcDatasetInformation inst);

        bool is_IfcDatasetReference() { return IsADBEntity("IfcDatasetReference"); }
        IfcDatasetReference get_IfcDatasetReference();
        void put_IfcDatasetReference(IfcDatasetReference inst);
    };


    class IfcDatasetSelect_get : public Select
    {
    public:
        IfcDatasetSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDatasetSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcDatasetInformation() { return IsADBEntity("IfcDatasetInformation"); }
        IfcDatasetInformation get_IfcDatasetInformation();
        bool is_IfcDatasetReference() { return IsADBEntity("IfcDatasetReference"); }
        IfcDatasetReference get_IfcDatasetReference();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcDatasetSelect_put : public Select
    {
    public:
        IfcDatasetSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDatasetSelect_put(Select* outer) : Select(outer) {}
        void put_IfcDatasetInformation(IfcDatasetInformation inst);
        void put_IfcDatasetReference(IfcDatasetReference inst);
    };


    class IfcDefinitionSelect : public Select
    {
    public:
        IfcDefinitionSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDefinitionSelect(Select* outer) : Select(outer) {}

        bool is_IfcObjectDefinition() { return IsADBEntity("IfcObjectDefinition"); }
        IfcObjectDefinition get_IfcObjectDefinition();
        void put_IfcObjectDefinition(IfcObjectDefinition inst);

        bool is_IfcPropertyDefinition() { return IsADBEntity("IfcPropertyDefinition"); }
        IfcPropertyDefinition get_IfcPropertyDefinition();
        void put_IfcPropertyDefinition(IfcPropertyDefinition inst);
    };


    class IfcDefinitionSelect_get : public Select
    {
    public:
        IfcDefinitionSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDefinitionSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcObjectDefinition() { return IsADBEntity("IfcObjectDefinition"); }
        IfcObjectDefinition get_IfcObjectDefinition();
        bool is_IfcPropertyDefinition() { return IsADBEntity("IfcPropertyDefinition"); }
        IfcPropertyDefinition get_IfcPropertyDefinition();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcDefinitionSelect_put : public Select
    {
    public:
        IfcDefinitionSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDefinitionSelect_put(Select* outer) : Select(outer) {}
        void put_IfcObjectDefinition(IfcObjectDefinition inst);
        void put_IfcPropertyDefinition(IfcPropertyDefinition inst);
    };


    class IfcDocumentSelect : public Select
    {
    public:
        IfcDocumentSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDocumentSelect(Select* outer) : Select(outer) {}

        bool is_IfcDocumentInformation() { return IsADBEntity("IfcDocumentInformation"); }
        IfcDocumentInformation get_IfcDocumentInformation();
        void put_IfcDocumentInformation(IfcDocumentInformation inst);

        bool is_IfcDocumentReference() { return IsADBEntity("IfcDocumentReference"); }
        IfcDocumentReference get_IfcDocumentReference();
        void put_IfcDocumentReference(IfcDocumentReference inst);
    };


    class IfcDocumentSelect_get : public Select
    {
    public:
        IfcDocumentSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDocumentSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcDocumentInformation() { return IsADBEntity("IfcDocumentInformation"); }
        IfcDocumentInformation get_IfcDocumentInformation();
        bool is_IfcDocumentReference() { return IsADBEntity("IfcDocumentReference"); }
        IfcDocumentReference get_IfcDocumentReference();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcDocumentSelect_put : public Select
    {
    public:
        IfcDocumentSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcDocumentSelect_put(Select* outer) : Select(outer) {}
        void put_IfcDocumentInformation(IfcDocumentInformation inst);
        void put_IfcDocumentReference(IfcDocumentReference inst);
    };


    class IfcFillStyleSelect : public Select
    {
    public:
        IfcFillStyleSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcFillStyleSelect(Select* outer) : Select(outer) {}

        IfcColour _IfcColour() { return IfcColour(this); }

        bool is_IfcExternallyDefinedHatchStyle() { return IsADBEntity("IfcExternallyDefinedHatchStyle"); }
        IfcExternallyDefinedHatchStyle get_IfcExternallyDefinedHatchStyle();
        void put_IfcExternallyDefinedHatchStyle(IfcExternallyDefinedHatchStyle inst);

        bool is_IfcFillAreaStyleHatching() { return IsADBEntity("IfcFillAreaStyleHatching"); }
        IfcFillAreaStyleHatching get_IfcFillAreaStyleHatching();
        void put_IfcFillAreaStyleHatching(IfcFillAreaStyleHatching inst);

        bool is_IfcFillAreaStyleTiles() { return IsADBEntity("IfcFillAreaStyleTiles"); }
        IfcFillAreaStyleTiles get_IfcFillAreaStyleTiles();
        void put_IfcFillAreaStyleTiles(IfcFillAreaStyleTiles inst);
    };


    class IfcFillStyleSelect_get : public Select
    {
    public:
        IfcFillStyleSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcFillStyleSelect_get(Select* outer) : Select(outer) {}
        IfcColour_get get_IfcColour() { return IfcColour_get(this); }
        bool is_IfcExternallyDefinedHatchStyle() { return IsADBEntity("IfcExternallyDefinedHatchStyle"); }
        IfcExternallyDefinedHatchStyle get_IfcExternallyDefinedHatchStyle();
        bool is_IfcFillAreaStyleHatching() { return IsADBEntity("IfcFillAreaStyleHatching"); }
        IfcFillAreaStyleHatching get_IfcFillAreaStyleHatching();
        bool is_IfcFillAreaStyleTiles() { return IsADBEntity("IfcFillAreaStyleTiles"); }
        IfcFillAreaStyleTiles get_IfcFillAreaStyleTiles();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcFillStyleSelect_put : public Select
    {
    public:
        IfcFillStyleSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcFillStyleSelect_put(Select* outer) : Select(outer) {}
        IfcColour_put put_IfcColour() { return IfcColour_put(this); }
        void put_IfcExternallyDefinedHatchStyle(IfcExternallyDefinedHatchStyle inst);
        void put_IfcFillAreaStyleHatching(IfcFillAreaStyleHatching inst);
        void put_IfcFillAreaStyleTiles(IfcFillAreaStyleTiles inst);
    };


    class IfcGeometricSetSelect : public Select
    {
    public:
        IfcGeometricSetSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcGeometricSetSelect(Select* outer) : Select(outer) {}

        bool is_IfcCurve() { return IsADBEntity("IfcCurve"); }
        IfcCurve get_IfcCurve();
        void put_IfcCurve(IfcCurve inst);

        bool is_IfcPoint() { return IsADBEntity("IfcPoint"); }
        IfcPoint get_IfcPoint();
        void put_IfcPoint(IfcPoint inst);

        bool is_IfcSurface() { return IsADBEntity("IfcSurface"); }
        IfcSurface get_IfcSurface();
        void put_IfcSurface(IfcSurface inst);
    };


    class IfcGeometricSetSelect_get : public Select
    {
    public:
        IfcGeometricSetSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcGeometricSetSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcCurve() { return IsADBEntity("IfcCurve"); }
        IfcCurve get_IfcCurve();
        bool is_IfcPoint() { return IsADBEntity("IfcPoint"); }
        IfcPoint get_IfcPoint();
        bool is_IfcSurface() { return IsADBEntity("IfcSurface"); }
        IfcSurface get_IfcSurface();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcGeometricSetSelect_put : public Select
    {
    public:
        IfcGeometricSetSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcGeometricSetSelect_put(Select* outer) : Select(outer) {}
        void put_IfcCurve(IfcCurve inst);
        void put_IfcPoint(IfcPoint inst);
        void put_IfcSurface(IfcSurface inst);
    };


    class IfcGridPlacementDirectionSelect : public Select
    {
    public:
        IfcGridPlacementDirectionSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcGridPlacementDirectionSelect(Select* outer) : Select(outer) {}

        bool is_IfcDirection() { return IsADBEntity("IfcDirection"); }
        IfcDirection get_IfcDirection();
        void put_IfcDirection(IfcDirection inst);

        bool is_IfcVirtualGridIntersection() { return IsADBEntity("IfcVirtualGridIntersection"); }
        IfcVirtualGridIntersection get_IfcVirtualGridIntersection();
        void put_IfcVirtualGridIntersection(IfcVirtualGridIntersection inst);
    };


    class IfcGridPlacementDirectionSelect_get : public Select
    {
    public:
        IfcGridPlacementDirectionSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcGridPlacementDirectionSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcDirection() { return IsADBEntity("IfcDirection"); }
        IfcDirection get_IfcDirection();
        bool is_IfcVirtualGridIntersection() { return IsADBEntity("IfcVirtualGridIntersection"); }
        IfcVirtualGridIntersection get_IfcVirtualGridIntersection();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcGridPlacementDirectionSelect_put : public Select
    {
    public:
        IfcGridPlacementDirectionSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcGridPlacementDirectionSelect_put(Select* outer) : Select(outer) {}
        void put_IfcDirection(IfcDirection inst);
        void put_IfcVirtualGridIntersection(IfcVirtualGridIntersection inst);
    };


    class IfcHatchLineDistanceSelect : public Select
    {
    public:
        IfcHatchLineDistanceSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcHatchLineDistanceSelect(Select* outer) : Select(outer) {}

        bool is_IfcPositiveLengthMeasure() { return IsADBType("IFCPOSITIVELENGTHMEASURE"); }
        Nullable<IfcPositiveLengthMeasure> get_IfcPositiveLengthMeasure() { return getSimpleValue<IfcPositiveLengthMeasure>("IFCPOSITIVELENGTHMEASURE", sdaiREAL); }
        void put_IfcPositiveLengthMeasure(IfcPositiveLengthMeasure value) { putSimpleValue("IFCPOSITIVELENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcVector() { return IsADBEntity("IfcVector"); }
        IfcVector get_IfcVector();
        void put_IfcVector(IfcVector inst);
    };


    class IfcHatchLineDistanceSelect_get : public Select
    {
    public:
        IfcHatchLineDistanceSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcHatchLineDistanceSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcPositiveLengthMeasure() { return IsADBType("IFCPOSITIVELENGTHMEASURE"); }
        Nullable<IfcPositiveLengthMeasure> get_IfcPositiveLengthMeasure() { return getSimpleValue<IfcPositiveLengthMeasure>("IFCPOSITIVELENGTHMEASURE", sdaiREAL); }
        bool is_IfcVector() { return IsADBEntity("IfcVector"); }
        IfcVector get_IfcVector();

        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcHatchLineDistanceSelect_put : public Select
    {
    public:
        IfcHatchLineDistanceSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcHatchLineDistanceSelect_put(Select* outer) : Select(outer) {}
        void put_IfcPositiveLengthMeasure(IfcPositiveLengthMeasure value) { putSimpleValue("IFCPOSITIVELENGTHMEASURE", sdaiREAL, value); }
        void put_IfcVector(IfcVector inst);
    };


    class IfcInterferenceSelect : public Select
    {
    public:
        IfcInterferenceSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcInterferenceSelect(Select* outer) : Select(outer) {}

        bool is_IfcElement() { return IsADBEntity("IfcElement"); }
        IfcElement get_IfcElement();
        void put_IfcElement(IfcElement inst);

        bool is_IfcSpatialElement() { return IsADBEntity("IfcSpatialElement"); }
        IfcSpatialElement get_IfcSpatialElement();
        void put_IfcSpatialElement(IfcSpatialElement inst);
    };


    class IfcInterferenceSelect_get : public Select
    {
    public:
        IfcInterferenceSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcInterferenceSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcElement() { return IsADBEntity("IfcElement"); }
        IfcElement get_IfcElement();
        bool is_IfcSpatialElement() { return IsADBEntity("IfcSpatialElement"); }
        IfcSpatialElement get_IfcSpatialElement();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcInterferenceSelect_put : public Select
    {
    public:
        IfcInterferenceSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcInterferenceSelect_put(Select* outer) : Select(outer) {}
        void put_IfcElement(IfcElement inst);
        void put_IfcSpatialElement(IfcSpatialElement inst);
    };


    class IfcLayeredItem : public Select
    {
    public:
        IfcLayeredItem(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLayeredItem(Select* outer) : Select(outer) {}

        bool is_IfcRepresentation() { return IsADBEntity("IfcRepresentation"); }
        IfcRepresentation get_IfcRepresentation();
        void put_IfcRepresentation(IfcRepresentation inst);

        bool is_IfcRepresentationItem() { return IsADBEntity("IfcRepresentationItem"); }
        IfcRepresentationItem get_IfcRepresentationItem();
        void put_IfcRepresentationItem(IfcRepresentationItem inst);
    };


    class IfcLayeredItem_get : public Select
    {
    public:
        IfcLayeredItem_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLayeredItem_get(Select* outer) : Select(outer) {}
        bool is_IfcRepresentation() { return IsADBEntity("IfcRepresentation"); }
        IfcRepresentation get_IfcRepresentation();
        bool is_IfcRepresentationItem() { return IsADBEntity("IfcRepresentationItem"); }
        IfcRepresentationItem get_IfcRepresentationItem();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcLayeredItem_put : public Select
    {
    public:
        IfcLayeredItem_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLayeredItem_put(Select* outer) : Select(outer) {}
        void put_IfcRepresentation(IfcRepresentation inst);
        void put_IfcRepresentationItem(IfcRepresentationItem inst);
    };


    class IfcLibrarySelect : public Select
    {
    public:
        IfcLibrarySelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLibrarySelect(Select* outer) : Select(outer) {}

        bool is_IfcLibraryInformation() { return IsADBEntity("IfcLibraryInformation"); }
        IfcLibraryInformation get_IfcLibraryInformation();
        void put_IfcLibraryInformation(IfcLibraryInformation inst);

        bool is_IfcLibraryReference() { return IsADBEntity("IfcLibraryReference"); }
        IfcLibraryReference get_IfcLibraryReference();
        void put_IfcLibraryReference(IfcLibraryReference inst);
    };


    class IfcLibrarySelect_get : public Select
    {
    public:
        IfcLibrarySelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLibrarySelect_get(Select* outer) : Select(outer) {}
        bool is_IfcLibraryInformation() { return IsADBEntity("IfcLibraryInformation"); }
        IfcLibraryInformation get_IfcLibraryInformation();
        bool is_IfcLibraryReference() { return IsADBEntity("IfcLibraryReference"); }
        IfcLibraryReference get_IfcLibraryReference();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcLibrarySelect_put : public Select
    {
    public:
        IfcLibrarySelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLibrarySelect_put(Select* outer) : Select(outer) {}
        void put_IfcLibraryInformation(IfcLibraryInformation inst);
        void put_IfcLibraryReference(IfcLibraryReference inst);
    };


    class IfcLightDistributionDataSourceSelect : public Select
    {
    public:
        IfcLightDistributionDataSourceSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLightDistributionDataSourceSelect(Select* outer) : Select(outer) {}

        bool is_IfcExternalReference() { return IsADBEntity("IfcExternalReference"); }
        IfcExternalReference get_IfcExternalReference();
        void put_IfcExternalReference(IfcExternalReference inst);

        bool is_IfcLightIntensityDistribution() { return IsADBEntity("IfcLightIntensityDistribution"); }
        IfcLightIntensityDistribution get_IfcLightIntensityDistribution();
        void put_IfcLightIntensityDistribution(IfcLightIntensityDistribution inst);
    };


    class IfcLightDistributionDataSourceSelect_get : public Select
    {
    public:
        IfcLightDistributionDataSourceSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLightDistributionDataSourceSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcExternalReference() { return IsADBEntity("IfcExternalReference"); }
        IfcExternalReference get_IfcExternalReference();
        bool is_IfcLightIntensityDistribution() { return IsADBEntity("IfcLightIntensityDistribution"); }
        IfcLightIntensityDistribution get_IfcLightIntensityDistribution();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcLightDistributionDataSourceSelect_put : public Select
    {
    public:
        IfcLightDistributionDataSourceSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcLightDistributionDataSourceSelect_put(Select* outer) : Select(outer) {}
        void put_IfcExternalReference(IfcExternalReference inst);
        void put_IfcLightIntensityDistribution(IfcLightIntensityDistribution inst);
    };


    class IfcMaterialSelect : public Select
    {
    public:
        IfcMaterialSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMaterialSelect(Select* outer) : Select(outer) {}

        bool is_IfcMaterialDefinition() { return IsADBEntity("IfcMaterialDefinition"); }
        IfcMaterialDefinition get_IfcMaterialDefinition();
        void put_IfcMaterialDefinition(IfcMaterialDefinition inst);

        bool is_IfcMaterialList() { return IsADBEntity("IfcMaterialList"); }
        IfcMaterialList get_IfcMaterialList();
        void put_IfcMaterialList(IfcMaterialList inst);

        bool is_IfcMaterialUsageDefinition() { return IsADBEntity("IfcMaterialUsageDefinition"); }
        IfcMaterialUsageDefinition get_IfcMaterialUsageDefinition();
        void put_IfcMaterialUsageDefinition(IfcMaterialUsageDefinition inst);
    };


    class IfcMaterialSelect_get : public Select
    {
    public:
        IfcMaterialSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMaterialSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcMaterialDefinition() { return IsADBEntity("IfcMaterialDefinition"); }
        IfcMaterialDefinition get_IfcMaterialDefinition();
        bool is_IfcMaterialList() { return IsADBEntity("IfcMaterialList"); }
        IfcMaterialList get_IfcMaterialList();
        bool is_IfcMaterialUsageDefinition() { return IsADBEntity("IfcMaterialUsageDefinition"); }
        IfcMaterialUsageDefinition get_IfcMaterialUsageDefinition();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcMaterialSelect_put : public Select
    {
    public:
        IfcMaterialSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMaterialSelect_put(Select* outer) : Select(outer) {}
        void put_IfcMaterialDefinition(IfcMaterialDefinition inst);
        void put_IfcMaterialList(IfcMaterialList inst);
        void put_IfcMaterialUsageDefinition(IfcMaterialUsageDefinition inst);
    };


    class IfcMetricValueSelect : public Select
    {
    public:
        IfcMetricValueSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMetricValueSelect(Select* outer) : Select(outer) {}

        bool is_IfcAppliedValue() { return IsADBEntity("IfcAppliedValue"); }
        IfcAppliedValue get_IfcAppliedValue();
        void put_IfcAppliedValue(IfcAppliedValue inst);

        bool is_IfcMeasureWithUnit() { return IsADBEntity("IfcMeasureWithUnit"); }
        IfcMeasureWithUnit get_IfcMeasureWithUnit();
        void put_IfcMeasureWithUnit(IfcMeasureWithUnit inst);

        bool is_IfcReference() { return IsADBEntity("IfcReference"); }
        IfcReference get_IfcReference();
        void put_IfcReference(IfcReference inst);

        bool is_IfcTable() { return IsADBEntity("IfcTable"); }
        IfcTable get_IfcTable();
        void put_IfcTable(IfcTable inst);

        bool is_IfcTimeSeries() { return IsADBEntity("IfcTimeSeries"); }
        IfcTimeSeries get_IfcTimeSeries();
        void put_IfcTimeSeries(IfcTimeSeries inst);

        IfcValue _IfcValue() { return IfcValue(this); }
    };


    class IfcMetricValueSelect_get : public Select
    {
    public:
        IfcMetricValueSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMetricValueSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcAppliedValue() { return IsADBEntity("IfcAppliedValue"); }
        IfcAppliedValue get_IfcAppliedValue();
        bool is_IfcMeasureWithUnit() { return IsADBEntity("IfcMeasureWithUnit"); }
        IfcMeasureWithUnit get_IfcMeasureWithUnit();
        bool is_IfcReference() { return IsADBEntity("IfcReference"); }
        IfcReference get_IfcReference();
        bool is_IfcTable() { return IsADBEntity("IfcTable"); }
        IfcTable get_IfcTable();
        bool is_IfcTimeSeries() { return IsADBEntity("IfcTimeSeries"); }
        IfcTimeSeries get_IfcTimeSeries();
        IfcValue_get get_IfcValue() { return IfcValue_get(this); }

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
        Nullable<IntValue> as_int() { IntValue val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiINTEGER, &val)) return val; else return Nullable<IntValue>(); }
        TextValue as_text() { TextValue val = NULL; sdaiGetAttrBN(m_instance, m_attrName, sdaiSTRING, &val); return val; }
        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
    };


    class IfcMetricValueSelect_put : public Select
    {
    public:
        IfcMetricValueSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcMetricValueSelect_put(Select* outer) : Select(outer) {}
        void put_IfcAppliedValue(IfcAppliedValue inst);
        void put_IfcMeasureWithUnit(IfcMeasureWithUnit inst);
        void put_IfcReference(IfcReference inst);
        void put_IfcTable(IfcTable inst);
        void put_IfcTimeSeries(IfcTimeSeries inst);
        IfcValue_put put_IfcValue() { return IfcValue_put(this); }
    };


    class IfcModulusOfRotationalSubgradeReactionSelect : public Select
    {
    public:
        IfcModulusOfRotationalSubgradeReactionSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfRotationalSubgradeReactionSelect(Select* outer) : Select(outer) {}

        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }

        bool is_IfcModulusOfRotationalSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfRotationalSubgradeReactionMeasure> get_IfcModulusOfRotationalSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfRotationalSubgradeReactionMeasure>("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE", sdaiREAL); }
        void put_IfcModulusOfRotationalSubgradeReactionMeasure(IfcModulusOfRotationalSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
    };


    class IfcModulusOfRotationalSubgradeReactionSelect_get : public Select
    {
    public:
        IfcModulusOfRotationalSubgradeReactionSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfRotationalSubgradeReactionSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        bool is_IfcModulusOfRotationalSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfRotationalSubgradeReactionMeasure> get_IfcModulusOfRotationalSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfRotationalSubgradeReactionMeasure>("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE", sdaiREAL); }

        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcModulusOfRotationalSubgradeReactionSelect_put : public Select
    {
    public:
        IfcModulusOfRotationalSubgradeReactionSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfRotationalSubgradeReactionSelect_put(Select* outer) : Select(outer) {}
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }
        void put_IfcModulusOfRotationalSubgradeReactionMeasure(IfcModulusOfRotationalSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFROTATIONALSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
    };


    class IfcModulusOfSubgradeReactionSelect : public Select
    {
    public:
        IfcModulusOfSubgradeReactionSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfSubgradeReactionSelect(Select* outer) : Select(outer) {}

        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }

        bool is_IfcModulusOfSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfSubgradeReactionMeasure> get_IfcModulusOfSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfSubgradeReactionMeasure>("IFCMODULUSOFSUBGRADEREACTIONMEASURE", sdaiREAL); }
        void put_IfcModulusOfSubgradeReactionMeasure(IfcModulusOfSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
    };


    class IfcModulusOfSubgradeReactionSelect_get : public Select
    {
    public:
        IfcModulusOfSubgradeReactionSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfSubgradeReactionSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        bool is_IfcModulusOfSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfSubgradeReactionMeasure> get_IfcModulusOfSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfSubgradeReactionMeasure>("IFCMODULUSOFSUBGRADEREACTIONMEASURE", sdaiREAL); }

        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcModulusOfSubgradeReactionSelect_put : public Select
    {
    public:
        IfcModulusOfSubgradeReactionSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfSubgradeReactionSelect_put(Select* outer) : Select(outer) {}
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }
        void put_IfcModulusOfSubgradeReactionMeasure(IfcModulusOfSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
    };


    class IfcModulusOfTranslationalSubgradeReactionSelect : public Select
    {
    public:
        IfcModulusOfTranslationalSubgradeReactionSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfTranslationalSubgradeReactionSelect(Select* outer) : Select(outer) {}

        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }

        bool is_IfcModulusOfLinearSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfLinearSubgradeReactionMeasure> get_IfcModulusOfLinearSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfLinearSubgradeReactionMeasure>("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE", sdaiREAL); }
        void put_IfcModulusOfLinearSubgradeReactionMeasure(IfcModulusOfLinearSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
    };


    class IfcModulusOfTranslationalSubgradeReactionSelect_get : public Select
    {
    public:
        IfcModulusOfTranslationalSubgradeReactionSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfTranslationalSubgradeReactionSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        bool is_IfcModulusOfLinearSubgradeReactionMeasure() { return IsADBType("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE"); }
        Nullable<IfcModulusOfLinearSubgradeReactionMeasure> get_IfcModulusOfLinearSubgradeReactionMeasure() { return getSimpleValue<IfcModulusOfLinearSubgradeReactionMeasure>("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE", sdaiREAL); }

        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcModulusOfTranslationalSubgradeReactionSelect_put : public Select
    {
    public:
        IfcModulusOfTranslationalSubgradeReactionSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcModulusOfTranslationalSubgradeReactionSelect_put(Select* outer) : Select(outer) {}
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }
        void put_IfcModulusOfLinearSubgradeReactionMeasure(IfcModulusOfLinearSubgradeReactionMeasure value) { putSimpleValue("IFCMODULUSOFLINEARSUBGRADEREACTIONMEASURE", sdaiREAL, value); }
    };


    class IfcObjectReferenceSelect : public Select
    {
    public:
        IfcObjectReferenceSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcObjectReferenceSelect(Select* outer) : Select(outer) {}

        bool is_IfcAddress() { return IsADBEntity("IfcAddress"); }
        IfcAddress get_IfcAddress();
        void put_IfcAddress(IfcAddress inst);

        bool is_IfcAppliedValue() { return IsADBEntity("IfcAppliedValue"); }
        IfcAppliedValue get_IfcAppliedValue();
        void put_IfcAppliedValue(IfcAppliedValue inst);

        bool is_IfcExternalReference() { return IsADBEntity("IfcExternalReference"); }
        IfcExternalReference get_IfcExternalReference();
        void put_IfcExternalReference(IfcExternalReference inst);

        bool is_IfcMaterialDefinition() { return IsADBEntity("IfcMaterialDefinition"); }
        IfcMaterialDefinition get_IfcMaterialDefinition();
        void put_IfcMaterialDefinition(IfcMaterialDefinition inst);

        bool is_IfcOrganization() { return IsADBEntity("IfcOrganization"); }
        IfcOrganization get_IfcOrganization();
        void put_IfcOrganization(IfcOrganization inst);

        bool is_IfcPerson() { return IsADBEntity("IfcPerson"); }
        IfcPerson get_IfcPerson();
        void put_IfcPerson(IfcPerson inst);

        bool is_IfcPersonAndOrganization() { return IsADBEntity("IfcPersonAndOrganization"); }
        IfcPersonAndOrganization get_IfcPersonAndOrganization();
        void put_IfcPersonAndOrganization(IfcPersonAndOrganization inst);

        bool is_IfcTable() { return IsADBEntity("IfcTable"); }
        IfcTable get_IfcTable();
        void put_IfcTable(IfcTable inst);

        bool is_IfcTimeSeries() { return IsADBEntity("IfcTimeSeries"); }
        IfcTimeSeries get_IfcTimeSeries();
        void put_IfcTimeSeries(IfcTimeSeries inst);
    };


    class IfcObjectReferenceSelect_get : public Select
    {
    public:
        IfcObjectReferenceSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcObjectReferenceSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcAddress() { return IsADBEntity("IfcAddress"); }
        IfcAddress get_IfcAddress();
        bool is_IfcAppliedValue() { return IsADBEntity("IfcAppliedValue"); }
        IfcAppliedValue get_IfcAppliedValue();
        bool is_IfcExternalReference() { return IsADBEntity("IfcExternalReference"); }
        IfcExternalReference get_IfcExternalReference();
        bool is_IfcMaterialDefinition() { return IsADBEntity("IfcMaterialDefinition"); }
        IfcMaterialDefinition get_IfcMaterialDefinition();
        bool is_IfcOrganization() { return IsADBEntity("IfcOrganization"); }
        IfcOrganization get_IfcOrganization();
        bool is_IfcPerson() { return IsADBEntity("IfcPerson"); }
        IfcPerson get_IfcPerson();
        bool is_IfcPersonAndOrganization() { return IsADBEntity("IfcPersonAndOrganization"); }
        IfcPersonAndOrganization get_IfcPersonAndOrganization();
        bool is_IfcTable() { return IsADBEntity("IfcTable"); }
        IfcTable get_IfcTable();
        bool is_IfcTimeSeries() { return IsADBEntity("IfcTimeSeries"); }
        IfcTimeSeries get_IfcTimeSeries();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcObjectReferenceSelect_put : public Select
    {
    public:
        IfcObjectReferenceSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcObjectReferenceSelect_put(Select* outer) : Select(outer) {}
        void put_IfcAddress(IfcAddress inst);
        void put_IfcAppliedValue(IfcAppliedValue inst);
        void put_IfcExternalReference(IfcExternalReference inst);
        void put_IfcMaterialDefinition(IfcMaterialDefinition inst);
        void put_IfcOrganization(IfcOrganization inst);
        void put_IfcPerson(IfcPerson inst);
        void put_IfcPersonAndOrganization(IfcPersonAndOrganization inst);
        void put_IfcTable(IfcTable inst);
        void put_IfcTimeSeries(IfcTimeSeries inst);
    };


    class IfcPointOrVertexPoint : public Select
    {
    public:
        IfcPointOrVertexPoint(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcPointOrVertexPoint(Select* outer) : Select(outer) {}

        bool is_IfcPoint() { return IsADBEntity("IfcPoint"); }
        IfcPoint get_IfcPoint();
        void put_IfcPoint(IfcPoint inst);

        bool is_IfcVertexPoint() { return IsADBEntity("IfcVertexPoint"); }
        IfcVertexPoint get_IfcVertexPoint();
        void put_IfcVertexPoint(IfcVertexPoint inst);
    };


    class IfcPointOrVertexPoint_get : public Select
    {
    public:
        IfcPointOrVertexPoint_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcPointOrVertexPoint_get(Select* outer) : Select(outer) {}
        bool is_IfcPoint() { return IsADBEntity("IfcPoint"); }
        IfcPoint get_IfcPoint();
        bool is_IfcVertexPoint() { return IsADBEntity("IfcVertexPoint"); }
        IfcVertexPoint get_IfcVertexPoint();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcPointOrVertexPoint_put : public Select
    {
    public:
        IfcPointOrVertexPoint_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcPointOrVertexPoint_put(Select* outer) : Select(outer) {}
        void put_IfcPoint(IfcPoint inst);
        void put_IfcVertexPoint(IfcVertexPoint inst);
    };


    class IfcProcessSelect : public Select
    {
    public:
        IfcProcessSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProcessSelect(Select* outer) : Select(outer) {}

        bool is_IfcProcess() { return IsADBEntity("IfcProcess"); }
        IfcProcess get_IfcProcess();
        void put_IfcProcess(IfcProcess inst);

        bool is_IfcTypeProcess() { return IsADBEntity("IfcTypeProcess"); }
        IfcTypeProcess get_IfcTypeProcess();
        void put_IfcTypeProcess(IfcTypeProcess inst);
    };


    class IfcProcessSelect_get : public Select
    {
    public:
        IfcProcessSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProcessSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcProcess() { return IsADBEntity("IfcProcess"); }
        IfcProcess get_IfcProcess();
        bool is_IfcTypeProcess() { return IsADBEntity("IfcTypeProcess"); }
        IfcTypeProcess get_IfcTypeProcess();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcProcessSelect_put : public Select
    {
    public:
        IfcProcessSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProcessSelect_put(Select* outer) : Select(outer) {}
        void put_IfcProcess(IfcProcess inst);
        void put_IfcTypeProcess(IfcTypeProcess inst);
    };


    class IfcProductRepresentationSelect : public Select
    {
    public:
        IfcProductRepresentationSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProductRepresentationSelect(Select* outer) : Select(outer) {}

        bool is_IfcProductDefinitionShape() { return IsADBEntity("IfcProductDefinitionShape"); }
        IfcProductDefinitionShape get_IfcProductDefinitionShape();
        void put_IfcProductDefinitionShape(IfcProductDefinitionShape inst);

        bool is_IfcRepresentationMap() { return IsADBEntity("IfcRepresentationMap"); }
        IfcRepresentationMap get_IfcRepresentationMap();
        void put_IfcRepresentationMap(IfcRepresentationMap inst);
    };


    class IfcProductRepresentationSelect_get : public Select
    {
    public:
        IfcProductRepresentationSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProductRepresentationSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcProductDefinitionShape() { return IsADBEntity("IfcProductDefinitionShape"); }
        IfcProductDefinitionShape get_IfcProductDefinitionShape();
        bool is_IfcRepresentationMap() { return IsADBEntity("IfcRepresentationMap"); }
        IfcRepresentationMap get_IfcRepresentationMap();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcProductRepresentationSelect_put : public Select
    {
    public:
        IfcProductRepresentationSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProductRepresentationSelect_put(Select* outer) : Select(outer) {}
        void put_IfcProductDefinitionShape(IfcProductDefinitionShape inst);
        void put_IfcRepresentationMap(IfcRepresentationMap inst);
    };


    class IfcProductSelect : public Select
    {
    public:
        IfcProductSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProductSelect(Select* outer) : Select(outer) {}

        bool is_IfcProduct() { return IsADBEntity("IfcProduct"); }
        IfcProduct get_IfcProduct();
        void put_IfcProduct(IfcProduct inst);

        bool is_IfcTypeProduct() { return IsADBEntity("IfcTypeProduct"); }
        IfcTypeProduct get_IfcTypeProduct();
        void put_IfcTypeProduct(IfcTypeProduct inst);
    };


    class IfcProductSelect_get : public Select
    {
    public:
        IfcProductSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProductSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcProduct() { return IsADBEntity("IfcProduct"); }
        IfcProduct get_IfcProduct();
        bool is_IfcTypeProduct() { return IsADBEntity("IfcTypeProduct"); }
        IfcTypeProduct get_IfcTypeProduct();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcProductSelect_put : public Select
    {
    public:
        IfcProductSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcProductSelect_put(Select* outer) : Select(outer) {}
        void put_IfcProduct(IfcProduct inst);
        void put_IfcTypeProduct(IfcTypeProduct inst);
    };


    class IfcPropertySetDefinitionSelect : public Select
    {
    public:
        IfcPropertySetDefinitionSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcPropertySetDefinitionSelect(Select* outer) : Select(outer) {}

        bool is_IfcPropertySetDefinition() { return IsADBEntity("IfcPropertySetDefinition"); }
        IfcPropertySetDefinition get_IfcPropertySetDefinition();
        void put_IfcPropertySetDefinition(IfcPropertySetDefinition inst);

        bool is_IfcPropertySetDefinitionSet() { return IsADBType("IFCPROPERTYSETDEFINITIONSET"); }

        //TList may be IfcPropertySetDefinitionSet or list of converible elements
        template <typename TList> void get_IfcPropertySetDefinitionSet(TList& lst) { SdaiAggr aggr = getAggrValue("IFCPROPERTYSETDEFINITIONSET"); IfcPropertySetDefinitionSetSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }

                //TList may be IfcPropertySetDefinitionSet or list of converible elements
        template <typename TList> void put_IfcPropertySetDefinitionSet(TList& lst) { IfcPropertySetDefinitionSetSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCPROPERTYSETDEFINITIONSET", aggr); }

                //TArrayElem[] may be IfcPropertySetDefinition[] or array of convertible elements
        template <typename TArrayElem> void put_IfcPropertySetDefinitionSet(TArrayElem arr[], size_t n) { IfcPropertySetDefinitionSet lst; ArrayToList(arr, n, lst); put_IfcPropertySetDefinitionSet(lst); }
    };


    class IfcPropertySetDefinitionSelect_get : public Select
    {
    public:
        IfcPropertySetDefinitionSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcPropertySetDefinitionSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcPropertySetDefinition() { return IsADBEntity("IfcPropertySetDefinition"); }
        IfcPropertySetDefinition get_IfcPropertySetDefinition();
        bool is_IfcPropertySetDefinitionSet() { return IsADBType("IFCPROPERTYSETDEFINITIONSET"); }

        //TList may be IfcPropertySetDefinitionSet or list of converible elements
        template <typename TList> void get_IfcPropertySetDefinitionSet(TList& lst) { SdaiAggr aggr = getAggrValue("IFCPROPERTYSETDEFINITIONSET"); IfcPropertySetDefinitionSetSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcPropertySetDefinitionSelect_put : public Select
    {
    public:
        IfcPropertySetDefinitionSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcPropertySetDefinitionSelect_put(Select* outer) : Select(outer) {}
        void put_IfcPropertySetDefinition(IfcPropertySetDefinition inst);

                //TList may be IfcPropertySetDefinitionSet or list of converible elements
        template <typename TList> void put_IfcPropertySetDefinitionSet(TList& lst) { IfcPropertySetDefinitionSetSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCPROPERTYSETDEFINITIONSET", aggr); }

                //TArrayElem[] may be IfcPropertySetDefinition[] or array of convertible elements
        template <typename TArrayElem> void put_IfcPropertySetDefinitionSet(TArrayElem arr[], size_t n) { IfcPropertySetDefinitionSet lst; ArrayToList(arr, n, lst); put_IfcPropertySetDefinitionSet(lst); }
    };


    class IfcResourceObjectSelect : public Select
    {
    public:
        IfcResourceObjectSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcResourceObjectSelect(Select* outer) : Select(outer) {}

        bool is_IfcActorRole() { return IsADBEntity("IfcActorRole"); }
        IfcActorRole get_IfcActorRole();
        void put_IfcActorRole(IfcActorRole inst);

        bool is_IfcAppliedValue() { return IsADBEntity("IfcAppliedValue"); }
        IfcAppliedValue get_IfcAppliedValue();
        void put_IfcAppliedValue(IfcAppliedValue inst);

        bool is_IfcApproval() { return IsADBEntity("IfcApproval"); }
        IfcApproval get_IfcApproval();
        void put_IfcApproval(IfcApproval inst);

        bool is_IfcConstraint() { return IsADBEntity("IfcConstraint"); }
        IfcConstraint get_IfcConstraint();
        void put_IfcConstraint(IfcConstraint inst);

        bool is_IfcContextDependentUnit() { return IsADBEntity("IfcContextDependentUnit"); }
        IfcContextDependentUnit get_IfcContextDependentUnit();
        void put_IfcContextDependentUnit(IfcContextDependentUnit inst);

        bool is_IfcConversionBasedUnit() { return IsADBEntity("IfcConversionBasedUnit"); }
        IfcConversionBasedUnit get_IfcConversionBasedUnit();
        void put_IfcConversionBasedUnit(IfcConversionBasedUnit inst);

        bool is_IfcExternalInformation() { return IsADBEntity("IfcExternalInformation"); }
        IfcExternalInformation get_IfcExternalInformation();
        void put_IfcExternalInformation(IfcExternalInformation inst);

        bool is_IfcExternalReference() { return IsADBEntity("IfcExternalReference"); }
        IfcExternalReference get_IfcExternalReference();
        void put_IfcExternalReference(IfcExternalReference inst);

        bool is_IfcMaterialDefinition() { return IsADBEntity("IfcMaterialDefinition"); }
        IfcMaterialDefinition get_IfcMaterialDefinition();
        void put_IfcMaterialDefinition(IfcMaterialDefinition inst);

        bool is_IfcOrganization() { return IsADBEntity("IfcOrganization"); }
        IfcOrganization get_IfcOrganization();
        void put_IfcOrganization(IfcOrganization inst);

        bool is_IfcPerson() { return IsADBEntity("IfcPerson"); }
        IfcPerson get_IfcPerson();
        void put_IfcPerson(IfcPerson inst);

        bool is_IfcPersonAndOrganization() { return IsADBEntity("IfcPersonAndOrganization"); }
        IfcPersonAndOrganization get_IfcPersonAndOrganization();
        void put_IfcPersonAndOrganization(IfcPersonAndOrganization inst);

        bool is_IfcPhysicalQuantity() { return IsADBEntity("IfcPhysicalQuantity"); }
        IfcPhysicalQuantity get_IfcPhysicalQuantity();
        void put_IfcPhysicalQuantity(IfcPhysicalQuantity inst);

        bool is_IfcProfileDef() { return IsADBEntity("IfcProfileDef"); }
        IfcProfileDef get_IfcProfileDef();
        void put_IfcProfileDef(IfcProfileDef inst);

        bool is_IfcPropertyAbstraction() { return IsADBEntity("IfcPropertyAbstraction"); }
        IfcPropertyAbstraction get_IfcPropertyAbstraction();
        void put_IfcPropertyAbstraction(IfcPropertyAbstraction inst);

        bool is_IfcShapeAspect() { return IsADBEntity("IfcShapeAspect"); }
        IfcShapeAspect get_IfcShapeAspect();
        void put_IfcShapeAspect(IfcShapeAspect inst);

        bool is_IfcTimeSeries() { return IsADBEntity("IfcTimeSeries"); }
        IfcTimeSeries get_IfcTimeSeries();
        void put_IfcTimeSeries(IfcTimeSeries inst);
    };


    class IfcResourceObjectSelect_get : public Select
    {
    public:
        IfcResourceObjectSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcResourceObjectSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcActorRole() { return IsADBEntity("IfcActorRole"); }
        IfcActorRole get_IfcActorRole();
        bool is_IfcAppliedValue() { return IsADBEntity("IfcAppliedValue"); }
        IfcAppliedValue get_IfcAppliedValue();
        bool is_IfcApproval() { return IsADBEntity("IfcApproval"); }
        IfcApproval get_IfcApproval();
        bool is_IfcConstraint() { return IsADBEntity("IfcConstraint"); }
        IfcConstraint get_IfcConstraint();
        bool is_IfcContextDependentUnit() { return IsADBEntity("IfcContextDependentUnit"); }
        IfcContextDependentUnit get_IfcContextDependentUnit();
        bool is_IfcConversionBasedUnit() { return IsADBEntity("IfcConversionBasedUnit"); }
        IfcConversionBasedUnit get_IfcConversionBasedUnit();
        bool is_IfcExternalInformation() { return IsADBEntity("IfcExternalInformation"); }
        IfcExternalInformation get_IfcExternalInformation();
        bool is_IfcExternalReference() { return IsADBEntity("IfcExternalReference"); }
        IfcExternalReference get_IfcExternalReference();
        bool is_IfcMaterialDefinition() { return IsADBEntity("IfcMaterialDefinition"); }
        IfcMaterialDefinition get_IfcMaterialDefinition();
        bool is_IfcOrganization() { return IsADBEntity("IfcOrganization"); }
        IfcOrganization get_IfcOrganization();
        bool is_IfcPerson() { return IsADBEntity("IfcPerson"); }
        IfcPerson get_IfcPerson();
        bool is_IfcPersonAndOrganization() { return IsADBEntity("IfcPersonAndOrganization"); }
        IfcPersonAndOrganization get_IfcPersonAndOrganization();
        bool is_IfcPhysicalQuantity() { return IsADBEntity("IfcPhysicalQuantity"); }
        IfcPhysicalQuantity get_IfcPhysicalQuantity();
        bool is_IfcProfileDef() { return IsADBEntity("IfcProfileDef"); }
        IfcProfileDef get_IfcProfileDef();
        bool is_IfcPropertyAbstraction() { return IsADBEntity("IfcPropertyAbstraction"); }
        IfcPropertyAbstraction get_IfcPropertyAbstraction();
        bool is_IfcShapeAspect() { return IsADBEntity("IfcShapeAspect"); }
        IfcShapeAspect get_IfcShapeAspect();
        bool is_IfcTimeSeries() { return IsADBEntity("IfcTimeSeries"); }
        IfcTimeSeries get_IfcTimeSeries();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcResourceObjectSelect_put : public Select
    {
    public:
        IfcResourceObjectSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcResourceObjectSelect_put(Select* outer) : Select(outer) {}
        void put_IfcActorRole(IfcActorRole inst);
        void put_IfcAppliedValue(IfcAppliedValue inst);
        void put_IfcApproval(IfcApproval inst);
        void put_IfcConstraint(IfcConstraint inst);
        void put_IfcContextDependentUnit(IfcContextDependentUnit inst);
        void put_IfcConversionBasedUnit(IfcConversionBasedUnit inst);
        void put_IfcExternalInformation(IfcExternalInformation inst);
        void put_IfcExternalReference(IfcExternalReference inst);
        void put_IfcMaterialDefinition(IfcMaterialDefinition inst);
        void put_IfcOrganization(IfcOrganization inst);
        void put_IfcPerson(IfcPerson inst);
        void put_IfcPersonAndOrganization(IfcPersonAndOrganization inst);
        void put_IfcPhysicalQuantity(IfcPhysicalQuantity inst);
        void put_IfcProfileDef(IfcProfileDef inst);
        void put_IfcPropertyAbstraction(IfcPropertyAbstraction inst);
        void put_IfcShapeAspect(IfcShapeAspect inst);
        void put_IfcTimeSeries(IfcTimeSeries inst);
    };


    class IfcResourceSelect : public Select
    {
    public:
        IfcResourceSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcResourceSelect(Select* outer) : Select(outer) {}

        bool is_IfcResource() { return IsADBEntity("IfcResource"); }
        IfcResource get_IfcResource();
        void put_IfcResource(IfcResource inst);

        bool is_IfcTypeResource() { return IsADBEntity("IfcTypeResource"); }
        IfcTypeResource get_IfcTypeResource();
        void put_IfcTypeResource(IfcTypeResource inst);
    };


    class IfcResourceSelect_get : public Select
    {
    public:
        IfcResourceSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcResourceSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcResource() { return IsADBEntity("IfcResource"); }
        IfcResource get_IfcResource();
        bool is_IfcTypeResource() { return IsADBEntity("IfcTypeResource"); }
        IfcTypeResource get_IfcTypeResource();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcResourceSelect_put : public Select
    {
    public:
        IfcResourceSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcResourceSelect_put(Select* outer) : Select(outer) {}
        void put_IfcResource(IfcResource inst);
        void put_IfcTypeResource(IfcTypeResource inst);
    };


    class IfcRotationalStiffnessSelect : public Select
    {
    public:
        IfcRotationalStiffnessSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcRotationalStiffnessSelect(Select* outer) : Select(outer) {}

        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }

        bool is_IfcRotationalStiffnessMeasure() { return IsADBType("IFCROTATIONALSTIFFNESSMEASURE"); }
        Nullable<IfcRotationalStiffnessMeasure> get_IfcRotationalStiffnessMeasure() { return getSimpleValue<IfcRotationalStiffnessMeasure>("IFCROTATIONALSTIFFNESSMEASURE", sdaiREAL); }
        void put_IfcRotationalStiffnessMeasure(IfcRotationalStiffnessMeasure value) { putSimpleValue("IFCROTATIONALSTIFFNESSMEASURE", sdaiREAL, value); }
    };


    class IfcRotationalStiffnessSelect_get : public Select
    {
    public:
        IfcRotationalStiffnessSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcRotationalStiffnessSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        bool is_IfcRotationalStiffnessMeasure() { return IsADBType("IFCROTATIONALSTIFFNESSMEASURE"); }
        Nullable<IfcRotationalStiffnessMeasure> get_IfcRotationalStiffnessMeasure() { return getSimpleValue<IfcRotationalStiffnessMeasure>("IFCROTATIONALSTIFFNESSMEASURE", sdaiREAL); }

        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcRotationalStiffnessSelect_put : public Select
    {
    public:
        IfcRotationalStiffnessSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcRotationalStiffnessSelect_put(Select* outer) : Select(outer) {}
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }
        void put_IfcRotationalStiffnessMeasure(IfcRotationalStiffnessMeasure value) { putSimpleValue("IFCROTATIONALSTIFFNESSMEASURE", sdaiREAL, value); }
    };


    class IfcSegmentIndexSelect : public Select
    {
    public:
        IfcSegmentIndexSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSegmentIndexSelect(Select* outer) : Select(outer) {}

        bool is_IfcArcIndex() { return IsADBType("IFCARCINDEX"); }

        //TList may be IfcArcIndex or list of converible elements
        template <typename TList> void get_IfcArcIndex(TList& lst) { SdaiAggr aggr = getAggrValue("IFCARCINDEX"); IfcArcIndexSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }

                //TList may be IfcArcIndex or list of converible elements
        template <typename TList> void put_IfcArcIndex(TList& lst) { IfcArcIndexSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCARCINDEX", aggr); }

                //TArrayElem[] may be IfcPositiveInteger[] or array of convertible elements
        template <typename TArrayElem> void put_IfcArcIndex(TArrayElem arr[], size_t n) { IfcArcIndex lst; ArrayToList(arr, n, lst); put_IfcArcIndex(lst); }

        bool is_IfcLineIndex() { return IsADBType("IFCLINEINDEX"); }

        //TList may be IfcLineIndex or list of converible elements
        template <typename TList> void get_IfcLineIndex(TList& lst) { SdaiAggr aggr = getAggrValue("IFCLINEINDEX"); IfcLineIndexSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }

                //TList may be IfcLineIndex or list of converible elements
        template <typename TList> void put_IfcLineIndex(TList& lst) { IfcLineIndexSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCLINEINDEX", aggr); }

                //TArrayElem[] may be IfcPositiveInteger[] or array of convertible elements
        template <typename TArrayElem> void put_IfcLineIndex(TArrayElem arr[], size_t n) { IfcLineIndex lst; ArrayToList(arr, n, lst); put_IfcLineIndex(lst); }
    };


    class IfcSegmentIndexSelect_get : public Select
    {
    public:
        IfcSegmentIndexSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSegmentIndexSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcArcIndex() { return IsADBType("IFCARCINDEX"); }

        //TList may be IfcArcIndex or list of converible elements
        template <typename TList> void get_IfcArcIndex(TList& lst) { SdaiAggr aggr = getAggrValue("IFCARCINDEX"); IfcArcIndexSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }
        bool is_IfcLineIndex() { return IsADBType("IFCLINEINDEX"); }

        //TList may be IfcLineIndex or list of converible elements
        template <typename TList> void get_IfcLineIndex(TList& lst) { SdaiAggr aggr = getAggrValue("IFCLINEINDEX"); IfcLineIndexSerializer<TList> sr; sr.FromSdaiAggr(lst, m_instance, aggr); }
    };


    class IfcSegmentIndexSelect_put : public Select
    {
    public:
        IfcSegmentIndexSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSegmentIndexSelect_put(Select* outer) : Select(outer) {}

                //TList may be IfcArcIndex or list of converible elements
        template <typename TList> void put_IfcArcIndex(TList& lst) { IfcArcIndexSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCARCINDEX", aggr); }

                //TArrayElem[] may be IfcPositiveInteger[] or array of convertible elements
        template <typename TArrayElem> void put_IfcArcIndex(TArrayElem arr[], size_t n) { IfcArcIndex lst; ArrayToList(arr, n, lst); put_IfcArcIndex(lst); }

                //TList may be IfcLineIndex or list of converible elements
        template <typename TList> void put_IfcLineIndex(TList& lst) { IfcLineIndexSerializer<TList> sr; SdaiAggr aggr = sr.ToSdaiAggr(lst, m_instance, NULL); putAggrValue("IFCLINEINDEX", aggr); }

                //TArrayElem[] may be IfcPositiveInteger[] or array of convertible elements
        template <typename TArrayElem> void put_IfcLineIndex(TArrayElem arr[], size_t n) { IfcLineIndex lst; ArrayToList(arr, n, lst); put_IfcLineIndex(lst); }
    };


    class IfcShell : public Select
    {
    public:
        IfcShell(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcShell(Select* outer) : Select(outer) {}

        bool is_IfcClosedShell() { return IsADBEntity("IfcClosedShell"); }
        IfcClosedShell get_IfcClosedShell();
        void put_IfcClosedShell(IfcClosedShell inst);

        bool is_IfcOpenShell() { return IsADBEntity("IfcOpenShell"); }
        IfcOpenShell get_IfcOpenShell();
        void put_IfcOpenShell(IfcOpenShell inst);
    };


    class IfcShell_get : public Select
    {
    public:
        IfcShell_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcShell_get(Select* outer) : Select(outer) {}
        bool is_IfcClosedShell() { return IsADBEntity("IfcClosedShell"); }
        IfcClosedShell get_IfcClosedShell();
        bool is_IfcOpenShell() { return IsADBEntity("IfcOpenShell"); }
        IfcOpenShell get_IfcOpenShell();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcShell_put : public Select
    {
    public:
        IfcShell_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcShell_put(Select* outer) : Select(outer) {}
        void put_IfcClosedShell(IfcClosedShell inst);
        void put_IfcOpenShell(IfcOpenShell inst);
    };


    class IfcSizeSelect : public Select
    {
    public:
        IfcSizeSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSizeSelect(Select* outer) : Select(outer) {}

        bool is_IfcDescriptiveMeasure() { return IsADBType("IFCDESCRIPTIVEMEASURE"); }
        IfcDescriptiveMeasure get_IfcDescriptiveMeasure() { return getTextValue("IFCDESCRIPTIVEMEASURE", sdaiSTRING); }
        void put_IfcDescriptiveMeasure(IfcDescriptiveMeasure value) { putTextValue("IFCDESCRIPTIVEMEASURE", sdaiSTRING, value); }

        bool is_IfcLengthMeasure() { return IsADBType("IFCLENGTHMEASURE"); }
        Nullable<IfcLengthMeasure> get_IfcLengthMeasure() { return getSimpleValue<IfcLengthMeasure>("IFCLENGTHMEASURE", sdaiREAL); }
        void put_IfcLengthMeasure(IfcLengthMeasure value) { putSimpleValue("IFCLENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcNormalisedRatioMeasure() { return IsADBType("IFCNORMALISEDRATIOMEASURE"); }
        Nullable<IfcNormalisedRatioMeasure> get_IfcNormalisedRatioMeasure() { return getSimpleValue<IfcNormalisedRatioMeasure>("IFCNORMALISEDRATIOMEASURE", sdaiREAL); }
        void put_IfcNormalisedRatioMeasure(IfcNormalisedRatioMeasure value) { putSimpleValue("IFCNORMALISEDRATIOMEASURE", sdaiREAL, value); }

        bool is_IfcPositiveLengthMeasure() { return IsADBType("IFCPOSITIVELENGTHMEASURE"); }
        Nullable<IfcPositiveLengthMeasure> get_IfcPositiveLengthMeasure() { return getSimpleValue<IfcPositiveLengthMeasure>("IFCPOSITIVELENGTHMEASURE", sdaiREAL); }
        void put_IfcPositiveLengthMeasure(IfcPositiveLengthMeasure value) { putSimpleValue("IFCPOSITIVELENGTHMEASURE", sdaiREAL, value); }

        bool is_IfcPositiveRatioMeasure() { return IsADBType("IFCPOSITIVERATIOMEASURE"); }
        Nullable<IfcPositiveRatioMeasure> get_IfcPositiveRatioMeasure() { return getSimpleValue<IfcPositiveRatioMeasure>("IFCPOSITIVERATIOMEASURE", sdaiREAL); }
        void put_IfcPositiveRatioMeasure(IfcPositiveRatioMeasure value) { putSimpleValue("IFCPOSITIVERATIOMEASURE", sdaiREAL, value); }

        bool is_IfcRatioMeasure() { return IsADBType("IFCRATIOMEASURE"); }
        Nullable<IfcRatioMeasure> get_IfcRatioMeasure() { return getSimpleValue<IfcRatioMeasure>("IFCRATIOMEASURE", sdaiREAL); }
        void put_IfcRatioMeasure(IfcRatioMeasure value) { putSimpleValue("IFCRATIOMEASURE", sdaiREAL, value); }
    };


    class IfcSizeSelect_get : public Select
    {
    public:
        IfcSizeSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSizeSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcDescriptiveMeasure() { return IsADBType("IFCDESCRIPTIVEMEASURE"); }
        IfcDescriptiveMeasure get_IfcDescriptiveMeasure() { return getTextValue("IFCDESCRIPTIVEMEASURE", sdaiSTRING); }
        bool is_IfcLengthMeasure() { return IsADBType("IFCLENGTHMEASURE"); }
        Nullable<IfcLengthMeasure> get_IfcLengthMeasure() { return getSimpleValue<IfcLengthMeasure>("IFCLENGTHMEASURE", sdaiREAL); }
        bool is_IfcNormalisedRatioMeasure() { return IsADBType("IFCNORMALISEDRATIOMEASURE"); }
        Nullable<IfcNormalisedRatioMeasure> get_IfcNormalisedRatioMeasure() { return getSimpleValue<IfcNormalisedRatioMeasure>("IFCNORMALISEDRATIOMEASURE", sdaiREAL); }
        bool is_IfcPositiveLengthMeasure() { return IsADBType("IFCPOSITIVELENGTHMEASURE"); }
        Nullable<IfcPositiveLengthMeasure> get_IfcPositiveLengthMeasure() { return getSimpleValue<IfcPositiveLengthMeasure>("IFCPOSITIVELENGTHMEASURE", sdaiREAL); }
        bool is_IfcPositiveRatioMeasure() { return IsADBType("IFCPOSITIVERATIOMEASURE"); }
        Nullable<IfcPositiveRatioMeasure> get_IfcPositiveRatioMeasure() { return getSimpleValue<IfcPositiveRatioMeasure>("IFCPOSITIVERATIOMEASURE", sdaiREAL); }
        bool is_IfcRatioMeasure() { return IsADBType("IFCRATIOMEASURE"); }
        Nullable<IfcRatioMeasure> get_IfcRatioMeasure() { return getSimpleValue<IfcRatioMeasure>("IFCRATIOMEASURE", sdaiREAL); }

        TextValue as_text() { TextValue val = NULL; sdaiGetAttrBN(m_instance, m_attrName, sdaiSTRING, &val); return val; }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcSizeSelect_put : public Select
    {
    public:
        IfcSizeSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSizeSelect_put(Select* outer) : Select(outer) {}
        void put_IfcDescriptiveMeasure(IfcDescriptiveMeasure value) { putTextValue("IFCDESCRIPTIVEMEASURE", sdaiSTRING, value); }
        void put_IfcLengthMeasure(IfcLengthMeasure value) { putSimpleValue("IFCLENGTHMEASURE", sdaiREAL, value); }
        void put_IfcNormalisedRatioMeasure(IfcNormalisedRatioMeasure value) { putSimpleValue("IFCNORMALISEDRATIOMEASURE", sdaiREAL, value); }
        void put_IfcPositiveLengthMeasure(IfcPositiveLengthMeasure value) { putSimpleValue("IFCPOSITIVELENGTHMEASURE", sdaiREAL, value); }
        void put_IfcPositiveRatioMeasure(IfcPositiveRatioMeasure value) { putSimpleValue("IFCPOSITIVERATIOMEASURE", sdaiREAL, value); }
        void put_IfcRatioMeasure(IfcRatioMeasure value) { putSimpleValue("IFCRATIOMEASURE", sdaiREAL, value); }
    };


    class IfcSolidOrShell : public Select
    {
    public:
        IfcSolidOrShell(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSolidOrShell(Select* outer) : Select(outer) {}

        bool is_IfcClosedShell() { return IsADBEntity("IfcClosedShell"); }
        IfcClosedShell get_IfcClosedShell();
        void put_IfcClosedShell(IfcClosedShell inst);

        bool is_IfcSolidModel() { return IsADBEntity("IfcSolidModel"); }
        IfcSolidModel get_IfcSolidModel();
        void put_IfcSolidModel(IfcSolidModel inst);
    };


    class IfcSolidOrShell_get : public Select
    {
    public:
        IfcSolidOrShell_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSolidOrShell_get(Select* outer) : Select(outer) {}
        bool is_IfcClosedShell() { return IsADBEntity("IfcClosedShell"); }
        IfcClosedShell get_IfcClosedShell();
        bool is_IfcSolidModel() { return IsADBEntity("IfcSolidModel"); }
        IfcSolidModel get_IfcSolidModel();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcSolidOrShell_put : public Select
    {
    public:
        IfcSolidOrShell_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSolidOrShell_put(Select* outer) : Select(outer) {}
        void put_IfcClosedShell(IfcClosedShell inst);
        void put_IfcSolidModel(IfcSolidModel inst);
    };


    class IfcSpaceBoundarySelect : public Select
    {
    public:
        IfcSpaceBoundarySelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpaceBoundarySelect(Select* outer) : Select(outer) {}

        bool is_IfcExternalSpatialElement() { return IsADBEntity("IfcExternalSpatialElement"); }
        IfcExternalSpatialElement get_IfcExternalSpatialElement();
        void put_IfcExternalSpatialElement(IfcExternalSpatialElement inst);

        bool is_IfcSpace() { return IsADBEntity("IfcSpace"); }
        IfcSpace get_IfcSpace();
        void put_IfcSpace(IfcSpace inst);
    };


    class IfcSpaceBoundarySelect_get : public Select
    {
    public:
        IfcSpaceBoundarySelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpaceBoundarySelect_get(Select* outer) : Select(outer) {}
        bool is_IfcExternalSpatialElement() { return IsADBEntity("IfcExternalSpatialElement"); }
        IfcExternalSpatialElement get_IfcExternalSpatialElement();
        bool is_IfcSpace() { return IsADBEntity("IfcSpace"); }
        IfcSpace get_IfcSpace();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcSpaceBoundarySelect_put : public Select
    {
    public:
        IfcSpaceBoundarySelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpaceBoundarySelect_put(Select* outer) : Select(outer) {}
        void put_IfcExternalSpatialElement(IfcExternalSpatialElement inst);
        void put_IfcSpace(IfcSpace inst);
    };


    class IfcSpatialReferenceSelect : public Select
    {
    public:
        IfcSpatialReferenceSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpatialReferenceSelect(Select* outer) : Select(outer) {}

        bool is_IfcGroup() { return IsADBEntity("IfcGroup"); }
        IfcGroup get_IfcGroup();
        void put_IfcGroup(IfcGroup inst);

        bool is_IfcProduct() { return IsADBEntity("IfcProduct"); }
        IfcProduct get_IfcProduct();
        void put_IfcProduct(IfcProduct inst);
    };


    class IfcSpatialReferenceSelect_get : public Select
    {
    public:
        IfcSpatialReferenceSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpatialReferenceSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcGroup() { return IsADBEntity("IfcGroup"); }
        IfcGroup get_IfcGroup();
        bool is_IfcProduct() { return IsADBEntity("IfcProduct"); }
        IfcProduct get_IfcProduct();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcSpatialReferenceSelect_put : public Select
    {
    public:
        IfcSpatialReferenceSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpatialReferenceSelect_put(Select* outer) : Select(outer) {}
        void put_IfcGroup(IfcGroup inst);
        void put_IfcProduct(IfcProduct inst);
    };


    class IfcSpecularHighlightSelect : public Select
    {
    public:
        IfcSpecularHighlightSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpecularHighlightSelect(Select* outer) : Select(outer) {}

        bool is_IfcSpecularExponent() { return IsADBType("IFCSPECULAREXPONENT"); }
        Nullable<IfcSpecularExponent> get_IfcSpecularExponent() { return getSimpleValue<IfcSpecularExponent>("IFCSPECULAREXPONENT", sdaiREAL); }
        void put_IfcSpecularExponent(IfcSpecularExponent value) { putSimpleValue("IFCSPECULAREXPONENT", sdaiREAL, value); }

        bool is_IfcSpecularRoughness() { return IsADBType("IFCSPECULARROUGHNESS"); }
        Nullable<IfcSpecularRoughness> get_IfcSpecularRoughness() { return getSimpleValue<IfcSpecularRoughness>("IFCSPECULARROUGHNESS", sdaiREAL); }
        void put_IfcSpecularRoughness(IfcSpecularRoughness value) { putSimpleValue("IFCSPECULARROUGHNESS", sdaiREAL, value); }
    };


    class IfcSpecularHighlightSelect_get : public Select
    {
    public:
        IfcSpecularHighlightSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpecularHighlightSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcSpecularExponent() { return IsADBType("IFCSPECULAREXPONENT"); }
        Nullable<IfcSpecularExponent> get_IfcSpecularExponent() { return getSimpleValue<IfcSpecularExponent>("IFCSPECULAREXPONENT", sdaiREAL); }
        bool is_IfcSpecularRoughness() { return IsADBType("IFCSPECULARROUGHNESS"); }
        Nullable<IfcSpecularRoughness> get_IfcSpecularRoughness() { return getSimpleValue<IfcSpecularRoughness>("IFCSPECULARROUGHNESS", sdaiREAL); }

        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcSpecularHighlightSelect_put : public Select
    {
    public:
        IfcSpecularHighlightSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSpecularHighlightSelect_put(Select* outer) : Select(outer) {}
        void put_IfcSpecularExponent(IfcSpecularExponent value) { putSimpleValue("IFCSPECULAREXPONENT", sdaiREAL, value); }
        void put_IfcSpecularRoughness(IfcSpecularRoughness value) { putSimpleValue("IFCSPECULARROUGHNESS", sdaiREAL, value); }
    };


    class IfcStructuralActivityAssignmentSelect : public Select
    {
    public:
        IfcStructuralActivityAssignmentSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcStructuralActivityAssignmentSelect(Select* outer) : Select(outer) {}

        bool is_IfcElement() { return IsADBEntity("IfcElement"); }
        IfcElement get_IfcElement();
        void put_IfcElement(IfcElement inst);

        bool is_IfcStructuralItem() { return IsADBEntity("IfcStructuralItem"); }
        IfcStructuralItem get_IfcStructuralItem();
        void put_IfcStructuralItem(IfcStructuralItem inst);
    };


    class IfcStructuralActivityAssignmentSelect_get : public Select
    {
    public:
        IfcStructuralActivityAssignmentSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcStructuralActivityAssignmentSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcElement() { return IsADBEntity("IfcElement"); }
        IfcElement get_IfcElement();
        bool is_IfcStructuralItem() { return IsADBEntity("IfcStructuralItem"); }
        IfcStructuralItem get_IfcStructuralItem();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcStructuralActivityAssignmentSelect_put : public Select
    {
    public:
        IfcStructuralActivityAssignmentSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcStructuralActivityAssignmentSelect_put(Select* outer) : Select(outer) {}
        void put_IfcElement(IfcElement inst);
        void put_IfcStructuralItem(IfcStructuralItem inst);
    };


    class IfcSurfaceOrFaceSurface : public Select
    {
    public:
        IfcSurfaceOrFaceSurface(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSurfaceOrFaceSurface(Select* outer) : Select(outer) {}

        bool is_IfcFaceBasedSurfaceModel() { return IsADBEntity("IfcFaceBasedSurfaceModel"); }
        IfcFaceBasedSurfaceModel get_IfcFaceBasedSurfaceModel();
        void put_IfcFaceBasedSurfaceModel(IfcFaceBasedSurfaceModel inst);

        bool is_IfcFaceSurface() { return IsADBEntity("IfcFaceSurface"); }
        IfcFaceSurface get_IfcFaceSurface();
        void put_IfcFaceSurface(IfcFaceSurface inst);

        bool is_IfcSurface() { return IsADBEntity("IfcSurface"); }
        IfcSurface get_IfcSurface();
        void put_IfcSurface(IfcSurface inst);
    };


    class IfcSurfaceOrFaceSurface_get : public Select
    {
    public:
        IfcSurfaceOrFaceSurface_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSurfaceOrFaceSurface_get(Select* outer) : Select(outer) {}
        bool is_IfcFaceBasedSurfaceModel() { return IsADBEntity("IfcFaceBasedSurfaceModel"); }
        IfcFaceBasedSurfaceModel get_IfcFaceBasedSurfaceModel();
        bool is_IfcFaceSurface() { return IsADBEntity("IfcFaceSurface"); }
        IfcFaceSurface get_IfcFaceSurface();
        bool is_IfcSurface() { return IsADBEntity("IfcSurface"); }
        IfcSurface get_IfcSurface();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcSurfaceOrFaceSurface_put : public Select
    {
    public:
        IfcSurfaceOrFaceSurface_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSurfaceOrFaceSurface_put(Select* outer) : Select(outer) {}
        void put_IfcFaceBasedSurfaceModel(IfcFaceBasedSurfaceModel inst);
        void put_IfcFaceSurface(IfcFaceSurface inst);
        void put_IfcSurface(IfcSurface inst);
    };


    class IfcSurfaceStyleElementSelect : public Select
    {
    public:
        IfcSurfaceStyleElementSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSurfaceStyleElementSelect(Select* outer) : Select(outer) {}

        bool is_IfcExternallyDefinedSurfaceStyle() { return IsADBEntity("IfcExternallyDefinedSurfaceStyle"); }
        IfcExternallyDefinedSurfaceStyle get_IfcExternallyDefinedSurfaceStyle();
        void put_IfcExternallyDefinedSurfaceStyle(IfcExternallyDefinedSurfaceStyle inst);

        bool is_IfcSurfaceStyleLighting() { return IsADBEntity("IfcSurfaceStyleLighting"); }
        IfcSurfaceStyleLighting get_IfcSurfaceStyleLighting();
        void put_IfcSurfaceStyleLighting(IfcSurfaceStyleLighting inst);

        bool is_IfcSurfaceStyleRefraction() { return IsADBEntity("IfcSurfaceStyleRefraction"); }
        IfcSurfaceStyleRefraction get_IfcSurfaceStyleRefraction();
        void put_IfcSurfaceStyleRefraction(IfcSurfaceStyleRefraction inst);

        bool is_IfcSurfaceStyleShading() { return IsADBEntity("IfcSurfaceStyleShading"); }
        IfcSurfaceStyleShading get_IfcSurfaceStyleShading();
        void put_IfcSurfaceStyleShading(IfcSurfaceStyleShading inst);

        bool is_IfcSurfaceStyleWithTextures() { return IsADBEntity("IfcSurfaceStyleWithTextures"); }
        IfcSurfaceStyleWithTextures get_IfcSurfaceStyleWithTextures();
        void put_IfcSurfaceStyleWithTextures(IfcSurfaceStyleWithTextures inst);
    };


    class IfcSurfaceStyleElementSelect_get : public Select
    {
    public:
        IfcSurfaceStyleElementSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSurfaceStyleElementSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcExternallyDefinedSurfaceStyle() { return IsADBEntity("IfcExternallyDefinedSurfaceStyle"); }
        IfcExternallyDefinedSurfaceStyle get_IfcExternallyDefinedSurfaceStyle();
        bool is_IfcSurfaceStyleLighting() { return IsADBEntity("IfcSurfaceStyleLighting"); }
        IfcSurfaceStyleLighting get_IfcSurfaceStyleLighting();
        bool is_IfcSurfaceStyleRefraction() { return IsADBEntity("IfcSurfaceStyleRefraction"); }
        IfcSurfaceStyleRefraction get_IfcSurfaceStyleRefraction();
        bool is_IfcSurfaceStyleShading() { return IsADBEntity("IfcSurfaceStyleShading"); }
        IfcSurfaceStyleShading get_IfcSurfaceStyleShading();
        bool is_IfcSurfaceStyleWithTextures() { return IsADBEntity("IfcSurfaceStyleWithTextures"); }
        IfcSurfaceStyleWithTextures get_IfcSurfaceStyleWithTextures();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcSurfaceStyleElementSelect_put : public Select
    {
    public:
        IfcSurfaceStyleElementSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcSurfaceStyleElementSelect_put(Select* outer) : Select(outer) {}
        void put_IfcExternallyDefinedSurfaceStyle(IfcExternallyDefinedSurfaceStyle inst);
        void put_IfcSurfaceStyleLighting(IfcSurfaceStyleLighting inst);
        void put_IfcSurfaceStyleRefraction(IfcSurfaceStyleRefraction inst);
        void put_IfcSurfaceStyleShading(IfcSurfaceStyleShading inst);
        void put_IfcSurfaceStyleWithTextures(IfcSurfaceStyleWithTextures inst);
    };


    class IfcTextFontSelect : public Select
    {
    public:
        IfcTextFontSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTextFontSelect(Select* outer) : Select(outer) {}

        bool is_IfcExternallyDefinedTextFont() { return IsADBEntity("IfcExternallyDefinedTextFont"); }
        IfcExternallyDefinedTextFont get_IfcExternallyDefinedTextFont();
        void put_IfcExternallyDefinedTextFont(IfcExternallyDefinedTextFont inst);

        bool is_IfcPreDefinedTextFont() { return IsADBEntity("IfcPreDefinedTextFont"); }
        IfcPreDefinedTextFont get_IfcPreDefinedTextFont();
        void put_IfcPreDefinedTextFont(IfcPreDefinedTextFont inst);
    };


    class IfcTextFontSelect_get : public Select
    {
    public:
        IfcTextFontSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTextFontSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcExternallyDefinedTextFont() { return IsADBEntity("IfcExternallyDefinedTextFont"); }
        IfcExternallyDefinedTextFont get_IfcExternallyDefinedTextFont();
        bool is_IfcPreDefinedTextFont() { return IsADBEntity("IfcPreDefinedTextFont"); }
        IfcPreDefinedTextFont get_IfcPreDefinedTextFont();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcTextFontSelect_put : public Select
    {
    public:
        IfcTextFontSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTextFontSelect_put(Select* outer) : Select(outer) {}
        void put_IfcExternallyDefinedTextFont(IfcExternallyDefinedTextFont inst);
        void put_IfcPreDefinedTextFont(IfcPreDefinedTextFont inst);
    };


    class IfcTimeOrRatioSelect : public Select
    {
    public:
        IfcTimeOrRatioSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTimeOrRatioSelect(Select* outer) : Select(outer) {}

        bool is_IfcDuration() { return IsADBType("IFCDURATION"); }
        IfcDuration get_IfcDuration() { return getTextValue("IFCDURATION", sdaiSTRING); }
        void put_IfcDuration(IfcDuration value) { putTextValue("IFCDURATION", sdaiSTRING, value); }

        bool is_IfcRatioMeasure() { return IsADBType("IFCRATIOMEASURE"); }
        Nullable<IfcRatioMeasure> get_IfcRatioMeasure() { return getSimpleValue<IfcRatioMeasure>("IFCRATIOMEASURE", sdaiREAL); }
        void put_IfcRatioMeasure(IfcRatioMeasure value) { putSimpleValue("IFCRATIOMEASURE", sdaiREAL, value); }
    };


    class IfcTimeOrRatioSelect_get : public Select
    {
    public:
        IfcTimeOrRatioSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTimeOrRatioSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcDuration() { return IsADBType("IFCDURATION"); }
        IfcDuration get_IfcDuration() { return getTextValue("IFCDURATION", sdaiSTRING); }
        bool is_IfcRatioMeasure() { return IsADBType("IFCRATIOMEASURE"); }
        Nullable<IfcRatioMeasure> get_IfcRatioMeasure() { return getSimpleValue<IfcRatioMeasure>("IFCRATIOMEASURE", sdaiREAL); }

        TextValue as_text() { TextValue val = NULL; sdaiGetAttrBN(m_instance, m_attrName, sdaiSTRING, &val); return val; }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcTimeOrRatioSelect_put : public Select
    {
    public:
        IfcTimeOrRatioSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTimeOrRatioSelect_put(Select* outer) : Select(outer) {}
        void put_IfcDuration(IfcDuration value) { putTextValue("IFCDURATION", sdaiSTRING, value); }
        void put_IfcRatioMeasure(IfcRatioMeasure value) { putSimpleValue("IFCRATIOMEASURE", sdaiREAL, value); }
    };


    class IfcTranslationalStiffnessSelect : public Select
    {
    public:
        IfcTranslationalStiffnessSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTranslationalStiffnessSelect(Select* outer) : Select(outer) {}

        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }

        bool is_IfcLinearStiffnessMeasure() { return IsADBType("IFCLINEARSTIFFNESSMEASURE"); }
        Nullable<IfcLinearStiffnessMeasure> get_IfcLinearStiffnessMeasure() { return getSimpleValue<IfcLinearStiffnessMeasure>("IFCLINEARSTIFFNESSMEASURE", sdaiREAL); }
        void put_IfcLinearStiffnessMeasure(IfcLinearStiffnessMeasure value) { putSimpleValue("IFCLINEARSTIFFNESSMEASURE", sdaiREAL, value); }
    };


    class IfcTranslationalStiffnessSelect_get : public Select
    {
    public:
        IfcTranslationalStiffnessSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTranslationalStiffnessSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        bool is_IfcLinearStiffnessMeasure() { return IsADBType("IFCLINEARSTIFFNESSMEASURE"); }
        Nullable<IfcLinearStiffnessMeasure> get_IfcLinearStiffnessMeasure() { return getSimpleValue<IfcLinearStiffnessMeasure>("IFCLINEARSTIFFNESSMEASURE", sdaiREAL); }

        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcTranslationalStiffnessSelect_put : public Select
    {
    public:
        IfcTranslationalStiffnessSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTranslationalStiffnessSelect_put(Select* outer) : Select(outer) {}
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }
        void put_IfcLinearStiffnessMeasure(IfcLinearStiffnessMeasure value) { putSimpleValue("IFCLINEARSTIFFNESSMEASURE", sdaiREAL, value); }
    };


    class IfcTrimmingSelect : public Select
    {
    public:
        IfcTrimmingSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTrimmingSelect(Select* outer) : Select(outer) {}

        bool is_IfcCartesianPoint() { return IsADBEntity("IfcCartesianPoint"); }
        IfcCartesianPoint get_IfcCartesianPoint();
        void put_IfcCartesianPoint(IfcCartesianPoint inst);

        bool is_IfcParameterValue() { return IsADBType("IFCPARAMETERVALUE"); }
        Nullable<IfcParameterValue> get_IfcParameterValue() { return getSimpleValue<IfcParameterValue>("IFCPARAMETERVALUE", sdaiREAL); }
        void put_IfcParameterValue(IfcParameterValue value) { putSimpleValue("IFCPARAMETERVALUE", sdaiREAL, value); }
    };


    class IfcTrimmingSelect_get : public Select
    {
    public:
        IfcTrimmingSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTrimmingSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcCartesianPoint() { return IsADBEntity("IfcCartesianPoint"); }
        IfcCartesianPoint get_IfcCartesianPoint();
        bool is_IfcParameterValue() { return IsADBType("IFCPARAMETERVALUE"); }
        Nullable<IfcParameterValue> get_IfcParameterValue() { return getSimpleValue<IfcParameterValue>("IFCPARAMETERVALUE", sdaiREAL); }

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcTrimmingSelect_put : public Select
    {
    public:
        IfcTrimmingSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcTrimmingSelect_put(Select* outer) : Select(outer) {}
        void put_IfcCartesianPoint(IfcCartesianPoint inst);
        void put_IfcParameterValue(IfcParameterValue value) { putSimpleValue("IFCPARAMETERVALUE", sdaiREAL, value); }
    };


    class IfcUnit : public Select
    {
    public:
        IfcUnit(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcUnit(Select* outer) : Select(outer) {}

        bool is_IfcDerivedUnit() { return IsADBEntity("IfcDerivedUnit"); }
        IfcDerivedUnit get_IfcDerivedUnit();
        void put_IfcDerivedUnit(IfcDerivedUnit inst);

        bool is_IfcMonetaryUnit() { return IsADBEntity("IfcMonetaryUnit"); }
        IfcMonetaryUnit get_IfcMonetaryUnit();
        void put_IfcMonetaryUnit(IfcMonetaryUnit inst);

        bool is_IfcNamedUnit() { return IsADBEntity("IfcNamedUnit"); }
        IfcNamedUnit get_IfcNamedUnit();
        void put_IfcNamedUnit(IfcNamedUnit inst);
    };


    class IfcUnit_get : public Select
    {
    public:
        IfcUnit_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcUnit_get(Select* outer) : Select(outer) {}
        bool is_IfcDerivedUnit() { return IsADBEntity("IfcDerivedUnit"); }
        IfcDerivedUnit get_IfcDerivedUnit();
        bool is_IfcMonetaryUnit() { return IsADBEntity("IfcMonetaryUnit"); }
        IfcMonetaryUnit get_IfcMonetaryUnit();
        bool is_IfcNamedUnit() { return IsADBEntity("IfcNamedUnit"); }
        IfcNamedUnit get_IfcNamedUnit();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcUnit_put : public Select
    {
    public:
        IfcUnit_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcUnit_put(Select* outer) : Select(outer) {}
        void put_IfcDerivedUnit(IfcDerivedUnit inst);
        void put_IfcMonetaryUnit(IfcMonetaryUnit inst);
        void put_IfcNamedUnit(IfcNamedUnit inst);
    };


    class IfcVectorOrDirection : public Select
    {
    public:
        IfcVectorOrDirection(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcVectorOrDirection(Select* outer) : Select(outer) {}

        bool is_IfcDirection() { return IsADBEntity("IfcDirection"); }
        IfcDirection get_IfcDirection();
        void put_IfcDirection(IfcDirection inst);

        bool is_IfcVector() { return IsADBEntity("IfcVector"); }
        IfcVector get_IfcVector();
        void put_IfcVector(IfcVector inst);
    };


    class IfcVectorOrDirection_get : public Select
    {
    public:
        IfcVectorOrDirection_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcVectorOrDirection_get(Select* outer) : Select(outer) {}
        bool is_IfcDirection() { return IsADBEntity("IfcDirection"); }
        IfcDirection get_IfcDirection();
        bool is_IfcVector() { return IsADBEntity("IfcVector"); }
        IfcVector get_IfcVector();

        SdaiInstance as_instance() { return getEntityInstance(NULL); }
    };


    class IfcVectorOrDirection_put : public Select
    {
    public:
        IfcVectorOrDirection_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcVectorOrDirection_put(Select* outer) : Select(outer) {}
        void put_IfcDirection(IfcDirection inst);
        void put_IfcVector(IfcVector inst);
    };


    class IfcWarpingStiffnessSelect : public Select
    {
    public:
        IfcWarpingStiffnessSelect(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcWarpingStiffnessSelect(Select* outer) : Select(outer) {}

        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }

        bool is_IfcWarpingMomentMeasure() { return IsADBType("IFCWARPINGMOMENTMEASURE"); }
        Nullable<IfcWarpingMomentMeasure> get_IfcWarpingMomentMeasure() { return getSimpleValue<IfcWarpingMomentMeasure>("IFCWARPINGMOMENTMEASURE", sdaiREAL); }
        void put_IfcWarpingMomentMeasure(IfcWarpingMomentMeasure value) { putSimpleValue("IFCWARPINGMOMENTMEASURE", sdaiREAL, value); }
    };


    class IfcWarpingStiffnessSelect_get : public Select
    {
    public:
        IfcWarpingStiffnessSelect_get(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcWarpingStiffnessSelect_get(Select* outer) : Select(outer) {}
        bool is_IfcBoolean() { return IsADBType("IFCBOOLEAN"); }
        Nullable<IfcBoolean> get_IfcBoolean() { return getSimpleValue<IfcBoolean>("IFCBOOLEAN", sdaiBOOLEAN); }
        bool is_IfcWarpingMomentMeasure() { return IsADBType("IFCWARPINGMOMENTMEASURE"); }
        Nullable<IfcWarpingMomentMeasure> get_IfcWarpingMomentMeasure() { return getSimpleValue<IfcWarpingMomentMeasure>("IFCWARPINGMOMENTMEASURE", sdaiREAL); }

        Nullable<bool> as_bool() { bool val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiBOOLEAN, &val)) return val; else return Nullable<bool>(); }
        Nullable<double> as_double() { double val = 0; if (sdaiGetAttrBN(m_instance, m_attrName, sdaiREAL, &val)) return val; else return Nullable<double>(); }
    };


    class IfcWarpingStiffnessSelect_put : public Select
    {
    public:
        IfcWarpingStiffnessSelect_put(SdaiInstance instance, TextValue attrName = NULL, void* adb = NULL) : Select(instance, attrName, adb) {}
        IfcWarpingStiffnessSelect_put(Select* outer) : Select(outer) {}
        void put_IfcBoolean(IfcBoolean value) { putSimpleValue("IFCBOOLEAN", sdaiBOOLEAN, value); }
        void put_IfcWarpingMomentMeasure(IfcWarpingMomentMeasure value) { putSimpleValue("IFCWARPINGMOMENTMEASURE", sdaiREAL, value); }
    };


        //
        // Unnamed aggregations
        //
    typedef std::list<IfcRelAssigns> SetOfIfcRelAssigns;
    template <typename TList> class SetOfIfcRelAssignsSerializer : public AggrSerializerInstance<TList, IfcRelAssigns> {};
    typedef std::list<IfcRelNests> SetOfIfcRelNests;
    template <typename TList> class SetOfIfcRelNestsSerializer : public AggrSerializerInstance<TList, IfcRelNests> {};
    typedef std::list<IfcRelDeclares> SetOfIfcRelDeclares;
    template <typename TList> class SetOfIfcRelDeclaresSerializer : public AggrSerializerInstance<TList, IfcRelDeclares> {};
    typedef std::list<IfcRelAggregates> SetOfIfcRelAggregates;
    template <typename TList> class SetOfIfcRelAggregatesSerializer : public AggrSerializerInstance<TList, IfcRelAggregates> {};
    typedef std::list<IfcRelAssociates> SetOfIfcRelAssociates;
    template <typename TList> class SetOfIfcRelAssociatesSerializer : public AggrSerializerInstance<TList, IfcRelAssociates> {};
    typedef std::list<IfcRelDefinesByObject> SetOfIfcRelDefinesByObject;
    template <typename TList> class SetOfIfcRelDefinesByObjectSerializer : public AggrSerializerInstance<TList, IfcRelDefinesByObject> {};
    typedef std::list<IfcRelDefinesByType> SetOfIfcRelDefinesByType;
    template <typename TList> class SetOfIfcRelDefinesByTypeSerializer : public AggrSerializerInstance<TList, IfcRelDefinesByType> {};
    typedef std::list<IfcRelDefinesByProperties> SetOfIfcRelDefinesByProperties;
    template <typename TList> class SetOfIfcRelDefinesByPropertiesSerializer : public AggrSerializerInstance<TList, IfcRelDefinesByProperties> {};
    typedef std::list<IfcRelAssignsToControl> SetOfIfcRelAssignsToControl;
    template <typename TList> class SetOfIfcRelAssignsToControlSerializer : public AggrSerializerInstance<TList, IfcRelAssignsToControl> {};
    typedef std::list<IfcRelAssignsToActor> SetOfIfcRelAssignsToActor;
    template <typename TList> class SetOfIfcRelAssignsToActorSerializer : public AggrSerializerInstance<TList, IfcRelAssignsToActor> {};
    typedef std::list<IfcExternalReferenceRelationship> SetOfIfcExternalReferenceRelationship;
    template <typename TList> class SetOfIfcExternalReferenceRelationshipSerializer : public AggrSerializerInstance<TList, IfcExternalReferenceRelationship> {};
    typedef std::list<IfcRelAssignsToProduct> SetOfIfcRelAssignsToProduct;
    template <typename TList> class SetOfIfcRelAssignsToProductSerializer : public AggrSerializerInstance<TList, IfcRelAssignsToProduct> {};
    typedef std::list<IfcRelPositions> SetOfIfcRelPositions;
    template <typename TList> class SetOfIfcRelPositionsSerializer : public AggrSerializerInstance<TList, IfcRelPositions> {};
    typedef std::list<IfcRelReferencedInSpatialStructure> SetOfIfcRelReferencedInSpatialStructure;
    template <typename TList> class SetOfIfcRelReferencedInSpatialStructureSerializer : public AggrSerializerInstance<TList, IfcRelReferencedInSpatialStructure> {};
    typedef std::list<IfcRelFillsElement> SetOfIfcRelFillsElement;
    template <typename TList> class SetOfIfcRelFillsElementSerializer : public AggrSerializerInstance<TList, IfcRelFillsElement> {};
    typedef std::list<IfcRelConnectsElements> SetOfIfcRelConnectsElements;
    template <typename TList> class SetOfIfcRelConnectsElementsSerializer : public AggrSerializerInstance<TList, IfcRelConnectsElements> {};
    typedef std::list<IfcRelInterferesElements> SetOfIfcRelInterferesElements;
    template <typename TList> class SetOfIfcRelInterferesElementsSerializer : public AggrSerializerInstance<TList, IfcRelInterferesElements> {};
    typedef std::list<IfcRelProjectsElement> SetOfIfcRelProjectsElement;
    template <typename TList> class SetOfIfcRelProjectsElementSerializer : public AggrSerializerInstance<TList, IfcRelProjectsElement> {};
    typedef std::list<IfcRelVoidsElement> SetOfIfcRelVoidsElement;
    template <typename TList> class SetOfIfcRelVoidsElementSerializer : public AggrSerializerInstance<TList, IfcRelVoidsElement> {};
    typedef std::list<IfcRelConnectsWithRealizingElements> SetOfIfcRelConnectsWithRealizingElements;
    template <typename TList> class SetOfIfcRelConnectsWithRealizingElementsSerializer : public AggrSerializerInstance<TList, IfcRelConnectsWithRealizingElements> {};
    typedef std::list<IfcRelSpaceBoundary> SetOfIfcRelSpaceBoundary;
    template <typename TList> class SetOfIfcRelSpaceBoundarySerializer : public AggrSerializerInstance<TList, IfcRelSpaceBoundary> {};
    typedef std::list<IfcRelContainedInSpatialStructure> SetOfIfcRelContainedInSpatialStructure;
    template <typename TList> class SetOfIfcRelContainedInSpatialStructureSerializer : public AggrSerializerInstance<TList, IfcRelContainedInSpatialStructure> {};
    typedef std::list<IfcRelCoversBldgElements> SetOfIfcRelCoversBldgElements;
    template <typename TList> class SetOfIfcRelCoversBldgElementsSerializer : public AggrSerializerInstance<TList, IfcRelCoversBldgElements> {};
    typedef std::list<IfcRelAdheresToElement> SetOfIfcRelAdheresToElement;
    template <typename TList> class SetOfIfcRelAdheresToElementSerializer : public AggrSerializerInstance<TList, IfcRelAdheresToElement> {};
    typedef std::list<IfcRelConnectsPortToElement> SetOfIfcRelConnectsPortToElement;
    template <typename TList> class SetOfIfcRelConnectsPortToElementSerializer : public AggrSerializerInstance<TList, IfcRelConnectsPortToElement> {};
    typedef std::list<IfcRelFlowControlElements> SetOfIfcRelFlowControlElements;
    template <typename TList> class SetOfIfcRelFlowControlElementsSerializer : public AggrSerializerInstance<TList, IfcRelFlowControlElements> {};
    typedef std::list<IfcPropertySetDefinition> SetOfIfcPropertySetDefinition;
    template <typename TList> class SetOfIfcPropertySetDefinitionSerializer : public AggrSerializerInstance<TList, IfcPropertySetDefinition> {};
    typedef std::list<IfcRepresentationMap> ListOfIfcRepresentationMap;
    template <typename TList> class ListOfIfcRepresentationMapSerializer : public AggrSerializerInstance<TList, IfcRepresentationMap> {};
    typedef std::list<IfcPerson> SetOfIfcPerson;
    template <typename TList> class SetOfIfcPersonSerializer : public AggrSerializerInstance<TList, IfcPerson> {};
    typedef std::list<IfcOrganization> SetOfIfcOrganization;
    template <typename TList> class SetOfIfcOrganizationSerializer : public AggrSerializerInstance<TList, IfcOrganization> {};
    typedef std::list<IfcPresentationLayerAssignment> SetOfIfcPresentationLayerAssignment;
    template <typename TList> class SetOfIfcPresentationLayerAssignmentSerializer : public AggrSerializerInstance<TList, IfcPresentationLayerAssignment> {};
    typedef std::list<IfcStyledItem> SetOfIfcStyledItem;
    template <typename TList> class SetOfIfcStyledItemSerializer : public AggrSerializerInstance<TList, IfcStyledItem> {};
    typedef std::list<IfcClosedShell> SetOfIfcClosedShell;
    template <typename TList> class SetOfIfcClosedShellSerializer : public AggrSerializerInstance<TList, IfcClosedShell> {};
    typedef std::list<IfcFaceBound> SetOfIfcFaceBound;
    template <typename TList> class SetOfIfcFaceBoundSerializer : public AggrSerializerInstance<TList, IfcFaceBound> {};
    typedef std::list<IfcTextureMap> SetOfIfcTextureMap;
    template <typename TList> class SetOfIfcTextureMapSerializer : public AggrSerializerInstance<TList, IfcTextureMap> {};
    typedef std::list<IfcCurve> SetOfIfcCurve;
    template <typename TList> class SetOfIfcCurveSerializer : public AggrSerializerInstance<TList, IfcCurve> {};
    typedef std::list<IfcAppliedValue> ListOfIfcAppliedValue;
    template <typename TList> class ListOfIfcAppliedValueSerializer : public AggrSerializerInstance<TList, IfcAppliedValue> {};
    typedef std::list<IfcRelAssociatesApproval> SetOfIfcRelAssociatesApproval;
    template <typename TList> class SetOfIfcRelAssociatesApprovalSerializer : public AggrSerializerInstance<TList, IfcRelAssociatesApproval> {};
    typedef std::list<IfcResourceApprovalRelationship> SetOfIfcResourceApprovalRelationship;
    template <typename TList> class SetOfIfcResourceApprovalRelationshipSerializer : public AggrSerializerInstance<TList, IfcResourceApprovalRelationship> {};
    typedef std::list<IfcApprovalRelationship> SetOfIfcApprovalRelationship;
    template <typename TList> class SetOfIfcApprovalRelationshipSerializer : public AggrSerializerInstance<TList, IfcApprovalRelationship> {};
    typedef std::list<IfcApproval> SetOfIfcApproval;
    template <typename TList> class SetOfIfcApprovalSerializer : public AggrSerializerInstance<TList, IfcApproval> {};
    typedef std::list<IfcProfileProperties> SetOfIfcProfileProperties;
    template <typename TList> class SetOfIfcProfilePropertiesSerializer : public AggrSerializerInstance<TList, IfcProfileProperties> {};
    typedef std::list<IfcRelAssignsToGroup> SetOfIfcRelAssignsToGroup;
    template <typename TList> class SetOfIfcRelAssignsToGroupSerializer : public AggrSerializerInstance<TList, IfcRelAssignsToGroup> {};
    typedef std::list<StringValue> ListOfIfcIdentifier;
    template <typename TList> class ListOfIfcIdentifierSerializer : public AggrSerializerText<ListOfIfcIdentifier, IfcIdentifier, sdaiSTRING> {};
    typedef std::list<IfcTextureCoordinate> SetOfIfcTextureCoordinate;
    template <typename TList> class SetOfIfcTextureCoordinateSerializer : public AggrSerializerInstance<TList, IfcTextureCoordinate> {};
    typedef std::list<IfcSurfaceStyleWithTextures> SetOfIfcSurfaceStyleWithTextures;
    template <typename TList> class SetOfIfcSurfaceStyleWithTexturesSerializer : public AggrSerializerInstance<TList, IfcSurfaceStyleWithTextures> {};
    typedef std::list<IfcSegment> ListOfIfcSegment;
    template <typename TList> class ListOfIfcSegmentSerializer : public AggrSerializerInstance<TList, IfcSegment> {};
    typedef std::list<IfcRelServicesBuildings> SetOfIfcRelServicesBuildings;
    template <typename TList> class SetOfIfcRelServicesBuildingsSerializer : public AggrSerializerInstance<TList, IfcRelServicesBuildings> {};
    typedef std::list<IfcCartesianPoint> ListOfIfcCartesianPoint;
    template <typename TList> class ListOfIfcCartesianPointSerializer : public AggrSerializerInstance<TList, IfcCartesianPoint> {};
    typedef std::list<IfcInteger> ListOfIfcInteger;
    template <typename TList> class ListOfIfcIntegerSerializer : public AggrSerializerSimple<TList, IfcInteger, sdaiINTEGER> {};
    typedef std::list<IfcParameterValue> ListOfIfcParameterValue;
    template <typename TList> class ListOfIfcParameterValueSerializer : public AggrSerializerSimple<TList, IfcParameterValue, sdaiREAL> {};
    typedef std::list<ListOfIfcCartesianPoint> ListOfListOfIfcCartesianPoint;
    template <typename TList> class ListOfListOfIfcCartesianPointSerializer : public AggrSerializerAggr<TList, ListOfIfcCartesianPoint, ListOfIfcCartesianPointSerializer<ListOfIfcCartesianPoint>> {};
    typedef std::list<IfcLengthMeasure> ListOfIfcLengthMeasure;
    template <typename TList> class ListOfIfcLengthMeasureSerializer : public AggrSerializerSimple<TList, IfcLengthMeasure, sdaiREAL> {};
    typedef std::list<ListOfIfcLengthMeasure> ListOfListOfIfcLengthMeasure;
    template <typename TList> class ListOfListOfIfcLengthMeasureSerializer : public AggrSerializerAggr<TList, ListOfIfcLengthMeasure, ListOfIfcLengthMeasureSerializer<ListOfIfcLengthMeasure>> {};
    typedef std::list<StringValue> ListOfIfcLabel;
    template <typename TList> class ListOfIfcLabelSerializer : public AggrSerializerText<ListOfIfcLabel, IfcLabel, sdaiSTRING> {};
    typedef std::list<IfcRelAssociatesClassification> SetOfIfcRelAssociatesClassification;
    template <typename TList> class SetOfIfcRelAssociatesClassificationSerializer : public AggrSerializerInstance<TList, IfcRelAssociatesClassification> {};
    typedef std::list<IfcClassificationReference> SetOfIfcClassificationReference;
    template <typename TList> class SetOfIfcClassificationReferenceSerializer : public AggrSerializerInstance<TList, IfcClassificationReference> {};
    typedef std::list<IfcFace> SetOfIfcFace;
    template <typename TList> class SetOfIfcFaceSerializer : public AggrSerializerInstance<TList, IfcFace> {};
    typedef std::list<IfcNormalisedRatioMeasure> ListOfIfcNormalisedRatioMeasure;
    template <typename TList> class ListOfIfcNormalisedRatioMeasureSerializer : public AggrSerializerSimple<TList, IfcNormalisedRatioMeasure, sdaiREAL> {};
    typedef std::list<ListOfIfcNormalisedRatioMeasure> ListOfListOfIfcNormalisedRatioMeasure;
    template <typename TList> class ListOfListOfIfcNormalisedRatioMeasureSerializer : public AggrSerializerAggr<TList, ListOfIfcNormalisedRatioMeasure, ListOfIfcNormalisedRatioMeasureSerializer<ListOfIfcNormalisedRatioMeasure>> {};
    typedef std::list<IfcPropertySet> SetOfIfcPropertySet;
    template <typename TList> class SetOfIfcPropertySetSerializer : public AggrSerializerInstance<TList, IfcPropertySet> {};
    typedef std::list<IfcPropertyDependencyRelationship> SetOfIfcPropertyDependencyRelationship;
    template <typename TList> class SetOfIfcPropertyDependencyRelationshipSerializer : public AggrSerializerInstance<TList, IfcPropertyDependencyRelationship> {};
    typedef std::list<IfcComplexProperty> SetOfIfcComplexProperty;
    template <typename TList> class SetOfIfcComplexPropertySerializer : public AggrSerializerInstance<TList, IfcComplexProperty> {};
    typedef std::list<IfcResourceConstraintRelationship> SetOfIfcResourceConstraintRelationship;
    template <typename TList> class SetOfIfcResourceConstraintRelationshipSerializer : public AggrSerializerInstance<TList, IfcResourceConstraintRelationship> {};
    typedef std::list<IfcProperty> SetOfIfcProperty;
    template <typename TList> class SetOfIfcPropertySerializer : public AggrSerializerInstance<TList, IfcProperty> {};
    typedef std::list<IfcComplexPropertyTemplate> SetOfIfcComplexPropertyTemplate;
    template <typename TList> class SetOfIfcComplexPropertyTemplateSerializer : public AggrSerializerInstance<TList, IfcComplexPropertyTemplate> {};
    typedef std::list<IfcPropertySetTemplate> SetOfIfcPropertySetTemplate;
    template <typename TList> class SetOfIfcPropertySetTemplateSerializer : public AggrSerializerInstance<TList, IfcPropertySetTemplate> {};
    typedef std::list<IfcPropertyTemplate> SetOfIfcPropertyTemplate;
    template <typename TList> class SetOfIfcPropertyTemplateSerializer : public AggrSerializerInstance<TList, IfcPropertyTemplate> {};
    typedef std::list<IfcCompositeCurve> SetOfIfcCompositeCurve;
    template <typename TList> class SetOfIfcCompositeCurveSerializer : public AggrSerializerInstance<TList, IfcCompositeCurve> {};
    typedef std::list<IfcProfileDef> SetOfIfcProfileDef;
    template <typename TList> class SetOfIfcProfileDefSerializer : public AggrSerializerInstance<TList, IfcProfileDef> {};
    typedef std::list<IfcRelAssignsToResource> SetOfIfcRelAssignsToResource;
    template <typename TList> class SetOfIfcRelAssignsToResourceSerializer : public AggrSerializerInstance<TList, IfcRelAssignsToResource> {};
    typedef std::list<IfcRepresentationContext> SetOfIfcRepresentationContext;
    template <typename TList> class SetOfIfcRepresentationContextSerializer : public AggrSerializerInstance<TList, IfcRepresentationContext> {};
    typedef std::list<IfcCoordinateOperation> SetOfIfcCoordinateOperation;
    template <typename TList> class SetOfIfcCoordinateOperationSerializer : public AggrSerializerInstance<TList, IfcCoordinateOperation> {};
    typedef std::list<IfcCostValue> ListOfIfcCostValue;
    template <typename TList> class ListOfIfcCostValueSerializer : public AggrSerializerInstance<TList, IfcCostValue> {};
    typedef std::list<IfcPhysicalQuantity> ListOfIfcPhysicalQuantity;
    template <typename TList> class ListOfIfcPhysicalQuantitySerializer : public AggrSerializerInstance<TList, IfcPhysicalQuantity> {};
    typedef std::list<IfcRelCoversSpaces> SetOfIfcRelCoversSpaces;
    template <typename TList> class SetOfIfcRelCoversSpacesSerializer : public AggrSerializerInstance<TList, IfcRelCoversSpaces> {};
    typedef std::list<IfcBoundaryCurve> SetOfIfcBoundaryCurve;
    template <typename TList> class SetOfIfcBoundaryCurveSerializer : public AggrSerializerInstance<TList, IfcBoundaryCurve> {};
    typedef std::list<IfcCurveStyleFontPattern> ListOfIfcCurveStyleFontPattern;
    template <typename TList> class ListOfIfcCurveStyleFontPatternSerializer : public AggrSerializerInstance<TList, IfcCurveStyleFontPattern> {};
    typedef std::list<IfcActorSelect> SetOfIfcActorSelect;
    template <typename TList> class SetOfIfcActorSelectSerializer : public AggrSerializerSelect<TList, IfcActorSelect> {};
    typedef std::list<IfcRelAssociatesDocument> SetOfIfcRelAssociatesDocument;
    template <typename TList> class SetOfIfcRelAssociatesDocumentSerializer : public AggrSerializerInstance<TList, IfcRelAssociatesDocument> {};
    typedef std::list<IfcDocumentReference> SetOfIfcDocumentReference;
    template <typename TList> class SetOfIfcDocumentReferenceSerializer : public AggrSerializerInstance<TList, IfcDocumentReference> {};
    typedef std::list<IfcDocumentInformationRelationship> SetOfIfcDocumentInformationRelationship;
    template <typename TList> class SetOfIfcDocumentInformationRelationshipSerializer : public AggrSerializerInstance<TList, IfcDocumentInformationRelationship> {};
    typedef std::list<IfcRelAssociatesDataset> SetOfIfcRelAssociatesDataset;
    template <typename TList> class SetOfIfcRelAssociatesDatasetSerializer : public AggrSerializerInstance<TList, IfcRelAssociatesDataset> {};
    typedef std::list<IfcDatasetReference> SetOfIfcDatasetReference;
    template <typename TList> class SetOfIfcDatasetReferenceSerializer : public AggrSerializerInstance<TList, IfcDatasetReference> {};
    typedef std::list<IfcDerivedUnitElement> SetOfIfcDerivedUnitElement;
    template <typename TList> class SetOfIfcDerivedUnitElementSerializer : public AggrSerializerInstance<TList, IfcDerivedUnitElement> {};
    typedef std::list<IfcReal> ListOfIfcReal;
    template <typename TList> class ListOfIfcRealSerializer : public AggrSerializerSimple<TList, IfcReal, sdaiREAL> {};
    typedef std::list<IfcRelConnectsPorts> SetOfIfcRelConnectsPorts;
    template <typename TList> class SetOfIfcRelConnectsPortsSerializer : public AggrSerializerInstance<TList, IfcRelConnectsPorts> {};
    typedef std::list<IfcDocumentInformation> SetOfIfcDocumentInformation;
    template <typename TList> class SetOfIfcDocumentInformationSerializer : public AggrSerializerInstance<TList, IfcDocumentInformation> {};
    typedef std::list<IfcTypeObject> SetOfIfcTypeObject;
    template <typename TList> class SetOfIfcTypeObjectSerializer : public AggrSerializerInstance<TList, IfcTypeObject> {};
    typedef std::list<IfcRelDefinesByTemplate> SetOfIfcRelDefinesByTemplate;
    template <typename TList> class SetOfIfcRelDefinesByTemplateSerializer : public AggrSerializerInstance<TList, IfcRelDefinesByTemplate> {};
    typedef std::list<IfcOrientedEdge> ListOfIfcOrientedEdge;
    template <typename TList> class ListOfIfcOrientedEdgeSerializer : public AggrSerializerInstance<TList, IfcOrientedEdge> {};
    typedef std::list<IfcPhysicalQuantity> SetOfIfcPhysicalQuantity;
    template <typename TList> class SetOfIfcPhysicalQuantitySerializer : public AggrSerializerInstance<TList, IfcPhysicalQuantity> {};
    typedef std::list<IfcRelSequence> SetOfIfcRelSequence;
    template <typename TList> class SetOfIfcRelSequenceSerializer : public AggrSerializerInstance<TList, IfcRelSequence> {};
    typedef std::list<IfcRelAssignsToProcess> SetOfIfcRelAssignsToProcess;
    template <typename TList> class SetOfIfcRelAssignsToProcessSerializer : public AggrSerializerInstance<TList, IfcRelAssignsToProcess> {};
    typedef std::list<IfcResourceObjectSelect> SetOfIfcResourceObjectSelect;
    template <typename TList> class SetOfIfcResourceObjectSelectSerializer : public AggrSerializerSelect<TList, IfcResourceObjectSelect> {};
    typedef std::list<IfcConnectedFaceSet> SetOfIfcConnectedFaceSet;
    template <typename TList> class SetOfIfcConnectedFaceSetSerializer : public AggrSerializerInstance<TList, IfcConnectedFaceSet> {};
    typedef std::list<IfcFillStyleSelect> SetOfIfcFillStyleSelect;
    template <typename TList> class SetOfIfcFillStyleSelectSerializer : public AggrSerializerSelect<TList, IfcFillStyleSelect> {};
    typedef std::list<IfcVector> ListOfIfcVector;
    template <typename TList> class ListOfIfcVectorSerializer : public AggrSerializerInstance<TList, IfcVector> {};
    typedef std::list<IfcGeometricSetSelect> SetOfIfcGeometricSetSelect;
    template <typename TList> class SetOfIfcGeometricSetSelectSerializer : public AggrSerializerSelect<TList, IfcGeometricSetSelect> {};
    typedef std::list<IfcRepresentation> SetOfIfcRepresentation;
    template <typename TList> class SetOfIfcRepresentationSerializer : public AggrSerializerInstance<TList, IfcRepresentation> {};
    typedef std::list<IfcGeometricRepresentationSubContext> SetOfIfcGeometricRepresentationSubContext;
    template <typename TList> class SetOfIfcGeometricRepresentationSubContextSerializer : public AggrSerializerInstance<TList, IfcGeometricRepresentationSubContext> {};
    typedef std::list<IfcGridAxis> ListOfIfcGridAxis;
    template <typename TList> class ListOfIfcGridAxisSerializer : public AggrSerializerInstance<TList, IfcGridAxis> {};
    typedef std::list<IfcGrid> SetOfIfcGrid;
    template <typename TList> class SetOfIfcGridSerializer : public AggrSerializerInstance<TList, IfcGrid> {};
    typedef std::list<IfcVirtualGridIntersection> SetOfIfcVirtualGridIntersection;
    template <typename TList> class SetOfIfcVirtualGridIntersectionSerializer : public AggrSerializerInstance<TList, IfcVirtualGridIntersection> {};
    typedef std::list<IfcProduct> SetOfIfcProduct;
    template <typename TList> class SetOfIfcProductSerializer : public AggrSerializerInstance<TList, IfcProduct> {};
    typedef std::list<IfcObjectPlacement> SetOfIfcObjectPlacement;
    template <typename TList> class SetOfIfcObjectPlacementSerializer : public AggrSerializerInstance<TList, IfcObjectPlacement> {};
    typedef std::list<IfcPositiveInteger> ListOfIfcPositiveInteger;
    template <typename TList> class ListOfIfcPositiveIntegerSerializer : public AggrSerializerSimple<TList, IfcPositiveInteger, sdaiINTEGER> {};
    typedef std::list<IfcSegmentIndexSelect> ListOfIfcSegmentIndexSelect;
    template <typename TList> class ListOfIfcSegmentIndexSelectSerializer : public AggrSerializerSelect<TList, IfcSegmentIndexSelect> {};
    typedef std::list<IfcPolygonalFaceSet> SetOfIfcPolygonalFaceSet;
    template <typename TList> class SetOfIfcPolygonalFaceSetSerializer : public AggrSerializerInstance<TList, IfcPolygonalFaceSet> {};
    typedef std::list<ListOfIfcPositiveInteger> ListOfListOfIfcPositiveInteger;
    template <typename TList> class ListOfListOfIfcPositiveIntegerSerializer : public AggrSerializerAggr<TList, ListOfIfcPositiveInteger, ListOfIfcPositiveIntegerSerializer<ListOfIfcPositiveInteger>> {};
    typedef std::list<IfcSurfaceTexture> ListOfIfcSurfaceTexture;
    template <typename TList> class ListOfIfcSurfaceTextureSerializer : public AggrSerializerInstance<TList, IfcSurfaceTexture> {};
    typedef std::list<IfcInteger> ArrayOfIfcInteger;
    template <typename TList> class ArrayOfIfcIntegerSerializer : public AggrSerializerSimple<TList, IfcInteger, sdaiINTEGER> {};
    typedef std::list<IfcPcurve> ListOfIfcPcurve;
    template <typename TList> class ListOfIfcPcurveSerializer : public AggrSerializerInstance<TList, IfcPcurve> {};
    typedef std::list<IfcIrregularTimeSeriesValue> ListOfIfcIrregularTimeSeriesValue;
    template <typename TList> class ListOfIfcIrregularTimeSeriesValueSerializer : public AggrSerializerInstance<TList, IfcIrregularTimeSeriesValue> {};
    typedef std::