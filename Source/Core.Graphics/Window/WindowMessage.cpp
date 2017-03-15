#include "stdafx.h"

#include "WindowMessage.h"

#include <type_traits>

#ifdef PLATFORM_WINDOWS
#   include <windows.h>
#else
#   error "no support"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(WM_NULL == (u32)EWindowMessage::Null);
STATIC_ASSERT(WM_CREATE == (u32)EWindowMessage::Create);
STATIC_ASSERT(WM_DESTROY == (u32)EWindowMessage::Destroy);
STATIC_ASSERT(WM_MOVE == (u32)EWindowMessage::Move);
STATIC_ASSERT(WM_SIZE == (u32)EWindowMessage::Size);
STATIC_ASSERT(WM_ACTIVATE == (u32)EWindowMessage::Activate);
STATIC_ASSERT(WM_SETFOCUS == (u32)EWindowMessage::Focus);
STATIC_ASSERT(WM_KILLFOCUS == (u32)EWindowMessage::NoFocus);
STATIC_ASSERT(WM_ENABLE == (u32)EWindowMessage::Enable);
STATIC_ASSERT(WM_PAINT == (u32)EWindowMessage::Paint);
STATIC_ASSERT(WM_CLOSE == (u32)EWindowMessage::Close);
STATIC_ASSERT(WM_QUIT == (u32)EWindowMessage::Quit);
STATIC_ASSERT(WM_SHOWWINDOW == (u32)EWindowMessage::Show);
STATIC_ASSERT(WM_KEYDOWN == (u32)EWindowMessage::KeyDown);
STATIC_ASSERT(WM_KEYUP == (u32)EWindowMessage::KeyUp);
STATIC_ASSERT(WM_SYSKEYDOWN == (u32)EWindowMessage::SysKeyDown);
STATIC_ASSERT(WM_SYSKEYUP == (u32)EWindowMessage::SysKeyUp);
STATIC_ASSERT(WM_MOUSEMOVE == (u32)EWindowMessage::MouseMove);
STATIC_ASSERT(WM_LBUTTONDOWN == (u32)EWindowMessage::LButtonDown);
STATIC_ASSERT(WM_LBUTTONUP == (u32)EWindowMessage::LButtonUp);
STATIC_ASSERT(WM_LBUTTONDBLCLK == (u32)EWindowMessage::LButtonDblClick);
STATIC_ASSERT(WM_RBUTTONDOWN == (u32)EWindowMessage::RButtonDown);
STATIC_ASSERT(WM_RBUTTONUP == (u32)EWindowMessage::RButtonUp);
STATIC_ASSERT(WM_RBUTTONDBLCLK == (u32)EWindowMessage::RButtonDblClick);
STATIC_ASSERT(WM_MBUTTONDOWN == (u32)EWindowMessage::MButtonDown);
STATIC_ASSERT(WM_MBUTTONUP == (u32)EWindowMessage::MButtonUp);
STATIC_ASSERT(WM_MBUTTONDBLCLK == (u32)EWindowMessage::MButtonDblClick);
STATIC_ASSERT(WM_MOUSEWHEEL == (u32)EWindowMessage::MouseWheel);
STATIC_ASSERT(WM_MOUSEHOVER == (u32)EWindowMessage::MouseHover);
STATIC_ASSERT(WM_MOUSELEAVE == (u32)EWindowMessage::MouseLeave);
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<MessageWParam COMMA WPARAM>::value);
STATIC_ASSERT(std::is_same<MessageLParam COMMA LPARAM>::value);
STATIC_ASSERT(std::is_same<MessageResult COMMA LRESULT>::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView WindowMessageToCStr(EWindowMessage msg) {
    switch (msg)
    {
    case EWindowMessage::Show: return MakeStringView("Show");
    case EWindowMessage::KeyDown: return MakeStringView("KeyDown");
    case EWindowMessage::KeyUp: return MakeStringView("KeyUp");
    case EWindowMessage::MouseMove: return MakeStringView("MouseMove");
    case EWindowMessage::LButtonDown: return MakeStringView("LButtonDown");
    case EWindowMessage::LButtonUp: return MakeStringView("LButtonUp");
    case EWindowMessage::LButtonDblClick: return MakeStringView("LButtonDblClick");
    case EWindowMessage::RButtonDown: return MakeStringView("RButtonDown");
    case EWindowMessage::RButtonUp: return MakeStringView("RButtonUp");
    case EWindowMessage::RButtonDblClick: return MakeStringView("RButtonDblClick");
    case EWindowMessage::MButtonDown: return MakeStringView("MButtonDown");
    case EWindowMessage::MButtonUp: return MakeStringView("MButtonUp");
    case EWindowMessage::MButtonDblClick: return MakeStringView("MButtonDblClick");
    case EWindowMessage::MouseWheel: return MakeStringView("MouseWheel");
    case EWindowMessage::MouseHover: return MakeStringView("MouseHover");
    case EWindowMessage::MouseLeave: return MakeStringView("MouseLeave");
    default:
        return MakeStringView("@Unknown");
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
