#include "AccentManager.h"

#include "providers/NoctaliaV5Provider.h"
#include "providers/NoctaliaV4Provider.h"
#include "providers/MatugenProvider.h"
#include "providers/Pywal16Provider.h"
#include "providers/PywalProvider.h"
#include "providers/WallustProvider.h"
#include "providers/WpgtkProvider.h"
#include "providers/KdeProvider.h"
#include "providers/GnomeProvider.h"

AccentManager::AccentManager()
{
    // Register providers in priority order
    m_providers.push_back(std::make_unique<NoctaliaV5Provider>());
    m_providers.push_back(std::make_unique<NoctaliaV4Provider>());
    m_providers.push_back(std::make_unique<MatugenProvider>());
    m_providers.push_back(std::make_unique<Pywal16Provider>());
    m_providers.push_back(std::make_unique<PywalProvider>());
    m_providers.push_back(std::make_unique<WallustProvider>());
    m_providers.push_back(std::make_unique<WpgtkProvider>());
    m_providers.push_back(std::make_unique<KdeProvider>());
    m_providers.push_back(std::make_unique<GnomeProvider>());
}

AccentColorResult AccentManager::detectAccentColor() const
{
    for (const auto& provider : m_providers) {
        AccentColorResult res = provider->getAccentColor();
        if (res.found && !res.colorHex.isEmpty()) {
            // Ensure format is #RRGGBB
            if (!res.colorHex.startsWith('#')) {
                res.colorHex = "#" + res.colorHex;
            }
            return res;
        }
    }
    
    // Return empty result if not found
    return AccentColorResult{false, "", "", ""};
}
