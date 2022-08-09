using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RDF;

namespace CS_IFC
{
    class IfcBlobTextureExample
    {
        public static void Run()
        {
            var model = ifcengine.sdaiCreateModelBN(0, null as string, "IFC4");
            ifcengine.SetSPFFHeaderItem(model, 9, 0, ifcengine.sdaiSTRING, "IFC4");
            ifcengine.SetSPFFHeaderItem(model, 9, 1, ifcengine.sdaiSTRING, null as string);

            var ifcBlobTexture = RDF.ifcengine.sdaiCreateInstanceBN(model, "IfcBlobTexture");

            byte[] rasterBytes = System.IO.File.ReadAllBytes(@"e:\downloads\IMG_20220424_141401.jpg");

            //encode to STEP P21 hex code
            string rasterCode = "0"; //first 0 means full bytes
            rasterCode += Convert.ToHexString(rasterBytes); //followed by hex string

            ifcengine.sdaiPutAttrBN(ifcBlobTexture, "RasterCode", ifcengine.sdaiBINARY, rasterCode);

            ifcengine.sdaiSaveModelBN(model, "CSBinary.ifc");
            ifcengine.sdaiCloseModel(model);
        }
    }
}

