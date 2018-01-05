
#pragma once

#include <AzCore/EBus/EBus.h>

namespace Chroma
{
    class ChromaRequests
        : public AZ::EBusTraits
    {

    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        // Public functions

		virtual void ChromaSDKInit() = 0;
		virtual void ChromaSDKUnInit() = 0;
		virtual void PlayPresetChromaEffect(int, int, bool) = 0;
		virtual void StopChromaEffect() = 0;
		virtual void ClearChromaEffects() = 0;
		virtual void SetEffectBrightness(float) = 0;
		virtual void SetEffectSpeed(int) = 0;
		virtual void SetEffectColor(float, float, float, float) = 0;
    };
    using ChromaRequestBus = AZ::EBus<ChromaRequests>;
} // namespace Chroma
