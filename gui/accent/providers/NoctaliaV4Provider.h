#pragma once
#include "../AccentProvider.h"

class NoctaliaV4Provider : public AccentProvider {
public:
    AccentColorResult getAccentColor() const override;
};
