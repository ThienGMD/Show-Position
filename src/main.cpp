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

// Chỉnh vị trí + neo của label theo setting "label-alignment" mà người dùng chọn
// (trước đây bị để cứng ở góc dưới-trái, đổi setting cũng chả ăn thua gì)
void applyLabelAlignment(CCLabelBMFont* label) {
    if (!label) return;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto alignment = Mod::get()->getSettingValue<std::string>("label-alignment");

    const float margin = 10.f;
    CCPoint anchor;
    CCPoint pos;

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
        // Lỡ setting bị lạ/không khớp giá trị nào thì cứ về góc dưới-trái cho chắc
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

    // Hàm init phải nhận đủ 3 tham số theo bản Geode mới nhất bên Android, thiếu là lỗi ngay
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto label = CCLabelBMFont::create("XPos: 0\nYPos: 0", "bigFont.fnt");
        label->setScale(0.30f);
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
                // Đọc lại vị trí/neo + độ mờ từ setting mỗi frame luôn, chứ bản Geode này
                // không có sẵn hàm nghe thay đổi setting (listenForSettingChanges), nên
                // cứ áp lại liên tục cho chắc ăn, đổi setting là thấy ngay không cần thoát màn
                applyLabelAlignment(m_fields->m_posLabel);
                m_fields->m_posLabel->setOpacity(static_cast<GLubyte>(Mod::get()->getSettingValue<int64_t>("label-opacity")));

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

        // Đặt menu theo winSize (góc trên-trái) chứ để tọa độ cứng (35,35) như cũ
        // là nó đè lên mấy nút có sẵn của PauseLayer (retry/edit/exit) trên máy tỉ lệ màn hình khác
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setID("show-position-menu"_spr);
        menu->setAnchorPoint({ 0.f, 0.f });

        const float marginX = 45.f;
        const float topY = winSize.height - 25.f;

        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(PosPauseLayer::onTogglePosition),
            0.6f
        );
        toggler->toggle(g_showPosition);
        toggler->setAnchorPoint({ 0.f, 0.5f });
        toggler->setPosition({ marginX, topY });

        auto label = CCLabelBMFont::create("Show Position", "bigFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint({ 0.f, 0.5f });
        label->setPosition({ marginX + 20.f, topY });

        auto copySprite = ButtonSprite::create("Copy Pos", "goldFont.fnt", "GJ_button_01.png", 0.5f);
        auto copyBtn = CCMenuItemSpriteExtra::create(
            copySprite,
            this,
            menu_selector(PosPauseLayer::onCopyPosition)
        );
        copyBtn->setAnchorPoint({ 0.f, 0.5f });
        copyBtn->setPosition({ marginX, topY - 30.f });

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
