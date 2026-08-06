#include "FileReader.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <map>
#include <optional>
#include <print>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>


static auto APP_NAME                  = "Vulkan Renderer";
static auto APP_ID                    = "com.alan.vulkan_renderer";
static auto APP_VERSION               = "1.0.0";
static const int MAX_FRAMES_IN_FLIGHT = 2;

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily{ std::nullopt };
    std::optional<uint32_t> presentFamily{ std::nullopt }; // Queue for presenting image to surface

    /**
     * @brief Returns whether the current queue family has a device feature set.
     */
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};


struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;      /// Min/Max number, width, height of swap chain images.
    std::vector<VkSurfaceFormatKHR> format;     /// Pixel format, color space etc.
    std::vector<VkPresentModeKHR> presentModes; /// Presentation Mode
};


class HelloTriangleApplication
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanUp();
    }

private:
    void initWindow()
    {
        SDL_SetAppMetadata(APP_NAME, APP_VERSION, APP_ID);
        if (!SDL_Init(SDL_INIT_VIDEO))
            throw std::exception("There was an error initializing the Window");

        _window = SDL_CreateWindow(APP_NAME, WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
    }

    void initVulkan()
    {
        createInstance();
        queryAvailableExtensions();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
    }

    void mainLoop()
    {
        while (_gameRunning)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_EVENT_QUIT:
                        _gameRunning = false;
                        break;
                    default:
                        continue;
                }
            }
            drawFrame();
        }
        // Since vk is asynchronous, drawing might be still going on when program terminates
        // so to prevent errors we need to wait on the logical device before destroying and cleaning up
        vkDeviceWaitIdle(_vkDevice);
    }

    void cleanUp()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(_vkDevice, _imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(_vkDevice, _renderingFinishedSemaphores[i], nullptr);
            vkDestroyFence(_vkDevice, _inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(_vkDevice, _commandPool, nullptr);

        for (const auto& frameBuffer : _swapChainFramebuffers)
            vkDestroyFramebuffer(_vkDevice, frameBuffer, nullptr);

        vkDestroyPipeline(_vkDevice, _graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_vkDevice, _pipelineLayout, nullptr);
        vkDestroyRenderPass(_vkDevice, _renderPass, nullptr);
        for (const auto& swapChainImageView : _swapChainImageViews)
            vkDestroyImageView(_vkDevice, swapChainImageView, nullptr);

        vkDestroySwapchainKHR(_vkDevice, _vkSwapChain, nullptr);
        vkDestroySurfaceKHR(_vkInstance, _vkSurface, nullptr);
        vkDestroyDevice(_vkDevice, nullptr);

        if (_enabledValidationLayers)
            DestroyDebugUtilsMessengerEXT(_vkInstance, _debugMessenger, nullptr);

        vkDestroyInstance(_vkInstance, nullptr);
        SDL_DestroyWindow(_window);
    }

    void drawFrame()
    {
        //// STEPS
        /// 1. Wait for previous frame to finish
        /// 2. Acquire swap chain image
        /// 3. Record command buffer
        /// 4. Submit command buffer
        /// 5. Present the swap chain image

        // Wait for previous frame to finish
        vkWaitForFences(_vkDevice, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(_vkDevice, 1, &_inFlightFences[_currentFrame]); // We need to manually reset the fence

        // Acquire an image from the swap chain
        uint32_t imageIndex;
        vkAcquireNextImageKHR(_vkDevice, _vkSwapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame],
                              VK_NULL_HANDLE, &imageIndex);

        // Record the command buffer
        vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);
        recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

        // Submit the command buffer
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // Specifies which semaphores to wait on before execution begin
        // the pipeline to wait in which in our cases is writing the color
        // attachment
        VkSemaphore waitSemaphores[]      = { _imageAvailableSemaphores[_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount     = 1;
        submitInfo.pWaitSemaphores        = waitSemaphores;
        submitInfo.pWaitDstStageMask      = waitStages;

        // Specify which command buffers to submit for execution
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &_commandBuffers[_currentFrame];

        // Specify which semaphores to signal once command is finished executing
        // FIX: Must acquire the signalling/rendering finished semaphore based on current imageindex
        // not the frame count
        VkSemaphore signalSemaphores[]  = { _renderingFinishedSemaphores[imageIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        // Submit the queue
        // The fence is optional providing signalling when the command buffers finish execution
        if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("There was an error submitting the command buffer");
        }

        // Presentation
        VkPresentInfoKHR presentInfoKhr{};
        presentInfoKhr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        // Wait for the command buffer ot finish executing(signaled via signalSemaphore)
        // before presenting
        presentInfoKhr.waitSemaphoreCount = 1;
        presentInfoKhr.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapChains[]   = { _vkSwapChain };
        presentInfoKhr.swapchainCount = 1;
        presentInfoKhr.pSwapchains    = swapChains;
        presentInfoKhr.pImageIndices  = &imageIndex;

        presentInfoKhr.pResults = nullptr; // Optional used to get result of each swap chain exec

        vkQueuePresentKHR(_presentQueue, &presentInfoKhr);

        _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void createCommandBuffers()
    {
        _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        /// Clean up automatically when command pool is freed
        /// so no need for explicit cleanup

        // Create a single command buffer with the initialized command pool
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = _commandPool;
        allocateInfo.level =
            VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Can be submitted directly but cannot be shared by another queue
        allocateInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

        if (vkAllocateCommandBuffers(_vkDevice, &allocateInfo, _commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("There was an error allocating command buffer(s)");
        }
    }

    void createSyncObjects()
    {
        _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        // FIX: Submit/Rendering finished semaphore must not be coupled to frames in flight, but
        // to the number of swap chain images acquired
        _renderingFinishedSemaphores.resize(_swapChainImages.size());
        _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // Fence by default are started in unsignaled state, but we wait on them to be signaled inside the
        // draw call. Since, they are only signalled after a frame finished rendering, we are essentially waiting
        // indefinitely on the first frame. To work around this, we need to create the fence in signaled state
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(_vkDevice, &semaphoreCreateInfo, nullptr, &_imageAvailableSemaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateFence(_vkDevice, &fenceCreateInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("There was an error creating the semaphores");
            }
        }

        for (size_t i = 0; i < _swapChainImages.size(); ++i)
        {
            if (vkCreateSemaphore(_vkDevice, &semaphoreCreateInfo, nullptr, &_renderingFinishedSemaphores[i]) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("There was an error creating the sumbit semaphores");
            }
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo =
            nullptr; // Only needed if we are using secondary command buffer specifying the state to inherit from

        if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("There was an error which starting command buffer recording");
        }

        // Configure the render pass beginning with the target framebuffer and the renderpass we created earlier
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.framebuffer = _swapChainFramebuffers[imageIndex];
        renderPassBeginInfo.renderPass  = _renderPass;

        // Specify the render area(anything outside of this is undefined)
        renderPassBeginInfo.renderArea.offset = { .x = 0, .y = 0 };
        renderPassBeginInfo.renderArea.extent = _swapChainExtent;

        // Set the clear values
        VkClearValue vkClearValue{ { 0.0f, 0.0f, 0.0f, 1.0f } };
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues    = &vkClearValue;

        // Begin the render pass
        // CONTENTS_INLINE -> Commands are embedded in the primary command buffer and there will be no secondary
        // COMMAND BUFFERS
        vkCmdBeginRenderPass(_commandBuffers[_currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        ////////////////////////////////////////////
        /// GRAPHICS PIPELINE BINDING
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
        // Configure viewport and scissor as we need them to be dynamic
        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(_swapChainExtent.width);
        viewport.height   = static_cast<float>(_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = _swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("There was an error recording the command");
        }
    }

    void createFramebuffers()
    {
        // Resize the frame buffer [] to be of the same size as the image views []
        _swapChainFramebuffers.resize(_swapChainImageViews.size());
        // Iterate through each of the image view and create a framebuffer
        for (size_t i = 0; i < _swapChainImageViews.size(); ++i)
        {
            VkImageView attachments[] = { _swapChainImageViews[i] };

            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass      = _renderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments    = attachments;
            framebufferCreateInfo.width           = _swapChainExtent.width;
            framebufferCreateInfo.height          = _swapChainExtent.height;
            framebufferCreateInfo.layers          = 1;

            if (vkCreateFramebuffer(_vkDevice, &framebufferCreateInfo, nullptr, &_swapChainFramebuffers[i]) !=
                VK_SUCCESS)
                throw std::runtime_error("There was an error creating framebuffers");
        }
    }

    void createCommandPool()
    {
        const auto [graphicsFamily, presentFamily] = findQueueFamilies(_physicalDevice);

        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Individual re-record command buffers
        commandPoolCreateInfo.queueFamilyIndex = graphicsFamily.value();

        if (vkCreateCommandPool(_vkDevice, &commandPoolCreateInfo, nullptr, &_commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("There was an error creating command pool");
        }
    }

    void createGraphicsPipeline()
    {
        const auto vertexShaderCode   = readFile("shaders/basic.vert.spv");
        const auto fragmentShaderCode = readFile("shaders/basic.frag.spv");

        const auto vertexShaderModule   = createShaderModule(vertexShaderCode);
        const auto fragmentShaderModule = createShaderModule(fragmentShaderCode);

        // Vertex shader pipeline
        VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
        vertexShaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageCreateInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT; // Vertex shader
        vertexShaderStageCreateInfo.module = vertexShaderModule;
        vertexShaderStageCreateInfo.pName  = "main"; // Entry point
        // vertexShaderStageCreateInfo.pSpecializationInfo = nullptr; // Shader constants (set to null by braced init)

        // Fragment shader pipeline
        VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
        fragmentShaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStageCreateInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageCreateInfo.module = fragmentShaderModule;
        fragmentShaderStageCreateInfo.pName  = "main"; // Entry point

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };


        ////////////////////////////////////////////////
        /// FIXED PIPELINE
        ///

        // Setup vertex input info
        VkPipelineVertexInputStateCreateInfo vertexCreateInfo{};
        vertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        // Attribute count and pointers are set to 0 and nullptr since we have hardcoded values in shader
        vertexCreateInfo.vertexBindingDescriptionCount   = 0;
        vertexCreateInfo.pVertexBindingDescriptions      = nullptr;
        vertexCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexCreateInfo.pVertexAttributeDescriptions    = nullptr;

        // Input assembly: Primitive to draw
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
        inputAssemblyCreateInfo.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Triangles from 3 vertices
        inputAssemblyCreateInfo.primitiveRestartEnable =
            VK_FALSE; // Optimization for reusing vertices when using _STRIP topology

        // Viewport (Transform)
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        // Using swap chain extends dimensions since we will be using these sizes for frame buffers
        viewport.width    = static_cast<float>(_swapChainExtent.width);
        viewport.height   = static_cast<float>(_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        // Scissor (Clipping)
        VkRect2D scissor{};
        scissor.offset = { .x = 0, .y = 0 };
        scissor.extent = _swapChainExtent;


        //////////////////////////////////
        /// DYNAMIC VIEWPORT AND SCISSOR
        std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        // Setup the dynamic states
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates    = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportCreateInfo{};
        viewportCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportCreateInfo.viewportCount = 1;
        viewportCreateInfo.scissorCount  = 1;
        //// The actual viewport and scissor rectangles will be set up at draw time
        /// Setting VP and SCI now
        viewportCreateInfo.pViewports = &viewport;
        viewportCreateInfo.pScissors  = &scissor;


        //////////////////////////////////
        /// RASTERIZATION

        VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo{};
        rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationCreateInfo.depthClampEnable =
            VK_FALSE; // Fragments in beyond far and near plane and NOT discarded but clamped
        rasterizationCreateInfo.rasterizerDiscardEnable =
            VK_FALSE; // If true, geometry never passes to rasterizer, disabling any frame buffer output.
        rasterizationCreateInfo.polygonMode =
            VK_POLYGON_MODE_FILL;                 // Fill the area of polygon(any other mode requires GPU feature)
        rasterizationCreateInfo.lineWidth = 1.0f; // Thickness in terms of number of fragment(Hardware dependent)
        rasterizationCreateInfo.cullMode  = VK_CULL_MODE_BACK_BIT;   // Backface culling
        rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // Vertex ordering

        // Alter depth values(useful for shadow mapping), disabled
        rasterizationCreateInfo.depthBiasEnable         = VK_FALSE;
        rasterizationCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
        rasterizationCreateInfo.depthBiasClamp          = 0.0f; // Optional
        rasterizationCreateInfo.depthBiasSlopeFactor    = 0.0f; // Optional

        ///////////////////////////////////////////////////
        /// Multisampling (AA) -> DISABLED FOR NOW
        ///
        VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
        multisamplingCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingCreateInfo.sampleShadingEnable   = VK_FALSE;
        multisamplingCreateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisamplingCreateInfo.minSampleShading      = 1.0f;     // Optional
        multisamplingCreateInfo.pSampleMask           = nullptr;  // Optional
        multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        multisamplingCreateInfo.alphaToOneEnable      = VK_FALSE; // Optional


        ////////////////////////////////////////////////////
        /// Depth and stencil testing -> DISABLED FOR NOW
        ///
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        // Color Blending
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;      // Optional
        // Alpha Blending
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;      // Optional

        // Blend constants
        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
        colorBlendingCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingCreateInfo.logicOpEnable     = VK_FALSE;
        colorBlendingCreateInfo.logicOp           = VK_LOGIC_OP_COPY; // Optional
        colorBlendingCreateInfo.attachmentCount   = 1;
        colorBlendingCreateInfo.pAttachments      = &colorBlendAttachment;
        colorBlendingCreateInfo.blendConstants[0] = 0.0f; // Optional
        colorBlendingCreateInfo.blendConstants[1] = 0.0f; // Optional
        colorBlendingCreateInfo.blendConstants[2] = 0.0f; // Optional
        colorBlendingCreateInfo.blendConstants[3] = 0.0f; // Optional

        // Pipeline Layout
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount         = 0;       // Optional
        pipelineLayoutCreateInfo.pSetLayouts            = nullptr; // Optional
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;       // Optional
        pipelineLayoutCreateInfo.pPushConstantRanges    = nullptr; // Optional

        if (vkCreatePipelineLayout(_vkDevice, &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout!");
        }


        //////////////////////////////////////////////////////////////////////////////////////
        /// PIPELINE CREATION
        ///
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        /// Pass in shader structures
        pipelineCreateInfo.stageCount = 2; // Vertex, Fragment
        pipelineCreateInfo.pStages    = shaderStages;

        /// Pass in fixed function stage structures
        pipelineCreateInfo.pVertexInputState   = &vertexCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineCreateInfo.pViewportState      = &viewportCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
        pipelineCreateInfo.pMultisampleState   = &multisamplingCreateInfo;
        pipelineCreateInfo.pDepthStencilState  = nullptr; // Optional
        pipelineCreateInfo.pColorBlendState    = &colorBlendingCreateInfo;
        pipelineCreateInfo.pDynamicState       = &dynamicStateCreateInfo;

        // Add the pipeline layout
        pipelineCreateInfo.layout = _pipelineLayout;

        // Add the render pass
        pipelineCreateInfo.renderPass = _renderPass;
        pipelineCreateInfo.subpass    = 0; // Index of subpass

        // Setup pipeline derivation(since we have none, put to nullptr and -1)
        // Helps in performance dpt by reusing partly configure pipelines
        // To use VK_PIPELINE_CREATE_DERIVATIVE_BIT  must be configured in VkGraphicsPipelineCreateInfo
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex  = -1;


        // vkCreateGraphicsPipeline can create multiple pipeline but we need only 1
        if (vkCreateGraphicsPipelines(_vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &_graphicsPipeline) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("There was an error creating a graphics pipeline.");
        }


        // NOTE: ONLY DESTROY SHADER MODULES AFTER PIPELINE CREATION
        // We can clean up the shader modules since they are loaded per pipeline
        vkDestroyShaderModule(_vkDevice, vertexShaderModule, nullptr);
        vkDestroyShaderModule(_vkDevice, fragmentShaderModule, nullptr);
    }

    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format  = _swapChainFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        // Determines what to do with the data before and after rendering
        colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;  // Clear pre-start
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Stored post-start for reading
        // No stencil buffer use
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // Layout prior to rendering
        // Contents are not preserved(NP as we are clearing it anyways)
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // Layout after rendering
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Image to be presented to the swap chain

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0; // First attachment layout(location=0) out vec4 outColor;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Subpass
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS; // Subpass binding point
        subpass.colorAttachmentCount = 1;                               // 1 color attachment
        subpass.pColorAttachments    = &colorAttachmentRef;             // Bind the color attachment ref

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments    = &colorAttachment;
        renderPassCreateInfo.subpassCount    = 1;
        renderPassCreateInfo.pSubpasses      = &subpass;

        // Subpass dependency
        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Refers to the implicit "before" subpass
        subpassDependency.dstSubpass = 0;                   // Refers to our subpass

        // Which stage to wait on
        // Wait for the swap chain to finish reading from the image before we can access it
        subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;

        // Operation on the color attachment stage should wait on this
        subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // add dependencies to render pass
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies   = &subpassDependency;

        if (vkCreateRenderPass(_vkDevice, &renderPassCreateInfo, nullptr, &_renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create render pass");
        }
    }


    VkShaderModule createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo shaderCreateInfo{};
        shaderCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderCreateInfo.codeSize = code.size();
        shaderCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_vkDevice, &shaderCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("There was an error creating the shader.");
        }
        return shaderModule;
    }

    void createInstance()
    {
// Enable Validation only in Debug mode
#ifdef NDEBUG
        const bool enableValidation = false;
        _enabledValidationLayers    = false;
#else
        constexpr bool enableValidation = true;
        _enabledValidationLayers        = true;
#endif


        // Optional information added for optimization
        VkApplicationInfo appInfo{};                        // Initializes pNext to nullptr (extension information)
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // sType needs to be specified

        // Application Info
        appInfo.pApplicationName   = APP_NAME;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

        // Engine Info
        appInfo.pEngineName   = "Vulkan Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

        // API Info
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Create an Instance info to specify to the driver which extensions and validation layers to use
        VkInstanceCreateInfo createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Connect SDL3 to Vulkan for creating vkInstance
        uint32_t instanceExtensionCount       = 0;
        const char* const* instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);

        std::vector<const char*> requiredExtensions;
        for (uint32_t i = 0; i < instanceExtensionCount; ++i)
            requiredExtensions.emplace_back(instanceExtensions[i]);

        if (enableValidation)
        {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Required on macOS which will give the `VK_ERROR_INCOMPATIBLE_DRIVER` when creating vkInstance
#if defined(__APPLE__) && defined(__MACH__)
        // MacOS Specific
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();


        //////  Validation Layer Enabling
        std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

        // Check for validation
        if (enableValidation && !queryLayerAvailability(validationLayers))
        {
            throw std::runtime_error("Validation layers requested but not supported!");
        }
        if (enableValidation)
        {
            // Enable validation
            createInfo.enabledLayerCount   = validationLayers.size();
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        SDL_Log("Enabled Vulkan Validation Layer!");

        // Create the vulkan instance
        if (vkCreateInstance(&createInfo, nullptr, &_vkInstance) == VK_SUCCESS)
            SDL_Log("Initialized Vulkan!");
        else
            SDL_Log("There was an error creating the Vulkan instance.");
    }


    void createImageViews()
    {
        // Resize the imageviews container to be the size of retrieved images
        _swapChainImageViews.resize(_swapChainImages.size());

        // Loop through each images and create an imageview for each
        for (std::size_t i = 0; i < _swapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = _swapChainImages[i];

            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Type of image, 1D, 2D, 3D, Cube map
            imageViewCreateInfo.format   = _swapChainFormat;

            // Apply no color channel swizzling
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            // Specify images purpose and which part of image should be accessed
            imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
            imageViewCreateInfo.subresourceRange.levelCount     = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount     = 1;

            // Create the image view
            if (vkCreateImageView(_vkDevice, &imageViewCreateInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("There was an error creating the swapchain ImageView!");
            }
        }
    }


    void setupDebugMessenger()
    {
        if (!_enabledValidationLayers)
            return;

        // Create messenger create info
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
        messengerCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerCreateInfo.pfnUserCallback = debugCallback;
        messengerCreateInfo.flags           = 0;       // Flags must be zero
        messengerCreateInfo.pUserData       = nullptr; // User data

        if (CreateDebugUtilsMessengerEXT(_vkInstance, &messengerCreateInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger");
    }

    void createSurface()
    {
        if (!SDL_Vulkan_CreateSurface(_window, _vkInstance, nullptr, &_vkSurface) != VK_SUCCESS)
            throw std::runtime_error("There was an error creating a rendering surface!");
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices familyIndices = findQueueFamilies(_physicalDevice);
        float queuePriority              = 1.0f; // Must specify a priority even if it's the one queue

        // Used Device features
        VkPhysicalDeviceFeatures physicalDeviceFeatures{}; // NOTE: Always empty initialize else the prog will crash

        std::set<uint32_t> queueFamilies = { familyIndices.graphicsFamily.value(),
                                             familyIndices.presentFamily.value() };
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (auto& queueFamily : queueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            // Create a single Queue for each type of queue we want
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount       = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Create Device create info for creating the logical device
        // with the required queue family
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos    = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures     = &physicalDeviceFeatures;
        deviceCreateInfo.enabledLayerCount    = 0;

        // Enable Swap chain and other layers
        deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // Instantiate Logical Device
        if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_vkDevice) != VK_SUCCESS)
            throw std::runtime_error("There was an error creating a vulkan logical device");

        // Queue are creating along with logical devices
        vkGetDeviceQueue(_vkDevice, familyIndices.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_vkDevice, familyIndices.presentFamily.value(), 0, &_presentQueue);
    }

    void pickPhysicalDevice()
    {
        uint32_t numDevices{};

        vkEnumeratePhysicalDevices(_vkInstance, &numDevices, nullptr);

        if (numDevices == 0)
            throw std::runtime_error("No devices with vulkan support present");

        std::vector<VkPhysicalDevice> physicalDevices(numDevices);
        vkEnumeratePhysicalDevices(_vkInstance, &numDevices, physicalDevices.data());

        std::multimap<int, VkPhysicalDevice> candidateDevices;

        // Associating each device with a score based on our requirements
        for (const auto& device : physicalDevices)
        {
            // Use the first suitable device
            if (isDeviceSuitable(device))
            {
                _physicalDevice = device;
                break;
            }
            // int score = rateDeviceSuitability(device);
            // candidateDevices.insert(std::make_pair(score, device));
        }

        // Check if the candidate gpu supports our required features
        // Since we are using a reverse iterator, we will get the GPU with the largest score
        // if (candidateDevices.rbegin()->first > 0)
        // {
        //     _physicalDevice = candidateDevices.rbegin()->second;
        // }
        // else
        // {
        //     throw std::runtime_error("Failed to find a suitable GPU!");
        // }
    }

    [[maybe_unused]] int rateDeviceSuitability(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        int score = 0;

        // Huge score increment to separate discrete from integrate GPU
        score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1000 : 0;

        // Max texture size
        score += deviceProperties.limits.maxImageDimension2D;

        // Don't support GPU that don't have a geometry shader
        if (!deviceFeatures.geometryShader)
            return 0;

        return score;
    }

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices;

        uint32_t queueFamiliesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamiliesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilyProperties.data());

        int i = 0;

        // Find at least one family that supports VK_QUEUE_GRAPHICS_BIT
        for (const auto& queueFamily : queueFamilyProperties)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
                break;
            }
            ++i;

            // if (indices.isComplete())
            //     break;
        }

        /////////////////////////////////
        /// FINDING PRESENTING SUPPORT //
        /////////////////////////////////
        VkBool32 presentFamily;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _vkSurface, &presentFamily);
        if (presentFamily)
            indices.presentFamily = i; // Might return the same queue(one queue with both graphics and present support)

        return indices;
    }

    void createSwapChain()
    {
        // Query the swap chain support details
        SwapChainSupportDetails supportDetails = querySwapChainSupportDetails(_physicalDevice);

        // Choose an apt format, present mode and extent(resolution)
        VkSurfaceFormatKHR format    = chooseSurfaceFormat(supportDetails.format);
        VkPresentModeKHR presentMode = choosePresentMode(supportDetails.presentModes);
        VkExtent2D extent            = chooseSwapExtent(supportDetails.capabilities);

        // At least increment image count by 1 to ensure we don't to wait
        // for driver to complete its internal operation to get another image.
        uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;

        // If we however cant have more than the min, clamp it to the max images supported.
        // Zero => No maximum
        if (supportDetails.capabilities.minImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount)
            imageCount = supportDetails.capabilities.maxImageCount;


        // Create swap chain create info
        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = _vkSurface;

        // Setup the colorspace and image format
        swapChainCreateInfo.minImageCount   = imageCount;
        swapChainCreateInfo.imageFormat     = format.format;
        swapChainCreateInfo.imageColorSpace = format.colorSpace;

        swapChainCreateInfo.imageArrayLayers = 1; // Doesn't need more than 1 unless for stereoscopic 3D
        // To render to separate image use `VK_IMAGE_USAGE_TRANSFER_DST_BIT`
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Use the image for direct rendering

        // Setup queue sharing mode
        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);


        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        // If Graphics family and present families are separate queue, we need to
        // set image sharing mode to concurrent
        if (indices.graphicsFamily != indices.presentFamily)
        {
            // Shareability without explicit ownership requirements
            swapChainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }
        else
        {
            // Set the preset mode to Exclusive
            // Ownership is exclusive and implicit transfer of ownership is required
            swapChainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;       // Optional
            swapChainCreateInfo.pQueueFamilyIndices   = nullptr; // Optional
        }

        // Apply no transformation to the swap chain image
        swapChainCreateInfo.preTransform = supportDetails.capabilities.currentTransform;

        // Setup the present mode
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped     = VK_TRUE; // Clip any invisible areas

        // Setup composite transparency (window transparency)
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // No transparency


        // Setup the extent
        swapChainCreateInfo.imageExtent = extent;

        // Pass in the old swap chain
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;


        // Create the swap chain
        if (vkCreateSwapchainKHR(_vkDevice, &swapChainCreateInfo, nullptr, &_vkSwapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("There was an error creating a swapchain");
        }

        // Retrieve the swap chain images
        uint32_t numImages;
        vkGetSwapchainImagesKHR(_vkDevice, _vkSwapChain, &numImages, nullptr);
        _swapChainImages.resize(numImages);
        vkGetSwapchainImagesKHR(_vkDevice, _vkSwapChain, &numImages, _swapChainImages.data());

        // Store SwapChain format and extent for future use
        _swapChainExtent = extent;
        _swapChainFormat = format.format;
    }

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {

        // If we support an srgb 32-bit color format
        // choose it
        for (auto& format : availableFormats)
        {
            if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB)
                return format;
        }

        // If not choose the first color format supported
        return availableFormats[0];
    }

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availableModes)
    {
        // Choose the mailbox present mode if possible for minimal latency
        for (auto& mode : availableModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;
        }

        // IF not supported fall back to FIFO which is guaranteed to be supported
        return VK_PRESENT_MODE_FIFO_KHR;
    }


    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        // Resolution
        // If the swap chain has no maximum extent return it(essentially equal to uint32_t's max value
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }


        // Else get the window height and width and clamp it between the swap chain's min and max extent.

        int width, height;
        SDL_GetWindowSizeInPixels(_window, &width, &height);

        VkExtent2D actualExtent = { .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height) };

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }


    /**
     * @brief Returns if a vulkan device(GPU) has certain feature set like being discrete or having geometry shaders.
     */
    bool isDeviceSuitable(VkPhysicalDevice device)
    {

        // Query basic features like name, type, vulkan version
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        // Query optional features like texture compression, 64-bit floats etc.
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        SDL_Log("Device Found!\n %s", deviceProperties.deviceName);

        // Only support Discrete GPU and ones that have a geometry shader
        // return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
        QueueFamilyIndices indices = findQueueFamilies(device);

        const bool isExtensionSupported = checkDeviceExtensionSupport(device);

        bool isSwapChainSuitable{ false };
        if (isExtensionSupported)
        {
            auto swapChainDetails = querySwapChainSupportDetails(device);
            isSwapChainSuitable   = !swapChainDetails.presentModes.empty() && !swapChainDetails.format.empty();
        }

        return indices.isComplete() && isExtensionSupported && isSwapChainSuitable;
    }


    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // Query the available extensions
        uint32_t numExtensions;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(numExtensions);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, availableExtensions.data());

        // Form a set to store our required extensions
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        // Every time our required extension is in the devices supported extension list
        // remove it
        for (const auto& [extensionName, specVersion] : availableExtensions)
        {
            requiredExtensions.erase(extensionName);
        }

        // Return true if all our mandatory extensions are supported
        return requiredExtensions.empty();
    }




    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func =
            (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator)
    {
        auto func =
            (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    bool queryLayerAvailability(const std::vector<const char*>& layers)
    {
        uint32_t numLayers{};
        vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
        std::vector<VkLayerProperties> availableLayers(numLayers);
        vkEnumerateInstanceLayerProperties(&numLayers, availableLayers.data());

        bool layerAvailable = false;

        for (const auto& targetLayer : layers)
        {
            for (const auto& availableLayer : availableLayers)
            {
                if (strcmp(targetLayer, availableLayer.layerName) == 0)
                {
                    layerAvailable = true;
                    break;
                }
            }
            if (!layerAvailable)
                return false;
        }
        return true;
    }

    void queryAvailableExtensions()
    {
        uint32_t numExtensions{};
        // query the number of extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr);
        std::vector<VkExtensionProperties> extensions(numExtensions);

        // Query the extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensions.data());

        std::println("Supported extensions: ");
        for (const auto& [extensionName, specVersion] : extensions)
        {
            std::println("\t{}", extensionName);
        }
    }

    SwapChainSupportDetails querySwapChainSupportDetails(VkPhysicalDevice device)
    {

        SwapChainSupportDetails details;

        // Query Device capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _vkSurface, &details.capabilities);


        // Query format count and supported surface formats
        uint32_t numFormats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vkSurface, &numFormats, nullptr);

        if (numFormats != 0)
        {
            details.format.resize(numFormats);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vkSurface, &numFormats, details.format.data());
        }

        // Query present modes
        uint32_t numPresentModes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vkSurface, &numPresentModes, nullptr);

        if (numPresentModes != 0)
        {
            details.presentModes.resize(numPresentModes);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vkSurface, &numPresentModes,
                                                      details.presentModes.data());
        }
        return details;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Vulkan Validation]: (ERROR) %s", pCallbackData->pMessage);
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "[Vulkan Validation]: (WARNING) %s", pCallbackData->pMessage);
        else
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "[Vulkan Validation]: %s", pCallbackData->pMessage);
        return VK_FALSE;
    }

