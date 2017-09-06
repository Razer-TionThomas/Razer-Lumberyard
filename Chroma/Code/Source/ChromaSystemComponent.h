#pragma once

#include <AzCore/Component/Component.h>
#include <Chroma/ChromaBus.h>
#include <AzCore/Component/TickBus.h>
#include "C:\Amazon\Lumberyard\1.9.0.1\dev\Code\CryEngine\CryAction\IGameObject.h"
#include <CrySystemBus.h> // For calendar demo

#include "ChromaSDKImpl.h"

namespace Chroma
{

	// Extracted from macro REGISTER_GAME_OBJECT();

    class ChromaSystemComponent
        : public AZ::Component
        , protected ChromaRequestBus::Handler
		, public AZ::TickBus::Handler
		, protected CrySystemEventBus::Handler // For calendar demo
		//, public CGameObjectExtensionHelper < ChromaSystemComponent, IGameObjectExtension >
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
			STATIC = 0,
			FLASH,
			WAVE,
			BREATHING,
			CUSTOM,
			RANDOM
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
		void PlayChromaEffect() override;
		void StopChromaEffect() override;
        ////////////////////////////////////////////////////////////////////////

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

		AZ::u32 g_effectSpeed;

		// Component Color Picker Variable
		AZ::Color ChromaColor;

		// Select Device Variable
		AZ::u32 chromaDeviceType;

		// Select Effect Variable
		AZ::u32 chromaEffectValue;

		// Store CurrentEffectId
		RZEFFECTID m_currEffect;

		// Helper Functions
		COLORREF GetCOLORREFValue(AZ::Color, double brightness);

		//CString m_currFileName;

		// Component button variables
		bool clearEffects;
		bool stopEffect;
		bool playEffect;
		bool importImage;
		bool importAnimation;
		//bool staticEffect;
		//bool flashEffect;
		//bool waveEffect;
		//bool breathingEffect;
		//bool randomEffect;
		//bool customEffect;

		// Repeat Checkbox Variable
		bool repeatAnimation;

		// Entity Inspector Functions
		void SetEffectBrightness();
		void SetEffectSpeed();
		void ClearEffects();
		void ShowFlashEffect();
		void ShowCustomEffect();
		void ShowRandomEffect();
		void ShowWaveEffect();
		void ShowBreathingEffect();
		void ShowStaticEffect();
		void SetChromaDeviceType(AZ::u32 deviceType);
		void SetEffectColor(AZ::Color color);
		void LoadSingleImage();
		void LoadAnimation();
		void ReadImageFile();
		void StopEffect();
		void PlayEffect();

    };

	//struct CChromaCreator
	//	: public IGameObjectExtensionCreatorBase
	//{
	//	IGameObjectExtensionPtr Create()
	//	{
	//		return gEnv->pEntitySystem->CreateComponent<ChromaSystemComponent>();
	//	}

	//	void GetGameObjectExtensionRMIData(void** ppRMI, size_t* nCount)
	//	{
	//		ChromaSystemComponent::GetGameObjectExtensionRMIData(ppRMI, nCount);
	//	}
	//};
}
