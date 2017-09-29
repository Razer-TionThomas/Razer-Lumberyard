
#include "StdAfx.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <Editor/QtViewPaneManager.h> // For calendar demo
#include <QCalendarWidget> // For calendar demo
#include <AzCore/Math/Sfmt.h>

#include "ChromaSystemComponent.h"

#include "IRenderer.h"

#include <qobject.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QMovie>

namespace Chroma
{
	ChromaSystemComponent::ChromaSystemComponent() {
		chromaDeviceType = 0;
		chromaEffectValue = 0;
		m_currFrameNum = 1;
		ChromaKey = ChromaSDK::Keyboard::RZKEY_ESC;
		ChromaLed = ChromaSDK::Mouse::RZLED_NONE;
		g_effectBrightness = 1.0;
		g_effectSpeed = 300;
		m_currEffect = GUID_NULL;
		repeatAnimation = FALSE;

		for (int d = 0; d < MAXDEVICES; d++)
			m_currDeviceEffects[d] = GUID_NULL;
	}

	ChromaSystemComponent::~ChromaSystemComponent() {

	}
	
	void ChromaSystemComponent::Init()
	{
	}

	void ChromaSystemComponent::Activate()
	{
		ChromaRequestBus::Handler::BusConnect();

		CrySystemEventBus::Handler::BusConnect(); // For calendar demo

		AZ::TickBus::Handler::BusConnect();		  // connect to the TickBus

		g_ChromaSDKImpl.Initialize();
	}

	void ChromaSystemComponent::Deactivate()
	{
		ChromaRequestBus::Handler::BusDisconnect();
	}

