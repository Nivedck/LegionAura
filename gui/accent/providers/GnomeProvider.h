#pragma once
#include "../AccentProvider.h"

class GnomeProvider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
