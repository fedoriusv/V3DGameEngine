#include "EditorContentScreen.h"

EditorContentScreen::EditorContentScreen(event::GameEventReceiver* gameEventRecevier) noexcept
    : m_gameEventRecevier(gameEventRecevier)
    , m_window(nullptr)

    , m_sceneData(nullptr)
    , m_selectedWidget(nullptr)
    , m_loaded(false)
{
}

EditorContentScreen::~EditorContentScreen()
{
}

void EditorContentScreen::registerWiget(ui::Widget* widget, scene::SceneData& sceneData)
{
    ASSERT(widget, "must be valid");
    m_window = static_cast<ui::WidgetWindow*>(widget);
    m_sceneData = &sceneData;

    m_loaded = false;
}

void EditorContentScreen::show()
{
    ASSERT(m_window, "must be valid");
    m_window->setVisible(true);
}

void EditorContentScreen::hide()
{
    ASSERT(m_window, "must be valid");
    m_window->setVisible(false);
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
            static std::function<void(ui::WidgetTreeNode&, scene::SceneNode*)> processNode = [this](ui::WidgetTreeNode& rootNode, scene::SceneNode* root) mutable
                {
                    ui::WidgetTreeNode sceneNode = ui::WidgetTreeNode(root->m_name, root->m_children.empty() ? ui::WidgetTreeNode::TreeNodeFlag::NoCollapsed : 0)
                        .setOnCreated([this, root](ui::Widget* w)
                            {
                                m_widgetItems.emplace(root, static_cast<ui::WidgetTreeNode*>(w));
                            })
                        .setToolTip(true, root->m_name)
                        .setUserData(root)
                        .setSelected(false)
                        .setOnClickEvent([this](ui::Widget* w, void* data) -> void
                            {
                                scene::SceneNode* node = static_cast<scene::SceneNode*>(data);
                                m_gameEventRecevier->sendEvent(new EditorSelectionEvent(node));
                            });

                    for (scene::SceneNode* node : root->m_children)
                    {
                        processNode(sceneNode, node);
                    }

                    if (root->m_children.empty())
                    {
                        ui::WidgetHorizontalLayout layout;
                        layout.addWidget(std::move(sceneNode));
                        layout.addWidget(ui::WidgetHorizontalLayout()
                            .setHAlignment(ui::WidgetLayout::HorizontalAlignment::AlignmentRight)
                            .setFontSize(ui::WidgetLayout::SmallFont)
                            .addWidget(ui::WidgetCheckBox("Visible", root->m_visible)
                                .setOnChangedValueEvent([this, root](ui::Widget* w, bool val) -> void
                                    {
                                        root->m_visible = val;
                                    }))
                            .addWidget(ui::WidgetCheckBox("Debug", false)
                                .setOnChangedValueEvent([this, root](ui::Widget* w, bool val) -> void
                                    {
                                        root->m_debug = val;
                                    })
                            )
                        );

                        rootNode.addWidget(std::move(layout));
                    }
                    else
                    {
                        rootNode.addWidget(std::move(sceneNode));
                    }
                };

            for (scene::SceneNode* node : m_sceneData->getNodeList())
            {
                processNode(sceneNode, node);
            }

            ////todo node list
            //m_widgetItems.resize(m_sceneData->m_generalRenderList.size(), nullptr);
            //for (u32 index = 0; index < m_sceneData->m_generalRenderList.size(); ++index)
            //{
            //    scene::NodeEntry& item = *m_sceneData->m_generalRenderList[index];
            //    //if (item._type == scene::MaterialType::Debug ||
            //    //    item._type == scene::MaterialType::DirectionLight || 
            //    //    item._type == scene::MaterialType::PunctualLights)
            //    //{
            //    //    //skip debug object list
            //    //    continue;
            //    //}
            //}
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
            if (m_selectedWidget)
            {
                m_selectedWidget->setSelected(false);
            }
            
            if (auto found = m_widgetItems.find(selectionEvent->_node); found != m_widgetItems.cend())
            {
                m_selectedWidget = found->second;
                m_selectedWidget->setSelected(true);
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
