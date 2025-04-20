#include "rex_directx/shader_reflection/dx_shader_reflection.h"

#include <d3dcompiler.h>
#include <d3d12shader.h>

#include "rex_directx/diagnostics/dx_call.h"
#include "rex_directx/system/dx_shader.h"

#include "rex_directx/utility/dx_util.h"
#include "rex_std/string_view.h"
#include "rex_std/bonus/string.h"
#include "rex_engine/diagnostics/assert.h"

#include "rex_engine/gfx/shader_reflection/shader_signature.h"
#include "rex_engine/gfx/shader_reflection/shader_io_reflection.h"
namespace rex
{
  namespace gfx
  {
    using ID3DShaderReflection = ID3D12ShaderReflection;
    using ID3DShaderReflectionVariable = ID3D12ShaderReflectionVariable;
    using ID3DShaderReflectionConstantBuffer = ID3D12ShaderReflectionConstantBuffer;
    using ID3DShaderReflectionType = ID3D12ShaderReflectionType;
    using D3D_SHADER_DESC = D3D12_SHADER_DESC;
    using D3D_SHADER_VARIABLE_DESC = D3D12_SHADER_VARIABLE_DESC;
    using D3D_SHADER_TYPE_DESC = D3D12_SHADER_TYPE_DESC;
    using D3D_SHADER_BUFFER_DESC = D3D12_SHADER_BUFFER_DESC;
    using D3D_SHADER_INPUT_BIND_DESC = D3D12_SHADER_INPUT_BIND_DESC;
    using D3D_SIGNATURE_PARAMETER_DESC = D3D12_SIGNATURE_PARAMETER_DESC;

    // Based on the component type, mask and precision, create a shader parameter type
    ShaderArithmeticType shader_arithmetic_type(D3D_REGISTER_COMPONENT_TYPE componentType, s32 componentMask, D3D_MIN_PRECISION precision);
    // Based on the component mask, create the correct shader parameter type for floats
    ShaderArithmeticType component_mask_to_float(s32 componentMask);
    // Based on the component mask and its precision, create the correct shader parameter type for uints
    ShaderArithmeticType component_mask_to_uint(s32 componentMask, D3D_MIN_PRECISION precision);
    // Based on the component mask and its precision, create the correct shader parameter type for uints
    ShaderArithmeticType component_mask_to_sint(s32 componentMask, D3D_MIN_PRECISION precision);

    ShaderIODeclaration reflect_shader_input_parameter(ID3D12ShaderReflection* refl, s32 idx)
    {
      D3D12_SIGNATURE_PARAMETER_DESC param_desc;
      DX_CALL(refl->GetInputParameterDesc(idx, &param_desc));
      ShaderIODeclaration input_param{};

      input_param.semantic_name = param_desc.SemanticName;
      input_param.semantic_index = param_desc.SemanticIndex;
      input_param.type = shader_arithmetic_type(param_desc.ComponentType, param_desc.Mask, param_desc.MinPrecision);
      input_param.size = format_byte_size(input_param.type);

      return input_param;
    }
    ShaderIODeclaration reflect_shader_output_parameter(ID3D12ShaderReflection* refl, s32 idx)
    {
      D3D12_SIGNATURE_PARAMETER_DESC param_desc;
      DX_CALL(refl->GetOutputParameterDesc(idx, &param_desc));
      ShaderIODeclaration output_param{};

      output_param.semantic_name = param_desc.SemanticName;
      output_param.semantic_index = param_desc.SemanticIndex;
      output_param.type = shader_arithmetic_type(param_desc.ComponentType, param_desc.Mask, param_desc.MinPrecision);
      output_param.size = format_byte_size(output_param.type);

      return output_param;
    }

