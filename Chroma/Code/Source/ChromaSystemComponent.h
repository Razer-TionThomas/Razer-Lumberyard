#pragma once

#include <AzCore/Component/Component.h>
#include <Chroma/ChromaBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Color.h>

#include "ChromaSDKImpl.h"

#define MAXFRAMES 200
#define MAXDEVICES 6

namespace Chroma
{

	// Extracted from macro REGISTER_GAME_OBJECT();

    class ChromaSystemComponent
        : public AZ::Component
        , protected ChromaRequestBus::Handler
		, public AZ::TickBus::Handler
		, protected CrySystemEventBus::Handler // For calendar demo
    {
    public:
		enum ChromDeviceType
		{
			KEYBOARD = 0,
			MOUSE,
			MOUSEPAD,
			HEADSET,
			KEYPAD,
			CHROMALINK
		};

		enum ChromaEffectValue
		{
			RANDOM = 0,
			FLASH,
			WAVELR1,
			WATERFALL,
			WAVECROSS,
			BREATHING,
			RIPPLELR,
			RIPPLEUD,
			EXPLOSION,
			RAINBOW
		};

		enum ChromaLEDs
		{
			LED0 = 0,
			LED1,
			LED2,
			LED3,
			LED4,
			LED5,
			LED6,
			LED7,
			LED8,
			LED9,
			LED10,
			LED11,
			LED12,
			LED13,
			LED14,
			LED15,
			LED16,
			LED17,
			LED18,
			LED19,
			LED20,
			LED21,
			LED22,
			LED23,
			LED24,
			LED25
		};

		ChromaSystemComponent();
		~ChromaSystemComponent() override;

        AZ_COMPONENT(ChromaSystemComponent, "{0C733541-0DFC-42FB-BE5E-AAAC7F9616F4}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // ChromaRequestBus interface implementation
		void ChromaSDKInit() override;
		void ChromaSDKUnInit() override;
		void PlayPresetChromaEffect(int effectId, int device, bool repeat ) override;
		void StopChromaEffect() override;
		void ClearChromaEffects() override;
		void SetEffectBrightness(float brightness) override;
		void SetEffectSpeed(int speed) override;
		void SetEffectColor(float r, float g, float b, float a) override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////

		// TickBus
		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
		////////////////////////////////////////////////////////////////////////

		// Global Brightness Control
		float g_effectBrightness;

		int g_effectSpeed;

		// Repeat Checkbox Variable
		bool repeatAnimation;

		// Component Color Picker Variable
		AZ::Color ChromaColor;

		// Select Device Variable
		AZ::u32 chromaDeviceType;

		// Select Effect Variable
		AZ::u32 chromaEffectValue;

		// Select Keyboard Key
		AZ::u32 ChromaKey;

		// Select (Non-keybaord) Led
		AZ::u32 ChromaLed;

		// Jump to Frame
		AZ::u32 newFrame = 1;

		// Set Max Frame (<MAXFRAMES)
		AZ::u32 maxFrame[MAXDEVICES] = { 1 };

		// Select row to color
		AZ::u32 cRow = 1;

		// Select col to color
		AZ::u32 cCol = 1;

		/*****(************************** Chroma SDK Varaibles   *****************************/
		// Custom Keyboard Grid
		ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE copyKeyboardEffect = {};
		ChromaSDK::Keypad::CUSTOM_EFFECT_TYPE copyKeypadEffect = {};
		ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2 copyMouseEffect = {};
		ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE copyMousepadEffect = {};
		ChromaSDK::Headset::CUSTOM_EFFECT_TYPE copyHeadsetEffect = {};
		ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE copyChromalinkEffect = {};
		
		// Store CurrentEffectId
		RZEFFECTID m_currEffect;

		// Store GroupEffectId
		RZEFFECTID m_currDeviceEffects[MAXDEVICES];

		// Store Current Frame Number
		int m_currFrameNum;

		// List of Frames for Custom Animations
		ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE keyboardFrames[MAXFRAMES] = {};
		ChromaSDK::Keypad::CUSTOM_EFFECT_TYPE keypadFrames[MAXFRAMES] = {};
		ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE2 mouseFrames[MAXFRAMES] = {};
		ChromaSDK::Mousepad::CUSTOM_EFFECT_TYPE mousepadFrames[MAXFRAMES] = {};
		ChromaSDK::Headset::CUSTOM_EFFECT_TYPE headsetFrames[MAXFRAMES] = {};
		ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE chromalinkFrames[MAXFRAMES] = {};

		// Mouse Custom Effect LEDs
		int chromaMouseLEDs[ChromaSDK::Mouse::MAX_LEDS2] = { ChromaSDK::Mouse::RZLED2::RZLED2_BACKLIGHT, ChromaSDK::Mouse::RZLED2::RZLED2_BOTTOM1, ChromaSDK::Mouse::RZLED2::RZLED2_BOTTOM2,
			ChromaSDK::Mouse::RZLED2::RZLED2_BOTTOM3, ChromaSDK::Mouse::RZLED2::RZLED2_BOTTOM4, ChromaSDK::Mouse::RZLED2::RZLED2_BOTTOM5, ChromaSDK::Mouse::RZLED2::RZLED2_LEFT_SIDE1, 
			ChromaSDK::Mouse::RZLED2::RZLED2_LEFT_SIDE2, ChromaSDK::Mouse::RZLED2::RZLED2_LEFT_SIDE3, ChromaSDK::Mouse::RZLED2::RZLED2_LEFT_SIDE4, ChromaSDK::Mouse::RZLED2::RZLED2_LEFT_SIDE5,
			ChromaSDK::Mouse::RZLED2::RZLED2_LEFT_SIDE6, ChromaSDK::Mouse::RZLED2::RZLED2_LEFT_SIDE7, ChromaSDK::Mouse::RZLED2::RZLED2_LOGO, ChromaSDK::Mouse::RZLED2::RZLED2_RIGHT_SIDE1,
			ChromaSDK::Mouse::RZLED2::RZLED2_RIGHT_SIDE2, ChromaSDK::Mouse::RZLED2::RZLED2_RIGHT_SIDE3, ChromaSDK::Mouse::RZLED2::RZLED2_RIGHT_SIDE4, ChromaSDK::Mouse::RZLED2::RZLED2_RIGHT_SIDE5,
			ChromaSDK::Mouse::RZLED2::RZLED2_RIGHT_SIDE6, ChromaSDK::Mouse::RZLED2::RZLED2_RIGHT_SIDE7, ChromaSDK::Mouse::RZLED2::RZLED2_SCROLLWHEEL
		};


		/*************************************************************************************/

		// Helper Functions
		COLORREF GetCOLORREFValue(AZ::Color, double brightness);

		ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE GetBlackKBFrame() {
			// Define Clear Frame
			RZEFFECTID clearKB = GUID_NULL;

			ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE KBClearFrame = {};

			for (UINT c = 0; c < ChromaSDK::Keyboard::MAX_COLUMN; c++)
			{
				for (int r = ChromaSDK::Keyboard::MAX_ROW - 1; r >= 0; r--)
				{
					KBClearFrame.Color[r][c] = RGB(0, 0, 0);
				}
			}

			return KBClearFrame;
		}

		//CString m_currFileName;

		void StopEffect();
		void ClearDeviceEffects();

		void ShowRandomEffect();
		void ShowFlashEffect();
		void ShowWaveLR1Effect();
		void ShowWaterFallEffect();
		void ShowWaveCross1Effect();
		void ShowBreathingEffect();
		void ShowRippleLREffect();
		void ShowRippleUDEffect();
		void ShowExplosionEffect();
		void ShowRainbowEffect();
    };
}