	void ChromaSystemComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serialize->Class<ChromaSystemComponent, AZ::Component>()
				->Version(0)
				//->SerializerForEmptyClass();
				->Field("Effect Brightness", &ChromaSystemComponent::g_effectBrightness)
				->Field("Effect Speed", &ChromaSystemComponent::g_effectSpeed)
				->Field("Color", &ChromaSystemComponent::ChromaColor)
				->Field("Color", &ChromaSystemComponent::repeatAnimation)
				->Field("Select Device", &ChromaSystemComponent::chromaDeviceType)
				->Field("Stop Effect", &ChromaSystemComponent::stopEffect)
				->Field("Clear Effect", &ChromaSystemComponent::clearEffects)
				->Field("Select Effect Preset", &ChromaSystemComponent::chromaEffectValue)
				->Field("Play Preset Effect", &ChromaSystemComponent::playPresetEffect)
				->Field("Current Frame", &ChromaSystemComponent::m_currFrameNum)
				->Field("Chroma Key", &ChromaSystemComponent::ChromaKey)
				->Field("Chroma Led", &ChromaSystemComponent::ChromaLed)
				->Field("Set Key", &ChromaSystemComponent::setKey)
				->Field("Set Led", &ChromaSystemComponent::setLed)
				->Field("Keyboard Row", &ChromaSystemComponent::cRow)
				->Field("Keyboard Col", &ChromaSystemComponent::cCol)
				->Field("Set Row", &ChromaSystemComponent::setRow)
				->Field("Set Col", &ChromaSystemComponent::setCol)
				->Field("Jump To Frame", &ChromaSystemComponent::newFrame)
				->Field("Next Frame", &ChromaSystemComponent::nextFrame)
				->Field("Prev Frame", &ChromaSystemComponent::prevFrame)
				->Field("Copy Frame", &ChromaSystemComponent::copyFrame)
				->Field("Paste Frame", &ChromaSystemComponent::pasteFrame)
				->Field("Fill Frame", &ChromaSystemComponent::fillFrame)
				->Field("Clear Frame", &ChromaSystemComponent::clearFrame)
				->Field("Clear All Frames", &ChromaSystemComponent::clearFrames)
				->Field("Load Image", &ChromaSystemComponent::importImage)
				->Field("Load Animation", &ChromaSystemComponent::importAnimation)
				->Field("Play Custom", &ChromaSystemComponent::playCustomEffect)
				->Field("Play All Custom", &ChromaSystemComponent::playAllCustomEffect)
				;

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<ChromaSystemComponent>("Chroma", "Adds Chroma lighting effects to game objects")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					// ->Attribute(AZ::Edit::Attributes::Category, "") Set a category
					->Attribute(AZ::Edit::Attributes::Category, "Peripheral Lighting")
					//->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaSystemComponent::chromaDeviceType, "Select Device", "Select Chroma Device")
						->EnumAttribute(ChromDeviceType::KEYBOARD, "Keyboard")
						->EnumAttribute(ChromDeviceType::MOUSE, "Mouse")
						->EnumAttribute(ChromDeviceType::MOUSEPAD, "Mousepad")
						->EnumAttribute(ChromDeviceType::HEADSET, "Headset")
						->EnumAttribute(ChromDeviceType::KEYPAD, "Keypad")
						->EnumAttribute(ChromDeviceType::CHROMALINK, "ChromaLink")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::SetChromaDeviceType)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaSystemComponent::ChromaColor, "Chroma Color", "Chroma Color picker")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::SetEffectColor)
					->DataElement(AZ::Edit::UIHandlers::Slider, &ChromaSystemComponent::g_effectBrightness, "Brightness Control", "Use to set color brightness for current effect")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::SetEffectBrightness)
						->Attribute(AZ::Edit::Attributes::Min, 0.0f)
						->Attribute(AZ::Edit::Attributes::Max, 1.0f)
						->Attribute(AZ::Edit::Attributes::Step, 0.01f)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaSystemComponent::g_effectSpeed, "Effect Speed", "Use to change speed of animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::SetEffectSpeed)
						->Attribute(AZ::Edit::Attributes::Min, 0)
						->Attribute(AZ::Edit::Attributes::Max, 5000)
						->Attribute(AZ::Edit::Attributes::ValueText, &ChromaSystemComponent::g_effectSpeed)
					->DataElement(AZ::Edit::UIHandlers::CheckBox, &ChromaSystemComponent::repeatAnimation, "Repeat Animation", "Set to repeat animation indefintely")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::stopEffect, "Stop Effect", "Stop the current animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::StopEffect)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::clearEffects, "Clear Effects", "Clear All Chroma Devices")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ClearEffects)
					->ClassElement(AZ::Edit::ClassElements::Group, "Chroma Preset Effects")
						->Attribute(AZ::Edit::Attributes::AutoExpand, false)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaSystemComponent::chromaEffectValue, "Select Preset Effect", "Select Chroma Effect Preset")
						->EnumAttribute(ChromaEffectValue::STATIC, "Static")
						->EnumAttribute(ChromaEffectValue::FLASH, "Alert")
						->EnumAttribute(ChromaEffectValue::WAVE, "Wave")
						->EnumAttribute(ChromaEffectValue::BREATHING, "Breathing")
						->EnumAttribute(ChromaEffectValue::RANDOM, "Random")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::playPresetEffect, "Play Preset Effect", "Plays selected preset effect")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PlayPresetEffect)
					->ClassElement(AZ::Edit::ClassElements::Group, "Chroma Custom Effects")
						->Attribute(AZ::Edit::Attributes::AutoExpand, false)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaSystemComponent::m_currFrameNum, "Current Frame", "Displays Current Chroma Frame")
						->Attribute(AZ::Edit::Attributes::ReadOnly, true)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaSystemComponent::ChromaKey, "Select Key", "Select Key to Color")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_ESC, "ESC (VK_ESC)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F1, "F1 (VK_F1)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F2, "F2 (VK_F2)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F3, "F3 (VK_F3)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F4, "F4 (VK_F4)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F5, "F5 (VK_F5)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F6, "F6 (VK_F6)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F7, "F7 (VK_F7)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F8, "F8 (VK_F8)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F9, "F9 (VK_F9)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F10, "F10 (VK_F10)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F11, "F11 (VK_F11)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F12, "F11 (VK_F12)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_0, "0 (VK_0)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_1, "1 (VK_1)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_2, "2 (VK_2)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_3, "3 (VK_3)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_4, "4 (VK_4)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_5, "5 (VK_5)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_6, "6 (VK_6)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_7, "7 (VK_7)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_8, "8 (VK_8)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_9, "9 (VK_9)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_A, "A (VK_A)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_B, "B (VK_B)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_C, "C (VK_C)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_D, "D (VK_D)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_E, "E (VK_E)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_F, "F (VK_F)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_G, "G (VK_G)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_H, "H (VK_H)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_I, "I (VK_I)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_J, "J (VK_J)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_K, "K (VK_K)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_L, "L (VK_L)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_M, "M (VK_M)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_N, "N (VK_N)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_O, "O (VK_O)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_P, "P (VK_P)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_Q, "Q (VK_Q)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_R, "R (VK_R)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_S, "S (VK_S)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_T, "T (VK_T)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_U, "U (VK_U)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_V, "V (VK_V)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_W, "W (VK_W)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_X, "X (VK_X)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_Y, "Y (VK_Y)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_Z, "Z (VK_Z)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMLOCK, "NUMLOCK (VK_NUMLOCK)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD0, "Numpad 0 (VK_NUMPAD0)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD1, "Numpad 1 (VK_NUMPAD1)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD2, "Numpad 2 (VK_NUMPAD2)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD3, "Numpad 3 (VK_NUMPAD3)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD4, "Numpad 4 (VK_NUMPAD4)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD5, "Numpad 5 (VK_NUMPAD5)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD6, "Numpad 6 (VK_NUMPAD6)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD7, "Numpad 7 (VK_NUMPAD7)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD8, "Numpad 8 (VK_NUMPAD8)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD9, "Numpad 9 (VK_NUMPAD9)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD_DIVIDE, "Divide (VK_DIVIDE)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD_MULTIPLY, "Multiply (VK_MULTIPLY)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD_SUBTRACT, "Subtract (VK_SUBTRACT)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD_ADD, "Add (VK_ADD)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD_ENTER, "Enter (VK_ENTER)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_NUMPAD_DECIMAL, "Decimal (VK_DECIMAL)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_PRINTSCREEN, "Print Screen (VK_PRINT)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_SCROLL, "Scroll Lock (VK_SCROLL)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_PAUSE, "Pause (VK_PAUSE)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_INSERT, "Insert (VK_INSERT)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_HOME, "Home (VK_HOME)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_PAGEUP, "Page Up (VK_PRIOR)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_DELETE, "Delete (VK_DELETE)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_END, "End (VK_END)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_PAGEDOWN, "Page Down (VK_NEXT)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_UP, "Up (VK_UP)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_LEFT, "Left (VK_LEFT)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_DOWN, "Down (VK_DOWN)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_RIGHT, "Right (VK_RIGHT)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_TAB, "Tab (VK_TAB)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_CAPSLOCK, "Caps Lock (VK_CAPITAL)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_BACKSPACE, "Backspace (VK_BACK)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_ENTER, "Enter (VK_RETURN)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_LCTRL, "Left Control (VK_LCONTROL)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_LWIN, "Left Window (VK_LWIN)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_LALT, "Left Alt (VK_LMENU)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_SPACE, "Spacebar (VK_SPACE)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_RALT, "Right Alt")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_FN, "Function key")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_RMENU, "Right Menu (VK_APPS)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_RCTRL, "Right Control (VK_RCONTROL)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_LSHIFT, "Left Shift  (VK_LSHIFT)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_RSHIFT, "Right Shift (VK_RSHIFT)")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_MACRO1, "Macro Key 1")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_MACRO2, "Macro Key 2")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_MACRO3, "Macro Key 3")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_MACRO4, "Macro Key 4")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_MACRO5, "Macro Key 5")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_1, "~")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_2, "-")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_3, "=")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_4, "[")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_5, "]")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_6, "\\")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_7, ";")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_8, "'")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_9, ",")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_10, ".")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_OEM_11, "/")
						->EnumAttribute(ChromaSDK::Keyboard::RZKEY_INVALID, "Invalid Keys")
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaSystemComponent::ChromaLed, "Select Led", "Select Led (non-keyboard devices) to Color")
						->EnumAttribute(ChromaLEDs::LED0, "LED 0 | ALL")
						->EnumAttribute(ChromaLEDs::LED1, "LED 1 | ALL")
						->EnumAttribute(ChromaLEDs::LED2, "LED 2 | ALL")
						->EnumAttribute(ChromaLEDs::LED3, "LED 3 | ALL")
						->EnumAttribute(ChromaLEDs::LED4, "LED 4 | ALL")
						->EnumAttribute(ChromaLEDs::LED5, "LED 5 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED6, "LED 6 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED7, "LED 7 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED8, "LED 8 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED9, "LED 9 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED10, "LED 10 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED11, "LED 11 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED12, "LED 12 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED13, "LED 13 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED14, "LED 14 | M, MP, KP")
						->EnumAttribute(ChromaLEDs::LED15, "LED 15 | M, KP")
						->EnumAttribute(ChromaLEDs::LED16, "LED 16 | M, KP")
						->EnumAttribute(ChromaLEDs::LED17, "LED 17 | M, KP")
						->EnumAttribute(ChromaLEDs::LED18, "LED 18 | M, KP")
						->EnumAttribute(ChromaLEDs::LED19, "LED 19 | M, KP")
						->EnumAttribute(ChromaLEDs::LED20, "LED 20 | M")
						->EnumAttribute(ChromaLEDs::LED21, "LED 21 | M")
						->EnumAttribute(ChromaLEDs::LED22, "LED 22 | M")
						->EnumAttribute(ChromaLEDs::LED23, "LED 23 | M")
						->EnumAttribute(ChromaLEDs::LED24, "LED 24 | M")
						->EnumAttribute(ChromaLEDs::LED25, "LED 25 | M")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::setKey, "Set Key Color", "Color Selected Key")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::SetKey)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::setLed, "Set Led Color", "Color Selected Key")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::SetLed)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaSystemComponent::cRow, "Keyboard Row", "Specify Keyboard Row to Paint")
						->Attribute(AZ::Edit::Attributes::Min, 1)
						->Attribute(AZ::Edit::Attributes::Max, 6)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaSystemComponent::cCol, " Keyboard Column", "Specify Keyboard Column to Paint")
						->Attribute(AZ::Edit::Attributes::Min, 1)
						->Attribute(AZ::Edit::Attributes::Max, 22)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::setRow, "Paint Row", "Color Entire Row On Keyboard")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PaintRow)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::setCol, "Paint Column", "Color Entire Column On Keyboard")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PaintCol)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaSystemComponent::newFrame, "Jump to Frame", "Frame must be <100")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::JumpToFrame)
						->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, true)
						->Attribute(AZ::Edit::Attributes::Min, 1)
						->Attribute(AZ::Edit::Attributes::Max, 100)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::nextFrame, "Next Frame", "Increment Frame Count")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::IncrementFrame)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::prevFrame, "Prev Frame", "Decrement Frame Count")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::DecrementFrame)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::copyFrame, "Copy Frame", "Copy Contents of Current Chroma Frame")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::CopyFrame)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::pasteFrame, "Paste Frame", "Paste Contents of Current Chroma Frame")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PasteFrame)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::fillFrame, "Fill Frame", "Color all Device Leds Selected Color")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::FillFrame)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::clearFrame, "Clear Frame", "Clears the current chroma frame")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ClearFrame)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::clearFrames, "Clear All Frames", "Clears all current frames")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ClearAllFrames)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::importImage, "Import Image", "Import jpg, png, bmp")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::LoadSingleImage)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::importAnimation, "Import Animation", "Import gif animations")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::LoadAnimation)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::playCustomEffect, "Play Custom Single Device", "Plays currrent device custom animation frames")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PlayCustomAnimation)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::playAllCustomEffect, "Play Custom All Devices", "Plays custom animation on all devices simultaneously")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PlayAllCustomAnimation)
					;
			}
		}

		else if (AZ::BehaviorContext* behavior = azrtti_cast<AZ::BehaviorContext*>(context)) {
			behavior->EBus<ChromaRequestBus>("ChromaRequestBus")
				->Event("ChromaSDKInit", &ChromaRequestBus::Events::ChromaSDKInit)
				->Event("ChromaSDKUnInit", &ChromaRequestBus::Events::ChromaSDKUnInit)
				->Event("PlayChromaEffect", &ChromaRequestBus::Events::PlayChromaEffect)
				->Event("StopChromaEffect", &ChromaRequestBus::Events::StopChromaEffect)
			;
		}
	}

	// Ebus Requests
	void ChromaSystemComponent::ChromaSDKInit() {
		AZ_Printf("Chroma", "Recevied a chroma init request via Ebus!!");
	}

	void ChromaSystemComponent::ChromaSDKUnInit() {
		AZ_Printf("Chroma", "Recevied a chroma uninit request via Ebus!!");
	}

	void ChromaSystemComponent::PlayChromaEffect() {
		AZ_Printf("Chroma", "Recevied a play chroma effect request via Ebus!!");
	}

	void ChromaSystemComponent::StopChromaEffect() {
		AZ_Printf("Chroma", "Recevied a stop chroma effect request via Ebus!!");
	}

	void ChromaSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("ChromaService", 0xd92ddd51));
	}

	void ChromaSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("ChromaService", 0xd92ddd51));
	}

	void ChromaSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void ChromaSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}

	void ChromaSystemComponent::OnCrySystemInitialized(ISystem&, const SSystemInitParams&) // For calendar demo
	{
		if (QtViewPaneManager* paneManager = QtViewPaneManager::instance())
		{
			//ViewPaneFactory paneFactory = []() { return new QCalendarWidget();
			////ShowChromaEffects();
			//};
			//paneManager->RegisterPane("Static Effect", "Chroma", paneFactory);
			//paneManager->RegisterPane("Breathing Effect", "Chroma", paneFactory);
			//paneManager->RegisterPane("Wave Effect", "Chroma", paneFactory);
		}
	}

	void ChromaSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time) {

	}

	COLORREF ChromaSystemComponent::GetCOLORREFValue(AZ::Color color, double brightness) {
		COLORREF retColor = 0;
		
		AZ::u8 red = color.GetR8()* brightness;
		AZ::u8 green = color.GetG8() * brightness;
		AZ::u8 blue = color.GetB8() * brightness;

		retColor = RGB(red, green, blue);

		return retColor;

	}

	void ChromaSystemComponent::SetEffectBrightness() {
		float test = g_effectBrightness;
	}

	void ChromaSystemComponent::SetEffectSpeed() {

	}

	void ChromaSystemComponent::StopEffect() {
		if (m_currEffect != GUID_NULL)
			g_ChromaSDKImpl.StopEffectImpl(m_currEffect);

		for (int j = 0; j < MAXDEVICES; j++) {
			if (m_currDeviceEffects[j] != GUID_NULL) {
				g_ChromaSDKImpl.StopEffectImpl(m_currDeviceEffects[j]);
			}
		}
	}

	void ChromaSystemComponent::ClearEffects() {
		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_NONE, NULL, NULL);

		copyKeyboardEffect = {};
	}

	AZ::Crc32 ChromaSystemComponent::SetChromaDeviceType(AZ::u32 deviceType) {
		g_ChromaSDKImpl.mDeviceType = deviceType;

		m_currFrameNum = 1;

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaSystemComponent::PlayPresetEffect() {
		switch (chromaEffectValue) {
		case 0:			// Static Effect 
		{
			ShowStaticEffect();
			break;
		}
		case 1:			// Flash Effect 
		{
			ShowFlashEffect();
			break;
		}
		case 2:			// Wave Effect 
		{
			ShowWaveEffect();
			break;
		}
		case 3:			// Breathing Effect 
		{
			ShowBreathingEffect();
			break;
		}
		case 4:			// Random Effect 
		{
			ShowRandomEffect();
			break;
		}

		}

	}

	void ChromaSystemComponent::ShowWaveEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE KeyboardEffect = {};

			RZEFFECTID frames[ChromaSDK::Keyboard::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				KeyboardEffect = {};

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					if ((r == 0) && (c == 20))
						continue;   // Skip the Razer logo.
					KeyboardEffect.Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &KeyboardEffect, &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);
			}

		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2 MouseEffect = {};

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
			{
				MouseEffect = {};

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					MouseEffect.Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &MouseEffect, &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE MousePadEffect = {};

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				MousePadEffect = {};
				MousePadEffect.Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &MousePadEffect, &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);
			}

		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			ChromaSDK::Headset::CUSTOM_EFFECT_TYPE HeadsetEffect = {};

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				HeadsetEffect = {};
				HeadsetEffect.Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &HeadsetEffect, &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			ChromaSDK::Keypad::CUSTOM_EFFECT_TYPE KeypadEffect = {};

			RZEFFECTID frames[ChromaSDK::Keypad::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
			{
				KeypadEffect = {};

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					KeypadEffect.Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &KeypadEffect, &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);
			}
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE ChromalinkEffect = {};

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				ChromalinkEffect = {};
				ChromalinkEffect.Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &ChromalinkEffect, &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);
			}
		}

		
		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;
	}

	void ChromaSystemComponent::ShowBreathingEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		RZEFFECTID Frames[11] = { 0 };
		double Brightness[11] = { 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0 };
		double brightness = 0.0;

		UINT index = 0;
		UINT count = 0;
		while (count < 11)		// Loop through 5 times (5 * number of brightness values (i.e 11))
		{
			brightness = Brightness[index];

			if (ChromaSystemComponent::chromaDeviceType == 0) {
				ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE KeyboardEffect = {};

				for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
					{
						KeyboardEffect.Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &KeyboardEffect, &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 1) {
				ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2 MouseEffect = {};

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
					{
						MouseEffect.Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &MouseEffect, &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 2) {
				ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE MousepadEffect = {};

				for (UINT r = 0; r < ChromaSDK::Mousepad::MAX_LEDS; r++)
				{
					MousepadEffect.Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &MousepadEffect, &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 3) {
				ChromaSDK::Headset::CUSTOM_EFFECT_TYPE HeadsetEffect = {};

				for (UINT r = 0; r < ChromaSDK::Headset::MAX_LEDS; r++)
				{
					HeadsetEffect.Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &HeadsetEffect, &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 4) {
				ChromaSDK::Keypad::CUSTOM_EFFECT_TYPE KeypadEffect = {};

				for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
					{
						KeypadEffect.Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &KeypadEffect, &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 5) {
				ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE ChromaLinkEffect = {};

				for (UINT r = 0; r < ChromaSDK::ChromaLink::MAX_LEDS; r++)
				{
					ChromaLinkEffect.Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &ChromaLinkEffect, &Frames[index]);
			}

			index++;
			count++;
		}

		for (int i = 0; i < 11; i++) {
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, Frames[i], g_effectSpeed);
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;
	}
	
	void ChromaSystemComponent::ShowStaticEffect() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			ChromaSDK::Keyboard::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_STATIC, &staticEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			ChromaSDK::Mouse::STATIC_EFFECT_TYPE staticEffect = {};
			staticEffect.LEDId = ChromaSDK::Mouse::RZLED_ALL;
			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_STATIC, &staticEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			ChromaSDK::Mousepad::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_STATIC, &staticEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			ChromaSDK::Headset::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_STATIC, &staticEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			ChromaSDK::Keypad::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_STATIC, &staticEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			ChromaSDK::ChromaLink::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &staticEffect, NULL);
		}
	}

	void ChromaSystemComponent::SetEffectColor(AZ::Color color) {

		//g_ChromaSDKImpl.staticColor = GetCOLORREFValue(ChromaColor);
	}

	void ChromaSystemComponent::ShowFlashEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		RZEFFECTID colorFrame;
		RZEFFECTID blankFrame;
		
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			ChromaSDK::Keyboard::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_STATIC, &staticEffect, &colorFrame);
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_NONE, NULL, &blankFrame);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			ChromaSDK::Mouse::STATIC_EFFECT_TYPE staticEffect = {};
			staticEffect.LEDId = ChromaSDK::Mouse::RZLED_ALL;
			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_STATIC, &staticEffect, &colorFrame);
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_NONE, NULL, &blankFrame);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			ChromaSDK::Mousepad::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_STATIC, &staticEffect, &colorFrame);
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_NONE, NULL, &blankFrame);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			ChromaSDK::Headset::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_STATIC, &staticEffect, &colorFrame);
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_NONE, NULL, &blankFrame);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			ChromaSDK::Keypad::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_STATIC, &staticEffect, &colorFrame);
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_NONE, NULL, &blankFrame);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			ChromaSDK::ChromaLink::STATIC_EFFECT_TYPE staticEffect = {};

			staticEffect.Color = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &staticEffect, &colorFrame);
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_NONE, NULL, &blankFrame);
		}

		g_ChromaSDKImpl.AddToGroup(GroupEffectId, blankFrame, g_effectSpeed);
		g_ChromaSDKImpl.AddToGroup(GroupEffectId, colorFrame, g_effectSpeed);

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;
	}

	void ChromaSystemComponent::ShowRandomEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		const int arraySize = 11;

		RZEFFECTID KBFrame = GUID_NULL;
		COLORREF rainbowArray[arraySize] = { RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET, WHITE, CYAN, PINK, GREY };

		UINT rand = 0;
		UINT colorIndex = 0;

		// Random number class
		AZ::Sfmt g;
		
		
		if (ChromaSystemComponent::chromaDeviceType == 0) {
 			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE KeyboardEffect = {};

			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
				{
					colorIndex = g.Rand32() % arraySize;
					KeyboardEffect.Color[r][c] = rainbowArray[colorIndex];
				}
			}
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &KeyboardEffect, NULL);

		}
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2 MouseEffect = {};

			for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
				{
					colorIndex = g.Rand32() % arraySize;
					MouseEffect.Color[r][c] = rainbowArray[colorIndex];
				}
			}
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &MouseEffect, NULL);

		}
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE MousepadEffect = {};

			for (UINT r = 0; r < ChromaSDK::Mousepad::MAX_LEDS; r++)
			{
				colorIndex = g.Rand32() % arraySize;
				MousepadEffect.Color[r] = rainbowArray[colorIndex];
			}
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &MousepadEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			ChromaSDK::Headset::CUSTOM_EFFECT_TYPE HeadsetEffect = {};

			for (UINT r = 0; r < ChromaSDK::Headset::MAX_LEDS; r++)
			{
				colorIndex = g.Rand32() % arraySize;
				HeadsetEffect.Color[r] = rainbowArray[colorIndex];
			}
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &HeadsetEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			ChromaSDK::Keypad::CUSTOM_EFFECT_TYPE KeypadEffect = {};

			for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
				{
					colorIndex = g.Rand32() % arraySize;
					KeypadEffect.Color[r][c] = rainbowArray[colorIndex];
				}
			}
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &KeypadEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE ChromaLinkEffect = {};

			for (UINT r = 0; r < ChromaSDK::ChromaLink::MAX_LEDS; r++)
			{
				colorIndex = g.Rand32() % arraySize;
				ChromaLinkEffect.Color[r] = rainbowArray[colorIndex];
			}
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &ChromaLinkEffect, NULL);
		}

	}

	void ChromaSystemComponent::LoadSingleImage() {
		ReadImageFile();
	}

	AZ::Crc32 ChromaSystemComponent::LoadAnimation() {
		return ReadGifAnimationFile();
	}

	bool ChromaSystemComponent::ReadImageFile() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			QString filename = QFileDialog::getOpenFileName(nullptr, "Load Image", QString(), "Search Filter Files (*.bmp;*.jpg;*.png)");
			QFile file(filename);
			if (!file.open(QIODevice::ReadOnly))
			{
				QMessageBox::information(0, _T("Unable to Open File"), _T("Unable to open image file."));
				return false;
			}

			QByteArray rawImage = file.readAll();
			QPixmap pixmap;

			pixmap.loadFromData(rawImage);
			if (!pixmap.isNull())
			{
				QSize pixmapSize = pixmap.size();
				QSize chromaSize(ChromaSDK::Keyboard::MAX_COLUMN, ChromaSDK::Keyboard::MAX_ROW);

				pixmap = pixmap.scaled(chromaSize);
				QImage image = pixmap.toImage();

				// For testing purposes
				image.save("LumberyardChromaImageOutput/Frame0.png");

				for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
				{
					// Get ths RGB bits for each row
					const uchar *rowData = image.scanLine(r);

					COLORREF *pColor = (COLORREF*)rowData;

					for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
					{
						// Fill up the array
						keyboardFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetBValue(*pColor), GetGValue(*pColor), GetRValue(*pColor));
						pColor++;
					}
				}
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM, &keyboardFrames[m_currFrameNum - 1], NULL);
			}
		}
		else {
			QMessageBox::information(0, _T("Invalid Device"), _T("Please change device type to keyboard for importing an image file."));
			return false;
		}

		return true;
	}

	AZ::Crc32 ChromaSystemComponent::ReadGifAnimationFile() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			QString filename = QFileDialog::getOpenFileName(nullptr, "Load Image", QString(), "Search Filter Files (*.gif)");
			QFile file(filename);
			if (!file.open(QIODevice::ReadOnly))
			{
				QMessageBox::information(0, _T("Unable to Open File"), _T("Unable to open animation file."));
				return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
			}

			QMovie chromaGIF(filename);
			chromaGIF.setScaledSize(QSize(ChromaSDK::Keyboard::MAX_COLUMN, ChromaSDK::Keyboard::MAX_ROW));

			int animationFrameCount = chromaGIF.frameCount();

			if (animationFrameCount > MAXFRAMES) {
				QMessageBox::information(0, _T("Invalid Animation File"), _T("The animation file exceeds tha maximum number of frames allowed"));
				return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
			}

			int currentFrameNum = 0;

			//For each frame in the animation
			for (int frameIndex = 0; frameIndex < animationFrameCount; frameIndex++) {
				if (m_currFrameNum > MAXFRAMES) {
					QMessageBox::information(0, _T("Frame Limit Hit"), _T("The maximum number of frames has been reached"));
					break;
				}

				currentFrameNum = chromaGIF.currentFrameNumber();

				QImage image = chromaGIF.currentImage();

				if (!image.isNull())
				{
					// For testing purposes
					switch (frameIndex) {
					case 0:
						image.save(_T("LumberyardChromaImageOutput/GIFFrame0.png")); break;
					case 5:
						image.save(_T("LumberyardChromaImageOutput/GIFFrame5.png")); break;
					case 10:
						image.save(_T("LumberyardChromaImageOutput/GIFFrame10.png")); break;
					case 15:
						image.save(_T("LumberyardChromaImageOutput/GIFFrame15.png")); break;
					}

					for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
					{
						// Get ths RGB bits for each row
						const uchar *rowData = image.scanLine(r);

						COLORREF *pColor = (COLORREF*)rowData;

						for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
						{
							// Fill up the array
							keyboardFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetBValue(*pColor), GetGValue(*pColor), GetRValue(*pColor));
							pColor++;
						}
					}
					g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM, &keyboardFrames[m_currFrameNum - 1], NULL);
					m_currFrameNum++;
					if (m_currFrameNum > maxFrame[0])
						maxFrame[0] = m_currFrameNum;
				}
				chromaGIF.jumpToNextFrame();
			}
		}
		else {
			QMessageBox::information(0, _T("Invalid Device"), _T("Please change device type to keyboard for importing an image file."));
			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
		}
		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaSystemComponent::SetKey() {
		// Only used for keyboard
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			keyboardFrames[m_currFrameNum-1].Key[HIBYTE(ChromaKey)][LOBYTE(ChromaKey)] = 0x01000000 | GetCOLORREFValue(ChromaColor, g_effectBrightness);
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
	}

	void ChromaSystemComponent::SetLed() {
		// Mouse Device - 9 x 7 is maxLEDs
		if (ChromaSystemComponent::chromaDeviceType == 1) {
			if (ChromaLed < ChromaSDK::Mouse::MAX_LEDS2) {
				mouseFrames[m_currFrameNum - 1].Color[HIBYTE(chromaMouseLEDs[ChromaLed])][LOBYTE(chromaMouseLEDs[ChromaLed])] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
			}
		}

		// Mousepad Device - 15 is maxLEDs
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			if (ChromaLed < ChromaSDK::Mousepad::MAX_LEDS) {
				mousepadFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
			}
		}

		// Headset Device - 5 is maxLEDs
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			if (ChromaLed < ChromaSDK::Headset::MAX_LEDS) {
				headsetFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
			}
		}

		// Keypad Device - 4 x 5 is maxLEDs
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			if (ChromaLed < ChromaSDK::Keypad::MAX_KEYS) {
				int row = ChromaLed / ChromaSDK::Keypad::MAX_COLUMN;
				int col = ChromaLed % ChromaSDK::Keypad::MAX_COLUMN;

				keypadFrames[m_currFrameNum - 1].Color[row][col] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
			}
		}

		// Chromalink Device - 5 is maxLEDs
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			if (ChromaLed < ChromaSDK::ChromaLink::MAX_LEDS) {
				chromalinkFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
			}
		}
	}

	void ChromaSystemComponent::PaintRow() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				keyboardFrames[m_currFrameNum - 1].Color[cRow-1][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		else {
			QMessageBox::information(0, _T("Invalid Device"), _T("Cannot Paint Row on Non-Keyboard Device"));
		}
	}

	void ChromaSystemComponent::PaintCol() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
			{
				keyboardFrames[m_currFrameNum - 1].Color[r][cCol-1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		else {
			QMessageBox::information(0, _T("Invalid Device"), _T("Cannot Paint Column on Non-Keyboard Device"));
		}
	}

	// Frame Management Functions
	void ChromaSystemComponent::FillFrame() {
		
		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
				{
					keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
			}

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
			}

			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
			}

			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
		}
	}

	void ChromaSystemComponent::ClearFrame() {
		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			keyboardFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			mouseFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
		}
		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			mousepadFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
		}
		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			headsetFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
		}
		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			keypadFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		}
		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			chromalinkFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
		}
	}

	AZ::Crc32 ChromaSystemComponent::ClearAllFrames() {
		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			for (int frameNum = 0; frameNum < maxFrame[0]; frameNum++) {
				keyboardFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[frameNum], NULL);
			}
			maxFrame[0] = 1;
			m_currDeviceEffects[0] = GUID_NULL;

		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			for (int frameNum = 0; frameNum < maxFrame[1]; frameNum++) {
				mouseFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[frameNum], NULL);
			}
			maxFrame[1] = 1;
			m_currDeviceEffects[1] = GUID_NULL;
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			for (int frameNum = 0; frameNum < maxFrame[2]; frameNum++) {
				mousepadFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[frameNum], NULL);
			}
			maxFrame[2] = 1;
			m_currDeviceEffects[2] = GUID_NULL;
		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			for (int frameNum = 0; frameNum < maxFrame[3]; frameNum++) {
				headsetFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[frameNum], NULL);
			}
			maxFrame[3] = 1;
			m_currDeviceEffects[3] = GUID_NULL;
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			for (int frameNum = 0; frameNum < maxFrame[4]; frameNum++) {
				keypadFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[frameNum], NULL);
			}
			maxFrame[4] = 1;
			m_currDeviceEffects[4] = GUID_NULL;
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			for (int frameNum = 0; frameNum < maxFrame[5]; frameNum++) {
				chromalinkFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[frameNum], NULL);
			}
			maxFrame[5] = 1;
			m_currDeviceEffects[5] = GUID_NULL;
		}

		m_currFrameNum = 1;
		m_currEffect = GUID_NULL;

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaSystemComponent::CopyFrame() {
		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			copyKeyboardEffect = keyboardFrames[m_currFrameNum - 1];
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			copyMouseEffect = mouseFrames[m_currFrameNum - 1];
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			copyMousepadEffect = mousepadFrames[m_currFrameNum - 1];
		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			copyHeadsetEffect = headsetFrames[m_currFrameNum - 1];
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			copyKeypadEffect = keypadFrames[m_currFrameNum - 1];
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			copyChromalinkEffect = chromalinkFrames[m_currFrameNum - 1];
		}
	}

	void ChromaSystemComponent::PasteFrame() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			keyboardFrames[m_currFrameNum - 1] = copyKeyboardEffect;
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			mouseFrames[m_currFrameNum - 1] = copyMouseEffect;
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			mousepadFrames[m_currFrameNum - 1] = copyMousepadEffect;
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			headsetFrames[m_currFrameNum - 1] = copyHeadsetEffect;
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			keypadFrames[m_currFrameNum - 1] = copyKeypadEffect;
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			chromalinkFrames[m_currFrameNum - 1] = copyChromalinkEffect;
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
		}

	}

	AZ::Crc32 ChromaSystemComponent::JumpToFrame() {
		if (newFrame > MAXFRAMES) {
			newFrame = MAXFRAMES;
		}
		else if (newFrame < 1) {
			newFrame = 1;
		}
		m_currFrameNum = newFrame;

		if (m_currFrameNum > maxFrame[ChromaSystemComponent::chromaDeviceType])
			maxFrame[ChromaSystemComponent::chromaDeviceType] = m_currFrameNum;

		ShowFrame(m_currFrameNum);

		CryLog("Current Chroma frame = %d ", m_currFrameNum);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::Crc32 ChromaSystemComponent::IncrementFrame() {
		if (m_currFrameNum == MAXFRAMES)
			return m_currFrameNum;
		else if (m_currFrameNum > MAXFRAMES) {
			m_currFrameNum = 100;
			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
		}
		
		newFrame = m_currFrameNum + 1;
		m_currFrameNum = newFrame;

		if (m_currFrameNum > maxFrame[ChromaSystemComponent::chromaDeviceType])
			maxFrame[ChromaSystemComponent::chromaDeviceType] = m_currFrameNum;

		ShowFrame(m_currFrameNum);

		CryLog("Current Chroma frame = %d ", m_currFrameNum);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::Crc32 ChromaSystemComponent::DecrementFrame() {
		if (m_currFrameNum == 1)
			return m_currFrameNum;
		else if (m_currFrameNum < 1) {
			m_currFrameNum = 1;
			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
		}

		newFrame = m_currFrameNum - 1;
		m_currFrameNum = newFrame;

		ShowFrame(m_currFrameNum);

		CryLog("Current Chroma frame = %d ", m_currFrameNum);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaSystemComponent::PlayCustomAnimation() {
		RZEFFECTID AnimationEffectId = GUID_NULL;
		if (IsEqualGUID(AnimationEffectId, GUID_NULL))
		{
			g_ChromaSDKImpl.CreateEffectGroup(&AnimationEffectId, repeatAnimation);
		}

		RZEFFECTID effectID = GUID_NULL;

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			for (int fInx = 0; fInx < maxFrame[0]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			for (int fInx = 0; fInx < maxFrame[1]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			for (int fInx = 0; fInx < maxFrame[2]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			for (int fInx = 0; fInx < maxFrame[3]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			for (int fInx = 0; fInx < maxFrame[4]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}

		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			for (int fInx = 0; fInx < maxFrame[5]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		g_ChromaSDKImpl.SetEffectImpl(AnimationEffectId);
		m_currEffect = AnimationEffectId;
	}

	void ChromaSystemComponent::PlayAllCustomAnimation() {
		RZEFFECTID KBAnimationEffectId = GUID_NULL;
		RZEFFECTID MouseAnimationEffectId = GUID_NULL;
		RZEFFECTID MPAnimationEffectId = GUID_NULL;
		RZEFFECTID HSAnimationEffectId = GUID_NULL;
		RZEFFECTID KPAnimationEffectId = GUID_NULL;
		RZEFFECTID CLAnimationEffectId = GUID_NULL;

		RZEFFECTID effectID = GUID_NULL;

		for (int m = 0; m < MAXDEVICES; m++) {
			if (maxFrame[m] > 1) {					// If maxFrames > 1, then there must be an animation specified
				switch (m) {
					case 0: {		// Keyboard
						g_ChromaSDKImpl.CreateEffectGroup(&KBAnimationEffectId, repeatAnimation);

						for (int fInx = 0; fInx < maxFrame[0]; fInx++) {
							effectID = GUID_NULL;
							g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[fInx], &effectID);
							g_ChromaSDKImpl.AddToGroup(KBAnimationEffectId, effectID, g_effectSpeed);
						}
						m_currDeviceEffects[0] = KBAnimationEffectId;
						break;
					}
					case 1: {		// Mouse
						g_ChromaSDKImpl.CreateEffectGroup(&MouseAnimationEffectId, repeatAnimation);

						for (int fInx = 0; fInx < maxFrame[1]; fInx++) {
							effectID = GUID_NULL;
							g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[fInx], &effectID);
							g_ChromaSDKImpl.AddToGroup(MouseAnimationEffectId, effectID, g_effectSpeed);
						}
						m_currDeviceEffects[1] = MouseAnimationEffectId;
						break;
					}
					case 2: {		// Mousepad
						g_ChromaSDKImpl.CreateEffectGroup(&MPAnimationEffectId, repeatAnimation);

						for (int fInx = 0; fInx < maxFrame[2]; fInx++) {
							effectID = GUID_NULL;
							g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[fInx], &effectID);
							g_ChromaSDKImpl.AddToGroup(MPAnimationEffectId, effectID, g_effectSpeed);
						}
						m_currDeviceEffects[2] = MPAnimationEffectId;
						break;
					}
					case 3: {		// Headset
						g_ChromaSDKImpl.CreateEffectGroup(&HSAnimationEffectId, repeatAnimation);

						for (int fInx = 0; fInx < maxFrame[3]; fInx++) {
							effectID = GUID_NULL;
							g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[fInx], &effectID);
							g_ChromaSDKImpl.AddToGroup(HSAnimationEffectId, effectID, g_effectSpeed);
						}
						m_currDeviceEffects[3] = HSAnimationEffectId;
						break;
					}
					case 4: {		// Keypad
						g_ChromaSDKImpl.CreateEffectGroup(&KPAnimationEffectId, repeatAnimation);

						for (int fInx = 0; fInx < maxFrame[4]; fInx++) {
							effectID = GUID_NULL;
							g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[fInx], &effectID);
							g_ChromaSDKImpl.AddToGroup(KPAnimationEffectId, effectID, g_effectSpeed);
						}
						m_currDeviceEffects[4] = KPAnimationEffectId;
						break;
					}
					case 5: {		// Chromalink
						g_ChromaSDKImpl.CreateEffectGroup(&CLAnimationEffectId, repeatAnimation);

						for (int fInx = 0; fInx < maxFrame[5]; fInx++) {
							effectID = GUID_NULL;
							g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[fInx], &effectID);
							g_ChromaSDKImpl.AddToGroup(CLAnimationEffectId, effectID, g_effectSpeed);
						}
						m_currDeviceEffects[5] = CLAnimationEffectId;
						break;
					}
				}

			}
		}

		for (int effectIndx = 0; effectIndx < MAXDEVICES; effectIndx++) {
			if (m_currDeviceEffects[effectIndx] != GUID_NULL) {
				g_ChromaSDKImpl.SetEffectImpl(m_currDeviceEffects[effectIndx]);
			}
		}
	}

	void ChromaSystemComponent::ShowFrame(int frame) {
		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[frame-1], NULL);
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[frame - 1], NULL);
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[frame - 1], NULL);
		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[frame - 1], NULL);
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[frame - 1], NULL);
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[frame-1], NULL);
		}
	}
}

