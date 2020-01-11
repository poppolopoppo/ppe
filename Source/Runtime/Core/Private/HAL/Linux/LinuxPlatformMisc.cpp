#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformMisc.h"

#ifdef PLATFORM_LINUX

#include "Allocator/Alloca.h"
#include "Container/StringHashMap.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformMemory.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "Meta/Utility.h"
#include "Misc/Guid.h"
#include "Time/Timepoint.h"

#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"
#include "HAL/Linux/LinuxPlatformFile.h"
#include "HAL/Linux/LinuxPlatformProcess.h"
#include "HAL/Linux/LinuxPlatformThread.h"

#include <fstream>

#include <cpuid.h>
#include <thread>

#include <sys/signal.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FLinuxPlatformMisc::FCPUInfo FetchCPUInfo_() {
    u32 eax, ebx, ecx, edx;
    __cpuid(0, eax, ebx, ecx, edx);

    FLinuxPlatformMisc::FCPUInfo result;
    *(u32*)&result = eax;

    return result;
}
//----------------------------------------------------------------------------
static size_t FetchNumCores_() {
    // Only physical cores
    return checked_cast<size_t>(::get_nprocs());
}
//----------------------------------------------------------------------------
static size_t FetchNumCoresWithSMT_() {
    // Get the number of logical processors, including hyper-threaded ones.
    return std::thread::hardware_concurrency();
}
//----------------------------------------------------------------------------
void GracefulTerminationHandler_(i32 signal, ::siginfo_t* info, void* context) {
    UNUSED(info);
    UNUSED(context);

    static bool GHasRequestedExit_ = false;

    if (GHasRequestedExit_) {
        // Keeping the established shell practice of returning 128 + signal for terminations by signal. Allows to distinguish SIGINT/SIGTERM/SIGHUP.
        _exit(128 + signal);
    }
    else {
        FLUSH_LOG();
        GHasRequestedExit_ = true;
    }
}
//----------------------------------------------------------------------------
struct FPersistentData_ : Meta::FNonCopyableNorMovable {

    class FStore {
    public:
        FStore() NOEXCEPT = default;
        explicit FStore(const FStringView& storeId)
        :   _storeId(storeId)
        {}

        FStringView StoreId() const { return _storeId; }

        FString ExportPath() const {
            FStringBuilder oss;
            oss << "~/.ppe/" << _storeId << ".cfg";
            return oss.ToString();
        }

        bool Get(const FStringView& section, const FStringView& key, FString* pValue) const {
            Assert(not section.empty());
            Assert(not key.empty());
            Assert(pValue);

            STACKLOCAL_TEXTWRITER(oss, 256);
            oss << section << '.' << key;
            const FStringView uid = oss.Written();
            const hash_t h = hash_string(uid);

            const auto it = _entries.find_like(uid, h);
            if (_entries.end() == it)
                return false;

            pValue->assign(it->second.MakeView());
            return true;
        }

        bool Erase(const FStringView& section, const FStringView& key) {
            Assert(not section.empty());
            Assert(not key.empty());

            STACKLOCAL_TEXTWRITER(oss, 256);
            oss << section << '.' << key;
            const FStringView uid = oss.Written();
            const hash_t h = hash_string(uid);

            const auto it = _entries.find_like(uid, h);
            if (_entries.end() == it)
                return false;

            _entries.erase(it);
            return true;
        }

        void Set(const FStringView& section, const FStringView& key, const FStringView& value) {
            Assert(not section.empty());
            Assert(not key.empty());

            FStringBuilder oss;
            oss << section << '.' << key;
            const FString uid = oss.ToString();

            _entries.insert_or_assign(uid, value);
        }

        bool Read(std::ifstream& iss) {
            char buf[256];
            while (iss.good()) {
                iss.getline(buf, lengthof(buf));
                if (iss.fail())
                    break;

                FStringView ln = MakeCStringView(buf);
                ln = Chomp(Strip(ln));

                if (ln.StartsWith(';'))
                    continue; // comment line

                const auto eq = ln.Find('=');
                if (eq == ln.end()) {
                    LOG(HAL, Error, L"malformed entry while reading persistent store {0}: {1}",
                        _storeId.MakeView(), ln );
                    return false;
                }

                const FStringView key = ln.CutBefore(eq);
                const FStringView value = ln.CutStartingAt(eq + 1);

                _entries.emplace_AssertUnique(key, value);
            }

            return true;
        }

        void Write(std::ofstream& oss) const {
            for (const TPair<const FString, FString>& it : _entries)
                oss << it.first.data() << '=' << it.second.data() << '\n';
        }

    private:
        FString _storeId;
        STRING_HASHMAP(Process, FString, ECase::Sensitive) _entries;
    };

    using FBarrier = FLinuxPlatformThread::FReadWriteLock;

    FPersistentData_() NOEXCEPT = default;

    bool Get(const FStringView& storeId, const FStringView& section, const FStringView& key, FString* pValue) {
        Assert(not storeId.empty());
        Assert(pValue);

        const Meta::FUniqueLock scopeLock(_barrier);

        FStore* pStore = nullptr;
        if (FetchStore_(storeId, &pStore) == false)
            return false;

        return pStore->Get(section, key, pValue);
    }

    bool Erase(const FStringView& storeId, const FStringView& section, const FStringView& key) {
        Assert(not storeId.empty());

        const Meta::FUniqueLock scopeLock(_barrier);

        FStore* pStore = nullptr;
        if (FetchStore_(storeId, &pStore) == false)
            return false;

        if (pStore->Erase(section, key)) {
            WriteStore_(*pStore); // synchronized FS copy
            return true;
        }
        else {
            return false;
        }
    }

    void Set(const FStringView& storeId, const FStringView& section, const FStringView& key, const FStringView& value) {
        Assert(not storeId.empty());

        const Meta::FUniqueLock scopeLock(_barrier);

        FStore* pStore = nullptr;
        if (FetchStore_(storeId, &pStore) == false) {
            pStore = &_stores.emplace_AssertUnique(ToString(storeId), storeId)->second;
        }

        pStore->Set(section, key, value);

        WriteStore_(*pStore); // synchronized FS copy
    }

private:
    mutable std::mutex _barrier;
    STRING_HASHMAP(Process, FStore, ECase::Sensitive) _stores;

    static void WriteStore_(const FStore& store) {
        std::ofstream oss(store.ExportPath().data());
        if (oss.good())
            store.Write(oss);
        else
            LOG(HAL, Error, L"failed to write persistent data store {0}", store.StoreId());
    }

