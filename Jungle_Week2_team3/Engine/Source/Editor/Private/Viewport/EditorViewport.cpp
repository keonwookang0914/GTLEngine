#include "Viewport/EditorViewport.h"
#include "Viewport/EditorViewportClient.h"
#include "EngineGlobals.h"
#include "Logging/LogMacros.h"
#include "RendererModule.h"
#include "Runtime/Core/CoreGlobals.h"
#include "imgui.h"

bool FEditorViewport::OnKeyDown(EKey Key, bool bIsRepeat)
{
    if (ImGui::GetIO().WantCaptureKeyboard)
    {
        return true;
    }

    if (ViewportClient == nullptr)
    {
        return false;
    }

    return ViewportClient->InputKey(Key, bIsRepeat ? EInputEvent::Repeat : EInputEvent::Pressed);
}

bool FEditorViewport::OnKeyUp(EKey Key)
{
    if (ViewportClient == nullptr)
    {
        return false;
    }

    return ViewportClient->InputKey(Key, EInputEvent::Released);
}

bool FEditorViewport::OnMouseDown(EKey Button, int32 X, int32 Y)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return true;
    }

    if (ViewportClient == nullptr)
    {
        return false;
    }

    ViewportClient->MouseMove(X, Y);
    return ViewportClient->InputKey(Button, EInputEvent::Pressed);
}

bool FEditorViewport::OnMouseUp(EKey Button, int32 X, int32 Y)
{
    if (ViewportClient == nullptr)
    {
        return false;
    }

    ViewportClient->MouseMove(X, Y);

    const bool bHandled = ViewportClient->InputKey(Button, EInputEvent::Released);

    if (Button == EKey::LeftMouseButton &&
        ViewportClient->GetReleaseType(X, Y) == EPointerReleaseType::Click)
    {
        ViewportClient->ProcessClick(X, Y);
    }

    return bHandled;
}

bool FEditorViewport::OnMouseDoubleClick(EKey Button, int32 X, int32 Y)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return true;
    }

    if (ViewportClient == nullptr)
    {
        return false;
    }

    ViewportClient->MouseMove(X, Y);

    const bool bHandled = ViewportClient->InputKey(Button, EInputEvent::DoubleClick);

    if (Button == EKey::LeftMouseButton)
    {
        ViewportClient->ProcessClick(X, Y);
    }

    return bHandled;
}

bool FEditorViewport::OnMouseMove(int32 X, int32 Y)
{
    if (ViewportClient == nullptr)
    {
        return false;
    }

    return ViewportClient->MouseMove(X, Y);
}

bool FEditorViewport::OnRawMouseMove(int32 DeltaX, int32 DeltaY)
{
    if (ViewportClient == nullptr)
    {
        return false;
    }

    return ViewportClient->CapturedMouseMove(DeltaX, DeltaY);
}

bool FEditorViewport::OnMouseWheel(float Delta, int32 X, int32 Y)
{
    if (ViewportClient == nullptr)
    {
        return false;
    }

    ViewportClient->MouseMove(X, Y);
    return ViewportClient->InputAxis(EKey::MouseWheelAxis, Delta);
}

bool FEditorViewport::OnSizeChanged(int32 Width, int32 Height)
{
    if (GRenderer)
    {
        GRenderer->OnWindowResized(Width, Height);
        return true;
    }
    return false;
}

FEditorViewportClient *FEditorViewport::GetEditorViewportClient() { return ViewportClient; }

void FEditorViewport::SetEditorViewportClient(FEditorViewportClient *InViewportClient)
{
    ViewportClient = InViewportClient;
}

void FEditorViewport::OnFocusLost()
{
    if (ViewportClient != nullptr)
    {
        ViewportClient->ResetInputState();
    }
}