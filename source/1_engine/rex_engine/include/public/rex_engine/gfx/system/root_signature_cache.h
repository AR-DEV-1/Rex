#pragma once

#include "rex_engine/gfx/resources/root_signature.h"
#include "rex_engine/gfx/system/shader_pipeline.h"

#include "rex_engine/engine/globals.h"

namespace rex
{
	namespace gfx
	{
		class RootSignatureCache
		{
		public:
			// Load a root signature from the cache, or store a new one
			RootSignature* load(const ShaderPipeline& pipeline);
			// Clear the cache
			void clear();

		private:
			rsl::unordered_map<ShaderPipeline, rsl::unique_ptr<RootSignature>> m_root_sig_cache;
		};

		namespace root_signature_cache
		{
			void init(globals::GlobalUniquePtr<RootSignatureCache> rootSignatureCache);
			RootSignatureCache* instance();
			void shutdown();
		}
	}
}