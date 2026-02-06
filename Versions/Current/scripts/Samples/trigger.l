function Trigger1()
	return 1;
end
--
function Trigger2()
	return 1;
end
--
function OnTrigger1()
	out("Trigger 1");
	Trigger( Trigger2, OnTrigger2 );
end
--
function OnTrigger2()
	out("Trigger 2");
	Trigger( Trigger1, OnTrigger1 );
end
--
function Trigger3()
	local num, who = HasInventoryItem( 178 )
	return num;
end
--
function OnTrigger3()
	out("Trigger 3");
end
--
-- добавление триггера
-- Trigger( условие, действие, цикличность, проверять условие каждые _ segment-ов )
-- два последних параметра по умолчанию имеют значения nil и 20

Trigger( Trigger1, OnTrigger1 );
Trigger( Trigger3, OnTrigger3, 1, 100 );

