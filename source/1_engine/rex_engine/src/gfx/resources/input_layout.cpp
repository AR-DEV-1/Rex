#include "rex_engine/gfx/resources/input_layout.h"

#include "rex_engine/diagnostics/assert.h"

#include "rex_engine/engine/invalid_object.h"
#include "rex_engine/string/stringid.h"
#include "rex_std/bonus/utility.h"

namespace rex
{
	namespace gfx
	{
    rsl::string_view shader_semantic_name(ShaderSemantic semantic)
    {
      switch (semantic)
      {
      case rex::gfx::ShaderSemantic::Color:             return "COLOR";
      case rex::gfx::ShaderSemantic::Position:          return "POSITION";
      case rex::gfx::ShaderSemantic::PSize:             return "PSIZE";
      case rex::gfx::ShaderSemantic::BiNormal:          return "BINORMAL";
      case rex::gfx::ShaderSemantic::BlendIndices:      return "BLENDINDICES";
      case rex::gfx::ShaderSemantic::BlendWeight:       return "BLENDWEIGHT";
      case rex::gfx::ShaderSemantic::Normal:            return "NORMAL";
      case rex::gfx::ShaderSemantic::PositionT:         return "POSITIONT";
      case rex::gfx::ShaderSemantic::Tangent:           return "TANGENT";
      case rex::gfx::ShaderSemantic::TexCoord:          return "TEXCOORD";
      case rex::gfx::ShaderSemantic::Fog:               return "FOG";
      case rex::gfx::ShaderSemantic::TessFactor:        return "TESSFACTOR";
      case rex::gfx::ShaderSemantic::InstanceMatrix:    return "InstMatrix";
      case rex::gfx::ShaderSemantic::InstanceIndex:     return "InstTileOffset";
      }

      return "";
    }

    bool operator==(const InputLayoutDesc& lhs, const InputLayoutDesc& rhs)
    {
      if (lhs.size() != rhs.size())
      {
        return false;
      }

      return rsl::memcmp(lhs.data(), rhs.data(), lhs.size() * sizeof(lhs[0])) == 0;
    }
    bool operator!=(const InputLayoutDesc& lhs, const InputLayoutDesc& rhs)
    {
      return !(lhs == rhs);
    }

    InputLayout::InputLayout(s32 vertexSize, const InputLayoutDesc& desc)
      : m_vertex_size(vertexSize)
      , m_desc(desc)
    {}

    s32 InputLayout::vertex_size() const
    {
      return m_vertex_size;
    }

    // Validate a given descriptor and see if it can be used with this input layout
    bool InputLayout::validate_desc(const InputLayoutDesc& desc)
    {
      if (desc.size() != m_desc.size())
      {
        return false;
      }

      for (const auto& elem : desc)
      {
        auto it = rsl::find_if(m_desc.cbegin(), m_desc.cend(),
          [&](const InputLayoutElementDesc& myElem)
          {
            return elem.semantic == myElem.semantic;
          });

        if (it == m_desc.cend())
        {
          return false;
        }

        ShaderArithmeticType format = it->format;
        if (!format.is_convertible_to(elem.format))
        {
          return false;
        }
      }

      return true;
    }
	}
}