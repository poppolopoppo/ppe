#include "stdafx.h"

#include "Serialize.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Text/Grammar.h"
#include "XML/XML.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs plac�s dans la zone d'initialisation d'une biblioth�que
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace Serialize {
POOL_TAG_DEF(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void SerializeStartup::Start() {
    POOL_TAG(Serialize)::Start();
    Lexer::LexerStartup::Start();
    Parser::ParserStartup::Start();
    GrammarStartup::Start();
    XML::XMLStartup::Start();
}
//----------------------------------------------------------------------------
void SerializeStartup::Shutdown() {
    XML::XMLStartup::Shutdown();
    GrammarStartup::Shutdown();
    Parser::ParserStartup::Shutdown();
    Lexer::LexerStartup::Shutdown();
    POOL_TAG(Serialize)::Shutdown();
}
//----------------------------------------------------------------------------
void SerializeStartup::ClearAll_UnusedMemory() {
    Lexer::LexerStartup::ClearAll_UnusedMemory();
    Parser::ParserStartup::ClearAll_UnusedMemory();
    GrammarStartup::ClearAll_UnusedMemory();
    XML::XMLStartup::ClearAll_UnusedMemory();
    POOL_TAG(Serialize)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
