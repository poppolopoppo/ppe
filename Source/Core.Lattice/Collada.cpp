#include "stdafx.h"

#include "Collada.h"

#include "GenericMaterial.h"
#include "GenericMesh.h"

#include "Core.Serialize/XML/Document.h"
#include "Core.Serialize/XML/Element.h"
#include "Core.Serialize/XML/Helpers.h"
#include "Core.Serialize/XML/Name.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/FileSystemConstNames.h"
#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define FOREACH_CORE_LATTICE_COLLADATOKEN(_Macro, ...) \
    COMMA_PROTECT(_Macro(Accessor                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Ambient                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Asset                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Author                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Authoring_Tool                   , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Bind_Material                    , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Bind_Vertex_Input                , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Bump                             , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Collada                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Color                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Contributor                      , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Count                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Created                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Diffuse                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Effect                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Emission                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Encoding                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Extra                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float_Array                      , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Geometry                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Id                               , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Image                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Index_Of_Refraction              , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Init_From                        , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Input                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Input_Semantic                   , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Input_Set                        , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Instance_Effect                  , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Instance_Geometry                , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Instance_Material                , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Instance_Visual_Scene            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Lambert                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Library_Controllers              , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Library_Effects                  , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Library_Geometries               , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Library_Images                   , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Library_Materials                , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Library_Visual_Scenes            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Material                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Matrix                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Mesh                             , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Meter                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Modified                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Name                             , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Newparam                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Node                             , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Offset                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(P                                , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Param                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Polylist                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Profile                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Profile_Common                   , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Reflective                       , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Reflectivity                     , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Sampler2d                        , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Scene                            , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Semantic                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Set                              , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Sid                              , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Source                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Specular                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Stride                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Surface                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Symbol                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Target                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Technique                        , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Technique_Common                 , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Texcoord                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Texture                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Type                             , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Unit                             , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Up_Axis                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Url                              , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Vcount                           , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Version                          , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Vertices                         , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Visual_Scene                     , ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Xmlns                            , ##__VA_ARGS__))
//----------------------------------------------------------------------------
namespace DAE {
//----------------------------------------------------------------------------
#define COLLADATOKEN_DEF(_Name, _Unused) \
    namespace _Storage { static POD_STORAGE(XML::Name) CONCAT(g, _Name); } \
    static const XML::Name& _Name = reinterpret_cast<XML::Name&>(CONCAT(_Storage::g, _Name));
FOREACH_CORE_LATTICE_COLLADATOKEN(COLLADATOKEN_DEF)
#undef COLLADATOKEN_DEF
//----------------------------------------------------------------------------
static void Start() {
#define COLLADATOKEN_START(_Name, _Unused) \
    new ((void*)&CONCAT(_Storage::g, _Name)) XML::Name(STRINGIZE(_Name));
FOREACH_CORE_LATTICE_COLLADATOKEN(COLLADATOKEN_START)
#undef COLLADATOKEN_START
}
//----------------------------------------------------------------------------
static void Shutdown() {
    typedef XML::Name xmlname_type;
#define COLLADATOKEN_SHUTDOWN(_Name, _Unused) \
    reinterpret_cast<XML::Name&>(CONCAT(_Storage::g, _Name)).~xmlname_type();
FOREACH_CORE_LATTICE_COLLADATOKEN(COLLADATOKEN_SHUTDOWN)
#undef COLLADATOKEN_SHUTDOWN
}
//----------------------------------------------------------------------------
} //!namespace DAE
//----------------------------------------------------------------------------
#undef FOREACH_CORE_LATTICE_COLLADATOKEN
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Collada::Collada(XML::Document* xml) : _xml(xml) {}
//----------------------------------------------------------------------------
Collada::~Collada() {}
//----------------------------------------------------------------------------
bool Collada::ImportGeometries(Array<PGenericMesh>& meshes) const {
    return false;
}
//----------------------------------------------------------------------------
bool Collada::ImportMaterials(Array<PGenericMaterial>& materials) const {
    Assert(_xml);

    bool succeed = true;
    const XML::Document& doc = *_xml;

    doc.XPath({ DAE::Collada, DAE::Library_Materials, DAE::Material }, [&doc, &materials, &succeed](const XML::Element& elt) {
        materials.emplace_back(new GenericMaterial());
        GenericMaterial& m = *materials.back();

        m.SetName(elt[DAE::Name]);
        if (m.Name().empty()) {
            succeed = false;
            return;
        }

        const XML::Element* instance_effect = elt.ChildXPath({ DAE::Instance_Effect });
        if (nullptr == instance_effect) {
            succeed = false;
            return;
        }

        const StringView effect_source = (*instance_effect)[DAE::Url];
        if (effect_source.empty()) {
            succeed = false;
            return;
        }

        const XML::Element* effect = doc.FindById(effect_source);
        if (nullptr == effect) {
            succeed = false;
            return;
        }

        const XML::Element* technique = effect->ChildXPath({ XML::Name::Any, DAE::Technique });
        if (nullptr == technique) {
            succeed = false;
            return;
        }

        m.SetTechnique((*technique)[DAE::Sid]);
        if (m.Technique().empty()) {
            succeed = false;
            return;
        }

        // Parameters

        if (const XML::Element* ambient = technique->ChildXPath({ XML::Name::Any, DAE::Ambient, DAE::Color })) {
            float4 color;
            if (not XML::Parse(&color, ambient->Text().MakeView())) {
                succeed = false;
                return;
            }
            m.SetParameter(DAE::Ambient.MakeView(), color);
        }

        if (const XML::Element* emission = technique->ChildXPath({ XML::Name::Any, DAE::Emission, DAE::Color })) {
            float4 color;
            if (not XML::Parse(&color, emission->Text().MakeView())) {
                succeed = false;
                return;
            }
            m.SetParameter(DAE::Emission.MakeView(), color);
        }

        if (const XML::Element* specular = technique->ChildXPath({ XML::Name::Any, DAE::Specular, DAE::Color })) {
            float4 color;
            if (not XML::Parse(&color, specular->Text().MakeView())) {
                succeed = false;
                return;
            }
            m.SetParameter(DAE::Specular.MakeView(), color);
        }

        // Textures

        if (const XML::Element* texture = technique->ChildXPath({ XML::Name::Any, DAE::Diffuse, DAE::Texture })) {
            const XML::Element* sampler = doc.FindById((*texture)[DAE::Texture]);
            if (nullptr == sampler) {
                succeed = false;
                return;
            }

            const XML::Element* source = sampler->ChildXPath({ DAE::Sampler2d, DAE::Source });
            if (nullptr == source) {
                succeed = false;
                return;
            }

            const XML::Element* surface = doc.FindById(source->Text().MakeView());
            if (nullptr == surface) {
                succeed = false;
                return;
            }

            const XML::Element* init_from = surface->ChildXPath({ DAE::Surface, DAE::Init_From });
            if (nullptr == init_from) {
                succeed = false;
                return;
            }

            const XML::Element* image = doc.FindById(init_from->Text().MakeView());
            if (nullptr == image) {
                succeed = false;
                return;
            }

            init_from = image->ChildXPath({ DAE::Init_From });
            if (nullptr == init_from) {
                succeed = false;
                return;
            }

            const Filename filename(ToWString(init_from->Text()));
            m.AddTexture2D(DAE::Diffuse.MakeView(), filename);
        }
    });

    return true;
}
//----------------------------------------------------------------------------
bool Collada::Load(Collada* pdst, const Filename& filename) {
    Assert(filename.Extname() == FileSystemConstNames::DaeExt());

    pdst->_xml = new XML::Document();
    return XML::Document::Load(pdst->_xml.get(), filename);
}
//----------------------------------------------------------------------------
bool Collada::Load(Collada* pdst, const Filename& filename, const StringView& content) {
    Assert(filename.Extname() == FileSystemConstNames::DaeExt());

    pdst->_xml = new XML::Document();
    return XML::Document::Load(pdst->_xml.get(), filename, content);
}
//----------------------------------------------------------------------------
void Collada::Start() {
    DAE::Start();
}
//----------------------------------------------------------------------------
void Collada::Shutdown() {
    DAE::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
