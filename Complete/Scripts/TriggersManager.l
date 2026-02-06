triggers = {};
maxTriggerIndex = 0;
--
function Trigger( trigger, handler, cycled, latency )
	maxTriggerIndex = maxTriggerIndex + 1;
	triggers[maxTriggerIndex] = {}
	triggers[maxTriggerIndex].trigger = trigger;
	triggers[maxTriggerIndex].handler = handler;
	if latency == nil then
		triggers[maxTriggerIndex].latency = 20;
	else
		triggers[maxTriggerIndex].latency = latency;
	end
	triggers[maxTriggerIndex].curLatency = 0;
	triggers[maxTriggerIndex].cycled = cycled;
end
--
function TriggersManager()
	out( "Triggers manager was started" );
	while 1 
	do
		for index, handler in triggers
		do
			if ( index ~= nil ) and ( handler ~= nil ) then
				if handler.curLatency >= handler.latency then
					if handler.trigger() ~= nil then 
						StartThread( handler.handler );
						if handler.cycled == nil then
							triggers[index] = nil;
							break;
						end	
					end
					if triggers[index] ~= nil then
						handler.curLatency = 0;
					end
				end
				handler.curLatency = handler.curLatency + 10;
			end
		end
		Sleep(10);
	end
end
--
TriggersManager();
--