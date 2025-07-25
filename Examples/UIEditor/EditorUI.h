#pragma once

#include "Common.h"
#include "UI/WidgetHandler.h"

#include "EditorScene.h"

namespace v3d
{
namespace renderer
{
    class Texture2D;
} //namespace renderer

namespace editor
{

    class UI
    {
    public:

        static ui::WidgetMenuBar& constructMainMenu(ui::WidgetHandler* handler);

        static void constructTransformProp(ui::WidgetHandler* handler);

        static void constuctTestUIWindow(ui::WidgetHandler* handler, EditorScene* scene);
    };

} //namespace editor
} //namespace v3d