    ShaderResourceDeclaration reflect_bound_resource(ID3D12ShaderReflection* refl, s32 idx, ShaderType type)
    {
      D3D12_SHADER_INPUT_BIND_DESC resource_desc;
      DX_CALL(refl->GetResourceBindingDesc(idx, &resource_desc));

      ShaderResourceDeclaration bound_resource{};

      switch (resource_desc.Type)
      {
      case D3D_SIT_CBUFFER:
        bound_resource.name = resource_desc.Name;
        bound_resource.shader_register = resource_desc.BindPoint;
        bound_resource.register_space = resource_desc.Space;
        bound_resource.shader_type = type;
        bound_resource.resource_type = ShaderParameterType::ConstantBuffer;
        break;
      case D3D_SIT_BYTEADDRESS:
        bound_resource.name = resource_desc.Name;
        bound_resource.shader_register = resource_desc.BindPoint;
        bound_resource.register_space = resource_desc.Space;
        bound_resource.shader_type = type;
        bound_resource.resource_type = ShaderParameterType::ByteAddress;
        break;
      case D3D_SIT_TEXTURE:
      {
        bound_resource.name = resource_desc.Name;
        bound_resource.shader_register = resource_desc.BindPoint;
        bound_resource.register_space = resource_desc.Space;
        bound_resource.shader_type = type;
        bound_resource.resource_type = ShaderParameterType::Texture;
        break;
      }
      case D3D_SIT_SAMPLER:
      {
        bound_resource.name = resource_desc.Name;
        bound_resource.shader_register = resource_desc.BindPoint;
        bound_resource.register_space = resource_desc.Space;
        bound_resource.shader_type = type;
        bound_resource.resource_type = ShaderParameterType::Sampler;
        break;
      }
      default: REX_ASSERT("Invalid bound resource type");
      }

      return bound_resource;
    }

    rsl::tiny_stack_string convert_shader_version_to_string(UINT version)
    {
      D3D12_SHADER_VERSION_TYPE shader_type = static_cast<D3D12_SHADER_VERSION_TYPE>((version & 0xFFFF0000) >> 16);
      s32 major = (version & 0x000000F0) >> 4;
      s32 minor = (version & 0x0000000F);

      rsl::tiny_stack_string version_str;
      switch (shader_type)
      {
      case D3D12_SHVER_PIXEL_SHADER:     version_str = "ps_"; break;
      case D3D12_SHVER_VERTEX_SHADER:    version_str = "vs_"; break;
      case D3D12_SHVER_GEOMETRY_SHADER:  version_str = "gs_"; break;
      case D3D12_SHVER_HULL_SHADER:      version_str = "hs_"; break;
      case D3D12_SHVER_DOMAIN_SHADER:    version_str = "ds_"; break;
      case D3D12_SHVER_COMPUTE_SHADER:   version_str = "cs_"; break;
      default: REX_ASSERT("Invalid shader type"); break;
      }

      version_str += rsl::format("{}_{}", major, minor);

      return version_str;
    }

    struct ShaderRegisterInfo
    {
      s32 index;
      s32 space;
    };
    ShaderRegisterInfo get_constant_buffer_register_info(ID3D12ShaderReflection* refl, ID3D12ShaderReflectionConstantBuffer* cb_refl)
    {
      D3D12_SHADER_BUFFER_DESC cb_desc;
      DX_CALL(cb_refl->GetDesc(&cb_desc));

      D3D12_SHADER_INPUT_BIND_DESC input_desc;
      DX_CALL(refl->GetResourceBindingDescByName(cb_desc.Name, &input_desc));

      ShaderRegisterInfo reg_info{};
      reg_info.index = input_desc.BindPoint;
      reg_info.space = input_desc.Space;

      return reg_info;
    }

