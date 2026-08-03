#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// Static variable to track the visibility of the position overlay
static bool g_showPosition = false;

// 1. Manage XPos and YPos display during gameplay
class $modify(PosPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_posLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Initialize label with placeholder text
        auto label = CCLabelBMFont::create("XPos: 0.0\nYPos: 0.0", "bigFont.fnt");
        label->setPosition({ 10.f, 25.f });
        label->setAnchorPoint({ 0.f, 0.f }); // Bottom-left alignment
        label->setScale(0.30f); 
        label->setOpacity(200);
        label->setVisible(g_showPosition);
        label->setID("show-position-label"_spr);

        this->addChild(label, 1000);
        m_fields->m_posLabel = label;

        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (m_fields->m_posLabel && m_player1) {
            m_fields->m_posLabel->setVisible(g_showPosition);

            if (g_showPosition) {
                auto pos = m_player1->getPosition();
                
                // Get user precision choice from Geode Settings
                int precision = Mod::get()->getSettingValue<int64_t>("decimal-precision");

                // Dynamic format string parsing precision safely into the float format (e.g., "XPos: {:.15f}")
                std::string formatStr = fmt::format("XPos: {:.{}f}\nYPos: {:.{}f}", pos.x, precision, pos.y, precision);
                
                m_fields->m_posLabel->setString(formatStr.c_str());
            }
        }
    }
};

// 2. Add Checkbox and Copy button to the Pause Menu
class $modify(PosPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        menu->setID("show-position-menu"_spr);

        // Visibility Toggle Checkbox
        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(PosPauseLayer::onTogglePosition),
            0.6f
        );
        toggler->toggle(g_showPosition);
        toggler->setPosition({ 35.f, 35.f });

        auto label = CCLabelBMFont::create("Show Position", "bigFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint({ 0.f, 0.5f });
        label->setPosition({ 50.f, 35.f });

        // Utility: Copy Button positioned right above the checkbox
        auto copySprite = ButtonSprite::create("Copy Pos", "goldFont.fnt", "GJ_button_01.png", 0.5f);
        auto copyBtn = CCMenuItemSpriteExtra::create(
            copySprite,
            this,
            menu_selector(PosPauseLayer::onCopyPosition)
        );
        copyBtn->setPosition({ 35.f, 65.f });

        menu->addChild(toggler);
        menu->addChild(label);
        menu->addChild(copyBtn);
        this->addChild(menu, 10);
    }

    void onTogglePosition(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        g_showPosition = !toggler->isToggled();
    }

    // Handles the clipboard utility callback
    void onCopyPosition(CCObject* sender) {
        auto playLayer = PlayLayer::get();
        if (playLayer && playLayer->m_player1) {
            auto pos = playLayer->m_player1->getPosition();
            int precision = Mod::get()->getSettingValue<int64_t>("decimal-precision");
            
            // Format coordinates matching chosen precision
            std::string copyStr = fmt::format("X: {:.{}f}, Y: {:.{}f}", pos.x, precision, pos.y, precision);
            
            // Geode native clipboard wrapper
            geode::utils::clipboard::write(copyStr);
            
            Notification::create("Copied to clipboard!", NotificationIcon::Success)->show();
        } else {
            Notification::create("Failed to get position!", NotificationIcon::Error)->show();
        }
    }
};
