for i = 0, 2 do
	SetUnitDiplomacy( GetUnit( "fritz" ), 2, i )
	state = GetUnitDiplomacy( GetUnit( "fritz" ), 2 )
	out( " state = ", state )
	SetDiplomacy( 0, 2, 0 );
	state = GetUnitDiplomacy( GetUnit( "fritz" ), 2 )
	out( " after player diplomacy changed: state = ", state )
end