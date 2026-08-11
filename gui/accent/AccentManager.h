#pragma once

#include "AccentProvider.h"
#include <vector>
#include <memory>

class AccentManager {
public:
    AccentManager();
    
    // Iterates through all registered providers in priority order
    // and returns the first valid AccentColorResult.
    AccentColorResult detectAccentColor() const;

private:
    std::vector<std::unique_ptr<AccentProvider>> m_providers;
};
