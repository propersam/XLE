// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#pragma warning(disable:4564)

#include "NativeManipulators.h"
#include "ManipulatorUtils.h"
#include "EditorInterfaceUtils.h"
#include "SimpleRenderingContext.h"
#include "GUILayerUtil.h"
#include "LevelEditorScene.h"       // for getting the intersection scene from the IViewContext
#include "NativeEngineDevice.h"
#include "../ToolsRig/IManipulator.h"
#include "../../RenderOverlays/DebuggingDisplay.h"
#include "../../RenderCore/IDevice.h"
#include "../../PlatformRig/BasicSceneParser.h"

extern "C" __declspec(dllimport) short __stdcall GetKeyState(int nVirtKey);

namespace GUILayer
{
    static void SetupModifierKeys(RenderOverlays::DebuggingDisplay::InputSnapshot& evnt)
    {
        using namespace RenderOverlays::DebuggingDisplay;
        typedef InputSnapshot::ActiveButton ActiveButton;
        static auto shift = KeyId_Make("shift");
        static auto control = KeyId_Make("control");
        static auto alt = KeyId_Make("alt");

        if (GetKeyState(0x10) < 0) evnt._activeButtons.push_back(ActiveButton(shift, false, true));
        if (GetKeyState(0x11) < 0) evnt._activeButtons.push_back(ActiveButton(control, false, true));
        if (GetKeyState(0x12) < 0) evnt._activeButtons.push_back(ActiveButton(alt, false, true));
    }

    bool NativeManipulatorLayer::MouseMove(IViewContext^ vc, Point scrPt)
    {
        using namespace RenderOverlays::DebuggingDisplay;
		InputSnapshot evnt(0, 0, 0, Coord2(scrPt.X, scrPt.Y), Coord2(0, 0));
			// "return true" has two effects --
			//		1. sets the cursor to a moving cursor
			//		2. turns mouse down into "drag-begin" event
		return SendInputEvent(vc, evnt);
    }

    void NativeManipulatorLayer::Render(SimpleRenderingContext^ context)
    {
        auto underlying = _manipContext->GetNativeManipulator();
        if (!underlying) return;
        underlying->Render(context->GetThreadContext(), context->GetParsingContext());
    }

    void NativeManipulatorLayer::OnBeginDrag() { _pendingBeginDrag = true; }
    void NativeManipulatorLayer::OnDragging(IViewContext^ vc, Point scrPt) 
	{
			//  We need to create a fake "mouse over" event and pass it through to
			//  the currently selected manipulator. We might also need to set the state
			//  for buttons and keys pressed down.
            //  For the first "OnDragging" operation after a "OnBeginDrag", we should
            //  emulate a mouse down event.
		using namespace RenderOverlays::DebuggingDisplay;
        auto btnState = 1<<0;
		InputSnapshot evnt(
			btnState, _pendingBeginDrag ? btnState : 0, 0,
			Coord2(scrPt.X, scrPt.Y), Coord2(0, 0));
        SetupModifierKeys(evnt);

		SendInputEvent(vc, evnt);
        _pendingBeginDrag = false;
	}

    void NativeManipulatorLayer::OnEndDrag(IViewContext^ vc, Point scrPt) 
	{
            // Emulate a "mouse up" operation 
        using namespace RenderOverlays::DebuggingDisplay;
        auto btnState = 1<<0;
		InputSnapshot evnt(
			0, btnState, 0,
			Coord2(scrPt.X, scrPt.Y), Coord2(0, 0));
        SetupModifierKeys(evnt);
        SendInputEvent(vc, evnt);
	}

    void NativeManipulatorLayer::OnMouseWheel(IViewContext^ vc, Point scrPt, int delta)
    {
        using namespace RenderOverlays::DebuggingDisplay;
		InputSnapshot evnt(
			0, 0, delta,
			Coord2(scrPt.X, scrPt.Y), Coord2(0, 0));
        SetupModifierKeys(evnt);
        SendInputEvent(vc, evnt);
    }

    NativeManipulatorLayer::NativeManipulatorLayer(ActiveManipulatorContext^ manipContext) 
    : _manipContext(manipContext)
    {
        _pendingBeginDrag = false;
    }

    NativeManipulatorLayer::~NativeManipulatorLayer()
    {
    }

    bool NativeManipulatorLayer::SendInputEvent(
        IViewContext^ vc, 
		const RenderOverlays::DebuggingDisplay::InputSnapshot& evnt)
	{
		auto underlying = _manipContext->GetNativeManipulator();
        if (!underlying) return false;

		auto hitTestContext = CreateIntersectionTestContext(
			vc->EngineDevice, nullptr,
			vc->Camera, vc->ViewportSize.Width, vc->ViewportSize.Height);

        // Only way to get the intersection scene is via the SceneManager
        // But what if we don't want to use a SceneManager. Is there a better
        // way to get the intersection scene?
		auto hitTestScene = vc->SceneManager->GetIntersectionScene();

		underlying->OnInputEvent(evnt, hitTestContext->GetNative(), hitTestScene->GetNative());
		delete hitTestContext;
		delete hitTestScene;
		return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

    IManipulatorSet^ ActiveManipulatorContext::ManipulatorSet::get() { return _manipulatorSet; }
    void ActiveManipulatorContext::ManipulatorSet::set(IManipulatorSet^ value)
    {
        if (value != _manipulatorSet) {
            delete _manipulatorSet;
            _manipulatorSet = value;
            OnManipulatorSetChange(this, nullptr);
        }
    }

    String^ ActiveManipulatorContext::ActiveManipulator::get() { return _activeManipulator; }
    void ActiveManipulatorContext::ActiveManipulator::set(String^ value)
    {
        if (value != _activeManipulator) {
            auto oldNativeManip = GetNativeManipulator();
            _activeManipulator = value;
            auto newNativeManip = GetNativeManipulator();

            if (oldNativeManip != newNativeManip) {
                if (oldNativeManip) oldNativeManip->SetActivationState(false);
                if (newNativeManip) newNativeManip->SetActivationState(true);
            }
            OnActiveManipulatorChange(this, nullptr);
        }
    }

    ActiveManipulatorContext::ActiveManipulatorContext()
    {
        _manipulatorSet = nullptr;
        _activeManipulator = "";
    }

    ActiveManipulatorContext::~ActiveManipulatorContext()
    {
        delete _manipulatorSet;
    }

    ToolsRig::IManipulator* ActiveManipulatorContext::GetNativeManipulator()
    {
        auto set = ManipulatorSet;
        auto active = ActiveManipulator;
		if (!set || !active) return nullptr;
		return set->GetManipulator(active).get();
    }

}
