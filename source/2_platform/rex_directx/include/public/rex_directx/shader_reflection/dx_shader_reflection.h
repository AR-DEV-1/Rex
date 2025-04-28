#pragma once

#include "rex_engine/gfx/shader_reflection/shader_reflection.h"

namespace rex
{
  namespace gfx
  {
    class DxShaderReflection : public ShaderReflectionBase
    {
    public:

    protected:
      ShaderSignature reflect(const Shader* shader) override;
    };
  }
}