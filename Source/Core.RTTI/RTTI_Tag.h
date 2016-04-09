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
struct MetaClassBaseDecl {
    typedef void (*create_delegate)();
    typedef void (*destroy_delegate)() ;

    create_delegate Create;
    create_delegate Destroy;

    MetaClassBaseDecl(create_delegate&& create, destroy_delegate&& destroy)
        : Create(std::move(create))
        , Destroy(std::move(destroy)) {}

    MetaClassBaseDecl(const MetaClassBaseDecl& ) = delete;
    MetaClassBaseDecl& operator=(const MetaClassBaseDecl& ) = delete;

    mutable IntrusiveListNode<MetaClassBaseDecl> Node;
};
//----------------------------------------------------------------------------
template <typename _Tag>
struct MetaClassDecl : public MetaClassBaseDecl {
    using typename MetaClassBaseDecl::create_delegate;
    using typename MetaClassBaseDecl::destroy_delegate;

    MetaClassDecl(create_delegate&& create, destroy_delegate&& destroy)
        : MetaClassBaseDecl(std::move(create), std::move(destroy)) {
        _Tag::Register(this);
    }

    ~MetaClassDecl() {
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
            static const char* Name() { return (_NameStr); } \
            \
            static void Register(const Core::RTTI::MetaClassBaseDecl* pdecl); \
            static void Unregister(const Core::RTTI::MetaClassBaseDecl* pdecl); \
            \
            static void Start(); \
            static void Shutdown(); \
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
