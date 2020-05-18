
#include "StdAfx.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <AzCore/Math/Sfmt.h>

#include "ChromaEditorSystemComponent.h"

#include "IRenderer.h"

#include <qobject.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QMovie>
#include <QTextStream>

#define ANIMATION_VERSION 1

namespace Chroma
{
	ChromaEditorSystemComponent::ChromaEditorSystemComponent() {
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

	ChromaEditorSystemComponent::~ChromaEditorSystemComponent() {

	}
	
	void ChromaEditorSystemComponent::Init()
	{
	}

	void ChromaEditorSystemComponent::Activate()
	{
		//ChromaRequestBus::Handler::BusConnect();

		CrySystemEventBus::Handler::BusConnect(); // For calendar demo

		AZ::TickBus::Handler::BusConnect();		  // connect to the TickBus

		g_ChromaSDKImpl.Initialize();
	}

	void ChromaEditorSystemComponent::Deactivate()
	{
		//ChromaRequestBus::Handler::BusDisconnect();
	}

	void ChromaEditorSystemComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serialize->Class<ChromaEditorSystemComponent, AZ::Component>()
				->Version(0)
				//->SerializerForEmptyClass();
				->Field("Effect Brightness", &ChromaEditorSystemComponent::g_effectBrightness)
				->Field("Effect Speed", &ChromaEditorSystemComponent::g_effectSpeed)
				->Field("Color", &ChromaEditorSystemComponent::ChromaColor)
				->Field("Repeat", &ChromaEditorSystemComponent::repeatAnimation)
				->Field("Select Device", &ChromaEditorSystemComponent::chromaDeviceType)
				->Field("Stop Effect", &ChromaEditorSystemComponent::stopEffect)
				->Field("Clear Effect", &ChromaEditorSystemComponent::clearEffects)
				->Field("Select Effect Preset", &ChromaEditorSystemComponent::chromaEffectValue)
				->Field("Play Preset Effect", &ChromaEditorSystemComponent::playPresetEffect)
				->Field("Current Frame", &ChromaEditorSystemComponent::m_currFrameNum)
				->Field("Chroma Key", &ChromaEditorSystemComponent::ChromaKey)
				->Field("Chroma Led", &ChromaEditorSystemComponent::ChromaLed)
				->Field("Set Key", &ChromaEditorSystemComponent::setKey)
				->Field("Set Led", &ChromaEditorSystemComponent::setLed)
				->Field("Row", &ChromaEditorSystemComponent::cRow)
				->Field("Column", &ChromaEditorSystemComponent::cCol)
				->Field("Set Row", &ChromaEditorSystemComponent::setRow)
				->Field("Set Col", &ChromaEditorSystemComponent::setCol)
				->Field("Jump To Frame", &ChromaEditorSystemComponent::newFrame)
				->Field("Next Frame", &ChromaEditorSystemComponent::nextFrame)
				->Field("Prev Frame", &ChromaEditorSystemComponent::prevFrame)
				->Field("Copy Frame", &ChromaEditorSystemComponent::copyFrame)
				->Field("Paste Frame", &ChromaEditorSystemComponent::pasteFrame)
				->Field("Fill Frame", &ChromaEditorSystemComponent::fillFrame)
				->Field("Clear Frame", &ChromaEditorSystemComponent::clearFrame)
				->Field("Reset Animation", &ChromaEditorSystemComponent::resetAnimation)
				->Field("Add Frame", &ChromaEditorSystemComponent::addFrame)
				->Field("Delete Frame", &ChromaEditorSystemComponent::deleteFrame)
				->Field("Load Image", &ChromaEditorSystemComponent::importImage)
				->Field("Load Animation", &ChromaEditorSystemComponent::importAnimation)
				->Field("Play Custom", &ChromaEditorSystemComponent::playCustomEffect)
				->Field("Play All Custom", &ChromaEditorSystemComponent::playAllCustomEffect)
				->Field("Import Effect", &ChromaEditorSystemComponent::importEffect)
				->Field("Export Effect", &ChromaEditorSystemComponent::exportEffect)
				->Field("Export Header", &ChromaEditorSystemComponent::exportHeader)
				;

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<ChromaEditorSystemComponent>("Chroma", "Adds Chroma lighting effects to game objects")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					// ->Attribute(AZ::Edit::Attributes::Category, "") Set a category
					->Attribute(AZ::Edit::Attributes::Category, "Peripheral Lighting")
					//->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaEditorSystemComponent::chromaDeviceType, "Select Device", "Select Chroma Device")
						->EnumAttribute(ChromDeviceType::KEYBOARD, "Keyboard")
						->EnumAttribute(ChromDeviceType::MOUSE, "Mouse")
						->EnumAttribute(ChromDeviceType::MOUSEPAD, "Mousepad")
						->EnumAttribute(ChromDeviceType::HEADSET, "Headset")
						->EnumAttribute(ChromDeviceType::KEYPAD, "Keypad")
						->EnumAttribute(ChromDeviceType::CHROMALINK, "ChromaLink")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::SetChromaDeviceType)
						->Attribute("ChangeNotify", AZ_CRC("RefreshValues", 0x28e720d4))
					->DataElement(AZ::Edit::UIHandlers::Color, &ChromaEditorSystemComponent::ChromaColor, "Chroma Color", "Chroma Color picker")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::SetEffectColor)
					->DataElement(AZ::Edit::UIHandlers::Slider, &ChromaEditorSystemComponent::g_effectBrightness, "Brightness Control", "Use to set color brightness for current effect")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::SetEffectBrightness)
						->Attribute(AZ::Edit::Attributes::Min, 0.0f)
						->Attribute(AZ::Edit::Attributes::Max, 1.0f)
						->Attribute(AZ::Edit::Attributes::Step, 0.01f)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaEditorSystemComponent::g_effectSpeed, "Effect Speed", "Use to change speed of animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::SetEffectSpeed)
						->Attribute(AZ::Edit::Attributes::Min, 0)
						->Attribute(AZ::Edit::Attributes::Max, 5000)
						->Attribute(AZ::Edit::Attributes::ValueText, &ChromaEditorSystemComponent::g_effectSpeed)
					->DataElement(AZ::Edit::UIHandlers::CheckBox, &ChromaEditorSystemComponent::repeatAnimation, "Repeat Animation", "Set to repeat animation indefintely")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::stopEffect, "", "Stop the current animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::StopEffect)
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Stop Effect")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::clearEffects, "", "Clear All Chroma Devices")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::ClearEffects)
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Clear Effects")
					->ClassElement(AZ::Edit::ClassElements::Group, "Chroma Preset Effects")
						->Attribute(AZ::Edit::Attributes::AutoExpand, false)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaEditorSystemComponent::chromaEffectValue, "Select Preset Effect", "Select Chroma Effect Preset")
						->EnumAttribute(ChromaEffectValue::RANDOM, "Random")
						->EnumAttribute(ChromaEffectValue::FLASH, "Flash")
						->EnumAttribute(ChromaEffectValue::WAVELR1, "Wave LR")
						->EnumAttribute(ChromaEffectValue::WATERFALL, "Waterfall ")
						->EnumAttribute(ChromaEffectValue::WAVECROSS, "CrossWave")
						->EnumAttribute(ChromaEffectValue::BREATHING, "Breathing")
						->EnumAttribute(ChromaEffectValue::RIPPLELR, "Ripple LR")
						->EnumAttribute(ChromaEffectValue::RIPPLEUD, "Ripple TB")
						->EnumAttribute(ChromaEffectValue::EXPLOSION, "Explosion")
						->EnumAttribute(ChromaEffectValue::RAINBOW, "Rainbow")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::playPresetEffect, "", "Plays selected preset effect")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::PlayPresetEffect)
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Play Preset Effect")
					->ClassElement(AZ::Edit::ClassElements::Group, "Chroma Custom Effects")
						->Attribute(AZ::Edit::Attributes::AutoExpand, false)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaEditorSystemComponent::m_currFrameNum, "Current Frame", "Displays Current Chroma Frame")
						->Attribute(AZ::Edit::Attributes::ReadOnly, true)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaEditorSystemComponent::ChromaKey, "Select Key", "Select Key to Color")
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
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaEditorSystemComponent::ChromaLed, "Select Led", "Select Led (non-keyboard devices) to Color")
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
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::setKey, "", "Color Selected Key")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::SetKey)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Set Key Color")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::setLed, "", "Color Selected Key")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::SetLed)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Set Led Color")
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaEditorSystemComponent::cRow, "Row", "Specify Device Row to Paint")
						->Attribute(AZ::Edit::Attributes::Min, 1)
						->Attribute(AZ::Edit::Attributes::Max, 6)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaEditorSystemComponent::cCol, " Column", "Specify Device Column to Paint")
						->Attribute(AZ::Edit::Attributes::Min, 1)
						->Attribute(AZ::Edit::Attributes::Max, 22)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::setRow, "", "Color Entire Row On Keyboard")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::PaintRow)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Paint Row")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::setCol, "", "Color Entire Column On Keyboard")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::PaintCol)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Paint Column")
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaEditorSystemComponent::newFrame, "Jump to Frame", "Frame must be <MAXFRAMES")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::JumpToFrame)
						->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, true)
						->Attribute(AZ::Edit::Attributes::Min, 1)
						->Attribute(AZ::Edit::Attributes::Max, MAXFRAMES)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::nextFrame, "", "Move to next frame in animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::NextFrame)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Next Frame")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::prevFrame, "", "Move to previous frame in animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::PreviousFrame)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Prev Frame")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::copyFrame, "", "Copy Contents of Current Chroma Frame")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::CopyFrame)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Copy Frame")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::pasteFrame, "", "Paste Contents of Current Chroma Frame")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::PasteFrame)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Paste Frame")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::fillFrame, "", "Color all Device Leds Selected Color")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::FillFrame)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Fill Frame")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::clearFrame, "", "Clears the current chroma frame")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::ClearFrame)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Clear Frame")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::addFrame, "", "Adds a new frame to the animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::AddFrame)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Add Frame")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::deleteFrame, "", "Deletes the current frame from the animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::DeleteFrame)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Delete Frame")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::resetAnimation, "", "Clears all frames and resets frame count to 1")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::ResetAnimation)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Reset Animation")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::importImage, "", "Import jpg, png, bmp")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::LoadSingleImage)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Import Image")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::importAnimation, "", "Import gif animations")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::LoadAnimation)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Import Animation")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::playCustomEffect, "", "Plays current device custom animation frames")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::PlayCustomAnimation)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Play Custom Single Device")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::playAllCustomEffect, "", "Plays custom animation on all devices simultaneously")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::PlayAllCustomAnimation)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Play Custom All Devices")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::importEffect, "", "Imports existing .chroma effect file into the Lumberyard editor")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::ImportChromaEffect)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Import Effect")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::exportEffect, "", "Creates .chroma effect file for current effect")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::ExportChromaEffect)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Export Effect")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaEditorSystemComponent::exportHeader, "", "Exports current animation color buffer into .h file")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaEditorSystemComponent::ExportChromaHeaderFile)
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Export Header File")
					;
			}
		}
	}

	/*void ChromaEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("ChromaService", 0xd92ddd51));
	}

	void ChromaEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("ChromaService", 0xd92ddd51));
	}

	void ChromaEditorSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void ChromaEditorSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}*/

	void ChromaEditorSystemComponent::OnCrySystemInitialized(ISystem&, const SSystemInitParams&) // For calendar demo
	{

	}

	void ChromaEditorSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time) {

	}

	COLORREF ChromaEditorSystemComponent::GetCOLORREFValue(AZ::Color color, double brightness) {
		COLORREF retColor = 0;
		
		AZ::u8 red = color.GetR8()* brightness;
		AZ::u8 green = color.GetG8() * brightness;
		AZ::u8 blue = color.GetB8() * brightness;

		retColor = RGB(red, green, blue);

		return retColor;

	}

	void ChromaEditorSystemComponent::SetEffectBrightness() {
		float test = g_effectBrightness;
	}

	void ChromaEditorSystemComponent::SetEffectSpeed() {
		int test = 0;
	}

	AZ::Crc32 ChromaEditorSystemComponent::SetChromaDeviceType() {
		g_ChromaSDKImpl.mDeviceType = this->chromaDeviceType;

		m_currFrameNum = 1;

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaEditorSystemComponent::SetEffectColor() {

		int test = 0;
	}

	void ChromaEditorSystemComponent::StopEffect() {
		if (m_currEffect != GUID_NULL)
			g_ChromaSDKImpl.StopEffectImpl(m_currEffect);

		for (int j = 0; j < MAXDEVICES; j++) {
			if (m_currDeviceEffects[j] != GUID_NULL) {
				g_ChromaSDKImpl.StopEffectImpl(m_currDeviceEffects[j]);
			}
		}
	}

	void ChromaEditorSystemComponent::ClearEffects() {
		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_NONE, NULL, NULL);

		copyKeyboardEffect = {};
	}

	void ChromaEditorSystemComponent::PlayPresetEffect() {
		switch (chromaEffectValue) {
		case 0:			// Random Effect 
		{
			ShowRandomEffect();
			break;
		}
		case 1:			// Flash Effect 
		{
			ShowFlashEffect();
			break;
		}
		case 2:			// Wave Effect 
		{
			ShowWaveLR1Effect();
			break;
		}
		case 3:			// Waterfall Effect 
		{
			ShowWaterFallEffect();
			break;
		}
		case 4:			// Double Wave Cross Effect 
		{
			ShowWaveCross1Effect();
			break;
		}
		case 5:			// Breathing Effect 
		{
			ShowBreathingEffect();
			break;
		}
		case 6:			// Ripple Left To Right Effect 
		{
			ShowRippleLREffect();
			break;
		}
		case 7:			// Ripple Left To Right Effect 
		{
			ShowRippleUDEffect();
			break;
		}
		case 8:			// Explosion Effect 
		{
			ShowExplosionEffect();
			break;
		}
		case 9:			// Rainbow Effect 
		{
			ShowRainbowEffect();
			break;
		}
		}
	}

