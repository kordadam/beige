#include "Beige.hpp"

#include <iostream>
#include <cstdint>
#include <vector>

#include <vulkan/vulkan.hpp>

Beige::Beige() {
    std::cout << "Beige object has been created!\n";

    uint32_t propertyCount{ 0u };
    vk::enumerateInstanceLayerProperties(&propertyCount, nullptr);
    std::vector<vk::LayerProperties> layerProperties{ propertyCount };
    vk::enumerateInstanceLayerProperties(&propertyCount, layerProperties.data());

    for (const auto& layerProperty : layerProperties) {
        std::cout << layerProperty.layerName << "\n";
    }
}

Beige::~Beige() {
    std::cout << "Beige object has been destroyed!\n";
}
