#pragma once

#include "Common.h"
#include "UI/WigetHandler.h"

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

        static ui::WigetMenuBar& constructMainMenu(ui::WigetHandler* handler);

        static void constructTransformProp(ui::WigetHandler* handler);

        static void constuctTestUIWindow(ui::WigetHandler* handler, renderer::Texture2D* texture);
    };

} //namespace editor
} //namespace v3d