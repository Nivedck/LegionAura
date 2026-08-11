#pragma once
#include "../AccentProvider.h"

class NoctaliaV5Provider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
