#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Memory/MemoryView.h"
#include "IO/String_fwd.h"

namespace PPE {
namespace Opaq {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// trivial opaque types
//----------------------------------------------------------------------------
using nil = std::monostate;
using boolean = bool;
using integer = i64;
using uinteger = u64;
using floating_point = double;
//----------------------------------------------------------------------------
// string literals are never copied/allocated
//----------------------------------------------------------------------------
using string_literal = FStringLiteral;
using wstring_literal = FWStringLiteral;
//----------------------------------------------------------------------------
// external strings are used for strings not allocated inside a value_block
//----------------------------------------------------------------------------
using string_external = FConstChar;
using wstring_external = FConstWChar;
//----------------------------------------------------------------------------
// main wrapper struct forward declaration
//----------------------------------------------------------------------------
struct value_init;
struct key_value_init;
struct value_view;
struct key_value_view;
//----------------------------------------------------------------------------
// *_init variants are used for inline definition
//----------------------------------------------------------------------------
using string_init = FStringView;
using wstring_init = FWStringView;
using array_init = TMemoryView<const value_init>;
using object_init = TMemoryView<const key_value_init>;
//----------------------------------------------------------------------------
// default allocator for opaque types
//----------------------------------------------------------------------------
using default_allocator = ALLOCATOR(Opaq);
//----------------------------------------------------------------------------
// dynamic value allocator can be overriden
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
struct value;
template <typename _Allocator = default_allocator>
struct key_value;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE
