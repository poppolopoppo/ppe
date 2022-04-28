#include "stdafx.h"

#include "WindowTestApp.h"
#include "Test_Includes.h"

#include "RHIModule.h"
#include "Window/WindowService.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "Maths/Threefy.h"

namespace PPE {
LOG_CATEGORY(, WindowTest)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FUnitTestFunc_ {
    FWStringView Name;
    bool (*Callback)(FWindowTestApp&) = nullptr;
};
//----------------------------------------------------------------------------
#define EACH_WINDOWTEST(_Macro) \
    /* compute */ \
    _Macro(Test_Compute1_) \
    _Macro(Test_Compute2_) \
    _Macro(Test_ArrayOfTextures1_) \
    _Macro(Test_ArrayOfTextures2_) \
    /* #TODO: _Macro(Test_InvalidID_)*/ \
    _Macro(Test_DynamicOffset1_) \
    _Macro(Test_PushConstant1_) \
    _Macro(Test_AsyncCompute1_) \
    _Macro(Test_AsyncCompute2_) \
    /* queue */ \
    _Macro(Test_CopyBuffer1_) \
    _Macro(Test_CopyImage1_) \
    _Macro(Test_CopyImage2_) \
    _Macro(Test_CopyImage3_) \
    /* drawing */ \
    _Macro(Test_Draw1_) \
    _Macro(Test_Draw2_) \
    _Macro(Test_Draw3_) \
    _Macro(Test_Draw4_) \
    _Macro(Test_Draw5_) \
    _Macro(Test_Draw6_) \
    _Macro(Test_Draw7_) \
    _Macro(Test_ReadAttachment1_) \
    /* debugger */ \
    _Macro(Test_ShaderDebugger1_) \
    _Macro(Test_ShaderDebugger2_) \
    _Macro(Test_RayTracingDebugger1_) \
    /* Impl */ \
    _Macro(Test_Scene1_) \
    _Macro(Test_Multithreading1_) \
    /* vendor-specific */ \
    _Macro(Test_DrawMeshes1_) \
    _Macro(Test_ShadingRate1_) \
    _Macro(Test_TraceRays1_) \
    _Macro(Test_TraceRays2_) \
    _Macro(Test_TraceRays3_) \

//----------------------------------------------------------------------------
#define WINDOWTEST_EXTERN_DECL(_Func) extern bool _Func(FWindowTestApp&);
EACH_WINDOWTEST(WINDOWTEST_EXTERN_DECL)
#undef WINDOWTEST_EXTERN_DECL
//----------------------------------------------------------------------------
static void LaunchWindowTests_(FWindowTestApp& app) {
    auto launchTest = [&app](FWStringView name, auto&& test) {
        Unused(name);
        // LOG(WindowTest, Debug, L"start framegraph test <{0}> ...", name);

        RHI::IFrameGraph& fg = *app.RHI().FrameGraph();
        fg.PrepareNewFrame();

        FTimedScope chrono;
        const bool success = test(app);
        const FMilliseconds elapsed = chrono.Elapsed();

        if (Likely(success)) {
#if USE_PPE_DEBUG && !USE_PPE_FASTDEBUG
            ONLY_IF_RHIDEBUG(fg.LogFrame());
#endif
            LOG(WindowTest, Emphasis, L" -- OK : {0:10f3} -- {1}", Fmt::DurationInMs(elapsed), name);
        } else {
            LOG(WindowTest, Error, L" !! KO : {0:10f3} -- {1}", Fmt::DurationInMs(elapsed), name);
        }

        return success;
    };

    FUnitTestFunc_ unitTests[] = {
#define LAUNCH_TEST_(_Func) { WSTRINGIZE(_Func), &_Func },
EACH_WINDOWTEST(LAUNCH_TEST_)
#undef LAUNCH_TEST_
    };

    static volatile bool enabled = true;
    if (enabled) {
        app.Window().NotifySystrayWarning(
            StringFormat(L"Launch {0} unit tests...", lengthof(unitTests)),
            L"WindowTest");

        app.Window().BeginTaskbarProgress();

        const size_t numLoops = (app.RHI().Features() & ERHIFeature::Debugging ? 10 : 200);

        size_t testIndex = 0;
        size_t testSucceed = 0;
        size_t testInvocation = 0;

        const size_t testCount = lengthof(unitTests);

        FThreefy_4x32 rng{};
        rng.RandomSeed();

        forrange(loop, 0, numLoops) {
            testIndex = 0;

            LOG(WindowTest, Info, L"-==================- [LOOP:{0:#4}] -==================-", loop);

            for (const auto& test : unitTests) {
                ++testIndex;
                ++testInvocation;

                if (launchTest(test.Name, test.Callback)) {
                    ++testSucceed;
                }
                else {
                    app.Window().NotifySystrayError(
                        StringFormat(L"frame graph test <{0}> failed!", test.Name),
                        L"failed unit test");
                }

                app.Window().SetTaskbarProgress(testIndex, testCount);

                Unused(app.PumpMessages());
            }

            FLUSH_LOG();
            rng.Shuffle(MakeView(unitTests));
        }

        ReportAllTrackingData();

        app.Window().EndTaskbarProgress();

        if (testSucceed == testCount) {
            app.Window().NotifySystrayInfo(
                StringFormat(L"all {0} tests passed", testInvocation),
                L"finished unit testing");
        }
        else {
            app.Window().NotifySystrayWarning(
                StringFormat(L"{0} / {1} tests passed", testSucceed, testInvocation),
                L"finished unit testing");
        }
    }

    app.ReleaseMemory();
}
//----------------------------------------------------------------------------
#undef EACH_WINDOWTEST
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWindowTestApp::FWindowTestApp(FModularDomain& domain)
:   parent_type(domain, "WindowTest", true) {

#if 0 //%_NOCOMMIT%
    FLogger::SetGlobalVerbosity(ELoggerVerbosity::None);
#endif

    FRHIModule& rhiModule = FRHIModule::Get(domain);
    rhiModule.SetStagingBufferSize(8_MiB);
}
//----------------------------------------------------------------------------
FWindowTestApp::~FWindowTestApp() = default;
//----------------------------------------------------------------------------
void FWindowTestApp::Start() {
    parent_type::Start();

    LaunchWindowTests_(*this);

    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FWindowTestApp::Shutdown() {
    Window().NotifySystrayWarning(L"Here I go", L"WindowTest");

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
