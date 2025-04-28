#pragma once

#include "rex_directx/utility/dx_util.h"

#include "rex_engine/engine/globals.h"
#include "rex_engine/gfx/system/shader_type.h"

namespace rex
{
  namespace gfx
  {
    struct CompileShaderDesc
    {
      ShaderType shader_type = ShaderType::None;        // The type of shader to compile
      rsl::string_view shader_entry_point;              // The shader entry point. Multiple shaders can be in the same file, just with different entry points
      rsl::tiny_stack_string shader_feature_target;     // The version of the shader (eg. vs_5_1 for DX12)
      rsl::string_view shader_source_code;              // The source code of the shader
      rsl::small_stack_string shader_name;              // For debugging purposes, you can assign a sname to a shader
    };

    class DxShaderCompiler
    {
    public:
      // Compile a single shader
      wrl::ComPtr<ID3DBlob> compile_shader(const CompileShaderDesc& desc);
    };

    namespace shader_compiler
    {
      void init(globals::GlobalUniquePtr<DxShaderCompiler> shaderCompiler);
      DxShaderCompiler* instance();
      void shutdown();
    }
  } // namespace gfx
} // namespace rex