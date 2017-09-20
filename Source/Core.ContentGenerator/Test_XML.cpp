#include "stdafx.h"

#include "Core.Lattice/Collada.h"
#include "Core.Lattice/GenericMaterial.h"
#include "Core.Lattice/GenericMesh.h"

#include "Core.Serialize/XML/XML.h"
#include "Core.Serialize/XML/Document.h"
#include "Core.Serialize/XML/Element.h"

#include "Core/IO/FormatHelpers.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
using namespace Core::Serialize;
//----------------------------------------------------------------------------
static void Test_XMLLoad_Simple_() {
    XML::FDocument xml;
    if (XML::FDocument::Load(&xml, L"Data:/Models/sample.xml"))
        std::cout << xml << eol;
}
//----------------------------------------------------------------------------
static void Test_XMLLoad_Collada_() {
    Lattice::FCollada collada;
    if (not Lattice::FCollada::Load(&collada, L"Data:/Models/astroBoy_walk_Maya.dae"))
        AssertNotReached();

    Lattice::FCollada::TArray<Lattice::PGenericMaterial> materials;
    if (not collada.ImportMaterials(materials))
        AssertNotReached();

    for (const Lattice::PGenericMaterial& m : materials) {
        std::cout
            << Repeat('-', 80) << eol
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
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_XML() {
    Test_XMLLoad_Simple_();
    Test_XMLLoad_Collada_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
