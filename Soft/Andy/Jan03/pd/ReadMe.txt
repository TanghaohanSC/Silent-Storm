3D
 DOFs for 3D joints
 
realistic friction model whatever it is

keep energy, try - x = x + v*t + 0.5*a*t^2

different collision planes
 several simultaneous contacts - can be serialized but then have to check speed direction at each contact point

different masses support

restitution

stability
 what to do with gravity in stable state
 discrete speed per DOF
 freeze on relaxed state
 measure effects of round errors in speed calcs
 less precise calcs in relaxation (prolly with epsilons on distance)

use body model
 self imposed constraints? - for constrained joints
 add strings to satisfy some restrictions
