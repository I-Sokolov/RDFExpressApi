using System;
using System.Collections.Generic;
using System.Linq;
using System.Collections;

#if _WIN64
        using int_t = System.Int64;
#else
        using int_t = System.Int32;
#endif

namespace CS_IFC
{
    using SdaiInstance = int_t;

    class GuideExample
    {
        public static void Run()
        {

            int_t model = RDF.ifcengine.sdaiCreateModelBN(0, null as string, "IFC4");
            RDF.ifcengine.SetSPFFHeaderItem(model, 9, 0, RDF.ifcengine.sdaiSTRING, "IFC4");
            RDF.ifcengine.SetSPFFHeaderItem(model, 9, 1, RDF.ifcengine.sdaiSTRING, null as string);

            //
            // Create instances and use in SDAI
            //

            //use static Create method to create model instance
            IFC4.IfcWall wall = IFC4.IfcWall.Create(model);
            IFC4.IfcDoor door = IFC4.IfcDoor.Create(model);

            //whenever SDAI instance is required, model instance can be used with implicit conversion
            long ok = RDF.ifcengine.sdaiIsKindOfBN(wall, "IfcProduct");
            assert(ok!=0);

            SdaiInstance sdaiWall = wall;
            ok = RDF.ifcengine.sdaiIsKindOfBN(sdaiWall, "IfcSlab");
            assert(ok==0);

            //other way, if you have SDAI instance you can construct model instance of appripriate type

            IFC4.IfcProduct product = sdaiWall; 
            assert(product != 0); //check instance is valid

            //or if somebody loves C# syle
            product = new IFC4.IfcProduct (sdaiWall);
            assert(product!=0); 

            IFC4.IfcSlab slab = sdaiWall;
            assert(slab==0); //wall is not a slab

            //
            // use put_* and set_* methods to access attribute
            // Hint: in Visual Studio pay attention to inetlli-sense for possible methods help tool-tips for possible arguments
            // 

            wall.Name = "MyWall";
            assert(wall.Name == "MyWall");

            //
            // nullable values
            //

            // get_* method will return domain type if attribute domain includes null value
            string text = wall.Description;
            assert(text == null); //not set

            // but if null value outside of domain type, get_* method will return nullable<> type extension
            // use nullable.Isnull and nullable.Value

            double? width = door.OverallWidth;
            assert(width==null); //not set

            door.OverallWidth = 900;

            width = door.OverallWidth;
            assert(width!=null && width! == 900);

            // Hint: use var to simplify code

            var height = door.OverallHeight;
            assert(height==null); //not set

            //
            // Enumerations
            // enum class defined for each EXPRESS enumeration
            //

            wall.PredefinedType = IFC4.IfcWallTypeEnum.MOVABLE;

            // get_* methods will return nullable extension
            IFC4.IfcWallTypeEnum? wallPredefinedType = wall.PredefinedType;
            assert(wallPredefinedType! == IFC4.IfcWallTypeEnum.MOVABLE);

            //Hint: simplify with var
            var doorPredefinedType = door.PredefinedType;
            assert(doorPredefinedType==null);

            //
            // Definded types
            // Use C# base types EXPRESS defined types
            //

            /*IFC4.IfcPositiveLengthMeasure*/ double measureHeight = 2000;
            door.OverallHeight = measureHeight;

            //and to get
            /*IFC4.IfcPositiveLengthMeasure>*/ double? getMeasure = door.OverallHeight;
            assert(getMeasure! == 2000);

            //
            // SELECTs
            //

            var actor = IFC4.IfcActor.Create(model);
            var person = IFC4.IfcPerson.Create(model);
            var organisation = IFC4.IfcOrganization.Create(model);

            person.FamilyName = "Smith";
            organisation.Name = "FBI";

            //when you put a value to SELECT you shold specify type of the value
            //to do this, attribute put_* methods return a put-selector with method for each possible type

            IFC4.IfcActorSelect selector = actor.TheActor;

            assert(!actor.TheActor.is_IfcPerson);
            assert(!actor.TheActor.is_IfcOrganization);
            assert(!selector.is_IfcPerson);
            assert(!selector.is_IfcOrganization);

            //
            actor.TheActor.IfcOrganization = organisation;

            assert(!actor.TheActor.is_IfcPerson);
            assert(actor.TheActor.is_IfcOrganization);
            assert(!selector.is_IfcPerson);
            assert(selector.is_IfcOrganization);

            //or this form
            selector.IfcPerson = person;
            assert(actor.TheActor.is_IfcPerson);
            assert(!actor.TheActor.is_IfcOrganization);
            assert(selector.is_IfcPerson);
            assert(!selector.is_IfcOrganization);

            //similary, attribute get_* methods return a get-selector and you can inquire content
            var p = selector.IfcPerson;

            assert(selector.is_IfcPerson);
            assert(!selector.is_IfcOrganization);

            IFC4.IfcPerson gotPerson = selector.IfcPerson;
            assert(gotPerson == person);

            IFC4.IfcOrganization gotOrganization = selector.IfcOrganization;
            assert(gotOrganization == 0);

            //get-selector may provide a method to get as base C++ type without specifing IFC type
            SdaiInstance inst = selector.as_instance;

            //check instance class
            assert((IFC4.IfcPerson)(inst)!=0);
            assert((IFC4.IfcOrganization)(inst)==0);
            assert(RDF.ifcengine.sdaiIsKindOfBN(inst, "IfcPerson")!=0);

            //work with nested SELECT
            var measure = IFC4.IfcMeasureWithUnit.Create(model);

            //when put you have to specify type path
            measure.ValueComponent.IfcSimpleValue.IfcInteger = 75;

            //you can get with type path
            assert(measure.ValueComponent.IfcSimpleValue.is_IfcInteger);
            assert(!measure.ValueComponent.IfcMeasureValue.is_IfcAreaMeasure);

            var valueSelector = measure.ValueComponent; //you can save selector in a variable

            var gotInt = valueSelector.IfcSimpleValue.IfcInteger;
            assert(gotInt != null && gotInt! == 75);

            var gotArea = measure.ValueComponent.IfcMeasureValue.IfcAreaMeasure;
            assert(gotArea==null);

            //if you are not interested in type, you can get as C++ base type
            gotInt = valueSelector.as_int;
            assert(gotInt != null && gotInt! == 75);

            var gotDouble = valueSelector.as_double;
            assert(gotDouble != null && gotDouble! == 75);

            var gotText = measure.ValueComponent.as_text;
            assert(gotText != null && gotText == "75");

            var gotBool = valueSelector.as_bool;
            assert(gotBool==null); //IfcInteger is not convertable to bool

            //
            // AGGRAGATIONS
            // For each unnamed aggragaion there is a ListOf*, SetOf* or BagOf* class
            // For each named aggragation there is a lst class with the given name
            // For put_* and get_* methods you can use these lists or any list with converible elements
            // Additionaly for some put_* methods you can use C-arrays
            // Hint: in Visual Studio pay attention to help tool-tips for possible arguments
            //

            var site = IFC4.IfcSite.Create(model);

            //put/get as IFC defined type
            var planeAngle = new IFC4.IfcCompoundPlaneAngleMeasure();
            planeAngle.Add(44);
            planeAngle.Add(34);
            planeAngle.Add(3);

            site.put_RefLatitude (planeAngle);

            IFC4.IfcCompoundPlaneAngleMeasure gotPlaneAngle = site.RefLatitude;
            assert(gotPlaneAngle.Count == 3 && gotPlaneAngle.First() == 44 && gotPlaneAngle[1]==34 && gotPlaneAngle[2]==3);

            //to put you can use any IEnumerable of base type
            var lstInt = new List<int_t> () ;
            lstInt.Add(56);
            site.put_RefLatitude(lstInt);

            int_t[] arrInt = { 43, 17, 3, 4 };
            site.put_RefLongitude(arrInt);

            //get_* method will return a subclass of List<> of base type
            List<int_t> gotLongitued = site.RefLongitude;
            assert(gotLongitued.Count == 4 && gotLongitued[2] == 3);

            IFC4.IfcCompoundPlaneAngleMeasure gotLatitude = site.RefLatitude;
            assert(gotLatitude.Count == 1 && gotLatitude.First() == 56);

            //
            // RELATIONSHIPS is an example of aggregations of entities
            //
            var group = IFC4.IfcRelAssignsToGroup.Create(model);

            IFC4.IfcObjectDefinition[] gropObjects = { wall, site };
            group.put_RelatedObjects(gropObjects);

            IFC4.SetOfIfcObjectDefinition gotGroup = group.RelatedObjects;
            assert(gotGroup.Count == 2 && gotGroup.First() == wall && gotGroup.Last() == site);

            //
            // AGGERGATION OF SELECT
            // To put and get aggregation of SELECT use list of selector
            // When you create a selector you have to specify entity instance it will be used for
            //

            var propEnumValue = IFC4.IfcPropertyEnumeratedValue.Create(model);

            var lstValue = new IFC4.ListOfIfcValue();

            var value = new IFC4.IfcValue (propEnumValue);
            value.IfcSimpleValue.IfcLabel = "MyLabel";
            lstValue.Add(value);

            value = new IFC4.IfcValue(propEnumValue);
            value.IfcMeasureValue.IfcCountMeasure = 4;
            lstValue.Add(value);

            propEnumValue.put_EnumerationValues(lstValue);

            IFC4.ListOfIfcValue gotValues = propEnumValue.EnumerationValues;
            assert(gotValues.Count == 2);

            string v1 = gotValues.First().IfcSimpleValue.IfcLabel;
            assert(v1 != null && v1== "MyLabel");
            
            double? v2 = gotValues.Last().IfcMeasureValue.IfcCountMeasure;
            assert(v2 != null && v2! == 4);

            //
            // AGGREGATION OF AGGREGATION
            //
            var pointList = IFC4.IfcCartesianPointList3D.Create(model);

            var coordList = new IFC4.ListOfListOfIfcLengthMeasure ();

            //point (1,0.1)
            coordList.Add(new IFC4.ListOfIfcLengthMeasure());
            coordList.Last().Add(1);
            coordList.Last().Add(0);
            coordList.Last().Add(1);

            //point (0,1,0)
            coordList.Add(new IFC4.ListOfIfcLengthMeasure());
            coordList.Last().Add(0);
            coordList.Last().Add(1);
            coordList.Last().Add(0);

            pointList.put_CoordList(coordList);

            IFC4.ListOfListOfIfcLengthMeasure coordListCheck = pointList.CoordList;
            assert_equal(coordList, coordListCheck);

            //
            // SELECT OF AGGREGATION
            //
            var prop = IFC4.IfcPropertySingleValue.Create(model);

            double[] cplx = { 2.1, 1.5 };
            prop.NominalValue.IfcMeasureValue.put_IfcComplexNumber (cplx);

            IFC4.IfcComplexNumber cplxNum = prop.NominalValue.IfcMeasureValue.IfcComplexNumber;
            assert(cplxNum.Count == 2 && cplxNum.First() == 2.1 && cplxNum.Last() == 1.5);

            //
            // IndexedPolyCurve
            //
           int_t ifcPolyCurveInstance = CreateIndexedPolyCurve(model);
           ParseIndexedPolyCurve(ifcPolyCurveInstance);
        }

