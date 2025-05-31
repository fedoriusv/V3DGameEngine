#include "EditorUI.h"
#include "UI/Wigets.h"

#include "Utils/Logger.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ImageFileLoader.h"
#include "Resource/Bitmap.h"

namespace v3d
{
namespace editor
{


ui::WigetMenuBar& UI::constructMainMenu(ui::WigetHandler* handler)
{
    ui::WigetMenuBar& menu = handler->createWiget<ui::WigetMenuBar>(ui::WigetMenuBar::MainMenu)
        //File
        .addWiget(ui::WigetMenu("File")
            .addWiget(ui::WigetMenuItem("New")
                .setOnClickedEvent([](ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("New click");
                    }))
            .addWiget(ui::WigetMenuItem("Open...")
                .setOnClickedEvent([](ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("Open click");
                    }))
            .addWiget(ui::WigetMenuItem("Save")
                .setOnClickedEvent([](ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("Save click");
                    }))
            .addWiget(ui::WigetMenu("Recent Open")
                .addWiget(ui::WigetMenuItem("temp.temp"))
            )
            .addWiget(ui::WigetMenuItem("Exit")
                .setOnClickedEvent([](ui::Wiget* w) -> void
                    {
                        LOG_DEBUG("Exit click");
                    }))
        )
        //Edit
        .addWiget(ui::WigetMenu("Edit")
            .setOnClickedEvent([](ui::Wiget* w) -> void
                {
                    LOG_DEBUG("Open click");
                }))
        //View
        .addWiget(ui::WigetMenu("View")
            .setOnClickedEvent([](ui::Wiget* w) -> void
                {
                    LOG_DEBUG("Save click");
                }))
        .addWiget(ui::WigetMenuItem("TODO")
            .setOnClickedEvent([](ui::Wiget* w) -> void
                {
                    LOG_DEBUG("TODO click");
                }))
        .addWiget(ui::WigetMenuItem("FPS:")
            .setOnUpdate([](ui::Wiget* w, f32 dt) -> void
                {
                    static f32 timePassed = 0;
                    static u32 FPSCounter = 0;

                    const f32 diffTime = dt * 1'000.f;
                    timePassed += diffTime;
                    ++FPSCounter;
                    if (timePassed >= 1'000.f) //sec
                    {
                        static_cast<ui::WigetMenuItem*>(w)->setText(std::format("FPS: {} ({:.2f} ms)", FPSCounter, timePassed / FPSCounter));

                        FPSCounter = 0;
                        timePassed = 0.f;
                    }
                }));

    return menu;
}

void UI::constructTransformProp(ui::WigetHandler* handler)
{
    //TODO
}

void UI::constuctTestUIWindow(ui::WigetHandler* handler, renderer::Texture2D* texture)
{
    handler->createWiget<ui::WigetWindow>("Window Test", math::Dimension2D(800, 800), math::Point2D(10, 10), ui::WigetWindow::Moveable | ui::WigetWindow::Resizeable)
        .setActive(true)
        .setVisible(true)
        .setOnActiveChanged([](ui::Wiget* w) -> void
            {
                LOG_DEBUG("New menu click");
            })
        .addWiget(ui::WigetButton("button")
            .setOnClickedEvent([](ui::Wiget* w) -> void
                {
                    LOG_DEBUG("Button click");
                })
            .setToolTip(true, "test tooltip")
        )
        .addWiget(ui::WigetButton("button1Sized")
            .setSize({ 100, 100 })
            .setOnClickedEvent([](ui::Wiget* w) -> void
                {
                    LOG_DEBUG("Button click");
                })
        )
        .addWiget(ui::WigetButton("button2Colored")
            .setColor({ 0.7, 0.5, 0.5, 1.0 })
            .setColorHovered({ 0.8, 0.4, 0.4, 1.0 })
            .setColorActive({ 0.9, 0.2, 0.2, 1.0 })
        )
        .addWiget(ui::WigetText("textColored")
            .setColor({ 0.7, 0.5, 0.5, 1.0 })
        )
        .addWiget(ui::WigetLayout()
            .setFontSize(ui::WigetLayout::LargeFont)
            .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentRight)
            .addWiget(ui::WigetButton("buttonStyled0"))
            .addWiget(ui::WigetText("textStyled0"))
        )
        .addWiget(ui::WigetHorizontalLayout()
            .setFontSize(ui::WigetLayout::MediumFont)
            .addWiget(ui::WigetButton("buttonLine0"))
            .addWiget(ui::WigetButton("buttonLine1"))
            .addWiget(ui::WigetButton("buttonLine2"))
            .addWiget(ui::WigetButton("buttonLine3"))
        )
        .addWiget(ui::WigetLayout(ui::WigetLayout::Border)
            .setSize({ 220, 220 })
            .addWiget(ui::WigetLayout()
                .setSize({ 200, 100 })
                .setFontSize(ui::WigetLayout::SmallFont)
                .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentCenter)
                .setVAlignment(ui::WigetLayout::VerticalAlignment::AlignmentTop)
                .addWiget(ui::WigetButton("buttonTop"))
                .addWiget(ui::WigetText("textTop"))
            )
            .addWiget(ui::WigetLayout()
                .setSize({ 200, 100 })
                .setFontSize(ui::WigetLayout::SmallFont)
                .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentCenter)
                .setVAlignment(ui::WigetLayout::VerticalAlignment::AlignmentBottom)
                .addWiget(ui::WigetButton("buttonBottom"))
                .addWiget(ui::WigetText("textBottom"))
            )
        )
        .addWiget(ui::WigetCheckBox("checkbox", true)
            .setOnChangedValueEvent([](ui::Wiget* w, bool value) -> void
                {
                    LOG_DEBUG("Checkbox Value %d", value);
                })
        )
        .addWiget(ui::WigetCheckBox("checkbox", true)
            .setOnChangedValueEvent([](ui::Wiget* w, bool value) -> void
                {
                    LOG_DEBUG("Checkbox next Value %d", value);
                })
        )
        .addWiget(ui::WigetRadioButtonGroup()
            .addElement("value 0")
            .addElement("value 1")
            .addElement("value 2")
            .setActiveIndex(1)
            .setOnChangedIndexEvent([](ui::Wiget* w, s32 index) -> void
                {
                    LOG_DEBUG("RadioButton Index %d", index);
                })
        )
        .addWiget(ui::WigetListBox()
            .addElement("AAAA")
            .addElement("BBBB")
            .addElement("CCCC")
            .addElement("DDDD")
            .addElement("FFFF")
            .setActiveIndex(1)
            .setOnChangedIndexEvent([](ui::Wiget* w, s32 index) -> void
                {
                    LOG_DEBUG("WigetListBox Index %d", index);
                })
        )
        .addWiget(ui::WigetComboBox()
            .addElement("AAAA")
            .addElement("BBBB")
            .addElement("CCCC")
            .addElement("DDDD")
            .addElement("FFFF")
            .setActiveIndex(1)
            .setOnChangedIndexEvent([](ui::Wiget* w, s32 index) -> void
                {
                    LOG_DEBUG("WigetComboBox Index %d", index);
                })
        )
        .addWiget(ui::WigetInputField()
            .setActive(true)
        )
        .addWiget(ui::WigetInputSlider()
            .setActive(true)
        )
        .addWiget(ui::WigetLayout(ui::WigetLayout::Border)
            .setHAlignment(ui::WigetLayout::HorizontalAlignment::AlignmentFill)
            .addWiget(ui::WigetImage(texture, { 200, 200 }, { 0.25, 0.25, 1, 1 }))
        );
}

} //namespace editor
} //namespace v3d