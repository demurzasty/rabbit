#pragma once

#include "fwd.hpp"

namespace rb {
    /**
     * @brief Render pass interface.
     *
     *        A render pass is a high level construct adopted from Vulkan and Metal.
     *        It is used to tell the driver the various segments of a frame and which
     *        render target attachments are used and which depend on each other.
     */
    class render_pass {
    public:
        /**
         * @brief Constructs graphics pipeline with provided descriptor.
         */
        render_pass(const render_pass_desc& desc);

        /**
         * @brief Default virtual destructor.
         */
        virtual ~render_pass() = default;
    };
}
