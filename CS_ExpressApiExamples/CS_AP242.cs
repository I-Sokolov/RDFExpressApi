using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using AP242;

namespace CS_IFC
{
    class CS_AP242
    {
        public static void Run()
        {
            test_multi_parent();

            test_list3();
        }


        static void test_list3()
        {
            /// 
            /// Create list of list of list of double
            /// 
            long model = RDF.ifcengine.sdaiCreateModelBN(0, null as string, "AP242");
            RDF.ifcengine.SetSPFFHeaderItem(model, 9, 0, RDF.ifcengine.sdaiSTRING, "AP242");
            RDF.ifcengine.SetSPFFHeaderItem(model, 9, 1, RDF.ifcengine.sdaiSTRING, null as string);

            assert(model!=0);

            var bspline_volume = rational_b_spline_volume.Create(model);

            list_of_list_of_list_of_double weights = bspline_volume.get_weights_data();
            assert(weights.Count == 0);

            for (int i = 0; i < 2; i++)
            {

                weights.Add(new list_of_list_of_double());
                var list2 = weights.Last();

                for (int j = 0; j < 3; j++)
                {

                    list2.Add(new list_of_double());
                    var list = list2.Last();

                    for (int k = 0; k < 2; k++)
                    {
                        list.Add(100 * i + 10 * j + k);
                    }
                }
            }

            bspline_volume.put_weights_data(weights);

            //bag
            var segment = composite_curve_segment.Create(model);

            bag_of_composite_curve bag = segment.get_using_curves();
            assert(bag.Count==0);

            //defined types on selects
            var equiv = equivalence_notable_instance.Create(model);

            list_of_equivalence_detected_difference_select lstCompared = equiv.get_compared_elements();
            assert(lstCompared.Count == 0);

            var vertexPoint = vertex_point.Create(model);
            vertexPoint.put_name("Test vertex point");
            var valCompared1 = new equivalence_detected_difference_select(equiv);
            valCompared1._a3ms_inspected_equivalence_element_select().put_vertex_point(vertexPoint);
            lstCompared.Add(valCompared1);
            equiv.put_compared_elements(lstCompared);

            lstCompared = equiv.get_compared_elements();
            assert(lstCompared.Count!=0);
            var test = lstCompared.First()._a3ms_inspected_equivalence_element_select().get_vertex_point().get_name();
            assert(!strcmp(test, "Test vertex point"));

            //
            var prodDefOccur = product_definition_occurrence.Create(model);
            assert(prodDefOccur.get_definition().get_product_definition() == 0);

            var prodDef = product_definition.Create(model);
            prodDefOccur.put_definition().put_product_definition(prodDef);
            assert(prodDefOccur.get_definition().get_product_definition() == prodDef);

            //            
            var appliedUsageRights = applied_usage_right.Create(model);
            set_of_ir_usage_item lstUsageItems = appliedUsageRights.get_items();
            assert(lstUsageItems.Count == 0);


            var usageItem = applied_classification_assignment.Create(model);
            var role = classification_role.Create(model);
            role.put_name("Test role");
            usageItem.put_role(role);

            lstUsageItems.Add(new ir_usage_item(appliedUsageRights));
            lstUsageItems.Last().put_applied_classification_assignment(usageItem);

            appliedUsageRights.put_items(lstUsageItems);

            lstUsageItems = appliedUsageRights.get_items();
            assert(lstUsageItems.Count == 1);
            assert(lstUsageItems.Last().get_action() == 0);
            test = lstUsageItems.Last().get_applied_classification_assignment().get_role().get_name();
            assert(!strcmp(test, "Test role"));

            //            
            var listedLogical = listed_logical_data.Create(model);
            ListOfLOGICAL_VALUE lstLogical = listedLogical.get_values();
            assert(lstLogical.Count == 0);

            lstLogical.Add(LOGICAL_VALUE.True);
            lstLogical.Add(LOGICAL_VALUE.False);
            lstLogical.Add(LOGICAL_VALUE.Unknown);

            listedLogical.put_values(lstLogical);

            lstLogical = listedLogical.get_values();
            assert(lstLogical.Count == 3 && lstLogical.First() == LOGICAL_VALUE.True && lstLogical.Last() == LOGICAL_VALUE.Unknown);

            //
            var extreme = extreme_instance.Create(model);

            var dir = direction.Create(model);

            set_of_location_of_extreme_value_select setLocations = extreme.get_locations_of_extreme_value();
            assert(setLocations.Count == 0);

            setLocations.Add(new location_of_extreme_value_select(extreme));
            setLocations.Last()._inspected_shape_element_select().put_direction(dir);

            extreme.put_locations_of_extreme_value(setLocations);

            List<location_of_extreme_value_select> getLocations = extreme.get_locations_of_extreme_value();
            assert(getLocations.Count == 1 && getLocations[0]._inspected_shape_element_select().get_direction() == dir);

            //
            RDF.ifcengine.sdaiSaveModelBN(model, "Test.ap");
            RDF.ifcengine.sdaiCloseModel(model);

            /// 
            /// Now read
            /// 
            var modelRead = RDF.ifcengine.sdaiOpenModelBN(0, "Test.ap", "AP242");

            var entity = RDF.ifcengine.sdaiGetEntity(modelRead, "RATIONAL_B_SPLINE_VOLUME");
            assert(entity!=0);

            var volumes = RDF.ifcengine.sdaiGetEntityExtent(modelRead, entity);
            var N_volumes = RDF.ifcengine.sdaiGetMemberCount(volumes);
            assert(N_volumes == 1);
            for (long i = 0; i < N_volumes; i++)
            {

                long volume = 0;
                RDF.ifcengine.engiGetAggrElement(volumes, i, RDF.ifcengine.sdaiINSTANCE, out volume);

                list_of_list_of_list_of_double weights2 = ((rational_b_spline_volume)volume).get_weights_data();

                ASSERT_EQ_LST(weights, weights2);
            }

            RDF.ifcengine.sdaiCloseModel(modelRead);
        }