        private static void assert(bool ok)
        {
            System.Diagnostics.Debug.Assert(ok);
        }

        private static void assert_equal(IEnumerable lst1, IEnumerable lst2)
        {
            var it1 = lst1.GetEnumerator();
            var it2 = lst2.GetEnumerator();

            bool m1 = it1.MoveNext();
            bool m2 = it2.MoveNext();
            while (m1 && m2)
            {
                var cmp1 = it1.Current as IComparable;
                var cmp2 = it2.Current as IComparable;
                if (cmp1 != null && cmp2 != null)
                {
                    assert(cmp1.Equals(cmp2));
                }
                else
                {
                    var lst11 = it1.Current as IEnumerable;
                    var lst22 = it2.Current as IEnumerable;
                    if (lst11 != null && lst22 != null)
                    {
                        assert_equal(lst11, lst22);
                    }
                    else
                    {
                        assert(false); //no comparison is implemented
                    }
                }

                m1 = it1.MoveNext();
                m2 = it2.MoveNext();
            }

            assert(!m1 && !m2);
        }


        class Point2D : IFC4.ListOfIfcLengthMeasure { public Point2D(double x, double y) { Add(x); Add(y); } };

        private static IFC4.IfcIndexedPolyCurve CreateIndexedPolyCurve(int_t model)
        {
            var poly = IFC4.IfcIndexedPolyCurve.Create(model);
            ////////

            Point2D[] points2D = {
                    new Point2D (0,0),            //arc1
                    new Point2D(5457, -1272),
                    new Point2D(2240, -5586),    //line1
                    new Point2D(2227, -5900),    //arc2
                    new Point2D(5294, -260),
                    new Point2D(-240, 171)        //line2
            };

            var points = IFC4.IfcCartesianPointList2D.Create(model);
            points.put_CoordList(points2D);

            poly.Points = points;

            //////

            var arc1 = new IFC4.IfcSegmentIndexSelect(poly);
            int_t[] _arc1 = { 1, 2, 3 };
            arc1.put_IfcArcIndex(_arc1);

            var line1 = new IFC4.IfcSegmentIndexSelect(poly);
            int_t[] _line1 = { 3, 4 };
            line1.put_IfcLineIndex(_line1);

            var arc2 = new IFC4.IfcSegmentIndexSelect(poly);
            int_t[] _arc2 = { 4, 5, 6 };
            arc2.put_IfcArcIndex(_arc2);

            var line2 = new IFC4.IfcSegmentIndexSelect(poly);
            int_t[] _line2 = { 6, 1 };
            line2.put_IfcLineIndex(_line2);


            var segments = new IFC4.ListOfIfcSegmentIndexSelect();
            segments.Add(arc1);
            segments.Add(line1);
            segments.Add(arc2);
            segments.Add(line2);

            poly.put_Segments(segments);

            poly.SelfIntersect = false;

            return poly;
        }

