#include "rex_engine/gfx/system/upload_buffer_lock.h"

#include "rex_engine/gfx/system/render_engine.h"

namespace rex
{
	namespace gfx
	{
    UploadBufferLock::UploadBufferLock(RenderEngine* owningEngine, UploadBuffer* uploadBuffer)
      : m_owning_copy_engine(owningEngine)
      , m_upload_buffer(uploadBuffer)
    {}

    UploadBufferLock::~UploadBufferLock()
    {
      m_owning_copy_engine->unlock_upload_buffer();
    }

    UploadBuffer* UploadBufferLock::upload_buffer()
    {
      return m_upload_buffer;
    }
	}
}