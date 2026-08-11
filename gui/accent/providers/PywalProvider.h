#pragma once
#include "../AccentProvider.h"

class PywalProvider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
