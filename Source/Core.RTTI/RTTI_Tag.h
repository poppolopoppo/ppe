#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTI_fwd.h"

#include "Core/Container/IntrusiveList.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Thread/AtomicSpinLock.h"

#ifdef USE_MEMORY_DOMAINS
#   define WITH_CORE_RTTI_TAGNAME //%__NOCOMMIT%
#endif

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMetaClassBaseDecl {
    typedef void (*create_delegate)();
    typedef void (*destroy_delegate)() ;

    create_delegate Create;
    create_delegate Destroy;

    FMetaClassBaseDecl(create_delegate&& create, destroy_delegate&& destroy)
        : Create(std::move(create))
        , Destroy(std::move(destroy)) {}

    FMetaClassBaseDecl(const FMetaClassBaseDecl& ) = delete;
    FMetaClassBaseDecl& operator=(const FMetaClassBaseDecl& ) = delete;

    mutable IntrusiveListNode<FMetaClassBaseDecl> Node;
};
//----------------------------------------------------------------------------
template <typename _Tag>
struct TMetaClassDecl : public FMetaClassBaseDecl {
    using typename FMetaClassBaseDecl::create_delegate;
    using typename FMetaClassBaseDecl::destroy_delegate;

    TMetaClassDecl(create_delegate&& create, destroy_delegate&& destroy)
        : FMetaClassBaseDecl(std::move(create), std::move(destroy)) {
        _Tag::Register(this);
    }

    ~TMetaClassDecl() {
        _Tag::Unregister(this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define _RTTI_TAG_DECL_IMPL(_NameId, _NameStr) \
    namespace RTTI_Tag { \
        struct _NameId { \
        public: \
            static FStringView Name() { return (_NameStr); } \
            \
            static void Register(const Core::RTTI::FMetaClassBaseDecl* pdecl); \
            static void Unregister(const Core::RTTI::FMetaClassBaseDecl* pdecl); \
            \
            static void Start(); \
            static void Shutdown(); \
            \
            _NameId() { Start(); } \
            ~_NameId() { Shutdown(); } \
            \
        }; \
    }
//----------------------------------------------------------------------------
#ifdef WITH_CORE_RTTI_TAGNAME
#   define RTTI_TAG_DECL(_Name) _RTTI_TAG_DECL_IMPL(_Name, STRINGIZE(_Name))
#else
#   define RTTI_TAG_DECL(_Name) _RTTI_TAG_DECL_IMPL(_Name, "")
#endif
//----------------------------------------------------------------------------
#define RTTI_TAG_FWD(_Name) namespace RTTI_Tag { struct _Name; }
//----------------------------------------------------------------------------
#define RTTI_TAG(_Name) RTTI_Tag::_Name
//----------------------------------------------------------------------------
RTTI_TAG_DECL(Default) // Default tag for RTTI base metaclasses
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
