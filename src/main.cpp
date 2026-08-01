#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// Variable to store the toggle state of the Show Position button
static bool g_showPosition = false;

// 1. Manage X-Pos and Y-Pos display in the game level
class $modify(PosPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_posLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Default format X-Pos: 0.00000 Y-Pos: 0.00000
        auto label = CCLabelBMFont::create("X-Pos: 0.00000 Y-Pos: 0.00000", "bigFont.fnt");
        label->setPosition({ 10.f, 15.f });
        label->setAnchorPoint({ 0.f, 0.5f }); // Left align
        label->setScale(0.35f);
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
                // Get exact X and Y positions with 5 decimal places
                auto pos = m_player1->getPosition();
                std::string posText = fmt::format("X-Pos: {:.5f} Y-Pos: {:.5f}", pos.x, pos.y);
                m_fields->m_posLabel->setString(posText.c_str());
            }
        }
    }
};

// 2. Add a Checkbox toggle to the Pause Menu
class $modify(PosPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        menu->setID("show-position-menu"_spr);

        // Checkbox toggle
        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(PosPauseLayer::onTogglePosition),
            0.6f
        );

        toggler->toggle(g_showPosition);
        toggler->setPosition({ 35.f, 35.f });

        // "Show Position" label next to the checkbox
        auto label = CCLabelBMFont::create("Show Position", "bigFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint({ 0.f, 0.5f });
        label->setPosition({ 50.f, 35.f });

        menu->addChild(toggler);
        menu->addChild(label);
        this->addChild(menu, 10);
    }

    void onTogglePosition(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        g_showPosition = !toggler->isToggled();
    }
};
