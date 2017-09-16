
#include "StdAfx.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <Editor/QtViewPaneManager.h> // For calendar demo
#include <QCalendarWidget> // For calendar demo
#include <AzCore/Math/Sfmt.h>

#include "ChromaSystemComponent.h"

#include "IRenderer.h"

namespace Chroma
{
	ChromaSystemComponent::ChromaSystemComponent() {
		chromaDeviceType = 0;
		chromaEffectValue = 0;
		m_currFrameNum = 1;
		ChromaKey = ChromaSDK::Keyboard::RZKEY_ESC;
		ChromaLed = ChromaSDK::Mouse::RZLED_NONE;
		g_effectBrightness = 1.0;
		g_effectSpeed = 100;
		m_currEffect = GUID_NULL;
		repeatAnimation = FALSE;
		m_currFrameNum = 1;
	}

	ChromaSystemComponent::~ChromaSystemComponent() {

	}
	
	void ChromaSystemComponent::Init()
	{
		chromaDeviceType = 0;
		chromaEffectValue = 0;
		m_currFrameNum = 1;
		ChromaKey = ChromaSDK::Keyboard::RZKEY_ESC;
		ChromaLed = ChromaSDK::Mouse::RZLED_NONE;
		g_effectBrightness = 1.0;
		g_effectSpeed = 100;
		m_currEffect = GUID_NULL;
		repeatAnimation = FALSE;
		m_currFrameNum = 1;
	}