private:
    SDL_Window* _window{ nullptr };
    static constexpr size_t WIDTH  = 1280;
    static constexpr size_t HEIGHT = 720;

    bool _gameRunning{ true };

    bool _enabledValidationLayers{ false };
    VkInstance _vkInstance{};
    VkDebugUtilsMessengerEXT _debugMessenger{};
    VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
    VkDevice _vkDevice{};
    VkQueue _graphicsQueue{}, _presentQueue{};
    VkSurfaceKHR _vkSurface{};
    VkSwapchainKHR _vkSwapChain{};
    VkFormat _swapChainFormat{};
    VkExtent2D _swapChainExtent{};
    std::vector<VkImage> _swapChainImages{};
    std::vector<VkImageView> _swapChainImageViews{};
    VkRenderPass _renderPass{};
    VkPipelineLayout _pipelineLayout{};
    VkPipeline _graphicsPipeline{};
    VkCommandPool _commandPool{};
    std::vector<VkCommandBuffer> _commandBuffers{};
    std::vector<VkFramebuffer> _swapChainFramebuffers;
    uint32_t _currentFrame{ 0 };

    /// Synchronization
    // Semaphore for swap chain image acquire and rendering
    std::vector<VkSemaphore> _imageAvailableSemaphores{}, _renderingFinishedSemaphores{};
    std::vector<VkFence> _inFlightFences{}; // Fence for syncing frame renders

    // Swapchain support
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME // "VK_KHR_swapchain
    };
};

int main()
{
    HelloTriangleApplication app{};

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        SDL_Log("%s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}