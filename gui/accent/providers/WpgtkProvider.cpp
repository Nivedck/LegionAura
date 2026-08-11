#include "WpgtkProvider.h"

AccentColorResult WpgtkProvider::getAccentColor() const {
    return AccentColorResult{false, "", "Wpgtk", ""};
}
