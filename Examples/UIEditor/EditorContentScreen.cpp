#include "EditorContentScreen.h"

EditorContentScreen::EditorContentScreen(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_gameEventRecevier(gameEventRecevier)
    , m_window(nullptr)

    , m_sceneData(nullptr)
    , m_loaded(false)
{
}

EditorContentScreen::~EditorContentScreen()
{
}

void EditorContentScreen::registerWiget(ui::WidgetWindow* widget, scene::SceneData& sceneData)
{
    ASSERT(widget, "must be valid");
    m_window = widget;
    m_sceneData = &sceneData;

    m_loaded = false;
}

void EditorContentScreen::build()
{
    ASSERT(m_window, "must be valid");
    ui::WidgetWindow& window = *m_window;
    window.removeWigets();

    bool showSceneContent = true;
    if (showSceneContent)
    {
        ui::WidgetTreeNode sceneNode("Scene", ui::WidgetTreeNode::TreeNodeFlag::Framed | ui::WidgetTreeNode::TreeNodeFlag::Open);
        if (m_sceneData)
        {
            m_widgetItems.resize(m_sceneData->m_generalList.size(), nullptr);
            for (u32 index = 0; index < m_sceneData->m_generalList.size(); ++index)
            {
                auto& item = m_sceneData->m_generalList[index];
                sceneNode.addWidget(ui::WidgetTreeNode(std::string(item->_geometry._ID.name()), ui::WidgetTreeNode::TreeNodeFlag::NoCollapsed)
                    .setOnCreated([this, index](ui::Widget* w)
                        {
                            m_widgetItems[index] = static_cast<ui::WidgetTreeNode*>(w);
                        })
                    .setIndex(index)
                    .setSelected(m_sceneData->m_generalList[index]->_selected)
                    .setOnClickEvent([this](ui::Widget* w, s32 index) -> void
                        {
                            m_sceneData->m_generalList[index]->_selected = true;
                            m_gameEventRecevier->sendEvent(new EditorSelectionEvent(index));
                        })
                    );
            }
        }

        window
            .addWidget(ui::WidgetLayout()
                .setFontSize(ui::WidgetLayout::MediumFont)
                .addWidget(sceneNode)
            );
    }
}

void EditorContentScreen::update(f32 dt)
{
    if (!m_loaded)
    {
        build();
        m_loaded = true;
    }
}

bool EditorContentScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    if (m_loaded)
    {
        if (event->_eventType == event::GameEvent::GameEventType::SelectObject)
        {
            const EditorSelectionEvent* selectionEvent = static_cast<const EditorSelectionEvent*>(event);
            if (m_widgetItems.size() < m_sceneData->m_generalList.size())
            {
                m_widgetItems.resize(m_sceneData->m_generalList.size(), nullptr);
                m_loaded = false;
            }

            for (auto& item : m_widgetItems)
            {
                item->setSelected(false);
            }

            if (selectionEvent->_selectedIndex != k_emptyIndex)
            {
                m_widgetItems[selectionEvent->_selectedIndex]->setSelected(true);
            }
        }

        return true;
    }

    return false;
}

bool EditorContentScreen::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}
