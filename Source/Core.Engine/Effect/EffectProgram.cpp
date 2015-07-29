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
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, EffectProgram, );
//----------------------------------------------------------------------------
EffectProgram::EffectProgram(Graphics::ShaderProfileType profile, Graphics::ShaderProgramType type)
:   ShaderProgram(profile, type) {}
//----------------------------------------------------------------------------
EffectProgram::~EffectProgram() {
    Assert(_constants.empty());
    Assert(_textures.empty());
}
//----------------------------------------------------------------------------
void EffectProgram::LinkReflectedData( 
    VECTOR(Effect, PSharedConstantBuffer)& sharedBuffers,
    SharedConstantBufferFactory *sharedBufferFactory,
    Graphics::IDeviceAPIShaderCompiler *compiler) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_constants.empty());
    Assert(_textures.empty());

    ASSOCIATIVE_VECTOR(Shader, Graphics::BindName, Graphics::PCConstantBufferLayout) layouts;
    ShaderProgram::Reflect(compiler, layouts, _textures);

    for (const Pair<Graphics::BindName, Graphics::PCConstantBufferLayout>& it : layouts) {
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
void EffectProgram::UnlinkReflectedData() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _textures.clear();
    _constants.clear();
}
//----------------------------------------------------------------------------
void EffectProgram::Set(Graphics::IDeviceAPIContext *context, const Effect *effect) const {
    const Graphics::ShaderProgramType stage = ShaderProgram::ProgramType();

    const Graphics::ConstantBuffer *stagePrograms[14];
    AssertRelease(_constants.size() <= lengthof(stagePrograms));

    const VECTOR(Effect, PSharedConstantBuffer)& sharedBuffers = effect->SharedBuffers();
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
