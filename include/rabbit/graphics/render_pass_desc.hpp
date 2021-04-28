#pragma once

#include "texture_format.hpp"

#include <vector>

namespace rb {
    /**
     * @brief Enumeration for render pass attachment load operations.
     */
    enum class attachment_load_operator {
        /**
         * @brief We don't care about the previous content of the respective render target attachment.
         */
        undefined,

        /**
         * @brief Loads the previous content of the respective render target attachment.
         */
        load,

        /**
         * @brief Clear the previous content of the respective render target attachment.
        */
        clear,
    };

    /**
     * @brief Enumeration for render pass attachment store operations.
     */
    enum class attachment_store_operator {
        /**
         * @brief We don't care about the outcome of the respective render target attachment.
         *
         *        Can be used, for example, if we only need the depth buffer for the depth test,
         *        but nothing is written to it.
         */
        undefined,

        /**
         * @brief Stores the outcome in the respective render target attachment.
         */
        store
    };

    struct attachment_format_desc {
        /**
         * @brief Specifies the render target attachment format.
         *
         *        If the render pass is used for render targets,
         *        the format depends on the render target attachments.
         *        If this is undefined, the corresponding attachment is not used.
         */
        texture_format format{ texture_format::undefined };

        /**
         * @brief Specifies the load operation of the previous attachment content.
         */
        attachment_load_operator attachment_load_operator{ attachment_load_operator::undefined };

        /**
         * @brief Specifies the store operation of the outcome for the respective attachment content.
         */
        attachment_store_operator attachment_store_operator{ attachment_store_operator::undefined };
    };

    /**
     * @brief Render pass descriptor structure.
     *
     *        A render pass object can be used across multiple render targets.
     *        Moreover, a render target can be created with a different render pass object
     *        than the one used for command_buffer::begin_render_pass as long as they are compatible.
     */
    struct render_pass_desc {
        /**
         * @brief Specifies the color attachments used within the render pass.
         *
         *        A render context usually uses a BGRA format instead of an RGBA format.
         */
        std::vector<attachment_format_desc> color_attachments;

        /**
         * @brief Specifies the depth attachment used within the render pass.
         *
         *        The depth attachment and stencil attachment usually share the same format.
         *        They are separated here to specify different load and store operations.
         */
        attachment_format_desc depth_attachment;

        /**
         * @brief Specifies the stencil attachment used within the render pass.
         *
         *        The depth attachment and stencil attachment usually share the same format.
         *        They are separated here to specify different load and store operations.
         */
        attachment_format_desc stencil_attachment;

        /**
         * @brief Specifies the number of samples for the respective render target attachment.
         *
         *        This must be greater than 0. If this is 1, multi-sampling is disabled.
         *        All attachments and the respective render target must have the same number of samples.
         */
        std::size_t sampler{ 1 };
    };
}
