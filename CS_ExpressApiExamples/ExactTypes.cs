using RDF;
using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using IFC4x3;

namespace CS_IFC
{
    internal class ExactTypes
    {
        public static void Run()
        {
            var model = ifcengine.sdaiOpenModelBN(0, "../../../CS_ExpressApiExamples/fixed-reference-swept-area-solid.ifc", "");

            ///

            var inst = ifcengine.internalGetInstanceFromP21Line(model, 7);
            var project = new IfcProject(inst);

            SetOfIfcUnit units = project.UnitsInContext.Units;
            var cnt2 = units.Count();

            var namedUnits = units.Select(item => item.IfcNamedUnit);
            var cnt3 = namedUnits.Count();

            IEnumerable<IfcSIUnit> siUnits = namedUnits.OfType<IfcSIUnit>();
            var cnt4 = siUnits.Count();

            IEnumerable<IfcSIUnitName?> unitNames = siUnits.Select(item => item.Name);
            var cnt5 = unitNames.Count();
            assert(cnt5 == 4);

            ///
            inst = ifcengine.internalGetInstanceFromP21Line(model, 106);
            var repr = new IfcRepresentation(inst);

            IfcRepresentationContext ctx = repr.ContextOfItems;
            assert (ctx.GetType() == Type.GetType("IFC4x3.IfcGeometricRepresentationSubContext"));

            //
            var items = repr.Items;
            var item = items.First();
            assert(item.GetType() == Type.GetType("IFC4x3.IfcGradientCurve"));

            ifcengine.sdaiCloseModel(model);
        }

        private static void assert(bool ok)
        {
            System.Diagnostics.Debug.Assert(ok);
        }

    }
}
