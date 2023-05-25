// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "WindowTestApp.h"
#include "Test_Includes.h"

#include "ApplicationModule.h"
#include "RHIModule.h"
#include "Window/WindowService.h"

#include "Diagnostic/Logger.h"
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
    /* compiler */ \
    _Macro(Compiler_Annotation1_) \
    _Macro(Compiler_Annotation2_) \
    _Macro(Compiler_Annotation3_) \
    _Macro(Compiler_Annotation4_) \
    _Macro(Compiler_ComputeLocalSize1_) \
    _Macro(Compiler_MeshShader1_) \
    _Macro(Compiler_MRT1_) \
    _Macro(Compiler_Optimization1_) \
    _Macro(Compiler_PushConst1_) \
    _Macro(Compiler_PushConst2_) \
    _Macro(Compiler_PushConst3_) \
    _Macro(Compiler_RayTracing1_) \
    _Macro(Compiler_Reflection1_) \
    _Macro(Compiler_Reflection2_) \
    _Macro(Compiler_Reflection3_) \
    _Macro(Compiler_Reflection4_) \
    _Macro(Compiler_Reflection5_) \
    _Macro(Compiler_ShaderTrace1_) \
    _Macro(Compiler_UniformArray1_) \
    _Macro(Compiler_UniformArray2_) \
    /* compute */ \
    _Macro(Compute_Compute1_) \
    _Macro(Compute_Compute2_) \
    _Macro(Compute_ArrayOfTextures1_) \
    _Macro(Compute_ArrayOfTextures2_) \
    /* #TODO: _Macro(Compute_InvalidID_)*/ \
    _Macro(Compute_DynamicOffset1_) \
    _Macro(Compute_PushConstant1_) \
    _Macro(Compute_AsyncCompute1_) \
    _Macro(Compute_AsyncCompute2_) \
    /* queue */ \
    _Macro(Queue_CopyBuffer1_) \
    _Macro(Queue_CopyImage1_) \
    _Macro(Queue_CopyImage2_) \
    _Macro(Queue_CopyImage3_) \
    /* drawing */ \
    _Macro(Drawing_Draw1_) \
    _Macro(Drawing_Draw2_) \
    _Macro(Drawing_Draw3_) \
    _Macro(Drawing_Draw4_) \
    _Macro(Drawing_Draw5_) \
    _Macro(Drawing_Draw6_) \
    _Macro(Drawing_Draw7_) \
    _Macro(Debugger_ReadAttachment1_) \
    /* debugger */ \
    _Macro(Debugger_ShaderDebugger1_) \
    _Macro(Debugger_ShaderDebugger2_) \
    _Macro(Debugger_RayTracingDebugger1_) \
    /* vendor-specific */ \
    _Macro(Drawing_DrawMeshes1_) \
    _Macro(Drawing_ShadingRate1_) \
    _Macro(Drawing_TraceRays1_) \
    _Macro(Drawing_TraceRays2_) \
    _Macro(Drawing_TraceRays3_) \
    /* Impl */ \
    _Macro(Impl_Scene1_) \
    _Macro(Impl_Multithreading1_) \
    _Macro(Impl_Multithreading2_) \
    _Macro(Impl_Multithreading3_) \
    _Macro(Impl_Multithreading4_) \

//----------------------------------------------------------------------------
#define WINDOWTEST_EXTERN_DECL(_Func) extern bool _Func(FWindowTestApp&);
EACH_WINDOWTEST(WINDOWTEST_EXTERN_DECL)
#undef WINDOWTEST_EXTERN_DECL
//----------------------------------------------------------------------------
static void LaunchWindowTests_(Application::FApplicationBase& baseApp, FTimespan) {
    auto& app = *checked_cast<FWindowTestApp*>(&baseApp);
    auto launchTest = [&app](FWStringView name, auto&& test) {
        Unused(name);

        RHI::IFrameGraph& fg = *app.RHI().FrameGraph();
        fg.PrepareNewFrame();

        const FTimedScope timer{};
        const bool success = test(app);

        const FMilliseconds elapsed = timer.Elapsed();
        Unused(elapsed);

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

        const size_t numLoops = (app.RHI().Features() & ERHIFeature::Debugging ? 1 : 100);

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

            // app.RHI().ReleaseMemory();
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
:   parent_type(domain, "Tools/WindowTest", true) {

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

    auto& appmodule = FApplicationModule::Get(Domain());
    appmodule.OnApplicationTick().FireAndForget(&LaunchWindowTests_);

    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FWindowTestApp::Shutdown() {
    Window().NotifySystrayWarning(L"Here I go", L"WindowTest");

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
void FWindowTestApp::Render(FTimespan dt) {
    parent_type::Render(dt);

    // call present() once before exit for captures
    using namespace RHI;
    IFrameGraph& fg = *RHI().FrameGraph();

    const TAutoResource<FImageID> colorTarget{ fg, fg.CreateImage(FImageDesc{}
        .SetDimension({ 32, 32 })
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::ColorAttachment | EImageUsage::Transfer),
        Default ARGS_IF_RHIDEBUG("ColorTarget")) };
    LOG_CHECKVOID(WindowTest, !!colorTarget);

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("WindowTest")
        .SetDebugFlags(EDebugFlags::Default)) };

    const FLinearColor clearColor = HSV_to_RGB_smooth({
        Saturate(float(std::cos(*FSeconds(Timeline().Total())) * .5 + .5)), 1.f, 1.f });
    const PFrameTask tClear = cmd->Task(FClearColorImage()
        .SetImage(colorTarget)
        .AddRange(0_mipmap, 1, 0_layer, 1)
        .Clear(clearColor));
    LOG_CHECKVOID(WindowTest, !!tClear);

    const PFrameTask tPresent = cmd->Task(FPresent{ RHI().Swapchain().Get(), colorTarget }
        .DependsOn(tClear));
    LOG_CHECKVOID(WindowTest, !!tPresent);

    LOG_CHECKVOID(WindowTest, fg.Execute(cmd));

    RequestExit(); // only render one loop
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
