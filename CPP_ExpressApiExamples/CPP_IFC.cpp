//
#include <iostream>

#include "..\EngineEx_Template.h"
#include "IFC4.h"
#include "IFC2x3.h"
#include "IFC4x3.h"
#include "CIS2.h"

using namespace IFC4;

#ifndef ASSERT
#define ASSERT assert
#endif

extern void IFC4_test()
{
    int_t  ifcModel = sdaiCreateModelBN(0, NULL, "IFC4");
    ASSERT(ifcModel);
    SetSPFFHeaderItem(ifcModel, 9, 0, sdaiSTRING, "IFC4");
    SetSPFFHeaderItem(ifcModel, 9, 1, sdaiSTRING, 0);

    auto ownerHistory = IfcOwnerHistory::Create(ifcModel);

    auto wall = IfcWall::Create(ifcModel);

    IfcGloballyUniqueId guid = wall.get_GlobalId();
    IfcLabel name = wall.get_Name();
    IfcText descr = wall.get_Description();
    IfcOwnerHistory oh = wall.get_OwnerHistory();
    auto predType = wall.get_PredefinedType();
    ASSERT(!descr && !name && !guid && !oh && predType.IsNull());

    wall.put_GlobalId("7-7-7");
    wall.put_Name("Wall name");
    wall.put_Description("My wall description");
    wall.put_OwnerHistory(ownerHistory);
    wall.put_PredefinedType(IfcWallTypeEnum::POLYGONAL);

    guid = wall.get_GlobalId();
    name = wall.get_Name();
    descr = wall.get_Description();
    oh = wall.get_OwnerHistory();
    predType = wall.get_PredefinedType();
    ASSERT(!strcmp(descr, "My wall description")
           && !strcmp(name, "Wall name")
           && !strcmp(guid, "7-7-7")
           && oh == ownerHistory
           && predType.Value() == IfcWallTypeEnum::POLYGONAL
    );

    auto profile = IfcRectangleProfileDef::Create(ifcModel);
    auto xdim = profile.get_XDim();
    auto ydim = profile.get_YDim();
    ASSERT(xdim.IsNull() && ydim.IsNull());
    profile.put_XDim(10000);
    profile.put_YDim(80);
    xdim = profile.get_XDim();
    ydim = profile.get_YDim();
    ASSERT(xdim.Value() == 10000 && ydim.Value() == 80);


    IfcTriangulatedFaceSet triangFaceSet = IfcTriangulatedFaceSet::Create(ifcModel);
    auto closed = triangFaceSet.get_Closed();
    ASSERT(closed.IsNull());
    triangFaceSet.put_Closed(false);
    closed = triangFaceSet.get_Closed();
    ASSERT(!closed.Value());

    IfcBSplineCurve curve = IfcBSplineCurveWithKnots::Create(ifcModel);
    ASSERT(curve.get_Degree().IsNull());
    curve.put_Degree(5);
    ASSERT(curve.get_Degree().Value() == 5);

    //type casting check
    IfcProduct product(wall);
    assert(product != 0);
    name = product.get_Name();
    ASSERT(!strcmp(name, "Wall name"));

    IfcBuilding building(wall);
    assert(building == 0);

    //
    // SELECT tests
    //

    //
    // various data type and nested selects
    //
    IfcMeasureWithUnit measureWithUnit = IfcMeasureWithUnit::Create(ifcModel);

    //numeric value (sequence notation)
    Nullable<IfcAbsorbedDoseMeasure> dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
    //Nullable<double> works also good
    ASSERT(dval.IsNull());

    //shortcuts methods
    auto as_double = measureWithUnit.get_ValueComponent().as_double();
    auto as_text = measureWithUnit.get_ValueComponent().as_text();
    auto as_int = measureWithUnit.get_ValueComponent().as_int();
    auto as_bool = measureWithUnit.get_ValueComponent().as_bool();
    ASSERT(as_double.IsNull() && as_text == NULL && as_int.IsNull() && as_bool.IsNull());

    //numeric value (alteranative notation)
    IfcMeasureValue_get getMeasureValue(measureWithUnit, "ValueComponent");
    dval = getMeasureValue.get_IfcRatioMeasure();
    ASSERT(dval.IsNull());

    // see below detached select test
    IfcMeasureValue measureValue_detachedSelect (measureWithUnit, "ValueComponent");
    dval = measureValue_detachedSelect.get_IfcRatioMeasure();
    ASSERT(dval.IsNull());

    //text based value
    IfcDescriptiveMeasure sval = getMeasureValue.get_IfcDescriptiveMeasure();
    ASSERT(sval == NULL);

    IfcText txt = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText();
    ASSERT(txt == NULL);

    //set to numeric
    measureWithUnit.put_ValueComponent().put_IfcMeasureValue().put_IfcRatioMeasure(0.5);

    dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
    ASSERT(dval.Value() == 0.5);

    sval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcDescriptiveMeasure();
    ASSERT(sval == NULL);

    txt = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText();
    ASSERT(txt == NULL);

    //check type methodt
    if (measureWithUnit.get_ValueComponent().get_IfcMeasureValue().is_IfcAreaMeasure()) {
        assert(false);
    }
    else if (!measureWithUnit.get_ValueComponent().get_IfcMeasureValue().is_IfcRatioMeasure()) {
        assert(false);
    }
    else if (measureWithUnit.get_ValueComponent().get_IfcSimpleValue().is_IfcText()) {
        assert(false);
    }
    else if (measureWithUnit.get_ValueComponent().get_IfcMeasureValue().is_IfcComplexNumber()) {
        assert(false);
    }

    //shortcuts methods
    as_double = measureWithUnit.get_ValueComponent().as_double();
    as_text = measureWithUnit.get_ValueComponent().as_text();
    as_int = measureWithUnit.get_ValueComponent().as_int();
    as_bool = measureWithUnit.get_ValueComponent().as_bool();
    ASSERT(as_double.Value() == 0.5 && 0 == strcmp(as_text, "0.500000") && as_int.Value() == 0 && as_bool.IsNull());

    //detached select behaviour
    //detached select is also changed when instance changed
    dval = measureValue_detachedSelect.get_IfcRatioMeasure();
    ASSERT(dval.Value()==0.5);
    //but changing the detached select will change host instance
    measureValue_detachedSelect.put_IfcAreaMeasure(2.7);
    dval = measureValue_detachedSelect.get_IfcAreaMeasure();
    ASSERT(dval.Value() == 2.7);
    //instance was changed
    dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
    ASSERT(dval.IsNull());
    dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcAreaMeasure();
    ASSERT(dval.Value()==2.7);

    //set DescriptiveMeasure
    measureWithUnit.put_ValueComponent().put_IfcMeasureValue().put_IfcDescriptiveMeasure("my descreptive measure");

    dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
    ASSERT(dval.IsNull());

    sval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcDescriptiveMeasure();
    ASSERT(0 == strcmp(sval, "my descreptive measure"));

    txt = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText();
    ASSERT(txt == NULL);

    as_double = measureWithUnit.get_ValueComponent().as_double();
    as_text = measureWithUnit.get_ValueComponent().as_text();
    as_int = measureWithUnit.get_ValueComponent().as_int();
    as_bool = measureWithUnit.get_ValueComponent().as_bool();
    ASSERT(as_double.IsNull() && 0 == strcmp(as_text, "my descreptive measure") && as_int.IsNull() && as_bool.IsNull());

    //set text
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcText("my text");

    ASSERT(measureWithUnit.get_ValueComponent().get_IfcSimpleValue().is_IfcText());

    dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
    ASSERT(dval.IsNull());

    sval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcDescriptiveMeasure();
    ASSERT(sval == NULL);

    txt = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText();
    ASSERT(0 == strcmp(txt, "my text"));

    IfcComplexNumber complexVal;
    measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcComplexNumber(complexVal);
    ASSERT(complexVal.empty());

    as_double = measureWithUnit.get_ValueComponent().as_double();
    as_text = measureWithUnit.get_ValueComponent().as_text();
    as_int = measureWithUnit.get_ValueComponent().as_int();
    as_bool = measureWithUnit.get_ValueComponent().as_bool();
    ASSERT(as_double.IsNull() && 0 == strcmp(as_text, "my text") && as_int.IsNull() && as_bool.IsNull());

    //
    // entities select
    //
    auto actor = IfcActor::Create(ifcModel);

    auto person = actor.get_TheActor().get_IfcPerson();
    auto organization = actor.get_TheActor().get_IfcOrganization();
    ASSERT(person == 0 && organization == 0);

    SdaiInstance instance = actor.get_TheActor().as_instance();
    ASSERT(instance == 0);

    auto setPerson = IfcPerson::Create(ifcModel);
    setPerson.put_Identification("justApeson");

    actor.put_TheActor().put_IfcPerson(setPerson);
    person = actor.get_TheActor().get_IfcPerson();
    ASSERT(setPerson == person);

    organization = actor.get_TheActor().get_IfcOrganization();
    ASSERT(organization == 0);

    instance = actor.get_TheActor().as_instance();
    ASSERT(instance == person);

    ASSERT(actor.get_TheActor().is_IfcPerson());
    ASSERT(!actor.get_TheActor().is_IfcOrganization());

    //
    // LOGICAL VALUES
    //

    ASSERT(curve.get_ClosedCurve().IsNull());
    curve.put_ClosedCurve(IfcLogical::Unknown);
    ASSERT(curve.get_ClosedCurve().Value() == IfcLogical::Unknown);

    auto ifcLogical = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcLogical();
    ASSERT(ifcLogical.IsNull());
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcLogical(IfcLogical::True);
    ifcLogical = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcLogical();
    ASSERT(ifcLogical.Value() == IfcLogical::True);

    auto relIntersect = IfcRelInterferesElements::Create(ifcModel);
    ifcLogical = relIntersect.get_ImpliedOrder();
    ASSERT(ifcLogical.IsNull());
    relIntersect.put_ImpliedOrder(LOGICAL_VALUE::False);
    ifcLogical = relIntersect.get_ImpliedOrder();
    ASSERT(ifcLogical.Value() == LOGICAL_VALUE::False);

    //
    // Aggregations
    //

    //as defined type
    auto site = IfcSite::Create(ifcModel);

    IfcCompoundPlaneAngleMeasure longitude;
    site.get_RefLongitude(longitude);
    ASSERT(longitude.size() == 0);

    longitude.push_back(54);
    site.put_RefLongitude(longitude);

    longitude.clear();
    site.get_RefLongitude(longitude);
    ASSERT(longitude.size() == 1 && longitude.front() == 54);

    std::list<int_t> lstInt;
    site.get_RefLongitude(lstInt);
    ASSERT(lstInt.size() == 1 && lstInt.front() == 54);

    int64_t rint[] = {3,4};
    site.put_RefLongitude(rint, 2);

    longitude.clear();
    site.get_RefLongitude(longitude);
    ASSERT(longitude.size() == 2 && longitude.front() == 3 && longitude.back() == 4);

    //double unnamed
    auto point = IfcCartesianPoint::Create(ifcModel);

    ListOfIfcLengthMeasure coords;
    point.get_Coordinates(coords);
    ASSERT(coords.empty());

    double my2DPoint[] = {1.0,2.0}; //can use array to set
    point.put_Coordinates(my2DPoint, 2);

    coords.clear();
    point.get_Coordinates(coords);
    ASSERT(coords.size() == 2 && coords.front() == 1 && coords.back() == 2);

    coords.push_back(3);
    point.put_Coordinates(coords); //can use sdt::list to set
    ASSERT(coords.size() == 3 && coords.front() == 1 && coords.back() == 3);

    //string
    ListOfIfcLabel middleNames;
    person.get_MiddleNames(middleNames);
    ASSERT(middleNames.empty());

    const char* DaliMiddleNames[] = {"Domingo", "Felipe", "Jacinto"};
    person.put_MiddleNames(DaliMiddleNames, 3);

    person.get_MiddleNames(middleNames);
    ASSERT(middleNames.size() == 3);
    int i = 0;
    for (auto m : middleNames) {
        ASSERT(0 == strcmp(m, DaliMiddleNames[i++]));
    }

    //
    // LIST of LIST
    //
    auto pointList = IfcCartesianPointList3D::Create(ifcModel);

    ListOfListOfIfcLengthMeasure coordList;

    pointList.get_CoordList(coordList);
    ASSERT(coordList.empty());

    //point (1,0.1)
    coordList.push_back(ListOfIfcLengthMeasure());
    coordList.back().push_back(1);
    coordList.back().push_back(0);
    coordList.back().push_back(1);

    //point (0,1,0)
    coordList.push_back(ListOfIfcLengthMeasure());
    coordList.back().push_back(0);
    coordList.back().push_back(1);
    coordList.back().push_back(0);

    pointList.put_CoordList(coordList);

    ListOfListOfIfcLengthMeasure coordListCheck;
    pointList.get_CoordList(coordListCheck);
    ASSERT(coordList == coordListCheck);

    //
    // Aggregation in select 
    // 
    auto prop = IfcPropertySingleValue::Create(ifcModel);

    IfcComplexNumber cplxNum;
    prop.get_NominalValue().get_IfcMeasureValue().get_IfcComplexNumber(cplxNum);
    ASSERT(cplxNum.size() == 0);

    double cplx[] = {2.1, 1.5};
    prop.put_NominalValue().put_IfcMeasureValue().put_IfcComplexNumber(cplx, 2);

    prop.get_NominalValue().get_IfcMeasureValue().get_IfcComplexNumber(cplxNum);
    ASSERT(cplxNum.size() == 2 && cplxNum.front() == 2.1 && cplxNum.back() == 1.5);


    //
    //IndexedPolyCurve
    //
    auto poly = IfcIndexedPolyCurve::Create(ifcModel);

    ASSERT(poly.get_Points() == 0);

    ListOfIfcSegmentIndexSelect segments;
    poly.get_Segments(segments);
    ASSERT(segments.empty());

    //2D points
    double rpt[]={
        0,0,
        1,0,
        1,1,
        0,1
    };

    //indexes of line and arc;
    IfcPositiveInteger line[] = {0,1};
    IfcPositiveInteger arc[] = {1,2,3};

    //create points list
    //
    auto points = IfcCartesianPointList2D::Create(ifcModel);

    ListOfListOfIfcLengthMeasure lstCoords; 
    for (i = 0; i < 4; i++) {
        lstCoords.push_back(ListOfIfcLengthMeasure());
        for (int j = 0; j < 2; j++) {
            lstCoords.back().push_back(rpt[2 * i + j]);
        }
    }   

    points.put_CoordList(lstCoords);

    //create segments list
    //
    segments.clear();
    
    IfcSegmentIndexSelect segment (poly);
    segment.put_IfcLineIndex(line, 2);
    segments.push_back(segment);
    
    segment.put_IfcArcIndex(arc,3);
    segments.push_back(segment);

    //
    //
    poly.put_Segments(segments);
    poly.put_Points(points);
    poly.put_SelfIntersect(false);

    //
    // get and check
    //
    points = 0;
    coordList.clear();
    segments.clear();

    auto pts = poly.get_Points();
    points = IfcCartesianPointList2D(pts); //TODO isInstanceOf!
    ASSERT(points != 0);

    points.get_CoordList(coordList);
    ASSERT(coordList.size() == 4);
    i = 0;
    for (auto& coord : coordList) {
        ASSERT(coord.size() == 2);
        ASSERT(coord.front() == rpt[2 * i] && coord.back() == rpt[2 * i + 1]);
        i++;
    }

    poly.get_Segments(segments);
    ASSERT(segments.size() == 2);
    
    IfcArcIndex arcInd;
    IfcLineIndex lineInd;
    segments.front().get_IfcArcIndex(arcInd);
    segments.front().get_IfcLineIndex(lineInd);

    ASSERT(arcInd.empty());
    ASSERT(lineInd.size()==2 && lineInd.front()==0 && lineInd.back()==1);

    arcInd.clear();
    lineInd.clear();
    segments.back().get_IfcArcIndex(arcInd);
    segments.back().get_IfcLineIndex(lineInd);

    ASSERT(arcInd.size()==3 && arcInd.front()==1 && arcInd.back()==3);
    ASSERT(lineInd.empty());

    //append line
    lineInd.push_back(3);
    lineInd.push_back(0);
    segment.put_IfcLineIndex(lineInd);
    segments.push_back(segment);

    poly.put_Segments(segments);

    //check now
    segments.clear();
    poly.get_Segments(segments);
    ASSERT(segments.size() == 3);

    segment = segments.back();
    arcInd.clear();
    lineInd.clear();
    segment.get_IfcArcIndex(arcInd);
    segment.get_IfcLineIndex(lineInd);

    ASSERT(arcInd.empty());
    ASSERT(lineInd.size() == 2 && lineInd.front() == 3 && lineInd.back() == 0);

    ///
    /// Aggregation of instances
    /// 
    auto prodRepr = wall.get_Representation();
    ASSERT(prodRepr == 0);

    prodRepr = IfcProductDefinitionShape::Create(ifcModel);
    wall.put_Representation(prodRepr);
    ASSERT(wall.get_Representation() == prodRepr);

    ListOfIfcRepresentation lstRep;
    prodRepr.get_Representations(lstRep);
    ASSERT(lstRep.empty());

    auto repr = IfcShapeRepresentation::Create(ifcModel);
    lstRep.push_back(repr);
    prodRepr.put_Representations(lstRep);

    lstRep.clear();
    prodRepr.get_Representations(lstRep);
    ASSERT(lstRep.size() == 1 && lstRep.front() == repr);

    SetOfIfcRepresentationItem lstItems;
    repr.get_Items(lstItems);
    ASSERT(lstItems.size() == 0);

    lstItems.push_back(poly);
    lstItems.push_back(triangFaceSet);
    lstItems.push_back(curve);
    
    repr.put_Items(lstItems);

    lstItems.clear();
    repr.get_Items(lstItems);
    ASSERT(lstItems.size() == 3 && lstItems.front() == poly && lstItems.back() == curve);

    ///
    /// Defined type aggregation of instance
    auto relProps = IfcRelDefinesByProperties::Create(ifcModel);

    SetOfIfcObjectDefinition relObj;
    relProps.get_RelatedObjects(relObj);
    ASSERT(relObj.empty());

    relObj.push_back(wall);

    relProps.put_RelatedObjects(relObj);
    
    relObj.clear();
    relProps.get_RelatedObjects(relObj);
    ASSERT(relObj.size() == 1 && relObj.front() == wall);

    IfcPropertySetDefinitionSet psSet;
    relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinitionSet(psSet);
    ASSERT(psSet.size() == 0);

    ASSERT(relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinition() == 0);

    auto emptyPset = IfcPropertySet::Create(ifcModel);
    emptyPset.put_Name("Empty property set");

    relProps.put_RelatingPropertyDefinition().put_IfcPropertySetDefinition(emptyPset);
    ASSERT(relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinition() == emptyPset);
    relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinitionSet(psSet);
    ASSERT(psSet.size() == 0);

    psSet.push_back(emptyPset);
    relProps.put_RelatingPropertyDefinition().put_IfcPropertySetDefinitionSet(psSet);
    ASSERT(relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinition() == 0);
    psSet.clear();
    relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinitionSet(psSet);
    ASSERT(psSet.size() == 1 && psSet.front()==emptyPset);

    /// 
    /// 
    sdaiSaveModelBN(ifcModel, "Test.ifc");
    sdaiCloseModel(ifcModel);

    ifcModel = sdaiOpenModelBN(NULL, "Test.ifc", "IFC4");

    auto entityIfcRelDefinesByProperties = sdaiGetEntity(ifcModel, "IfcRelDefinesByProperties");
    assert(entityIfcRelDefinesByProperties);

    int_t* rels = sdaiGetEntityExtent(ifcModel, entityIfcRelDefinesByProperties);
    auto N_rels = sdaiGetMemberCount(rels);
    assert(N_rels == 1);
    for (i = 0; i < N_rels; i++) {

        int_t rel = 0;
        engiGetAggrElement(rels, i, sdaiINSTANCE, &rel);

        auto get = IfcRelDefinesByProperties(rel).get_RelatingPropertyDefinition();
        ASSERT(get.get_IfcPropertySetDefinition() == 0);
        psSet.clear();
        get.get_IfcPropertySetDefinitionSet(psSet);
        ASSERT(psSet.size() == 1);
        name = psSet.front().get_Name();
        ASSERT(!strcmp (name, "Empty property set"));
    }

    sdaiCloseModel(ifcModel);
}