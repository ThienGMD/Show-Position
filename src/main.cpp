#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <sstream>
#include <iomanip>

using namespace geode::prelude;

// Global runtime visibility tracker
static bool g_showPosition = false;

// Safe precision formatter supporting unlimited decimals without string limits
std::string formatCoordinate(const std::string& prefix, float value, int precision) {
    if (precision <= 0) {
        return fmt::format("{}: {:.0f}", prefix, value);
    }
    
    double preciseValue = static_cast<double>(value);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision);
    ss << preciseValue;
    
    return prefix + ": " + ss.str();
}

// FIX POSITION BUG: Calculates safe alignment within the game's scaled screen boundaries
void updateLabelLayout(CCLabelBMFont* label, const std::string& alignment) {
    // Standard Cocos2d-x scaled window size used by Geometry Dash
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    // Padding offsets to prevent the text from clipping out of mobile screen edges
    float paddingX = 10.f;
    float paddingY = 15.f;
    
    if (alignment == "bottom-left") {
        label->setPosition({ paddingX, paddingY });
        label->setAnchorPoint({ 0.f, 0.f });
        label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    } 
    else if (alignment == "center-left") {
        label->setPosition({ paddingX, winSize.height / 2.f });
        label->setAnchorPoint({ 0.f, 0.5f });
        label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    } 
    else if (alignment == "top-left") {
        label->setPosition({ paddingX, winSize.height - paddingY });
        label->setAnchorPoint({ 0.f, 1.f });
        label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    } 
    else if (alignment == "bottom-right") {
        label->setPosition({ winSize.width - paddingX, paddingY });
        label->setAnchorPoint({ 1.f, 0.f });
        label->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    } 
    else if (alignment == "center-right") {
        label->setPosition({ winSize.width - paddingX, winSize.height / 2.f });
        label->setAnchorPoint({ 1.f, 0.5f });
        label->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    } 
    else if (alignment == "top-right") {
        label->setPosition({ winSize.width - paddingX, winSize.height - paddingY });
        label->setAnchorPoint({ 1.f, 1.f });
        label->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    }
}

// 1. Manage XPos and YPos display during gameplay
class $modify(PosPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_posLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto label = CCLabelBMFont::create("XPos: 0\nYPos: 0", "bigFont.fnt");
        label->setScale(0.30f); 
        label->setVisible(g_showPosition);
        label->setID("show-position-label"_spr);

        auto currentMod = Mod::get();
        int opacity = static_cast<int>(currentMod->getSettingValue<int64_t>("label-opacity"));
        std::string alignment = currentMod->getSettingValue<std::string>("label-alignment");
        
        label->setOpacity(static_cast<GLubyte>(opacity));
        updateLabelLayout(label, alignment);

        // Add label to the main layer with a very high Z-Order to overlay correctly
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
                
                auto currentMod = Mod::get();
                int precision = static_cast<int>(currentMod->getSettingValue<int64_t>("decimal-precision"));
                int opacity = static_cast<int>(currentMod->getSettingValue<int64_t>("label-opacity"));
                std::string alignment = currentMod->getSettingValue<std::string>("label-alignment");

                m_fields->m_posLabel->setOpacity(static_cast<GLubyte>(opacity));
                updateLabelLayout(m_fields->m_posLabel, alignment);

                std::string xText = formatCoordinate("XPos", pos.x, precision);
                std::string yText = formatCoordinate("YPos", pos.y, precision);
                
                m_fields->m_posLabel->setString(fmt::format("{}\n{}", xText, yText).c_str());
            }
        }
    }
};

// 2. Add Checkbox and Optimized Copy button to PauseLayer
class $modify(PosPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        menu->setID("show-position-menu"_spr);

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

        auto copySprite = ButtonSprite::create("Copy Pos", "goldFont.fnt", "GJ_button_01.png", 0.4f);
        auto copyBtn = CCMenuItemSpriteExtra::create(
            copySprite,
            this,
            menu_selector(PosPauseLayer::onCopyPosition)
        );
        copyBtn->setPosition({ 35.f, 60.f }); 

        menu->addChild(toggler);
        menu->addChild(label);
        menu->addChild(copyBtn);
        this->addChild(menu, 10);
    }

    void onTogglePosition(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        g_showPosition = !toggler->isToggled();
    }

    void onCopyPosition(CCObject* sender) {
        auto playLayer = PlayLayer::get();
        if (playLayer && playLayer->m_player1) {
            auto pos = playLayer->m_player1->getPosition();
            int precision = static_cast<int>(Mod::get()->getSettingValue<int64_t>("decimal-precision"));
            
            std::string xStr = formatCoordinate("X", pos.x, precision);
            std::string yStr = formatCoordinate("Y", pos.y, precision);
            std::string copyStr = fmt::format("{}, {}", xStr, yStr);
            
            geode::utils::clipboard::write(copyStr);
            Notification::create("Copied to clipboard!", NotificationIcon::Success)->show();
        } else {
            Notification::create("Failed to get position!", NotificationIcon::Error)->show();
        }
    }
};
