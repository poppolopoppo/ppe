// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Misc/Function.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct foo_t {
    int mem(int n) {
        return 42 + n;
    }
    int mem_const(int n) const noexcept {
        return 42 + n;
    }
    void noop() const {

    }
    template <typename... _Args>
    int var_add(_Args&&... args) const noexcept {
        return 42 + (static_cast<int>(std::forward<_Args>(args)) + ...);
    }
    const char* yes() const noexcept {
        return "yes";
    }
    const char* no() const noexcept {
        return "no";
    }
};

int free_fun() {
    return 69;
}

void hello_world() {
}

template <auto _Lhs, auto _Rhs>
struct check_equal_to {
    static constexpr bool value{_Lhs == _Rhs};
};


struct non_copyable_t {
    non_copyable_t() = default;
    non_copyable_t(const non_copyable_t& ) = delete;
};

int test_function() {
    using namespace Meta;
    int (*test)() = []() { return 3; };
    foo_t foo;
    const foo_t foo_const;

    STATIC_ASSERT(std::is_invocable_r_v<int, decltype(test)>);
    STATIC_ASSERT(std::is_invocable_r_v<int, decltype(&foo_t::mem), foo_t*, int>);
    STATIC_ASSERT(std::is_invocable_r_v<int, decltype(&foo_t::mem_const), const foo_t*, int>);

    TCallableObject obj{test};
    TCallableObject obj2{&foo_t::mem};
    TCallableObject obj3 = obj2;
    TCallableObject obj4{ [](int n){ return n + 1; } };
    TCallableObject obj5{ [obj, &foo, obj3](int n){ return obj3(&foo, obj()) * (2 + n); } };
    TCallableObject obj6{ []() -> int { return 20; } };

    STATIC_ASSERT(std::is_same_v<int(), TStaticFunction<&free_fun>::function_type>);
    STATIC_ASSERT(std::is_same_v<int(int), TStaticFunction<&foo_t::mem>::function_type>);
    STATIC_ASSERT(std::is_same_v<int(), decltype(obj)::function_type>);
    STATIC_ASSERT(std::is_same_v<int(int), decltype(obj4)::function_type>);
    STATIC_ASSERT(std::is_same_v<int(), decltype(obj6)::function_type>);

    TFunctionRef<int()> fref1{ &free_fun };
    TFunctionRef fref2 = &free_fun;
    TFunctionRef<int()> fref3{ StaticFunction<&free_fun> };
    // TFunctionRef fref3_0{ StaticFunction<&free_fun> };
    TFunctionRef<int(int)> fref4( StaticFunction<&foo_t::mem>, foo );
    TFunctionRef<int(int)> fref5( StaticFunction<&foo_t::mem_const>, foo );
    TFunctionRef<int()> fref6{ []() -> int { return 33; } };
    TFunctionRef fref7{ fref6 };
    TFunctionRef<void()> fref8( StaticFunction<&foo_t::noop>, &foo_const );

    auto t1 = std::forward_as_tuple(32, false);
    auto t2 = std::forward_as_tuple("toto", 24.4);
    auto t3 = std::tuple_cat(
        std::forward_as_tuple(32, false), 
        std::forward_as_tuple("toto", 24.4));

    TFunction<int()> ref{obj};
    TFunction<int(foo_t&, int)> ref2{obj2};
    TFunction<int(int)> ref3{obj2, &foo};
    TFunction<int(int)> ref4{&foo_t::mem, &foo};
    TFunction<int(int)> ref5{&foo_t::mem_const, foo};
    TFunction<int(int)> ref6{&foo_t::mem_const, &foo_const};
    TFunction<int()> ref7{test};
    TFunction ref8{test};
    TFunction ref9{&foo_t::mem, &foo};
    TFunction ref10{TStaticFunction<&foo_t::mem>{}, &foo};
    TFunction ref11 = TStaticFunction<&free_fun>{};

    TStaticFunction<&free_fun> fn;
    TStaticFunction<&foo_t::mem> fn2;
    TStaticFunction<&foo_t::mem_const> fn3;
    TStaticFunction<&foo_t::yes> fn4;
    TStaticFunction<&foo_t::var_add<int, int, int>> fn5;

    fn();
    fn2(&foo, -2);
    fn3(foo, -3);
    fn4(foo);
    fn5(foo, 1, 2, 3);

    TFunction<int()> fnref{*fn};
    TFunction<int()> fnref2 = TFunction<int()>::Bind<&free_fun>();
    TFunction<int(foo_t&, int)> fnref3 = TFunction<int(foo_t&, int)>::Bind<&foo_t::mem>();
    TFunction<int(int)> fnref4 = TFunction<int(int)>::Bind<&foo_t::mem>(&foo);
    TFunction<int(int)> fnref5 = fnref4;
    TFunction<void()> fnref6 = &hello_world;
    fnref6();
    TFunction<void()> fnref7;

    TFunction<int()> fnref8 = TFunction<int()>::Bind<&foo_t::mem_const>(foo, 10);   
    TFunction<int(foo_t&, int)> fnref9_0{&foo_t::mem_const};
    TFunction<int(int)> fnref9_1{&foo_t::mem_const, foo };
    TFunction<int()> fnref9_2{&foo_t::mem, &foo, 10 };
    TFunction<int()> fnref10{fn5, foo, 1, 2, 3 };
    TFunction<int()> fnref11{[]() -> int {
        return 0;
    }};

    fnref11();
    TFunction<int()> fnref12{[&fnref10]() -> int {
        return fnref10();
    }};

    fnref12();
    TFunction<int()> fnref13 = [fnref10]() -> int {
        return fnref10();
    };

    fnref13();
    // // TStaticFunction<32> fn2;
    TCallableObject va2r{fref3};
    TFunction<int()> fnref14{ fref3 };
    TFunction<int()> fnref15 = [fnref10, fnref11, fnref12, fnref13, fnref14]() -> int {
        return fnref10() + fnref11() + fnref12() + fnref13() + fnref14();
    };
    fnref15();

    TCallableObject var{fnref5};
    var(fnref5, 1);
    STATIC_ASSERT(Meta::TDecay<decltype(var)>::arity_v == 2);
    VariadicFunctor(fnref5, 1, &fnref5, "anything", "you", "want");

    TCallableObject obj12{ fnref12 };
    obj12(&fnref12);

    return TFunctionRef{fnref15}();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
