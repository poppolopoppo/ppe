#include "stdafx.h"

#include "EffectProgram.h"

#include "Effect.h"
#include "SharedConstantBuffer.h"
#include "SharedConstantBufferFactory.h"
#include "Material/IMaterialParameter.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Memory/UniqueView.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FEffectProgram, );
//----------------------------------------------------------------------------
FEffectProgram::FEffectProgram(Graphics::EShaderProfileType profile, Graphics::EShaderProgramType type)
:   FShaderProgram(profile, type) {}
//----------------------------------------------------------------------------
FEffectProgram::~FEffectProgram() {
    Assert(_constants.empty());
    Assert(_textures.empty());
}
//----------------------------------------------------------------------------
void FEffectProgram::LinkReflectedData( 
    VECTOR(FEffect, PSharedConstantBuffer)& sharedBuffers,
    FSharedConstantBufferFactory *sharedBufferFactory,
    Graphics::IDeviceAPIShaderCompiler *compiler) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_constants.empty());
    Assert(_textures.empty());

    ASSOCIATIVE_VECTOR(Shader, Graphics::FBindName, Graphics::PCConstantBufferLayout) layouts;
    FShaderProgram::Reflect(compiler, layouts, _textures);

    for (const TPair<Graphics::FBindName, Graphics::PCConstantBufferLayout>& it : layouts) {
        const size_t sharedBuffersCount = sharedBuffers.size();
        EffectSharedBufferIndex sharedBufferIndex = sharedBuffersCount;
        forrange(i, 0, sharedBuffersCount)
            if (sharedBuffers[i]->Mergeable(it.first, it.second.get())) {
                sharedBufferIndex = i;
                break;
            }

        if (sharedBufferIndex == sharedBuffersCount)
            sharedBuffers.push_back(sharedBufferFactory->GetOrCreate(it.first, it.second.get()) );

        Assert(sharedBufferIndex < sharedBuffers.size());
        _constants.Vector().emplace_back(it.first, sharedBufferIndex);
    }
}
//----------------------------------------------------------------------------
void FEffectProgram::UnlinkReflectedData() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _textures.clear();
    _constants.clear();
}
//----------------------------------------------------------------------------
void FEffectProgram::Set(Graphics::IDeviceAPIContext *context, const FEffect *effect) const {
    const Graphics::EShaderProgramType stage = FShaderProgram::ProgramType();

    const Graphics::FConstantBuffer *stagePrograms[14];
    AssertRelease(_constants.size() <= lengthof(stagePrograms));

    const VECTOR(FEffect, PSharedConstantBuffer)& sharedBuffers = effect->SharedBuffers();
    forrange(i, 0, _constants.size()) {
        const EffectSharedBufferIndex sharedBufferIndex = _constants.Vector()[i].second;
        const PSharedConstantBuffer& sharedBuffer = sharedBuffers[sharedBufferIndex];
        stagePrograms[i] = sharedBuffer.get();
    }

    context->SetConstantBuffers(stage, MakeConstView(&stagePrograms[0], &stagePrograms[_constants.size()]));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
