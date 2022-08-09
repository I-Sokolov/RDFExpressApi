using System;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

using RDF;
using IFC4;

namespace CS_IFC
{
    class CS_IFC
    {
        public static void IFC4_test()
        {
            var ifcModel = ifcengine.sdaiCreateModelBN(0, (string)null, "IFC4");
            ASSERT(ifcModel!=0);
            ifcengine.SetSPFFHeaderItem(ifcModel, 9, 0, ifcengine.sdaiSTRING, "IFC4");
            ifcengine.SetSPFFHeaderItem(ifcModel, 9, 1, ifcengine.sdaiSTRING, (string)null);

            var ownerHistory = IfcOwnerHistory.Create(ifcModel);

            var wall = IfcWall.Create(ifcModel);

            var guid = wall.get_GlobalId();
            var name = wall.get_Name();
            var descr = wall.get_Description();
            IfcOwnerHistory oh = wall.get_OwnerHistory();
            var predType = wall.get_PredefinedType();
            ASSERT(descr==null && name==null && guid==null && oh==0 && predType==null);

            wall.put_GlobalId("7-7-7");
            wall.put_Name("Wall name");
            wall.put_Description("My wall description");
            wall.put_OwnerHistory(ownerHistory);
            wall.put_PredefinedType(IfcWallTypeEnum.POLYGONAL);

            guid = wall.get_GlobalId();
            name = wall.get_Name();
            descr = wall.get_Description();
            oh = wall.get_OwnerHistory();
            predType = wall.get_PredefinedType();
            ASSERT(descr == "My wall description"
                   && name == "Wall name"
                   && guid == "7-7-7"
                   && oh == ownerHistory
                   && predType.Value == IfcWallTypeEnum.POLYGONAL
            );

            var profile = IfcRectangleProfileDef.Create(ifcModel);
            var xdim = profile.get_XDim();
            var ydim = profile.get_YDim();
            ASSERT(xdim == null && ydim == null);
            profile.put_XDim(10000);
            profile.put_YDim(80);
            xdim = profile.get_XDim();
            ydim = profile.get_YDim();
            ASSERT(xdim.Value == 10000 && ydim.Value == 80);


            IfcTriangulatedFaceSet triangFaceSet = IfcTriangulatedFaceSet.Create(ifcModel);
            var closed = triangFaceSet.get_Closed();
            ASSERT(closed==null);
            triangFaceSet.put_Closed(false);
            closed = triangFaceSet.get_Closed();
            ASSERT(!closed.Value);

            IfcBSplineCurve curve = IfcBSplineCurveWithKnots.Create(ifcModel);
            ASSERT(!curve.get_Degree().HasValue);
            curve.put_Degree(5);
            ASSERT(curve.get_Degree().Value == 5);

            //type casting check
            var product = new IfcProduct (wall);
            ASSERT(product != 0);
            name = product.get_Name();
            ASSERT(name == "Wall name");

            IfcBuilding building = new IfcBuilding(wall);
            ASSERT(building == 0);

            //
            // SELECT tests
            //

            //
            // various data type and nested selects
            //
            IfcMeasureWithUnit measureWithUnit = IfcMeasureWithUnit.Create(ifcModel);

            //numeric value (sequence notation)
            double? dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
            //Nullable<double> works also good
            ASSERT(!dval.HasValue);

            //shortcuts methods
            var as_double = measureWithUnit.get_ValueComponent().as_double();
            var as_text = measureWithUnit.get_ValueComponent().as_text();
            var as_int = measureWithUnit.get_ValueComponent().as_int();
            var as_bool = measureWithUnit.get_ValueComponent().as_bool();
            ASSERT(!as_double.HasValue && as_text == null && as_int == null && as_bool == null);

            //numeric value (alteranative notation)
            var getMeasureValue = new IfcMeasureValue_get(measureWithUnit, "ValueComponent");
            dval = getMeasureValue.get_IfcRatioMeasure();
            ASSERT(dval == null);

            // see below detached select test
            var measureValue_detachedSelect = new IfcMeasureValue(measureWithUnit, "ValueComponent");
            dval = measureValue_detachedSelect.get_IfcRatioMeasure();
            ASSERT(!dval.HasValue);

            //text based value
            string sval = getMeasureValue.get_IfcDescriptiveMeasure();
            ASSERT(sval == null);

            string txt = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText();
            ASSERT(txt == null);

            //set to numeric
            measureWithUnit.put_ValueComponent().put_IfcMeasureValue().put_IfcRatioMeasure(0.5);

            dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
            ASSERT(dval.Value == 0.5);

            sval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcDescriptiveMeasure();
            ASSERT(sval == null);

            txt = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText();
            ASSERT(txt == null);

            //check type methodt
            if (measureWithUnit.get_ValueComponent().get_IfcMeasureValue().is_IfcAreaMeasure())
            {
                ASSERT(false);
            }
            else if (!measureWithUnit.get_ValueComponent().get_IfcMeasureValue().is_IfcRatioMeasure())
            {
                ASSERT(false);
            }
            else if (measureWithUnit.get_ValueComponent().get_IfcSimpleValue().is_IfcText())
            {
                ASSERT(false);
            }
            else if (measureWithUnit.get_ValueComponent().get_IfcMeasureValue().is_IfcComplexNumber())
            {
                ASSERT(false);
            }

            //shortcuts methods
            as_double = measureWithUnit.get_ValueComponent().as_double();
            as_text = measureWithUnit.get_ValueComponent().as_text();
            as_int = measureWithUnit.get_ValueComponent().as_int();
            as_bool = measureWithUnit.get_ValueComponent().as_bool();
            ASSERT(as_double.Value == 0.5 && as_text == "0.500000" && as_int.Value == 0 && !as_bool.HasValue);

            //detached select behaviour
            //detached select is also changed when instance changed
            dval = measureValue_detachedSelect.get_IfcRatioMeasure();
            ASSERT(dval! == 0.5);
            //but changing the detached select will change host instance
            measureValue_detachedSelect.put_IfcAreaMeasure(2.7);
            dval = measureValue_detachedSelect.get_IfcAreaMeasure();
            ASSERT(dval.Value == 2.7);
            //instance was changed
            dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
            ASSERT(dval == null);
            dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcAreaMeasure();
            ASSERT(dval! == 2.7);

            //set DescriptiveMeasure
            measureWithUnit.put_ValueComponent().put_IfcMeasureValue().put_IfcDescriptiveMeasure("my descreptive measure");

            dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
            ASSERT(dval==null);

            sval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcDescriptiveMeasure();
            ASSERT(sval == "my descreptive measure");

            txt = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText();
            ASSERT(txt == null);

            as_double = measureWithUnit.get_ValueComponent().as_double();
            as_text = measureWithUnit.get_ValueComponent().as_text();
            as_int = measureWithUnit.get_ValueComponent().as_int();
            as_bool = measureWithUnit.get_ValueComponent().as_bool();
            ASSERT(as_double == null && (as_text == "my descreptive measure") && as_int == null && as_bool==null) ;

            //set text
            measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcText("my text");

            ASSERT(measureWithUnit.get_ValueComponent().get_IfcSimpleValue().is_IfcText());

            dval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcRatioMeasure();
            ASSERT(dval==null);

            sval = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcDescriptiveMeasure();
            ASSERT(sval == null);

            txt = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText();
            ASSERT(txt== "my text");

            IfcComplexNumber complexVal = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcComplexNumber();
            ASSERT(complexVal.Count==0);

            as_double = measureWithUnit.get_ValueComponent().as_double();
            as_text = measureWithUnit.get_ValueComponent().as_text();
            as_int = measureWithUnit.get_ValueComponent().as_int();
            as_bool = measureWithUnit.get_ValueComponent().as_bool();
            ASSERT(as_double==null && (as_text == "my text") && as_int==null && as_bool==null);

            //
            // simple aggrgations in select
            //
            double[] arrDouble = { 2, 5 };
            measureWithUnit.put_ValueComponent().put_IfcMeasureValue().put_IfcComplexNumber(arrDouble);
            complexVal = measureWithUnit.get_ValueComponent().get_IfcMeasureValue().get_IfcComplexNumber();
            ASSERT(complexVal.Count==2 && complexVal[0]==2 && complexVal[1]==5);

            //
            // entities select
            //
            var actor = IfcActor.Create(ifcModel);

            var person = actor.get_TheActor().get_IfcPerson();
            var organization = actor.get_TheActor().get_IfcOrganization();
            ASSERT(person == 0 && organization == 0);

            Int64 instance = actor.get_TheActor().as_instance();
            ASSERT(instance == 0);

            var setPerson = IfcPerson.Create(ifcModel);
            setPerson.put_Identification("justApeson");

            actor.put_TheActor().put_IfcPerson(setPerson);
            person = actor.get_TheActor().get_IfcPerson();
            ASSERT(setPerson == person);

            organization = actor.get_TheActor().get_IfcOrganization();
            ASSERT(organization == 0);

            instance = actor.get_TheActor().as_instance();
            ASSERT(instance == person);

            ASSERT(person.get_Identification() == "justApeson");

            ASSERT(actor.get_TheActor().is_IfcPerson());
            ASSERT(!actor.get_TheActor().is_IfcOrganization());

            //
            // LOGICAL VALUES
            //
            ASSERT(curve.get_ClosedCurve()==null);
            curve.put_ClosedCurve(LOGICAL_VALUE.Unknown);
            ASSERT(curve.get_ClosedCurve().Value == LOGICAL_VALUE.Unknown);

            var ifcLogical = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcLogical();
            ASSERT(ifcLogical==null);
            measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcLogical(LOGICAL_VALUE.True);
            ifcLogical = measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcLogical();
            ASSERT(ifcLogical.Value == LOGICAL_VALUE.True);

            var relIntersect = IfcRelInterferesElements.Create(ifcModel);
            ifcLogical = relIntersect.get_ImpliedOrder();
            ASSERT(ifcLogical==null);
            relIntersect.put_ImpliedOrder(LOGICAL_VALUE.False);
            ifcLogical = relIntersect.get_ImpliedOrder();
            ASSERT(ifcLogical.Value == LOGICAL_VALUE.False);

            //
            // Aggregations
            //

            //as defined type
            var site = IfcSite.Create(ifcModel);

            IfcCompoundPlaneAngleMeasure longitude;
            longitude = site.get_RefLongitude();
            ASSERT(longitude.Count == 0);

            longitude = new IfcCompoundPlaneAngleMeasure();
            longitude.Add(54);
            site.put_RefLongitude(longitude);

            longitude = site.get_RefLongitude();
            ASSERT(longitude.Count == 1 && longitude[0] == 54);


            Int64[] rint = { 3, 4 };
            site.put_RefLongitude(rint);

            longitude = site.get_RefLongitude();
            ASSERT(longitude.Count == 2 && longitude[0] == 3 && longitude[1] == 4);

            //double unnamed
            var point = IfcCartesianPoint.Create(ifcModel);

            ListOfIfcLengthMeasure coords = point.get_Coordinates();
            ASSERT(coords.Count==0);

            double[] my2DPoint = { 1.0, 2.0 }; //can use array to set
            point.put_Coordinates(my2DPoint);

            coords = point.get_Coordinates();
            ASSERT(coords.Count == 2 && coords[0] == 1 && coords[1] == 2);

            coords.Add(3);
            point.put_Coordinates(coords); //can use sdt.list to set
            ASSERT(coords.Count == 3 && coords[0] == 1 && coords[2] == 3);

            //string
            ListOfIfcLabel middleNames = person.get_MiddleNames();
            ASSERT(middleNames.Count==0);

            string[] DaliMiddleNames = { "Domingo", "Felipe", "Jacinto" };
            person.put_MiddleNames(DaliMiddleNames);

            middleNames = person.get_MiddleNames();
            ASSERT(middleNames.Count == 3);
            int i = 0;
            foreach (var m in middleNames)
            {
                ASSERT(m == DaliMiddleNames[i++]);
            }

            //
            // LIST of LIST
            //
            var pointList = IfcCartesianPointList3D.Create(ifcModel);

            ListOfListOfIfcLengthMeasure coordList = pointList.get_CoordList();
            ASSERT(coordList.Count==0);

            //point (1,0.1)
            coordList.Add(new ListOfIfcLengthMeasure());
            coordList.Last().Add(1);
            coordList.Last().Add(0);
            coordList.Last().Add(1);

            //point (0,1,0)
            coordList.Add(new ListOfIfcLengthMeasure());
            coordList.Last().Add(0);
            coordList.Last().Add(1);
            coordList.Last().Add(0);

            pointList.put_CoordList(coordList);

            ListOfListOfIfcLengthMeasure coordListCheck = pointList.get_CoordList();
            ASSERT_EQ(coordList, coordListCheck);

            //
            // Aggregation in select 
            // 
            var prop = IfcPropertySingleValue.Create(ifcModel);

            IfcComplexNumber cplxNum = prop.get_NominalValue().get_IfcMeasureValue().get_IfcComplexNumber();
            ASSERT(cplxNum.Count == 0);

            double[] cplx = { 2.1, 1.5 };
            prop.put_NominalValue().put_IfcMeasureValue().put_IfcComplexNumber(cplx);

            cplxNum=prop.get_NominalValue().get_IfcMeasureValue().get_IfcComplexNumber();
            ASSERT(cplxNum.Count == 2 && cplxNum[0] == 2.1 && cplxNum[1] == 1.5);

            //
            //IndexedPolyCurve
            //
            var poly = IfcIndexedPolyCurve.Create(ifcModel);

            ASSERT(poly.get_Points() == 0);

            ListOfIfcSegmentIndexSelect gotSegments = poly.get_Segments();
            ASSERT(gotSegments.Count==0);

            //2D points
            double[] rpt ={
                        0,0,
                        1,0,
                        1,1,
                        0,1
            };

            //indexes of line and arc;
            Int64[] line = { 0, 1 };
            Int64[] arc = { 1, 2, 3 };

            //create points list
            //
            var points = IfcCartesianPointList2D.Create(ifcModel);

            var lstCoords = new ListOfListOfIfcLengthMeasure(); 
            for (i = 0; i < 4; i++)
            {
                lstCoords.Add(new ListOfIfcLengthMeasure());
                for (int j = 0; j < 2; j++)
                {
                    lstCoords.Last().Add(rpt[2 * i + j]);
                }
            }

            points.put_CoordList(lstCoords);

            //create segments list =
            //
            IfcSegmentIndexSelect[] putSegments = { new IfcSegmentIndexSelect(poly), new IfcSegmentIndexSelect(poly) };
            putSegments[0].put_IfcLineIndex(line);
            putSegments[1].put_IfcArcIndex(arc);

            //
            //
            poly.put_Segments(putSegments);
            poly.put_Points(points);
            poly.put_SelfIntersect(false);

            //
            // get and check
            //
            points = 0;

            var pts = poly.get_Points();
            points = new IfcCartesianPointList2D(pts); //TODO isInstanceOf!
            ASSERT(points != 0);

            building = new IfcBuilding(pts);
            ASSERT(building == 0);

            coordList = points.get_CoordList();
            ASSERT_EQ(coordList, lstCoords);

            gotSegments = poly.get_Segments();
            ASSERT(gotSegments.Count == 2);

            IfcArcIndex arcInd = gotSegments[0].get_IfcArcIndex();
            IfcLineIndex lineInd = gotSegments[0].get_IfcLineIndex();
            ASSERT(arcInd.Count == 0);
            ASSERT_EQ(lineInd, line);

            arcInd = gotSegments[1].get_IfcArcIndex();
            lineInd = gotSegments[1].get_IfcLineIndex();
            ASSERT_EQ(arcInd, arc);
            ASSERT(lineInd.Count==0);

            //append line
            var line2 = new IfcLineIndex();
            line2.Add(3);
            line2.Add(1);
            var segment3 = new IfcSegmentIndexSelect(poly);
            segment3.put_IfcLineIndex(line2);

            var putLstSegments = putSegments.ToList();
            putLstSegments.Add(segment3);

            poly.put_Segments(putLstSegments);

            //check now
            gotSegments = poly.get_Segments();
            ASSERT(gotSegments.Count == 3);

            arcInd = gotSegments[0].get_IfcArcIndex();
            lineInd = gotSegments[0].get_IfcLineIndex();
            ASSERT(arcInd.Count == 0);
            ASSERT_EQ(lineInd, line);

            arcInd = gotSegments[1].get_IfcArcIndex();
            lineInd = gotSegments[1].get_IfcLineIndex();
            ASSERT_EQ(arcInd, arc);
            ASSERT(lineInd.Count == 0);

            arcInd = gotSegments[2].get_IfcArcIndex();
            lineInd = gotSegments[2].get_IfcLineIndex();
            ASSERT(arcInd.Count==0);
            ASSERT_EQ(lineInd, line2);


            ///
            /// Aggregation of instances
            /// 
            var prodRepr = wall.get_Representation();
            ASSERT(prodRepr == 0);

            prodRepr = IfcProductDefinitionShape.Create(ifcModel);
            wall.put_Representation(prodRepr);
            ASSERT(wall.get_Representation() == prodRepr);

            ListOfIfcRepresentation lstRep = prodRepr.get_Representations();
            ASSERT(lstRep.Count==0);

            var repr = IfcShapeRepresentation.Create(ifcModel);
            lstRep.Add(repr);
            prodRepr.put_Representations(lstRep);

            lstRep = prodRepr.get_Representations();
            ASSERT(lstRep.Count == 1 && lstRep.First() == repr);

            SetOfIfcRepresentationItem lstItems = repr.get_Items();
            ASSERT(lstItems.Count == 0);

            lstItems.Add(poly);
            lstItems.Add(triangFaceSet);
            lstItems.Add(curve);

            repr.put_Items(lstItems);

            var lstGotItems = repr.get_Items();
            ASSERT_EQ(lstGotItems, lstItems);

            ///
            /// Defined type aggregation of instance
            var relProps = IfcRelDefinesByProperties.Create(ifcModel);

            SetOfIfcObjectDefinition relObj = relProps.get_RelatedObjects();
            ASSERT(relObj.Count==0);

            relObj.Add(wall);
            relProps.put_RelatedObjects(relObj);

            var relObjGot = relProps.get_RelatedObjects();
            ASSERT_EQ(relObj, relObjGot);

            IfcPropertySetDefinitionSet psSet = relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinitionSet();
            ASSERT(psSet.Count == 0);
            ASSERT(relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinition() == 0);

            var emptyPset = IfcPropertySet.Create(ifcModel);
            emptyPset.put_Name("Empty property set");

            relProps.put_RelatingPropertyDefinition().put_IfcPropertySetDefinition(emptyPset);
            ASSERT(relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinition() == emptyPset);

            psSet = relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinitionSet();
            ASSERT(psSet.Count == 0);

            psSet.Add(emptyPset);
            relProps.put_RelatingPropertyDefinition().put_IfcPropertySetDefinitionSet(psSet);
            ASSERT(relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinition() == 0);
            
            var psSetGot = relProps.get_RelatingPropertyDefinition().get_IfcPropertySetDefinitionSet();
            ASSERT_EQ(psSet, psSetGot);

            /// 
            /// 
            ifcengine.sdaiSaveModelBN(ifcModel, "Test.ifc");
            ifcengine.sdaiCloseModel(ifcModel);

            ifcModel = ifcengine.sdaiOpenModelBN(0, "Test.ifc", "IFC4");

            var entityIfcRelDefinesByProperties = ifcengine.sdaiGetEntity(ifcModel, "IfcRelDefinesByProperties");
            ASSERT(entityIfcRelDefinesByProperties!=0);

            var rels = ifcengine.sdaiGetEntityExtent(ifcModel, entityIfcRelDefinesByProperties);
            var N_rels = ifcengine.sdaiGetMemberCount(rels);
            ASSERT(N_rels == 1);
            for (i = 0; i < N_rels; i++)
            {

                Int64 rel = 0;
                ifcengine.engiGetAggrElement(rels, i, ifcengine.sdaiINSTANCE, out rel);

                var get = ((IfcRelDefinesByProperties)(rel)).get_RelatingPropertyDefinition();
                ASSERT(get.get_IfcPropertySetDefinition() == 0);
                psSet = get.get_IfcPropertySetDefinitionSet();
                ASSERT(psSet.Count == 1);
                name = psSet[0].get_Name();
                ASSERT(name == "Empty property set");
            }

            ifcengine.sdaiCloseModel(ifcModel);
        }

        private static void ASSERT(bool c)
        {
            Debug.Assert(c);
        }

        private static void ASSERT_EQ(IEnumerable lst1, IEnumerable lst2)
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
                    ASSERT(cmp1.Equals(cmp2));
                }
                else
                {
                    var lst11 = it1.Current as IEnumerable;
                    var lst22 = it2.Current as IEnumerable;
                    if (lst11 != null && lst22 != null)
                    {
                        ASSERT_EQ(lst11, lst22);
                    }
                    else
                    {
                        ASSERT(false); //no comparision is implemented
                    }
                }

                m1 = it1.MoveNext();
                m2 = it2.MoveNext();
            }

            ASSERT(!m1 && !m2);
        }

    }
}
