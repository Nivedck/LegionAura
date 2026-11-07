#pragma once

#include "AccentProvider.h"

class AccentManager {
public:
    AccentColorResult detectAccentColor() const;
    AccentColorResult detectAccentColor(const QString& wallpaperPath,
                                        const QString& source = "Manual selection") const;
};
