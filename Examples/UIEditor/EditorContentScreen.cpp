#include "EditorContentScreen.h"

EditorContentScreen::EditorContentScreen() noexcept
{
}

EditorContentScreen::~EditorContentScreen()
{
}

void EditorContentScreen::build()
{

}

void EditorContentScreen::update(f32 dt)
{
}

void EditorContentScreen::handleNotify(const utils::Reporter<EditorReport>* reporter, const EditorReport& data)
{
}

bool EditorContentScreen::handleGameEvent(event::GameEventHandler* handler, const event::GameEvent* event)
{
    return false;
}

bool EditorContentScreen::handleInputEvent(v3d::event::InputEventHandler* handler, const v3d::event::InputEvent* event)
{
    return false;
}
