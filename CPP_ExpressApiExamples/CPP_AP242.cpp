
#include <vector>

#include "AP242.h"
using namespace AP242;

static void test_list3();
static void test_multi_parent();

extern void AP242_test()
{
    test_multi_parent();
    
    test_list3();
}


template <typename L> static void ASSERT_EQ_LST(L& lst1, L& lst2)
{
    assert(lst1.size() == lst2.size());
    auto it1 = lst1.begin();
    auto it2 = lst2.begin();
    for (; it1 != lst1.end() && it2 != lst2.end(); it1++, it2++) {
        assert(*it1 == *it2);
    }
}


template <typename L> static void ASSERT_EQ_LST2(L& lst1, L& lst2)
{
    assert(lst1.size() == lst2.size());
    auto it1 = lst1.begin();
    auto it2 = lst2.begin();
    for (; it1 != lst1.end() && it2 != lst2.end(); it1++, it2++) {
        ASSERT_EQ_LST(*it1, *it2);
    }
}


template <typename L> static void ASSERT_EQ_LST3(L& lst1, L& lst2)
{
    assert(lst1.size() == lst2.size());
    auto it1 = lst1.begin();
    auto it2 = lst2.begin();
    for (; it1 != lst1.end() && it2 != lst2.end(); it1++, it2++) {
        ASSERT_EQ_LST2(*it1, *it2);
    }
}


static void test_list3()
{
    /// 
    /// Create list of list of list of double
    /// 
    int_t  model = sdaiCreateModelBN(0, NULL, "AP242");
    SetSPFFHeaderItem(model, 9, 0, sdaiSTRING, "AP242");
    SetSPFFHeaderItem(model, 9, 1, sdaiSTRING, 0);

    assert(model);

    auto bspline_volume = rational_b_spline_volume::Create(model);

    list_of_list_of_list_of_double weights;

    bspline_volume.get_weights_data(weights);
    assert(weights.size() == 0);

    for (int i = 0; i < 2; i++) {

        weights.push_back(list_of_list_of_double());
        auto& list2 = weights.back();

        for (int j = 0; j < 3; j++) {

            list2.push_back(list_of_double());
            auto& list = list2.back();

            for (int k = 0; k < 2; k++) {
                list.push_back(100 * i + 10 * j + k);
            }
        }       
    }

    bspline_volume.put_weights_data(weights);

    //bag
    auto segment = composite_curve_segment::Create(model);

    bag_of_composite_curve bag;
    segment.get_using_curves(bag);
    assert(bag.empty());

    //defined types on selects
    auto equiv = equivalence_notable_instance::Create(model);
    
    list_of_equivalence_detected_difference_select lstCompared;
    equiv.get_compared_elements(lstCompared);
    assert(lstCompared.size() == 0);

    auto vertexPoint = vertex_point::Create(model);
    vertexPoint.put_name("Test vertex point");
    equivalence_detected_difference_select valCompared1 (equiv);
    valCompared1._a3ms_inspected_equivalence_element_select().put_vertex_point(vertexPoint);
    lstCompared.push_back(valCompared1);
    equiv.put_compared_elements(lstCompared);

    lstCompared.clear();
    equiv.get_compared_elements(lstCompared);
    assert(lstCompared.size());
    auto test = lstCompared.front()._a3ms_inspected_equivalence_element_select().get_vertex_point().get_name();
    assert(!strcmp(test, "Test vertex point"));

    //
    auto prodDefOccur = product_definition_occurrence::Create(model);
    assert(prodDefOccur.get_definition().get_product_definition() == 0);
    
    auto prodDef = product_definition::Create(model);
    prodDefOccur.put_definition().put_product_definition(prodDef);
    assert(prodDefOccur.get_definition().get_product_definition() == prodDef);

    //
    set_of_ir_usage_item lstUsageItems;
    auto appliedUsageRights = applied_usage_right::Create(model);
    appliedUsageRights.get_items(lstUsageItems);
    assert(lstUsageItems.size() == 0);


    auto usageItem = applied_classification_assignment::Create(model);
    auto role = classification_role::Create(model);
    role.put_name("Test role");
    usageItem.put_role(role);    

    lstUsageItems.push_back(ir_usage_item(appliedUsageRights));
    lstUsageItems.back().put_applied_classification_assignment(usageItem);

    appliedUsageRights.put_items(lstUsageItems);

    lstUsageItems.clear();
    appliedUsageRights.get_items(lstUsageItems);
    assert(lstUsageItems.size() == 1);
    assert(lstUsageItems.back().get_action() == 0);
    test = lstUsageItems.back().get_applied_classification_assignment().get_role().get_name();
    assert(!strcmp(test, "Test role"));

    //
    ListOfLOGICAL_VALUE lstLogical;
    auto listedLogical = listed_logical_data::Create(model);
    listedLogical.get_values(lstLogical);
    assert(lstLogical.size() == 0);

    lstLogical.push_back(LOGICAL_VALUE::True);
    lstLogical.push_back(LOGICAL_VALUE::False);
    lstLogical.push_back(LOGICAL_VALUE::Unknown);

    listedLogical.put_values(lstLogical);

    lstLogical.clear();
    listedLogical.get_values(lstLogical);
    assert(lstLogical.size() == 3 && lstLogical.front() == LOGICAL_VALUE::True && lstLogical.back() == LOGICAL_VALUE::Unknown);

    //
    auto extreme = extreme_instance::Create(model);
    
    auto dir = direction::Create(model);

    set_of_location_of_extreme_value_select setLocations;
    extreme.get_locations_of_extreme_value(setLocations);
    assert(setLocations.size() == 0);

    setLocations.push_back(location_of_extreme_value_select(extreme));
    setLocations.back()._inspected_shape_element_select().put_direction(dir);

    extreme.put_locations_of_extreme_value(setLocations);

    std::vector<location_of_extreme_value_select> getLocations;
    extreme.get_locations_of_extreme_value(getLocations);
    assert(getLocations.size() == 1 && getLocations[0]._inspected_shape_element_select().get_direction() == dir);

    //
    sdaiSaveModelBN(model, "Test.ap");
    sdaiCloseModel(model);

    /// 
    /// Now read
    /// 
    auto modelRead = sdaiOpenModelBN(NULL, "Test.ap", "AP242");
    
    auto entity = sdaiGetEntity(modelRead, "RATIONAL_B_SPLINE_VOLUME");
    assert(entity);

    auto volumes = sdaiGetEntityExtent(modelRead, entity);
    auto N_volumes = sdaiGetMemberCount(volumes);
    assert(N_volumes == 1);
    for (int_t i = 0; i < N_volumes; i++) {
     
        int_t volume = 0;
        engiGetAggrElement(volumes, i, sdaiINSTANCE, &volume);

        list_of_list_of_list_of_double weights2;
        rational_b_spline_volume(volume).get_weights_data(weights2);

        ASSERT_EQ_LST3(weights, weights2);
    }
    
    sdaiCloseModel(modelRead);
}

