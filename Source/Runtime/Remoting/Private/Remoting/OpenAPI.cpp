// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Remoting/OpenAPI.h"

#include <shobjidl_core.h>

#include "Uri.h"

#include "MetaClass.h"
#include "MetaObject.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/OpaqueData.h"
#include "RTTI/TypeTraits.h"

#include "IO/Format.h"
#include "IO/TextFormat.h"
#include "Meta/BitField.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType<bool>,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            std::move(description), "boolean", "", NoFunction );
    });
}
//----------------------------------------------------------------------------
template <typename T>
static void DefineNativeSchema_(
    Meta::TEnableIf<std::is_integral_v<T>, FOpenAPI*> api,
    Meta::TType<T>,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        FStringLiteral format;
        IF_CONSTEXPR(std::is_unsigned_v<T>)
            format = (sizeof(T) > sizeof(u16) ? "int64" : "int32");
        else // OAS doesn't support unsigned integers bcoz JS/JSON encoding long on 53 bits
            format = (sizeof(T) > sizeof(u32) ? "int64" : "int32");

        *schema = api->Scalar(std::move(description), "integer", format, NoFunction);

        IF_CONSTEXPR(std::is_unsigned_v<T>)
            schema->emplace_back("minimum"_json, 0);
    });
}
//----------------------------------------------------------------------------
template <typename T>
static void DefineNativeSchema_(
    Meta::TEnableIf<std::is_floating_point_v<T>, FOpenAPI*> api,
    Meta::TType<T>,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            std::move(description), "number", std::is_same_v<float, T>
                ? FStringLiteral("float")
                : FStringLiteral("double"), NoFunction );
    });
}
//----------------------------------------------------------------------------
template <typename _Char>
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< TBasicString<_Char> >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            std::move(description), "string", "", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FName >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            std::move(description), "string", "token", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< FDirpath >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            std::move(description), "string", "path", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< FFilename >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            std::move(description), "string", "filename", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FBinaryData >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            std::move(description), "string", "binary", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FAny >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    const FStringLiteral variants[] = {
#define PPE_REMOTING_NATIVE_SCHEMA_REF(_Name, _Type, _Uid) \
            STRINGIZE(_Name),
        FOREACH_RTTI_NATIVETYPES(PPE_REMOTING_NATIVE_SCHEMA_REF)
#undef PPE_REMOTING_NATIVE_SCHEMA_REF
        "OpaqueArray",
        "OpaqueData"
    };

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        FOpenAPI::FBuildSpan properties{ api->Content.Heap() };
        properties.emplace_back(Serialize::FJson::TypeId, api->Scalar("", "string", ""));
        properties.emplace_back(Serialize::FJson::TypeId, api->Scalar("", "string", ""));

        *schema = api->Object(std::move(description), std::move(properties), { Serialize::FJson::Literal_TypeId });

        FOpenAPI::FBuildSpan mapping{ api->Content.Heap() };
        mapping.reserve(lengthof(variants));

        for (FStringLiteral variant : variants) {
            if (variant == "Any") continue;
            mapping.emplace_back(variant, TextFormat(api->Content.Allocator(), "#/components/schemas/Any{0}", variant));
        }

        auto& discriminator =Emplace_Back(*schema, "discriminator")
            ->value.emplace<FOpenAPI::FBuildSpan>(api->Content.Heap());

        discriminator.emplace_back("propertyName", Serialize::FJson::TypeId);
        discriminator.emplace_back("mapping", std::move(mapping));
    });

    for (FStringLiteral variant : variants) {
        if (variant == "Any") continue;

        api->Schema(
            TextFormat(api->Content.Allocator(), "Any{}", variant),
            [&](FOpenAPI::FSchema* schema) {
                FOpenAPI::FBuildSpan properties{ api->Content.Heap() };
                properties.emplace_back("inner", api->Ref(
                    TextFormat(api->Content.Allocator(), "#/components/schemas/{0}", variant)));

                *schema = api->AllOf("",
                    "#/components/schemas/Any",
                    api->Object("", std::move(properties), { "inner" }));
            });
    }
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FOpaqueArray >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Array(std::move(description),  "#/components/schemas/Any");
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FOpaqueData >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        *schema = api->Object(std::move(description), FOpenAPI::FBuildSpan{ForceInit}, {}, true);
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::PMetaObject >,
    FOpenAPI::FText&& name,
    FOpenAPI::FText&& description ) {
    using namespace RTTI;

    Assert(api);
    Assert(not name.empty());

    api->Schema(std::move(name), [&](FOpenAPI::FSchema* schema) {
        FOpenAPI::FBuildSpan properties(api->Content.Heap());
        properties.emplace_back(Serialize::FJson::Class, api->Scalar(
            "object class", "string", ""));
        properties.emplace_back(Serialize::FJson::Export, api->Scalar(
            "name if exported", "string", ""));
        properties.emplace_back(Serialize::FJson::TopObject, api->Scalar(
            "true if top object", "boolean", ""));

        *schema = api->Object(
            std::move(description),
            std::move(properties),
            { "class", "properties" },
            true );
    });
}
//----------------------------------------------------------------------------
static bool IsAbstractSchema_(const RTTI::FMetaClass& class_) {
    return class_.IsAbstract() && std::addressof(class_) != RTTI::FMetaObject::RTTI_FMetaClass::Get();
}
//----------------------------------------------------------------------------
static void DefineSpecializedSchema_(
    FOpenAPI* api,
    const RTTI::FMetaClass& class_,
    const RTTI::FMetaClass* parent ) {
    Assert(api);

    api->Schema(class_.Name().MakeLiteral(), [&](FOpenAPI::FSchema* schema) {
        auto propertySchema = [api](const RTTI::FMetaProperty* prop) -> FOpenAPI::FBuildSpan::value_type {
            FOpenAPI::FSchema item{api->Content.Heap()};
            api->Traits(&item, prop->Traits());
            return {
                prop->Name().MakeLiteral(),
                std::move(item) };
        };

        FOpenAPI::FText parentSchema{ForceInit};
        FOpenAPI::FBuildSpan properties(api->Content.Heap());
        if (not parent) {
            parent = RTTI::FMetaObject::RTTI_FMetaClass::Get();
            parentSchema = "#/components/schemas/MetaObject";
            properties.append(class_.AllProperties().Map(propertySchema));
        }
        else {
            parentSchema = TextFormat(api->Content.Allocator(), "#/components/schemas/{0}", parent->Name());
            properties.append(class_.AllProperties()
                .FilterBy([parent](const RTTI::FMetaProperty* prop) {
                    // only declare properties which were not declared in the parent
                    return (not parent->HasProperty(*prop, true));
                })
                .Map(propertySchema));
        }

        *schema = api->AllOf("",
             std::move(parentSchema),
            api->Object("", std::move(properties), {}) );

        if (IsAbstractSchema_(class_)) {
            for (const RTTI::FMetaClass* child : class_.Children())
                api->DefineRTTIMetaClass(*child, std::addressof(class_));
        }

        for (; parent->Parent(); parent = parent->Parent())
            NOOP(); // look for top-most object

        FOpenAPI::FBuildSpan& parentDecl = *Opaq::XPathAs<FOpenAPI::FBuildSpan>(*api->Schemas, parent->Name().MakeLiteral());

        TPtrRef<FOpenAPI::FBuildSpan> mappings;
        auto& discriminator = Emplace_Back(parentDecl, "discriminator")->value;
        if (Likely(not discriminator.Nil())) {
            mappings = XPathAs<FOpenAPI::FBuildSpan>(discriminator, { "mapping"_json });
        }
        else {
            auto& dico = discriminator.emplace<FOpenAPI::FBuildSpan>(api->Content.Heap());
            dico.emplace_back("propertyName", Serialize::FJson::Class);
            mappings = Emplace_Back(dico, "mappings")
                ->value.emplace<FOpenAPI::FBuildSpan>(api->Content.Heap());
        }

        mappings->emplace_back(class_.Name().MakeLiteral(),
            TextFormat(api->Content.Allocator(), "#/components/schemas/{0}", class_.Name()));
    });
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FOpenAPI::~FOpenAPI() {
    Content.Clear_ForgetMemory();
}
//----------------------------------------------------------------------------
// Header
//----------------------------------------------------------------------------
void FOpenAPI::Header(
    FText&& title,
    FText&& version,
    FText&& description,
    FText&& hostUri ) {
    Assert(not title.empty());
    Assert(not version.empty());
    Assert(not description.empty());
    Assert(not hostUri.empty());

    using namespace RTTI; // for ""_json

    FBuildSpan info(Content.Heap());
    info.reserve(3);
    info.emplace_back("title", std::move(title));
    info.emplace_back("version", std::move(version));
    info.emplace_back("description", std::move(description));

    FBuildSpan server(Content.Heap());
    server.emplace_back("url", std::move(hostUri));

    FBuildArray servers(Content.Heap());
    servers.push_back(std::move(server));

    FBuildSpan header(Content.Heap());
    header.reserve(6);
    header.emplace_back("openapi", "3.0.1"_json);
    header.emplace_back("info", std::move(info));
    header.emplace_back("servers", std::move(servers));

    Tags = Emplace_Back(header, "tags")->value.emplace<FBuildArray>(Content.Heap());
    Paths = Emplace_Back(header, "tags")->value.emplace<FBuildSpan>(Content.Heap());
    Schemas = Emplace_Back(Emplace_Back(header, "components")
        ->value.emplace<FBuildSpan>(Content.Heap()), "schemas")
        ->value.emplace<FBuildSpan>(Content.Heap());
}
//----------------------------------------------------------------------------
// Schema
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::AllOf(
    FText&& description,
    FText&& composed,
    FSchema&& additional ) {
    Assert(not composed.empty());

    FSchema span(Content.Heap());

    if (not description.empty())
        span.emplace_back("description", std::move(description));

    FBuildArray& allOf = Emplace_Back(span, "allOf")
        ->value.emplace<FBuildArray>(Content.Heap());

    allOf.emplace_back(std::move(composed));

    if (not additional.empty())
        allOf.push_back(std::move(additional));

    return span;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::AllOf(
    FText&& description,
    std::initializer_list<FStringLiteral> composed,
    FSchema&& additional ) {
    Assert(composed.size() > 0);

    FSchema span(Content.Heap());

    if (not description.empty())
        span.emplace_back("description", std::move(description));

    FBuildArray& allOf = Emplace_Back(span, "allOf")
        ->value.emplace<FBuildArray>(Content.Heap());

    allOf.append(MakeIterable(composed).Map([this](FStringLiteral name) -> FRef {
        return name;
    }));

    if (not additional.empty())
        allOf.push_back(std::move(additional));

    return span;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Array(FText&& description, FText&& type, FText&& format) {
    using namespace RTTI;

    Assert(not type.empty());

    FSchema items(Content.Heap());
    items.emplace_back("type", std::move(type));

    if (not format.empty())
        items.emplace_back("format", std::move(format));

    return Array(std::move(description), std::move(items));
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Array(FText&& description, const FRef& schema) {
    using namespace RTTI;

    Assert(not schema.empty());

    return Array(std::move(description), Ref(schema));
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Array(FText&& description, FSchema&& items) {
    using namespace RTTI;

    Assert(not items.empty());

    FSchema schema(Content.Heap());
    schema.emplace_back("type", "array"_json);
    schema.emplace_back("items", std::move(items));

    if (not description.empty())
        schema.emplace_back("description", std::move(description));

    return schema;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Enum(FText&& description, FText&& type, FText&& format, std::initializer_list<FStringLiteral> values) {
    using namespace RTTI;

    return Scalar(std::move(description), std::move(type), std::move(format), [&](FSchema* span) {
        span->emplace_back("enum", FBuildArray{
            MakeIterable(values),
            Content.Heap()
        });
    });
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Object(
    FText&& description,
    FBuildSpan&& properties,
    std::initializer_list<FStringLiteral> required,
    bool additionalProperties/* = false */) {
    using namespace RTTI;

    return Scalar(std::move(description), "object", "", [&](FSchema* schema) {
        schema->emplace_back("properties", std::move(properties));
        schema->emplace_back("additionalProperties", additionalProperties);

        if (required.size() > 0)
            schema->emplace_back("required", FBuildArray{
                MakeIterable(required),
                Content.Heap()
            });
    });
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::OneOf(FText&& description, std::initializer_list<FStringLiteral> refs) {
    Assert(refs.size() > 0);

    FBuildArray list(Content.Heap());
    list.append(MakeIterable(refs));

    return OneOf(std::move(description), std::move(list));
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::OneOf(FText&& description, FBuildArray&& list) {
    Assert(list.size() > 0);

    FSchema span(Content.Heap());
    span.emplace_back("oneOf", std::move(list));

    if (not description.empty())
        span.emplace_back("description", std::move(description));

    return span;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Ref(FText&& literal) {
    Assert(not literal.empty());

    FSchema schema(Content.Heap());
    schema.emplace_back("$ref", std::move(literal));
    return schema;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Scalar(FText&& description, FText&& type, FText&& format, const FSchemaFunc& schema) {
    Assert(not type.empty());

    FSchema span(Content.Heap());
    span.emplace_back("type", std::move(type));

    if (not format.empty())
        span.emplace_back("format", std::move(format));

    if (not description.empty())
        span.emplace_back("description", std::move(description));

    if (schema.Valid())
        schema(&span);

    return span;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Scalar(FText&& description, FText&& type, FText&& format) {
    return Scalar(std::move(description), std::move(type), std::move(format), NoFunction);
}
//----------------------------------------------------------------------------
// Tag
//----------------------------------------------------------------------------
void FOpenAPI::Tag(FText&& name, FText&& description, const FTagFunc& tag) {
    Assert(Tags);
    Assert(not name.empty());
    Assert(not MakeIterable(*Tags).Any([&](const Serialize::FJson::FValue& x) {
        return (*XPathAs<Serialize::FJson::FText>(std::get<Serialize::FJson::FObject>(x), "name"_json) == name);
    }));

    FTag span(Content.Heap());
    span.emplace_back("name", std::move(name));

    if (not description.empty())
        span.emplace_back("description", std::move(description));

    if (tag.Valid())
        tag(&span);

    Emplace_Back(*Tags, std::move(span));
}
//----------------------------------------------------------------------------
// Definition
//----------------------------------------------------------------------------
const FOpenAPI::FRef& FOpenAPI::Schema(
    FText&& name,
    FSchemaFunc&& definition,
    bool force_override/* = false */) {
    Assert(not name.empty());
    Assert(definition);

    const auto it = Refs.Find(name);
    if (it == Refs.end() || force_override) {
        const FRef& result = Refs.Insert_AssertUnique(name, Format("#/components/schemas/{0}", name))->second;
        FBuildSpan schema{ Content.Heap() };
        definition(&schema);
        Schemas->emplace_back(std::move(name), std::move(schema));
        return result;
    }

    return it->second;
}
//----------------------------------------------------------------------------
// Operation
//----------------------------------------------------------------------------
void FOpenAPI::Operation(
    FText&& id,
    FText&& path,
    FText&& method,
    FText&& summary,
    FText&& description,
    std::initializer_list<FStringLiteral> tags,
    const FOperationFunc& operation ) {
    Assert(not id.empty());
    Assert(not path.empty());
    Assert(path.MakeView().StartsWith(Network::FUri::PathSeparator));
    Assert(not method.empty());
    Assert(operation.Valid());

    FOperation span(Content.Heap());
    span.emplace_back("operationId", std::move(id));
    Emplace_Back(span, "parameters")->value.emplace<FBuildArray>(Content.Heap());
    Emplace_Back(span, "responses")->value.emplace<FBuildSpan>(Content.Heap());

    if (not summary.empty())
        span.emplace_back("summary", std::move(summary));

    if (not description.empty())
        span.emplace_back("description", std::move(description));

    if (not empty(tags)) span.emplace_back("tags", FBuildArray{
        MakeIterable(tags),
        Content.Heap()
    });

    operation(&span);

    Assert_NoAssume(not XPathAs<Serialize::FJson::FObject>(span, "responses"_json)->empty());

    Emplace_Back(Emplace_Back(*Paths, std::move(path))
        ->value.emplace<FBuildSpan>(Content.Heap()),
        std::move(method))
        ->value = std::move(span);
}
//----------------------------------------------------------------------------
void FOpenAPI::Operation_Body(FOperation* operation, FText&& mediaType, FText&& description, FSchema&& schema, bool required) {
    Assert(operation);
    Assert(not mediaType.empty());
    Assert(not schema.empty());

    auto& body = Emplace_Back(*operation, "requestBody")->value.emplace<FBuildSpan>(Content.Heap());

    if (required)
        body.emplace_back("required", true);

    if (not description.empty())
        body.emplace_back("description", std::move(description));

    Emplace_Back(
    Emplace_Back(
        Emplace_Back(body, "content")
            ->value.emplace<FBuildSpan>(Content.Heap()),
        std::move(mediaType))
        ->value.emplace<FBuildSpan>(Content.Heap()),
        "schema")->value = std::move(schema);
}
//----------------------------------------------------------------------------
void FOpenAPI::Operation_Parameter(FOperation* operation, FText&& in, FText&& name, FText&& description, FSchema&& schema, bool required) {
    Assert(operation);
    Assert(not in.empty());
    Assert(not name.empty());
    Assert(not schema.empty());

    FBuildSpan parameter(Content.Heap());
    parameter.emplace_back("in", std::move(in));
    parameter.emplace_back("name", std::move(name));
    parameter.emplace_back("schema", std::move(schema));

    if (required)
        parameter.emplace_back("required", true);

    if (not description.empty())
        parameter.emplace_back("description", std::move(description));

    XPathAs<FBuildArray>(*operation, "parameters"_json)
        ->push_back(std::move(parameter));
}
//----------------------------------------------------------------------------
void FOpenAPI::Operation_Response(FOperation* operation, FText&& code, FText&& description) {
    Assert(operation);
    Assert(not code.empty());
    Assert(not description.empty());

    auto& response = Emplace_Back(*XPathAs<FBuildSpan>(*operation, "responses"_json), std::move(code))
        ->value.emplace<FBuildSpan>(Content.Heap());

    if (not description.empty())
        response.emplace_back("description", std::move(description));
}
//----------------------------------------------------------------------------
void FOpenAPI::Operation_Response(FOperation* operation, FText&& code, FText&& mediaType, FText&& description, FSchema&& schema) {
    Assert(operation);
    Assert(not code.empty());
    Assert(not mediaType.empty());
    Assert(not schema.empty());

    auto& response = Emplace_Back(*XPathAs<FBuildSpan>(*operation, "responses"_json), std::move(code))
        ->value.emplace<FBuildSpan>(Content.Heap());

    if (not description.empty())
        response.emplace_back("description", std::move(description));

    auto& content = Emplace_Back(response, "content")
        ->value.emplace<FBuildSpan>(Content.Heap());

    Emplace_Back(Emplace_Back(content, std::move(mediaType))
        ->value.emplace<FBuildSpan>(Content.Heap()),
        "schema")->value = std::move(schema);
}
//----------------------------------------------------------------------------
// Schema
//----------------------------------------------------------------------------
void FOpenAPI::Traits(FSchema* schema, const RTTI::PTypeTraits& traits) {
    Assert(schema);
    Assert(traits);

    if (const RTTI::IScalarTraits* scalar = traits->AsScalar()) {
        if (const RTTI::FMetaClass* class_ = scalar->ObjectClass()) {
            DefineRTTIMetaClass(*class_);
            *schema = Ref(TextFormat(Content.Allocator(),"#/components/schemas/{0}", class_->Name()));
        }
        else
            *schema = Ref(TextFormat(Content.Allocator(), "#/components/schemas/{0}",
                MakeTraits(scalar->NativeType())->TypeName()/* erase concrete type name */) );
    }
    else if (const RTTI::IListTraits* list = traits->AsList()) {
        FSchema item(Content.Heap());
        Traits(&item, list->ValueTraits());
        *schema = Array(list->TypeName(), std::move(item));
    }
    else if (RTTI::MakeTraits<RTTI::FOpaqueData>() == traits) {
        *schema = Ref("#/components/schemas/OpaqueData"_json);
    }
    else if (RTTI::MakeTraits<RTTI::FOpaqueArray>() == traits) {
        *schema = Ref("#/components/schemas/OpaqueArray"_json );
    }
    else {
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
bool FOpenAPI::DefineRTTIMetaClass(const RTTI::FMetaClass& class_, const RTTI::FMetaClass* parent/* = nullptr */) {
    if (not XPath(*Schemas, FRef{ class_.Name().MakeLiteral() })) {
        DefineSpecializedSchema_(this, class_, parent);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
void FOpenAPI::DefineRTTISchemas() {
    using namespace RTTI;

#define PPE_REMOTING_NATIVE_SCHEMA_DEF(_Name, _Type, _Uid) \
    DefineNativeSchema_(this, Meta::Type<_Type>, STRINGIZE(_Name), STRINGIZE(_Uid) );
    FOREACH_RTTI_NATIVETYPES(PPE_REMOTING_NATIVE_SCHEMA_DEF)
#undef PPE_REMOTING_NATIVE_SCHEMA_DEF

    DefineNativeSchema_(this, Meta::Type<FOpaqueArray>, "OpaqueArray"_json,
        TextFormat(Content.Allocator(), "{}", RTTI::MakeTraits<FOpaqueArray>()->TypeId()) );
    DefineNativeSchema_(this, Meta::Type<FOpaqueData>, "OpaqueData"_json,
        TextFormat(Content.Allocator(), "{}", RTTI::MakeTraits<FOpaqueData>()->TypeId()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
