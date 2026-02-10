#include "EditorUI.h"
#include "UI/Widgets.h"

#include "Utils/Logger.h"
#include "Resource/ResourceManager.h"
#include "Resource/Loader/ImageFileLoader.h"
#include "Resource/Bitmap.h"

namespace v3d
{
namespace editor
{


ui::WidgetMenuBar& UI::constructMainMenu(ui::WidgetHandler* handler)
{
    ui::WidgetMenuBar& menu = handler->createWidget<ui::WidgetMenuBar>(ui::WidgetMenuBar::MainMenu)
        //File
        .addWidget(ui::WidgetMenu("File")
            .addWidget(ui::WidgetMenuItem("New")
                .setOnClickedEvent([](ui::Widget* w) -> void
                    {
                        LOG_DEBUG("New click");
                    }))
            .addWidget(ui::WidgetMenuItem("Open...")
                .setOnClickedEvent([](ui::Widget* w) -> void
                    {
                        LOG_DEBUG("Open click");
                    }))
            .addWidget(ui::WidgetMenuItem("Save")
                .setOnClickedEvent([](ui::Widget* w) -> void
                    {
                        LOG_DEBUG("Save click");
                    }))
            .addWidget(ui::WidgetMenu("Recent Open")
                .addWidget(ui::WidgetMenuItem("temp.temp"))
            )
            .addWidget(ui::WidgetMenuItem("Exit")
                .setOnClickedEvent([](ui::Widget* w) -> void
                    {
                        LOG_DEBUG("Exit click");
                    }))
        )
        //Edit
        .addWidget(ui::WidgetMenu("Edit")
            .setOnClickedEvent([](ui::Widget* w) -> void
                {
                    LOG_DEBUG("Open click");
                }))
        //View
        .addWidget(ui::WidgetMenu("View")
            .setOnClickedEvent([](ui::Widget* w) -> void
                {
                    LOG_DEBUG("Save click");
                }))
        .addWidget(ui::WidgetMenuItem("TODO")
            .setOnClickedEvent([](ui::Widget* w) -> void
                {
                    LOG_DEBUG("TODO click");
                }))
        .addWidget(ui::WidgetMenuItem("FPS:")
            .setOnUpdate([](ui::Widget* w, f32 dt) -> void
                {
                    static f32 timePassed = 0;
                    static u32 FPSCounter = 0;

                    const f32 diffTime = dt * 1'000.f;
                    timePassed += diffTime;
                    ++FPSCounter;
                    if (timePassed >= 1'000.f) //sec
                    {
                        static_cast<ui::WidgetMenuItem*>(w)->setText(std::format("FPS: {} ({:.2f} ms)", FPSCounter, timePassed / FPSCounter));

                        FPSCounter = 0;
                        timePassed = 0.f;
                    }
                }));

    return menu;
}

void UI::constructTransformProp(ui::WidgetHandler* handler)
{
    //TODO
}

void UI::constuctTestUIWindow(ui::WidgetHandler* handler, scene::SceneData& scene)
{
    ObjectHandle uv_grid = scene.m_globalResources.get("uv_grid");
    ASSERT(uv_grid.isValid(), "must be valid");
    renderer::Texture2D* texture = objectFromHandle<renderer::Texture2D>(uv_grid);

    handler->createWidget<ui::WidgetWindow>("Window Test", math::Dimension2D(800, 800), math::Point2D(10, 10), ui::WidgetWindow::Moveable | ui::WidgetWindow::Resizeable)
        .setActive(true)
        .setVisible(true)
        .setOnActiveChanged([](ui::Widget* w) -> void
            {
                LOG_DEBUG("New menu click");
            })
        .addWidget(ui::WidgetButton("button")
            .setOnClickedEvent([](ui::Widget* w) -> void
                {
                    LOG_DEBUG("Button click");
                })
            .setToolTip(true, "test tooltip")
        )
        .addWidget(ui::WidgetButton("button1Sized")
            .setSize({ 100, 100 })
            .setOnClickedEvent([](ui::Widget* w) -> void
                {
                    LOG_DEBUG("Button click");
                })
        )
        .addWidget(ui::WidgetButton("button2Colored")
            .setColor({ 0.7, 0.5, 0.5, 1.0 })
            .setColorHovered({ 0.8, 0.4, 0.4, 1.0 })
            .setColorActive({ 0.9, 0.2, 0.2, 1.0 })
        )
        .addWidget(ui::WidgetText("textColored")
            .setColor({ 0.7, 0.5, 0.5, 1.0 })
        )
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::LargeFont)
            .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentRight)
            .addWidget(ui::WidgetButton("buttonStyled0"))
            .addWidget(ui::WidgetText("textStyled0"))
        )
        .addWidget(ui::WidgetHorizontalLayout()
            .setFontSize(ui::WidgetLayout::MediumFont)
            .addWidget(ui::WidgetButton("buttonLine0"))
            .addWidget(ui::WidgetButton("buttonLine1"))
            .addWidget(ui::WidgetButton("buttonLine2"))
            .addWidget(ui::WidgetButton("buttonLine3"))
        )
        .addWidget(ui::WidgetLayout(ui::WidgetLayout::Border)
            .setSize({ 220, 220 })
            .addWidget(ui::WidgetLayout()
                .setSize({ 200, 100 })
                .setFontSize(ui::WidgetLayout::SmallFont)
                .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentCenter)
                .setVAlignment(ui::WidgetLayout::VerticalAlignment::AlignmentTop)
                .addWidget(ui::WidgetButton("buttonTop"))
                .addWidget(ui::WidgetText("textTop"))
            )
            .addWidget(ui::WidgetLayout()
                .setSize({ 200, 100 })
                .setFontSize(ui::WidgetLayout::SmallFont)
                .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentCenter)
                .setVAlignment(ui::WidgetLayout::VerticalAlignment::AlignmentBottom)
                .addWidget(ui::WidgetButton("buttonBottom"))
                .addWidget(ui::WidgetText("textBottom"))
            )
        )
        .addWidget(ui::WidgetCheckBox("checkbox", true)
            .setOnChangedValueEvent([](ui::Widget* w, bool value) -> void
                {
                    LOG_DEBUG("Checkbox Value %d", value);
                })
        )
        .addWidget(ui::WidgetCheckBox("checkbox", true)
            .setOnChangedValueEvent([](ui::Widget* w, bool value) -> void
                {
                    LOG_DEBUG("Checkbox next Value %d", value);
                })
        )
        .addWidget(ui::WidgetRadioButtonGroup()
            .addElement("value 0")
            .addElement("value 1")
            .addElement("value 2")
            .setActiveIndex(1)
            .setOnChangedIndexEvent([](ui::Widget* w, s32 index) -> void
                {
                    LOG_DEBUG("RadioButton Index %d", index);
                })
        )
        .addWidget(ui::WidgetListBox()
            .addElement("AAAA")
            .addElement("BBBB")
            .addElement("CCCC")
            .addElement("DDDD")
            .addElement("FFFF")
            .setActiveIndex(1)
            .setOnChangedIndexEvent([](ui::Widget* w, s32 index) -> void
                {
                    LOG_DEBUG("WidgetListBox Index %d", index);
                })
        )
        .addWidget(ui::WidgetComboBox()
            .addElement("AAAA")
            .addElement("BBBB")
            .addElement("CCCC")
            .addElement("DDDD")
            .addElement("FFFF")
            .setActiveIndex(1)
            .setOnChangedIndexEvent([](ui::Widget* w, s32 index) -> void
                {
                    LOG_DEBUG("WidgetComboBox Index %d", index);
                })
        )
        .addWidget(ui::WidgetInputText("text")
            .setActive(true)
        )
        .addWidget(ui::WidgetInputInt(1)
            .setActive(true)
        )
        .addWidget(ui::WidgetInputFloat3(1, 2, 3)
        .setActive(true)
        )
        .addWidget(ui::WidgetInputSlider()
            .setActive(true)
        )
        .addWidget(ui::WidgetLayout(ui::WidgetLayout::Border)
            .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentFill)
            .addWidget(ui::WidgetImage(texture, { 200, 200 }, { 0.25, 0.25, 1, 1 }))
        );
}

} //namespace editor
} //namespace v3d