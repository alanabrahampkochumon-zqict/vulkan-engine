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

#include "../platform/Window.h"

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

namespace tempest::renderer
{
    class SwapChain
    {
    public:
        [[nodiscard]] const std::vector<vk::raii::Image>& getImages() const { return _images; }
        [[nodiscard]] const std::vector<vk::raii::ImageView>& getImageViews() const { return _imageViews; }
        [[nodiscard]] const vk::raii::SwapchainKHR& getBaseSwapChain() const { return _swapChainInstance; }
        [[nodiscard]] const vk::Extent2D& getExtent() const { return _extent; }
        [[nodiscard]] const vk::SurfaceFormatKHR& getFormat() const { return _format; }

    private:
        void createSwapChain() noexcept;
        void createImageView() noexcept;

        [[nodiscard]] vk::Extent2D chooseSwapChainExtent(const platform::TempestWindow& window) const noexcept;
        [[nodiscard]] uint32_t SwapChain::chooseMinImageCount() const noexcept;

    private:
        std::vector<vk::raii::Image> _images;
        std::vector<vk::raii::ImageView> _imageViews;
        vk::raii::SwapchainKHR _swapChainInstance{ nullptr };
        vk::Extent2D _extent{};
        vk::SurfaceFormatKHR _format;
        vk::PresentModeKHR _selectedPresentMode{};
        vk::SurfaceCapabilitiesKHR& _capabilities;
    };
} // namespace tempest::renderer