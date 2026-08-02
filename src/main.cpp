#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// Biến tĩnh lưu trạng thái hiển thị tọa độ
static bool g_showPosition = false;

// 1. Quản lý hiển thị X-Pos và Y-Pos trong màn chơi
class $modify(PosPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_posLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Tạo nhãn hiển thị tọa độ với ID an toàn
        auto label = CCLabelBMFont::create("X-Pos: 0.00000 Y-Pos: 0.00000", "bigFont.fnt");
        label->setPosition({ 10.f, 15.f });
        label->setAnchorPoint({ 0.f, 0.5f }); 
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
            // Luôn cập nhật trạng thái ẩn/hiện theo biến g_showPosition
            m_fields->m_posLabel->setVisible(g_showPosition);

            if (g_showPosition) {
                auto pos = m_player1->getPosition();
                std::string posText = fmt::format("X-Pos: {:.5f} Y-Pos: {:.5f}", pos.x, pos.y);
                m_fields->m_posLabel->setString(posText.c_str());
            }
        }
    }
};

// 2. Thêm nút Checkbox bật/tắt vào Pause Menu
class $modify(PosPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        menu->setID("show-position-menu"_spr);

        // Khởi tạo Checkbox toggle
        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(PosPauseLayer::onTogglePosition),
            0.6f
        );

        // FIX 1: Đồng bộ nút checkbox đúng với giá trị thực tế của biến g_showPosition
        // Trong Cocos2d-x, hàm toggle(true) sẽ kích hoạt trạng thái "on" của nút
        toggler->toggle(g_showPosition);
        toggler->setPosition({ 35.f, 35.f });

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
        // FIX 2: Lật ngược trạng thái (toggle) biến tĩnh dựa trên việc người dùng vừa click vào nút
        // Thao tác click làm thay đổi trạng thái hiển thị của nút ngay lập tức
        g_showPosition = !toggler->isToggled();
    }
};
