#include "stdafx.h"

#include "Remoting/OpenAPI.h"

#include "Uri.h"

#include "MetaClass.h"
#include "MetaObject.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/OpaqueData.h"
#include "RTTI/TypeTraits.h"

#include "IO/Format.h"
#include "IO/StringBuilder.h"
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
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            description, "boolean", Default, NoFunction );
    });
}
//----------------------------------------------------------------------------
template <typename T>
static void DefineNativeSchema_(
    Meta::TEnableIf<std::is_integral_v<T>, FOpenAPI*> api,
    Meta::TType<T>,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        FStringView format;
        IF_CONSTEXPR(std::is_unsigned_v<T>)
            format = (sizeof(T) > sizeof(u16) ? "int64" : "int32");
        else // OAS doesn't support unsigned integers bcoz JS/JSON encoding long on 53 bits
            format = (sizeof(T) > sizeof(u32) ? "int64" : "int32");

        *schema = api->Scalar(description, "integer", format, NoFunction);

        IF_CONSTEXPR(std::is_unsigned_v<T>)
            schema->Add("minimum"_json).Assign(static_cast<Serialize::FJson::FInteger>(0));
    });
}
//----------------------------------------------------------------------------
template <typename T>
static void DefineNativeSchema_(
    Meta::TEnableIf<std::is_floating_point_v<T>, FOpenAPI*> api,
    Meta::TType<T>,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            description, "number", std::is_same_v<float, T>
                ? MakeStringView("float")
                : MakeStringView("double"), NoFunction );
    });
}
//----------------------------------------------------------------------------
template <typename _Char>
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< TBasicString<_Char> >,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            description, "string", Default, NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FName >,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            description, "string", "token", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< FDirpath >,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            description, "string", "path", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< FFilename >,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            description, "string", "filename", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FBinaryData >,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Scalar(
            description, "string", "binary", NoFunction );
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FAny >,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    const std::initializer_list<FStringView> variants{
#define PPE_REMOTING_NATIVE_SCHEMA_REF(_Name, _Type, _Uid) \
            STRINGIZE(_Name),
        FOREACH_RTTI_NATIVETYPES(PPE_REMOTING_NATIVE_SCHEMA_REF)
#undef PPE_REMOTING_NATIVE_SCHEMA_REF
        "OpaqueArray",
        "OpaqueData"
    };

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        FOpenAPI::FBuildSpan properties{ api->Content.Heap() };
        properties.Add(Serialize::FJson::TypeId).Assign(std::move(api->Scalar(Default, "string"_json, Default).Inner()));
        *schema = api->Object(description, std::move(properties), { Serialize::FJson::TypeId });

        FOpenAPI::FBuildSpan mapping{ api->Content.Heap() };
        mapping.reserve(variants.size());
        for (const FStringView& variant : variants) {
            if (variant == "Any") continue;
            mapping.Add(FOpenAPI::LiteralText(variant))
                .Assign(api->Content.MakeText(StringFormat("#/components/schemas/Any{0}", variant)));
        }

        auto& discriminator = schema->Add("discriminator"_json)
            .Construct<FOpenAPI::FBuildSpan>(api->Content.Heap());
        discriminator.Add("propertyName"_json).Assign(Serialize::FJson::TypeId);
        discriminator.Add("mapping"_json).Assign(std::move(mapping));
    });

    for (const FStringView& variant : variants) {
        if (variant == "Any") continue;

        api->Schema(
            FString("Any") + variant,
            [&](FOpenAPI::FSchema* schema) {
                FOpenAPI::FBuildSpan properties{ api->Content.Heap() };
                properties.Add("inner"_json).Assign(std::move(api->Ref(
                    api->Content.MakeText(StringFormat("#/components/schemas/{0}", variant))).Inner()));

                *schema = api->AllOf(Default,
                    { "#/components/schemas/Any" },
                    api->Object(Default, std::move(properties), { "inner"_json }));
            });
    }
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FOpaqueArray >,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Array(
            description, FOpenAPI::FRef{ "#/components/schemas/Any"_json });
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::FOpaqueData >,
    const FStringView& name,
    const FStringView& description ) {
    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        *schema = api->Object(description, {}, {}, true);
    });
}
//----------------------------------------------------------------------------
static void DefineNativeSchema_(
    FOpenAPI* api,
    Meta::TType< RTTI::PMetaObject >,
    const FStringView& name,
    const FStringView& description ) {
    using namespace RTTI;

    Assert(api);
    Assert(not name.empty());

    api->Schema(name, [&](FOpenAPI::FSchema* schema) {
        FOpenAPI::FBuildSpan properties(api->Content.Heap());
        properties.Add(Serialize::FJson::Class).Assign(std::move(api->Scalar(
            "object class", "string", Default).Inner()));
        properties.Add(Serialize::FJson::Export).Assign(std::move(api->Scalar(
            "name if exported", "string", Default).Inner()));
        properties.Add(Serialize::FJson::TopObject).Assign(std::move(api->Scalar(
            "true if top object", "boolean", Default).Inner()));

        *schema = api->Object(
            description,
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

    api->Schema(class_.Name(), [&](FOpenAPI::FSchema* schema) {
        auto propertySchema = [api](const RTTI::FMetaProperty* prop) -> FOpenAPI::FBuildSpan::value_type {
            FOpenAPI::FSchema item;
            api->Traits(&item, prop->Traits());
            return {
                FOpenAPI::LiteralText(prop->Name().MakeView()),
                std::move(item.Inner()) };
        };

        FString parentSchema;
        FOpenAPI::FBuildSpan properties(api->Content.Heap());
        if (not parent) {
            parent = RTTI::FMetaObject::RTTI_FMetaClass::Get();
            parentSchema = "#/components/schemas/MetaObject";
            properties.insert(class_.AllProperties().Map(propertySchema));
        }
        else {
            parentSchema = StringFormat("#/components/schemas/{0}", parent->Name());
            properties.insert(class_.AllProperties()
                .FilterBy([parent](const RTTI::FMetaProperty* prop) {
                    // only declare properties which were not declared in the parent
                    return (not parent->HasProperty(*prop, true));
                })
                .Map(propertySchema));
        }

        *schema = api->AllOf(Default,
            { parentSchema },
            api->Object(Default, std::move(properties), {}) );

        if (IsAbstractSchema_(class_)) {
            for (const RTTI::FMetaClass* child : class_.Children())
                api->DefineRTTIMetaClass(*child, std::addressof(class_));
        }

        for (; parent->Parent(); parent = parent->Parent());
        FOpenAPI::FBuildSpan& parentDecl = api->Schemas->Get(FOpenAPI::LiteralText(parent->Name())).ToObject();

        TPtrRef<FOpenAPI::FBuildSpan> mappings;
        auto& discriminator = parentDecl.FindOrAdd("discriminator"_json);
        if (Likely(discriminator.Valid())) {
            mappings = discriminator.ToObject().Get("mapping"_json).ToObject();
        }
        else {
            auto& dico = discriminator.Construct<FOpenAPI::FBuildSpan>(api->Content.Heap());
            dico.Add("propertyName"_json).Assign(Serialize::FJson::Class);
            mappings = dico.Add("mapping"_json).Construct<FOpenAPI::FBuildSpan>(api->Content.Heap());
        }

        mappings->Add(FOpenAPI::LiteralText(class_.Name()))
             .Assign(api->Content.MakeText(StringFormat("#/components/schemas/{0}", class_.Name())));
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
    const FStringView& title,
    const FStringView& version,
    const FStringView& description,
    const FStringView& hostUri ) {
    Assert(not title.empty());
    Assert(not version.empty());
    Assert(not description.empty());
    Assert(not hostUri.empty());

    using namespace RTTI; // for ""_json

    FBuildSpan info(Content.Heap());
    info.Add("title"_json) = Content.MakeText(title);
    info.Add("version"_json) = Content.MakeText(version);
    info.Add("description"_json) = Content.MakeText(description);

    FBuildSpan server(Content.Heap());
    server.Add("url"_json) = Content.MakeText(hostUri);

    FBuildSpan& header = Content.Root().Construct<FBuildSpan>(Content.Heap());
    header.reserve(6);
    header.Add("openapi"_json) = "3.0.1"_json;
    header.Add("info"_json).Assign(std::move(info));
    header.Add("servers"_json).Construct<FBuildArray>(Content.Heap()).Emplace(std::move(server));

    Tags = header.Add("tags"_json).Construct<FBuildArray>(Content.Heap());
    Paths = header.Add("paths"_json).Construct<FBuildSpan>(Content.Heap());
    Schemas = header.Add("components"_json)
        .Construct<FBuildSpan>(Content.Heap())
        .Add("schemas"_json)
        .Construct<FBuildSpan>(Content.Heap());
}
//----------------------------------------------------------------------------
// Schema
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::AllOf(
    const FStringView& description,
    std::initializer_list<FStringView> composed,
    FSchema&& additional ) {
    Assert(composed.size() > 0);

    FSchema span(Content.Heap());

    if (not description.empty())
        span.Add("description"_json).Assign(Content.MakeText(description));

    FBuildArray& allOf = span.Add("allOf"_json)
        .Construct<FBuildArray>(Content.Heap());

    allOf.Append(MakeIterable(composed).Map([this](const FStringView& name) {
        return Ref(Content.MakeText(name)).Inner();
    }));

    if (not additional.empty())
        allOf.Emplace(std::move(additional.Inner()));

    return span;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Array(const FStringView& description, const FStringView& type, const FStringView& format) {
    using namespace RTTI;

    Assert(not type.empty());

    FSchema items(Content.Heap());
    items.Add("type"_json).Assign(Content.MakeText(type));

    if (not format.empty())
        items.Add("format"_json).Assign(Content.MakeText(format));

    return Array(description, std::move(items));
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Array(const FStringView& description, const FRef& schema) {
    using namespace RTTI;

    Assert(not schema.empty());

    return Array(description, Ref(schema.Inner()));
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Array(const FStringView& description, FSchema&& items) {
    using namespace RTTI;

    Assert(not items.empty());

    FSchema schema(Content.Heap());
    schema.Add("type"_json).Assign("array"_json);
    schema.Add("items"_json)
        .Assign(std::move(items.Inner()));

    if (not description.empty())
        schema.Add("description"_json).Assign(Content.MakeText(description));

    return schema;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Enum(const FStringView& description, const FStringView& type, const FStringView& format, std::initializer_list<FStringView> values) {
    using namespace RTTI;

    return Scalar(description, type, format, [&](FSchema* span) {
        span->Add("enum"_json).Assign(FBuildArray{
            MakeIterable(values).Map([this](const FStringView& x) {
                return Content.MakeText(x);
            }),
            Content.Heap()
        });
    });
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Object(
    const FStringView& description,
    FBuildSpan&& properties,
    std::initializer_list<FStringView> required,
    bool additionalProperties/* = false */) {
    using namespace RTTI;

    return Scalar(description, "object", Default, [&](FSchema* schema) {
        schema->Add("properties"_json)
            .Assign(std::move(properties));

        schema->Add("additionalProperties"_json).Assign(additionalProperties);

        if (required.size() > 0)
            schema->Add("required"_json).Assign(FBuildArray{
                MakeIterable(required).Map([this](const FStringView& x) {
                    return Content.MakeText(x);
                }),
                Content.Heap()
            });
    });
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::OneOf(const FStringView& description, std::initializer_list<FStringView> refs) {
    Assert(refs.size() > 0);

    FBuildArray list(Content.Heap());
    list.Append(MakeIterable(refs).Map([this](FStringView ref) {
        return Ref(FOpenAPI::LiteralText(ref)).Inner();
    }));

    return OneOf(description, std::move(list));
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::OneOf(const FStringView& description, FBuildArray&& list) {
    Assert(list.size() > 0);

    FSchema span(Content.Heap());
    span.Add("oneOf"_json).Assign(std::move(list));

    if (not description.empty())
        span.Add("description"_json).Assign(Content.MakeText(description));

    return span;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Ref(const FOpenAPI::FText& literal) {
    Assert(not literal.empty());

    FSchema schema(Content.Heap());
    schema.Add("$ref"_json).Assign(literal);
    return schema;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Scalar(const FStringView& description, const FStringView& type, const FStringView& format, const FSchemaFunc& schema) {
    Assert(not type.empty());

    FSchema span(Content.Heap());
    span.Add("type"_json).Assign(Content.MakeText(type));

    if (not format.empty())
        span.Add("format"_json).Assign(Content.MakeText(format));

    if (not description.empty())
        span.Add("description"_json).Assign(Content.MakeText(description));

    if (schema.Valid())
        schema(&span);

    return span;
}
//----------------------------------------------------------------------------
FOpenAPI::FSchema FOpenAPI::Scalar(const FStringView& description, const FStringView& type, const FStringView& format) {
    return Scalar(description, type, format, NoFunction);
}
//----------------------------------------------------------------------------
// Tag
//----------------------------------------------------------------------------
void FOpenAPI::Tag(const FStringView& name, const FStringView& description, const FTagFunc& tag) {
    Assert(Tags);
    Assert(not name.empty());
    Assert(not Tags->Iterable().Any([&](const Serialize::FJson::FValue& x) {
        return (x.ToObject().Get("name"_json).ToString() == name);
    }));

    FTag span(Content.Heap());
    span.Add("name"_json) = Content.MakeText(name);

    if (not description.empty())
        span.Add("description"_json) = Content.MakeText(description);

    if (tag.Valid())
        tag(&span);

    Emplace_Back(*Tags, std::move(span.Inner()));
}
//----------------------------------------------------------------------------
// Definition
//----------------------------------------------------------------------------
const FOpenAPI::FRef& FOpenAPI::Schema(
    const FStringView& name,
    FSchemaFunc&& definition,
    bool force_override/* = false */) {
    Assert(not name.empty());
    Assert(definition);

    const FText key = Content.MakeText(name);
    const auto it = Refs.Find(key);
    if (it == Refs.end() || force_override) {
        const FRef& result = Refs.FindOrAdd(key) = FRef{
            Content.MakeText(StringFormat("#/components/schemas/{0}", name))
        };
        FBuildSpan schema{ Content.Heap() };
        definition(static_cast<FSchema*>(&schema));
        Schemas->FindOrAdd(key).Assign(std::move(schema));
        return result;
    }

    return it->second;
}
//----------------------------------------------------------------------------
// Operation
//----------------------------------------------------------------------------
void FOpenAPI::Operation(
    const FStringView& id,
    const FStringView& path,
    const FStringView& method,
    const FStringView& summary,
    const FStringView& description,
    std::initializer_list<FStringView> tags,
    const FOperationFunc& operation ) {
    Assert(not id.empty());
    Assert(not path.empty());
    Assert(path.StartsWith(Network::FUri::PathSeparator));
    Assert(not method.empty());
    Assert(operation.Valid());

    FOperation span(Content.Heap());
    span.Add("operationId"_json).Assign(Content.MakeText(id));
    span.Add("parameters"_json).Construct<FBuildArray>(Content.Heap());
    span.Add("responses"_json).Construct<FBuildSpan>(Content.Heap());

    if (not summary.empty())
        span.Add("summary"_json).Assign(Content.MakeText(summary));

    if (not description.empty())
        span.Add("description"_json).Assign(Content.MakeText(description));

    if (not empty(tags)) span.Add("tags"_json).Assign(FBuildArray{
        MakeIterable(tags).Map([this](const FStringView& x) {
            Assert(not x.empty());
            return Content.MakeText(x);
        }),
        Content.Heap()
    });

    operation(&span);

    Assert_NoAssume(not span.Get("responses"_json).ToObject().empty());

    (*Paths)
        .FindOrAdd(Content.MakeText(path))
        .Construct<FBuildSpan>(Content.Heap())
        .Add(Content.MakeText(method))
        .Assign(std::move(span.Inner()));
}
//----------------------------------------------------------------------------
void FOpenAPI::Operation_Body(FOperation* operation, const FStringView& mediaType, const FStringView& description, FSchema&& schema, bool required) {
    Assert(operation);
    Assert(not mediaType.empty());
    Assert(not schema.empty());

    auto& body = operation->FindOrAdd("requestBody"_json).Construct<FBuildSpan>(Content.Heap());

    if (required)
        body.FindOrAdd("required"_json).Assign(true);

    if (not description.empty())
        body.FindOrAdd("description"_json).Assign(Content.MakeText(description));

    body.FindOrAdd("content"_json).Construct<FBuildSpan>(Content.Heap())
        .Add(Content.MakeText(mediaType)).Construct<FBuildSpan>(Content.Heap())
        .Add("schema"_json)
        .Assign(std::move(schema.Inner()));
}
//----------------------------------------------------------------------------
void FOpenAPI::Operation_Parameter(FOperation* operation, const FStringView& in, const FStringView& name, const FStringView& description, FSchema&& schema, bool required) {
    Assert(operation);
    Assert(not in.empty());
    Assert(not name.empty());
    Assert(not schema.empty());

    FBuildSpan parameter(Content.Heap());
    parameter.Add("in"_json).Assign(Content.MakeText(in));
    parameter.Add("name"_json).Assign(Content.MakeText(name));
    parameter.Add("schema"_json).Assign(std::move(schema.Inner()));

    if (required)
        parameter.Add("required"_json).Assign(true);

    if (not description.empty())
        parameter.FindOrAdd("description"_json).Assign(Content.MakeText(description));

    operation->Get("parameters"_json)
        .ToArray()
        .Emplace(std::move(parameter));
}
//----------------------------------------------------------------------------
void FOpenAPI::Operation_Response(FOperation* operation, const FStringView& code, const FStringView& description) {
    Assert(operation);
    Assert(not code.empty());
    Assert(not description.empty());

    auto& response = operation->Get("responses"_json)
        .ToObject()
        .Add(Content.MakeText(code))
        .Construct<FBuildSpan>(Content.Heap());

    if (not description.empty())
        response.FindOrAdd("description"_json).Assign(Content.MakeText(description));
}
//----------------------------------------------------------------------------
void FOpenAPI::Operation_Response(FOperation* operation, const FStringView& code, const FStringView& mediaType, const FStringView& description, FSchema&& schema) {
    Assert(operation);
    Assert(not code.empty());
    Assert(not mediaType.empty());
    Assert(not schema.empty());

    auto& response = operation->Get("responses"_json)
        .ToObject()
        .Add(Content.MakeText(code))
        .Construct<FBuildSpan>(Content.Heap());

    FBuildSpan& content = response.Add("content"_json).Construct<FBuildSpan>(Content.Heap());
    content
        .Add(Content.MakeText(mediaType))
        .Construct<FBuildSpan>(Content.Heap())
        .Add("schema"_json)
        .Assign(std::move(schema.Inner()));

    if (not description.empty())
        response.FindOrAdd("description"_json).Assign(Content.MakeText(description));
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
            *schema = Ref(Content.MakeText(StringFormat("#/components/schemas/{0}", class_->Name())));
        }
        else
            *schema = Ref(Content.MakeText(StringFormat("#/components/schemas/{0}",
                MakeTraits(scalar->NativeType())->TypeName()/* erase concrete type name */) ));
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
    if (Schemas->end() == Schemas->Find(Content.MakeText(class_.Name().MakeView()))) {
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

    DefineNativeSchema_(this, Meta::Type<FOpaqueArray>, "OpaqueArray",
        ToString(RTTI::MakeTraits<FOpaqueArray>()->TypeId()) );
    DefineNativeSchema_(this, Meta::Type<FOpaqueData>, "OpaqueData",
        ToString(RTTI::MakeTraits<FOpaqueData>()->TypeId()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
