#pragma once

#include "rex_std/bonus/memory/memory_size.h"
#include "rex_std/bonus/types.h"
#include "rex_std/memory.h"
#include "rex_std/string_view.h"

#include "rex_engine/diagnostics/assert.h"

namespace rex
{
  namespace memory
  {
    class Blob
    {
    public:
      // Initialize a blob with no underlying buffer
      Blob();
      Blob(const Blob& other) = delete;
      // Initialize a blob taken the underlying buffer from the other buffer
      // leaving the other buffer with no underlying buffer afterwards
      Blob(Blob&& other) noexcept;
      // Initialize a blob from a byte array.
      // This simply assigns the underlying buffer to this byte array
      explicit Blob(rsl::unique_array<rsl::byte>&& data);
      // Initialize a blob from a array
      // This simply assigns the underlying buffer to this byte array
      template <typename T>
      explicit Blob(rsl::unique_array<T>&& data)
          : m_data()
      {
        // "release" sets internal count to 0
        // so we need to make sure we cache this data before actually calling release.
        const s32 total_size = static_cast<s32>(data.byte_size());
        m_data         = rsl::unique_array<rsl::byte>((rsl::byte*)data.release(), total_size); // NOLINT(google-readability-casting)
      }
      // Initialize a blob from a pointer with size.
      // The blob takes ownership of the passed in pointer
      Blob(void* data, rsl::memory_size size);
      // Release the underlying pointer, giving the memory back to where it came from
      ~Blob();

      Blob& operator=(const Blob& other) = delete;
      // Release the underlying buffer and takes it from the other blob
      // The other blob doesn't hold a buffer anymore afterwards
      Blob& operator=(Blob&& other) noexcept;

      // Returns true if the blob holds a buffer
      // Returns false if it doesn't
      explicit operator bool() const;

      // Converts the blob to a string view
      explicit operator rsl::string_view() const;

      // Access into the underlying buffer by byte offset.
      rsl::byte& operator[](int32 index);
      const rsl::byte& operator[](int32 index) const;

      // Zero out the underlying buffer, setting all bytes to 0
      void zero_initialize();

      // Returns a const access to the Blob's underlying buffer
      const rsl::byte* data() const;
      // Returns the size of the underlying buffer
      rsl::memory_size size() const;

      // Returns the address of the underlying buffer casted into a specific type
      template <typename T>
      const T* data_as() const;

      // Read bytes from the underlying buffer at a certain offset and cast them to a type
      template <typename T>
      const T& read(rsl::memory_size offset = 0_bytes) const;

      // Read x amount of bytes from the underlying buffer at a certain offset and copy them into a desintation
      void* read_bytes(void* dst, rsl::memory_size inSize, rsl::memory_size inOffset = 0) const;

      // Write x amount of bytes into the underlying buffer, starting from a certain offset
      void write(const void* inData, rsl::memory_size inSize, rsl::memory_size inOffset = 0_bytes);

      // Return the internal data as another unique array and release it
      // Useful when you want to avoid reallocating the data
      template <typename T>
      rsl::unique_array<T> release_as_array()
      {
        REX_ASSERT_X(m_data.count() % sizeof(T) == 0, "You can't release a blob if you can't fit all the data in the resulting array");

        s32 count = static_cast<s32>(m_data.count());
        T* data_ptr = reinterpret_cast<T*>(m_data.release());
        return rsl::unique_array<T>(data_ptr, count / sizeof(T));
      }
      // Return the internal data as reinterpreted as something else
      // Useful when you want to avoid reallocating the data
      template <typename T>
      T release_as()
      {
        static_assert(rsl::is_pod_v<T>, "only pod classes are allowed to be released from blobs");
        REX_ASSERT_X(m_data.count() == sizeof(T), "Release blob content to a type is only allowed if the size of the type is the same as the size of the blob");

        T* data_ptr = reinterpret_cast<T*>(m_data.release());
        return *data_ptr;
      }

    private:
      rsl::unique_array<rsl::byte> m_data;
    };

    //-------------------------------------------------------------------------
    template <typename T>
    const T& Blob::read(rsl::memory_size offset) const
    {
      return *(T*)(m_data.get() + offset);
    }

    //-------------------------------------------------------------------------
    template <typename T>
    const T* Blob::data_as() const
    {
      return reinterpret_cast<const T*>(m_data.get());
    }

    rsl::string_view blob_to_string_view(const Blob& blob);
  } // namespace memory
} // namespace rex