    ConstantBufferDeclaration reflect_constant_buffer(ID3D12ShaderReflection* refl, ID3D12ShaderReflectionConstantBuffer* cbReflection)
    {
      D3D12_SHADER_BUFFER_DESC shader_buffer_desc{};
      cbReflection->GetDesc(&shader_buffer_desc);

      ShaderRegisterInfo reg_info = get_constant_buffer_register_info(refl, cbReflection);

      ConstantBufferDeclaration cb_ref{};
      cb_ref.name = shader_buffer_desc.Name;
      cb_ref.shader_register = reg_info.index;
      cb_ref.register_space = reg_info.space;
      cb_ref.size = shader_buffer_desc.Size;
      s32 num_vars = shader_buffer_desc.Variables;
      for (s32 var_idx = 0; var_idx < num_vars; ++var_idx)
      {
        ID3D12ShaderReflectionVariable* var = cbReflection->GetVariableByIndex(var_idx);
        D3D12_SHADER_VARIABLE_DESC cb_var_desc;
        var->GetDesc(&cb_var_desc);

        CBufferVarReflDecl& desc = cb_ref.variables.emplace_back();
        desc.name = cb_var_desc.Name;
        desc.offset = cb_var_desc.StartOffset;
        desc.size = cb_var_desc.Size;
        ID3D12ShaderReflectionType* type = var->GetType();
        D3D12_SHADER_TYPE_DESC type_desc;
        DX_CALL(type->GetDesc(&type_desc));
        desc.class_type = d3d::from_dx12(type_desc.Class);
        desc.var_type = d3d::from_dx12(type_desc.Type);
      }

      return cb_ref;
    }
    rsl::vector<ConstantBufferDeclaration> reflect_constant_buffers(ID3D12ShaderReflection* refl, s32 numConstantBuffers)
    {
      rsl::vector<ConstantBufferDeclaration> constant_buffers;
      constant_buffers.reserve(numConstantBuffers);

      for (card32 i = 0; i < numConstantBuffers; ++i)
      {
        ID3D12ShaderReflectionConstantBuffer* cb = refl->GetConstantBufferByIndex(i);
        constant_buffers.push_back(reflect_constant_buffer(refl, cb));
      }

      return constant_buffers;
    }
    rsl::vector<ShaderIODeclaration> reflect_input_params(ID3D12ShaderReflection* refl, s32 numInputParams)
    {
      rsl::vector<ShaderIODeclaration> input_params;
      input_params.reserve(numInputParams);

      for (card32 i = 0; i < numInputParams; ++i)
      {
        input_params.emplace_back(reflect_shader_input_parameter(refl, i));
      }

      return input_params;
    }
    rsl::vector<ShaderIODeclaration> reflect_output_params(ID3D12ShaderReflection* refl, s32 numOutputParams)
    {
      rsl::vector<ShaderIODeclaration> output_params;
      output_params.reserve(numOutputParams);

      for (card32 i = 0; i < numOutputParams; ++i)
      {
        output_params.emplace_back(reflect_shader_output_parameter(refl, i));
      }

      return output_params;
    }

    ShaderResourceDeclarations reflect_bound_resources(ID3D12ShaderReflection* refl, s32 numBoundResources, ShaderType type)
    {
      ShaderResourceDeclarations bound_resources{};

      for (card32 i = 0; i < numBoundResources; ++i)
      {
        auto bound_resource = reflect_bound_resource(refl, i, type);
        switch (bound_resource.resource_type)
        {
        case ShaderParameterType::Texture:
          bound_resources.textures.push_back(bound_resource);
          break;
        case ShaderParameterType::Sampler:
          bound_resources.samplers.push_back(bound_resource);
          break;
        case ShaderParameterType::ConstantBuffer:
          bound_resources.constant_buffers.push_back(bound_resource);
          break;
        case ShaderParameterType::ByteAddress:
          bound_resources.byte_address_buffers.push_back(bound_resource);
          break;
        default:
          REX_ASSERT("Unknown shader parameter type");
          break;
        }        
      }

      return bound_resources;
    }

