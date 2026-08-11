#include "GnomeProvider.h"

AccentColorResult GnomeProvider::getAccentColor() const {
    return AccentColorResult{false, "", "Gnome", ""};
}