    bool FetchStore_(const FStringView& storeId, FStore** pStore) {
        auto it = _stores.find_like(storeId, hash_string(storeId));

        if (_stores.end() == it) {
            FStore store(storeId);

            std::ifstream iss(store.ExportPath().data());
            if (iss.bad() || not store.Read(iss))
                return false;

            it = _stores.emplace_AssertUnique(storeId, std::move(store));
        }

        *pStore = &it->second;
        return true;
    }
};
//----------------------------------------------------------------------------
static FPersistentData_& PersistentData_() NOEXCEPT {
    static FPersistentData_ GInstance;
    return GInstance;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FLinuxPlatformMisc::CPUInfo() -> FCPUInfo {
    static const FCPUInfo GCPUInfo = FetchCPUInfo_();
    return GCPUInfo;
}
//----------------------------------------------------------------------------
FString FLinuxPlatformMisc::CPUBrand() {
    // http://msdn.microsoft.com/en-us/library/vstudio/hskdteyh(v=vs.100).aspx
    char brandString[0x40] = { 0 };

    union {
        struct {
            i32 eax, ebx, ecx, edx;
        }   regs;
        i32 data[4];
    }   CPUInfo;
    constexpr size_t CPUInfoSize = sizeof(CPUInfo);
    STATIC_ASSERT(sizeof(CPUInfo) == sizeof(i32) * 4);

    __cpuid(0x80000000, CPUInfo.regs.eax, CPUInfo.regs.ebx, CPUInfo.regs.ecx, CPUInfo.regs.edx);
    const u32 maxExtIDs = CPUInfo.regs.eax;

    if (maxExtIDs >= 0x80000004)
    {
        const u32 firstBrandString = 0x80000002;
        const u32 numBrandStrings = 3;
        for (u32 i = 0; i < numBrandStrings; i++) {
            __cpuid(firstBrandString + i, CPUInfo.regs.eax, CPUInfo.regs.ebx, CPUInfo.regs.ecx, CPUInfo.regs.edx);
            FPlatformMemory::Memcpy(brandString + CPUInfoSize * i, CPUInfo.data, CPUInfoSize);
        }
    }

    return FString(Strip(MakeCStringView(brandString)));
}
//----------------------------------------------------------------------------
FString FLinuxPlatformMisc::CPUVendor() {
    union {
        char Buffer[12 + 1];
        struct
        {
            int dw0;
            int dw1;
            int dw2;
        }   Dw;
    }   vendorResult;

    int args[4];
    __cpuid(0, args[0], args[1], args[2], args[3]);

    vendorResult.Dw.dw0 = args[1];
    vendorResult.Dw.dw1 = args[3];
    vendorResult.Dw.dw2 = args[2];
    vendorResult.Buffer[12] = 0;

    return FString(Strip(MakeCStringView(vendorResult.Buffer)));
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformMisc::NumCores() NOEXCEPT {
    static const size_t GNumCores = FetchNumCores_();
    return GNumCores;
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformMisc::NumCoresWithSMT() NOEXCEPT {
    static const size_t GNumCoresWHyperThreading = FetchNumCoresWithSMT_();
    return GNumCoresWHyperThreading;
}
//----------------------------------------------------------------------------
FString FLinuxPlatformMisc::OSName() {
    struct ::utsname un;
    Verify(0 == ::uname(&un));

    FStringView osname = MakeCStringView(un.sysname);
    FStringView release = MakeCStringView(un.release);
    FStringView version = MakeCStringView(un.version);

    const FStringView arch = (::strcmp(un.machine, "x86_64") == 0
        ? "64 bit"
        : "32 bit" );

    return StringFormat("{0} {1} - {2} - {3}", osname, arch, release, version );
}
//----------------------------------------------------------------------------
FString FLinuxPlatformMisc::MachineName() {
    struct ::utsname un;
    Verify(0 == ::uname(&un));

    return ToString(MakeCStringView(un.nodename));
}
//----------------------------------------------------------------------------
FString FLinuxPlatformMisc::UserName() {
    char buf[PATH_MAX + 1];
    Verify(0 == ::getlogin_r(buf, lengthof(buf)));
    return ToString(MakeCStringView(buf));
}
//----------------------------------------------------------------------------
void FLinuxPlatformMisc::SetGracefulTerminationHandler() {
    struct ::sigaction action;

    action.sa_sigaction = GracefulTerminationHandler_;
    ::sigfillset(&action.sa_mask);

    action.sa_flags = SA_SIGINFO | SA_RESTART | SA_ONSTACK;

    ::sigaction(SIGINT, &action, nullptr);
    ::sigaction(SIGTERM, &action, nullptr);
    ::sigaction(SIGHUP, &action, nullptr);	//  this should actually cause the server to just re-read configs (restart?)
}
//----------------------------------------------------------------------------
void FLinuxPlatformMisc::SetUTF8Output() {
    // Force locale to EN with UTF-8 encoding
    std::setlocale(LC_ALL, "en_US.UTF-8");
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
bool FLinuxPlatformMisc::Is64bitOperatingSystem() {
#ifdef ARCH_X64
    return true;
#else
    struct ::utsname un;
    return !(::uname(&un) || strcmp(un.machine, "x86_64"));
#endif
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMisc::IsRunningOnBatery() {
    static int GIsRunningOnBattery = -1;
    static FTimepoint GLastBatteryCheck{ 0 };

    // don't poll the OS for battery state on every tick. Just do it once every 10 seconds.
    if (GIsRunningOnBattery >= 0 && FTimepoint::ElapsedSince(GLastBatteryCheck) < FSeconds{ 10 })
        return (GIsRunningOnBattery > 0);

    GLastBatteryCheck = FTimepoint::Now();
    GIsRunningOnBattery = 0;

    // [RCL] 2015-09-30 FIXME: find a more robust way?
    const int kHardCodedNumBatteries = 10;
    for (int battery = 0; battery < kHardCodedNumBatteries; ++battery) {
        char fname[128];
        sprintf(fname, "/sys/class/power_supply/ADP%d/online", battery);

        int st = ::open(fname, O_RDONLY);
        if (st != -1) {
            // found ACAD device. check its state.
            char scratch[8];
            ssize_t readBytes = ::read(st, scratch, 1);
            ::close(st);

            if (readBytes > 0)
                GIsRunningOnBattery = (scratch[0] == '0') ? 1 : 0;

            break;	// quit checking after we found at least one
        }
    }

    // lack of ADP most likely means that we're not on laptop at all
    return (GIsRunningOnBattery > 0);
}
//----------------------------------------------------------------------------
void FLinuxPlatformMisc::PreventScreenSaver() {
    NOOP(); // #TODO
}
//----------------------------------------------------------------------------
void FLinuxPlatformMisc::ClipboardCopy(const char* src, size_t len) {
    Assert(src);
    UNUSED(src);
    UNUSED(len);
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void FLinuxPlatformMisc::ClipboardCopy(const wchar_t* src, size_t len) {
    Assert(src);
    UNUSED(src);
    UNUSED(len);
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMisc::ClipboardPaste(FString& dst) {
    UNUSED(dst);
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
#if !defined(GRND_NONBLOCK)
	#define GRND_NONBLOCK 0x0001
#endif
void FLinuxPlatformMisc::CreateGuid(FGuid& dst) {
    ::syscall(SYS_getrandom, &dst, sizeof(dst), GRND_NONBLOCK);

    // https://tools.ietf.org/html/rfc4122#section-4.4
    // https://en.wikipedia.org/wiki/Universally_unique_identifier
    //
    // The 4 bits of digit M indicate the UUID version, and the 1–3
    //   most significant bits of digit N indicate the UUID variant.
    // xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
    dst.Data.as_u32[1] = (dst.Data.as_u32[1] & 0xffff0fff) | 0x00004000; // version 4
    dst.Data.as_u32[2] = (dst.Data.as_u32[2] & 0x3fffffff) | 0x80000000; // variant 1
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMisc::EnvironmentVariable(const char* key, char* value, size_t capacity) {
    Assert(key);
    Assert(value);
    Assert(capacity > 0);

    if (const char* str = ::getenv(key)) {
        FMemoryViewWriter writer(value, capacity);
        FTextWriter oss(&writer);
        oss << MakeCStringView(str) << Eos;
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void FLinuxPlatformMisc::SetEnvironmentVariable(const char* key, const char* value) {
    Assert(key);
    Assert(value);

    VerifyRelease(0 == ::setenv(key, value, 1));
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMisc::PersistentVariable(const char* storeId, const char* section, const char* key, FString* value) {
    Assert(storeId);
    Assert(section);
    Assert(key);

    return PersistentData_().Get(
        MakeCStringView(storeId),
        MakeCStringView(section),
        MakeCStringView(key),
        value );
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMisc::SetPersistentVariable(const char* storeId, const char* section, const char* key, const char* value) {
    Assert(storeId);
    Assert(key);
    Assert(section);
    Assert(value);

    PersistentData_().Set(
        MakeCStringView(storeId),
        MakeCStringView(section),
        MakeCStringView(key),
        MakeCStringView(value) );

    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMisc::ErasePersistentVariable(const char* storeId, const char* section, const char* key) {
    Assert(storeId);
    Assert(section);
    Assert(key);

    return PersistentData_().Erase(
        MakeCStringView(storeId),
        MakeCStringView(section),
        MakeCStringView(key) );
}
//----------------------------------------------------------------------------
bool FLinuxPlatformMisc::ExternalTextEditor(const wchar_t* filename, size_t line/* = 0 */, size_t column/* = 0 */) {
    Assert(filename);

    CONSTEXPR const TPair<FWStringView, FWStringView> editors[] = {
        // visual studio code
        { L"code", L"-g \"{0}:{1}:{2}\"" },
        // sublime text 3
        { L"subl", L"\"{0}:{1}:{2}\"" },
        // gedit (gnome)
        { L"gedit", L"\"{0}\"" },
        // kate (kde)
        { L"kate", L"\"{0}\"" },
    };

    FWStringBuilder args;
    for (const auto& editor : editors) {
        args.Reset();
        Format(args, editor.second, MakeCStringView(filename), line, column);
        args << Eos;

        if (FLinuxPlatformProcess::ExecDetachedProcess(editor.first.data(), args.Written().data(), nullptr)) {
            LOG(HAL, Emphasis, L"opened external editor: {0} {1}", editor.first, args.Written());
            return true;
        }
        else {
            LOG(HAL, Error, L"failed to open external editor: {0} {1}",
                editor.first, args.Written() );
        }
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
