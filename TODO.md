# PPE TODO LIST

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

---

## Rendering

- [ ] Vulkan integration, bootstrap new Runtime.RHI module :)
    - Vulkan:
        - dynamic loader -> https://subscription.packtpub.com/book/game_development/9781786468154/1/ch01lvl1sec12/preparing-for-loading-vulkan-api-functions
        - frame graph RHI, instead of pure device abstraction -> https://github.com/azhirnov/FrameGraph.git
        - diagrams -> https://github.com/David-DiGioia/vulkan-diagrams

        - RHI:
            - add external with https://github.com/KhronosGroup/Vulkan-Headers
            - https://github.com/SaschaWillems/Vulkan/ for great samples
            - english/french tutorials: https://vulkan-tutorial.com/ (but using GFLW :/)
            - hello vulkan, pure vulkan sample: https://github.com/GPUOpen-LibrariesAndSDKs/HelloVulkan/
        - GPU Open:
            - https://gpuopen.com/learn/understanding-vulkan-objects/
            - https://gpuopen.com/learn/vulkan-device-memory/
            - Barriers explained: https://gpuopen.com/learn/vulkan-barriers-explained/
            - Memory management: https://gpuopen.com/learn/vulkan-device-memory/
            - Render passes: https://gpuopen.com/learn/vulkan-renderpasses/
            - Validation layers: https://gpuopen.com/learn/using-the-vulkan-validation-layers/
            - Cheat-Sheet: https://www.khronos.org/files/vulkan11-reference-guide.pdf
        - Best practices:
            - Adreno Vulkan developer guide: https://developer.qualcomm.com/qfile/34706/80-nb295-7_a-adreno_vulkan_developer_guide.pdf
            - Vulkan dos and don'ts by Nvidia: https://devblogs.nvidia.com/vulkan-dos-donts/
            - Optimizing AAA for desktop: https://gpuopen.com/presentations/2019/Vulkanised2019_06_optimising_aaa_vulkan_title_on_desktop.pdf
            - https://gpuopen.com/learn/concurrent-execution-asynchronous-queues/
            - https://gpuopen.com/learn/reducing-vulkan-api-call-overhead/
                - https://github.com/zeux/volk
            - https://gpuopen.com/learn/optimizing-gpu-occupancy-resource-usage-large-thread-groups/
            - https://www.fasterthan.life/blog/2017/7/12/i-am-graphics-and-so-can-you-part-3-breaking-ground
        - RenderGraph:
            - “Design Patterns for Low-Level Real-Time Rendering” https://www.youtube.com/watch?v=mdPeXJ0eiGc
            - “Work Stealing" https://www.youtube.com/watch?v=iLHNF7SgVN4
            - "Reducers and other Cilk++ HyperObjects" https://www.cse.wustl.edu/~angelee/cse539/papers/reducer.pdf
            - http://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/
            - <3<3<3 vulkan abstraction layer that represent frame as a task graph: https://github.com/azhirnov/FrameGraph

- [ ] Volumetric clouds
    - https://gist.github.com/pxv8270/e3904c49cbd8ff52cb53d95ceda3980e
    - https://github.com/AmanSachan1/Meteoros
    - https://www.guerrilla-games.com/read/nubis-authoring-real-time-volumetric-cloudscapes-with-the-decima-engine

- [ ] Path Tracing
    - https://blog.demofox.org/category/path-tracing/
    - https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/
    - https://blog.demofox.org/2020/06/06/casual-shadertoy-path-tracing-2-image-improvement-and-glossy-reflections/
    - https://blog.demofox.org/2020/06/14/casual-shadertoy-path-tracing-3-fresnel-rough-refraction-absorption-orbit-camera/
    - https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/

## Architecture

- [ ] Handle configuration/command line inside Core, seed from external configuration files, configuration cache
    - format ? (ini like UE? meh... json? no comment, meh...)
    - don't want a direct dependency to RTTI (but could be separated cleanly)
    - override from the commande line  } could generate a RTTI dynamic object
    - override from the console        } with all properties
- [ ] RTTI dynamic metaclass/metaobject ?
    - https://godbolt.org/z/aTvhaevoG
    - long time goal, some limitations with current code
        - may not be possible to remove a field ? since we use static offsets
- [ ] Fast dynamic dispatch by hacking VTable: https://medium.com/@calebleak/fast-virtual-functions-hacking-the-vtable-for-fun-and-profit-25c36409c5e0
- [ ] Wrap RTTI meta classes with Network module and OpenAPI/Swagger (https://editor.swagger.io/)
- [ ] Destiny's Multithreaded Rendering Architecture: https://www.youtube.com/watch?v=0nTDFLMLX9k
- [ ] Destiny's Shader System: http://advances.realtimerendering.com/destiny/gdc_2017/Destiny_shader_system_GDC_2017_v.4.0.pdf
- [ ] Physics engine:
    - https://www.youtube.com/watch?v=-_IspRG548E
    - Physics for game programmer: https://www.youtube.com/watch?v=7_nKOET6zwI
    - Liquid Crystal Demo: https://kotsoft.itch.io/liquid-crystal-pre-alpha
    - https://www.shadertoy.com/view/XdGGWD

## Algorithm

- [ ] HyperLogLog
    - https://blog.demofox.org/2015/03/09/hyperloglog-estimate-unique-value-counts-like-the-pros/
- [ ] BloomFilter
    - https://blog.demofox.org/2015/02/08/estimating-set-membership-with-a-bloom-filter/

## ECS

- [ ] Nice series of articles about ECS + generational indices:
    - https://bitsquid.blogspot.com/2014/08/building-data-oriented-entity-system.html
    - https://bitsquid.blogspot.com/2014/09/building-data-oriented-entity-system.html
    - https://bitsquid.blogspot.com/2014/10/building-data-oriented-entity-system.html
    - https://bitsquid.blogspot.com/2014/10/building-data-oriented-entity-system_10.html
- [ ] RustConf 2018: Using Rust for GameDev (with proposed ECS)
    - https://www.youtube.com/watch?v=aKLntZcp27M
- [ ] EnTT : ECS with modern C++ (used by Mojang)
    - https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system
    - https://github.com/skypjack/entt/wiki/Crash-Course:-cooperative-scheduler
    - https://github.com/skypjack/entt/wiki/Crash-Course:-runtime-reflection-system

> Note that TSparseArray<> is already a generational index array :)

## Memory

- [x] Add a large mipmap allocator inside FMallocBinned in addition to current medium mipmaps
    - current medium mips can't allocate more than 2mb, and virtual memory cache is weak (concurrency bottleneck)
        - large mips would have 2mb/bit <=> 64mb per chunk
        - could use large mips to allocate medium mips
        - this could avoid reserving/commiting too much memory and achieve better load balancing
    - NOPE, prefer using more virtual memory to keep the code simpler ==> *OR* implement a system that can allocate from several contiguous mips
        - might be done atomically at the end of the mip table
        - allocations would be aligned on mip granularity <=> 2mb
        - should cap allocation size to something reasonnable, like 64mb
        - could work better than virtual memory cache with a careful implementation

## Content-pipeline

- [ ] Integrate AMD's texture library for content pipeline (replacing clunky STB)
    - *OR* https://github.com/bkaradzic/bimg
- [ ] Integrate AMD's mesh library for content pipeline (replacing old custom code)
