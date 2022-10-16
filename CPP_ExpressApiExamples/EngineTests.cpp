
#include "IFC4.h"

using namespace IFC4;

#ifndef ASSERT
#define ASSERT(c) {if (!(c)) { printf ("ASSERT at LINE %d FILE %s\n", __LINE__, __FILE__); assert (false);}}
#endif


#define FILE_NAME "EngineTests.ifc"

static void TestBinaries(SdaiModel ifcModel)
{
#define NC 4
    char rasterCode[NC * 1024 * 4 + 2];
    rasterCode[0] = '1';
    rasterCode[NC * 1024 * 4] = '8';
    rasterCode[NC * 1024 * 4 + 1] = 0;
    for (int i = 1; i < NC * 1024 * 4; i++) {
        rasterCode[i] = 'A' + i % 3;
    }

    //
    //scalar attribute
    auto blobTexture = IfcBlobTexture::Create(ifcModel);
    blobTexture.put_RepeatS(true);
    blobTexture.put_RepeatT(true);
    blobTexture.put_RasterFormat("PNG");
    blobTexture.put_Mode("MODULATE");

    ASSERT(blobTexture.get_RasterCode() == NULL);
    blobTexture.put_RasterCode(rasterCode);
    ASSERT(0 == strcmp(blobTexture.get_RasterCode(), rasterCode));

    //put/get with SDAI
    sdaiPutAttrBN(blobTexture, "RasterCode", sdaiBINARY, rasterCode);
    IfcBinary gotData = NULL;
    sdaiGetAttrBN(blobTexture, "RasterCode", sdaiBINARY, &gotData);
    ASSERT(!strcmp(gotData, rasterCode));

    //can also get as string
    sdaiGetAttrBN(blobTexture, "RasterCode", sdaiSTRING, &gotData);
    ASSERT(!strcmp(gotData, rasterCode));

    //
    //aggregation
    auto pixelTexture = IfcPixelTexture::Create(ifcModel);

    ListOfIfcBinary lstBin;
    pixelTexture.get_Pixel(lstBin);
    ASSERT(lstBin.size() == 0);

    lstBin.push_back(rasterCode);
    lstBin.push_back(rasterCode);

    pixelTexture.put_Pixel(lstBin);
    
    lstBin.clear();
    pixelTexture.get_Pixel(lstBin);
    ASSERT(lstBin.size() == 2 && !strcmp(lstBin.front(), rasterCode) && !strcmp(lstBin.back(), rasterCode));

    //
    //select
    auto value = IfcAppliedValue::Create(ifcModel);

    auto bin = value.get_AppliedValue().get_IfcValue().get_IfcSimpleValue().get_IfcBinary();
    ASSERT(bin == 0);

    value.put_AppliedValue().put_IfcValue().put_IfcSimpleValue().put_IfcBinary(rasterCode);
    bin = value.get_AppliedValue().get_IfcValue().get_IfcSimpleValue().get_IfcBinary();
    ASSERT(!strcmp(bin, rasterCode));

    //simplified form
    bin = value.get_AppliedValue().as_text();
    ASSERT(!strcmp(bin, rasterCode));

    //
    //save and read

    sdaiSaveModelBN(ifcModel, FILE_NAME);

    //
    // Re-read
    //
    SdaiModel readModel = sdaiOpenModelBN(NULL, FILE_NAME, "IFC4");

    auto entityBlobTexture = sdaiGetEntity(readModel, "IfcBlobTexture");
    auto blobTextureAggr = sdaiGetEntityExtent(readModel, entityBlobTexture);
    auto N = sdaiGetMemberCount(blobTextureAggr);
    ASSERT(N == 1);
    for (int_t i = 0; i < N; i++) {
        int_t inst = 0;
        engiGetAggrElement(blobTextureAggr, i, sdaiINSTANCE, &inst);
        auto code = IfcBlobTexture(inst).get_RasterCode();
        ASSERT(0 == strcmp(code, rasterCode));
    }

    const char* argName = NULL;
    engiGetEntityArgumentName(entityBlobTexture, 1, sdaiSTRING, &argName);
    ASSERT(0 == strcmp(argName, "RepeatT"));

    int_t argType;
    engiGetEntityArgumentType(entityBlobTexture, 1, &argType);
    ASSERT(argType == sdaiBOOLEAN);

    auto entityPixelTexture = sdaiGetEntity(readModel, "IfcPixelTexture");
    auto pixelTextureAggr = sdaiGetEntityExtent(readModel, entityPixelTexture);
    N = sdaiGetMemberCount(pixelTextureAggr);
    ASSERT(N == 1);
    for (int_t i = 0; i < N; i++) {
        int_t inst = 0;
        engiGetAggrElement(pixelTextureAggr, i, sdaiINSTANCE, &inst);
        ListOfIfcBinary lstBin2;
        IfcPixelTexture(inst).get_Pixel(lstBin2);
        ASSERT(lstBin2.size() == 2 && !strcmp(lstBin2.front(), rasterCode) && !strcmp(lstBin2.back(), rasterCode));
    }

    auto entityValue = sdaiGetEntity(readModel, "IfcAppliedValue");
    auto valueAggr = sdaiGetEntityExtent(readModel, entityValue);
    N = sdaiGetMemberCount(pixelTextureAggr);
    ASSERT(N == 1);
    for (int_t i = 0; i < N; i++) {
        int_t inst = 0;
        engiGetAggrElement(valueAggr, i, sdaiINSTANCE, &inst);
        auto v = IfcAppliedValue(inst).get_AppliedValue().get_IfcValue().get_IfcSimpleValue().get_IfcBinary();
        ASSERT(!strcmp(v, rasterCode));
    }

}

