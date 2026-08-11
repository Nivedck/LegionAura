#include "KdeProvider.h"

AccentColorResult KdeProvider::getAccentColor() const {
    return AccentColorResult{false, "", "Kde", ""};
}
