
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

#if 0
    //check sdaiPutAttr (sdaiSTRING) over complex argument
    IfcMeasureWithUnit measureWithUnit = IfcMeasureWithUnit::Create(model);
    
    //make complex with logical
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcLogical(IfcLogical::True);
    ASSERT(measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcLogical().Value() == IfcLogical::True);

    sdaiPutAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, "F");

    auto adb = sdaiCreateEmptyADB();        
    ASSERT(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb)); //it is not a complex now
    const char* eval = NULL;
    ASSERT(sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiENUM, &eval)); //it is enum
    ASSERT(!strcmp(eval, "T")); //but keeps old value

    //make complex with string
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcText("Text1");
    ASSERT(!strcmp("Text1", measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText()));

    sdaiPutAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, "New Text");
    adb = sdaiCreateEmptyADB();
    ASSERT(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb)); //it is not a complex now
    eval = NULL;
    ASSERT(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, &eval)); //and this can not be casted to string
    //(in debugger I see arg->sdai_type is 0-type now)
    
    //make complex with integer
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcInteger(8);
    ASSERT(8==measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcInteger().Value());

    sdaiPutAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, "9");
    adb = sdaiCreateEmptyADB();
    ASSERT(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb)); //it is not a complex now
    int ival = NULL;
    ASSERT(sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiINTEGER, &ival)); //this is integer with old value
    ASSERT(ival == 8);

    //make complex with real
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcReal(3.14);
    ASSERT(3.14 == measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcReal().Value());

    sdaiPutAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, "9");
    adb = sdaiCreateEmptyADB();
    ASSERT(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb)); //it is not a complex now
    double dval = NULL;
    ASSERT(sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiREAL, &dval)); //this is real with old value
    ASSERT(dval == 3.14);
#endif
}

static void TestGetAttrType(SdaiModel ifcModel, const char* entityName, const char* attrName, int_t expected)
{
    SdaiEntity entity = sdaiGetEntity(ifcModel, entityName);
    auto attr = sdaiGetAttrDefinition(entity, attrName);
    auto type = engiGetAttrType(attr);
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
}