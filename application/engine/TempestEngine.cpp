/**
 * @file TempestEngine.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: August 7, 2026
 *
 * @brief Implementation of declarations in VulkanRenderer.cppm
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

module;
#include "FileReader.h"

#include <cstring>
#include <format>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <sdl3/SDL.h>
#include <sdl3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

module TempestEngine;


namespace tempest
{
    void TempestEngine::init(const std::string& applicationName, const std::string& version, const std::string& id,
                             const size_t width, const size_t height)
    {
        appName      = applicationName;
        appVersion   = version;
        appId        = id;
        this->width  = width;
        this->height = height;

        SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appId.c_str());
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("Cannot initialize SDL window");
        }

        window = SDL_CreateWindow(appName.c_str(), this->width, this->height, SDL_WINDOW_VULKAN);

        initVulkan();

        _isRunning = true;
    }


    void TempestEngine::run()
    {
        while (_isRunning)
        {
            handleEvents();
            drawFrame();
        }
        device.waitIdle();
    }


    void TempestEngine::cleanup() const
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }


    void TempestEngine::initVulkan()
    {
        createVulkanInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createCommandPool();
        createCommandBuffer();
        createTextureImage();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorSetLayout();
        createDescriptorPool();
        createTextureSampler();
        createTextureImageView();
        createDescriptorSets();
        createGraphicsPipeline();
        createSyncObjects();
    }


    void TempestEngine::drawFrame()
    {
        // Wait for previous frame to finish
        // Wait for all the fences to be signalled
        if (const auto fenceResult = device.waitForFences(*drawFences[frameIndex], vk::True, UINT64_MAX);
            fenceResult != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to wait for fence");
        }
        device.resetFences(*drawFences[frameIndex]); // Fence needs to be manually reset

        // Acquire image from swap chain
        auto [result, imageIndex] =
            swapChain.acquireNextImage(UINT64_MAX, *presentFinishedSemaphores[frameIndex], nullptr);
        commandBuffers[frameIndex].reset();

        // Record a command buffer
        recordCommandBuffer(imageIndex);

        // Submit the recorded command buffer
        vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        updateUniformBuffer(frameIndex);
        const vk::SubmitInfo submitInfo{ .waitSemaphoreCount   = 1,
                                         .pWaitSemaphores      = &*presentFinishedSemaphores[frameIndex],
                                         .pWaitDstStageMask    = &waitDestinationStageMask,
                                         .commandBufferCount   = 1,
                                         .pCommandBuffers      = &*commandBuffers[frameIndex],
                                         .signalSemaphoreCount = 1,
                                         .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex] };
        graphicsQueue.submit(submitInfo, *drawFences[frameIndex]);

        // Subpass dependencies
        // DstSubpass must always be greater than srcSubpass
        // (OPTIONAL CODE)
        // vk::SubpassDependency dependency{ .srcSubpass    = vk::SubpassExternal, // Implicit subpass
        //                                   .dstSubpass    = 0,                   // 0 -> refers to our subpass
        //                                   .srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        //                                   .dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        //                                   .srcAccessMask = vk::AccessFlagBits::eNone,
        //                                   .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite };
        // vk::RenderPassCreateInfo renderPassInfo{ .dependencyCount = 1, .pDependencies = &dependency };

        // Present the swap chain
        const vk::PresentInfoKHR presentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
            .swapchainCount     = 1,
            .pSwapchains        = &*swapChain, // Swap chain to present the image to
            .pImageIndices      = &imageIndex, // The image index
            .pResults           = nullptr,     // Allows you to check whether the swapchain presentation was successful
        };

        const auto presentResult = graphicsQueue.presentKHR(presentInfoKHR);
        if (presentResult != vk::Result::eSuccess)
        {
            throw std::runtime_error("There was an error presenting the image.");
        }

        frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }


    void TempestEngine::createVulkanInstance()
    {
        const vk::ApplicationInfo applicationInfo{ .pApplicationName   = appName.c_str(),
                                                   .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
                                                   .pEngineName        = ENGINE_NAME.c_str(),
                                                   .engineVersion      = VK_MAKE_VERSION(0, 0, 1),
                                                   .apiVersion         = vk::ApiVersion13 };

        //------------
        // EXTENSIONS
        //------------
        const auto requiredExtensions  = getRequiredExtensions();
        const auto extensionProperties = context.enumerateInstanceExtensionProperties();

        // Check if the extension we require are support by vulkan
        auto unsupportedPropertiesIt =
            std::ranges::find_if(requiredExtensions, [&extensionProperties](const auto& requiredExtension) {
                return std::ranges::none_of(extensionProperties, [requiredExtension](const auto& extensionProperty) {
                    return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                });
            });
        if (unsupportedPropertiesIt != requiredExtensions.end())
        {
            throw std::runtime_error(std::format("Required extension not supported! {}", *unsupportedPropertiesIt));
        }


        //--------
        // LAYERS
        //--------
        std::vector<const char*> requiredLayers;
        if (enableValidationLayers)
        {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }
        // Enumerate through each layer and determine if the layers we need are supported
        const auto supportedLayers = context.enumerateInstanceLayerProperties();
        const auto unsupportedLayers =
            std::ranges::find_if(requiredLayers, [&supportedLayers](const auto& requiredLayer) {
                return std::ranges::none_of(supportedLayers, [requiredLayer](const auto& layerProperty) {
                    return strcmp(layerProperty.layerName, requiredLayer) == 0;
                });
            });

        if (unsupportedLayers != requiredLayers.end())
        {
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayers));
        }


        //-------------------
        // INSTANCE CREATION
        //-------------------
        const vk::InstanceCreateInfo instanceCreateInfo{
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames     = requiredLayers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data(),
        };

        instance = vk::raii::Instance(context, instanceCreateInfo);
    }


    void TempestEngine::setupDebugMessenger()
    {
        if (!enableValidationLayers)
            return;
        constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        };
        constexpr vk::DebugUtilsMessageTypeFlagsEXT messageType{ vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral };

        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{ .messageSeverity = severityFlags,
                                                                   .messageType     = messageType,
                                                                   .pfnUserCallback = &debugCallback };
        debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsCreateInfo);
    }


    void TempestEngine::createSurface()
    {
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &surface))
        {
            throw std::runtime_error("There was an error creating vulkan surface.");
        }
        this->surface = vk::raii::SurfaceKHR(instance, surface);
    }


    void TempestEngine::pickPhysicalDevice()
    {
        /// Properties represent the details about the device like name, vulkan version support etc.
        /// Features represent the feature-set supported by the device like certain shader support
        const auto physicalDevices = vk::raii::PhysicalDevices(instance);

        if (physicalDevices.empty())
            throw std::runtime_error("No Graphics card supporting vulkan found!");

        std::multimap<uint32_t, vk::raii::PhysicalDevice> gpus;

        for (const auto& pd : physicalDevices)
        {
            const auto properties = pd.getProperties();
            uint32_t score        = 0;

            // Support at least vulkan 1.3
            if (properties.apiVersion < vk::ApiVersion13)
                continue;

            // Must have graphics queue
            auto queueFamilies   = pd.getQueueFamilyProperties2();
            bool supportGraphics = std::ranges::any_of(queueFamilies, [](const auto& queueFamily) {
                return !!(queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
            });
            if (!supportGraphics)
                continue;

            // Must have required extensions
            std::vector requiredExtensions = { vk::KHRSwapchainExtensionName };
            const auto supportedExtensions = pd.enumerateDeviceExtensionProperties();
            bool supportsAllRequiredExtensions =
                std::ranges::all_of(requiredExtensions, [&supportedExtensions](const auto& requiredExtension) {
                    return std::ranges::any_of(
                        supportedExtensions, [requiredExtension](const auto& supportedExtension) {
                            return std::strcmp(supportedExtension.extensionName, requiredExtension);
                        });
                });
            if (!supportsAllRequiredExtensions)
                continue;

            // Must have some features like dynamic render
            const auto devFeatures = pd.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                                                     vk::PhysicalDeviceVulkan13Features,
                                                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
            const auto requiredFeatures = devFeatures.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
                devFeatures.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                devFeatures.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                devFeatures.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
                devFeatures.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
            if (!requiredFeatures)
                continue;


            // Discrete GPU is preferred
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                score += 1000;


            // Sort by highest supported texture dimension
            score += properties.limits.maxImageDimension3D;

            gpus.insert(std::make_pair(score, pd));
        }

        // If there is a gpu and score is greater than 0 for the last GPU (which gives the GPU with the highest score)
        // use that gpu
        if (!gpus.empty() && gpus.rbegin()->first > 0)
            physicalDevice = gpus.rbegin()->second;
        else
            throw std::runtime_error("No appropriate graphics card found!");
    }


    void TempestEngine::createLogicalDevice()
    {
        /// Request a device with graphics family queue
        /// and vulkan 1.1 shaderDrawparams, dynamic rendering and extended dynamic state
        /// Enable swap chain extension
        auto queueProperties = physicalDevice.getQueueFamilyProperties();

        // Iterate through each queue and find the first one that supports both graphics and presentation
        for (uint32_t qFamilyIndex = 0; qFamilyIndex < queueProperties.size(); ++qFamilyIndex)
        {
            if (queueProperties[qFamilyIndex].queueFlags & vk::QueueFlagBits::eGraphics &&
                physicalDevice.getSurfaceSupportKHR(qFamilyIndex, *surface))
            {
                queueIndex = qFamilyIndex;
                break;
            }
        }
        if (queueIndex == ~0)
            throw std::runtime_error("Couldn't find a queue supporting both graphics and presentation");

        // We need to specify a priority even if we have only 1 queue
        float queuePriority = 0.5f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = queueIndex,
                                                         .queueCount       = 1,
                                                         .pQueuePriorities = &queuePriority };


        /// Vulkan Feature request chain
        /// Combine all the features required into a single struct without using pNext
        vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                           vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
            featureChain = {
                { .features = { .samplerAnisotropy = true } },          // Physical Device Features
                { .shaderDrawParameters = true },                       // Vulkan 1.1 features
                { .synchronization2 = true, .dynamicRendering = true }, // Vulkan 1.3 features
                { .extendedDynamicState = true }                        // Dynamic state
            };

        std::vector requiredDeviceExtensions{ vk::KHRSwapchainExtensionName };
        vk::DeviceCreateInfo deviceCreateInfo{ .pNext                = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                               .queueCreateInfoCount = 1,
                                               .pQueueCreateInfos    = &deviceQueueCreateInfo,
                                               .enabledExtensionCount =
                                                   static_cast<uint32_t>(requiredDeviceExtensions.size()),
                                               .ppEnabledExtensionNames = requiredDeviceExtensions.data() };

        device = vk::raii::Device(physicalDevice, deviceCreateInfo);

        // Queue is automatically created with logical device
        graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
    }


    void TempestEngine::createSwapChain()
    {
        const vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        swapChainExtent                                      = chooseSwapExtent(surfaceCapabilities);
        const uint32_t minImageCount                         = chooseMinImageCount(surfaceCapabilities);

        const auto availableFormats      = physicalDevice.getSurfaceFormatsKHR(*surface);
        swapChainSurfaceFormat           = chooseSurfaceFormat(availableFormats);
        const auto availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

        const vk::SwapchainCreateInfoKHR swapChainCI{
            .surface          = surface,
            .minImageCount    = minImageCount,
            .imageFormat      = swapChainSurfaceFormat.format,
            .imageColorSpace  = swapChainSurfaceFormat.colorSpace,
            .imageExtent      = swapChainExtent,
            .imageArrayLayers = 1, // Always 1 unless for stereoscopic 3D
            // The image usage, for intermediate ops use ::eTransferDst
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            // Ownership must be exclusively transferred to other queue
            // eConcurrent: Shared by multiple queue without exclusive ownership transfer
            .imageSharingMode = vk::SharingMode::eExclusive,
            // SupportTransforms from capabilities. To specify no transform provide currentTransform.
            .preTransform = surfaceCapabilities.currentTransform,
            // Whether to use alpha channel for blending with other windows, almost always false
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode    = choosePresentationMode(availablePresentModes),
            .clipped        = true, // Clip obscured pixels
            // For swap chain recreation
            .oldSwapchain = nullptr

        };
        swapChain       = vk::raii::SwapchainKHR(device, swapChainCI);
        swapChainImages = swapChain.getImages();
    }


    void TempestEngine::createImageViews()
    {
        assert(swapChainImageViews.empty());
        swapChainImageViews.reserve(swapChainImages.size());
        for (const auto& image : swapChainImages)
        {
            swapChainImageViews.emplace_back(createImageView(image, swapChainSurfaceFormat.format));
        }
    }


    void TempestEngine::createDescriptorSetLayout()
    {

        std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
            { {
                  .binding         = 0,
                  .descriptorType  = vk::DescriptorType::eUniformBuffer,
                  .descriptorCount = 1,
                  .stageFlags      = vk::ShaderStageFlagBits::eVertex,
              },
              { .binding         = 1,
                .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags      = vk::ShaderStageFlagBits::eFragment } }
        };
        const vk::DescriptorSetLayoutCreateInfo layoutInfo{ .bindingCount = static_cast<uint32_t>(bindings.size()),
                                                            .pBindings    = bindings.data() };
        descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
    }


    void TempestEngine::createGraphicsPipeline()
    {
        const auto shader = createShaderModule(readFile("shaders/slang.spv"));
        /// Note: pSpecializationInfo can be used to specify shader constants.
        const vk::PipelineShaderStageCreateInfo vertexShaderCreateInfo{ .stage  = vk::ShaderStageFlagBits::eVertex,
                                                                        .module = shader,
                                                                        .pName  = "vertMain" };
        const vk::PipelineShaderStageCreateInfo fragmentShaderCreateInfo{ .stage  = vk::ShaderStageFlagBits::eFragment,
                                                                          .module = shader,
                                                                          .pName  = "fragMain" };

        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

        /// States like viewport dimensions, line width, and blend constants can be changed
        /// without recreating the graphics pipeline at draw time, but we need to specify a dynamic state to do so.
        /// By creating a dynamic state, teh configuration of these values will be ignored, requiring those to be
        /// specified at draw time.
        std::vector dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicState{ .dynamicStateCount =
                                                             static_cast<uint32_t>(dynamicStates.size()),
                                                         .pDynamicStates = dynamicStates.data() };

        // Describes teh format of vertex data passed into vertex shader
        // Binding: Describe the spacing between data and whether they are per vertex or per instance
        // Attribute Description: Type of attributes passed to the vertex, with the binding and offset
        auto bindingDescription   = Vertex::getBindingDescription();
        auto attributeDescription = Vertex::getAttributeDescription();
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .vertexBindingDescriptionCount   = static_cast<uint32_t>(1),
            .pVertexBindingDescriptions      = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()),
            .pVertexAttributeDescriptions    = attributeDescription.data()
        };

        // Topology or primitive types(TriangleList, Fan, Line, Point etc.)
        // primitiveRestartEnable: Breaks up lines and tris using special index of 0xffff, or 0xffffffff
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = vk::PrimitiveTopology::eTriangleList };

        // Create a viewport with the swap chain dimensions
        // Viewport describe the transformation from the image(swap chain) to framebuffer
        vk::Viewport viewport{ .x        = 0.0f,
                               .y        = 0.0f,
                               .width    = static_cast<float>(swapChainExtent.width),
                               .height   = static_cast<float>(swapChainExtent.height),
                               .minDepth = 0.0f,
                               .maxDepth = 1.0f };

        // scissor defined the region of pixels to store(filtering)
        vk::Rect2D scissor{ .offset = vk::Offset2D{ .x = 0, .y = 0 }, .extent = swapChainExtent };

        vk::PipelineViewportStateCreateInfo viewportState{
            .viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor
        };


        // Rasterizer
        vk::PipelineRasterizationStateCreateInfo rasterizer{
            // If set to true, then fragments beyond far and near planes are clamped, and not discarded
            // useful for shadow maps (requires GPU feature)
            .depthClampEnable = vk::False,
            // If set to true, not geometry pass through the rasterizer, disables all output to framebuffer
            .rasterizerDiscardEnable = vk::False,
            .polygonMode             = vk::PolygonMode::eFill,      // Fill vs Wireframe vs Dots
            .cullMode                = vk::CullModeFlagBits::eBack, // Back face culling,
            // Winding direction (since we are using -y) to draw the triangles in the opposite direction
            .frontFace       = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = vk::False, // Bias the depth value based on slope(useful for shadow maps)
            .lineWidth       = 1.0f,      // Lines thicker than 1.0f require wideLines GPU feature
        };

        // Multisampling
        // Disabled for now.
        vk::PipelineMultisampleStateCreateInfo multisampling{ .rasterizationSamples = vk::SampleCountFlagBits::e1,
                                                              .sampleShadingEnable  = vk::False };

        // Depth and stencil tests
        // Unused right now
        // vk::PipelineDepthStencilStateCreateInfo depthStencilTests{};

        // Color blending
        // Color blending per attached framebuffer
        // Alpha blending(alpha * c1 + (1 - alpha) * c1)
        vk::PipelineColorBlendAttachmentState colorAttachmentState{
            .blendEnable         = vk::False,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp        = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp        = vk::BlendOp::eAdd,
            // Determines which components will be affected
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        // Global color blending vk::PipelineColorBlendStateCreateInfo
        vk::PipelineColorBlendStateCreateInfo colorBlending{
            .logicOpEnable   = vk::False, // Enabling this blending will turn off first blending
            .logicOp         = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments    = &colorAttachmentState
        };

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ .setLayoutCount         = 1,
                                                         .pSetLayouts            = &*descriptorSetLayout,
                                                         .pushConstantRangeCount = 0 };
        pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

        // Dynamic rendering
        // Create the pipeline with graphics and rendering pipelines
        vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
            { .stageCount          = 2,
              .pStages             = shaderStages,
              .pVertexInputState   = &vertexInputInfo,
              .pInputAssemblyState = &inputAssembly,
              .pViewportState      = &viewportState,
              .pRasterizationState = &rasterizer,
              .pMultisampleState   = &multisampling,
              .pColorBlendState    = &colorBlending,
              .pDynamicState       = &dynamicState,
              .layout              = pipelineLayout,
              .renderPass          = nullptr },

            { .colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format }
        };
        // BasePipelineHandle and BasePipelineIndex -> used for inheriting pipelines

        graphicsPipeline =
            vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

        if (graphicsPipeline == nullptr)
        {
            throw std::runtime_error("There was an error creating graphics pipeline");
        }
    }


    void TempestEngine::createVertexBuffer()
    {
        // Since the memory used by gpu for fast transfer are not host accessible we need
        // to create a staging buffer(HOST_VISIBLE) and then transfer the data into the fast DeviceLocal Memory.
        const vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        //------------- STAGING BUFFER ------------------
        // The driver may not copy the memory immediately due to caching.
        // SOL 1: use vk::MemoryPropertyFlagBits::eHostCoherent(Used here)
        // SOL 2: use vk::raii::Device::flushMappedMemoryRanges after writing to mapped memory
        //        vk::raii::Device::invalidateMappedMemoryRanges before reading from mapped memory.
        auto [stagingBuffer, stagingBufferMemory] =
            createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
        memcpy(dataStaging, vertices.data(), bufferSize);
        stagingBufferMemory.unmapMemory();


        //------------- GPU BUFFER ------------------
        std::tie(vertexBuffer, vertexBufferMemory) =
            createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                         vk::MemoryPropertyFlagBits::eDeviceLocal);
        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    }


    void TempestEngine::createIndexBuffer()
    {
        constexpr vk::DeviceSize bufferSize = sizeof(indices) * sizeof(indices[0]);
        auto [stagingBuffer, stagingBufferMemory] =
            createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* data = stagingBufferMemory.mapMemory(0, bufferSize);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        stagingBufferMemory.unmapMemory();

        std::tie(indexBuffer, indexBufferMemory) =
            createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                         vk::MemoryPropertyFlagBits::eDeviceLocal);

        copyBuffer(stagingBuffer, indexBuffer, bufferSize);
    }


    void TempestEngine::createUniformBuffers()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            constexpr vk::DeviceSize size = sizeof(UniformBufferObject);
            auto [buffer, bufferMemory] =
                createBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            uniformBuffers.emplace_back(std::move(buffer));
            uniformBuffersMemory.emplace_back(std::move(bufferMemory));
            // Persistent mapping since we are updating buffer every frame, it is better to persistent mapping.
            uniformBuffersMapped.emplace_back(uniformBuffersMemory.back().mapMemory(0, size));
        }
    }


    void TempestEngine::createDescriptorPool()
    {
        // Specify the type of descriptor and their number
        std::array<vk::DescriptorPoolSize, 2> poolSizes{
            { { .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = MAX_FRAMES_IN_FLIGHT },
              { .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = MAX_FRAMES_IN_FLIGHT } }
        };
        // eFreeDescriptorSet frees the descriptor on destruction as this property needs to be set manually.
        const vk::DescriptorPoolCreateInfo poolInfo{ .flags   = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                                     .maxSets = MAX_FRAMES_IN_FLIGHT,
                                                     .poolSizeCount = poolSizes.size(),
                                                     .pPoolSizes    = poolSizes.data() };
        descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
    }


    void TempestEngine::createDescriptorSets()
    {
        // We need to specify the descriptor pool to allocate from as well as the number of descriptor set to allocate
        // and the descriptor set layout they are based on.
        std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
        const vk::DescriptorSetAllocateInfo allocInfo{ .descriptorPool     = descriptorPool,
                                                       .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                                                       .pSetLayouts        = layouts.data() };

        // Create a descriptor set per frame in flight
        // Allocate the descriptor
        descriptorSets = device.allocateDescriptorSets(allocInfo);
        // Populate it with data
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            // For overwriting the whole buffer we can use vk::WholeBuffer for range.
            vk::DescriptorBufferInfo bufferInfo{ .buffer = uniformBuffers[i],
                                                 .offset = 0,
                                                 .range  = sizeof(UniformBufferObject) };
            vk::DescriptorImageInfo imageInfo{ .sampler     = textureSampler,
                                               .imageView   = textureImageView,
                                               .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };
            // The below struct specifies configuration for descriptor set updates.
            std::array<vk::WriteDescriptorSet, 2> descriptorWrites{
                { { // Which descriptor set to update and bind
                    .dstSet          = descriptorSets[i],
                    .dstBinding      = 0,
                    .dstArrayElement = 0, // Can be array when specifying array of DescriptorSet(s)
                    .descriptorCount = 1,
                    .descriptorType  = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo     = &bufferInfo },
                  { .dstSet          = descriptorSets[i],
                    .dstBinding      = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo      = &imageInfo } }
            };
            // pImageInfo and pTexelBufferInfo can be used to bind image data and buffer views respectively
            // apply the updates
            device.updateDescriptorSets(descriptorWrites, {});
        }
    }


    void TempestEngine::createTextureImage()
    {
        // Read the image
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
        {
            throw std::runtime_error("Failed to load texture image");
        }
        constexpr auto properties =
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        // Move the image to staging buffer
        const auto& [stagingBuffer, stagingBufferMemory] =
            createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, properties);
        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();

        // Free the image buffer
        stbi_image_free(pixels);

        std::tie(textureImage, textureImageMemory) =
            createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                        vk::MemoryPropertyFlagBits::eDeviceLocal);
        vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();
        // Transition the image from undefined to transfer optimal layout
        transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eTransferDstOptimal);
        // Copy the image from staging buffer to textureImage buffer
        copyBufferToImage(commandBuffer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth),
                          static_cast<uint32_t>(texHeight));
        // Transition to a layout optimal for sampling
        transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eTransferDstOptimal,
                              vk::ImageLayout::eShaderReadOnlyOptimal);
        // End the command
        endSingleTimeCommands(std::move(commandBuffer));
    }


    void TempestEngine::createTextureImageView()
    { textureImageView = createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb); }


    vk::raii::ImageView TempestEngine::createImageView(const vk::Image& image, const vk::Format format) const
    {
        const vk::ImageViewCreateInfo viewInfo{ .image            = image,
                                                .viewType         = vk::ImageViewType::e2D,
                                                .format           = format,
                                                .subresourceRange = { .aspectMask     = vk::ImageAspectFlagBits::eColor,
                                                                      .baseMipLevel   = 0,
                                                                      .levelCount     = 1,
                                                                      .baseArrayLayer = 0,
                                                                      .layerCount     = 1 } };

        return vk::raii::ImageView(device, viewInfo);
    }



    vk::raii::ShaderModule TempestEngine::createShaderModule(const std::vector<char>& code) const
    {
        const vk::ShaderModuleCreateInfo shaderCreateInfo{
            .codeSize = code.size(),
            .pCode    = reinterpret_cast<uint32_t const*>(code.data()),
        };
        vk::raii::ShaderModule module{ device, shaderCreateInfo };
        return module;
    };


    void TempestEngine::handleEvents()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    _isRunning = false;
                    break;
                default:
                    break;
                    // SDL_Log("Unhandled event!");
            }
        }
    }

    std::vector<const char*> TempestEngine::getRequiredExtensions() noexcept
    {
        uint32_t extensionCount  = 0;
        const auto sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        std::vector extensions(sdlExtensions, sdlExtensions + extensionCount);
        // Setup up the debug callback extension
        if (enableValidationLayers)
            extensions.push_back(vk::EXTDebugUtilsExtensionName);
        return extensions;
    }


    VKAPI_ATTR vk::Bool32 VKAPI_CALL TempestEngine::debugCallback(
        const vk::DebugUtilsMessageSeverityFlagBitsEXT severity, const vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData)
    {
        if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        {
            SDL_Log(
                "%s",
                std::format("Validation Layer(type: {})\nMessage:\n{}\n", vk::to_string(type), pCallbackData->pMessage)
                    .c_str());
        }
        return vk::False;
    }


    vk::SurfaceFormatKHR TempestEngine::chooseSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& surfaceFormats) const noexcept
    {
        assert(surfaceFormats.size() > 0);
        // Find a suitable format with srgb colorspace, and return the first one if not supported
        const auto formatIt = std::ranges::find_if(surfaceFormats, [](const auto& surfaceFormat) {
            return surfaceFormat.format == vk::Format::eB8G8R8A8Srgb &&
                surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });
        return formatIt != surfaceFormats.end() ? *formatIt : surfaceFormats[0];
    }


    vk::PresentModeKHR TempestEngine::choosePresentationMode(
        const std::vector<vk::PresentModeKHR>& presentModes) const noexcept
    {
        // Ensure that at least Fifo is supported
        assert(std::ranges::any_of(presentModes, [](const auto& presentMode) {
            return presentMode == vk::PresentModeKHR::eFifo;
        }));
        // If mailbox is supported choose that else use Fifo as fallback
        return std::ranges::any_of(presentModes,
                                   [](const auto& presentMode) {
                                       return presentMode == vk::PresentModeKHR::eMailbox;
                                   })
            ? vk::PresentModeKHR::eMailbox
            : vk::PresentModeKHR::eFifo;
    }


    vk::Extent2D TempestEngine::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept
    {
        // Choose the resolution of the swap chain
        // If the current extend is not the max value for uint32_t(set by some window managers)
        // then use the current extent, otherwise use the SDL buffer's width and height
        // This is necessary since some displays can have a larger buffer like apple's retina display w * dpi
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        // We need to clamp the width and height with the valid ranges of vulkan surface
        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        return vk::Extent2D{
            .width = std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height =
                std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
        };
    }


    uint32_t TempestEngine::chooseMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept
    {
        // Choose an appropriate image count in the range between minImageCount < n <= maxImageCount/3
        auto minImageCount = std::max(3u, capabilities.minImageCount); // Choose between min and images max
        if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < minImageCount)
        {
            minImageCount = capabilities.maxImageCount;
        }
        return minImageCount;
    }


    void TempestEngine::createCommandPool() noexcept
    {
        // For allocation command buffer we need a command pool first
        const vk::CommandPoolCreateInfo commandPoolCreateInfo{
            // Allow individual re-recording
            // Transient-> Allows command buffers to be rerecorded with new commands often.
            .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueIndex
        };

        // Command buffers execute by submitting them to ONE of the device queues, like graphics or presentation,
        // and each queue type require a different command buffer
        commandPool = vk::raii::CommandPool(device, commandPoolCreateInfo);
    }


    void TempestEngine::createCommandBuffer() noexcept
    {
        const vk::CommandBufferAllocateInfo commandBufferInfo{
            .commandPool = commandPool,
            // Can be submitted to a queue for execution, but cannot be called from other Command Buffers
            // Secondary-> Cannot be submitted, but can be called from Primary Command Buffers
            .level = vk::CommandBufferLevel::ePrimary,

            .commandBufferCount = MAX_FRAMES_IN_FLIGHT
        };

        // 1. Allocates a std::vector of buffers
        // 2. Copy ctor is deleted
        commandBuffers = std::move(vk::raii::CommandBuffers(device, commandBufferInfo));
    }


    void TempestEngine::createSyncObjects() noexcept
    {
        assert(presentFinishedSemaphores.empty() && renderFinishedSemaphores.empty() && drawFences.empty());
        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            presentFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
            drawFences.emplace_back(device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
        }
    }


    void TempestEngine::createTextureSampler() noexcept
    {
        const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        vk::SamplerCreateInfo samplerInfo{ .magFilter    = vk::Filter::eLinear,
                                           .minFilter    = vk::Filter::eLinear,
                                           .mipmapMode   = vk::SamplerMipmapMode::eLinear,
                                           .addressModeU = vk::SamplerAddressMode::eRepeat,
                                           .addressModeV = vk::SamplerAddressMode::eRepeat,
                                           .addressModeW = vk::SamplerAddressMode::eRepeat,
                                           // Anisotropic filtering
                                           .anisotropyEnable = vk::True,
                                           .maxAnisotropy    = properties.limits.maxSamplerAnisotropy,
                                           // If comparison is enabled then texels will be compared to a value.
                                           .compareEnable = vk::True,
                                           .compareOp     = vk::CompareOp::eAlways };
        // What color to use beyond the clamp. Cannot specify an arbitrary color.
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
        // Unnormalized coordinates allows us to sample beyond the [0,1) range.
        samplerInfo.unnormalizedCoordinates = vk::False;
        // Map mapping
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod     = 0.0f;
        samplerInfo.maxLod     = 0.0f;

        textureSampler = vk::raii::Sampler(device, samplerInfo);
    }


    std::pair<vk::raii::Image, vk::raii::DeviceMemory> TempestEngine::createImage(
        const uint32_t width, const uint32_t height, const vk::Format format, const vk::ImageTiling tiling,
        const vk::ImageUsageFlags usage, const vk::MemoryPropertyFlags properties) const noexcept
    {
        const vk::ImageCreateInfo imageInfo{ .imageType   = vk::ImageType::e2D,
                                             .format      = format,
                                             .extent      = { .width = width, .height = height, .depth = 1 },
                                             .mipLevels   = 1,
                                             .arrayLayers = 1,
                                             .samples     = vk::SampleCountFlagBits::e1,
                                             .tiling      = tiling,
                                             .usage       = usage,
                                             .sharingMode = vk::SharingMode::eExclusive };

        auto image = vk::raii::Image(device, imageInfo);

        const vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
        const vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size,
                                                .memoryTypeIndex =
                                                    findMemoryType(memRequirements.memoryTypeBits, properties) };
        auto imageMemory = vk::raii::DeviceMemory(device, allocInfo);
        image.bindMemory(imageMemory, 0);

        return std::make_pair(std::move(image), std::move(imageMemory));
    }


    void TempestEngine::recordCommandBuffer(const uint32_t imageIndex) noexcept
    {
        // Begin the command recording
        // const vk::CommandBufferBeginInfo beginInfo{};
        commandBuffers[frameIndex].begin({});

        // Transition swap chain image to ImageLayout::eColorAttachmentOptimal
        transitionImageLayout(imageIndex, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
                              {}, // no need to wait for previous operation
                              vk::AccessFlagBits2::eColorAttachmentWrite,
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput);

        constexpr vk::ClearColorValue clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };

        // Dynamic rendering doesn't require a RenderPass but we need to specify the attachment info
        vk::RenderingAttachmentInfo attachmentInfo{
            .imageView   = swapChainImageViews[imageIndex],
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp      = vk::AttachmentLoadOp::eClear,  // What to do to the attachment before rendering
            .storeOp     = vk::AttachmentStoreOp::eStore, // What to do to the attachment after rendering
            .clearValue  = clearColor
        };

        // Create the rendering info
        const vk::RenderingInfo renderingInfo{ .renderArea           = { .offset = { .x = 0, .y = 0 },
                                                                         .extent = swapChainExtent },
                                               .layerCount           = 1,
                                               .colorAttachmentCount = 1,
                                               .pColorAttachments    = &attachmentInfo };

        // Begin Rendering
        commandBuffers[frameIndex].beginRendering(renderingInfo);
        commandBuffers[frameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
        // Bind the vertex buffer
        commandBuffers[frameIndex].bindVertexBuffers(0, *vertexBuffer, { 0 });
        commandBuffers[frameIndex].bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);
        // Set dynamic states
        // NOTE: The negative height is due to the fact the glm has an inverted y axis in the projection matrix.
        commandBuffers[frameIndex].setViewport(0,
                                               vk::Viewport(0.0f, static_cast<float>(swapChainExtent.height),
                                                            static_cast<float>(swapChainExtent.width),
                                                            -static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
        commandBuffers[frameIndex].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

        // Bind the descriptor set for uniforms
        commandBuffers[frameIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0,
                                                      *descriptorSets[frameIndex], nullptr);
        // Draw
        commandBuffers[frameIndex].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        // End rendering
        commandBuffers[frameIndex].endRendering();

        // Transition image layout for presentation
        transitionImageLayout(imageIndex, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                              vk::AccessFlagBits2::eColorAttachmentWrite, {},
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              vk::PipelineStageFlagBits2::eBottomOfPipe);
        commandBuffers[frameIndex].end();
    }


    void TempestEngine::transitionImageLayout(const uint32_t imageIndex, const vk::ImageLayout oldLayout,
                                              const vk::ImageLayout newLayout, const vk::AccessFlags2 srcAccessMask,
                                              const vk::AccessFlags2 dstAccessMask,
                                              const vk::PipelineStageFlags2 srcStageMask,
                                              const vk::PipelineStageFlags2 dstStageMask) noexcept
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask        = srcStageMask,
            .srcAccessMask       = srcAccessMask,
            .dstStageMask        = dstStageMask,
            .dstAccessMask       = dstAccessMask,
            .oldLayout           = oldLayout,
            .newLayout           = newLayout,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .image               = swapChainImages[imageIndex],
            // Since we transition render target, apply no mipmapping and the layer must be 1 unless for stereoscopic 3D
            .subresourceRange = { .aspectMask     = vk::ImageAspectFlagBits::eColor,
                                  .baseMipLevel   = 0,
                                  .levelCount     = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount     = 1 }
        };

        const vk::DependencyInfo dependencyInfo = { .dependencyFlags         = {},
                                                    .imageMemoryBarrierCount = 1,
                                                    .pImageMemoryBarriers    = &barrier };

        commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);
    }

    void TempestEngine::transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image,
                                              const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout) noexcept
    {
        // To transition an image layout, we need to create a pipeline barrier
        // This can be used for transitioning queue families when vk::SharingMode::eExclusive is used.
        vk::ImageMemoryBarrier barrier{
            .oldLayout           = oldLayout,
            .newLayout           = newLayout,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .image               = image,
            // Specify the affect part of the image
            .subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1 }
        };

        // SrcStage -> PipelineBarrier -> DstStage

        vk::PipelineStageFlags sourceStage, destinationStage;
        // We need to handle two transitions Undefined -> TransferOpt and TransferOpt -> ShaderOpt
        // When transitioning from Undefined to ShaderOptimal
        // we are going through TopOfPipe -> Transfer -> FragmentShader
        // Moreover, transfer is a pseudo stage with compute and graphics pipeline.
        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = {};


            sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
                 newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage      = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else
        {
            throw std::runtime_error("Unsupported layout transition");
        }
        commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);

        // commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {},{}, barrier);
    }

    void TempestEngine::copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer,
                                          vk::raii::Image& image, const uint32_t width, const uint32_t height) noexcept
    {
        // We need to specify which part of the buffer will be copied to which part of the image.
        const vk::BufferImageCopy region{ .bufferOffset = 0,
                                          // Specify that our image is tightly packed
                                          .bufferRowLength   = 0,
                                          .bufferImageHeight = 0,
                                          .imageSubresource  = { .aspectMask     = vk::ImageAspectFlagBits::eColor,
                                                                 .mipLevel       = 0,
                                                                 .baseArrayLayer = 1,
                                                                 .layerCount     = 1 },
                                          .imageOffset       = { .x = 0, .y = 0, .z = 0 },
                                          .imageExtent       = { .width = width, .height = height, .depth = 1 } };
        // Layout indicate the layout the image is currently using.
        // Copy to many images from the buffer is possible.
        commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
    }


    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> TempestEngine::createBuffer(
        const vk::DeviceSize size, const vk::BufferUsageFlags usageFlags,
        const vk::MemoryPropertyFlags properties) const noexcept
    {
        const vk::BufferCreateInfo bufferInfo{ .size        = size,
                                               .usage       = usageFlags,
                                               .sharingMode = vk::SharingMode::eExclusive };
        auto buffer                                  = vk::raii::Buffer(device, bufferInfo);
        const vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
        const vk::MemoryAllocateInfo allocateInfo{ .allocationSize = memRequirements.size,
                                                   .memoryTypeIndex =
                                                       findMemoryType(memRequirements.memoryTypeBits, properties) };
        auto devMemory = vk::raii::DeviceMemory(device, allocateInfo);
        buffer.bindMemory(*devMemory, 0);
        return { std::move(buffer), std::move(devMemory) };
    }


    void TempestEngine::copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer,
                                   const vk::DeviceSize bufferSize) const noexcept
    {
        auto copyCommandBuffer = beginSingleTimeCommands();
        copyCommandBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, bufferSize));
        endSingleTimeCommands(std::move(copyCommandBuffer));
    }


    void TempestEngine::updateUniformBuffer(const uint32_t currentImageIdx) noexcept
    {
        static auto startTime  = std::chrono::high_resolution_clock::now();
        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj  = glm::perspective(
            glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height),
            0.1f, 10.0f);
        memcpy(uniformBuffersMapped[currentImageIdx], &ubo, sizeof(ubo));
    }


    vk::raii::CommandBuffer TempestEngine::beginSingleTimeCommands() const noexcept
    {
        // Create a command buffer for one time submission
        const vk::CommandBufferAllocateInfo allocateInfo{ .commandPool        = commandPool,
                                                          .level              = vk::CommandBufferLevel::ePrimary,
                                                          .commandBufferCount = 1 };
        vk::raii::CommandBuffer commandBuffer = std::move(vk::raii::CommandBuffers(device, allocateInfo).front());
        constexpr vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
        // Begin recording
        commandBuffer.begin(beginInfo);

        // Return the command buffer.
        return commandBuffer;
    }


    void TempestEngine::endSingleTimeCommands(const vk::raii::CommandBuffer&& commandBuffer) const noexcept
    {
        // End the command buffer
        commandBuffer.end();

        // Submit the command
        const vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
        graphicsQueue.submit(submitInfo);
        graphicsQueue.waitIdle();
    }


    uint32_t TempestEngine::findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const
    {
        // Graphics cards provide different memory types, so we need to query and choose one
        // that best fits our requirements
        const vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
        // Has memoryTypes and memoryHeaps

        // Return the index of memory type if it matches the properties we need.
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find a suitable memory type.");
    }
} // namespace tempest
