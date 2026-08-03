#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// Biến tĩnh lưu trạng thái ẩn/hiện tọa độ
static bool g_showPosition = false;

// Hàm trợ giúp định dạng chuỗi an toàn dựa trên số lượng chữ số thập phân người dùng nhập
std::string formatCoordinate(const std::string& prefix, float value, int precision) {
    // TRƯỜNG HỢP 1: Người dùng chỉnh min = 0 (Không hiện số thập phân)
    if (precision <= 0) {
        return fmt::format("{}: {:.0f}", prefix, value);
    }
    // TRƯỜNG HỢP 2: Người dùng chỉnh số lớn hơn giới hạn an toàn (Tránh crash game do tràn bộ nhớ)
    else if (precision > 15) {
        // Sử dụng định dạng mặc định {} để hiển thị hết khả năng của float mà không bị crash
        return fmt::format("{}: {}", prefix, value);
    }
    // TRƯỜNG HỢP 3: Giá trị chuẩn từ 1 đến 15
    else {
        return fmt::format("{}: {:.{}f}", prefix, value, precision);
    }
}

// 1. Quản lý hiển thị tọa độ trong PlayLayer
class $modify(PosPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_posLabel = nullptr;
    };

    // FIX LỖI: Cập nhật hàm init nhận đủ 3 tham số theo chuẩn Geode mới nhất trên Android
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto label = CCLabelBMFont::create("XPos: 0\nYPos: 0", "bigFont.fnt");
        label->setPosition({ 10.f, 25.f });
        label->setAnchorPoint({ 0.f, 0.f }); 
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
                
                // Lấy số chữ số thập phân từ cài đặt Geode
                auto currentMod = Mod::get();
                int precision = static_cast<int>(currentMod->getSettingValue<int64_t>("decimal-precision"));

                // Gọi hàm định dạng an toàn
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