static void TestPutAttr(SdaiModel model)
{
    auto window = IfcWindow::Create(model);

    ASSERT(window.get_PredefinedType().IsNull());
    window.put_PredefinedType(IfcWindowTypeEnum::SKYLIGHT);
    ASSERT(window.get_PredefinedType().Value()==IfcWindowTypeEnum::SKYLIGHT);
    sdaiPutAttrBN(window, "PredefinedType", sdaiENUM, NULL);
    ASSERT(window.get_PredefinedType().IsNull());

    ASSERT(window.get_OverallWidth().IsNull());
    window.put_OverallWidth(50);
    ASSERT(window.get_OverallWidth().Value() == 50);
    sdaiPutAttrBN(window, "OverallWidth", sdaiREAL, NULL);
    ASSERT(window.get_OverallWidth().IsNull());

    //*/$
    auto unit = IfcSIUnit::Create(model);

    //
    IfcMeasureWithUnit measureWithUnit = IfcMeasureWithUnit::Create(model);
    
    //make complex with logical
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcLogical(IfcLogical::True);
    ASSERT(measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcLogical().Value() == IfcLogical::True);

    auto type = engiGetInstanceAttrTypeBN(measureWithUnit, "ValueComponent");
    ASSERT(type == sdaiADB);

    auto ifcMeasureWithUnit = sdaiGetEntity(model, "IfcMeasureWithUnit");
    type = engiGetAttrTypeBN(ifcMeasureWithUnit, "ValueComponent");
    ASSERT(type == sdaiADB);

    type = engiGetAttrTypeBN(measureWithUnit, "ValueComponent");
    ASSERT(type == sdaiADB);

    auto adb = sdaiCreateEmptyADB();        
    ASSERT(sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb));

    type = sdaiGetADBType(adb);
    ASSERT(type == sdaiENUM);

    type = engiGetInstanceAttrTypeBN(window, "FillsVoids");
    ASSERT(type == 0);
}

static void TestGetAttrType(SdaiModel ifcModel, const char* entityName, const char* attrName, int_t expected)
{
    SdaiEntity entity = sdaiGetEntity(ifcModel, entityName);
    auto attr = sdaiGetAttrDefinition(entity, attrName);
    auto type = engiGetAttrType(attr);
    ASSERT(type == expected);

    type = engiGetAttrTypeBN(entity, attrName);
    ASSERT(type == expected);
}

