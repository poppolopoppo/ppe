// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Misc/Opaque.h"
#include "Misc/OpaqueBuilder.h"


#include "Allocator/SlabAllocator.h"
#include "Allocator/SlabHeap.h"
#include "Diagnostic/Logger.h"
#include "Meta/Utility.h"
#include "Misc/Function.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Opaq)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Test_Opaq_Init_() {
    using namespace Opaq;

    value_init{ FStringLiteral("ansi literal string") };
    value_init{ "ansi literal string2"_view };
    value_init{ "ansi literal string2"_literal };
    value_init{ L"wide literal string" };

    value_block obj_block = NewBlock(object_init{
        {"first name", "Paul"},
        {"address", object_init{
            {"number", 13},
            {"street", "rue des Cascades"},
            {"zipcode", 75020},
            {"city", "Paris"}
        }},
        {"children", array_init{
            object_init{
                {"first name", L"Anaïs"},
                {"age", 6},
            },
            object_init{
                {"first name", L"Théophile"},
                {"age", 1},
            },
        }},
        {"bag", array_init{
            true,
            "butter",
            L"süß",
            3.1418,
            array_init{1,2,3, array_init{true, 0.5f, object_init{
                {"x", 1/6.0},
                {"y", -2/6.0},
                {"z", 3/6.0},
            }}},
        }},
    });

    PPE_LOG(Test_Opaq, Info, "block object: {}", *obj_block);

    Unused(object_init{
        {"recursive", std::get<object_view>(*obj_block)}
    });


    PPE_SLOG(Test_Opaq, Info, "block format:", {
        {"format", [&](FTextWriter& oss){
            Format(oss, "this is {} {}", 2, "awesome");
        }},
        {"wformat", [&](FWTextWriter& oss){
            Format(oss, L"this is {} {}", 2, MakeStringView("awesome"));
        }}
    });

    DeleteBlock(obj_block);
}
//----------------------------------------------------------------------------
static void Test_Opaq_Alloc_() {
    using namespace Opaq;

    string<> small = MakeStringView("sm");
    Unused(small);

    string<> address = MakeStringView("33 colombus street Alabama");

    value<> value;
    TBuilder<> builder(&value);
    builder.Object([&]() {
        builder.KeyValue("First name", "Larry");
        builder.KeyValue("Last name", "Treinor");
        builder.KeyValue("Age", 38);
        builder.KeyValue("Male", true);
        builder.KeyValue("Address", std::move(address));
        builder.KeyValue("Children", [&]() {
            builder.Array([&]() {
                builder.Object([&]() {
                    builder.KeyValue("First name", "Tania");
                    builder.KeyValue("Last name", "Treinor");
                }, 2);
                builder.Object([&]() {
                    builder.KeyValue("First name", "Tony");
                    builder.KeyValue("Last name", "Treinor");
                }, 2);
            }, 2);
        });
    });

    PPE_LOG(Test_Opaq, Info, "builder object: {}", value);

    value_block value_block = builder.ToValueBlock(
        TStaticAllocator<default_allocator>::Allocate(builder.BlockSize()));
    DEFERRED {
        TStaticAllocator<default_allocator>::Deallocate(value_block.Reset());
    };

    PPE_LOG(Test_Opaq, Info, "builder block: {}", value_block);
}
//----------------------------------------------------------------------------
static void Test_Opaq_Slab_() {
    using namespace Opaq;

    using heap_type = SLABHEAP(UnitTest);
    heap_type heap;

    struct unit_test_slab_tag{};
    using allocator_type = TThreadLocalSlabAllocator<unit_test_slab_tag, ALLOCATOR(UnitTest)>;

    array<allocator_type> arr;
    Unused(arr);

    value<allocator_type> value;
    TBuilder<allocator_type> builder(&value);
    {
        const allocator_type::FScope scope(heap);

        builder.Object([&]() {
        builder.KeyValue("First name", "Larry"_view);
        builder.KeyValue("Last name", "Memoization Treinor"_view);
        builder.KeyValue("Age", 38);
        builder.KeyValue("Male", true);
        builder.KeyValue("Children", [&]() {
            builder.Array([&]() {
                builder.Object([&]() {
                    builder.KeyValue("First name", "Tania"_view);
                    builder.KeyValue("Last name", "Memoization Treinor");
                    builder.KeyValue("Long string", "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book."_view);
                    }, 2);
                builder.Object([&]() {
                    builder.KeyValue("First name", "Tony");
                    builder.KeyValue("Last name", "Memoization Treinor"_view);
                    builder.KeyValue("Long string", "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book."_view);
                    }, 2);
                }, 2);
            });
        });
    }

    PPE_LOG(Test_Opaq, Info, "slab builder object: {}", value);

    value_block value_block = builder.ToValueBlock(
        TStaticAllocator<default_allocator>::Allocate(builder.BlockSize()));
    DEFERRED {
        TStaticAllocator<default_allocator>::Deallocate(value_block.block);
    };

    value.Reset();
    heap.DiscardAll();

    PPE_LOG(Test_Opaq, Info, "slab builder block: {}", value_block);
}
//----------------------------------------------------------------------------
static void Test_Opaq_SlabMemoization_() {
    using namespace Opaq;

    using heap_type = SLABHEAP(UnitTest);
    heap_type heap;

    struct unit_test_slab_tag{};
    using allocator_type = TThreadLocalSlabAllocator<unit_test_slab_tag, ALLOCATOR(UnitTest)>;

    array<allocator_type> arr;
    Unused(arr);

    value<allocator_type> value;
    TBuilder<allocator_type> builder(&value);

    TBuilder<allocator_type>::text_memoization_ansi memoization_ansi(builder);
    TBuilder<allocator_type>::text_memoization_wide memoization_wide(builder);
    builder.SetTextMemoization(memoization_ansi);
    builder.SetTextMemoization(memoization_wide);

    {
        const allocator_type::FScope scope(heap);

        builder.Object([&]() {
        builder.KeyValue("First name", "Larry"_view);
        builder.KeyValue("Last name", "Memoization Treinor"_view);
        builder.KeyValue("Age", 38);
        builder.KeyValue("Male", true);
        builder.KeyValue("Children", [&]() {
            builder.Array([&]() {
                builder.Object([&]() {
                    builder.KeyValue("First name", "Tania"_view);
                    builder.KeyValue("Last name", "Memoization Treinor"_view);
                    builder.KeyValue("Long string", "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book."_view);
                    }, 2);
                builder.Object([&]() {
                    builder.KeyValue("First name", "Tony");
                    builder.KeyValue("Last name", "Memoization Treinor"_view);
                    builder.KeyValue("Long string", "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book."_view);
                    }, 2);
                }, 2);
            });
        });
    }

    PPE_LOG(Test_Opaq, Info, "slab builder object memoized: {}", value);

    value_block value_block = builder.ToValueBlock(
        TStaticAllocator<default_allocator>::Allocate(builder.BlockSize()));
    DEFERRED {
        TStaticAllocator<default_allocator>::Deallocate(value_block.block);
    };

    value.Reset();
    memoization_ansi.clear_ReleaseMemory();
    memoization_wide.clear_ReleaseMemory();
    heap.DiscardAll();

    PPE_LOG(Test_Opaq, Info, "slab builder block memoized: {}", value_block);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Opaq() {
    PPE_DEBUG_NAMEDSCOPE("Test_Opaq");

    Test_Opaq_Init_();
    Test_Opaq_Alloc_();
    Test_Opaq_Slab_();
    Test_Opaq_SlabMemoization_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