/************************************* Chroma Preset Effects *******************************************************
These effects are precoded by the author of this plugin and can be used for quick and simple effect implementation.
Note that the brightness and effect speed can still be modified in the editor and the frame data can be exported into
a .chroma effect file
********************************************************************************************************************/

	// Flash Effect - Single Flash of All LEDs
	void ChromaEditorSystemComponent::ShowFlashEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID frames[2];

			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
					keyboardFrames[m_currFrameNum].Color[r][c] = RGB(0,0,0);
				}
			}

			// Add the color frame
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &frames[0]);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[0], g_effectSpeed);
			AddFrame();

			// Add the blank frame
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum], &frames[1]);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[1], g_effectSpeed);
			AddFrame();

		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
			{
				//MouseEffect = {};

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}

		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {

			RZEFFECTID frames[ChromaSDK::Keypad::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
			{
				//KeypadEffect = {};

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		
		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Wave Effect - Left to Right
	void ChromaEditorSystemComponent::ShowWaveLR1Effect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID frames[ChromaSDK::Keyboard::MAX_COLUMN + 1];

			int tempFrameIndex = 0;

			// Start Wave With Editor Color
			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				tempFrameIndex = m_currFrameNum;
				AddFrame();

				memcpy_s(&keyboardFrames[m_currFrameNum - 1], sizeof(keyboardFrames[m_currFrameNum - 1]), &keyboardFrames[tempFrameIndex - 1], sizeof(keyboardFrames[tempFrameIndex - 1]));
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			AddFrame();
		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
			{
				//MouseEffect = {};

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}

		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {

			RZEFFECTID frames[ChromaSDK::Keypad::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
			{
				//KeypadEffect = {};

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Wave - Vertical Up to Down
	void ChromaEditorSystemComponent::ShowWaterFallEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID frames[ChromaSDK::Keyboard::MAX_ROW + 1];
			RZEFFECTID lastframe;

			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW+2; r++)
			{
				//KeyboardEffect = {};

				for (int c = ChromaSDK::Keyboard::MAX_COLUMN - 1; c >= 0; c--)
				{
					if (r > 1) {
						if (r < ChromaSDK::Keyboard::MAX_ROW) {
							keyboardFrames[m_currFrameNum - 1].Color[r - 2][c] = GetCOLORREFValue(ChromaColor, 0.3);
							keyboardFrames[m_currFrameNum - 1].Color[r - 1][c] = GetCOLORREFValue(ChromaColor, 0.5);
							keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, 1.0);
						}
						else if (r == ChromaSDK::Keyboard::MAX_ROW) {
							keyboardFrames[m_currFrameNum - 1].Color[r - 2][c] = GetCOLORREFValue(ChromaColor, 0.3);
							keyboardFrames[m_currFrameNum - 1].Color[r - 1][c] = GetCOLORREFValue(ChromaColor, 0.5);
						}
						else if (r > ChromaSDK::Keyboard::MAX_ROW) {
							keyboardFrames[m_currFrameNum - 1].Color[r - 2][c] = GetCOLORREFValue(ChromaColor, 0.3);
						}
					}
					else if (r == 1) {
						keyboardFrames[m_currFrameNum - 1].Color[r - 1][c] = GetCOLORREFValue(ChromaColor, 0.5);
						keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, 1.0);
					}
					else if (r == 0) {
						keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, 1.0);
					}
				}

				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &frames[r]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[r], g_effectSpeed);

				AddFrame();
				if (m_currFrameNum < MAXFRAMES)
					m_currFrameNum++;
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			AddFrame();

		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_ROW + 1];

			for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
			{
				//MouseEffect = {};

				for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &frames[r]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[r], g_effectSpeed);

				AddFrame();
			}
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}

		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {

			RZEFFECTID frames[ChromaSDK::Keypad::MAX_ROW + 1];

			for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
			{
				//KeypadEffect = {};

				for (int c = ChromaSDK::Keyboard::MAX_COLUMN - 1; c >= 0; c--)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &frames[r]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[r], g_effectSpeed);

				AddFrame();
			}
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Wave - Double Cross Effect
	void ChromaEditorSystemComponent::ShowWaveCross1Effect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID Frames[ChromaSDK::Keyboard::MAX_COLUMN + 1];

			for (int c1 = 0, c2 = ChromaSDK::Keyboard::MAX_COLUMN-1; c1 < ChromaSDK::Keyboard::MAX_COLUMN; c1++, c2--) {

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keyboardFrames[m_currFrameNum - 1].Color[r][c1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
					keyboardFrames[m_currFrameNum - 1].Color[r][c2] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &Frames[c1]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Frames[c1], g_effectSpeed);

				AddFrame();
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			AddFrame();
		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
			{
				//MouseEffect = {};

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}

		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {

			RZEFFECTID frames[ChromaSDK::Keypad::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
			{
				//KeypadEffect = {};

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Breathing Effect
	void ChromaEditorSystemComponent::ShowBreathingEffect() {
		const int brightnessCount = 11;

		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		RZEFFECTID Frames[brightnessCount] = { GUID_NULL };
		double Brightness[brightnessCount] = { 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0 };
		double brightness = 0.0;

		UINT index = 0;
		UINT count = 0;
		while (count < brightnessCount)
		{
			brightness = Brightness[index];

			if (ChromaEditorSystemComponent::chromaDeviceType == 0) {

				for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
					{
						keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
					{
						mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {

				for (UINT r = 0; r < ChromaSDK::Mousepad::MAX_LEDS; r++)
				{
					mousepadFrames[m_currFrameNum - 1].Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {

				for (UINT r = 0; r < ChromaSDK::Headset::MAX_LEDS; r++)
				{
					headsetFrames[m_currFrameNum - 1].Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {

				for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
					{
						keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {

				for (UINT r = 0; r < ChromaSDK::ChromaLink::MAX_LEDS; r++)
				{
					chromalinkFrames[m_currFrameNum - 1].Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &Frames[index]);
			}

			index++;
			count++;

			AddFrame();
		}

		for (int i = 0; i < brightnessCount; i++) {
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, Frames[i], g_effectSpeed);
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;
	}

	// Ripple Effect - 1 Column Left to Right
	void ChromaEditorSystemComponent::ShowRippleLREffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID frames[ChromaSDK::Keyboard::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					if ((r == 0) && (c == 20))
						continue;   // Skip the Razer logo.
					keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			AddFrame();
		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
			{
				//MouseEffect = {};

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}

		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {

			RZEFFECTID frames[ChromaSDK::Keypad::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
			{

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Ripple Effect - 1 Row Top to Bottom
	void ChromaEditorSystemComponent::ShowRippleUDEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID frames[ChromaSDK::Keyboard::MAX_ROW + 1];

			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
			{

				for (int c = ChromaSDK::Keyboard::MAX_COLUMN - 1; c >= 0; c--)
				{
					if ((r == 0) && (c == 20))
						continue;   // Skip the Razer logo.
					keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &frames[r]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[r], g_effectSpeed);

				AddFrame();
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			AddFrame();
		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_ROW + 1];

			for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &frames[r]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[r], g_effectSpeed);

				AddFrame();
			}
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			RZEFFECTID frames[ChromaSDK::Keypad::MAX_ROW + 1];

			for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
			{
				//KeypadEffect = {};

				for (int c = ChromaSDK::Keyboard::MAX_COLUMN - 1; c >= 0; c--)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &frames[r]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[r], g_effectSpeed);

				AddFrame();
			}
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Explosion Effect
	void ChromaEditorSystemComponent::ShowExplosionEffect() {
		#define ExplosionSize 4 

		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			RZEFFECTID frame0;
			RZEFFECTID frames[ExplosionSize];

			UINT baseIndex = m_currFrameNum - 1;
			
			// Show base color
			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keyboardFrames[baseIndex].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
			}

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[baseIndex], &frame0);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frame0, g_effectSpeed);

			AddFrame();

			// Show Explosion Frames
			int midRow = ChromaSDK::Keyboard::MAX_ROW / 2;
			int midCol = ChromaSDK::Keyboard::MAX_COLUMN / 2;
			int rInc = 1, cInc = 4;

			for (int count = 0; count < ChromaSDK::Keyboard::MAX_COLUMN / ExplosionSize; count++)
			{
				memcpy_s(&keyboardFrames[m_currFrameNum - 1], sizeof(keyboardFrames[m_currFrameNum - 1]), &keyboardFrames[baseIndex], sizeof(keyboardFrames[baseIndex]));
				for (int r = midRow-rInc; r <= midRow+rInc; r++)
				{
					if (r < 0)
						r = 0;
					for (int c = midCol-cInc; c < midCol+(cInc); c++)
					{
						if (c < 0)
							c = 0;
						keyboardFrames[m_currFrameNum - 1].Color[r][c] = RGB(235, 54, 0);
					}
				}
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &frames[count]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[count], g_effectSpeed);

				AddFrame();

				rInc++;
				cInc = cInc+4;
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			AddFrame();
		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			RZEFFECTID frames[ChromaSDK::Mouse::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
			{
				//MouseEffect = {};

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}

		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				AddFrame();
			}
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			RZEFFECTID frames[ChromaSDK::Keypad::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
			{
				//KeypadEffect = {};

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				AddFrame();
			}
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			RZEFFECTID frames[4] = {};

			//frame 0
			for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
			{
				chromalinkFrames[m_currFrameNum - 1].Color[i] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[0]);

			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[0], g_effectSpeed);

			AddFrame();

			//frame 1
			chromalinkFrames[m_currFrameNum - 1].Color[0] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[2] = RGB(235, 54, 0);
			chromalinkFrames[m_currFrameNum - 1].Color[3] = RGB(235, 54, 0);
			chromalinkFrames[m_currFrameNum - 1].Color[4] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[1]);

			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[1], g_effectSpeed);

			AddFrame();

			//frame 2
			chromalinkFrames[m_currFrameNum - 1].Color[0] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[1] = RGB(235, 54, 0);
			chromalinkFrames[m_currFrameNum - 1].Color[2] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[3] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[4] = RGB(235, 54, 0);

			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[2]);

			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[2], g_effectSpeed);

			AddFrame();

			//frame 3
			for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
			{
				chromalinkFrames[m_currFrameNum - 1].Color[i] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[3]);

			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[3], g_effectSpeed);

			AddFrame();
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}
	
	void ChromaEditorSystemComponent::ShowRainbowEffect() {
		const int RainbowSize = 7;

		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);
		
		RZEFFECTID Frames[RainbowSize] = { GUID_NULL };

		COLORREF rainbowArray[RainbowSize] = { RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET };

		UINT colorIndex = 0;
		UINT count = 0;
		while (count < RainbowSize)
		{
			if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
				for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
					{
						keyboardFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetRValue(rainbowArray[colorIndex])*g_effectBrightness, 
																			 GetGValue(rainbowArray[colorIndex])*g_effectBrightness, 
																			 GetBValue(rainbowArray[colorIndex])*g_effectBrightness);
					}
				}
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &Frames[colorIndex]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
					{
						mouseFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetRValue(rainbowArray[colorIndex])*g_effectBrightness,
																		  GetGValue(rainbowArray[colorIndex])*g_effectBrightness,
																		  GetBValue(rainbowArray[colorIndex])*g_effectBrightness);
					}
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &Frames[colorIndex]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {

				for (UINT r = 0; r < ChromaSDK::Mousepad::MAX_LEDS; r++)
				{
					mousepadFrames[m_currFrameNum - 1].Color[r] = RGB(GetRValue(rainbowArray[colorIndex])*g_effectBrightness,
																	  GetGValue(rainbowArray[colorIndex])*g_effectBrightness,
																	  GetBValue(rainbowArray[colorIndex])*g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &Frames[colorIndex]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {

				for (UINT r = 0; r < ChromaSDK::Headset::MAX_LEDS; r++)
				{
					headsetFrames[m_currFrameNum - 1].Color[r] = RGB(GetRValue(rainbowArray[colorIndex])*g_effectBrightness,
																	 GetGValue(rainbowArray[colorIndex])*g_effectBrightness,
																	 GetBValue(rainbowArray[colorIndex])*g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &Frames[colorIndex]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {

				for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
					{
						keypadFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetRValue(rainbowArray[colorIndex])*g_effectBrightness,
																		   GetGValue(rainbowArray[colorIndex])*g_effectBrightness,
																		   GetBValue(rainbowArray[colorIndex])*g_effectBrightness);
					}
				}
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &Frames[colorIndex]);
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {

				for (UINT r = 0; r < ChromaSDK::ChromaLink::MAX_LEDS; r++)
				{
					chromalinkFrames[m_currFrameNum - 1].Color[r] = RGB(GetRValue(rainbowArray[colorIndex])*g_effectBrightness,
																		GetGValue(rainbowArray[colorIndex])*g_effectBrightness,
																		GetBValue(rainbowArray[colorIndex])*g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &Frames[colorIndex]);
			}

			colorIndex++;
			count++;

			AddFrame();
		}

		for (int i = 0; i < RainbowSize; i++) {
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, Frames[i], g_effectSpeed);
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;
	}

	void ChromaEditorSystemComponent::ShowRandomEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		const int arraySize = 11;

		RZEFFECTID KBFrame = GUID_NULL;
		COLORREF randomArray[arraySize] = { RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET, WHITE, CYAN, PINK, GREY };

		UINT rand = 0;
		UINT colorIndex = 0;

		// Random number class
		AZ::Sfmt g;
		
		
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
 			//ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE KeyboardEffect = {};

			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
				{
					colorIndex = g.Rand32() % arraySize;
					keyboardFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetRValue(randomArray[colorIndex])*g_effectBrightness,
																		 GetGValue(randomArray[colorIndex])*g_effectBrightness,
																		 GetBValue(randomArray[colorIndex])*g_effectBrightness);
				}
			}
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);

		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			//ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2 MouseEffect = {};

			for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
				{
					colorIndex = g.Rand32() % arraySize;
					mouseFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetRValue(randomArray[colorIndex])*g_effectBrightness,
																	  GetGValue(randomArray[colorIndex])*g_effectBrightness,
																	  GetBValue(randomArray[colorIndex])*g_effectBrightness);
				}
			}
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);

		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			//ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE MousepadEffect = {};

			for (UINT r = 0; r < ChromaSDK::Mousepad::MAX_LEDS; r++)
			{
				colorIndex = g.Rand32() % arraySize;
				mousepadFrames[m_currFrameNum - 1].Color[r] = RGB(GetRValue(randomArray[colorIndex])*g_effectBrightness,
																  GetGValue(randomArray[colorIndex])*g_effectBrightness,
																  GetBValue(randomArray[colorIndex])*g_effectBrightness);
			}
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			//ChromaSDK::Headset::CUSTOM_EFFECT_TYPE HeadsetEffect = {};

			for (UINT r = 0; r < ChromaSDK::Headset::MAX_LEDS; r++)
			{
				colorIndex = g.Rand32() % arraySize;
				headsetFrames[m_currFrameNum - 1].Color[r] = RGB(GetRValue(randomArray[colorIndex])*g_effectBrightness,
																 GetGValue(randomArray[colorIndex])*g_effectBrightness,
																 GetBValue(randomArray[colorIndex])*g_effectBrightness);
			}
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			//ChromaSDK::Keypad::CUSTOM_EFFECT_TYPE KeypadEffect = {};

			for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
				{
					colorIndex = g.Rand32() % arraySize;
					keypadFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetRValue(randomArray[colorIndex])*g_effectBrightness,
																	   GetGValue(randomArray[colorIndex])*g_effectBrightness,
																	   GetBValue(randomArray[colorIndex])*g_effectBrightness);
				}
			}
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			//ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE ChromaLinkEffect = {};

			for (UINT r = 0; r < ChromaSDK::ChromaLink::MAX_LEDS; r++)
			{
				colorIndex = g.Rand32() % arraySize;
				chromalinkFrames[m_currFrameNum - 1].Color[r] = RGB(GetRValue(randomArray[colorIndex])*g_effectBrightness,
																	GetGValue(randomArray[colorIndex])*g_effectBrightness,
																	GetBValue(randomArray[colorIndex])*g_effectBrightness);
			}
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
		}

	}

	/************************************* END OF CHROMA PRESET EFFECTS *******************************************************/

	void ChromaEditorSystemComponent::LoadSingleImage() {
		ReadImageFile();
	}

	AZ::Crc32 ChromaEditorSystemComponent::LoadAnimation() {
		return ReadGifAnimationFile();
	}

	bool ChromaEditorSystemComponent::ReadImageFile() {
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
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
				QString newImageDir = QDir::currentPath() + QDir::separator() + "ChromaImageOutput/Frame0.png";
				//if (QDir().mkdir("Gems2")) {
				if (!image.save(newImageDir)) {
					CryLog("Image file save failed! ");
				}
				//}

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

	AZ::Crc32 ChromaEditorSystemComponent::ReadGifAnimationFile() {
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
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
					QString newImageDir = QDir::currentPath() + QDir::separator() + "ChromaImageOutput/";
					switch (frameIndex) {
					case 0:
						image.save(_T(newImageDir + "GIFFrame0.png")); break;
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

		m_currFrameNum = 1;
		newFrame = maxFrame[0];

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaEditorSystemComponent::SetKey() {
		// Only used for keyboard
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			//keyboardFrames[m_currFrameNum-1].Key[HIBYTE(ChromaKey)][LOBYTE(ChromaKey)] = 0x01000000 | GetCOLORREFValue(ChromaColor, g_effectBrightness);
			keyboardFrames[m_currFrameNum - 1].Color[HIBYTE(ChromaKey)][LOBYTE(ChromaKey)] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
	}

	void ChromaEditorSystemComponent::SetLed() {
		// Mouse Device - 9 x 7 is maxLEDs
		if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			if (ChromaLed < ChromaSDK::Mouse::MAX_LEDS2) {
				mouseFrames[m_currFrameNum - 1].Color[HIBYTE(chromaMouseLEDs[ChromaLed])][LOBYTE(chromaMouseLEDs[ChromaLed])] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
			}
		}

		// Mousepad Device - 15 is maxLEDs
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			if (ChromaLed < ChromaSDK::Mousepad::MAX_LEDS) {
				mousepadFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
			}
		}

		// Headset Device - 5 is maxLEDs
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			if (ChromaLed < ChromaSDK::Headset::MAX_LEDS) {
				headsetFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
			}
		}

		// Keypad Device - 4 x 5 is maxLEDs
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			if (ChromaLed < ChromaSDK::Keypad::MAX_KEYS) {
				int row = ChromaLed / ChromaSDK::Keypad::MAX_COLUMN;
				int col = ChromaLed % ChromaSDK::Keypad::MAX_COLUMN;

				keypadFrames[m_currFrameNum - 1].Color[row][col] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
			}
		}

		// Chromalink Device - 5 is maxLEDs
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			if (ChromaLed < ChromaSDK::ChromaLink::MAX_LEDS) {
				chromalinkFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
			}
		}
	}

	void ChromaEditorSystemComponent::PaintRow() {
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				if (cRow == 0 && c == 20)
					continue;

				keyboardFrames[m_currFrameNum - 1].Color[cRow-1][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
			{
				mouseFrames[m_currFrameNum - 1].Color[cRow - 1][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
			{
				keypadFrames[m_currFrameNum - 1].Color[cRow - 1][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		}
		else {
			QMessageBox::information(0, _T("Invalid Device"), _T("Can only paint rows on keyboards, keypads, and mice devices"));
		}
	}

	void ChromaEditorSystemComponent::PaintCol() {
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
			{
				if (r == 0 && cCol == 20)
					continue;

				keyboardFrames[m_currFrameNum - 1].Color[r][cCol-1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
			{
				mouseFrames[m_currFrameNum - 1].Color[r][cCol - 1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
			{
				keypadFrames[m_currFrameNum - 1].Color[r][cCol - 1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		}
		else {
			QMessageBox::information(0, _T("Invalid Device"), _T("Can only paint columns on keyboards, keypads, and mice devices"));
		}
	}

	// Frame Management Functions
	void ChromaEditorSystemComponent::FillFrame() {
		
		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
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
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
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
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
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
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
		}
	}

	void ChromaEditorSystemComponent::ClearFrame() {
		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			keyboardFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			mouseFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
		}
		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			mousepadFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
		}
		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			headsetFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
		}
		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			keypadFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		}
		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			chromalinkFrames[m_currFrameNum - 1] = {};
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
		}
	}

	AZ::Crc32 ChromaEditorSystemComponent::AddFrame() {
		if (newFrame == MAXFRAMES)
			return newFrame;
		else if (newFrame > MAXFRAMES) {
			newFrame = MAXFRAMES;
			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
		}

		maxFrame[ChromaEditorSystemComponent::chromaDeviceType]++;

		newFrame = maxFrame[ChromaEditorSystemComponent::chromaDeviceType];

		CryLog("Number of frames for current animation = %d ", maxFrame[ChromaEditorSystemComponent::chromaDeviceType]);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::Crc32 ChromaEditorSystemComponent::DeleteFrame() {
		if (newFrame == 0)
			return newFrame;
		else if (newFrame < 0) {
			newFrame = 0;
			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
		}

		// Clear Current Frame Buffer
		ClearFrame();

		// Shift frames in device array over to the left 1 slot
		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			for (int i = m_currFrameNum - 1; i < maxFrame[ChromaEditorSystemComponent::chromaDeviceType]; i++) {
				//ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE temp = keyboardFrames[i];
				
				if (i+1 <= maxFrame[ChromaEditorSystemComponent::chromaDeviceType])
					keyboardFrames[i] = keyboardFrames[i+1];
			}
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		//// Mouse Device
		//else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
		//	mouseFrames[m_currFrameNum - 1] = {};
		//	g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
		//}
		//// Mousepad Device
		//else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
		//	mousepadFrames[m_currFrameNum - 1] = {};
		//	g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
		//}
		//// Headset Device
		//else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
		//	headsetFrames[m_currFrameNum - 1] = {};
		//	g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
		//}
		//// Keypad Device
		//else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
		//	keypadFrames[m_currFrameNum - 1] = {};
		//	g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		//}
		//// Chromalink Device
		//else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
		//	chromalinkFrames[m_currFrameNum - 1] = {};
		//	g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
		//}

		maxFrame[ChromaEditorSystemComponent::chromaDeviceType]--;

		newFrame = maxFrame[ChromaEditorSystemComponent::chromaDeviceType];

		if (m_currFrameNum > newFrame)
			m_currFrameNum = newFrame;

		CryLog("Number of frames for current animation = %d ", maxFrame[ChromaEditorSystemComponent::chromaDeviceType]);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::Crc32 ChromaEditorSystemComponent::ResetAnimation() {
		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			for (int frameNum = 0; frameNum < maxFrame[0]; frameNum++) {
				keyboardFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[frameNum], NULL);
			}
			maxFrame[0] = 1;
			m_currDeviceEffects[0] = GUID_NULL;

		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			for (int frameNum = 0; frameNum < maxFrame[1]; frameNum++) {
				mouseFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[frameNum], NULL);
			}
			maxFrame[1] = 1;
			m_currDeviceEffects[1] = GUID_NULL;
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			for (int frameNum = 0; frameNum < maxFrame[2]; frameNum++) {
				mousepadFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[frameNum], NULL);
			}
			maxFrame[2] = 1;
			m_currDeviceEffects[2] = GUID_NULL;
		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			for (int frameNum = 0; frameNum < maxFrame[3]; frameNum++) {
				headsetFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[frameNum], NULL);
			}
			maxFrame[3] = 1;
			m_currDeviceEffects[3] = GUID_NULL;
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			for (int frameNum = 0; frameNum < maxFrame[4]; frameNum++) {
				keypadFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[frameNum], NULL);
			}
			maxFrame[4] = 1;
			m_currDeviceEffects[4] = GUID_NULL;
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			for (int frameNum = 0; frameNum < maxFrame[5]; frameNum++) {
				chromalinkFrames[frameNum] = {};
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[frameNum], NULL);
			}
			maxFrame[5] = 1;
			m_currDeviceEffects[5] = GUID_NULL;
		}

		m_currFrameNum = 1;
		newFrame = 1;
		m_currEffect = GUID_NULL;

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaEditorSystemComponent::CopyFrame() {
		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			copyKeyboardEffect = keyboardFrames[m_currFrameNum - 1];
		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			copyMouseEffect = mouseFrames[m_currFrameNum - 1];
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			copyMousepadEffect = mousepadFrames[m_currFrameNum - 1];
		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			copyHeadsetEffect = headsetFrames[m_currFrameNum - 1];
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			copyKeypadEffect = keypadFrames[m_currFrameNum - 1];
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			copyChromalinkEffect = chromalinkFrames[m_currFrameNum - 1];
		}
	}

	void ChromaEditorSystemComponent::PasteFrame() {
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			keyboardFrames[m_currFrameNum - 1] = copyKeyboardEffect;
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			mouseFrames[m_currFrameNum - 1] = copyMouseEffect;
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			mousepadFrames[m_currFrameNum - 1] = copyMousepadEffect;
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			headsetFrames[m_currFrameNum - 1] = copyHeadsetEffect;
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			keypadFrames[m_currFrameNum - 1] = copyKeypadEffect;
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
		}
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			chromalinkFrames[m_currFrameNum - 1] = copyChromalinkEffect;
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
		}

	}

	AZ::Crc32 ChromaEditorSystemComponent::JumpToFrame() {
		if (newFrame > MAXFRAMES) {
			newFrame = MAXFRAMES;
		}
		else if (newFrame < 1) {
			newFrame = 1;
		}
		else if (newFrame > maxFrame[ChromaEditorSystemComponent::chromaDeviceType]) {
			newFrame = maxFrame[ChromaEditorSystemComponent::chromaDeviceType];
		}

		m_currFrameNum = newFrame;

		ShowFrame(m_currFrameNum);

		CryLog("Current Chroma frame = %d ", m_currFrameNum);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::Crc32 ChromaEditorSystemComponent::NextFrame() {
		if (m_currFrameNum == MAXFRAMES)
			return m_currFrameNum;
		else if (m_currFrameNum > MAXFRAMES) {
			m_currFrameNum = MAXFRAMES;
			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
		}
		else if (m_currFrameNum == maxFrame[ChromaEditorSystemComponent::chromaDeviceType])
			return m_currFrameNum;
		
		m_currFrameNum++;

		ShowFrame(m_currFrameNum);

		CryLog("Current Chroma frame = %d ", m_currFrameNum);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZ::Crc32 ChromaEditorSystemComponent::PreviousFrame() {
		if (m_currFrameNum == 1)
			return m_currFrameNum;
		else if (m_currFrameNum < 1) {
			m_currFrameNum = 1;
			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
		}
		else if (m_currFrameNum == 0)
			return m_currFrameNum;

		m_currFrameNum--;

		ShowFrame(m_currFrameNum);

		CryLog("Current Chroma frame = %d ", m_currFrameNum);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaEditorSystemComponent::PlayCustomAnimation() {
		RZEFFECTID AnimationEffectId = GUID_NULL;
		if (IsEqualGUID(AnimationEffectId, GUID_NULL))
		{
			g_ChromaSDKImpl.CreateEffectGroup(&AnimationEffectId, repeatAnimation);
		}

		RZEFFECTID effectID = GUID_NULL;

		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			for (int fInx = 0; fInx < maxFrame[0]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			for (int fInx = 0; fInx < maxFrame[1]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			for (int fInx = 0; fInx < maxFrame[2]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			for (int fInx = 0; fInx < maxFrame[3]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			for (int fInx = 0; fInx < maxFrame[4]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}

		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			for (int fInx = 0; fInx < maxFrame[5]; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}
		}

		g_ChromaSDKImpl.SetEffectImpl(AnimationEffectId);
		m_currEffect = AnimationEffectId;
	}

	void ChromaEditorSystemComponent::PlayAllCustomAnimation() {
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

	void ChromaEditorSystemComponent::ShowFrame(int frame) {
		// Keyboard Device
		if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[frame-1], NULL);
		}

		// Mouse Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[frame - 1], NULL);
		}

		// Mousepad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[frame - 1], NULL);
		}

		// Headset Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[frame - 1], NULL);
		}

		// Keypad Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[frame - 1], NULL);
		}

		// Chromalink Device
		else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[frame-1], NULL);
		}
	}

	AZ::Crc32 ChromaEditorSystemComponent::ImportChromaEffect() {
		enum EChromaSDKDeviceTypeEnum
		{
			DE_1D = 0,
			DE_2D,
		};

		enum EChromaSDKDevice1DEnum
		{
			DE_ChromaLink = 0,
			DE_Headset,
			DE_Mousepad,
		};

		enum EChromaSDKDevice2DEnum
		{
			DE_Keyboard = 0,
			DE_Keypad,
			DE_Mouse,
		};

		QString chromaFilePath = QFileDialog::getOpenFileName(nullptr, "Import Chroma Effect", QString(), "Filter Files (*.chroma)");

		QFile chromaFile(chromaFilePath);

		if (chromaFile.open(QFile::ReadOnly))
		{
			QDataStream inSteam(&chromaFile);

			// Read the Effect File Animation Version
			int version = ANIMATION_VERSION;
			inSteam.readRawData(reinterpret_cast<char*>(&version), sizeof(version));
			if (version != 1)
				CryLog("Imported Invalid Animation Version %d", version);

			// Read the Effect File Device Type
			qint8 deviceType = 0;
			qint8 device = 0;

			// Read the Effect File DeviceType and Device
			inSteam.readRawData(reinterpret_cast<char*>(&deviceType), sizeof(deviceType));
			inSteam.readRawData(reinterpret_cast<char*>(&device), sizeof(device));

			if (deviceType == (qint8)DE_1D) {
				if (device == EChromaSDKDevice1DEnum::DE_Mousepad) {
					ChromaEditorSystemComponent::chromaDeviceType = 2;
				}
				else if (device == EChromaSDKDevice1DEnum::DE_Headset) {
					ChromaEditorSystemComponent::chromaDeviceType = 3;
				}
				else if (device == EChromaSDKDevice1DEnum::DE_ChromaLink) {
					ChromaEditorSystemComponent::chromaDeviceType = 5;
				}
			}
			else {
				if (device == EChromaSDKDevice2DEnum::DE_Keyboard) {
					ChromaEditorSystemComponent::chromaDeviceType = 0;
				}
				else if (device == EChromaSDKDevice2DEnum::DE_Mouse) {
					ChromaEditorSystemComponent::chromaDeviceType = 1;
				}
				else if (device == EChromaSDKDevice2DEnum::DE_Keypad) {
					ChromaEditorSystemComponent::chromaDeviceType = 4;
				}
			}

			// Read the Effect File FrameCount
			UINT frameCount = 0;
			inSteam.readRawData(reinterpret_cast<char*>(&frameCount), sizeof(frameCount));
			maxFrame[ChromaEditorSystemComponent::chromaDeviceType] += frameCount;

			// Read the Effect File Duration
			float duration = 0.0;

			// Read the Effect Colors
			int color = 0;
			for (int f = m_currFrameNum-1; f < maxFrame[ChromaEditorSystemComponent::chromaDeviceType]; f++) {

				inSteam.readRawData(reinterpret_cast<char*>(&duration), sizeof(duration));
				g_effectSpeed = duration * 1000.0f;

				if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
					for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
					{
						for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
						{
							inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

							if (color != 0)
								CryLog("Import Effect: Row = %d | Col = %d | Color = %d", r, c, color);

							keyboardFrames[f].Color[r][c] = color;
						}
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
					for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
					{
						for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
						{
							inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

							if (color != 0)
								CryLog("Import Effect: Row = %d | Col = %d | Color = %d", r, c, color);

							mouseFrames[f].Color[r][c] = color;
						}
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
					for (UINT c = 0; c < ChromaSDK::Mousepad::MAX_LEDS; c++)
					{
						inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

						if (color != 0)
							CryLog("Import Effect: LED = %d | Color = %d", c, color);

						mousepadFrames[f].Color[c] = color;
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
					for (UINT c = 0; c < ChromaSDK::Headset::MAX_LEDS; c++)
					{
						inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

						if (color != 0)
							CryLog("Import Effect: LED = %d | Color = %d", c, color);

						headsetFrames[f].Color[c] = color;
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
					for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
					{
						for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
						{
							inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

							if (color != 0)
								CryLog("Import Effect: Row = %d | Col = %d | Color = %d", r, c, color);

							keypadFrames[f].Color[r][c] = color;
						}
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
					for (UINT c = 0; c < ChromaSDK::ChromaLink::MAX_LEDS; c++)
					{
						inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

						if (color != 0)
							CryLog("Import Effect: LED = %d | Color = %d", c, color);

						chromalinkFrames[f].Color[c] = color;
					}
				}
			}
		}

		chromaFile.close();
		//m_currFrameNum = 1;
		newFrame = maxFrame[ChromaEditorSystemComponent::chromaDeviceType];
		ShowFrame(m_currFrameNum);

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	void ChromaEditorSystemComponent::ExportChromaEffect() {
		enum EChromaSDKDeviceTypeEnum
		{
			DE_1D = 0,
			DE_2D,
		};

		enum EChromaSDKDevice1DEnum
		{
			DE_ChromaLink = 0,
			DE_Headset,
			DE_Mousepad,
		};

		enum EChromaSDKDevice2DEnum
		{
			DE_Keyboard = 0,
			DE_Keypad,
			DE_Mouse,
		};

		QString chromaFilePath = QFileDialog::getSaveFileName(nullptr, "Save Chroma Effect", QString(), "Filter Files (*.chroma)");

		QFile chromaFile(chromaFilePath);

		if (chromaFile.open(QFile::WriteOnly))
		{
			QDataStream outStream(&chromaFile);

			// Write the Effect File Animation Version
			int version = ANIMATION_VERSION;
			outStream.writeRawData(reinterpret_cast<const char*>(&version), sizeof(version));

			// Write the Effect File Device Type
			qint8 deviceType = 0;
			qint8 device = 0;
			if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_2D;
				device = (qint8)EChromaSDKDevice2DEnum::DE_Keyboard;
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_2D;
				device = (qint8)EChromaSDKDevice2DEnum::DE_Mouse;
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_2D;
				device = (qint8)EChromaSDKDevice2DEnum::DE_Keypad;
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_1D;
				device = (qint8)EChromaSDKDevice1DEnum::DE_Mousepad;
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_1D;
				device = (qint8)EChromaSDKDevice1DEnum::DE_Headset;
			}
			else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_1D;
				device = (qint8)EChromaSDKDevice1DEnum::DE_ChromaLink;
			}

			// Write the Effect File DeviceType and Device
			//outStream << deviceType << device;
			outStream.writeRawData(reinterpret_cast<const char*>(&deviceType), sizeof(deviceType));
			outStream.writeRawData(reinterpret_cast<const char*>(&device), sizeof(device));

			// Write the Effect File FrameCount
			UINT frameCount = maxFrame[ChromaEditorSystemComponent::chromaDeviceType];
			outStream.writeRawData(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));

			// Write the Effect File Duration
			// Compute the duration as the (frameCount-1)*frameDelay
			float duration = (float)(g_effectSpeed / 1000.0f);

			// Write the Effect Colors
			int color = 0;
			QByteArray colorbytes;

			for (int f = 0; f < maxFrame[ChromaEditorSystemComponent::chromaDeviceType]; f++) {

				outStream.writeRawData(reinterpret_cast<const char*>(&duration), sizeof(duration));

				if (ChromaEditorSystemComponent::chromaDeviceType == 0) {
					for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
					{
						for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
						{
							color = keyboardFrames[f].Color[r][c];
							if (color != 0)
								CryLog("Export Effect: Row = %d | Col = %d | Color = %d", r, c, color);

							char slen = strlen(reinterpret_cast<const char*>(&color));

							outStream.writeRawData(reinterpret_cast<const char*>(&color), 4);
						}
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 1) {
					for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
					{
						for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
						{
							color = mouseFrames[f].Color[r][c];
							if (color != 0)
								CryLog("Export Effect: Row = %d | Col = %d | Color = %d", r, c, color);
							outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
						}
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 2) {
					for (UINT c = 0; c < ChromaSDK::Mousepad::MAX_LEDS; c++)
					{
						color = mousepadFrames[f].Color[c];
						if (color != 0)
							CryLog("Export Effect: LED = %d | Color = %d", c, color);
						outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 3) {
					for (UINT c = 0; c < ChromaSDK::Headset::MAX_LEDS; c++)
					{
						color = headsetFrames[f].Color[c];
						if (color != 0)
							CryLog("Export Effect: LED = %d | Color = %d", c, color);
						outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 4) {
					for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
					{
						for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
						{
							color = keypadFrames[f].Color[r][c];
							if (color != 0)
								CryLog("Export Effect: Row = %d | Col = %d | Color = %d", r, c, color);
							outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
						}
					}
				}
				else if (ChromaEditorSystemComponent::chromaDeviceType == 5) {
					for (UINT c = 0; c < ChromaSDK::ChromaLink::MAX_LEDS; c++)
					{
						color = chromalinkFrames[f].Color[c];
						if (color != 0)
							CryLog("Export Effect: LED = %d | Color = %d", c, color);
						outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
					}
				}
			}
		}

		chromaFile.close();

	}

	void ChromaEditorSystemComponent::ExportChromaHeaderFile() {
		// Export the Color Buffer Code Values
		#define BREAK       _T("\n")
		#define LINE_BREAK  _T("\n\n")

		QString chromaHeaderPath = QFileDialog::getSaveFileName(nullptr, "Export Chroma Colors In Header File", QString(), "Filter Files (*.h)");

		QFile chromaHeaderFile(chromaHeaderPath);

		if (chromaHeaderFile.open(QFile::WriteOnly | QFile::Text))
		{
			QString FileTitle = chromaHeaderFile.fileName();
			int fileNameIndex = FileTitle.lastIndexOf('/');
			FileTitle = FileTitle.remove(0, fileNameIndex + 1);
			FileTitle.chop(2);
			FileTitle = FileTitle.toUpper();

			QString ChromaCode;

			QTextStream outStream(&chromaHeaderFile);

			outStream << "#ifndef _" << FileTitle << "_H_" << BREAK;
			outStream << "#define _" << FileTitle << "_H_" << LINE_BREAK;

			outStream << "#pragma once" << LINE_BREAK;

			outStream << "#include \"RzChromaSDKDefines.h\"" << BREAK;
			outStream << "#include \"RzChromaSDKTypes.h\"" << BREAK;
			outStream << "#include \"RzErrors.h\"" << LINE_BREAK;

			outStream << "using namespace ChromaSDK;" << LINE_BREAK;

			int color = 0;
			int framecount = maxFrame[ChromaEditorSystemComponent::chromaDeviceType];

			outStream << "ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE " << FileTitle.append("Frames") << "[" << framecount << "] = { 0 };" << LINE_BREAK;

			outStream << "COLORREF EffectFrameData[" << framecount << "][" << ChromaSDK::Keyboard::MAX_ROW << "][" << ChromaSDK::Keyboard::MAX_COLUMN << "] = {" << BREAK;
			//outStream << "     {" << BREAK;

			for (int findex = 0; findex < framecount; findex++) {
				outStream << "        {" << BREAK;
				for (UINT i = 0; i < ChromaSDK::Keyboard::MAX_ROW; i++)
				{
					outStream << "        {";
					for (UINT j = 0; j < ChromaSDK::Keyboard::MAX_COLUMN; j++)
					{
						color = keyboardFrames[findex].Color[i][j];

						outStream << color;

						if (j < ChromaSDK::Keyboard::MAX_COLUMN - 1)
						{
							outStream << ", ";
						}
					}

					outStream << "}";

					if (i < ChromaSDK::Keyboard::MAX_ROW - 1)
					{
						outStream << "," << BREAK;
					}
					else
						outStream << BREAK;
				}

				outStream << "        }," << LINE_BREAK;
			}

			outStream << "};" << LINE_BREAK;

			// Write out the for loops to store data in CUSTOM_KEY_EFFECT_TYPE Struct if more than 1 frame)
			if (framecount > 1) {
				outStream << "	for (int frameIndex = 0; frameIndex < " << framecount << "; frameIndex++) {" << BREAK;
				outStream << "		for (int row = 0; row < " << ChromaSDK::Keyboard::MAX_ROW << "; row++) {" << BREAK;
				outStream << "			for (int col = 0; col < " << ChromaSDK::Keyboard::MAX_COLUMN << "; col++) {" << BREAK;
				outStream << "				EffectDataArray [frameIndex].Color[row][col] = EffectFrameData[frameIndex][row][col];" << BREAK;
				outStream << "			}" << BREAK;
				outStream << "		}" << BREAK;
				outStream << "	}" << LINE_BREAK;
			}

			outStream << "#endif";

			chromaHeaderFile.close();
		}
	}
}

