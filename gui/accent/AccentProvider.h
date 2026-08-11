#pragma once

#include <QString>

struct AccentColorResult {
    bool found = false;
    QString colorHex; // Normalized #RRGGBB
    QString providerName;
    QString sourcePath;
};

class AccentProvider {
public:
    virtual ~AccentProvider() = default;
    
    // Attempt to detect and extract the accent color from this provider's ecosystem.
    virtual AccentColorResult getAccentColor() const = 0;
};
