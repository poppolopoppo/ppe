// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Diagnostic/FeedbackContext.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformDialog.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
LOG_CATEGORY(, FeedbackContex);
//----------------------------------------------------------------------------
class FDefaultFeedbackContext_ final : public IFeedbackContext {
public:
    FDefaultFeedbackContext_() = default;

    virtual void LogMessage(ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringView& text,
        Opaq::object_init&& object = {}) override final;

    virtual void LogMessage(ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringLiteral& text,
        Opaq::object_init&& object = {}) override final;

    virtual void ClearWarningsAndErrors() override final;

    virtual FProgressHandle BeginProgress(const FStringView& text, size_t amount = 0) override final;
    virtual void IncProgressPos(FProgressHandle handle) override final;
    virtual void SetProgressPos(FProgressHandle handle, size_t at) override final;
    virtual void SetProgressText(FProgressHandle handle, const FStringView& text) override final;
    virtual void EndProgress(FProgressHandle&& handle) override final;

    virtual bool YesNo(const FStringView& title, const FStringView& text)  override final;
};
//----------------------------------------------------------------------------
void FDefaultFeedbackContext_::LogMessage(ELoggerVerbosity verbosity,
    const FConstChar& source, u32 line,
    const FStringView& text,
    Opaq::object_init&& object/* = {} */) {
#if USE_PPE_LOGGER
    FLogger::LogStructured(LogCategory_FeedbackContex(),
        FLoggerSiteInfo{verbosity, source, line},
        text, std::move(object));
#else
    Unused(verbosity, source, line, text, object);
#endif
}
//----------------------------------------------------------------------------
void FDefaultFeedbackContext_::LogMessage(ELoggerVerbosity verbosity,
    const FConstChar& source, u32 line,
    const FStringLiteral& text,
    Opaq::object_init&& object/* = {} */) {
#if USE_PPE_LOGGER
    FLogger::LogStructured(LogCategory_FeedbackContex(),
        FLoggerSiteInfo{verbosity, source, line},
        text, std::move(object));
#else
    Unused(verbosity, source, line, text, object);
#endif
}
//----------------------------------------------------------------------------
void FDefaultFeedbackContext_::ClearWarningsAndErrors() {
    PPE_LOG_FLUSH();
}
//----------------------------------------------------------------------------
auto FDefaultFeedbackContext_::BeginProgress(const FStringView& text, size_t amount/* = 0 */) -> FProgressHandle {
    return FPlatformDialog::BeginProgress(UTF_8_TO_WCHAR(text), amount);
}
//----------------------------------------------------------------------------
void FDefaultFeedbackContext_::IncProgressPos(FProgressHandle handle) {
    FPlatformDialog::IncProgressPos(handle);
}
//----------------------------------------------------------------------------
void FDefaultFeedbackContext_::SetProgressPos(FProgressHandle handle, size_t at/* = 0.f */) {
    FPlatformDialog::SetProgressPos(handle, at);
}
//----------------------------------------------------------------------------
void FDefaultFeedbackContext_::SetProgressText(FProgressHandle handle, const FStringView& text) {
    FPlatformDialog::SetProgressText(handle, UTF_8_TO_WCHAR(text));
}
//----------------------------------------------------------------------------
void FDefaultFeedbackContext_::EndProgress(FProgressHandle&& handle) {
    FPlatformDialog::EndProgress(handle);
    handle = Zero;
}
//----------------------------------------------------------------------------
bool FDefaultFeedbackContext_::YesNo(const FStringView& title, const FStringView& text) {
    return (FPlatformDialog::YesNo(UTF_8_TO_WCHAR(text), UTF_8_TO_WCHAR(title)) == FPlatformDialog::Yes);
}
//----------------------------------------------------------------------------
static auto& GlobalFeedbackContext_() {
    ONE_TIME_DEFAULT_INITIALIZE(TThreadSafe<UFeedbackContext COMMA EThreadBarrier::CriticalSection>, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Global feedback context
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::Start() {
    GlobalFeedbackContext_().LockExclusive().Value() = MakeUnique<FDefaultFeedbackContext_>();
}
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::Shutdown() {
    GlobalFeedbackContext_().LockExclusive().Value().reset();
}
//----------------------------------------------------------------------------
UFeedbackContext FGlobalFeedbackContext::Override(UFeedbackContext&& context) NOEXCEPT {
    AssertRelease(context.valid());
    const auto exclusive = GlobalFeedbackContext_().LockExclusive();

    UFeedbackContext result = std::move(exclusive.Value());
    exclusive.Value() = std::move(context);

    return result;
}
//----------------------------------------------------------------------------
// Global helpers
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::LogMessage(ELoggerVerbosity verbosity,
    const FConstChar& source, u32 line,
    const FStringView& text,
    Opaq::object_init&& object/* = {} */) {
    GlobalFeedbackContext_().LockShared()->LogMessage(verbosity, source, line, text, std::move(object));
}
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::LogMessage(ELoggerVerbosity verbosity,
    const FConstChar& source, u32 line,
    const FStringLiteral& text,
    Opaq::object_init&& object/* = {} */) {
    GlobalFeedbackContext_().LockShared()->LogMessage(verbosity, source, line, text, std::move(object));
}
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::ClearWarningsAndErrors() {
    GlobalFeedbackContext_().LockShared()->ClearWarningsAndErrors();
}
//----------------------------------------------------------------------------
auto FGlobalFeedbackContext::BeginProgress(const FStringView& text, size_t amount/* = 0 */) -> FProgressHandle {
    return GlobalFeedbackContext_().LockShared()->BeginProgress(text, amount);
}
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::IncProgressPos(FProgressHandle handle) {
    GlobalFeedbackContext_().LockShared()->IncProgressPos(handle);
}
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::SetProgressPos(FProgressHandle handle, size_t at) {
    GlobalFeedbackContext_().LockShared()->SetProgressPos(handle, at);
}
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::SetProgressText(FProgressHandle handle, const FStringView& text) {
    GlobalFeedbackContext_().LockShared()->SetProgressText(handle, text);
}
//----------------------------------------------------------------------------
void FGlobalFeedbackContext::EndProgress(FProgressHandle&& handle) {
    return GlobalFeedbackContext_().LockShared()->EndProgress(std::move(handle));
}
//----------------------------------------------------------------------------
bool FGlobalFeedbackContext::YesNo(const FStringView& title, const FStringView& text) {
    return GlobalFeedbackContext_().LockShared()->YesNo(title, text);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFeedbackProgressBar::FFeedbackProgressBar(const FStringView& text, size_t amount/* = 0*/)
:   _handle(FGlobalFeedbackContext{}.BeginProgress(text, amount))
{}
//----------------------------------------------------------------------------
FFeedbackProgressBar::FFeedbackProgressBar(const FStringView& text, IFeedbackContext& context)
:   FFeedbackProgressBar(text, 0, context)
{}
//----------------------------------------------------------------------------
FFeedbackProgressBar::FFeedbackProgressBar(const FStringView& text, size_t amount, IFeedbackContext& context)
:   _context(context)
,   _handle(context.BeginProgress(text, amount))
{}
//----------------------------------------------------------------------------
FFeedbackProgressBar::FFeedbackProgressBar(FFeedbackProgressBar&& rvalue) NOEXCEPT
:   _context(std::move(rvalue._context))
,   _handle(std::move(rvalue._handle)) {
    rvalue._handle = Default;
}
//----------------------------------------------------------------------------
template <typename _Lambda>
void FFeedbackProgressBar::WithContext_(_Lambda&& lambda) const {
    if (_handle != Default) {
        if (_context.valid())
            return lambda(_context);

        FGlobalFeedbackContext gbl;
        return lambda(gbl);
    }
}
//----------------------------------------------------------------------------
void FFeedbackProgressBar::Inc() const {
    WithContext_([this](IFeedbackContext& ctx) {
        ctx.IncProgressPos(_handle);
    });
}
//----------------------------------------------------------------------------
void FFeedbackProgressBar::Set(size_t at) const {
    WithContext_([this, at](IFeedbackContext& ctx) {
        ctx.SetProgressPos(_handle, at);
    });
}
//----------------------------------------------------------------------------
void FFeedbackProgressBar::Print(const FStringView& text) const {
    WithContext_([this, &text](IFeedbackContext& ctx) {
        ctx.SetProgressText(_handle, text);
    });
}
//----------------------------------------------------------------------------
void FFeedbackProgressBar::IncPrint(const FStringView& text) const {
    WithContext_([this, &text](IFeedbackContext& ctx) {
        ctx.IncProgressPos(_handle);
        ctx.SetProgressText(_handle, text);
    });
}
//----------------------------------------------------------------------------
void FFeedbackProgressBar::Close() {
    WithContext_([this](IFeedbackContext& ctx) {
        ctx.EndProgress(std::move(_handle));
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
