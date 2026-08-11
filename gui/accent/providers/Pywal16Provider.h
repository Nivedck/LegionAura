#pragma once
#include "../AccentProvider.h"

class Pywal16Provider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
