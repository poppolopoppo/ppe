#pragma once

#include "Core_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "IO/String_fwd.h"
#include "Misc/Opaque_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(FeedbackContext);
//----------------------------------------------------------------------------
class IFeedbackContext {
public:
    virtual ~IFeedbackContext() = default;

    using FProgressHandle = uintptr_t;

    virtual void LogMessage(ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringView& text,
        Opaq::object_init&& object = {}) = 0;

    virtual void LogMessage(ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringLiteral& text,
        Opaq::object_init&& object = {}) = 0;

    virtual void ClearWarningsAndErrors() = 0;

    NODISCARD virtual FProgressHandle BeginProgress(const FStringView& text, size_t amount = 0) = 0;
    virtual void IncProgressPos(FProgressHandle handle) = 0;
    virtual void SetProgressPos(FProgressHandle handle, size_t at) = 0;
    virtual void SetProgressText(FProgressHandle handle, const FStringView& text) = 0;
    virtual void EndProgress(FProgressHandle&& handle) = 0;

    NODISCARD virtual bool YesNo(const FStringView& title, const FStringView& text) = 0;
};
//----------------------------------------------------------------------------
class FGlobalFeedbackContext final : public IFeedbackContext {
public:
    using FProgressHandle = IFeedbackContext::FProgressHandle;

    FGlobalFeedbackContext() = default;

    PPE_CORE_API virtual void LogMessage(ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringView& text,
        Opaq::object_init&& object = {}) override;

    PPE_CORE_API virtual void LogMessage(ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringLiteral& text,
        Opaq::object_init&& object = {}) override;

    PPE_CORE_API virtual void ClearWarningsAndErrors() override;

    NODISCARD PPE_CORE_API virtual FProgressHandle BeginProgress(const FStringView& text, size_t amount = 0) override;
    PPE_CORE_API virtual void IncProgressPos(FProgressHandle handle) override;
    PPE_CORE_API virtual void SetProgressPos(FProgressHandle handle, size_t at) override;
    PPE_CORE_API virtual void SetProgressText(FProgressHandle handle, const FStringView& text) override;
    PPE_CORE_API virtual void EndProgress(FProgressHandle&& handle) override;

    NODISCARD PPE_CORE_API virtual bool YesNo(const FStringView& title, const FStringView& text) override;

public:
    PPE_CORE_API static void Start();
    PPE_CORE_API static void Shutdown();

    PPE_CORE_API static UFeedbackContext Override(UFeedbackContext&& context) NOEXCEPT;
};
//----------------------------------------------------------------------------
class FFeedbackProgressBar : Meta::FNonCopyable {
public:
    PPE_CORE_API explicit FFeedbackProgressBar(const FStringView& text, size_t amount = 0);
    PPE_CORE_API FFeedbackProgressBar(const FStringView& text, IFeedbackContext& context);
    PPE_CORE_API FFeedbackProgressBar(const FStringView& text, size_t amount, IFeedbackContext& context);

    PPE_CORE_API FFeedbackProgressBar(FFeedbackProgressBar&& rvalue) NOEXCEPT;
    FFeedbackProgressBar& operator =(FFeedbackProgressBar&& rvalue) = delete;

    ~FFeedbackProgressBar() {
        Close();
    }

    PPE_CORE_API void Inc() const;  
    PPE_CORE_API void Set(size_t at) const;
    PPE_CORE_API void Print(const FStringView& text) const;
    PPE_CORE_API void IncPrint(const FStringView& text) const;
    PPE_CORE_API void Close();

private:
    TPtrRef<IFeedbackContext> _context;
    IFeedbackContext::FProgressHandle _handle{ Default };

    template <typename _Lambda>
    void WithContext_(_Lambda&& lambda) const;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
