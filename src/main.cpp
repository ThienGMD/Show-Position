#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <algorithm> // Cho hàm std::min

using namespace geode::prelude;

// Biến tĩnh lưu trạng thái ẩn/hiện tọa độ
static bool g_showPosition = false;

// Hàm trợ giúp định dạng chuỗi an toàn dựa trên số lượng chữ số thập phân
std::string formatCoordinate(const std::string& prefix, float value, int precision) {
    if (precision <= 0) {
        return fmt::format("{}: {:.0f}", prefix, value);
    }
    // Giới hạn max là 10,000 để an toàn, tránh văng game do tràn OOM
    int safePrecision = std::min(precision, 10000);
    return fmt::format("{}: {:.{}f}", prefix, value, safePrecision);
}

// Chỉnh vị trí + neo của label theo setting "label-alignment"
void applyLabelAlignment(CCLabelBMFont* label, const std::string& alignment) {
    if (!label) return;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    const float margin = 10.f;
    
    CCPoint anchor;
    CCPoint pos;

    // Sử dụng ccp() thay vì {} để tránh lỗi ambiguous trên Android
    if (alignment == "bottom-left") {
        anchor = ccp(0.f, 0.f);
        pos = ccp(margin, margin);
    } else if (alignment == "center-left") {
        anchor = ccp(0.f, 0.5f);
        pos = ccp(margin, winSize.height / 2.f);
    } else if (alignment == "top-left") {
        anchor = ccp(0.f, 1.f);
        pos = ccp(margin, winSize.height - margin);
    } else if (alignment == "bottom-right") {
        anchor = ccp(1.f, 0.f);
        pos = ccp(winSize.width - margin, margin);
    } else if (alignment == "center-right") {
        anchor = ccp(1.f, 0.5f);
        pos = ccp(winSize.width - margin, winSize.height / 2.f);
    } else if (alignment == "top-right") {
        anchor = ccp(1.f, 1.f);
        pos = ccp(winSize.width - margin, winSize.height - margin);
    } else {
        anchor = ccp(0.f, 0.f);
        pos = ccp(margin, margin);
    }

    label->setAnchorPoint(anchor);
    label->setPosition(pos);
}

// 1. Quản lý hiển thị tọa độ trong PlayLayer
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
        applyLabelAlignment(label, currentMod->getSettingValue<std::string>("label-alignment"));
        label->setOpacity(static_cast<GLubyte>(currentMod->getSettingValue<int64_t>("label-opacity")));

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
                
                // Cập nhật vị trí và độ mờ liên tục mỗi frame thay vì dùng Listener dễ gây crash
                applyLabelAlignment(m_fields->m_posLabel, currentMod->getSettingValue<std::string>("label-alignment"));
                m_fields->m_posLabel->setOpacity(static_cast<GLubyte>(currentMod->getSettingValue<int64_t>("label-opacity")));

                int precision = static_cast<int>(currentMod->getSettingValue<int64_t>("decimal-precision"));

                std::string xText = formatCoordinate("XPos", pos.x, precision);
                std::string yText = formatCoordinate("YPos", pos.y, precision);
                
                m_fields->m_posLabel->setString(fmt::format("{}\n{}", xText, yText).c_str());
            }
        }
    }
};

// 2. Thêm Checkbox và nút Copy vào PauseLayer
class $modify(PosPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();
        menu->setPosition(ccp(0.f, 0.f));
        menu->setID("show-position-menu"_spr);
        menu->setAnchorPoint(ccp(0.f, 0.f));

        const float marginX = 45.f;
        const float topY = winSize.height - 25.f;

        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(PosPauseLayer::onTogglePosition),
            0.6f
        );
        toggler->toggle(g_showPosition);
        toggler->setAnchorPoint(ccp(0.f, 0.5f));
        toggler->setPosition(ccp(marginX, topY));

        auto label = CCLabelBMFont::create("Show Position", "bigFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint(ccp(0.f, 0.5f));
        label->setPosition(ccp(marginX + 20.f, topY));

        auto copySprite = ButtonSprite::create("Copy Pos", "goldFont.fnt", "GJ_button_01.png", 0.5f);
        auto copyBtn = CCMenuItemSpriteExtra::create(
            copySprite,
            this,
            menu_selector(PosPauseLayer::onCopyPosition)
        );
        copyBtn->setAnchorPoint(ccp(0.f, 0.5f));
        copyBtn->setPosition(ccp(marginX, topY - 30.f));

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