static void TestGetAttrType(SdaiModel ifcModel)
{
    TestGetAttrType(ifcModel, "IfcWall", "GlobalId", sdaiSTRING);
    TestGetAttrType(ifcModel, "IfcWall", "OwnerHistory", sdaiINSTANCE);
    TestGetAttrType(ifcModel, "IfcWall", "IsDecomposedBy", sdaiINSTANCE | engiTypeFlagAggr);
    TestGetAttrType(ifcModel, "IfcWall", "PredefinedType", sdaiENUM);
    TestGetAttrType(ifcModel, "IfcPolyLoop", "Polygon", sdaiINSTANCE | engiTypeFlagAggr);
    TestGetAttrType(ifcModel, "IfcPolygonalFaceSet", "PnIndex", sdaiINTEGER | engiTypeFlagAggr);
    TestGetAttrType(ifcModel, "IfcRelDefinesByProperties", "RelatingPropertyDefinition", sdaiINSTANCE | engiTypeFlagAggrOption);
    TestGetAttrType(ifcModel, "IfcMeasureWithUnit", "ValueComponent", sdaiADB);
    TestGetAttrType(ifcModel, "IfcFeatureElementAddition", "ProjectsElements", sdaiINSTANCE);
    TestGetAttrType(ifcModel, "IfcSite", "RefLatitude", sdaiINTEGER | engiTypeFlagAggr);
    TestGetAttrType(ifcModel, "IfcRationalBSplineCurveWithKnots", "WeightsData", sdaiREAL | engiTypeFlagAggr);
    TestGetAttrType(ifcModel, "IfcWindow", "FillsVoids", sdaiINSTANCE | engiTypeFlagAggr);
}


