/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#ifndef EDITOR_COMPONENT_BASE_H
#define EDITOR_COMPONENT_BASE_H

#include <AzCore/base.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Math/Crc.h>
#include <AzCore/Math/transform.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/Entity.h>

namespace AZ
{
    class Vector2;
}

namespace AzToolsFramework
{
    namespace Components
    {
        class SelectionComponent;

        // Editor Component Base serves as a base class for all World Editor components.
        // it provides useful functionality to your own custom components if you derive from it.
        class EditorComponentBase
            : public AZ::Component
        {
        public:
            AZ_RTTI(EditorComponentBase, "{D5346BD4-7F20-444E-B370-327ACD03D4A0}", AZ::Component)
            // Think about AZ_EDITOR_COMPONENT

            EditorComponentBase();

            //////////////////////////////////////////////////////////////////////////
            // Editor Component API - use these to do useful things in the editor

            // SetDirty - call this whenever you alter something in an unexpected manner - for example
            // if your entity could get its properties modified by a DIFFERENT entity during an edit of that other entity.
            // This sets the dirty flag on the ENTIRE entity.
            // Editing an entity's property in the property editor automatically sets it dirty, so its not needed for that
            // situation.  Its only when your properties were modified from the outside, like a script which loops over
            // all lights and alters their radii, for example.
            void SetDirty();

            //////////////////////////////////////////////////////////////////////////
            // AZ::Component
            virtual void Init() override;
            virtual void Activate() override;
            virtual void Deactivate() override;
            //////////////////////////////////////////////////////////////////////////

            // Unless you GetRequiredServices for "TransformService" in your component, THIS MAY BE NULL!
            AZ::TransformInterface* GetTransform() const;

            // Unless you GetRequiredServices for "SelectionService" in your component, THIS MAY BE NULL!
            SelectionComponent* GetSelection() const;

            //Safe Transform Ease of Use
            //returns Identity if not
            AZ::Transform GetWorldTM() const;
            AZ::Transform GetLocalTM() const;

            //Safe Selected Ease of Use
            bool IsSelected() const;
            bool IsPrimarySelection() const;

            void UnregisterIcon();
            bool HasSelectionComponent() const { return m_selection != nullptr; }

            /** Called by Prefab Builder. Implement if you need to generate one or more game components
                to represent your editor component in the runtime. Any components you generate should be
                attached to the provided game Entity. If you don't need to generate a game component, then
                you don't need to bother overriding this function.
                */
            virtual void BuildGameEntity(AZ::Entity* /*gameEntity*/) {}
            /** Called by Prefab Builder. Called after game entity has been saved out, right
            *   before deleting it. If you are retaining ownership of a component you handed
            *   to the game entity in BuildGameEntity, then remove the component from the
            *   game entity here.
            */
            virtual void FinishedBuildingGameEntity(AZ::Entity* /*gameEntity*/) {}
            /** Implement to support dragging and dropping of an asset onto your editor component.
            */
            virtual void SetPrimaryAsset(const AZ::Data::AssetId& /*assetId*/) { }

            static void Reflect(AZ::ReflectContext* context);

        private:
            AZ::TransformInterface* m_transform;
            SelectionComponent* m_selection;
        };
    }
} // namespace AzToolsFramework

#endif
