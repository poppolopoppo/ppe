#pragma once

#include "Core/Core.h"

namespace Core {
namespace MiniDump {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct UserData
{
    uint32_t Type;
    size_t BufferSize;
    void * Buffer;
};
//----------------------------------------------------------------------------
struct MemoryLocation
{
    size_t MemorySize;
    void * MemoryBase;
};
//----------------------------------------------------------------------------
enum class Result
{
    Success = 0,
    NoDbgHelpDLL,
    InvalidFilename,
    CantCreateFile,
    DumpFailed,
    FailedToCloseHandle,
    NotAvailable,
};
//----------------------------------------------------------------------------
void Start();
void Shutdown();
//----------------------------------------------------------------------------
Result Write(   const char *filename, void *exception_ptrs,
                const UserData *pdata, size_t dataCount,
                const MemoryLocation *pmemory, size_t memoryCount );
//----------------------------------------------------------------------------
const char *ResultMessage(Result code);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace MiniDump
} //!namespace Core
