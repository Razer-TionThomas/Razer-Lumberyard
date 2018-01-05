
#include "StdAfx.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <AzCore/Math/Sfmt.h>

#include "ChromaSystemComponent.h"

#include "IRenderer.h"

//#include <qobject.h>
//#include <qfiledialog>
//#include <qmessagebox>
//#include <qmovie>

#define ANIMATION_VERSION 1

namespace Chroma
{
	ChromaSystemComponent::ChromaSystemComponent() {
		chromaDeviceType = 0;
		chromaEffectValue = 0;
		ChromaColor = AZ::Color::Color(1.0f, 1.0f, 1.0f, 1.0f);
		m_currFrameNum = 1;
		ChromaKey = ChromaSDK::Keyboard::RZKEY_ESC;
		ChromaLed = ChromaSDK::Mouse::RZLED_NONE;
		g_effectBrightness = 1.0;
		g_effectSpeed = 300;
		m_currEffect = GUID_NULL;
		repeatAnimation = FALSE;

		//for (int d = 0; d < MAXDEVICES; d++)
		//	m_currDeviceEffects[d] = GUID_NULL;

	}

	ChromaSystemComponent::~ChromaSystemComponent() {

	}
	
	void ChromaSystemComponent::Init()
	{
	}

	void ChromaSystemComponent::Activate()
	{
		ChromaRequestBus::Handler::BusConnect();

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
				->SerializerForEmptyClass();

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<ChromaSystemComponent>("Chroma", "Adds Chroma APIs for Ebus")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					;
			}
		}

		else if (AZ::BehaviorContext* behavior = azrtti_cast<AZ::BehaviorContext*>(context)) {
			behavior->EBus<ChromaRequestBus>("ChromaRequestBus")
				->Event("ChromaSDKInit", &ChromaRequestBus::Events::ChromaSDKInit)
				->Event("ChromaSDKUnInit", &ChromaRequestBus::Events::ChromaSDKUnInit)
				->Event("PlayPresetChromaEffect", &ChromaRequestBus::Events::PlayPresetChromaEffect)
				->Event("StopChromaEffect", &ChromaRequestBus::Events::StopChromaEffect)
				->Event("ClearChromaEffects", &ChromaRequestBus::Events::ClearChromaEffects)
				->Event("SetEffectBrightness", &ChromaRequestBus::Events::SetEffectBrightness)
				->Event("SetEffectSpeed", &ChromaRequestBus::Events::SetEffectSpeed)
				->Event("SetEffectColor", &ChromaRequestBus::Events::SetEffectColor)
			;
		}
	}

	// Ebus Requests
	void ChromaSystemComponent::ChromaSDKInit() {
		AZ_Printf("Chroma", "Recevied a chroma init request via Ebus!!");
		g_ChromaSDKImpl.Initialize();
	}

	void ChromaSystemComponent::ChromaSDKUnInit() {
		AZ_Printf("Chroma", "Recevied a chroma uninit request via Ebus!!");
		g_ChromaSDKImpl.UnInitialize();
	}

	void ChromaSystemComponent::PlayPresetChromaEffect(int effectId, int device, bool repeat) {
		AZ_Printf("Chroma", "Recevied a play chroma effect request via Ebus!!");

		ChromaSystemComponent::chromaDeviceType = device;
		ChromaSystemComponent::repeatAnimation = repeat;

		switch (effectId) {
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

		case 2:			// WaveLR1 Effect
		{
			ShowWaveLR1Effect();
			break;
		}

		case 3:			// WaterFall Effect
		{
			ShowWaterFallEffect();
			break;
		}

		case 4:			// WaveCross Effect
		{
			ShowWaveCross1Effect();
			break;
		}

		case 5:			// Breathing Effect
		{
			ShowBreathingEffect();
			break;
		}

		case 6:			// RippleLR Effect
		{
			ShowRippleLREffect();
			break;
		}

		case 7:			// RippleUD Effect
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

	void ChromaSystemComponent::StopChromaEffect() {
		AZ_Printf("Chroma", "Recevied a stop chroma effect request via Ebus!!");
		StopEffect();
	}

	void ChromaSystemComponent::ClearChromaEffects() {
		AZ_Printf("Chroma", "Recevied a clear effects request via Ebus!!");
		ClearDeviceEffects();
	}

	void ChromaSystemComponent::SetEffectBrightness(float brightness) {
		AZ_Printf("Chroma", "Recevied a SetBrightness request via Ebus!!");
		ChromaSystemComponent::g_effectBrightness = brightness;
	}

	void ChromaSystemComponent::SetEffectSpeed(int speed) {
		AZ_Printf("Chroma", "Recevied a SetSpeed request via Ebus!!");
		ChromaSystemComponent::g_effectSpeed = speed;
	}

	void ChromaSystemComponent::SetEffectColor(float red, float green, float blue, float alpha) {
		AZ_Printf("Chroma", "Recevied a SetColor request via Ebus!!");
		ChromaColor = AZ::Color::Color(red, green, blue, alpha);
	}

	/**** END OF EBUS *****/

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

	void ChromaSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time) {

	}

	COLORREF ChromaSystemComponent::GetCOLORREFValue(AZ::Color color, double brightness) {
		#pragma warning(disable:4244)
		
		COLORREF retColor = 0;
		
		AZ::u8 red = color.GetR8()* brightness;
		AZ::u8 green = color.GetG8() * brightness;
		AZ::u8 blue = color.GetB8() * brightness;

		retColor = RGB(red, green, blue);

		#pragma warning(default:4244)

		return retColor;
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

	void ChromaSystemComponent::ClearDeviceEffects() {
		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_NONE, NULL, NULL);
		g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_NONE, NULL, NULL);

		copyKeyboardEffect = {};
	}

	/************************************* Chroma Preset Effects *******************************************************
	These effects are precoded by the author of this plugin and can be used for quick and simple effect implementation.
	Note that the brightness and effect speed can still be modified in the editor and the frame data can be exported into
	a .chroma effect file
	********************************************************************************************************************/

	// Flash Effect - Single Flash of All LEDs
	void ChromaSystemComponent::ShowFlashEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID frames[2];

			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
					keyboardFrames[m_currFrameNum].Color[r][c] = RGB(0, 0, 0);
				}
			}

			// Add the color frame
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &frames[0]);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[0], g_effectSpeed);
			

			// Add the blank frame
			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum], &frames[1]);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[1], g_effectSpeed);
			

		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {

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

				
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}

		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {

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

				
			}
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Wave Effect - Left to Right
	void ChromaSystemComponent::ShowWaveLR1Effect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {

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
				

				memcpy_s(&keyboardFrames[m_currFrameNum - 1], sizeof(keyboardFrames[m_currFrameNum - 1]), &keyboardFrames[tempFrameIndex - 1], sizeof(keyboardFrames[tempFrameIndex - 1]));
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {

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

				
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}

		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {

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

				
			}
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Wave - Vertical Up to Down
	void ChromaSystemComponent::ShowWaterFallEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID frames[ChromaSDK::Keyboard::MAX_ROW + 1];
			RZEFFECTID lastframe;

			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW + 2; r++)
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

				
				if (m_currFrameNum < MAXFRAMES)
					m_currFrameNum++;
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			

		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {

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

				
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}

		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {

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

				
			}
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Wave - Double Cross Effect
	void ChromaSystemComponent::ShowWaveCross1Effect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {

			RZEFFECTID Frames[ChromaSDK::Keyboard::MAX_COLUMN + 1];

			for (int c1 = 0, c2 = ChromaSDK::Keyboard::MAX_COLUMN - 1; c1 < ChromaSDK::Keyboard::MAX_COLUMN; c1++, c2--) {

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keyboardFrames[m_currFrameNum - 1].Color[r][c1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
					keyboardFrames[m_currFrameNum - 1].Color[r][c2] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &Frames[c1]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, Frames[c1], g_effectSpeed);

				
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {

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

				
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}

		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {

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

				
			}
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Breathing Effect
	void ChromaSystemComponent::ShowBreathingEffect() {
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

			if (ChromaSystemComponent::chromaDeviceType == 0) {

				for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
					{
						keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 1) {

				for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
					{
						mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 2) {

				for (UINT r = 0; r < ChromaSDK::Mousepad::MAX_LEDS; r++)
				{
					mousepadFrames[m_currFrameNum - 1].Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 3) {

				for (UINT r = 0; r < ChromaSDK::Headset::MAX_LEDS; r++)
				{
					headsetFrames[m_currFrameNum - 1].Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 4) {

				for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
				{
					for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
					{
						keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, brightness);
					}
				}
				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &Frames[index]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 5) {

				for (UINT r = 0; r < ChromaSDK::ChromaLink::MAX_LEDS; r++)
				{
					chromalinkFrames[m_currFrameNum - 1].Color[r] = GetCOLORREFValue(ChromaColor, brightness);
				}
				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &Frames[index]);
			}

			index++;
			count++;

			
		}

		for (int i = 0; i < brightnessCount; i++) {
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, Frames[i], g_effectSpeed);
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;
	}

	// Ripple Effect - 1 Column Left to Right
	void ChromaSystemComponent::ShowRippleLREffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {

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

				
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {

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

				
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}

		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {

			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {

			RZEFFECTID frames[ChromaSDK::Keypad::MAX_COLUMN + 1];

			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
			{

				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}

				g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], &frames[c]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[c], g_effectSpeed);

				
			}
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {

			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}


		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Ripple Effect - 1 Row Top to Bottom
	void ChromaSystemComponent::ShowRippleUDEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {

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

				
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {

			RZEFFECTID frames[ChromaSDK::Mouse::MAX_ROW + 1];

			for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
			{
				for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
				{
					mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], &frames[r]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[r], g_effectSpeed);

				
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {

			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
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

				
			}
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			RZEFFECTID frames[ChromaSDK::ChromaLink::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
			{
				//ChromalinkEffect = {};
				chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	// Explosion Effect
	void ChromaSystemComponent::ShowExplosionEffect() {
#define ExplosionSize 4 

		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		// Keyboard Device
		if (ChromaSystemComponent::chromaDeviceType == 0) {
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

			

			// Show Explosion Frames
			int midRow = ChromaSDK::Keyboard::MAX_ROW / 2;
			int midCol = ChromaSDK::Keyboard::MAX_COLUMN / 2;
			int rInc = 1, cInc = 4;

			for (int count = 0; count < ChromaSDK::Keyboard::MAX_COLUMN / ExplosionSize; count++)
			{
				memcpy_s(&keyboardFrames[m_currFrameNum - 1], sizeof(keyboardFrames[m_currFrameNum - 1]), &keyboardFrames[baseIndex], sizeof(keyboardFrames[baseIndex]));
				for (int r = midRow - rInc; r <= midRow + rInc; r++)
				{
					if (r < 0)
						r = 0;
					for (int c = midCol - cInc; c < midCol + (cInc); c++)
					{
						if (c < 0)
							c = 0;
						keyboardFrames[m_currFrameNum - 1].Color[r][c] = RGB(235, 54, 0);
					}
				}
				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &frames[count]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[count], g_effectSpeed);

				

				rInc++;
				cInc = cInc + 4;
			}

			// Reset Keyboard To All Black
			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE clearKB = GetBlackKBFrame();
			keyboardFrames[m_currFrameNum - 1] = clearKB;
			RZEFFECTID lastframe;

			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], &lastframe);
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, lastframe, 0);

			
		}

		// Mouse Device
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
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

				
			}
		}

		// Mousepad Device
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
			RZEFFECTID frames[ChromaSDK::Mousepad::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
			{
				//MousePadEffect = {};
				mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}

		}

		// Headset Device
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
			RZEFFECTID frames[ChromaSDK::Headset::MAX_LEDS + 1];

			for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
			{
				//HeadsetEffect = {};
				headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &frames[l]);
				g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[l], g_effectSpeed);

				
			}
		}

		// Keypad Device
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
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

				
			}
		}

		// Chromalink Device
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
			RZEFFECTID frames[4] = {};

			//frame 0
			for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
			{
				chromalinkFrames[m_currFrameNum - 1].Color[i] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[0]);

			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[0], g_effectSpeed);

			

			//frame 1
			chromalinkFrames[m_currFrameNum - 1].Color[0] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[2] = RGB(235, 54, 0);
			chromalinkFrames[m_currFrameNum - 1].Color[3] = RGB(235, 54, 0);
			chromalinkFrames[m_currFrameNum - 1].Color[4] = GetCOLORREFValue(ChromaColor, g_effectBrightness);

			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[1]);

			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[1], g_effectSpeed);

			

			//frame 2
			chromalinkFrames[m_currFrameNum - 1].Color[0] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[1] = RGB(235, 54, 0);
			chromalinkFrames[m_currFrameNum - 1].Color[2] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[3] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			chromalinkFrames[m_currFrameNum - 1].Color[4] = RGB(235, 54, 0);

			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[2]);

			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[2], g_effectSpeed);

			

			//frame 3
			for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
			{
				chromalinkFrames[m_currFrameNum - 1].Color[i] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
			}
			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], &frames[3]);

			g_ChromaSDKImpl.AddToGroup(GroupEffectId, frames[3], g_effectSpeed);

			
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;

	}

	void ChromaSystemComponent::ShowRainbowEffect() {
		const int RainbowSize = 7;

		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		RZEFFECTID Frames[RainbowSize] = { GUID_NULL };

		COLORREF rainbowArray[RainbowSize] = { RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET };

		UINT colorIndex = 0;
		UINT count = 0;
		while (count < RainbowSize)
		{
			if (ChromaSystemComponent::chromaDeviceType == 0) {
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
			else if (ChromaSystemComponent::chromaDeviceType == 1) {

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
			else if (ChromaSystemComponent::chromaDeviceType == 2) {

				for (UINT r = 0; r < ChromaSDK::Mousepad::MAX_LEDS; r++)
				{
					mousepadFrames[m_currFrameNum - 1].Color[r] = RGB(GetRValue(rainbowArray[colorIndex])*g_effectBrightness,
						GetGValue(rainbowArray[colorIndex])*g_effectBrightness,
						GetBValue(rainbowArray[colorIndex])*g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], &Frames[colorIndex]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 3) {

				for (UINT r = 0; r < ChromaSDK::Headset::MAX_LEDS; r++)
				{
					headsetFrames[m_currFrameNum - 1].Color[r] = RGB(GetRValue(rainbowArray[colorIndex])*g_effectBrightness,
						GetGValue(rainbowArray[colorIndex])*g_effectBrightness,
						GetBValue(rainbowArray[colorIndex])*g_effectBrightness);
				}
				g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], &Frames[colorIndex]);
			}
			else if (ChromaSystemComponent::chromaDeviceType == 4) {

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
			else if (ChromaSystemComponent::chromaDeviceType == 5) {

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

			
		}

		for (int i = 0; i < RainbowSize; i++) {
			g_ChromaSDKImpl.AddToGroup(GroupEffectId, Frames[i], g_effectSpeed);
		}

		g_ChromaSDKImpl.SetEffectImpl(GroupEffectId);
		m_currEffect = GroupEffectId;
	}

	void ChromaSystemComponent::ShowRandomEffect() {
		RZEFFECTID GroupEffectId = GUID_NULL;
		g_ChromaSDKImpl.CreateEffectGroup(&GroupEffectId, repeatAnimation);

		const int arraySize = 11;

		RZEFFECTID KBFrame = GUID_NULL;
		COLORREF randomArray[arraySize] = { RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET, WHITE, CYAN, PINK, GREY };

		UINT rand = 0;
		UINT colorIndex = 0;

		// Random number class
		AZ::Sfmt g;


		if (ChromaSystemComponent::chromaDeviceType == 0) {
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
		else if (ChromaSystemComponent::chromaDeviceType == 1) {
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
		else if (ChromaSystemComponent::chromaDeviceType == 2) {
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
		else if (ChromaSystemComponent::chromaDeviceType == 3) {
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
		else if (ChromaSystemComponent::chromaDeviceType == 4) {
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
		else if (ChromaSystemComponent::chromaDeviceType == 5) {
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

	//void ChromaSystemComponent::LoadSingleImage() {
	//	ReadImageFile();
	//}

	//AZ::Crc32 ChromaSystemComponent::LoadAnimation() {
	//	return ReadGifAnimationFile();
	//}

	//bool ChromaSystemComponent::ReadImageFile() {
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		QString filename = QFileDialog::getOpenFileName(nullptr, "Load Image", QString(), "Search Filter Files (*.bmp;*.jpg;*.png)");
	//		QFile file(filename);
	//		if (!file.open(QIODevice::ReadOnly))
	//		{
	//			QMessageBox::information(0, _T("Unable to Open File"), _T("Unable to open image file."));
	//			return false;
	//		}

	//		QByteArray rawImage = file.readAll();
	//		QPixmap pixmap;

	//		pixmap.loadFromData(rawImage);
	//		if (!pixmap.isNull())
	//		{
	//			QSize pixmapSize = pixmap.size();
	//			QSize chromaSize(ChromaSDK::Keyboard::MAX_COLUMN, ChromaSDK::Keyboard::MAX_ROW);

	//			pixmap = pixmap.scaled(chromaSize);
	//			QImage image = pixmap.toImage();

	//			// For testing purposes
	//			QString newImageDir = QDir::currentPath() + QDir::separator() + "ChromaImageOutput/Frame0.png";
	//			//if (QDir().mkdir("Gems2")) {
	//			if (!image.save(newImageDir)) {
	//				CryLog("Image file save failed! ");
	//			}
	//			//}

	//			for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
	//			{
	//				// Get ths RGB bits for each row
	//				const uchar *rowData = image.scanLine(r);

	//				COLORREF *pColor = (COLORREF*)rowData;

	//				for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
	//				{
	//					// Fill up the array
	//					keyboardFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetBValue(*pColor), GetGValue(*pColor), GetRValue(*pColor));
	//					pColor++;
	//				}
	//			}
	//			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM, &keyboardFrames[m_currFrameNum - 1], NULL);
	//		}
	//	}
	//	else {
	//		QMessageBox::information(0, _T("Invalid Device"), _T("Please change device type to keyboard for importing an image file."));
	//		return false;
	//	}

	//	return true;
	//}

	//AZ::Crc32 ChromaSystemComponent::ReadGifAnimationFile() {
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		QString filename = QFileDialog::getOpenFileName(nullptr, "Load Image", QString(), "Search Filter Files (*.gif)");
	//		QFile file(filename);
	//		if (!file.open(QIODevice::ReadOnly))
	//		{
	//			QMessageBox::information(0, _T("Unable to Open File"), _T("Unable to open animation file."));
	//			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//		}

	//		QMovie chromaGIF(filename);
	//		chromaGIF.setScaledSize(QSize(ChromaSDK::Keyboard::MAX_COLUMN, ChromaSDK::Keyboard::MAX_ROW));

	//		int animationFrameCount = chromaGIF.frameCount();

	//		if (animationFrameCount > MAXFRAMES) {
	//			QMessageBox::information(0, _T("Invalid Animation File"), _T("The animation file exceeds tha maximum number of frames allowed"));
	//			return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//		}

	//		int currentFrameNum = 0;

	//		//For each frame in the animation
	//		for (int frameIndex = 0; frameIndex < animationFrameCount; frameIndex++) {
	//			if (m_currFrameNum > MAXFRAMES) {
	//				QMessageBox::information(0, _T("Frame Limit Hit"), _T("The maximum number of frames has been reached"));
	//				break;
	//			}

	//			currentFrameNum = chromaGIF.currentFrameNumber();

	//			QImage image = chromaGIF.currentImage();

	//			if (!image.isNull())
	//			{
	//				// For testing purposes
	//				QString newImageDir = QDir::currentPath() + QDir::separator() + "ChromaImageOutput/";
	//				switch (frameIndex) {
	//				case 0:
	//					image.save(_T(newImageDir + "GIFFrame0.png")); break;
	//				case 5:
	//					image.save(_T("LumberyardChromaImageOutput/GIFFrame5.png")); break;
	//				case 10:
	//					image.save(_T("LumberyardChromaImageOutput/GIFFrame10.png")); break;
	//				case 15:
	//					image.save(_T("LumberyardChromaImageOutput/GIFFrame15.png")); break;
	//				}

	//				for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
	//				{
	//					// Get ths RGB bits for each row
	//					const uchar *rowData = image.scanLine(r);

	//					COLORREF *pColor = (COLORREF*)rowData;

	//					for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
	//					{
	//						// Fill up the array
	//						keyboardFrames[m_currFrameNum - 1].Color[r][c] = RGB(GetBValue(*pColor), GetGValue(*pColor), GetRValue(*pColor));
	//						pColor++;
	//					}
	//				}
	//				g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM, &keyboardFrames[m_currFrameNum - 1], NULL);
	//				
	//				m_currFrameNum++;
	//				if (m_currFrameNum > maxFrame[0])
	//					maxFrame[0] = m_currFrameNum;
	//			}
	//			chromaGIF.jumpToNextFrame();
	//		}
	//	}
	//	else {
	//		QMessageBox::information(0, _T("Invalid Device"), _T("Please change device type to keyboard for importing an image file."));
	//		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//	}

	//	m_currFrameNum = 1;
	//	newFrame = maxFrame[0];

	//	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//}

	//void ChromaSystemComponent::SetKey() {
	//	// Only used for keyboard
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		//keyboardFrames[m_currFrameNum-1].Key[HIBYTE(ChromaKey)][LOBYTE(ChromaKey)] = 0x01000000 | GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		keyboardFrames[m_currFrameNum - 1].Color[HIBYTE(ChromaKey)][LOBYTE(ChromaKey)] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
	//	}
	//}

	//void ChromaSystemComponent::SetLed() {
	//	// Mouse Device - 9 x 7 is maxLEDs
	//	if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		if (ChromaLed < ChromaSDK::Mouse::MAX_LEDS2) {
	//			mouseFrames[m_currFrameNum - 1].Color[HIBYTE(chromaMouseLEDs[ChromaLed])][LOBYTE(chromaMouseLEDs[ChromaLed])] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
	//		}
	//	}

	//	// Mousepad Device - 15 is maxLEDs
	//	else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//		if (ChromaLed < ChromaSDK::Mousepad::MAX_LEDS) {
	//			mousepadFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
	//		}
	//	}

	//	// Headset Device - 5 is maxLEDs
	//	else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//		if (ChromaLed < ChromaSDK::Headset::MAX_LEDS) {
	//			headsetFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
	//		}
	//	}

	//	// Keypad Device - 4 x 5 is maxLEDs
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		if (ChromaLed < ChromaSDK::Keypad::MAX_KEYS) {
	//			int row = ChromaLed / ChromaSDK::Keypad::MAX_COLUMN;
	//			int col = ChromaLed % ChromaSDK::Keypad::MAX_COLUMN;

	//			keypadFrames[m_currFrameNum - 1].Color[row][col] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
	//		}
	//	}

	//	// Chromalink Device - 5 is maxLEDs
	//	else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//		if (ChromaLed < ChromaSDK::ChromaLink::MAX_LEDS) {
	//			chromalinkFrames[m_currFrameNum - 1].Color[ChromaLed] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
	//		}
	//	}
	//}

	//void ChromaSystemComponent::PaintRow() {
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
	//		{
	//			if (cRow == 0 && c == 20)
	//				continue;

	//			keyboardFrames[m_currFrameNum - 1].Color[cRow-1][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
	//		{
	//			mouseFrames[m_currFrameNum - 1].Color[cRow - 1][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
	//		{
	//			keypadFrames[m_currFrameNum - 1].Color[cRow - 1][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else {
	//		QMessageBox::information(0, _T("Invalid Device"), _T("Can only paint rows on keyboards, keypads, and mice devices"));
	//	}
	//}

	//void ChromaSystemComponent::PaintCol() {
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
	//		{
	//			if (r == 0 && cCol == 20)
	//				continue;

	//			keyboardFrames[m_currFrameNum - 1].Color[r][cCol-1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
	//		{
	//			mouseFrames[m_currFrameNum - 1].Color[r][cCol - 1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
	//		{
	//			keypadFrames[m_currFrameNum - 1].Color[r][cCol - 1] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else {
	//		QMessageBox::information(0, _T("Invalid Device"), _T("Can only paint columns on keyboards, keypads, and mice devices"));
	//	}
	//}

	//// Frame Management Functions
	//void ChromaSystemComponent::FillFrame() {
	//	
	//	// Keyboard Device
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
	//		{
	//			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
	//			{
	//				keyboardFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//			}
	//		}

	//		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
	//	}

	//	// Mouse Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
	//		{
	//			for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
	//			{
	//				mouseFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//			}
	//		}

	//		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
	//	}

	//	// Mousepad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//		for (UINT l = 0; l < ChromaSDK::Mousepad::MAX_LEDS; l++)
	//		{
	//			mousepadFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
	//	}

	//	// Headset Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//		for (UINT l = 0; l < ChromaSDK::Headset::MAX_LEDS; l++)
	//		{
	//			headsetFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
	//	}

	//	// Keypad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
	//		{
	//			for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
	//			{
	//				keypadFrames[m_currFrameNum - 1].Color[r][c] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//			}
	//		}

	//		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
	//	}

	//	// Chromalink Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//		for (UINT l = 0; l < ChromaSDK::ChromaLink::MAX_LEDS; l++)
	//		{
	//			chromalinkFrames[m_currFrameNum - 1].Color[l] = GetCOLORREFValue(ChromaColor, g_effectBrightness);
	//		}

	//		g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
	//	}
	//}

	//void ChromaSystemComponent::ClearFrame() {
	//	// Keyboard Device
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		keyboardFrames[m_currFrameNum - 1] = {};
	//		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
	//	}
	//	// Mouse Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		mouseFrames[m_currFrameNum - 1] = {};
	//		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
	//	}
	//	// Mousepad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//		mousepadFrames[m_currFrameNum - 1] = {};
	//		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
	//	}
	//	// Headset Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//		headsetFrames[m_currFrameNum - 1] = {};
	//		g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
	//	}
	//	// Keypad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		keypadFrames[m_currFrameNum - 1] = {};
	//		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
	//	}
	//	// Chromalink Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//		chromalinkFrames[m_currFrameNum - 1] = {};
	//		g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
	//	}
	//}

	//AZ::Crc32 ChromaSystemComponent::ClearAllFrames() {
	//	// Keyboard Device
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		for (int frameNum = 0; frameNum < maxFrame[0]; frameNum++) {
	//			keyboardFrames[frameNum] = {};
	//			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[frameNum], NULL);
	//		}
	//		maxFrame[0] = 1;
	//		m_currDeviceEffects[0] = GUID_NULL;

	//	}

	//	// Mouse Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		for (int frameNum = 0; frameNum < maxFrame[1]; frameNum++) {
	//			mouseFrames[frameNum] = {};
	//			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[frameNum], NULL);
	//		}
	//		maxFrame[1] = 1;
	//		m_currDeviceEffects[1] = GUID_NULL;
	//	}

	//	// Mousepad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//		for (int frameNum = 0; frameNum < maxFrame[2]; frameNum++) {
	//			mousepadFrames[frameNum] = {};
	//			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[frameNum], NULL);
	//		}
	//		maxFrame[2] = 1;
	//		m_currDeviceEffects[2] = GUID_NULL;
	//	}

	//	// Headset Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//		for (int frameNum = 0; frameNum < maxFrame[3]; frameNum++) {
	//			headsetFrames[frameNum] = {};
	//			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[frameNum], NULL);
	//		}
	//		maxFrame[3] = 1;
	//		m_currDeviceEffects[3] = GUID_NULL;
	//	}

	//	// Keypad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		for (int frameNum = 0; frameNum < maxFrame[4]; frameNum++) {
	//			keypadFrames[frameNum] = {};
	//			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[frameNum], NULL);
	//		}
	//		maxFrame[4] = 1;
	//		m_currDeviceEffects[4] = GUID_NULL;
	//	}

	//	// Chromalink Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//		for (int frameNum = 0; frameNum < maxFrame[5]; frameNum++) {
	//			chromalinkFrames[frameNum] = {};
	//			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[frameNum], NULL);
	//		}
	//		maxFrame[5] = 1;
	//		m_currDeviceEffects[5] = GUID_NULL;
	//	}

	//	m_currFrameNum = 1;
	//	m_currEffect = GUID_NULL;

	//	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//}

	//void ChromaSystemComponent::CopyFrame() {
	//	// Keyboard Device
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		copyKeyboardEffect = keyboardFrames[m_currFrameNum - 1];
	//	}

	//	// Mouse Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		copyMouseEffect = mouseFrames[m_currFrameNum - 1];
	//	}

	//	// Mousepad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//		copyMousepadEffect = mousepadFrames[m_currFrameNum - 1];
	//	}

	//	// Headset Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//		copyHeadsetEffect = headsetFrames[m_currFrameNum - 1];
	//	}

	//	// Keypad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		copyKeypadEffect = keypadFrames[m_currFrameNum - 1];
	//	}

	//	// Chromalink Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//		copyChromalinkEffect = chromalinkFrames[m_currFrameNum - 1];
	//	}
	//}

	//void ChromaSystemComponent::PasteFrame() {
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		keyboardFrames[m_currFrameNum - 1] = copyKeyboardEffect;
	//		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		mouseFrames[m_currFrameNum - 1] = copyMouseEffect;
	//		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//		mousepadFrames[m_currFrameNum - 1] = copyMousepadEffect;
	//		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//		headsetFrames[m_currFrameNum - 1] = copyHeadsetEffect;
	//		g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		keypadFrames[m_currFrameNum - 1] = copyKeypadEffect;
	//		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[m_currFrameNum - 1], NULL);
	//	}
	//	else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//		chromalinkFrames[m_currFrameNum - 1] = copyChromalinkEffect;
	//		g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[m_currFrameNum - 1], NULL);
	//	}

	//}

	//AZ::Crc32 ChromaSystemComponent::JumpToFrame() {
	//	if (newFrame > MAXFRAMES) {
	//		newFrame = MAXFRAMES;
	//	}
	//	else if (newFrame < 1) {
	//		newFrame = 1;
	//	}
	//	m_currFrameNum = newFrame;

	//	if (m_currFrameNum > maxFrame[ChromaSystemComponent::chromaDeviceType])
	//		maxFrame[ChromaSystemComponent::chromaDeviceType] = m_currFrameNum;

	//	ShowFrame(m_currFrameNum);

	//	CryLog("Current Chroma frame = %d ", m_currFrameNum);

	//	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//}

	//AZ::Crc32 ChromaSystemComponent::IncrementFrame() {
	//	if (m_currFrameNum == MAXFRAMES)
	//		return m_currFrameNum;
	//	else if (m_currFrameNum > MAXFRAMES) {
	//		m_currFrameNum = MAXFRAMES;
	//		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//	}
	//	
	//	newFrame = m_currFrameNum + 1;
	//	m_currFrameNum = newFrame;

	//	if (m_currFrameNum > maxFrame[ChromaSystemComponent::chromaDeviceType])
	//		maxFrame[ChromaSystemComponent::chromaDeviceType] = m_currFrameNum;

	//	ShowFrame(m_currFrameNum);

	//	CryLog("Current Chroma frame = %d ", m_currFrameNum);

	//	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//}

	//AZ::Crc32 ChromaSystemComponent::DecrementFrame() {
	//	if (m_currFrameNum == 1)
	//		return m_currFrameNum;
	//	else if (m_currFrameNum < 1) {
	//		m_currFrameNum = 1;
	//		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//	}

	//	newFrame = m_currFrameNum - 1;
	//	m_currFrameNum = newFrame;

	//	ShowFrame(m_currFrameNum);

	//	CryLog("Current Chroma frame = %d ", m_currFrameNum);

	//	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//}

	//void ChromaSystemComponent::PlayCustomAnimation() {
	//	RZEFFECTID AnimationEffectId = GUID_NULL;
	//	if (IsEqualGUID(AnimationEffectId, GUID_NULL))
	//	{
	//		g_ChromaSDKImpl.CreateEffectGroup(&AnimationEffectId, repeatAnimation);
	//	}

	//	RZEFFECTID effectID = GUID_NULL;

	//	// Keyboard Device
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		for (int fInx = 0; fInx < maxFrame[0]; fInx++) {
	//			effectID = GUID_NULL;
	//			g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[fInx], &effectID);
	//			g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
	//		}
	//	}

	//	// Mouse Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		for (int fInx = 0; fInx < maxFrame[1]; fInx++) {
	//			effectID = GUID_NULL;
	//			g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[fInx], &effectID);
	//			g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
	//		}
	//	}

	//	// Mousepad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//		for (int fInx = 0; fInx < maxFrame[2]; fInx++) {
	//			effectID = GUID_NULL;
	//			g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[fInx], &effectID);
	//			g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
	//		}
	//	}

	//	// Headset Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//		for (int fInx = 0; fInx < maxFrame[3]; fInx++) {
	//			effectID = GUID_NULL;
	//			g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[fInx], &effectID);
	//			g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
	//		}
	//	}

	//	// Keypad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		for (int fInx = 0; fInx < maxFrame[4]; fInx++) {
	//			effectID = GUID_NULL;
	//			g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[fInx], &effectID);
	//			g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
	//		}

	//	}

	//	// Chromalink Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//		for (int fInx = 0; fInx < maxFrame[5]; fInx++) {
	//			effectID = GUID_NULL;
	//			g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[fInx], &effectID);
	//			g_ChromaSDKImpl.AddToGroup(AnimationEffectId, effectID, g_effectSpeed);
	//		}
	//	}

	//	g_ChromaSDKImpl.SetEffectImpl(AnimationEffectId);
	//	m_currEffect = AnimationEffectId;
	//}

	//void ChromaSystemComponent::PlayAllCustomAnimation() {
	//	RZEFFECTID KBAnimationEffectId = GUID_NULL;
	//	RZEFFECTID MouseAnimationEffectId = GUID_NULL;
	//	RZEFFECTID MPAnimationEffectId = GUID_NULL;
	//	RZEFFECTID HSAnimationEffectId = GUID_NULL;
	//	RZEFFECTID KPAnimationEffectId = GUID_NULL;
	//	RZEFFECTID CLAnimationEffectId = GUID_NULL;

	//	RZEFFECTID effectID = GUID_NULL;

	//	for (int m = 0; m < MAXDEVICES; m++) {
	//		if (maxFrame[m] > 1) {					// If maxFrames > 1, then there must be an animation specified
	//			switch (m) {
	//				case 0: {		// Keyboard
	//					g_ChromaSDKImpl.CreateEffectGroup(&KBAnimationEffectId, repeatAnimation);

	//					for (int fInx = 0; fInx < maxFrame[0]; fInx++) {
	//						effectID = GUID_NULL;
	//						g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[fInx], &effectID);
	//						g_ChromaSDKImpl.AddToGroup(KBAnimationEffectId, effectID, g_effectSpeed);
	//					}
	//					m_currDeviceEffects[0] = KBAnimationEffectId;
	//					break;
	//				}
	//				case 1: {		// Mouse
	//					g_ChromaSDKImpl.CreateEffectGroup(&MouseAnimationEffectId, repeatAnimation);

	//					for (int fInx = 0; fInx < maxFrame[1]; fInx++) {
	//						effectID = GUID_NULL;
	//						g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[fInx], &effectID);
	//						g_ChromaSDKImpl.AddToGroup(MouseAnimationEffectId, effectID, g_effectSpeed);
	//					}
	//					m_currDeviceEffects[1] = MouseAnimationEffectId;
	//					break;
	//				}
	//				case 2: {		// Mousepad
	//					g_ChromaSDKImpl.CreateEffectGroup(&MPAnimationEffectId, repeatAnimation);

	//					for (int fInx = 0; fInx < maxFrame[2]; fInx++) {
	//						effectID = GUID_NULL;
	//						g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[fInx], &effectID);
	//						g_ChromaSDKImpl.AddToGroup(MPAnimationEffectId, effectID, g_effectSpeed);
	//					}
	//					m_currDeviceEffects[2] = MPAnimationEffectId;
	//					break;
	//				}
	//				case 3: {		// Headset
	//					g_ChromaSDKImpl.CreateEffectGroup(&HSAnimationEffectId, repeatAnimation);

	//					for (int fInx = 0; fInx < maxFrame[3]; fInx++) {
	//						effectID = GUID_NULL;
	//						g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[fInx], &effectID);
	//						g_ChromaSDKImpl.AddToGroup(HSAnimationEffectId, effectID, g_effectSpeed);
	//					}
	//					m_currDeviceEffects[3] = HSAnimationEffectId;
	//					break;
	//				}
	//				case 4: {		// Keypad
	//					g_ChromaSDKImpl.CreateEffectGroup(&KPAnimationEffectId, repeatAnimation);

	//					for (int fInx = 0; fInx < maxFrame[4]; fInx++) {
	//						effectID = GUID_NULL;
	//						g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[fInx], &effectID);
	//						g_ChromaSDKImpl.AddToGroup(KPAnimationEffectId, effectID, g_effectSpeed);
	//					}
	//					m_currDeviceEffects[4] = KPAnimationEffectId;
	//					break;
	//				}
	//				case 5: {		// Chromalink
	//					g_ChromaSDKImpl.CreateEffectGroup(&CLAnimationEffectId, repeatAnimation);

	//					for (int fInx = 0; fInx < maxFrame[5]; fInx++) {
	//						effectID = GUID_NULL;
	//						g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[fInx], &effectID);
	//						g_ChromaSDKImpl.AddToGroup(CLAnimationEffectId, effectID, g_effectSpeed);
	//					}
	//					m_currDeviceEffects[5] = CLAnimationEffectId;
	//					break;
	//				}
	//			}

	//		}
	//	}

	//	for (int effectIndx = 0; effectIndx < MAXDEVICES; effectIndx++) {
	//		if (m_currDeviceEffects[effectIndx] != GUID_NULL) {
	//			g_ChromaSDKImpl.SetEffectImpl(m_currDeviceEffects[effectIndx]);
	//		}
	//	}
	//}

	//void ChromaSystemComponent::ShowFrame(int frame) {
	//	// Keyboard Device
	//	if (ChromaSystemComponent::chromaDeviceType == 0) {
	//		g_ChromaSDKImpl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY, &keyboardFrames[frame-1], NULL);
	//	}

	//	// Mouse Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//		g_ChromaSDKImpl.CreateMouseEffectImpl(ChromaSDK::Mouse::CHROMA_CUSTOM2, &mouseFrames[frame - 1], NULL);
	//	}

	//	// Mousepad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//		g_ChromaSDKImpl.CreateMousematEffectImpl(ChromaSDK::Mousepad::CHROMA_CUSTOM, &mousepadFrames[frame - 1], NULL);
	//	}

	//	// Headset Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//		g_ChromaSDKImpl.CreateHeadsetEffectImpl(ChromaSDK::Headset::CHROMA_CUSTOM, &headsetFrames[frame - 1], NULL);
	//	}

	//	// Keypad Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//		g_ChromaSDKImpl.CreateKeypadEffectImpl(ChromaSDK::Keypad::CHROMA_CUSTOM, &keypadFrames[frame - 1], NULL);
	//	}

	//	// Chromalink Device
	//	else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//		g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &chromalinkFrames[frame-1], NULL);
	//	}
	//}

	//AZ::Crc32 ChromaSystemComponent::ImportChromaEffect() {
	//	enum EChromaSDKDeviceTypeEnum
	//	{
	//		DE_1D = 0,
	//		DE_2D,
	//	};

	//	enum EChromaSDKDevice1DEnum
	//	{
	//		DE_ChromaLink = 0,
	//		DE_Headset,
	//		DE_Mousepad,
	//	};

	//	enum EChromaSDKDevice2DEnum
	//	{
	//		DE_Keyboard = 0,
	//		DE_Keypad,
	//		DE_Mouse,
	//	};

	//	QString chromaFilePath = QFileDialog::getOpenFileName(nullptr, "Import Chroma Effect", QString(), "Filter Files (*.chroma)");

	//	QFile chromaFile(chromaFilePath);

	//	if (chromaFile.open(QFile::ReadOnly | QFile::Text))
	//	{
	//		if (chromaFile.isTextModeEnabled()) {
	//			QDataStream inSteam(&chromaFile);

	//			// Read the Effect File Animation Version
	//			int version = ANIMATION_VERSION;
	//			inSteam.readRawData(reinterpret_cast<char*>(&version), sizeof(version));
	//			if (version != 1)
	//				CryLog("Imported Invalid Animation Version %d", version);

	//			// Read the Effect File Device Type
	//			qint8 deviceType = 0;
	//			qint8 device = 0;

	//			// Read the Effect File DeviceType and Device
	//			inSteam.readRawData(reinterpret_cast<char*>(&deviceType), sizeof(deviceType));
	//			inSteam.readRawData(reinterpret_cast<char*>(&device), sizeof(device));

	//			if (deviceType == (qint8)DE_1D) {
	//				if (device == EChromaSDKDevice1DEnum::DE_Mousepad) {
	//					ChromaSystemComponent::chromaDeviceType = 2;
	//				}
	//				else if (device == EChromaSDKDevice1DEnum::DE_Headset) {
	//					ChromaSystemComponent::chromaDeviceType = 3;
	//				}
	//				else if (device == EChromaSDKDevice1DEnum::DE_ChromaLink) {
	//					ChromaSystemComponent::chromaDeviceType = 5;
	//				}
	//			}
	//			else {
	//				if (device == EChromaSDKDevice2DEnum::DE_Keyboard) {
	//					ChromaSystemComponent::chromaDeviceType = 0;
	//				}
	//				else if (device == EChromaSDKDevice2DEnum::DE_Mouse) {
	//					ChromaSystemComponent::chromaDeviceType = 1;
	//				}
	//				else if (device == EChromaSDKDevice2DEnum::DE_Keypad) {
	//					ChromaSystemComponent::chromaDeviceType = 4;
	//				}
	//			}

	//			// Read the Effect File FrameCount
	//			UINT frameCount = 0;
	//			inSteam.readRawData(reinterpret_cast<char*>(&frameCount), sizeof(frameCount));
	//			maxFrame[ChromaSystemComponent::chromaDeviceType] = frameCount;

	//			// Read the Effect File Duration
	//			float duration = 0.0;

	//			// Read the Effect Colors
	//			int color = 0;
	//			for (int f = 0; f < maxFrame[ChromaSystemComponent::chromaDeviceType]; f++) {

	//				inSteam.readRawData(reinterpret_cast<char*>(&duration), sizeof(duration));
	//				g_effectSpeed = duration * 1000.0f;

	//				if (ChromaSystemComponent::chromaDeviceType == 0) {
	//					for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
	//					{
	//						for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
	//						{
	//							inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));
	//							
	//							if (color != 0)
	//								CryLog("Import Effect: Row = %d | Col = %d | Color = %d", r, c, color);

	//							keyboardFrames[f].Color[r][c] = color;
	//						}
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//					for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
	//					{
	//						for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
	//						{
	//							inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

	//							if (color != 0)
	//								CryLog("Import Effect: Row = %d | Col = %d | Color = %d", r, c, color);

	//							mouseFrames[f].Color[r][c] = color;
	//						}
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//					for (UINT c = 0; c < ChromaSDK::Mousepad::MAX_LEDS; c++)
	//					{
	//						inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

	//						if (color != 0)
	//							CryLog("Import Effect: LED = %d | Color = %d", c, color);

	//						mousepadFrames[f].Color[c] = color;
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//					for (UINT c = 0; c < ChromaSDK::Headset::MAX_LEDS; c++)
	//					{
	//						inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

	//						if (color != 0)
	//							CryLog("Import Effect: LED = %d | Color = %d", c, color);

	//						headsetFrames[f].Color[c] = color;
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//					for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
	//					{
	//						for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
	//						{
	//							inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

	//							if (color != 0)
	//								CryLog("Import Effect: Row = %d | Col = %d | Color = %d", r, c, color);

	//							keypadFrames[f].Color[r][c] = color;
	//						}
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//					for (UINT c = 0; c < ChromaSDK::ChromaLink::MAX_LEDS; c++)
	//					{
	//						inSteam.readRawData(reinterpret_cast<char*>(&color), sizeof(color));

	//						if (color != 0)
	//							CryLog("Import Effect: LED = %d | Color = %d", c, color);

	//						chromalinkFrames[f].Color[c] = color;
	//					}
	//				}
	//			}
	//		}
	//	}

	//	chromaFile.close();
	//	m_currFrameNum = 1;
	//	newFrame = maxFrame[ChromaSystemComponent::chromaDeviceType];
	//	ShowFrame(m_currFrameNum);

	//	return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	//}

	//void ChromaSystemComponent::ExportChromaEffect() {
	//	enum EChromaSDKDeviceTypeEnum
	//	{
	//		DE_1D = 0,
	//		DE_2D,
	//	};

	//	enum EChromaSDKDevice1DEnum
	//	{
	//		DE_ChromaLink = 0,
	//		DE_Headset,
	//		DE_Mousepad,
	//	};

	//	enum EChromaSDKDevice2DEnum
	//	{
	//		DE_Keyboard = 0,
	//		DE_Keypad,
	//		DE_Mouse,
	//	};

	//	QString chromaFilePath = QFileDialog::getSaveFileName(nullptr, "Save Chroma Effect", QString(), "Filter Files (*.chroma)");
	//	//QString chromaFilePath = QDir::cleanPath(QDir::currentPath() + QDir::separator() + "ChromaEffectFiles/lumberyardTest3.chroma");

	//	QFile chromaFile(chromaFilePath);

	//	if (chromaFile.open(QFile::WriteOnly | QFile::Text))
	//	{
	//		if (chromaFile.isTextModeEnabled()) {
	//			//chromaFile.write("Hello Chroma File!");

	//			QDataStream outStream(&chromaFile);

	//			// Write the Effect File Animation Version
	//			int version = ANIMATION_VERSION;
	//			outStream.writeRawData(reinterpret_cast<const char*>(&version), sizeof(version));

	//			// Write the Effect File Device Type
	//			qint8 deviceType = 0;
	//			qint8 device = 0;
	//			if (ChromaSystemComponent::chromaDeviceType == 0) {
	//				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_2D;
	//				device = (qint8)EChromaSDKDevice2DEnum::DE_Keyboard;
	//			}
	//			else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_2D;
	//				device = (qint8)EChromaSDKDevice2DEnum::DE_Mouse;
	//			}
	//			else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_2D;
	//				device = (qint8)EChromaSDKDevice2DEnum::DE_Keypad;
	//			}
	//			else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_1D;
	//				device = (qint8)EChromaSDKDevice1DEnum::DE_Mousepad;
	//			}
	//			else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_1D;
	//				device = (qint8)EChromaSDKDevice1DEnum::DE_Headset;
	//			}
	//			else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//				deviceType = (qint8)EChromaSDKDeviceTypeEnum::DE_1D;
	//				device = (qint8)EChromaSDKDevice1DEnum::DE_ChromaLink;
	//			}

	//			// Write the Effect File DeviceType and Device
	//			//outStream << deviceType << device;
	//			outStream.writeRawData(reinterpret_cast<const char*>(&deviceType), sizeof(deviceType));
	//			outStream.writeRawData(reinterpret_cast<const char*>(&device), sizeof(device));

	//			// Write the Effect File FrameCount
	//			UINT frameCount = maxFrame[ChromaSystemComponent::chromaDeviceType];
	//			outStream.writeRawData(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));

	//			// Write the Effect File Duration
	//			// Compute the duration as the (frameCount-1)*frameDelay
	//			float duration = (float)(g_effectSpeed / 1000.0f);

	//			// Write the Effect Colors
	//			int color = 0;
	//			for (int f = 0; f < maxFrame[ChromaSystemComponent::chromaDeviceType]; f++) {

	//				outStream.writeRawData(reinterpret_cast<const char*>(&duration), sizeof(duration));

	//				if (ChromaSystemComponent::chromaDeviceType == 0) {
	//					for (UINT r = 0; r < ChromaSDK::Keyboard::MAX_ROW; r++)
	//					{
	//						for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
	//						{
	//							color = keyboardFrames[f].Color[r][c];
	//							if (color != 0)
	//								CryLog("Export Effect: Row = %d | Col = %d | Color = %d", r, c, color);
	//							outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
	//						}
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 1) {
	//					for (UINT r = 0; r < ChromaSDK::Mouse::MAX_ROW; r++)
	//					{
	//						for (UINT c = 0; c < ChromaSDK::Mouse::MAX_COLUMN; c++)
	//						{
	//							color = mouseFrames[f].Color[r][c];
	//							if (color != 0)
	//								CryLog("Export Effect: Row = %d | Col = %d | Color = %d", r, c, color);
	//							outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
	//						}
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 2) {
	//					for (UINT c = 0; c < ChromaSDK::Mousepad::MAX_LEDS; c++)
	//					{
	//						color = mousepadFrames[f].Color[c];
	//						if (color != 0)
	//							CryLog("Export Effect: LED = %d | Color = %d", c, color);
	//						outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 3) {
	//					for (UINT c = 0; c < ChromaSDK::Headset::MAX_LEDS; c++)
	//					{
	//						color = headsetFrames[f].Color[c];
	//						if (color != 0)
	//							CryLog("Export Effect: LED = %d | Color = %d", c, color);
	//						outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 4) {
	//					for (UINT r = 0; r < ChromaSDK::Keypad::MAX_ROW; r++)
	//					{
	//						for (UINT c = 0; c < ChromaSDK::Keypad::MAX_COLUMN; c++)
	//						{
	//							color = keypadFrames[f].Color[r][c];
	//							if (color != 0)
	//								CryLog("Export Effect: Row = %d | Col = %d | Color = %d", r, c, color);
	//							outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
	//						}
	//					}
	//				}
	//				else if (ChromaSystemComponent::chromaDeviceType == 5) {
	//					for (UINT c = 0; c < ChromaSDK::ChromaLink::MAX_LEDS; c++)
	//					{
	//						color = chromalinkFrames[f].Color[c];
	//						if (color != 0)
	//							CryLog("Export Effect: LED = %d | Color = %d", c, color);
	//						outStream.writeRawData(reinterpret_cast<const char*>(&color), sizeof(color));
	//					}
	//				}
	//			}
	//		}
	//	}

	//	chromaFile.close();
	//}

}