        private static void ParseIndexedPolyCurve(int_t instance)
        {
            IFC4.IfcIndexedPolyCurve polyCurve = instance;
            if (polyCurve.IsNull)
            {
                System.Console.WriteLine("This is not a IfcIndexedPolyCurve");
                return;
            }
     
            System.Console.WriteLine("Dump {0}", polyCurve.EntityName);

            System.Console.WriteLine("---------------- Segments -------------------");
            int nseg = 0;
            foreach (var segment in polyCurve.Segments)
            {
                System.Console.Write("  #{0} ", nseg++);

                if (segment.is_IfcLineIndex)
                {
                    System.Console.Write("is line: ");
                    var line = segment.IfcLineIndex;
                    
                    foreach (var index in line)
                    {
                        System.Console.Write("{0} ", index);
                    }
                }
                else if (segment.is_IfcArcIndex) 
                {
                    System.Console.Write("is arc: ");
                    var arc = segment.IfcArcIndex;

                    foreach (var index in arc)
                    {
                        System.Console.Write("{0} ", index);
                    }
                }
                else 
                {
                    System.Console.WriteLine("ERROR");
                }

                System.Console.WriteLine();
            }

            System.Console.WriteLine("--------------------- Points -----------------------");
            IFC4.ListOfListOfIfcLengthMeasure coords = null;

            var points3D = new IFC4.IfcCartesianPointList3D(polyCurve.Points);
            if (!points3D.IsNull)
            {
                System.Console.WriteLine("  3D: ");
                coords = points3D.CoordList;
            }
            else
            {
                var points2D = new IFC4.IfcCartesianPointList2D(polyCurve.Points);
                if (!points2D.IsNull)
                {
                    System.Console.WriteLine("  2D: ");
                    coords = points2D.CoordList;
                }
                else
                {
                    System.Console.WriteLine("ERROR");
                }
            }

            int npt = 0;
            foreach (var pt in coords)
            {
                System.Console.Write("    [{0}]:", npt++);
                foreach (var v in pt)
                {
                    System.Console.Write("{0},", v);
                }
                System.Console.WriteLine();
            }
        }
    }
}
