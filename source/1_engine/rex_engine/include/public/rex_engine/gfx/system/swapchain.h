#pragma once

#include "rex_engine/gfx/resources/render_target.h"
#include "rex_engine/gfx/core/texture_format.h"

namespace rex
{
  namespace gfx
  {
    // An empty structure, it's just to indicate that the swapchain's backbuffer should be used as color buffer
		// We can't save the backbuffer's render target as a member of the swapchain info as it updates every frame
		// When render passes want to use it as a render target, they need some form of indirection to this render target
		// Therefore they can use the below handle
		struct SwapchainFrameBufferHandle {};
		// This function returns a new swapchain frame buffer handle
		SwapchainFrameBufferHandle swapchain_frame_buffer_handle();

    class Swapchain
    {
    public:
      Swapchain(s32 width, s32 height, TextureFormat format);
      virtual ~Swapchain() = default;

      // Swap the back with the front buffer
      virtual void present() = 0;

      // Return the width of the buffers of the swapchain
      s32 width() const;
      // Return the height of the buffers of the swapchain
      s32 height() const;
      // Return the format of the buffers of the swapchain
      TextureFormat format() const;
      // Return the number of buffers of the swapchain
      s32 num_buffers() const;
      // Return the current backbuffer as a render target
      RenderTarget* current_buffer();

      // Return the index of the current bufffer
      virtual s32 current_buffer_idx() const = 0;

      // Resize the backbuffers to a new resolution
      virtual void resize(s32 newWidth, s32 newHeight) = 0;

    protected:
      // Empty the list of buffers
      void clear_buffers();
      // Store a new render target pointing to one of the back buffers
      void store_render_target(rsl::unique_ptr<RenderTarget> buffer);
      // called on resize to change the cached width and height
      void on_resize(s32 width, s32 height);

      // Return the buffer at the given index
      RenderTarget* buffer_at(s32 idx);

    private:
      rsl::vector<rsl::unique_ptr<RenderTarget>> m_swapchain_buffers;
      s32 m_width;
      s32 m_height;
      TextureFormat m_format;
    };
  }
}