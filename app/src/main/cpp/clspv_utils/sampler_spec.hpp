//
// Created by Eric Berdahl on 10/22/17.
//

#ifndef CLSPVUTILS_SAMPLER_SPEC_HPP
#define CLSPVUTILS_SAMPLER_SPEC_HPP

#include "clspv_utils_fwd.hpp"

#include <vulkan/vulkan.hpp>

namespace clspv_utils {

    struct sampler_spec_t {
        int opencl_flags    = 0;
        int descriptor_set  = -1;
        int binding         = -1;
    };

    bool isSamplerSupported(int opencl_flags);

    vk::UniqueSampler createCompatibleSampler(vk::Device device, int opencl_flags);

    void validateSampler(const sampler_spec_t& spec, int requiredDescriptorSet = -1);
}

#endif //CLSPVUTILS_SAMPLER_SPEC_HPP
