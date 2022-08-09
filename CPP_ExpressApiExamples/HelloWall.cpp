
#include "IFC4.h"

using namespace IFC4;

static void SetupAggregation(int_t model, IfcObjectDefinition aggregator, IfcObjectDefinition part);
static void SetupContainment(int_t model, IfcSpatialStructureElement spatialElement, IfcProduct product);
static void CreateGeometry(int_t model, IfcWall wall);
static IfcIndexedPolyCurve CreateFootprintCurve(int_t model);

extern void HelloWall()
{
    int_t  model = sdaiCreateModelBN(0, NULL, "IFC4");
    SetSPFFHeaderItem(model, 9, 0, sdaiSTRING, "IFC4");
    SetSPFFHeaderItem(model, 9, 1, sdaiSTRING, 0);

    //spatial structure
    //
    auto project = IfcProject::Create(model);
    project.put_GlobalId("1o1ykWxGT4ZxPjHNe4gayR");
    project.put_Name("HelloWall project");
    project.put_Description("Example to demonstract early-binding abilities");
    
    auto ifcSite = IfcSite::Create(model);
    ifcSite.put_Name("HelloWall site");
    SetupAggregation(model, project, ifcSite);

    auto ifcBuilding = IfcBuilding::Create(model);
    ifcBuilding.put_Name("HelloWall building");
    SetupAggregation(model, ifcSite, ifcBuilding);

    auto ifcStory = IfcBuildingStorey::Create(model);
    ifcStory.put_Name("My first storey");
    SetupAggregation(model, ifcBuilding, ifcStory);

    //wall
    //
    auto wall = IfcWall::Create(model);
    wall.put_GlobalId("2o1ykWxGT4ZxPjHNe4gayR");
    wall.put_Name("My wall");
    wall.put_Description("My wall description");
    wall.put_PredefinedType(IfcWallTypeEnum::SOLIDWALL);

    CreateGeometry(model, wall);

    SetupContainment(model, ifcStory, wall);

    sdaiSaveModelBN(model, "HelloWall.ifc");
    sdaiCloseModel(model);
}

static void SetupAggregation(int_t model, IfcObjectDefinition aggregator, IfcObjectDefinition part)
{
    auto aggregate = IfcRelAggregates::Create(model);
    aggregate.put_RelatingObject(aggregator);

    SetOfIfcObjectDefinition aggregated;
    aggregated.push_back(part);
    aggregate.put_RelatedObjects(aggregated);
}

static void SetupContainment(int_t model, IfcSpatialStructureElement spatialElement, IfcProduct product)
{
    auto contain = IfcRelContainedInSpatialStructure::Create(model);
    contain.put_RelatingStructure(spatialElement);

    SetOfIfcProduct products;
    products.push_back(product);
    contain.put_RelatedElements(products);
}

static void CreateGeometry(int_t model, IfcWall wall)
{
    IfcCurve footprint = CreateFootprintCurve(model);

    auto profile = IfcArbitraryClosedProfileDef::Create(model);
    profile.put_OuterCurve(footprint);

    double zdir[] = {0,0,1};
    auto zDir = IfcDirection::Create(model);
    zDir.put_DirectionRatios(zdir, 3);

    auto solid = IfcExtrudedAreaSolid::Create(model);
    solid.put_SweptArea(profile);
    solid.put_ExtrudedDirection(zDir);
    solid.put_Depth(2500);

    SetOfIfcRepresentationItem lstReprItems;
    lstReprItems.push_back(solid);

    auto shapeRepr = IfcShapeRepresentation::Create(model);
    shapeRepr.put_RepresentationIdentifier("Body");
    shapeRepr.put_RepresentationType("SweptSolid");
    shapeRepr.put_Items(lstReprItems);

    ListOfIfcRepresentation lstRepr;
    lstRepr.push_back(shapeRepr);

    auto prodShape = IfcProductDefinitionShape::Create(model);
    prodShape.put_Representations(lstRepr);

    wall.put_Representation(prodShape);
}

static IfcIndexedPolyCurve CreateFootprintCurve(int_t model)
{
    auto poly = IfcIndexedPolyCurve::Create(model);
    ////////

    struct Point2D { double x, y; };
    Point2D points2D[] = {
        {0,0},            //arc1
        {5457, -1272},
        {2240, -5586},    //line1
        {2227, -5900},    //arc2
        {5294, -260},
        {-240, 171}        //line2
    };

    ListOfListOfIfcLengthMeasure lstCoords; 
    for (int i = 0; i < 6; i++) {
        lstCoords.push_back(ListOfIfcLengthMeasure());
        lstCoords.back().push_back(points2D[i].x);
        lstCoords.back().push_back(points2D[i].y);
    }

    auto points = IfcCartesianPointList2D::Create(model);
    points.put_CoordList(lstCoords);

    poly.put_Points(points);

    //////

    IfcSegmentIndexSelect arc1(poly);
    IfcPositiveInteger _arc1[] = {1,2,3};
    arc1.put_IfcArcIndex(_arc1, 3);

    IfcSegmentIndexSelect line1(poly);
    IfcPositiveInteger _line1[] = {3,4};
    line1.put_IfcLineIndex(_line1, 2);

    IfcSegmentIndexSelect arc2(poly);
    IfcPositiveInteger _arc2[] = {4,5,6};
    arc2.put_IfcArcIndex(_arc2, 3);

    IfcSegmentIndexSelect line2(poly);
    IfcPositiveInteger _line2[] = {6,1};
    line2.put_IfcLineIndex(_line2, 2);


    ListOfIfcSegmentIndexSelect segments;
    segments.push_back(arc1);
    segments.push_back(line1);
    segments.push_back(arc2);
    segments.push_back(line2);

    poly.put_Segments(segments);

    poly.put_SelfIntersect(false);

    return poly;
}