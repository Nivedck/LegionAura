#pragma once
#include "../AccentProvider.h"

class KdeProvider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
