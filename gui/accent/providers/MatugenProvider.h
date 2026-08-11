#pragma once
#include "../AccentProvider.h"

class MatugenProvider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