static void TestGetADBValue(SdaiModel ifcModel)
{
    ///
    auto instance = internalGetInstanceFromP21Line(ifcModel, 319);

    SdaiAggr listValues = 0;
    sdaiGetAttrBN(instance, "ListValues", sdaiAGGR, &listValues);

    auto count = sdaiGetMemberCount(listValues);
    ASSERT(count == 9);

    void* adbValue = NULL;
    int_t intV = 0;
    double doubleV = 0;
    const char* textV = 0;
    const wchar_t* wcV = nullptr;
    bool boolV = false;

    /// <summary>
    /// 0-string
    /// </summary>
    engiGetAggrElement(listValues, 0, sdaiADB, &adbValue);
    auto type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiSTRING);

    ASSERT(!sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(!boolV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!textV);
    ASSERT(sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!strcmp(textV, "List value 2"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!strcmp(textV, "List value 2"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!strcmp(textV, "List value 2"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcscmp(wcV, L"List value 2"));
    ASSERT(!sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 0);

    /// <summary>
    /// 1-integer
    /// </summary>
    engiGetAggrElement(listValues, 1, sdaiADB, &adbValue);
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiINTEGER);

    ASSERT(!sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(!boolV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcV);
    ASSERT(sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 13);
    ASSERT(sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 13);

    /// <summary>
    /// 2-real
    /// </summary>
    engiGetAggrElement(listValues, 2, sdaiADB, &adbValue);
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiREAL);

    ASSERT(!sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(!boolV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcV);
    ASSERT(sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 8.5);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 0);

    /// <summary>
    /// 3-boolean
    /// </summary>
    engiGetAggrElement(listValues, 3, sdaiADB, &adbValue);
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiBOOLEAN);

    ASSERT(!sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    ASSERT(intV == 0);
    ASSERT(sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!strcmp(textV, "T"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(boolV);
    ASSERT(sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!strcmp(textV, "T"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!strcmp(textV, "T"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!strcmp(textV, "T"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!strcmp(textV, "T"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcscmp(wcV, L"T"));
    ASSERT(!sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 0);

    /// <summary>
    /// 4-logical
    /// </summary>
    engiGetAggrElement(listValues, 4, sdaiADB, &adbValue);
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiLOGICAL);

    ASSERT(!sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    ASSERT(intV == 0);
    ASSERT(sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!strcmp(textV, "U"));
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(!boolV);
    ASSERT(sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!strcmp(textV, "U"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!strcmp(textV, "U"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!strcmp(textV, "U"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!strcmp(textV, "U"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcscmp(wcV, L"U"));
    ASSERT(!sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 0);

    /// <summary>
    /// 5-binary
    /// </summary>
    engiGetAggrElement(listValues, 5, sdaiADB, &adbValue);
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiBINARY);

    ASSERT(!sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(!boolV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!textV);
    ASSERT(sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!strcmp(textV, "02F"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!strcmp(textV, "02F"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!strcmp(textV, "02F"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcscmp(wcV, L"02F"));
    ASSERT(!sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 0);

    /// <summary>
    /// 6-instance
    /// </summary>
    engiGetAggrElement(listValues, 6, sdaiADB, &adbValue);
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiINSTANCE);

    ASSERT(!sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV == 0);

    ASSERT(sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    intV = sdaiGetInstanceType(intV);
    engiGetEntityName(intV, sdaiSTRING, &textV);
    ASSERT(!strcmp(textV, "IfcPropertyListValue"));
    auto typePath = sdaiGetADBTypePath(adbValue, 0);
    ASSERT(!strcmp(typePath, ""));

    ASSERT(!sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(!boolV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 0);


    /// <summary>
    /// 7-aggregation
    /// </summary>
    engiGetAggrElement(listValues, 7, sdaiADB, &adbValue);
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiAGGR);

    ASSERT(sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV != 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(!boolV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 0);

    /// <summary>
    /// 8-enum
    /// </summary>
    engiGetAggrElement(listValues, 8, sdaiADB, &adbValue);
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiENUM);

    ASSERT(!sdaiGetADBValue(adbValue, sdaiAGGR, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    ASSERT(intV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiLOGICAL, &textV));
    ASSERT(!textV);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiBOOLEAN, &boolV));
    ASSERT(!boolV);
    ASSERT(sdaiGetADBValue(adbValue, sdaiENUM, &textV));
    ASSERT(!strcmp(textV, "BELL"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiBINARY, &textV));
    ASSERT(!strcmp(textV, "BELL"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiSTRING, &textV));
    ASSERT(!strcmp(textV, "BELL"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiEXPRESSSTRING, &textV));
    ASSERT(!strcmp(textV, "BELL"));
    ASSERT(sdaiGetADBValue(adbValue, sdaiUNICODE, &wcV));
    ASSERT(!wcscmp(wcV, L"BELL"));
    ASSERT(!sdaiGetADBValue(adbValue, sdaiREAL, &doubleV));
    ASSERT(doubleV == 0);
    ASSERT(!sdaiGetADBValue(adbValue, sdaiINTEGER, &intV));
    ASSERT(intV == 0);

    /// <summary>
    /// 
    /// </summary>
    IFC4::IfcPropertyListValue propListValue(instance);
    ASSERT(propListValue);

    IFC4::ListOfIfcValue lstIfcValue;
    propListValue.get_ListValues(lstIfcValue);
    ASSERT(lstIfcValue.size() == 9);

    auto it = lstIfcValue.begin();
    auto label = it->_IfcSimpleValue().get_IfcLabel();
    ASSERT(!strcmp(label, "List value 2"));

    it++;
    auto in = it->_IfcSimpleValue().get_IfcInteger();
    ASSERT(in.Value() == 13);

    it++;
    auto id = it->_IfcMeasureValue().get_IfcLengthMeasure();
    ASSERT(id.Value() == 8.5);

    it++;
    auto ib = it->_IfcSimpleValue().get_IfcBoolean();
    ASSERT(ib.Value() == true);

    it++;
    auto il = it->_IfcSimpleValue().get_IfcLogical();
    ASSERT(il.Value() == LOGICAL_VALUE::Unknown);

    it++;
    auto ibin = it->_IfcSimpleValue().get_IfcBinary();
    ASSERT(!strcmp(ibin, "02F"));

    it++;//instance but no access functions because it violates schema
    ASSERT(it->_IfcMeasureValue().get_IfcAreaMeasure().IsNull());
    adbValue = it->ADB();
    type = sdaiGetADBType(adbValue);
    ASSERT(type == sdaiINSTANCE);
    ASSERT(sdaiGetADBValue(adbValue, sdaiINSTANCE, &intV));
    intV = sdaiGetInstanceType(intV);
    engiGetEntityName(intV, sdaiSTRING, &textV);
    ASSERT(!strcmp(textV, "IfcPropertyListValue"));

    it++;
    std::list<double> ic;
    it->_IfcMeasureValue().get_IfcComplexNumber(ic);
    ASSERT(ic.back() == 1 && ic.front() == 1 && ic.size() == 2);

    //
    auto entityCtx = sdaiGetEntity(ifcModel, "IfcContext");
    auto ind = engiGetEntityAttributeIndex(entityCtx, "UnitsInContext");
    ASSERT(ind == 15);
    ind = engiGetEntityAttributeIndexEx(entityCtx, "UnitsInContext", true, false);
    ASSERT(ind == 8);
    ind = engiGetEntityAttributeIndexEx(entityCtx, "UnitsInContext", false, false);
    ASSERT(ind == 4);
}

static void TestAttrIndex(SdaiModel ifcModel)
{
    auto wall = internalGetInstanceFromP21Line(ifcModel, 8);

    //direct attribute
    auto attr = engiGetEntityAttributeByIndex(wall, 0, false, false);

    const char* attrName = nullptr;
    bool inverse = false;
    engiGetAttributeTraits(attr, &attrName, nullptr, &inverse, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT(!inverse && !strcmp(attrName, "PredefinedType"));

    const char* type = NULL;
    sdaiGetAttr(wall, attr, sdaiENUM, &type);
    ASSERT(!strcmp(type, "SOLIDWALL"));

    sdaiGetAttrBN(wall, "PredefinedType", sdaiENUM, &type);
    ASSERT(!strcmp(type, "SOLIDWALL"));

    auto ind = engiGetEntityAttributeIndexEx(wall, "PredefinedType", true, true);
    ASSERT(ind == 32);
    ind = engiGetEntityAttributeIndexEx(wall, "PredefinedType", true, false);
    ASSERT(ind == 8);
    ind = engiGetEntityAttributeIndexEx(wall, "PredefinedType", false, true);
    ASSERT(ind == 0);

    //inverse attribute
    auto story = internalGetInstanceFromP21Line(ifcModel, 6);
    
    attr = engiGetEntityAttributeByIndex(wall, 9, true, true);

    SdaiInstance building = 0;
    sdaiGetAttr(story, attr, sdaiINSTANCE, &building);
    auto stepId = internalGetP21Line(building);
    ASSERT(stepId == 7);

    sdaiGetAttrBN(story, "Decomposes", sdaiINSTANCE, &building);
    stepId = internalGetP21Line(building);
    ASSERT(stepId == 7);

    ind = engiGetEntityAttributeIndexEx(wall, "Decomposes", true, true);
    ASSERT(ind == 9);
    ind = engiGetEntityAttributeIndexEx(wall, "Decomposes", true, false);
    ASSERT(ind == -1);
    ind = engiGetEntityAttributeIndexEx(wall, "Decomposes", false, true);
    ASSERT(ind == -1);
}

extern void EngineTests(void)
{
    SdaiModel  ifcModel = sdaiCreateModelBN(0, NULL, "IFC4");
    SetSPFFHeaderItem(ifcModel, 9, 0, sdaiSTRING, "IFC4");
    SetSPFFHeaderItem(ifcModel, 9, 1, sdaiSTRING, 0);

    TestBinaries(ifcModel);
    TestPutAttr(ifcModel);
    TestGetAttrType(ifcModel);

    sdaiSaveModelBN(ifcModel, FILE_NAME);
    sdaiCloseModel(ifcModel);

    //
    ifcModel = sdaiOpenModelBN(NULL, "IFC4_test.ifc", "IFC4");

    TestAttrIndex(ifcModel);
    TestGetADBValue(ifcModel);

    sdaiCloseModel(ifcModel);
}