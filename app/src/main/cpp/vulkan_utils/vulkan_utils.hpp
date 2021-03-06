//
// Created by Eric Berdahl on 10/22/17.
//

#ifndef VULKAN_UTILS_HPP
#define VULKAN_UTILS_HPP

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory>
#include <ostream>
#include <vector>

namespace vulkan_utils {

    template <typename Type, typename Deleter>
    std::vector<Type> extractUniques(vk::ArrayProxy<vk::UniqueHandle<Type,Deleter> > uniques) {
        std::vector<Type> result;
        result.reserve(uniques.size());
        for (auto & u : uniques) {
            result.push_back(*u);
        }
        return result;
    };

    template <typename Type, typename Deleter>
    std::vector<Type> extractUniques(const std::vector<vk::UniqueHandle<Type,Deleter> >& uniques) {
        std::vector<Type> result;
        result.reserve(uniques.size());
        for (auto & u : uniques) {
            result.push_back(*u);
        }
        return result;
    };

    vk::UniqueDeviceMemory allocate_device_memory(vk::Device device,
                                                  const vk::MemoryRequirements&             mem_reqs,
                                                  const vk::PhysicalDeviceMemoryProperties& mem_props,
                                                  vk::MemoryPropertyFlags                   property_flags = vk::MemoryPropertyFlags());

    vk::UniqueCommandBuffer allocate_command_buffer(vk::Device device, vk::CommandPool cmd_pool);

    class device_memory {
    public:
        struct unmapper_t {
            unmapper_t(device_memory* s) : self(s) {}

            void    operator()(const void* ptr) { self->unmap(); }

            device_memory*  self;
        };

        template <typename T>
        using mapped_ptr = std::unique_ptr<T, unmapper_t>;

    public:
        device_memory() {}

        device_memory(vk::Device                                dev,
                      const vk::MemoryRequirements&             mem_reqs,
                      const vk::PhysicalDeviceMemoryProperties  mem_props);

        device_memory(const device_memory& other) = delete;

        device_memory(device_memory&& other);

        ~device_memory();

        device_memory&  operator=(const device_memory& other) = delete;

        device_memory&  operator=(device_memory&& other);

        void    swap(device_memory& other);

        vk::Device          getDevice() const { return mDevice; }

        void    bind(vk::Buffer buffer, vk::DeviceSize memoryOffset);

        void    bind(vk::Image image, vk::DeviceSize memoryOffset);

        template <typename T>
        inline mapped_ptr<T> map()
        {
            auto basicMap = map();
            return std::unique_ptr<T, unmapper_t>(static_cast<T*>(basicMap.release()), basicMap.get_deleter());
        }

        mapped_ptr<void> map();

    private:
        void    unmap();

    private:
        vk::Device              mDevice;
        vk::UniqueDeviceMemory  mMemory;
        bool                    mMapped;
    };

    inline void swap(device_memory& lhs, device_memory& rhs)
    {
        lhs.swap(rhs);
    }

    class uniform_buffer {
    public:
        template <typename T>
        using mapped_ptr = device_memory::mapped_ptr<T>;

    public:
        uniform_buffer () {}

        uniform_buffer (vk::Device dev, const vk::PhysicalDeviceMemoryProperties memoryProperties, vk::DeviceSize num_bytes);

        uniform_buffer (const uniform_buffer& other) = delete;

        uniform_buffer (uniform_buffer&& other);

        ~uniform_buffer();

        uniform_buffer& operator=(const uniform_buffer& other) = delete;

        uniform_buffer& operator=(uniform_buffer&& other);

        void    swap(uniform_buffer& other);

        vk::BufferMemoryBarrier  prepareForComputeRead();
        vk::BufferMemoryBarrier  prepareForTransferSrc();
        vk::BufferMemoryBarrier  prepareForTransferDst();
        vk::DescriptorBufferInfo use();

    public:
        template <typename T = void>
        inline mapped_ptr<T> map()
        {
            return mem.map<T>();
        }

    private:
        device_memory       mem;
        vk::UniqueBuffer    buf;
    };

    inline void swap(uniform_buffer& lhs, uniform_buffer& rhs)
    {
        lhs.swap(rhs);
    }

    class storage_buffer {
    public:
        template <typename T>
        using mapped_ptr = device_memory::mapped_ptr<T>;

    public:
        storage_buffer () {}

        storage_buffer (vk::Device dev, const vk::PhysicalDeviceMemoryProperties memoryProperties, vk::DeviceSize num_bytes);

        storage_buffer (const storage_buffer & other) = delete;

        storage_buffer (storage_buffer && other);

