#include "CaptureProfile.h"

#if ENABLE_RENDERDOC_PROFILE
#include <C:/Program Files/RenderDoc/renderdoc_app.h>

typedef int(RENDERDOC_CC* LPRENDERDOC_GetAPI)(RENDERDOC_Version version, void** outAPIPointers);

// API interface
RENDERDOC_API_1_6_0* g_renderdoc = nullptr;
HMODULE g_renderdocModule = nullptr;
#endif //ENABLE_RENDERDOC_PROFILE

namespace v3d
{
namespace renderer
{

#if ENABLE_RENDERDOC_PROFILE

bool CaptureRenderdocProfile::create()
{
    // At init, on windows
    g_renderdocModule = LoadLibraryA("C:/Program Files/RenderDoc/renderdoc.dll");
    if (g_renderdocModule)
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(g_renderdocModule, "RENDERDOC_GetAPI");
        if (!RENDERDOC_GetAPI)
        {
            return false;
        }

        s32 ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&g_renderdoc);
        if (g_renderdoc)
        {
            g_renderdoc->MaskOverlayBits(0, 0);
            g_renderdoc->UnloadCrashHandler();
            g_renderdoc->SetCaptureKeys(nullptr, 0);
            g_renderdoc->SetFocusToggleKeys(nullptr, 0);
            g_renderdoc->SetCaptureOptionU32(eRENDERDOC_Option_DebugOutputMute, 0);

            m_loaded = true;
            showOverlay(true);

            return true;
        }
    }

    return false;
}

void CaptureRenderdocProfile::destroy()
{
    ASSERT(!m_capturing, "must be false");
    g_renderdoc = nullptr;
    if (g_renderdocModule)
    {
        FreeLibrary(g_renderdocModule);
        g_renderdocModule = nullptr;
    }

    m_loaded = false;
}

void CaptureRenderdocProfile::beginCapture(platform::Window* window)
{
    if (g_renderdoc)
    {
        g_renderdoc->StartFrameCapture(nullptr, (RENDERDOC_WindowHandle)window->getWindowHandle());
        m_capturing = true;
    }
}

void CaptureRenderdocProfile::endCapture(platform::Window* window)
{
    if (g_renderdoc && m_capturing)
    {
        g_renderdoc->EndFrameCapture(nullptr, (RENDERDOC_WindowHandle)window->getWindowHandle());
        m_capturing = false;
    }
}

void CaptureRenderdocProfile::capture(const std::string& file)
{
    if (g_renderdoc && !m_capturing)
    {
        g_renderdoc->SetCaptureFilePathTemplate(file.c_str());
    }
}

void CaptureRenderdocProfile::showOverlay(bool show)
{
    if (g_renderdoc)
    {
        g_renderdoc->MaskOverlayBits(0, (show) ? eRENDERDOC_Overlay_Default : 0);
    }
}

#endif //ENABLE_RENDERDOC_PROFILE

} //namespace renderer
} //namespace v3d