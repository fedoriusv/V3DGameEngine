#include "EditorViewScreen.h"
#include "UI/Widgets.h"

EditorViewScreen::EditorViewScreen(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_popup(nullptr)
    , m_gameEventRecevier(gameEventRecevier)

    , m_sceneData(nullptr)

    , m_loaded(false)
{
}

EditorViewScreen::~EditorViewScreen()
{
}

void EditorViewScreen::registerWiget(ui::Widget* widget, scene::SceneData& sceneData)
{
    ASSERT(widget, "must be valid");
    m_popup = static_cast<ui::WidgetPopup*>(widget);
    m_sceneData = &sceneData;

    m_loaded = false;
}

void EditorViewScreen::show()
{
    ASSERT(m_popup, "must be valid");
    m_popup->open("");
}

void EditorViewScreen::hide()
{
    ASSERT(m_popup, "must be valid");
}

void EditorViewScreen::update(f32 dt)
{
    if (!m_loaded)
    {
        build();
        m_loaded = true;
    }
}

bool EditorViewScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    return false;
}

bool EditorViewScreen::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}

void EditorViewScreen::build()
{
    ui::WidgetPopup& popup = *m_popup;
    popup
        .setSize({ 350, 700 })

        //Camera
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::LargeFont)
            .addWidget(ui::WidgetText("Camera")
                .setColor({ 1.0, 1.0, 0.2, 1.0 })
            )
        )
        .addWidget(ui::WidgetHorizontalLayout()
            .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentLeft)
            .addWidget(ui::WidgetText("Near"))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._vewportParams._near)
                .setRange(0.1f, 1.0f)
                .setStep(0.01f)
                .setSize({ 100, 20 })
                .setOnChangedValueEvent([this](ui::Widget* w, f32 n)
                    {
                        scene::CameraEditorHandler* editorCamera = static_cast<scene::CameraEditorHandler*>(m_sceneData->m_viewportState._camera);
                        editorCamera->setNear(n);
                        editorCamera->update(0.0f);
                    })
            )
            .addWidget(ui::WidgetText("Far"))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._vewportParams._far)
                .setRange(10.f, 100000.0f)
                .setStep(1.f)
                .setSize({ 100, 20 })
                .setOnChangedValueEvent([this](ui::Widget* w, f32 f)
                    {
                        scene::CameraEditorHandler* editorCamera = static_cast<scene::CameraEditorHandler*>(m_sceneData->m_viewportState._camera);
                        editorCamera->setFar(f);
                        editorCamera->update(0.0f);
                    })
            )
        )
        .addWidget(ui::WidgetHorizontalLayout()
            .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentLeft)
            .addWidget(ui::WidgetText("FOV "))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._vewportParams._fov)
                .setRange(20.f, 120.f)
                .setOnChangedValueEvent([this](ui::Widget* w, f32 fov)
                    {
                        scene::CameraEditorHandler* editorCamera = static_cast<scene::CameraEditorHandler*>(m_sceneData->m_viewportState._camera);
                        editorCamera->setFOV(fov);
                        editorCamera->update(0.0f);
                    })
            )
        )
        .addWidget(ui::WidgetHorizontalLayout()
            .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentLeft)
            .addWidget(ui::WidgetText("Speed  Move "))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._vewportParams._moveSpeed)
                .setRange(1.f, 10.f)
                .setStep(1.f)
                .setSize({ 80, 20 })
                .setOnChangedValueEvent([this](ui::Widget* w, f32 speed)
                    {
                        scene::CameraEditorHandler* editorCamera = static_cast<scene::CameraEditorHandler*>(m_sceneData->m_viewportState._camera);
                        editorCamera->setMoveSpeed(speed);
                    })
            )
            .addWidget(ui::WidgetText("Rotate "))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._vewportParams._rotateSpeed)
                .setRange(1.f, 100.f)
                .setStep(1.f)
                .setSize({ 80, 20 })
                .setOnChangedValueEvent([this](ui::Widget* w, f32 speed)
                    {
                        scene::CameraEditorHandler* editorCamera = static_cast<scene::CameraEditorHandler*>(m_sceneData->m_viewportState._camera);
                        editorCamera->setRotationSpeed(speed);
                    })
            )
        )
        //Camera

        // Tonemap
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::LargeFont)
            .addWidget(ui::WidgetText("Tonemap")
                .setColor({ 1.0, 1.0, 0.2, 1.0 })
            )
        )
        .addWidget(ui::WidgetComboBox()
            .addElement("Default")
            .addElement("Reinhard")
            .addElement("ACES")
            .addElement("Khronos PBR Neutral")
            .setActiveIndex(m_sceneData->m_settings._tonemapParams._tonemapper)
            .setOnChangedIndexEvent([this](ui::Widget* w, s32 id)
                {
                    m_sceneData->m_settings._tonemapParams._tonemapper = id;
                })
        )
        .addWidget(ui::WidgetComboBox()
            .addElement("No LUT")
            .addElement("Default LUT")
            .addElement("Neutral Color")
            .setActiveIndex(m_sceneData->m_settings._tonemapParams._lut)
            .setOnChangedIndexEvent([this](ui::Widget* w, s32 id)
                {
                    m_sceneData->m_settings._tonemapParams._lut = id;
                })
        )
        .addWidget(ui::WidgetHorizontalLayout()
            .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentLeft)
            .addWidget(ui::WidgetText("Gamma "))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._tonemapParams._gamma)
                .setRange(0.1f, 10.f)
                .setStep(0.01f)
                .setSize({ 50, 20 })
                .setOnChangedValueEvent([this](ui::Widget* w, f32 gamma)
                    {
                        m_sceneData->m_settings._tonemapParams._gamma = gamma;
                    })
            )
            .addWidget(ui::WidgetText("EV100 "))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._tonemapParams._ev100)
            .setRange(0.1f, 1000.f)
            .setStep(0.1f)
            .setSize({ 50, 20 })
            .setOnChangedValueEvent([this](ui::Widget* w, f32 ev)
                {
                    m_sceneData->m_settings._tonemapParams._ev100 = ev;
                })
            )
        )
        // Tonemap

        // Shadows
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::LargeFont)
            .addWidget(ui::WidgetText("Shadows")
                .setColor({ 1.0, 1.0, 0.2, 1.0 })
            )
        )
        .addWidget(ui::WidgetCheckBox("Show shadow cascades", m_sceneData->m_settings._shadowsParams._debug)
            .setOnChangedValueEvent([this](ui::Widget* w, bool enable)
            {
                m_sceneData->m_settings._shadowsParams._debug = enable;
            })
        )
        .addWidget(ui::WidgetHorizontalLayout()
            .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentLeft)
            .addWidget(ui::WidgetText("Split factor "))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._shadowsParams._splitFactor)
            .setRange(0.01f, 1.0f)
            .setStep(0.01f)
            .setSize({ 80, 20 })
            .setOnChangedValueEvent([this](ui::Widget* w, f32 value)
                {
                    m_sceneData->m_settings._shadowsParams._splitFactor = value;
                })
            )
            .addWidget(ui::WidgetText("Long range "))
            .addWidget(ui::WidgetInputDragFloat(m_sceneData->m_settings._shadowsParams._longRange)
            .setRange(1.0f, 10000.f)
            .setStep(1.0f)
            .setSize({ 80, 20 })
            .setOnChangedValueEvent([this](ui::Widget* w, f32 value)
                {
                    m_sceneData->m_settings._shadowsParams._longRange = value;
                })
            )
        )
        .addWidget(ui::WidgetText("Shadow cascades bias (1 = 1.0)"))
        .addWidget(ui::WidgetInputDragFloat4(
            m_sceneData->m_settings._shadowsParams._cascadeBaseBias[0] * 1.0f,
            m_sceneData->m_settings._shadowsParams._cascadeBaseBias[1] * 1.0f,
            m_sceneData->m_settings._shadowsParams._cascadeBaseBias[2] * 1.0f,
            m_sceneData->m_settings._shadowsParams._cascadeBaseBias[3] * 1.0f)
            .setStep(0.001f)
            .setOnChangedValueEvent([this](ui::Widget* w, const math::float4& v)
                {
                    m_sceneData->m_settings._shadowsParams._cascadeBaseBias[0] = v._x * 1.f;
                    m_sceneData->m_settings._shadowsParams._cascadeBaseBias[1] = v._y * 1.f;
                    m_sceneData->m_settings._shadowsParams._cascadeBaseBias[2] = v._z * 1.f;
                    m_sceneData->m_settings._shadowsParams._cascadeBaseBias[3] = v._w * 1.f;
                })
        )
        .addWidget(ui::WidgetText("Shadow cascades slope (1 = 1.0)"))
        .addWidget(ui::WidgetInputDragFloat4(
            m_sceneData->m_settings._shadowsParams._cascadeSlopeBias[0],
            m_sceneData->m_settings._shadowsParams._cascadeSlopeBias[1],
            m_sceneData->m_settings._shadowsParams._cascadeSlopeBias[2],
            m_sceneData->m_settings._shadowsParams._cascadeSlopeBias[3])
            .setStep(0.001f)
            .setOnChangedValueEvent([this](ui::Widget* w, const math::float4& v)
            {
                m_sceneData->m_settings._shadowsParams._cascadeSlopeBias[0] = v._x;
                m_sceneData->m_settings._shadowsParams._cascadeSlopeBias[1] = v._y;
                m_sceneData->m_settings._shadowsParams._cascadeSlopeBias[2] = v._z;
                m_sceneData->m_settings._shadowsParams._cascadeSlopeBias[3] = v._w;
            })
        )
        // Shadows

        // Debug
        .addWidget(ui::WidgetLayout()
            .setFontSize(ui::WidgetLayout::LargeFont)
            .addWidget(ui::WidgetText("Debug")
                .setColor({ 1.0, 1.0, 0.2, 1.0 })
            )
        )
        .addWidget(ui::WidgetCheckBox("Show Debug lines", true)
            .setOnChangedValueEvent([this](ui::Widget* w, bool enable)
            {
                //TODO
            })
        )
        .addWidget(ui::WidgetCheckBox("Show Ground grid", false)
        .setOnChangedValueEvent([this](ui::Widget* w, bool enable)
            {
                //TODO
            })
        )
        .addWidget(ui::WidgetComboBox()
        .addElement("Final")
        .addElement("GBuffer Albedo")
        .addElement("GBuffer Normals")
        .addElement("Depth")
        .addElement("Lights")
        .setActiveIndex(0)
        .setOnChangedIndexEvent([this](ui::Widget* w, s32 id)
            {
                //TODO
            })
        );
        // Debug
}
