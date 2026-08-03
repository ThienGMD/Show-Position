#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// Biến tĩnh lưu trạng thái hiển thị tọa độ
static bool g_showPosition = false;

// 1. Quản lý hiển thị XPos và YPos trong màn chơi
class $modify(PosPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_posLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Khởi tạo nhãn với 2 dòng, độ chính xác mặc định 15 chữ số
        auto label = CCLabelBMFont::create("XPos: 0.000000000000000\nYPos: 0.000000000000000", "bigFont.fnt");
        
        // Điều chỉnh vị trí: Xích lên một chút (Y: 25.f) để không bị che khuất bởi viền dưới màn hình
        label->setPosition({ 10.f, 25.f });
        // Điểm neo {0.f, 0.f} giúp căn lề trái và cố định gốc tọa độ từ đáy nhãn lên trên
        label->setAnchorPoint({ 0.f, 0.f }); 
        label->setScale(0.30f); // Thu nhỏ scale một chút vì chuỗi 15 chữ số rất dài
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
                // Định dạng hiển thị chính xác 15 chữ số thập phân và xuống dòng (\n)
                std::string posText = fmt::format("XPos: {:.15f}\nYPos: {:.15f}", pos.x, pos.y);
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

        menu->addChild(toggler);
        menu->addChild(label);
        this->addChild(menu, 10);
    }

    void onTogglePosition(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        g_showPosition = !toggler->isToggled();
    }
};
