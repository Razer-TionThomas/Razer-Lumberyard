ChromaTrigger = {
	Properties =
	{
		greetingMessage = "Lua Greeting Property",
		chromaBrightness = 
		{
			default  = 1,
			min = 0,
			max = 10,
			description = "Chroma brightness value",
		},
		chromaFramecount = 
		{
			default  = 1,
			min = 0,
			max = 100,
			description = "Chroma frame count",
		},
		chromaFrameDelay = 
		{
			default  = 0,
			min = 0,
			max = 1000,
			description = "Chroma frame delay in ms",
		}
	}
}

function ChromaTrigger:OnActivate() 
	Debug.Log("Hello from Chroma script!");
	Debug.Log(self.Properties.greetingMessage);
	self.tickBusHandler = TickBus.CreateHandler(self);
	--self.tickBusHandler:Connect();
	
	ChromaRequestBus.Broadcast.ChromaSDKInit();
end

function ChromaTrigger:OnDeactivate() 
	Debug.Log("Chroma script deactivated!");
	self.tickBusHandler:Disconnect()
end

function ChromaTrigger:OnTick(dt)
	Debug.Log("Got a Tick!" .. dt);
end
return ChromaTrigger;