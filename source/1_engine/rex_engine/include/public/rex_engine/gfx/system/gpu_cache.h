#pragma once

#include "rex_engine/engine/globals.h"

#include "rex_engine/gfx/resources/input_layout.h"
#include "rex_engine/gfx/resources/root_signature.h"
#include "rex_engine/gfx/system/shader_pipeline.h"

namespace rex
{
	namespace gfx
	{
		class GpuCache
		{
		public:
			// Load an input layout from the cache, or store a new one
			InputLayout* load_input_layout(const InputLayoutDesc& desc);
			// Load a root signature from the cache or store a new one
			RootSignature* load_root_signature(const ShaderPipeline& pipeline);

		private:
			// Input layout don't need to be created anymore in recent graphics APIs
			// However we often need to keep the graphics API specific data around
			// That's why we'll cache these
			rsl::unordered_map<ShaderPipeline, rsl::unique_ptr<RootSignature>> m_root_sig_cache;
			rsl::unordered_map<InputLayoutDesc, rsl::unique_ptr<InputLayout>> m_input_layout_cache;
		};

		namespace gpu_cache
		{
			void init(globals::GlobalUniquePtr<GpuCache> cache);
			GpuCache* instance();
			void shutdown();
		}
	}
}