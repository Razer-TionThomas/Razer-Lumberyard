
#include "StdAfx.h"
#include <platform_impl.h>
#include <AzCore/Component/ComponentApplicationBus.h>

#include "ChromaSystemComponent.h"

#if defined (CHROMA_EDITOR)
#include "ChromaEditorSystemComponent.h"
#endif

#include <IGem.h>

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

				Chroma::ChromaSystemComponent::CreateDescriptor(),

#if defined(CHROMA_EDITOR)
                Chroma::ChromaEditorSystemComponent::CreateDescriptor(),
#endif
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
//#ifndef CHROMA_EDITOR
				azrtti_typeid<ChromaSystemComponent>(),
//#endif

#if defined(CHROMA_EDITOR)
                azrtti_typeid<ChromaEditorSystemComponent>(),
#endif
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(Chroma_a33e409ebf5c44b7b6ae0609936f5af8, Chroma::ChromaModule)
