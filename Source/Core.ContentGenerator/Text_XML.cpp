#include "stdafx.h"

#include "Core.Serialize/XML/XML.h"
#include "Core.Serialize/XML/Document.h"
#include "Core.Serialize/XML/Element.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
using namespace Core::Serialize;
//----------------------------------------------------------------------------
static void Test_XMLLoad_Simple_() {
    XML::Document xml;
    if (XML::Document::Load(&xml, L"Data:/Models/sample.xml"))
        std::cout << xml << std::endl;
}
//----------------------------------------------------------------------------
static void Test_XMLLoad_Collada_() {
    XML::Document xml;
    if (XML::Document::Load(&xml, L"Data:/Models/astroBoy_walk_Maya.dae"))
        std::cout << xml << std::endl;
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
