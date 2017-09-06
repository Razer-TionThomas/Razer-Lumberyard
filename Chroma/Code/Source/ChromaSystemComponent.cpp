
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
		g_effectBrightness = 1.0;
		g_effectSpeed = 100;
		m_currEffect = GUID_NULL;
		repeatAnimation = FALSE;
	}

	ChromaSystemComponent::~ChromaSystemComponent() {

	}
	
	void ChromaSystemComponent::Init()
	{
		chromaDeviceType = 0;
		chromaEffectValue = 0;
		g_effectBrightness = 1.0;
		g_effectSpeed = 100;
		m_currEffect = GUID_NULL;
		repeatAnimation = FALSE;
	}

	void ChromaSystemComponent::Activate()
	{
		chromaDeviceType = 0;
		chromaEffectValue = 0;
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
				//->Field("Static Effect", &ChromaSystemComponent::staticEffect)
				//->Field("Flash Effect", &ChromaSystemComponent::flashEffect)
				//->Field("Breathing Effect", &ChromaSystemComponent::breathingEffect)
				//->Field("Wave Effect", &ChromaSystemComponent::waveEffect)
				//->Field("Random Effect", &ChromaSystemComponent::randomEffect)
				//->Field("Custom Effect", &ChromaSystemComponent::customEffect)
				->Field("Select Effect Preset", &ChromaSystemComponent::chromaEffectValue)
				->Field("Play Effect", &ChromaSystemComponent::playEffect)
				->Field("Stop Effect", &ChromaSystemComponent::stopEffect)
				->Field("Clear Effect", &ChromaSystemComponent::clearEffects)
				->Field("Load Image", &ChromaSystemComponent::importImage)
				->Field("Load Animation", &ChromaSystemComponent::importAnimation);

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<ChromaSystemComponent>("Chroma", "Adds Chroma lighting effects to game objects")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					// ->Attribute(AZ::Edit::Attributes::Category, "") Set a category
					->Attribute(AZ::Edit::Attributes::Category, "RGBLighting")
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
					->DataElement(AZ::Edit::UIHandlers::Slider, &ChromaSystemComponent::g_effectBrightness, "Brightness Control", "TBD")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::SetEffectBrightness)
						->Attribute(AZ::Edit::Attributes::Min, 0.0f)
						->Attribute(AZ::Edit::Attributes::Max, 1.0f)
						->Attribute(AZ::Edit::Attributes::Step, 0.1f)
					->DataElement(AZ::Edit::UIHandlers::Default, &ChromaSystemComponent::g_effectSpeed, "Effect Speed", "TBD")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::SetEffectSpeed)
						->Attribute(AZ::Edit::Attributes::ValueText, "100")
					->DataElement(AZ::Edit::UIHandlers::CheckBox, &ChromaSystemComponent::repeatAnimation, "Repeat Animation", "TBD")
					->DataElement(AZ::Edit::UIHandlers::ComboBox, &ChromaSystemComponent::chromaEffectValue, "Select Preset Effect", "Select Chroma Effect Preset")
						->EnumAttribute(ChromaEffectValue::STATIC, "Static")
						->EnumAttribute(ChromaEffectValue::FLASH, "Flash")
						->EnumAttribute(ChromaEffectValue::WAVE, "Wave")
						->EnumAttribute(ChromaEffectValue::BREATHING, "Breathing")
						->EnumAttribute(ChromaEffectValue::CUSTOM, "Custom")
						->EnumAttribute(ChromaEffectValue::RANDOM, "Random")
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::playEffect, "Play Effect", "TBD")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::PlayEffect)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::stopEffect, "Stop Effect", "TBD")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::StopEffect)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::clearEffects, "Clear Effects", "TBD")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ClearEffects)
					//->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::breathingEffect, "Breathing Effect", "TBD")
					//	->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ShowBreathingEffect)
					//->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::waveEffect, "Wave Effect", "TBD")
					//	->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ShowWaveEffect)
					//->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::randomEffect, "Random Effect", "TBD")
					//	->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ShowRandomEffect)
					//->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::customEffect, "Custom Effect", "Keyboard Only")
					//->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ShowCustomEffect)
					//->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::staticEffect, "Static Effect", "Button to trigger static effect")
					//	->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ShowStaticEffect)
					//->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::flashEffect, "Flash Effect", "Button to trigger flash effect")
					//	->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::ShowFlashEffect)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::importImage, "Import Image", "Import jpg, png, bmp")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::LoadSingleImage)
					->DataElement(AZ::Edit::UIHandlers::Button, &ChromaSystemComponent::importAnimation, "Import Animation", "Import gif animations")
						->Attribute(AZ::Edit::Attributes::ChangeNotify, &ChromaSystemComponent::LoadAnimation)
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
	}

	void ChromaSystemComponent::SetChromaDeviceType(AZ::u32 deviceType) {
		g_ChromaSDKImpl.mDeviceType = deviceType;
	}

	void ChromaSystemComponent::PlayEffect() {
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
}

