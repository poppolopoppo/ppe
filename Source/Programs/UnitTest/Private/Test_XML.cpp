// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

/* TODO
#include "Collada.h"
#include "GenericMaterial.h"
#include "GenericMesh.h"
*/

#include "Markup/Xml.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Xml)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
using namespace PPE::Serialize;
//----------------------------------------------------------------------------
static void Test_XMLLoad_Simple_() {
    FXml xml;
    if (FXml::Load(&xml, L"Data:/Models/sample.xml"))
        LOG(Test_Xml, Info, L"sample.xml:\n{0}", xml);
}
//----------------------------------------------------------------------------
/* TODO
static void Test_XMLLoad_Collada_() {
    Lattice::FCollada collada;
    if (not Lattice::FCollada::Load(&collada, L"Data:/Models/astroBoy_walk_Maya.dae"))
        AssertNotReached();

    Lattice::FCollada::TArray<Lattice::PGenericMaterial> materials;
    if (not collada.ImportMaterials(materials))
        AssertNotReached();

    for (const Lattice::PGenericMaterial& m : materials) {
        std::cout
            << Fmt::Repeat('-', 80) << eol
            << "Name = " << m->Name() << eol
            << "Technique = " << m->Technique() << eol;

        std::cout << "Parameters (" << m->Parameters().size() << ")" << eol;
        for (const auto& it : m->Parameters())
            std::cout << "  * '" << it.first << "' = " << it.second << eol;

        std::cout << "Textures (" << m->Textures().size() << ")" << eol;
        for (const auto& it : m->Textures())
            std::cout << "  * '" << it.first << "' = " << it.second.Filename << eol;

        std::cout << eol;
    }
}
*/
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_XML() {
    PPE_DEBUG_NAMEDSCOPE("Test_Xml");

    Test_XMLLoad_Simple_();
    //Test_XMLLoad_Collada_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
