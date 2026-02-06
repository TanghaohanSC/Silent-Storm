who = nil;
--
function Trigger3()
	local num;
	num, who = HasInventoryItem( 178 );
	return ( who ~= nil ) and ( num ~= nil );
end
--
function OnTrigger3()

BeginSequence();

	fritz = GetUnit("fritz");
	CameraMove( GetCamera( 6 ), 3000 );
	UnitSetPose( fritz, POSE_WALK );
	UnitMoveToWaypoint( fritz, "wp1" )
	WaitForInterface();
	WaitForUnit( fritz );

EndSequencePart();

	CameraMove( GetCamera( 7 ), 2000 );
	UnitMoveToWaypoint( fritz, "wp2" )
	WaitForInterface();
	CameraMove( GetCamera( 12 ), 8000 );
	WaitForUnit( fritz );

EndSequencePart();

	UnitSetShootMode( fritz, SM_LONGBURST );
	UnitShoot( fritz, who, HL_HEAD );
	WaitForUnit( fritz );
	WaitForInterface();

EndSequence();

end
--
Trigger( Trigger3, OnTrigger3 );
