// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Misc/Opaque.h"
#include "Misc/OpaqueBuilder.h"

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

    value<> value;
    TBuilder<> builder(&value);
    builder.Object([&]() {
        builder.KeyValue("First name", "Larry");
        builder.KeyValue("Last name", "Treinor");
        builder.KeyValue("Age", 38);
        builder.KeyValue("Male", true);
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
        TStaticAllocator<default_allocator>::Deallocate(value_block.block);
    };

    PPE_LOG(Test_Opaq, Info, "builder block: {}", value_block);
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
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
