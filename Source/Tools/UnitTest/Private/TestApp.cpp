#include "stdafx.h"

#include "TestApp.h"

namespace PPE {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern void Test_Allocators();
extern void Test_Format();
extern void Test_Containers();
extern void Test_RTTI();
extern void Test_Network();
extern void Test_Pixmap();
extern void Test_Thread();
extern void Test_XML();
extern void Test_Lattice();
extern void Test_VFS();
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

    Test_Allocators();
    Test_Containers();
    Test_Format();
    Test_Thread();
    Test_VFS();
    Test_RTTI();
    Test_XML();
    Test_Lattice();
    Test_Pixmap();
    Test_Network();
}
//----------------------------------------------------------------------------
void FTestApp::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE