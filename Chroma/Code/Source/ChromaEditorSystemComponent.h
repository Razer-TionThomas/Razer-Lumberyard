#pragma once

#include <AzCore/Component/Component.h>
//#include <Chroma/ChromaBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Color.h>
#include <CrySystemBus.h> // For calendar demo

#include "ChromaSDKImpl.h"

#define MAXFRAMES 200
#define MAXDEVICES 6

namespace Chroma
{

	// Extracted from macro REGISTER_GAME_OBJECT();

    class ChromaEditorSystemComponent
        : public AZ::Component
       // , protected ChromaRequestBus::Handler
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

		ChromaEditorSystemComponent();
		~ChromaEditorSystemComponent() override;

        AZ_COMPONENT(ChromaEditorSystemComponent, "{241E753E-0F02-4165-B212-C02B86F38E0B}");

        static void Reflect(AZ::ReflectContext* context);

        //static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        //static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        //static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        //static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		// CrySystemEventBus
		void OnCrySystemInitialized(ISystem&, const SSystemInitParams&) override; // For calendar demo

		// TickBus
		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
		////////////////////////////////////////////////////////////////////////

		// Global Brightness Control
		float g_effectBrightness;

		int g_effectSpeed;

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

		// Component button variables
		bool clearEffects;
		bool stopEffect;
		bool playPresetEffect;
		bool playCustomEffect;
		bool importImage;
		bool importAnimation;
		bool setKey;
		bool setLed;
		bool nextFrame;
		bool prevFrame;
		bool setRow;
		bool setCol;
		bool clearFrame;
		bool addFrame;
		bool deleteFrame;
		bool copyFrame;
		bool pasteFrame;
		bool fillFrame;
		bool resetAnimation;
		bool playAllCustomEffect;
		bool exportEffect;
		bool exportHeader;
		bool importEffect;

		// Repeat Checkbox Variable
		bool repeatAnimation;

		// Entity Inspector Functions
		void SetEffectBrightness();
		void SetEffectSpeed();
		void ClearEffects();
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
		AZ::Crc32 SetChromaDeviceType();
		void SetEffectColor();
		void LoadSingleImage();
		AZ::Crc32 LoadAnimation();
		bool ReadImageFile();
		AZ::Crc32 ReadGifAnimationFile();
		void StopEffect();
		void PlayPresetEffect();
		void SetKey();
		void SetLed();
		AZ::Crc32 JumpToFrame();
		AZ::Crc32 NextFrame();
		AZ::Crc32 PreviousFrame();
		void PaintRow();
		void PaintCol();
		void ShowFrame(int frame);
		void PlayCustomAnimation();
		void ClearFrame();
		AZ::Crc32 ResetAnimation();
		AZ::Crc32 AddFrame();
		AZ::Crc32 DeleteFrame();
		void CopyFrame();
		void PasteFrame();
		void FillFrame();
		void PlayAllCustomAnimation();
		void ExportChromaEffect();
		void ExportChromaHeaderFile();
		AZ::Crc32 ImportChromaEffect();

    };
}
