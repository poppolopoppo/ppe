#pragma once

#include "Remoting_fwd.h"

#include "IO/String.h"
#include "Json/Json.h"
#include "RTTI_fwd.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_REMOTING_API FOpenAPI : Meta::FNonCopyableNorMovable {

    Serialize::FJson::FAllocator Alloc;
    Serialize::FJson Content{ Alloc };

    using FBuildArray = Serialize::FJson::FArray;
    using FBuildSpan = Serialize::FJson::FObject;
    using FText = Serialize::FJson::FText;

    TPtrRef<FBuildArray> Tags;
    TPtrRef<FBuildSpan> Schemas;
    TPtrRef<FBuildSpan> Paths;

    INSTANTIATE_CLASS_TYPEDEF(, FRef, FText);
    INSTANTIATE_CLASS_TYPEDEF(, FOperation, FBuildSpan);
    INSTANTIATE_CLASS_TYPEDEF(, FSchema, FBuildSpan);
    INSTANTIATE_CLASS_TYPEDEF(, FTag, FBuildSpan);

    ASSOCIATIVE_SPARSEARRAY(Json, FText, FRef) Refs;

    using FTagFunc = TFunction< void(FTag*) >;
    using FOperationFunc = TFunction< void(FOperation*) >;
    using FSchemaFunc = TFunction< void(FSchema*) >;

    FOpenAPI() = default;
    FOpenAPI(
        const FStringView& title,
        const FStringView& version,
        const FStringView& description,
        const FStringView& hostUri ) {
        Header(title, version, description, hostUri);
    }

    ~FOpenAPI();

    void Header(
        const FStringView& title,
        const FStringView& version,
        const FStringView& description,
        const FStringView& hostUri );

    // ** Schema **
    FSchema AllOf(
        const FStringView& description,
        std::initializer_list<FStringView> composed,
        FSchema&& additional );
    FSchema Array(
        const FStringView& description,
        const FStringView& type,
        const FStringView& format );
    FSchema Array(const FStringView& description, const FRef& schema);
    FSchema Array(const FStringView& description, FSchema&& items);
    FSchema Enum(
        const FStringView& description,
        const FStringView& type,
        const FStringView& format,
        std::initializer_list<FStringView> values );
    FSchema Object(
        const FStringView& description,
        FBuildSpan&& properties,
        std::initializer_list<FStringView> required,
        bool additionalProperties = false );
    FSchema OneOf(
        const FStringView& description,
        std::initializer_list<FStringView> refs );
    FSchema OneOf(
        const FStringView& description,
        FBuildArray&& list );;
    FSchema Ref(const Serialize::FJson::FText& literal);
    FSchema Scalar(
        const FStringView& description,
        const FStringView& type,
        const FStringView& format,
        const FSchemaFunc& schema );
    FSchema Scalar(const FStringView& description, const FStringView& type, const FStringView& format);

    // ** Tag **
    void Tag(const FStringView& name, const FStringView& description, const FTagFunc& tag);

    // ** Schemas **
    const FRef& Schema(const FStringView& name, FSchemaFunc&& definition, bool force_override = false);

    // ** Operation **
    void Operation(
        const FStringView& id,
        const FStringView& path,
        const FStringView& method,
        const FStringView& summary,
        const FStringView& description,
        std::initializer_list<FStringView> tags,
        const FOperationFunc& operation );
    void Operation_Body(FOperation* operation,
        const FStringView& mediaType, const FStringView& description, FSchema&& schema, bool required = false);
    void Operation_Parameter(FOperation* operation,
        const FStringView& in, const FStringView& name, const FStringView& description, FSchema&& schema, bool required = false);
    void Operation_Response(FOperation* operation,
        const FStringView& code, const FStringView& mediaType, const FStringView& description, FSchema&& schema );
    void Operation_Response(FOperation* operation, const FStringView& code, const FStringView& description);

    // ** RTTI **
    void DefineRTTISchemas();
    bool DefineRTTIMetaClass(const RTTI::FMetaClass& class_, const RTTI::FMetaClass* parent = nullptr);
    void Traits(FSchema* schema, const RTTI::PTypeTraits& traits);

    static FText LiteralText(const FStringView& str) {
        return Serialize::FJson::LiteralText(str);
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
