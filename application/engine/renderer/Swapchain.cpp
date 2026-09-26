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
    void SwapChain::createSwapChain(const RenderDevice& device, const platform::TempestSurface& surface) noexcept
    {
        const auto& vkSurface = *surface.getBaseSurface();
        const vk::SurfaceCapabilitiesKHR surfaceCapabilities =
            device.getPhysicalDevice().getSurfaceCapabilitiesKHR(vkSurface);
        _extent                      = chooseSwapExtent(surfaceCapabilities);
        const uint32_t minImageCount = chooseMinImageCount(surfaceCapabilities);

        const auto availableFormats      = device.getPhysicalDevice().getSurfaceFormatsKHR(vkSurface);
        swapChainSurfaceFormat           = chooseSurfaceFormat(availableFormats);
        const auto availablePresentModes = device.getPhysicalDevice().getSurfacePresentModesKHR(vkSurface);

        const vk::SwapchainCreateInfoKHR swapChainCI{
            .surface          = surface,
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
            .preTransform = surfaceCapabilities.currentTransform,
            // Whether to use alpha channel for blending with other windows, almost always false
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode    = choosePresentationMode(availablePresentModes),
            .clipped        = true, // Clip obscured pixels
            // For swap chain recreation
            .oldSwapchain = nullptr

        };
        _swapChainInstance = vk::raii::SwapchainKHR(device, swapChainCI);
        _images            = _swapChainInstance.getImages();
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


    void SwapChain::createImageView() noexcept
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


    vk::Extent2D SwapChain::chooseSwapChainExtent(const platform::TempestWindow& window) const noexcept
    {
        // Choose the resolution of the swap chain
        // If the current extend is not the max value for uint32_t(set by some window managers)
        // then use the current extent, otherwise use the SDL buffer's width and height
        // This is necessary since some displays can have a larger buffer like apple's retina display w * dpi
        if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return _capabilities.currentExtent;

        // We need to clamp the width and height with the valid ranges of vulkan surface
        const auto [width, height] = window.getWindowExtent();
        return vk::Extent2D{
            .width =
                std::clamp<uint32_t>(width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width),
            .height =
                std::clamp<uint32_t>(height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height),
        };
    }
} // namespace tempest::renderer