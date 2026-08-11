#pragma once
#include "../AccentProvider.h"

class WallustProvider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
