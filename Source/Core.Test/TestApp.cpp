#include "stdafx.h"

#include "TestApp.h"

#include "Core.RTTI/Atom.h"
#include "Core.RTTI/NativeTypes.h"
#include "Core.RTTI/TypeInfos.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/IO/Format.h"
#include "Core/Maths/ScalarVector.h"

namespace Core {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Test_Atoms_() {
    const auto print_atom = [](const RTTI::FAtom& atom) {
        const RTTI::FTypeInfos type_info = atom.TypeInfos();
        Format(std::cout, "{0}[0x{1:#8x}] : {2} = {3} (default:{4:a})\n",
            type_info.Name(),
            type_info.Id(),
            atom.HashValue(),
            atom,
            atom.IsDefaultValue() );
    };

    int i = 0;
    RTTI::FAtom iatom = RTTI::MakeAtom(&i);
    print_atom(iatom);

    TPair<FString, double> p = { "toto", 42.69 };
    RTTI::FAtom patom = RTTI::MakeAtom(&p);
    print_atom(patom);

    TVector<float2> v = { float2(0), float2(1) };
    RTTI::FAtom vatom = RTTI::MakeAtom(&v);
    print_atom(vatom);

    VECTOR(RTTI, float2) v2;
    RTTI::FAtom vatom2 = RTTI::MakeAtom(&v2);

    if (not vatom.PromoteMove(vatom2))
        AssertNotReached();

    print_atom(vatom);
    print_atom(vatom2);

    TAssociativeVector<RTTI::FName, bool> d;
    d.Add(RTTI::FName("Toto")) = true;
    d.Add(RTTI::FName("Split")) = false;
    RTTI::FAtom datom = RTTI::MakeAtom(&d);
    print_atom(datom);

    TAssociativeVector<FString, TVector<word4> > t;
    t.Add("Toto").assign({ word4(0), word4(1), word4(2) });
    t.Add("Split").assign({ word4(3) });
    t.Add("Tricky").assign({ word4(4), word4(5) });
    RTTI::FAtom tatom = RTTI::MakeAtom(&t);
    print_atom(tatom);

    TAssociativeVector<FString, TVector<word4> > t2;
    RTTI::FAtom tatom2 = RTTI::MakeAtom(&t2);

    if (not tatom.CopyTo(tatom2))
        AssertNotReached();

    print_atom(tatom);
    print_atom(tatom2);

    if (not tatom2.MoveTo(tatom))
        AssertNotReached();

    print_atom(tatom);
    print_atom(tatom2);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTestApp::FTestApp()
    : parent_type(L"TestApp") {
}
//----------------------------------------------------------------------------
FTestApp::~FTestApp() {}
//----------------------------------------------------------------------------
void FTestApp::Start() {
    parent_type::Start();

    Test_Atoms_();
}
//----------------------------------------------------------------------------
void FTestApp::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace Core