    ShaderArithmeticType shader_arithmetic_type(D3D_REGISTER_COMPONENT_TYPE componentType, s32 componentMask, D3D_MIN_PRECISION precision)
    {
      switch (componentType)
      {
      default: REX_ASSERT("Invalid component type"); break;
      case D3D_REGISTER_COMPONENT_SINT32:  return component_mask_to_sint(componentType, precision); break;
      case D3D_REGISTER_COMPONENT_UINT32:  return component_mask_to_uint(componentType, precision); break;
      case D3D_REGISTER_COMPONENT_FLOAT32: return component_mask_to_float(componentMask); break;
      }

      return invalid_obj<ShaderArithmeticType>();
    }
    ShaderArithmeticType component_mask_to_float(s32 componentMask)
    {
      if (componentMask & 8)
        return ShaderArithmeticType::Float4;
      if (componentMask & 4)
        return ShaderArithmeticType::Float3;
      if (componentMask & 2)
        return ShaderArithmeticType::Float2;
      if (componentMask & 1)
        return ShaderArithmeticType::Float;

      REX_ASSERT("Invalid component mask");
      return ShaderArithmeticType::Unknown;
    }
    ShaderArithmeticType component_mask_to_uint(s32 componentMask, D3D_MIN_PRECISION precision)
    {
      if (componentMask & 3)
      {
        if (precision == D3D_MIN_PRECISION_UINT_16)
        {
          return ShaderArithmeticType::Ushort2;
        }
        else if (precision == D3D_MIN_PRECISION_DEFAULT)
        {
          return ShaderArithmeticType::Uint2;
        }
      }
      if (componentMask & 1)
      {
        return ShaderArithmeticType::Uint;
      }

      REX_ASSERT("Invalid component mask");
      return ShaderArithmeticType::Unknown;
    }
    ShaderArithmeticType component_mask_to_sint(s32 componentMask, D3D_MIN_PRECISION precision)
    {
      if (componentMask & 3)
      {
        if (precision == D3D_MIN_PRECISION_SINT_16)
        {
          return ShaderArithmeticType::Short2;
        }
        else if (precision == D3D_MIN_PRECISION_DEFAULT)
        {
          return ShaderArithmeticType::Int2;
        }
      }
      if (componentMask & 1)
      {
        return ShaderArithmeticType::Int;
      }

      REX_ASSERT("Invalid component mask");
      return ShaderArithmeticType::Unknown;
    }

    ShaderSignature DxShaderReflection::reflect(const Shader* shader)
    {
      REX_ASSERT_X(shader, "Cannot create reflection data on a null shader");

      // Create the shader reflection object
      const gfx::DxShader* dx_shader = d3d::to_dx12(shader);
      const void* byte_code = dx_shader->dx_bytecode().pShaderBytecode;
      s32 byte_count = static_cast<s32>(dx_shader->dx_bytecode().BytecodeLength);
      wrl::ComPtr<ID3D12ShaderReflection> reflection_object;
      DX_CALL(D3DReflect(byte_code, byte_count, IID_PPV_ARGS(reflection_object.GetAddressOf())));

      // Get the description of the shader
      D3D12_SHADER_DESC shader_desc;
      DX_CALL(reflection_object->GetDesc(&shader_desc));

      s32 num_constant_buffers = shader_desc.ConstantBuffers;
      s32 num_input_params = shader_desc.InputParameters;
      s32 num_output_params = shader_desc.OutputParameters;
      s32 num_bound_resources = shader_desc.BoundResources;

      ShaderSignatureDesc desc{};

      desc.shader_version = convert_shader_version_to_string(shader_desc.Version);
      desc.constant_buffers = reflect_constant_buffers(reflection_object.Get(), num_constant_buffers);
      desc.input_params = reflect_input_params(reflection_object.Get(), num_input_params);
      desc.output_params = reflect_output_params(reflection_object.Get(), num_output_params);
      desc.bound_resources = reflect_bound_resources(reflection_object.Get(), num_bound_resources, shader->type());
      desc.type = shader->type();

      return ShaderSignature(rsl::move(desc));
    }
  }
}