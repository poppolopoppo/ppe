#include "stdafx.h"

#include "WindowMessage.h"

#include <type_traits>

#ifdef OS_WINDOWS
#   include <windows.h>
#else
#   error "no support"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(WM_NULL == (u32)WindowMessage::Null);
STATIC_ASSERT(WM_CREATE == (u32)WindowMessage::Create);
STATIC_ASSERT(WM_DESTROY == (u32)WindowMessage::Destroy);
STATIC_ASSERT(WM_MOVE == (u32)WindowMessage::Move);
STATIC_ASSERT(WM_SIZE == (u32)WindowMessage::Size);
STATIC_ASSERT(WM_ACTIVATE == (u32)WindowMessage::Activate);
STATIC_ASSERT(WM_SETFOCUS == (u32)WindowMessage::Focus);
STATIC_ASSERT(WM_KILLFOCUS == (u32)WindowMessage::NoFocus);
STATIC_ASSERT(WM_ENABLE == (u32)WindowMessage::Enable);
STATIC_ASSERT(WM_PAINT == (u32)WindowMessage::Paint);
STATIC_ASSERT(WM_CLOSE == (u32)WindowMessage::Close);
STATIC_ASSERT(WM_QUIT == (u32)WindowMessage::Quit);
STATIC_ASSERT(WM_SHOWWINDOW == (u32)WindowMessage::Show);
STATIC_ASSERT(WM_KEYDOWN == (u32)WindowMessage::KeyDown);
STATIC_ASSERT(WM_KEYUP == (u32)WindowMessage::KeyUp);
STATIC_ASSERT(WM_SYSKEYDOWN == (u32)WindowMessage::SysKeyDown);
STATIC_ASSERT(WM_SYSKEYUP == (u32)WindowMessage::SysKeyUp);
STATIC_ASSERT(WM_MOUSEMOVE == (u32)WindowMessage::MouseMove);
STATIC_ASSERT(WM_LBUTTONDOWN == (u32)WindowMessage::LButtonDown);
STATIC_ASSERT(WM_LBUTTONUP == (u32)WindowMessage::LButtonUp);
STATIC_ASSERT(WM_LBUTTONDBLCLK == (u32)WindowMessage::LButtonDblClick);
STATIC_ASSERT(WM_RBUTTONDOWN == (u32)WindowMessage::RButtonDown);
STATIC_ASSERT(WM_RBUTTONUP == (u32)WindowMessage::RButtonUp);
STATIC_ASSERT(WM_RBUTTONDBLCLK == (u32)WindowMessage::RButtonDblClick);
STATIC_ASSERT(WM_MBUTTONDOWN == (u32)WindowMessage::MButtonDown);
STATIC_ASSERT(WM_MBUTTONUP == (u32)WindowMessage::MButtonUp);
STATIC_ASSERT(WM_MBUTTONDBLCLK == (u32)WindowMessage::MButtonDblClick);
STATIC_ASSERT(WM_MOUSEWHEEL == (u32)WindowMessage::MouseWheel);
STATIC_ASSERT(WM_MOUSEHOVER == (u32)WindowMessage::MouseHover);
STATIC_ASSERT(WM_MOUSELEAVE == (u32)WindowMessage::MouseLeave);
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<MessageWParam COMMA WPARAM>::value);
STATIC_ASSERT(std::is_same<MessageLParam COMMA LPARAM>::value);
STATIC_ASSERT(std::is_same<MessageResult COMMA LRESULT>::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView WindowMessageToCStr(WindowMessage msg) {
    switch (msg)
    {
    case WindowMessage::Show: return MakeStringView("Show");
    case WindowMessage::KeyDown: return MakeStringView("KeyDown");
    case WindowMessage::KeyUp: return MakeStringView("KeyUp");
    case WindowMessage::MouseMove: return MakeStringView("MouseMove");
    case WindowMessage::LButtonDown: return MakeStringView("LButtonDown");
    case WindowMessage::LButtonUp: return MakeStringView("LButtonUp");
    case WindowMessage::LButtonDblClick: return MakeStringView("LButtonDblClick");
    case WindowMessage::RButtonDown: return MakeStringView("RButtonDown");
    case WindowMessage::RButtonUp: return MakeStringView("RButtonUp");
    case WindowMessage::RButtonDblClick: return MakeStringView("RButtonDblClick");
    case WindowMessage::MButtonDown: return MakeStringView("MButtonDown");
    case WindowMessage::MButtonUp: return MakeStringView("MButtonUp");
    case WindowMessage::MButtonDblClick: return MakeStringView("MButtonDblClick");
    case WindowMessage::MouseWheel: return MakeStringView("MouseWheel");
    case WindowMessage::MouseHover: return MakeStringView("MouseHover");
    case WindowMessage::MouseLeave: return MakeStringView("MouseLeave");
    default:
        return MakeStringView("@Unknown");
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