        static void test_multi_parent()
        {
            long model = RDF.ifcengine.sdaiCreateModelBN(0, null as string, "AP242");
            RDF.ifcengine.SetSPFFHeaderItem(model, 9, 0, RDF.ifcengine.sdaiSTRING, "AP242");
            RDF.ifcengine.SetSPFFHeaderItem(model, 9, 1, RDF.ifcengine.sdaiSTRING, null as string);

            //wrapper test
            var inst = a3m_equivalence_criterion_with_specified_elements.Create(model);
            string NAME = "sey Name";
            inst.put_name(NAME);

            list_of_oriented_edge edge= new list_of_oriented_edge();
            edge.Add (oriented_edge.Create(model));
            edge.Add (oriented_edge.Create(model));

            var eloop = edge_loop.Create(model);
            eloop.put_name(NAME);
            eloop.put_edge_list(edge);

            RDF.ifcengine.sdaiSaveModelBN(model, "Test.ap");
            RDF.ifcengine.sdaiCloseModel(model);

            /// Now read
            /// 
            var modelRead = RDF.ifcengine.sdaiOpenModelBN(0, "Test.ap", "AP242");

            var entity = RDF.ifcengine.sdaiGetEntity(modelRead, "a3m_equivalence_criterion_with_specified_elements");// "a3m_equivalence_criterion");
            assert(entity!=0);
            var items = RDF.ifcengine.sdaiGetEntityExtent(modelRead, entity);
            var N_items = RDF.ifcengine.sdaiGetMemberCount(items);
            assert(N_items == 1);
            for (long i = 0; i < N_items; i++)
            {

                long item = 0;
                RDF.ifcengine.engiGetAggrElement(items, i, RDF.ifcengine.sdaiINSTANCE, out item);

                var name = ((a3m_equivalence_criterion)item).get_name();
                assert(!strcmp(name, NAME));
            }

            entity = RDF.ifcengine.sdaiGetEntity(modelRead, "edge_loop");
            assert(entity!=0);
            items = RDF.ifcengine.sdaiGetEntityExtent(modelRead, entity);
            N_items = RDF.ifcengine.sdaiGetMemberCount(items);
            assert(N_items == 1);
            for (long i = 0; i < N_items; i++)
            {

                long item = 0;
                RDF.ifcengine.engiGetAggrElement(items, i, RDF.ifcengine.sdaiINSTANCE, out item);

                eloop = (edge_loop)item;
                var name = eloop.get_name();
                assert(name == NAME);

                var lst = eloop.get_edge_list();
                assert(lst.Count == 2);
            }


            RDF.ifcengine.sdaiCloseModel(modelRead);

        }

        static bool strcmp (string s1, string s2)
        {
            return s1 != s2;
        }

        static void assert(bool c)
        {
            System.Diagnostics.Debug.Assert(c);
        }
        private static void ASSERT_EQ_LST(IEnumerable lst1, IEnumerable lst2)
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
                        ASSERT_EQ_LST(lst11, lst22);
                    }
                    else
                    {
                        assert(false); //no comparision is implemented
                    }
                }

                m1 = it1.MoveNext();
                m2 = it2.MoveNext();
            }

            assert(!m1 && !m2);
        }

    }
}
