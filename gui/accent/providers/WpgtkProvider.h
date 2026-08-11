#pragma once
#include "../AccentProvider.h"

class WpgtkProvider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
