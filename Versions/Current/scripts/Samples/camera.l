---- запускать на map 2400 и взять кем-то кроме fritz - а документы

who = nil;
--
function Trigger3()
	local num;
	num, who = HasInventoryItem( 178 );
	return ( who ~= nil ) and ( num ~= nil );
end
--
function OnTrigger3()
	fritz = GetUnit("fritz");
	CameraMove( GetCamera( 6 ), 3000 );
	UnitSetPose( fritz, POSE_WALK );
	UnitGoToWaypoint( fritz, "wp1" )
	WaitForInterface();
	WaitForUnit( fritz );
	CameraMove( GetCamera( 7 ), 2000 );
	UnitGoToWaypoint( fritz, "wp2" )
	WaitForInterface();
	CameraMove( GetCamera( 12 ), 8000 );
	WaitForUnit( fritz );
	UnitSetShootMode( fritz, SM_LONGBURST );
	UnitShoot( fritz, who, HL_HEAD );
end
--
Trigger( Trigger3, OnTrigger3 );