        ~storage_buffer ();

        storage_buffer & operator=(const storage_buffer & other) = delete;

        storage_buffer & operator=(storage_buffer && other);

        void    swap(storage_buffer & other);

        vk::BufferMemoryBarrier  prepareForComputeRead();
        vk::BufferMemoryBarrier  prepareForComputeWrite();
        vk::BufferMemoryBarrier  prepareForTransferSrc();
        vk::BufferMemoryBarrier  prepareForTransferDst();
        vk::DescriptorBufferInfo use();

    public:
        template <typename T = void>
        inline mapped_ptr<T> map()
        {
            return mem.map<T>();
        }

    private:
        device_memory       mem;
        vk::UniqueBuffer    buf;
    };

    inline void swap(storage_buffer & lhs, storage_buffer & rhs)
    {
        lhs.swap(rhs);
    }

    class staging_buffer;

    class image {
    public:
        enum Usage {
            kUsage_ReadOnly,
            kUsage_ReadWrite
        };

    public:
        static bool supportsFormatUse(vk::PhysicalDevice device, vk::Format format, Usage usage);

        image();

        image(vk::Device                                dev,
              const vk::PhysicalDeviceMemoryProperties  memoryProperties,
              vk::Extent3D                              extent,
              vk::Format                                format,
              Usage                                     usage);

        image(const image& other) = delete;

        image(image&& other);

        ~image();

        image&  operator=(const image& other) = delete;

        image&  operator=(image&& other);

        void    swap(image& other);

        staging_buffer  createStagingBuffer();

        vk::DescriptorImageInfo use();
        vk::ImageMemoryBarrier  prepare(vk::ImageLayout newLayout);

        vk::Extent3D getExtent() const { return mExtent; }

    private:
        vk::Device                          mDevice;
        vk::PhysicalDeviceMemoryProperties  mMemoryProperties;
        vk::ImageLayout                     mImageLayout;
        vk::UniqueDeviceMemory              mDeviceMemory;
        vk::Extent3D                        mExtent;
        vk::UniqueImage                     mImage;
        vk::UniqueImageView                 mImageView;
        vk::Format                          mFormat;
    };

    inline void swap(image& lhs, image& rhs)
    {
        lhs.swap(rhs);
    }

    class staging_buffer {
    public:
        template <typename T>
        using mapped_ptr = device_memory::mapped_ptr <T>;

    public:
        staging_buffer ();

        staging_buffer (vk::Device                           device,
                        vk::PhysicalDeviceMemoryProperties   memoryProperties,
                        image*                               image,
                        vk::Extent3D                         extent,
                        std::size_t                          pixelSize);

        staging_buffer (const staging_buffer & other) = delete;

        staging_buffer (staging_buffer && other);

        ~staging_buffer ();

        staging_buffer & operator=(const staging_buffer & other) = delete;

        staging_buffer & operator=(staging_buffer && other);

        void    swap(staging_buffer & other);

        void    copyToImage(vk::CommandBuffer commandBuffer);
        void    copyFromImage(vk::CommandBuffer commandBuffer);

        template <typename T>
        inline mapped_ptr<T> map()
        {
            return mStorageBuffer.map<T>();
        }

    private:
        vk::Device              mDevice;
        image*                  mImage;
        storage_buffer          mStorageBuffer;
        vk::Extent3D            mExtent;
    };

    inline void swap(staging_buffer & lhs, staging_buffer & rhs)
    {
        lhs.swap(rhs);
    }

    double timestamp_delta_ns(std::uint64_t                         startTimestamp,
                              std::uint64_t                         endTimestamp,
                              const vk::PhysicalDeviceProperties&   deviceProperties,
                              const vk::QueueFamilyProperties&      queueFamilyProperties);

    vk::Extent3D computeNumberWorkgroups(const vk::Extent3D& workgroupSize, const vk::Extent3D& dataSize);

    void copyBufferToImage(vk::CommandBuffer    commandBuffer,
                           storage_buffer&      buffer,
                           image&               image);

    void copyImageToBuffer(vk::CommandBuffer    commandBuffer,
                           image&               image,
                           storage_buffer&      buffer);
}

std::ostream& operator<<(std::ostream& os, vk::MemoryPropertyFlags vkFlags);
std::ostream& operator<<(std::ostream& os, vk::MemoryHeapFlags vkFlags);

std::ostream& operator<<(std::ostream& os, const vk::MemoryType& memoryType);
std::ostream& operator<<(std::ostream& os, const vk::MemoryHeap& memoryHeap);

#endif //VULKAN_UTILS_HPP
