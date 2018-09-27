//
// Created by Eric Berdahl on 10/31/17.
//

#ifndef CLSPVTEST_FILL_KERNEL_HPP
#define CLSPVTEST_FILL_KERNEL_HPP

#include "clspv_utils/clspv_utils_fwd.hpp"
#include "clspv_utils/kernel.hpp"
#include "gpu_types.hpp"
#include "test_utils.hpp"
#include "vulkan_utils/vulkan_utils.hpp"

#include <vulkan/vulkan.h>

namespace fill_kernel {

    clspv_utils::execution_time_t
    invoke(clspv_utils::kernel&             kernel,
           vulkan_utils::storage_buffer&    dst_buffer,
           int                              pitch,
           int                              device_format,
           int                              offset_x,
           int                              offset_y,
           int                              width,
           int                              height,
           const gpu_types::float4&         color);

    test_utils::KernelTest::invocation_tests getAllTestVariants();

    template <typename PixelType>
    test_utils::InvocationResult test(clspv_utils::kernel&              kernel,
                                      const std::vector<std::string>&   args,
                                      bool                              verbose) {
        test_utils::InvocationResult invocationResult;
        auto& device = kernel.getDevice();

        vk::Extent3D bufferExtent(64, 64, 1);
        const gpu_types::float4 color = { 0.25f, 0.50f, 0.75f, 1.0f };

        for (auto arg = args.begin(); arg != args.end(); arg = std::next(arg)) {
            if (*arg == "-w") {
                arg = std::next(arg);
                if (arg == args.end()) throw std::runtime_error("badly formed arguments to fill test");
                bufferExtent.width = std::atoi(arg->c_str());
            }
            else if (*arg == "-h") {
                arg = std::next(arg);
                if (arg == args.end()) throw std::runtime_error("badly formed arguments to fill test");
                bufferExtent.height = std::atoi(arg->c_str());
            }
        }

        std::ostringstream os;
        os << "<w:" << bufferExtent.width << " h:" << bufferExtent.height << " d:" << bufferExtent.depth << ">";
        invocationResult.mParameters = os.str();

        // allocate image buffer
        const std::size_t buffer_length = bufferExtent.width * bufferExtent.height * bufferExtent.depth;
        const std::size_t buffer_size = buffer_length * sizeof(PixelType);
        vulkan_utils::storage_buffer dst_buffer(device.getDevice(), device.getMemoryProperties(), buffer_size);

        const PixelType src_value = pixels::traits<PixelType>::translate((gpu_types::float4){ 0.0f, 0.0f, 0.0f, 0.0f });
        auto dstBufferMap = dst_buffer.map<PixelType>();
        std::fill(dstBufferMap.get(), dstBufferMap.get() + buffer_length, src_value);
        dstBufferMap.reset();

        invocationResult.mExecutionTime = invoke(kernel,
                                                 dst_buffer, // dst_buffer
                                                 bufferExtent.width,   // pitch
                                                 pixels::traits<PixelType>::device_pixel_format, // device_format
                                                 0, 0, // offset_x, offset_y
                                                 bufferExtent.width, bufferExtent.height, // width, height
                                                 color); // color

        dstBufferMap = dst_buffer.map<PixelType>();
        test_utils::check_results(dstBufferMap.get(),
                                  bufferExtent,
                                  bufferExtent.width,
                                  color,
                                  verbose,
                                  invocationResult);

        return invocationResult;
    }

    template <typename PixelType>
    test_utils::InvocationTest getTestVariant()
    {
        test_utils::InvocationTest result;

        std::ostringstream os;
        os << "<dst:" << pixels::traits<PixelType>::type_name << ">";
        result.mVariation = os.str();

        result.mTestFn = test<PixelType>;

        return result;
    }


}

#endif //CLSPVTEST_FILL_KERNEL_HPP