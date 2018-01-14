#include "stdafx.h"

#include "TestApp.h"

namespace Core {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern void Test_Allocators();
extern void Test_Format();
extern void Test_Containers();
extern void Test_RTTI();
//extern void Test_Lattice(); %_NOCOMMIT% TODO
extern void Test_Network();
extern void Test_Pixmap();
extern void Test_Thread();
//extern void Test_XML(); %_NOCOMMIT% TODO
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

    Test_Format();
    Test_Allocators();
    Test_Containers();
    Test_Thread();
    Test_RTTI();
    //Test_Lattice(); %_NOCOMMIT%
    Test_Pixmap();
    //Test_XML(); %_NOCOMMIT%
     //Test_Network(); %_NOCOMMIT%
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
