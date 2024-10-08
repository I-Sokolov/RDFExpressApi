﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using RDF;
using IFC4;

#if _WIN64
        using int_t = System.Int64;
#else
        using int_t = System.Int32;
#endif

namespace CS_IFC
{
    class HelloWall
    {
        public static void Run()
        {
            var model = ifcengine.sdaiCreateModelBN(0, null as string, "IFC4");
            ifcengine.SetSPFFHeaderItem(model, 9, 0, ifcengine.sdaiSTRING, "IFC4");
            ifcengine.SetSPFFHeaderItem(model, 9, 1, ifcengine.sdaiSTRING, null as string);

            //spatial structure
            //
            var project = IfcProject.Create(model);
            project.GlobalId = "1o1ykWxGT4ZxPjHNe4gayR";
            project.Name = "HelloWall project";
            project.Description = "Example to demonstract early-binding abilities";

            var ifcSite = IfcSite.Create(model);
            ifcSite.Name = "HelloWall site";
            SetupAggregation(model, project, ifcSite);

            var ifcBuilding = IfcBuilding.Create(model);
            ifcBuilding.Name = "HelloWall building";
            SetupAggregation(model, ifcSite, ifcBuilding);

            var ifcStory = IfcBuildingStorey.Create(model);
            ifcStory.Name = "My first storey";
            SetupAggregation(model, ifcBuilding, ifcStory);

            //wall
            //
            var wall = IfcWall.Create(model);
            wall.GlobalId = "2o1ykWxGT4ZxPjHNe4gayR";
            wall.Name = "My wall";
            wall.Description = "My wall description";
            wall.PredefinedType = IfcWallTypeEnum.SOLIDWALL;

            CreateGeometry(model, wall);

            SetupContainment(model, ifcStory, wall);

            ifcengine.sdaiSaveModelBN(model, "HelloWall.ifc");
            ifcengine.sdaiCloseModel(model);
        }

        private static void SetupAggregation(int_t model, IfcObjectDefinition aggregator, IfcObjectDefinition part)
        {
            var aggregate = IfcRelAggregates.Create(model);
            aggregate.RelatingObject = aggregator;

            var aggregated = new SetOfIfcObjectDefinition();
            aggregated.Add(part);
            aggregate.put_RelatedObjects (aggregated);
        }

        private static void SetupContainment(int_t model, IfcSpatialStructureElement spatialElement, IfcProduct product)
        {
            var contain = IfcRelContainedInSpatialStructure.Create(model);
            contain.RelatingStructure = spatialElement;

            var products = new SetOfIfcProduct();
            products.Add(product);
            contain.put_RelatedElements (products);
        }

        private static void CreateGeometry(int_t model, IfcWall wall)
        {
            IfcCurve footprint = CreateFootprintCurve(model);

            var profile = IfcArbitraryClosedProfileDef.Create(model);
            profile.OuterCurve = footprint;

            double[] zdir = { 0, 0, 1 };
            var zDir = IfcDirection.Create(model);
            zDir.put_DirectionRatios(zdir);

            var solid = IfcExtrudedAreaSolid.Create(model);
            solid.SweptArea = profile;
            solid.ExtrudedDirection = zDir;
            solid.Depth = 2500;

            var lstReprItems = new SetOfIfcRepresentationItem ();
            lstReprItems.Add(solid);

            var shapeRepr = IfcShapeRepresentation.Create(model);
            shapeRepr.RepresentationIdentifier = "Body";
            shapeRepr.RepresentationType = "SweptSolid";
            shapeRepr.put_Items (lstReprItems);

            var lstRepr = new ListOfIfcRepresentation();
            lstRepr.Add(shapeRepr);

            var prodShape = IfcProductDefinitionShape.Create(model);
            prodShape.put_Representations(lstRepr);

            wall.Representation = prodShape;
        }

        class Point2D : ListOfIfcLengthMeasure { public Point2D(double x, double y) { Add(x); Add(y); } };

        private static IfcIndexedPolyCurve CreateFootprintCurve(int_t model)
        {
            var poly = IfcIndexedPolyCurve.Create(model);
            ////////

            Point2D[] points2D = {
                    new Point2D (0,0),            //arc1
                    new Point2D(5457, -1272),
                    new Point2D(2240, -5586),    //line1
                    new Point2D(2227, -5900),    //arc2
                    new Point2D(5294, -260),
                    new Point2D(-240, 171)        //line2
            };

            var points = IfcCartesianPointList2D.Create(model);
            points.put_CoordList(points2D);

            poly.Points = points;

            //////

            var arc1 = new IfcSegmentIndexSelect(poly);
            int_t[] _arc1 = { 1, 2, 3 };
            arc1.put_IfcArcIndex(_arc1);

            var line1 = new IfcSegmentIndexSelect(poly);
            int_t[] _line1 = { 3, 4 };
            line1.put_IfcLineIndex(_line1);

            var arc2 = new IfcSegmentIndexSelect(poly);
            int_t[] _arc2 = { 4, 5, 6 };
            arc2.put_IfcArcIndex(_arc2);

            var line2 = new IfcSegmentIndexSelect(poly);
            int_t[] _line2 = { 6, 1 };
            line2.put_IfcLineIndex(_line2);


            var segments = new ListOfIfcSegmentIndexSelect();
            segments.Add(arc1);
            segments.Add(line1);
            segments.Add(arc2);
            segments.Add(line2);

            poly.put_Segments(segments);

            poly.SelfIntersect = false;

            return poly;
        }
    }
}
