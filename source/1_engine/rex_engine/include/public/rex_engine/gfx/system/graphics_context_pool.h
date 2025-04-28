#pragma once

#include "rex_engine/pooling/growing_pool.h"
#include "rex_engine/gfx/system/graphics_context.h"
#include "rex_engine/gfx/system/command_allocator_pool.h"

#include "rex_std/functional.h"

namespace rex
{
  namespace gfx
  {
    class CommandAllocator;
    
    template <typename TGraphicsCtx>
    class ScopedGraphicsContext : public ScopedPoolObject<TGraphicsCtx, GraphicsContext>
    {
    public:
      ScopedGraphicsContext(TGraphicsCtx* ctx, GrowingPool<GraphicsContext>* pool)
        : ScopedPoolObject<TGraphicsCtx, GraphicsContext>(ctx, pool)
      {}
      ScopedGraphicsContext(const ScopedGraphicsContext&) = delete;
      ScopedGraphicsContext(ScopedGraphicsContext&&) = default;
      ~ScopedGraphicsContext()
      {
        if (this->get())
        {
          this->get()->execute_on_gpu();
        }
      }
      ScopedGraphicsContext& operator=(const ScopedGraphicsContext&) = delete;
      ScopedGraphicsContext& operator=(ScopedGraphicsContext&&) = default;

    private:

    };

    // The pool holding graphics contexts.
    // A pool is owned by the engine and is used to get a context from, for the given engine
    class GraphicsContextPool
    {
      using alloc_context_func = rsl::function<rsl::unique_ptr<GraphicsContext>()>;

    public:
      GraphicsContextPool();

      // Request a new graphics context, which has for sure finished its commands.
      // If none is found, create a new one
      template <typename TGraphicsCtx>
      ScopedGraphicsContext<TGraphicsCtx> request(const alloc_context_func& allocFunc)
      {
        // We don't care which one we get, so we'll just get first we can find
        auto find_free_ctx = [](const rsl::unique_ptr<GraphicsContext>&) { return true; }; // any idle one will do
        GraphicsContext* ctx = m_context_pool.request(find_free_ctx, allocFunc);

        return ScopedGraphicsContext<TGraphicsCtx>(static_cast<TGraphicsCtx*>(ctx), &m_context_pool);
      }


    private:
      GrowingPool<GraphicsContext> m_context_pool;
    };
  }
}