static void test_multi_parent()
{
    int_t  model = sdaiCreateModelBN(0, NULL, "AP242");
    SetSPFFHeaderItem(model, 9, 0, sdaiSTRING, "AP242");
    SetSPFFHeaderItem(model, 9, 1, sdaiSTRING, 0);

    //engine test
    int_t entity = sdaiGetEntity(model, "a3m_equivalence_criterion");
    assert(entity);
    assert(7 == engiGetEntityNoAttributes(entity));
    const char* rAttr[] =
        {"name","assessment_specification","comparing_element_types","compared_element_types","measured_data_type", "detected_difference_types","accuracy_types"};
    const int_t rTypes[] =
        {sdaiSTRING, sdaiINSTANCE, sdaiAGGR, sdaiAGGR, sdaiENUM, sdaiAGGR, sdaiAGGR};
    for (int i = 0; i < 7; i++) {
        int_t attribute = engiGetEntityAttributeByIndex(entity, i, true, true);
        const char* name = NULL;
        engiGetAttributeTraits(attribute, &name, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        assert(!strcmp(name, rAttr[i]));

        int_t type = 0;
        engiGetEntityArgumentType(entity, i, &type);
        assert(type == rTypes[i]);
    }
    
    //wrapper test
    auto inst = a3m_equivalence_criterion_with_specified_elements::Create(model);
    const char* NAME = "sey Name";
    inst.put_name(NAME);

    oriented_edge edge[2];
    edge[0] = oriented_edge::Create(model);
    edge[1] = oriented_edge::Create(model);

    auto eloop = edge_loop::Create(model);
    eloop.put_name(NAME);
    eloop.put_edge_list(edge, 2);

    sdaiSaveModelBN(model, "Test.ap");
    sdaiCloseModel(model);

    /// Now read
    /// 
    auto modelRead = sdaiOpenModelBN(NULL, "Test.ap", "AP242");

    entity = sdaiGetEntity(modelRead, "a3m_equivalence_criterion_with_specified_elements");// "a3m_equivalence_criterion");
    assert(entity);

    int_t* items = sdaiGetEntityExtent(modelRead, entity);
    auto N_items = sdaiGetMemberCount(items);
    assert(N_items == 1);
    for (int_t i = 0; i < N_items; i++) {

        int_t item = 0;
        engiGetAggrElement(items, i, sdaiINSTANCE, &item);

        auto name = a3m_equivalence_criterion(item).get_name();
        assert(!strcmp(name, NAME));
    }


    entity = sdaiGetEntity(modelRead, "edge_loop");
    assert(entity);
    items = sdaiGetEntityExtent(modelRead, entity);
    N_items = sdaiGetMemberCount(items);
    assert(N_items == 1);
    for (int_t i = 0; i < N_items; i++) {

        int_t item = 0;
        engiGetAggrElement(items, i, sdaiINSTANCE, &item);

        eloop = edge_loop(item);
        auto name = eloop.get_name();
        assert(!strcmp(name, NAME));

        std::list<oriented_edge> lst;
        eloop.get_edge_list(lst);
        assert(lst.size() == 2);
    }

    sdaiCloseModel(modelRead);

}
