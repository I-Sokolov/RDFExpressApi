
#include "IFC4.h"

using namespace IFC4;

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

    assert(blobTexture.get_RasterCode() == NULL);
    blobTexture.put_RasterCode(rasterCode);
    assert(0 == strcmp(blobTexture.get_RasterCode(), rasterCode));

    //put/get with SDAI
    sdaiPutAttrBN(blobTexture, "RasterCode", sdaiBINARY, rasterCode);
    IfcBinary gotData = NULL;
    sdaiGetAttrBN(blobTexture, "RasterCode", sdaiBINARY, &gotData);
    assert(!strcmp(gotData, rasterCode));

    //can also get as string
    sdaiGetAttrBN(blobTexture, "RasterCode", sdaiSTRING, &gotData);
    assert(!strcmp(gotData, rasterCode));

    //
    //aggregation
    auto pixelTexture = IfcPixelTexture::Create(ifcModel);

    ListOfIfcBinary lstBin;
    pixelTexture.get_Pixel(lstBin);
    assert(lstBin.size() == 0);

    lstBin.push_back(rasterCode);
    lstBin.push_back(rasterCode);

    pixelTexture.put_Pixel(lstBin);
    
    lstBin.clear();
    pixelTexture.get_Pixel(lstBin);
    assert(lstBin.size() == 2 && !strcmp(lstBin.front(), rasterCode) && !strcmp(lstBin.back(), rasterCode));

    //
    //select
    auto value = IfcAppliedValue::Create(ifcModel);

    auto bin = value.get_AppliedValue().get_IfcValue().get_IfcSimpleValue().get_IfcBinary();
    assert(bin == 0);

    value.put_AppliedValue().put_IfcValue().put_IfcSimpleValue().put_IfcBinary(rasterCode);
    bin = value.get_AppliedValue().get_IfcValue().get_IfcSimpleValue().get_IfcBinary();
    assert(!strcmp(bin, rasterCode));

    //simplified form
    bin = value.get_AppliedValue().as_text();
    assert(!strcmp(bin, rasterCode));

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
    assert(N == 1);
    for (int_t i = 0; i < N; i++) {
        int_t inst = 0;
        engiGetAggrElement(blobTextureAggr, i, sdaiINSTANCE, &inst);
        auto code = IfcBlobTexture(inst).get_RasterCode();
        assert(0 == strcmp(code, rasterCode));
    }

    auto entityPixelTexture = sdaiGetEntity(readModel, "IfcPixelTexture");
    auto pixelTextureAggr = sdaiGetEntityExtent(readModel, entityPixelTexture);
    N = sdaiGetMemberCount(pixelTextureAggr);
    assert(N == 1);
    for (int_t i = 0; i < N; i++) {
        int_t inst = 0;
        engiGetAggrElement(pixelTextureAggr, i, sdaiINSTANCE, &inst);
        ListOfIfcBinary lstBin;
        IfcPixelTexture(inst).get_Pixel(lstBin);
        assert(lstBin.size() == 2 && !strcmp(lstBin.front(), rasterCode) && !strcmp(lstBin.back(), rasterCode));
    }

    auto entityValue = sdaiGetEntity(readModel, "IfcAppliedValue");
    auto valueAggr = sdaiGetEntityExtent(readModel, entityValue);
    N = sdaiGetMemberCount(pixelTextureAggr);
    assert(N == 1);
    for (int_t i = 0; i < N; i++) {
        int_t inst = 0;
        engiGetAggrElement(valueAggr, i, sdaiINSTANCE, &inst);
        auto v = IfcAppliedValue(inst).get_AppliedValue().get_IfcValue().get_IfcSimpleValue().get_IfcBinary();
        assert(!strcmp(v, rasterCode));
    }

}

static void TestPutAttr(SdaiModel model)
{
    auto window = IfcWindow::Create(model);

    assert(window.get_PredefinedType().IsNull());
    window.put_PredefinedType(IfcWindowTypeEnum::SKYLIGHT);
    assert(window.get_PredefinedType().Value()==IfcWindowTypeEnum::SKYLIGHT);
    sdaiPutAttrBN(window, "PredefinedType", sdaiENUM, NULL);
    assert(window.get_PredefinedType().IsNull());

    assert(window.get_OverallWidth().IsNull());
    window.put_OverallWidth(50);
    assert(window.get_OverallWidth().Value() == 50);
    sdaiPutAttrBN(window, "OverallWidth", sdaiREAL, NULL);
    assert(window.get_OverallWidth().IsNull());

    //*/$
    auto unit = IfcSIUnit::Create(model);

#if 0
    //check sdaiPutAttr (sdaiSTRING) over complex argument
    IfcMeasureWithUnit measureWithUnit = IfcMeasureWithUnit::Create(model);
    
    //make complex with logical
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcLogical(IfcLogical::True);
    assert(measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcLogical().Value() == IfcLogical::True);

    sdaiPutAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, "F");

    auto adb = sdaiCreateEmptyADB();        
    assert(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb)); //it is not a complex now
    const char* eval = NULL;
    assert(sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiENUM, &eval)); //it is enum
    assert(!strcmp(eval, "T")); //but keeps old value

    //make complex with string
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcText("Text1");
    assert(!strcmp("Text1", measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcText()));

    sdaiPutAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, "New Text");
    adb = sdaiCreateEmptyADB();
    assert(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb)); //it is not a complex now
    eval = NULL;
    assert(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, &eval)); //and this can not be casted to string
    //(in debugger I see arg->sdai_type is 0-type now)
    
    //make complex with integer
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcInteger(8);
    assert(8==measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcInteger().Value());

    sdaiPutAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, "9");
    adb = sdaiCreateEmptyADB();
    assert(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb)); //it is not a complex now
    int ival = NULL;
    assert(sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiINTEGER, &ival)); //this is integer with old value
    assert(ival == 8);

    //make complex with real
    measureWithUnit.put_ValueComponent().put_IfcSimpleValue().put_IfcReal(3.14);
    assert(3.14 == measureWithUnit.get_ValueComponent().get_IfcSimpleValue().get_IfcReal().Value());

    sdaiPutAttrBN(measureWithUnit, "ValueComponent", sdaiSTRING, "9");
    adb = sdaiCreateEmptyADB();
    assert(!sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiADB, &adb)); //it is not a complex now
    double dval = NULL;
    assert(sdaiGetAttrBN(measureWithUnit, "ValueComponent", sdaiREAL, &dval)); //this is real with old value
    assert(dval == 3.14);
#endif
}

extern void EngineTests(void)
{
    SdaiModel  ifcModel = sdaiCreateModelBN(0, NULL, "IFC4");
    SetSPFFHeaderItem(ifcModel, 9, 0, sdaiSTRING, "IFC4");
    SetSPFFHeaderItem(ifcModel, 9, 1, sdaiSTRING, 0);

    TestBinaries(ifcModel);
    TestPutAttr(ifcModel);

    sdaiSaveModelBN(ifcModel, FILE_NAME);
    sdaiCloseModel(ifcModel);
}