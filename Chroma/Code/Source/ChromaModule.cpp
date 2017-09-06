
#include "StdAfx.h"
#include <platform_impl.h>
#include <AzCore/Component/ComponentApplicationBus.h>

#if CHROMA_EDITOR
#include "ChromaSystemComponent.h"
#endif

#include <IGem.h>
//#include <FlowSystem/Nodes/FlowBaseNode.h>

namespace Chroma
{
    class ChromaModule
        : public CryHooksModule
    {
    public:


        AZ_RTTI(ChromaModule, "{562255E1-BFDA-41A9-B78A-CBE9CC80C321}", CryHooksModule);

        ChromaModule()
            : CryHooksModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
#if CHROMA_EDITOR
                ChromaSystemComponent::CreateDescriptor(),
#endif
            });

			//EBUS_EVENT(AZ::ComponentApplicationBus, RegisterComponentDescriptor, ChromaSystemComponent::CreateDescriptor());
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
#if CHROMA_EDITOR
                azrtti_typeid<ChromaSystemComponent>(),
#endif
            };
        }

		void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override {
			//switch (event)
			//{
			//case ESYSTEM_EVENT_FLOW_SYSTEM_REGISTER_EXTERNAL_NODES:
			////	RegisterExternalFlowNodes();
			//	break;

			//case ESYSTEM_EVENT_GAME_POST_INIT:
			//	IComponentFactoryRegistry::RegisterAllComponentFactoryNodes(*gEnv->pEntitySystem->GetComponentFactoryRegistry());
			//	PostGameInit();
			//	break;

			//case ESYSTEM_EVENT_FULL_SHUTDOWN:
			//case ESYSTEM_EVENT_FAST_SHUTDOWN:
			//	// Put your shutdown code here
			//	// Other Gems may have been shutdown already, but none will have destructed
			//	break;
			//}
		}

		/*void PostGameInit() {
			IEntityClassRegistry::SEntityClassDesc clsDesc;
			clsDesc.sName = "Chroma";
			clsDesc.sScriptFile = "Scripts/Entities/Device_Lighting/Chroma.lua";
			static CChromaCreator _creator;
			GetISystem()->GetIGame()->GetIGameFramework()->GetIGameObjectSystem()->RegisterExtension("Chroma", &_creator, &clsDesc);
		}*/
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(Chroma_a33e409ebf5c44b7b6ae0609936f5af8, Chroma::ChromaModule)
