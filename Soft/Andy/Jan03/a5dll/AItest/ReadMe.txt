-world as a binding center
 -AIworld provide
  raytrace
  pathfind
  [danger calcs]
 -AI for units - where? in the world?!
 -outer connection = object positions/properties/unit info/ commands interface
 -can view AImap for outer clients
 -construction from template / deploy party
 -world simulation
-conversion scene->set of hedras
 -AIworld (former AImap)
  -sync world with AIworld
  -set of hulls with properties + pathing info [+ coverage info]
  -every entity in world can have representation in AIworld
   -use same object/unit interface or some another of internal kind?
   -where to store this representation - maybe separately from main model?
  -units should move and change poses, but when shooting pose should be correct
  -volume structure (most probably octree)
 -AIworld visualization, 
  -parameterization (show hull info/path info/paths/whatever)
  -special object & embedding of it into interface mission
-determine quality of cover with monte carlo method
-collision detection
 -for moving sphere
