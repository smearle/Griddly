#include "VulkanDevice.hpp"
#include <spdlog/spdlog.h>
#include "VulkanInitializers.hpp"
#include "VulkanInstance.hpp"
#include "VulkanPhysicalDeviceInfo.hpp"
#include "VulkanQueueFamilyIndices.hpp"
#include "VulkanUtil.hpp"

namespace vk {

VulkanDevice::VulkanDevice(VulkanInstance& vulkanInstance) : vulkanInstance_(vulkanInstance) {
}

VulkanDevice::~VulkanDevice() {
  if (device_ != VK_NULL_HANDLE) {
    if (commandPool_ != VK_NULL_HANDLE) {
      vkDestroyCommandPool(device_, commandPool_, NULL);
    }
    vkDestroyDevice(device_, NULL);
  }
}

void VulkanDevice::initDevice(bool useGPU) {
  std::vector<VkPhysicalDevice> physicalDevices = getAvailablePhysicalDevices();
  std::vector<VulkanPhysicalDeviceInfo> supportedPhysicalDevices = getSupportedPhysicalDevices(physicalDevices);

  if (supportedPhysicalDevices.size() > 0) {
    auto physicalDeviceInfo = selectPhysicalDevice(useGPU, supportedPhysicalDevices);

    // This should never be hit if the previous check succeeds, but is here for completeness
    if (physicalDeviceInfo == supportedPhysicalDevices.end()) {
      spdlog::error("Could not select a physical device, isGpu={0}", useGPU);
      return;
    }

    auto graphicsQueueFamilyIndex = physicalDeviceInfo->queueFamilyIndices.graphicsIndices;
    auto computeQueueFamilyIndex = physicalDeviceInfo->queueFamilyIndices.computeIndices;

    auto deviceQueueCreateInfo = vk::initializers::deviceQueueCreateInfo(graphicsQueueFamilyIndex, 1.0f);
    auto deviceCreateInfo = vk::initializers::deviceCreateInfo(deviceQueueCreateInfo);

    vk_check(vkCreateDevice(physicalDeviceInfo->physicalDevice, &deviceCreateInfo, NULL, &device_));
    vkGetDeviceQueue(device_, computeQueueFamilyIndex, 0, &computeQueue_);

    auto commandPoolCreateInfo = vk::initializers::commandPoolCreateInfo(computeQueueFamilyIndex);
    vk_check(vkCreateCommandPool(device_, &commandPoolCreateInfo, nullptr, &commandPool_));

  } else {
    spdlog::error("No devices supporting vulkan present for rendering.");
  }
}

std::vector<VulkanPhysicalDeviceInfo>::iterator VulkanDevice::selectPhysicalDevice(bool useGpu, std::vector<VulkanPhysicalDeviceInfo>& supportedDevices) {
  for (auto it = supportedDevices.begin(); it != supportedDevices.end(); ++it) {
    if (useGpu == it->isGpu) {
      return it;
    }
  }
  return supportedDevices.end();
}

std::vector<VulkanPhysicalDeviceInfo> VulkanDevice::getSupportedPhysicalDevices(std::vector<VkPhysicalDevice>& physicalDevices) {
  std::vector<VulkanPhysicalDeviceInfo> supportedPhysicalDevices;
  for (auto& physicalDevice : physicalDevices) {
    VulkanPhysicalDeviceInfo physicalDeviceInfo = getPhysicalDeviceInfo(physicalDevice);
    spdlog::info("Device {0}, isGpu {1}, isSupported {2}.", physicalDeviceInfo.deviceName, physicalDeviceInfo.isGpu, physicalDeviceInfo.isSupported);

    if (physicalDeviceInfo.isSupported) {
      supportedPhysicalDevices.push_back(physicalDeviceInfo);
    }
  }

  return supportedPhysicalDevices;
}

std::vector<VkPhysicalDevice> VulkanDevice::getAvailablePhysicalDevices() {
  uint32_t deviceCount = 0;
  vk_check(vkEnumeratePhysicalDevices(vulkanInstance_.getInstance(), &deviceCount, nullptr));
  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vk_check(vkEnumeratePhysicalDevices(vulkanInstance_.getInstance(), &deviceCount, physicalDevices.data()));

  return physicalDevices;
}

VulkanPhysicalDeviceInfo VulkanDevice::getPhysicalDeviceInfo(VkPhysicalDevice& physicalDevice) {
  VulkanQueueFamilyIndices queueFamilyIndices;
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

  auto deviceName = deviceProperties.deviceName;

  spdlog::info("Device found {0}, checking for Vulkan support.", deviceName);

  bool isGpu = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  bool isSupported = hasQueueFamilySupport(physicalDevice, queueFamilyIndices);

  return {
      physicalDevice,
      std::string(deviceName),
      isGpu,
      isSupported,
      queueFamilyIndices};
}

bool VulkanDevice::hasQueueFamilySupport(VkPhysicalDevice& device, VulkanQueueFamilyIndices& queueFamilyIndices) {
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

  uint32_t i = 0;
  for (auto& queueFamily : queueFamilyProperties) {
    if (queueFamily.queueCount > 0) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        queueFamilyIndices.graphicsIndices = i;
      }

      if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
        queueFamilyIndices.computeIndices = i;
      }
    }

    if (queueFamilyIndices.graphicsIndices < UINT32_MAX && queueFamilyIndices.computeIndices < UINT32_MAX) {
      return true;
    }
    i++;
  }
  return false;
}
}  // namespace vk