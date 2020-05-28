# PPE TODO LIST

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

---

## Rendering

- [ ] Vulkan integration, bootstrap new Runtime.RHI module :)
    - RHI:
        - add external with https://github.com/KhronosGroup/Vulkan-Headers
        - https://github.com/SaschaWillems/Vulkan/ for great samples
        - french tutorials https://vulkan-tutorial.com/
    - RenderGraph:
        - “Design Patterns for Low-Level Real-Time Rendering” https://www.youtube.com/watch?v=mdPeXJ0eiGc
        - “Work Stealing" https://www.youtube.com/watch?v=iLHNF7SgVN4
        - "Reducers and other Cilk++ HyperObjects" https://www.cse.wustl.edu/~angelee/cse539/papers/reducer.pdf
        - http://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

## Architecture

- [ ] Handle configuration/command line inside Core, seed from external configuration files, configuration cache
    - format ? (ini like UE? meh... json? no comment, meh...)
    - don't want a direct dependency to RTTI (but could be separated cleanly)
    - override from the commande line  } could generate a RTTI dynamic object
    - override from the console        } with all properties
- [ ] RTTI dynamic metaclass/metaobject ?
    - long time goal, some limitations with current code
        - may not be possible to remove a field ? since we use static offsets
- [ ] Wrap RTTI meta classes with Network module and OpenAPI/Swagger (https://editor.swagger.io/)

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
