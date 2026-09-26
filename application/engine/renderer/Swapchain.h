#pragma once
/**
 * @file Swapchain.h
 * @author Alan Abraham P Kochumon
 * @date Created on: September 26, 2026
 *
 * @brief Swapchain abstraction.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include "../platform/TempestSurface.h"
#include "../platform/Window.h"

#include "PresentMode.h"

#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class RenderDevice;
    class SwapChain
    {
    public:
        [[nodiscard]] const std::vector<vk::raii::Image>& getImages() const { return _images; }
        [[nodiscard]] const std::vector<vk::raii::ImageView>& getImageViews() const { return _imageViews; }
        [[nodiscard]] const vk::raii::SwapchainKHR& getBaseSwapChain() const { return _swapChainInstance; }
        [[nodiscard]] const vk::Extent2D& getExtent() const { return _extent; }
        [[nodiscard]] const vk::SurfaceFormatKHR& getFormat() const { return _format; }

        void createSwapChain(PresentMode presentMode) noexcept;

        [[nodiscard]] std::vector<PresentMode> querySupportedPresentModes() const noexcept;

    private:
        void createImageViews() noexcept;

        [[nodiscard]] vk::Extent2D chooseSwapChainExtent() const noexcept;
        [[nodiscard]] uint32_t SwapChain::chooseMinImageCount() const noexcept;
        [[nodiscard]] PresentMode choosePresentationMode(PresentMode presentMode) const noexcept;
        [[nodiscard]] vk::SurfaceFormatKHR chooseSurfaceFormat() const noexcept;
        [[nodiscard]] vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format,
                                                          vk::ImageAspectFlags aspectFlags,
                                                          uint32_t mipLevel) const noexcept;

    private:
        std::vector<vk::Image> _images;
        std::vector<vk::raii::ImageView> _imageViews;
        vk::raii::SwapchainKHR _swapChainInstance{ nullptr };
        vk::Extent2D _extent{};
        vk::SurfaceFormatKHR _format;
        vk::PresentModeKHR _selectedPresentMode{};
        vk::SurfaceCapabilitiesKHR& _capabilities;
        RenderDevice& _device;
        platform::TempestSurface& _surface;
        platform::TempestWindow& _window;
    };
} // namespace tempest::renderer