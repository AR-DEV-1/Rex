#pragma once

#include "rex_engine/engine/defines.h"
#include "rex_engine/gfx/system/shader_type.h"
#include "rex_engine/engine/invalid_object.h"

namespace rex
{
  namespace gfx
  {
    REX_STATIC_WARNING("Combine shader type with shader visibility");

    // shader visibility is used to indicate which shader type has access to the specified resource
    enum class ShaderVisibility
    {
      None = 0,
      Vertex = BIT(0),        // only the vertex shader has access to this resource
      Pixel = BIT(1),         // only the pixel shader has access to this resource
      Geometry = BIT(2),      // only the geometry shader has access to this resource
      Compute = BIT(3),       // only the compute shader has access to this resource
      Hull = BIT(4),          // only the hull shader has access to this resource
      Domain = BIT(5),        // only the domain shader has access to this resource
      Amplification = BIT(6), // only the amplification shader has access to this resource
      Mesh = BIT(7),          // only the mesh shader has access to this resource

      All = Vertex | Pixel | Geometry | Compute | Hull | Domain | Amplification | Mesh // all shaders have access to this resource
    };
    using ShaderType = ShaderVisibility;

    // Convert a shader type enum to the shader visibility flag it belongs to
    //constexpr ShaderVisibility shader_type_to_visibility(ShaderType type)
    //{
    //  switch (type)
    //  {
    //  case rex::gfx::ShaderType::Vertex:            return ShaderVisibility::Vertex;
    //  case rex::gfx::ShaderType::Pixel:             return ShaderVisibility::Pixel;
    //  case rex::gfx::ShaderType::Geometry:          return ShaderVisibility::Geometry;
    //  case rex::gfx::ShaderType::Hull:              return ShaderVisibility::Hull;
    //  case rex::gfx::ShaderType::Domain:            return ShaderVisibility::Domain;
    //  case rex::gfx::ShaderType::Amplification:     return ShaderVisibility::Amplification;
    //  case rex::gfx::ShaderType::Mesh:              return ShaderVisibility::Mesh;
    //  default: break;
    //  }

    //  return invalid_obj<ShaderVisibility>();
    //}
  }
}