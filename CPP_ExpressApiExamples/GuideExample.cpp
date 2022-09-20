
#include <vector>
#include "IFC4.h"

#ifndef ASSERT
#define ASSERT(c) {if (!(c)) { printf ("ASSERT at LINE %d FILE %s\n", __LINE__, __FILE__); assert (false);}}
#endif


extern void GuideExamples()
{
    int_t  model = sdaiCreateModelBN(0, NULL, "IFC4");
    SetSPFFHeaderItem(model, 9, 0, sdaiSTRING, "IFC4");
    SetSPFFHeaderItem(model, 9, 1, sdaiSTRING, 0);

    //
    // Create instances and use in SDAI
    //

    //use static Create method to create model instance
    IFC4::IfcWall wall = IFC4::IfcWall::Create(model);
    IFC4::IfcDoor door = IFC4::IfcDoor::Create(model);

    //whenever SDAI instance is required, model instance can be used with implicit conversion
    int_t ok = sdaiIsKindOfBN(wall, "IfcProduct");
    ASSERT(ok);

    SdaiInstance sdaiWall = wall;
    ok = sdaiIsKindOfBN(sdaiWall, "IfcSlab");
    ASSERT(!ok);

    //other way, if you have SDAI instance you can construct model instance of appripriate type

    IFC4::IfcProduct product(sdaiWall);
    ASSERT(product); //check instance is valid

    IFC4::IfcSlab slab(sdaiWall);
    ASSERT(!slab); //wall is not a slab

    //
    // use put_* and set_* methods to access attribute
    // Hint: in Visual Studio pay attention to inetlli-sense for possible methods help tool-tips for possible arguments
    // 

    wall.put_Name("MyWall");
    ASSERT(!- strcmp(wall.get_Name(), "MyWall"));

    //
    // Nullable values
    //

    // get_* method will return domain type if attribute domain includes NULL value
    IFC4::IfcText text = wall.get_Description();
    ASSERT(text == NULL); //not set

    // but if NULL value outside of domain type, get_* method will return Nullable<> type extension
    // use Nullable::IsNull and Nullable::Value

    IFC4::Nullable<double> width = door.get_OverallWidth();
    ASSERT(width.IsNull()); //not set

    door.put_OverallWidth(900);

    width = door.get_OverallWidth();
    ASSERT(!width.IsNull() && width.Value() == 900);

    // Hint: use auto to simplify code
    
    auto height = door.get_OverallHeight();
    ASSERT(height.IsNull()); //not set
    
    //
    // Enumerations
    // enum class defined for each EXPRESS enumeration
    //

    wall.put_PredefinedType(IFC4::IfcWallTypeEnum::MOVABLE);

    // get_* methods will return Nullable extension
    IFC4::Nullable<IFC4::IfcWallTypeEnum> wallPredefinedType = wall.get_PredefinedType();
    ASSERT(wallPredefinedType.Value() == IFC4::IfcWallTypeEnum::MOVABLE);

    //Hint: simplify with auto
    auto doorPredefinedType = door.get_PredefinedType();
    ASSERT(doorPredefinedType.IsNull());

    //
    // Definded types
    // C++ types are declared for each EXPRESS defined type and implicitly converted to base C++ type
    //
   
    IFC4::IfcPositiveLengthMeasure measureHeight = 2000;
    double doubleHeight = 2000;

    //both are suitable to put
    door.put_OverallHeight(measureHeight); 
    door.put_OverallHeight(doubleHeight);

    //both are suitable to get
    IFC4::Nullable<IFC4::IfcPositiveLengthMeasure> getAsMeasure = door.get_OverallHeight();
    IFC4::Nullable<double> getAsDouble = door.get_OverallHeight();
    ASSERT(getAsMeasure.Value() == 2000 && getAsDouble.Value() == 2000);

    //
    // SELECTs
    //

    auto actor = IFC4::IfcActor::Create(model);
    auto person = IFC4::IfcPerson::Create(model);

    //when you put a value to SELECT you shold specify type of the value
    //to do this, attribute put_* methods return a put-selector with method for each possible type

    actor.put_TheActor().put_IfcPerson(person);

    //or this form
    IFC4::IfcActorSelect_put putSelector = actor.put_TheActor();
    putSelector.put_IfcPerson(person);

    //similary, attribute get_* methods return a get-selector and you can inquire content
    IFC4::IfcActorSelect_get getSelector = actor.get_TheActor();
    
    ASSERT(getSelector.is_IfcPerson());
    ASSERT(!getSelector.is_IfcOrganization());

    IFC4::IfcPerson gotPerson = getSelector.get_IfcPerson();
    ASSERT(gotPerson == person);

    IFC4::IfcOrganization gotOrganization = getSelector.get_IfcOrganization();
    ASSERT(gotOrganization == 0);

    //get-selector may provide a method to get as base C++ type without specifing IFC type
    
    SdaiInstance inst =  getSelector.as_instance();
    
    //check instance class
    ASSERT(IFC4::IfcPerson(inst));
    ASSERT(!IFC4::IfcOrganization(inst));
    ASSERT(sdaiIsKindOfBN(inst, "IfcPerson"));


    //work with nested SELECT
    auto measure = IFC4::IfcMeasureWithUnit::Create(model);

    //when put you have to specify type path
    measure.put_ValueComponent().put_IfcSimpleValue().put_IfcInteger(75);

    //you can get with type path
    ASSERT(measure.get_ValueComponent().get_IfcSimpleValue().is_IfcInteger());
    ASSERT(!measure.get_ValueComponent().get_IfcMeasureValue().is_IfcAreaMeasure());

    auto valueSelector = measure.get_ValueComponent(); //you can save selector in a variable

    auto gotInt = valueSelector.get_IfcSimpleValue().get_IfcInteger();
    ASSERT(!gotInt.IsNull() && gotInt.Value() == 75);

    auto gotArea = measure.get_ValueComponent().get_IfcMeasureValue().get_IfcAreaMeasure();
    ASSERT(gotArea.IsNull());

    //if you are not interested in type, you can get as C++ base type
    gotInt = valueSelector.as_int();
    ASSERT(!gotInt.IsNull() && gotInt.Value() == 75);

    auto gotDouble = valueSelector.as_double();
    ASSERT(!gotDouble.IsNull() && gotDouble.Value() == 75);

    auto gotText = measure.get_ValueComponent().as_text();
    ASSERT(gotText != NULL && !strcmp(gotText, "75"));

    auto gotBool = valueSelector.as_bool();
    ASSERT(gotBool.IsNull()); //IfcInteger is not convertable to bool

    //
    // AGGRAGATIONS
    // For each unnamed aggragaion there is a ListOf*, SetOf* or BagOf* class
    // For each named aggragation there is a lst class with the given name
    // For put_* and get_* methods you can use these lists or any list with converible elements
    // Additionaly for some put_* methods you can use C-arrays
    // Hint: in Visual Studio pay attention to help tool-tips for possible arguments
    //

    auto site = IFC4::IfcSite::Create(model);

    //put/get as IFC defined type
    IFC4::IfcCompoundPlaneAngleMeasure planeAngle;
    planeAngle.push_back(44);
    planeAngle.push_back(34);
    planeAngle.push_back(3);

    site.put_RefLatitude(planeAngle);

    IFC4::IfcCompoundPlaneAngleMeasure gotPlaneAngle;
    site.get_RefLatitude(gotPlaneAngle);
    ASSERT(gotPlaneAngle.size() == 3 && gotPlaneAngle.front() == 44);

    //to put you can use lists of convertable types or array
    std::list<int_t> lstInt;
    lstInt.push_back(56);
    site.put_RefLatitude(lstInt);

    int arrInt[] = {43,17,3,4};
    site.put_RefLongitude(arrInt, 4);

    //and get as list of convertable type
    std::vector<int_t> vector;
    site.get_RefLongitude(vector);
    ASSERT(vector.size() == 4 && vector[2] == 3);

    IFC4::ListOfIfcInteger lstIfcInt;
    site.get_RefLatitude(lstIfcInt);
    ASSERT(lstIfcInt.size() == 1 && lstIfcInt.front() == 56);

    //
    // RELATIONSHIPS is an example of aggregations of entities
    //
    auto group = IFC4::IfcRelAssignsToGroup::Create(model);
    
    IFC4::IfcObjectDefinition gropObjects[] = {wall, site};
    group.put_RelatedObjects(gropObjects, 2);

    IFC4::SetOfIfcObjectDefinition gotGroup;
    group.get_RelatedObjects(gotGroup);
    ASSERT(gotGroup.size() == 2 && gotGroup.front() == wall && gotGroup.back() == site);
        
    //
    // AGGERGATION OF SELECT
    // To put and get aggregation of SELECT use list of selector
    // When you create a selector you have to specify entity instance it will be used for
    //
    
    auto propEnumValue = IFC4::IfcPropertyEnumeratedValue::Create(model);
    
    IFC4::ListOfIfcValue lstValue;      

    IFC4::IfcValue value(propEnumValue);    
    value._IfcSimpleValue().put_IfcLabel("MyLabel");
    lstValue.push_back(value);
    value._IfcMeasureValue().put_IfcCountMeasure(4);
    lstValue.push_back(value);

    propEnumValue.put_EnumerationValues(lstValue);

    IFC4::ListOfIfcValue gotValues;
    propEnumValue.get_EnumerationValues(gotValues);
    ASSERT(gotValues.size() == 2);
    const char* v1 = gotValues.front()._IfcSimpleValue().get_IfcLabel();
    ASSERT(v1!=NULL && !strcmp(v1, "MyLabel"));
    IFC4::Nullable<double> v2 = gotValues.back()._IfcMeasureValue().get_IfcCountMeasure();
    ASSERT(!v2.IsNull() && v2.Value() == 4);

    //
    // AGGREGATION OF AGGREGATION
    //
    auto pointList = IFC4::IfcCartesianPointList3D::Create(model);

    IFC4::ListOfListOfIfcLengthMeasure coordList;

    //point (1,0.1)
    coordList.push_back(IFC4::ListOfIfcLengthMeasure());
    coordList.back().push_back(1);
    coordList.back().push_back(0);
    coordList.back().push_back(1);

    //point (0,1,0)
    coordList.push_back(IFC4::ListOfIfcLengthMeasure());
    coordList.back().push_back(0);
    coordList.back().push_back(1);
    coordList.back().push_back(0);

    pointList.put_CoordList(coordList);

    IFC4::ListOfListOfIfcLengthMeasure coordListCheck;
    pointList.get_CoordList(coordListCheck);
    ASSERT(coordList == coordListCheck);

    //
    // SELECT OF AGGREGATION
    //
    auto prop = IFC4::IfcPropertySingleValue::Create(model);

    double cplx[] = {2.1, 1.5};
    prop.put_NominalValue().put_IfcMeasureValue().put_IfcComplexNumber(cplx, 2);

    IFC4::IfcComplexNumber cplxNum;
    prop.get_NominalValue().get_IfcMeasureValue().get_IfcComplexNumber(cplxNum);
    ASSERT(cplxNum.size() == 2 && cplxNum.front() == 2.1 && cplxNum.back() == 1.5);

}