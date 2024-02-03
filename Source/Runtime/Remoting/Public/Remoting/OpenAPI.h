#pragma once

#include "Remoting_fwd.h"

#include "Container/AssociativeVector.h"
#include "IO/TextFormat.h"
#include "Json/Json.h"
#include "Memory/MemoryDomain.h"
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

    // INSTANTIATE_CLASS_TYPEDEF(, FRef, FText);
    // INSTANTIATE_CLASS_TYPEDEF(, FOperation, FBuildSpan);
    // INSTANTIATE_CLASS_TYPEDEF(, FSchema, FBuildSpan);
    // INSTANTIATE_CLASS_TYPEDEF(, FTag, FBuildSpan);
    using FRef = FText;
    using FOperation = FBuildSpan;
    using FSchema = FBuildSpan;
    using FTag = FBuildSpan;

    ASSOCIATIVE_VECTOR(Opaq, FText, FRef) Refs;

    using FTagFunc = TFunction< void(FTag*) >;
    using FOperationFunc = TFunction< void(FOperation*) >;
    using FSchemaFunc = TFunction< void(FSchema*) >;

    FOpenAPI() = default;
    FOpenAPI(
        FText&& title,
        FText&& version,
        FText&& description,
        FText&& hostUri ) {
        Header(std::move(title), std::move(version), std::move(description), std::move(hostUri));
    }

    ~FOpenAPI();

    void Header(
        FText&& title,
        FText&& version,
        FText&& description,
        FText&& hostUri );

    // ** Schema **
    FSchema AllOf(
        FText&& description,
        FText&& composed,
        FSchema&& additional );
    FSchema AllOf(
        FText&& description,
        std::initializer_list<FStringLiteral> composed,
        FSchema&& additional );
    FSchema Array(
        FText&& description,
        FText&& type,
        FText&& format );
    FSchema Array(FText&& description, const FRef& schema);
    FSchema Array(FText&& description, FSchema&& items);
    FSchema Enum(
        FText&& description,
        FText&& type,
        FText&& format,
        std::initializer_list<FStringLiteral> values );
    FSchema Object(
        FText&& description,
        FBuildSpan&& properties,
        std::initializer_list<FStringLiteral> required,
        bool additionalProperties = false );
    FSchema OneOf(
        FText&& description,
        std::initializer_list<FStringLiteral> refs );
    FSchema OneOf(
        FText&& description,
        FBuildArray&& list );;
    FSchema Ref(Serialize::FJson::FText&& ref);
    FSchema Ref(const Serialize::FJson::FText& literal) { return Ref(FText(literal)); }
    FSchema Scalar(
        FText&& description,
        FText&& type,
        FText&& format,
        const FSchemaFunc& schema );
    FSchema Scalar(FText&& description, FText&& type, FText&& format);

    // ** Tag **
    void Tag(FText&& name, FText&& description, const FTagFunc& tag);

    // ** Schemas **
    const FRef& Schema(FText&& name, FSchemaFunc&& definition, bool force_override = false);

    // ** Operation **
    void Operation(
        FText&& id,
        FText&& path,
        FText&& method,
        FText&& summary,
        FText&& description,
        std::initializer_list<FStringLiteral> tags,
        const FOperationFunc& operation );
    void Operation_Body(FOperation* operation,
        FText&& mediaType, FText&& description, FSchema&& schema, bool required = false);
    void Operation_Parameter(FOperation* operation,
        FText&& in, FText&& name, FText&& description, FSchema&& schema, bool required = false);
    void Operation_Response(FOperation* operation,
        FText&& code, FText&& mediaType, FText&& description, FSchema&& schema );
    void Operation_Response(FOperation* operation, FText&& code, FText&& description);

    // ** RTTI **
    void DefineRTTISchemas();
    bool DefineRTTIMetaClass(const RTTI::FMetaClass& class_, const RTTI::FMetaClass* parent = nullptr);
    void Traits(FSchema* schema, const RTTI::PTypeTraits& traits);

    template <typename _Arg0, typename ..._Args>
    NODISCARD FText Format(FStringLiteral format, _Arg0&& arg0, _Args&&... args) {
        return TextFormat(Content.Allocator(), format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
