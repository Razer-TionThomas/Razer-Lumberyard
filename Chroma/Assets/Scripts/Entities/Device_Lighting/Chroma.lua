----------------------------------------------------------------------------------------------------
--
-- All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
-- its licensors.
--
-- For complete copyright and license terms please see the LICENSE at the root of this
-- distribution (the "License"). All use of this software is governed by the License,
-- or, if provided, by the license below or the license accompanying this file. Do not
-- remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--
--
----------------------------------------------------------------------------------------------------
Chroma = 
{
	type = "Chroma",
	Properties = 
	{
		bEnabled = 1, --[0,1,1,"Set this to true/false to turn the Chroma effect on/off."]
		bIgnoreVisareas = 0, --[0,1,1,"Continue to render Chroma when player is inside a visarea."]
		bDisableOcclusion = 0, --[0,1,1,"Don't check if object should be occluded from Chroma (is under cover)."]
		fRadius = 10000.0, --[0,10000,1,"Set the radius, or coverage area, of the Chroma effect."]
		fAmount = 1.0, --[0,100,0.1,"Set the overall amount of the Chroma entity's various effects."]
		fDiffuseDarkening = 0.5, --[0,1,0.1,"Set the amount of darkening due to wetness that is applied to the surfaces in the world."]
		fChromaDropsAmount = 0.5, --[0,100,0.1,"Set the amount of Chroma drops that can be seen in the air."]
		fChromaDropsSpeed = 1.0, --[0,100,0.1,"Set the speed at which the Chroma drops travel."]
		fChromaDropsLighting = 1.0, --[0,100,0.1,"Set the brightness or backlighting of the Chroma drops."]
	},
	Editor =
	{
		Icon="shake.bmp",
		Category="Device Lighting",
	},
}

function Chroma:OnInit()
	self:OnReset();
end

function Chroma:OnPropertyChange()
	self:OnReset();
end

function Chroma:OnReset()
end

function Chroma:OnSave(tbl)
	tbl.bEnabled = self.Properties.bEnabled;
end

function Chroma:OnLoad(tbl)
	self.Properties.bEnabled = tbl.bEnabled;
end

function Chroma:OnShutDown()
end


----------------------
-- Flow Nodes Start
----------------------

function Chroma:Event_Enable( sender )
	self.Properties.bEnabled = 1;
	self:ActivateOutput("Enable", true);
end

function Chroma:Event_Disable( sender )
	self.Properties.bEnabled = 0;
	self:ActivateOutput("Disable", true);
end

function Chroma:Event_Amount( sender, amount )
	self.Properties.fAmount = amount;
end

function Chroma:Event_Radius( sender, radius )
	self.Properties.fRadius = radius;
end

Chroma.FlowEvents =
{
	Inputs =
	{
		Disable = { Chroma.Event_Disable, "bool" },
		Enable = { Chroma.Event_Enable, "bool" },
		Amount = { Chroma.Event_Amount, "float" },
		Radius = { Chroma.Event_Radius, "float" },
	},
	Outputs =
	{
		Disable = "bool",
		Enable = "bool",
	},
}
