#pragma once

#include "Component.hpp"
#include <typeindex>
#include <unordered_map>

using ComponentType = std::uint8_t;

namespace ecs {

  class ComponentManager {
    public:


    private:
      std::unordered_map<std::type_index, ComponentType> _componentTypes;
      std::unordered_map<ComponentType, std:>
  };

}