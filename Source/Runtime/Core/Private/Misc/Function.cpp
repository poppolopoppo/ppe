#include "stdafx.h"

#include "Misc/Function.h"

#include "HAL/PlatformMemory.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBaseFunction::~FBaseFunction() {
    if (is_destructible_())
        Meta::Destroy(payload_());
}
//----------------------------------------------------------------------------
void FBaseFunction::Reset() {
    if (is_destructible_())
        Meta::Destroy(payload_());

    _data = 0;
    ONLY_IF_ASSERT(FPlatformMemory::Memset(&_inSitu, 0xDD, GInSituSize));
}
//----------------------------------------------------------------------------
bool FBaseFunction::Equals(const FBaseFunction& other) const {
    return (_data == other._data && 
            FPlatformMemory::Memcmp(&_inSitu, &other._inSitu, GInSituSize) == 0);
}
//----------------------------------------------------------------------------
void FBaseFunction::Swap(FBaseFunction& other) {
    std::swap(_data, other._data);
    std::swap(_inSitu, other._inSitu);
}
//----------------------------------------------------------------------------
FBaseFunction::FBaseFunction(intptr_t data)
    : _data(data) {
    ONLY_IF_ASSERT(FPlatformMemory::Memset(&_inSitu, 0xCC, GInSituSize));
}
//----------------------------------------------------------------------------
FBaseFunction& FBaseFunction::operator =(const FBaseFunction& other) {
    Reset();
    assign_copy_(other);
    return (*this);
}
//----------------------------------------------------------------------------
FBaseFunction& FBaseFunction::operator =(FBaseFunction&& rvalue) {
    Reset();
    Swap(rvalue);
    return (*this);
}
//----------------------------------------------------------------------------
void FBaseFunction::assign_copy_(const FBaseFunction& other) {
    _data = other._data;

    if (_data && is_wrapped_()) {
        if (is_destructible_())
            ((const IPayload_*)&other._inSitu)->CopyTo(&_inSitu);
        else
            FPlatformMemory::Memcpy(&_inSitu, &other._inSitu, sizeof(_inSitu));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