	void ChromaSystemComponent::Activate()
	{
		chromaDeviceType = 0;
		chromaEffectValue = 0;
		m_currFrameNum = 1;
		ChromaKey = ChromaSDK::Keyboard::RZKEY_ESC;
		ChromaLed = ChromaSDK::Mouse::RZLED_NONE;
		g_effectBrightness = 1.0;
		g_effectSpeed = 100;
		m_currEffect = GUID_NULL;
		repeatAnimation = FALSE;

		ChromaRequestBus::Handler::BusConnect();

		CrySystemEventBus::Handler::BusConnect(); // For calendar demo

												  // connect to the TickBus
		AZ::TickBus::Handler::BusConnect();

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
				->Field("Load Image", &ChromaSystemComponent::importImage)
				->Field("Load Animation", &ChromaSystemComponent::importAnimation)
				->Field("Play Custom Effect", &ChromaSystemComponent::playCustomEffect)
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
						//->Attribute(AZ::Edit::Attributes::ValueText, &ChromaSystemComponent::g_effectSpeed)
					->DataElement(AZ::Edit::UIHandlers::CheckBox, &ChromaSystemComponent::repeatAnimation, "Repeat Animation", "Set to repeat animation indefintely")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::stopEffect, "Stop Effect", "Stop the current animation")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::StopEffect)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::clearEffects, "Clear Effects", "Clear All Chroma Devices")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ClearEffects)
					->ClassElement(AZ::Edit::ClassElements::Group, "Chroma Preset Effects")
						->Attribute(AZ::Edit::Attributes::AutoExpand, false)
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaSystemComponent::chromaEffectValue, "Select Preset Effect", "Select Chroma Effect Preset")
						->EnumAttribute(ChromaEffectValue::STATIC, "Static")
						->EnumAttribute(ChromaEffectValue::FLASH, "Flash")
						->EnumAttribute(ChromaEffectValue::WAVE, "Wave")
						->EnumAttribute(ChromaEffectValue::BREATHING, "Breathing")
						->EnumAttribute(ChromaEffectValue::CUSTOM, "Custom")
						->EnumAttribute(ChromaEffectValue::RANDOM, "Random")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::playPresetEffect, "Play Preset Effect", "Plays selected preset effect")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PlayPresetEffect)
					->ClassElement(AZ::Edit::ClassElements::Group, "Chroma Custom Effects")
						->Attribute(AZ::Edit::Attributes::AutoExpand, false)
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
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_SCROLLWHEEL, "Scroll Wheel LED")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_LOGO, "Logo LED")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_BACKLIGHT, "Backlight LED")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_LEFT_SIDE1, "Left LED 1")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_LEFT_SIDE2, "Left LED 2")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_LEFT_SIDE3, "Left LED 3")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_LEFT_SIDE4, "Left LED 4")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_LEFT_SIDE5, "Left LED 5")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_LEFT_SIDE6, "Left LED 6")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_LEFT_SIDE7, "Left LED 7")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_BOTTOM1, "Bottom LED 1")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_BOTTOM2, "Bottom LED 2")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_BOTTOM3, "Bottom LED 3")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_BOTTOM4, "Bottom LED 4")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_BOTTOM5, "Bottom LED 5")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_RIGHT_SIDE1, "Right LED 1")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_RIGHT_SIDE2, "Right LED 2")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_RIGHT_SIDE3, "Right LED 3")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_RIGHT_SIDE4, "Right LED 4")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_RIGHT_SIDE5, "Right LED 5")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_RIGHT_SIDE6, "Right LED 6")
						->EnumAttribute(ChromaSDK::Mouse::RZLED2_RIGHT_SIDE7, "Right LED 7")
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
						->Attribute(AZ::Edit::Attributes::ValueText, &ChromaSystemComponent::m_currFrameNum)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::nextFrame, "Next Frame", "Increment Frame Count")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::IncrementFrame)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::prevFrame, "Prev Frame", "Decrement Frame Count")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::DecrementFrame)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::importImage, "Import Image", "Import jpg, png, bmp")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::LoadSingleImage)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::importAnimation, "Import Animation", "Import gif animations")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::LoadAnimation)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::playCustomEffect, "Play Custom Animation", "Plays custom animation frames")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PlayCustomAnimation)
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
		g_ChromaSDKImpl.StopEffectImpl(m_currEffect);
	}
	void ChromaSystemComponent::ClearEffects() {
		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_NONE, NULL, NULL);

		keyboardGrid = {};
	}

	void ChromaSystemComponent::SetChromaDeviceType(AZ::u32 deviceType) {
		g_ChromaSDKImpl.mDeviceType = deviceType;
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
		case 4:			// Custom Effect 
		{
			ShowCustomEffect();
			break;
		}
		case 5:			// Random Effect 
		{
			ShowRandomEffect();
			break;
		}

		}

	}

	void ChromaSystemComponent::ShowWaveEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		if (ChromaSystemComponent::chromaDeviceType == 0) {
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE KeyboardEffect = {};

			RZEFFECTID frames[ChromaSDK::Keyboard::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				//memcpy_s(&KeyboardEffect, sizeof(KeyboardEffect), &g_KeyboardEffect, sizeof(g_KeyboardEffect));
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
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2 MouseEffect = {};

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
			{
				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					MouseEffect.Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &MouseEffect, &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);
			}
		}
		/*else if (ChromaSystemComponent::chromaDeviceType == 2) {
		ChromaSDK::Mousepad::WAVE_EFFECT_TYPE waveEffect = {};
		waveEffect.Direction = ChromaSDK::Mousepad::WAVE_EFFECT_TYPE::DIRECTION_LEFT_TO_RIGHT;

		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_WAVE, &waveEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
		ChromaSDK::Keypad::WAVE_EFFECT_TYPE waveEffect = {};
		waveEffect.Direction = ChromaSDK::Keypad::WAVE_EFFECT_TYPE::DIRECTION_LEFT_TO_RIGHT;

		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_WAVE, &waveEffect, NULL);
		}*/

		/*if (ChromaSystemComponent::chromaDeviceType == 0) {
			ChromaSDK::Keyboard::WAVE_EFFECT_TYPE waveEffect = {};
			waveEffect.Direction = ChromaSDK::Keyboard::WAVE_EFFECT_TYPE::DIRECTION_LEFT_TO_RIGHT;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_WAVE, &waveEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
			ChromaSDK::Mouse::WAVE_EFFECT_TYPE waveEffect = {};
			waveEffect.Direction = ChromaSDK::Mouse::WAVE_EFFECT_TYPE::BACK_TO_FRONT;

			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_WAVE, &waveEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			ChromaSDK::Mousepad::WAVE_EFFECT_TYPE waveEffect = {};
			waveEffect.Direction = ChromaSDK::Mousepad::WAVE_EFFECT_TYPE::DIRECTION_LEFT_TO_RIGHT;

			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_WAVE, &waveEffect, NULL);
		}
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
			ChromaSDK::Keypad::WAVE_EFFECT_TYPE waveEffect = {};
			waveEffect.Direction = ChromaSDK::Keypad::WAVE_EFFECT_TYPE::DIRECTION_LEFT_TO_RIGHT;

			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_WAVE, &waveEffect, NULL);
		}*/
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

		//for (int repeatNum = 0; repeatNum < 5; repeatNum++) {
			for (int i = 0; i < 11; i++) {
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Frames[i], g_effectSpeed);
			}
		//}
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

	void ChromaSystemComponent::LoadAnimation() {

	}

	void ChromaSystemComponent::ReadImageFile() {

		CString FrameName;

		//if (m_Frames.GetCount() <= MAXIMAGES) {
			//CFileDialog File(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Image Files (*.bmp;*.jpg;*.png)| *.bmp;*.jpg;*.png ||"));
			//if (File.DoModal() == IDOK)
			//{
				// Check if a GIF is currently loaded. If it is, then remove it from the frame list window
				/*if (m_Frames.GetCount() == 2) {
					m_Frames.GetText(1, FrameName);
					if (FrameName.Mid(0, 3) == "GIF") {
						m_Frames.DeleteString(1);
					}
				}*/

				//m_currFileName = File.GetPathName();

				//		if (m_Frames.GetCount() > 1) {
				//			int iIndex = m_FilePaths.GetCount() - 1;

				//			POSITION Pos = m_FilePaths.FindIndex(iIndex);

				//			if (Pos != NULL) {

				//				m_FilePaths.SetAt(Pos, m_currFileName);

				//				FrameName.Format(_T("Frame %d (%s)"), iIndex + 1, m_currFileName.GetBuffer());

				//				m_Frames.InsertString(++iIndex, FrameName);
				//			}
				//		}
				//		else {												//Pressed Load Button To Add First Frame
				//			POSITION Pos = m_FilePaths.GetHeadPosition();

				//			if (Pos != NULL) {

				//				m_FilePaths.SetAt(Pos, m_currFileName);

				//				FrameName.Format(_T("Frame 1 (%s)"), m_currFileName.GetBuffer());

				//				m_Frames.InsertString(1, FrameName);
				//			}
				//		}

				//	}
				//}
				//else {
				//	AfxMessageBox(_T("Unable to add frame. Max number of frames reached"), MB_OK);
				//}
			//}
	}

	void ChromaSystemComponent::ShowCustomEffect() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			float white[4] = { 1.f, 1.f, 1.f, 1.f };
			if (gEnv && gEnv->pRenderer)
			{
				gEnv->pRenderer->Draw2dLabel(32.f, 32.f, 2.f, white, false,
					"Chroma Gem says hello. Coming soon from Razer.");
			}

			static RZEFFECTID GroupEffectId = GUID_NULL;

			if (IsEqualGUID(GroupEffectId, GUID_NULL))
			{
				COLORREF Color[3] = { WHITE, RED, BLUE };

				ChromaSDK::Keyboard::WAVE_EFFECT_TYPE waveEff = {};
				waveEff.Direction = ChromaSDK::Keyboard::WAVE_EFFECT_TYPE::DIRECTION_LEFT_TO_RIGHT;

				RZEFFECTID Effects[30];

				g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

				ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE customEffect = {};

				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_NONE, NULL, &Effects[0]);

				customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_C)][LOBYTE(ChromaSDK::Keyboard::RZKEY_C)] = YELLOW | 0x01000000;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[0]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Effects[0], 1000);

				customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_H)][LOBYTE(ChromaSDK::Keyboard::RZKEY_H)] = PURPLE | 0x01000000;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[1]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Effects[1], 1000);

				customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_R)][LOBYTE(ChromaSDK::Keyboard::RZKEY_R)] = CYAN | 0x01000000;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[2]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Effects[2], 1000);

				customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_O)][LOBYTE(ChromaSDK::Keyboard::RZKEY_O)] = ORANGE | 0x01000000;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[3]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Effects[3], 1000);

				customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_M)][LOBYTE(ChromaSDK::Keyboard::RZKEY_M)] = PINK | 0x01000000;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[4]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Effects[4], 1000);

				customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_A)][LOBYTE(ChromaSDK::Keyboard::RZKEY_A)] = GREY | 0x01000000;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[5]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Effects[5], 3000);


				COLORREF tempColor = NULL;
				UINT colorIndex = 0;
				for (UINT i = 0; i < 15; i++) {
					tempColor = Color[colorIndex];

					customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_C)][LOBYTE(ChromaSDK::Keyboard::RZKEY_C)] = tempColor | 0x01000000;
					g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[6 + i]);

					customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_H)][LOBYTE(ChromaSDK::Keyboard::RZKEY_H)] = tempColor | 0x01000000;
					g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[6 + i]);

					customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_R)][LOBYTE(ChromaSDK::Keyboard::RZKEY_R)] = tempColor | 0x01000000;
					g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[6 + i]);

					customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_O)][LOBYTE(ChromaSDK::Keyboard::RZKEY_O)] = tempColor | 0x01000000;
					g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[6 + i]);

					customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_M)][LOBYTE(ChromaSDK::Keyboard::RZKEY_M)] = tempColor | 0x01000000;
					g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[6 + i]);

					customEffect.Key[HIBYTE(ChromaSDK::Keyboard::RZKEY_A)][LOBYTE(ChromaSDK::Keyboard::RZKEY_A)] = tempColor | 0x01000000;
					g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &customEffect, &Effects[6 + i]);

					g_ChromaSDKImpl.AddToGroup(GroupEffectId, Effects[6 + i], 200);

					colorIndex++;
					if (colorIndex == 3)
						colorIndex = 0;
				}

				g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
				m_currEffect = GroupEffectId;
			}
		}
	}

	void ChromaSystemComponent::SetKey() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			keyboardFrames[m_currFrameNum-1].Key[HIBYTE(ChromaKey)][LOBYTE(ChromaKey)] = 0x01000000 | GetCOLORREFValue(ChromaColor, g_effectBrightness);
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
	}

	void ChromaSystemComponent::SetLed() {

	}

	void ChromaSystemComponent::PaintRow() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				keyboardFrames[m_currFrameNum - 1].Color[cRow][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
	}

	void ChromaSystemComponent::PaintCol() {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
			{
				keyboardFrames[m_currFrameNum - 1].Color[r][cCol] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
		}
	}

	// Frame Management Functions
	void ChromaSystemComponent::JumpToFrame() {
		if (newFrame > MAXFRAMES) {
			newFrame = MAXFRAMES;
		}
		else if (newFrame < 1) {
			newFrame = 1;
		}
		m_currFrameNum = newFrame;

		if (m_currFrameNum > maxFrame)
			maxFrame = m_currFrameNum;

		ShowFrame(m_currFrameNum);
	}

	void ChromaSystemComponent::IncrementFrame() {
		if (m_currFrameNum == MAXFRAMES)
			return;
		else if (m_currFrameNum > MAXFRAMES) {
			m_currFrameNum = 100;
			return;
		}
		
		newFrame = m_currFrameNum + 1;
		m_currFrameNum = newFrame;

		if (m_currFrameNum > maxFrame)
			maxFrame = m_currFrameNum;

		ShowFrame(m_currFrameNum);

		float white[4] = { 1.f, 1.f, 1.f, 1.f };
		if (gEnv && gEnv->pRenderer)
		{
			gEnv->pRenderer->Draw2dLabel(32.f, 32.f, 2.f, white, false,
				"Current Chroma Frame = %d");
		}
	}

	void ChromaSystemComponent::DecrementFrame() {
		if (m_currFrameNum == 1)
			return;
		else if (m_currFrameNum < 1) {
			m_currFrameNum = 1;
			return;
		}

		newFrame = m_currFrameNum - 1;
		m_currFrameNum = newFrame;

		ShowFrame(m_currFrameNum);
	}

	void ChromaSystemComponent::PlayCustomAnimation() {
		RZEFFECTID AnimationEffectId = GUID_NULL;
		if (IsEqualGUID(AnimationEffectId, GUID_NULL))
		{
			g_ChromaSDKImpl.CreateEffectGroup(&AnimationEffectId, repeatAnimation);
		}

		RZEFFECTID effectID = GUID_NULL;

		if (ChromaSystemComponent::chromaDeviceType == 0) {
			for (int fInx = 0; fInx < maxFrame; fInx++) {
				effectID = GUID_NULL;
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[fInx], &effectID);
				g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
			}

			g_ChromaSDKImpl.SetEffectImpl(AnimationEffectId);
			m_currEffect = AnimationEffectId;
		}
	}

	void ChromaSystemComponent::ShowFrame(int frame) {
		if (ChromaSystemComponent::chromaDeviceType == 0) {
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[frame-1], NULL);
		}
	}
}

