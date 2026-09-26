/**
 * @file Swapchain.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: September 26, 2026
 *
 * @brief Implementation of member functions declared in Swapchain.h
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "Swapchain.h"

#include "GraphicsContext.h"
#include "RenderDevice.h"

namespace tempest::renderer
{
    SwapChain::SwapChain(RenderDevice& device, platform::TempestSurface& surface, platform::TempestWindow& window,
                         const PresentMode presentMode)

        : _selectedPresentMode(presentMode), _device(device), _surface(surface), _window(window)
    {
        create(presentMode);
        _capabilities = querySurfaceCapabilities();
    }

    SwapChain::~SwapChain() noexcept { cleanupSwapChain(); }

    void SwapChain::create(const PresentMode presentMode) noexcept { createVulkanSwapChain(presentMode, nullptr); }

    void SwapChain::createVulkanSwapChain(const PresentMode presentMode, const vk::SwapchainKHR* oldSwapChain) noexcept
    {
        const auto& vkSurface        = *_surface.getBaseSurface();
        _extent                      = chooseSwapChainExtent();
        const uint32_t minImageCount = chooseMinImageCount();

        _format                          = chooseSurfaceFormat();
        const auto availablePresentModes = _device.getPhysicalDevice().getSurfacePresentModesKHR(vkSurface);

        const vk::SwapchainCreateInfoKHR swapChainCreateInfo{
            .surface          = _surface.getBaseSurface(),
            .minImageCount    = minImageCount,
            .imageFormat      = _format.format,
            .imageColorSpace  = _format.colorSpace,
            .imageExtent      = _extent,
            .imageArrayLayers = 1, // Always 1 unless for stereoscopic 3D
            // The image usage, for intermediate ops use ::eTransferDst
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            // Ownership must be exclusively transferred to other queue
            // eConcurrent: Shared by multiple queue without exclusive ownership transfer
            .imageSharingMode = vk::SharingMode::eExclusive,
            // SupportTransforms from capabilities. To specify no transform provide currentTransform.
            .preTransform = _capabilities.currentTransform,
            // Whether to use alpha channel for blending with other windows, almost always false
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode    = toVKPresentMode(choosePresentationMode(presentMode)),
            .clipped        = true, // Clip obscured pixels
            // For swap chain recreation
            .oldSwapchain = *oldSwapChain

        };
        _swapChainInstance = vk::raii::SwapchainKHR(_device.getDevice(), swapChainCreateInfo);
        _images            = _swapChainInstance.getImages();
    }

    void SwapChain::cleanupSwapChain() noexcept
    {
        // Rest will be done by vk::raii dtor
        _imageViews.clear();
        _images.clear();
        _swapChainInstance = nullptr;
    }

    std::vector<PresentMode> SwapChain::querySupportedPresentModes() const noexcept
    {
        const auto vkPresentModes = _device.getPhysicalDevice().getSurfacePresentModesKHR(_surface.getBaseSurface());
        std::vector<PresentMode> presentModes;
        for (const auto vkPresentMode : vkPresentModes)
        {
            if (const auto presentMode = fromVKPresentMode(vkPresentMode); presentMode != PresentMode::UNSUPPORTED)
            {
                presentModes.emplace_back(presentMode);
            }
        }
        return presentModes;
    }

    void SwapChain::recreateSwapChain(const PresentMode presentMode) noexcept
    {
        cleanupSwapChain();
        createVulkanSwapChain(presentMode == PresentMode::UNSUPPORTED ? _selectedPresentMode : presentMode,
                              &*_swapChainInstance);
        createImageViews();
        // TODO:
        // createColorResources();
        // createDepthResources();
    }

    void SwapChain::createImageViews() noexcept
    {
        assert(_imageViews.empty() && "SwapChain Image Views are not empty!");
        _imageViews.reserve(_images.size());
        for (const auto& image : _images)
        {
            _imageViews.emplace_back(createImageView(image, _format.format, vk::ImageAspectFlagBits::eColor, 1));
        }
    }

    uint32_t SwapChain::chooseMinImageCount() const noexcept
    {
        // Choose an appropriate image count in the range between minImageCount < n <= maxImageCount/3
        auto minImageCount = std::max(3u, _capabilities.minImageCount); // Choose between min and images max
        if (_capabilities.maxImageCount > 0 && _capabilities.maxImageCount < minImageCount)
        {
            minImageCount = _capabilities.maxImageCount;
        }
        return minImageCount;
    }

    vk::Extent2D SwapChain::chooseSwapChainExtent() const noexcept
    {
        // Choose the resolution of the swap chain
        // If the current extend is not the max value for uint32_t(set by some window managers)
        // then use the current extent, otherwise use the SDL buffer's width and height
        // This is necessary since some displays can have a larger buffer like apple's retina display w * dpi
        if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return _capabilities.currentExtent;

        // We need to clamp the width and height with the valid ranges of vulkan surface
        const auto [width, height] = _window.getWindowExtent();
        return vk::Extent2D{
            .width =
                std::clamp<uint32_t>(width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width),
            .height =
                std::clamp<uint32_t>(height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height),
        };
    }

    PresentMode SwapChain::choosePresentationMode(const PresentMode presentMode) const noexcept
    {
        const auto supportedPresentModes = querySupportedPresentModes();

        // Ensure that at least FIFO(VSYNC) is supported.
        assert(std::ranges::any_of(supportedPresentModes, [](const auto& presentMode) {
            return presentMode == PresentMode::VSYNC;
        }));
        // Choose the selected present mode if available choose VSYNC
        for (const auto supportedPresentMode : supportedPresentModes)
        {
            if (presentMode == supportedPresentMode)
            {
                return presentMode;
            }
        }
        return PresentMode::VSYNC;
    }

    vk::SurfaceFormatKHR SwapChain::chooseSurfaceFormat() const noexcept
    {
        // Query the surface format
        const auto surfaceFormats = _device.getPhysicalDevice().getSurfaceFormatsKHR();
        assert(surfaceFormats.size() > 0);
        // Find a suitable format with srgb colorspace, and return the first one if not supported
        const auto formatIt = std::ranges::find_if(surfaceFormats, [](const auto& surfaceFormat) {
            return surfaceFormat.format == vk::Format::eB8G8R8A8Srgb &&
                surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });
        return formatIt != surfaceFormats.end() ? *formatIt : surfaceFormats[0];
    }


    /// TODO: Migrate to a separate image view class maybe?
    vk::raii::ImageView SwapChain::createImageView(const vk::Image& image, const vk::Format format,
                                                   const vk::ImageAspectFlags aspectFlags,
                                                   const uint32_t mipLevel) const noexcept
    {
        const vk::ImageViewCreateInfo viewInfo{ .image            = image,
                                                .viewType         = vk::ImageViewType::e2D,
                                                .format           = format,
                                                .subresourceRange = { .aspectMask     = aspectFlags,
                                                                      .baseMipLevel   = 0,
                                                                      .levelCount     = mipLevel,
                                                                      .baseArrayLayer = 0,
                                                                      .layerCount     = 1 } };

        return vk::raii::ImageView(_device.getDevice(), viewInfo);
    }


    vk::SurfaceCapabilitiesKHR SwapChain::querySurfaceCapabilities() const noexcept
    { return _device.getPhysicalDevice().getSurfaceCapabilitiesKHR(_surface.getBaseSurface()); }
} // namespace tempest::renderer