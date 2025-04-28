#pragma once

#include "rex_engine/gfx/resources/upload_buffer.h"

namespace rex
{
	namespace gfx
	{
    class RenderEngine;

    // A lock around the upload buffer
    // Returned from the engine when the upload buffer gets locked
    // It automatically unlocks the upload buffer when it goes out of scope
    class UploadBufferLock
    {
    public:
      UploadBufferLock(RenderEngine* owningEngine, UploadBuffer* uploadBuffer);
      UploadBufferLock(const UploadBufferLock&) = delete;
      UploadBufferLock(UploadBuffer&&) = delete;

      ~UploadBufferLock();

      UploadBufferLock& operator=(const UploadBufferLock&) = delete;
      UploadBufferLock& operator=(UploadBufferLock&&) = delete;

      // Return the locked upload buffer
      UploadBuffer* upload_buffer();

    private:
      RenderEngine* m_owning_engine;
      UploadBuffer* m_upload_buffer;
    };
	}
}