#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/gfx/shader_reflection/shader_resource_declaration.h"
#include "rex_engine/gfx/shader_reflection/shader_param_declaration.h"

namespace rex
{
	namespace gfx
	{
		class ViewTableBuilder
		{
		public:
			ViewTableBuilder(s32 registerSpace, ShaderParameterType expectedResourceType);

			ViewOffset add_resource(const ShaderResourceDeclaration& resource);

			ShaderParameterDeclaration build(s32 slot, ShaderVisibility visibility);

		private:
			void add_new_range();

		private:
			s32 m_start_register;
			s32 m_expected_register;
			s32 m_expected_register_space;
			s32 m_total_num_views;
			ShaderParameterType m_expected_resource_type;
			rsl::vector<ViewRangeDeclaration> m_ranges;
		};
	}
}