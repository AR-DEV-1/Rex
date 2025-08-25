#pragma once

namespace rex
{
  namespace gfx
  {
    // The resource class just acts as an base class for all gpu resources
    class Resource
    {
    public:
      virtual ~Resource() = default;

      virtual void debug_set_name(rsl::string_view name) {}

    private:

    };
  }
}