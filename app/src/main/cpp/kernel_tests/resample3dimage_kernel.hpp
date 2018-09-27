//
// Created by Eric Berdahl on 10/31/17.
//

#ifndef CLSPVTEST_RESAMPLE3DIMAGE_KERNEL_HPP
#define CLSPVTEST_RESAMPLE3DIMAGE_KERNEL_HPP

#include "clspv_utils/clspv_utils_fwd.hpp"
#include "test_utils.hpp"
#include "vulkan_utils/vulkan_utils.hpp"

#include <vector>


namespace resample3dimage_kernel {

    clspv_utils::execution_time_t
    invoke(clspv_utils::kernel &kernel,
           vulkan_utils::image &src_image,
           vulkan_utils::storage_buffer &dst_buffer,
           int width,
           int height,
           int depth);

    test_utils::InvocationResult test(clspv_utils::kernel &kernel,
                                      const std::vector<std::string> &args,
                                      bool verbose);

    test_utils::KernelTest::invocation_tests getAllTestVariants();

}

#endif //CLSPVTEST_RESAMPLE3DIMAGE_KERNEL_HPP