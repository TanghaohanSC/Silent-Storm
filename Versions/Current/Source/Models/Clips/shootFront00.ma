//Maya ASCII 4.0 scene
//Name: shootFront00+.ma
//Last modified: Thu, Aug 29, 2002 12:07:28 PM
requires maya "4.0";
currentUnit -l centimeter -a degree -t ntsc;
createNode animClip -n "clip00Source";
	setAttr ".ihi" 0;
	setAttr ".du" 30;
	setAttr ".ci" no;
createNode animCurveTU -n "animCurveTU2230";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 1;
createNode animCurveTU -n "animCurveTU2231";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 1;
createNode animCurveTU -n "animCurveTU2232";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 1;
createNode animCurveTA -n "animCurveTA4346";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTA -n "animCurveTA4347";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTA -n "animCurveTA4348";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTL -n "animCurveTL4531";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTL -n "animCurveTL4532";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTL -n "animCurveTL4533";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTU -n "animCurveTU2233";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 4;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4349";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 1.2148014608834885;
	setAttr ".roti" 3;
createNode animCurveTA -n "animCurveTA4350";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 39.976592627826598;
	setAttr ".roti" 3;
createNode animCurveTA -n "animCurveTA4351";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 -3.9741223038096645;
	setAttr ".roti" 3;
createNode animCurveTL -n "animCurveTL4534";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.15718318740000001;
createNode animCurveTL -n "animCurveTL4535";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.1303855689;
createNode animCurveTL -n "animCurveTL4536";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.29401398909999998;
createNode animCurveTU -n "animCurveTU2234";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2235";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 4 3 4 30 4;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  5 3;
createNode animCurveTA -n "animCurveTA4352";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 -191.64564632981188 3 -179.59495029602377 
		30 -191.64564632981188;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTA -n "animCurveTA4353";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 -83.828209591919901 3 -83.603127492830509 
		30 -83.828209591919901;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTA -n "animCurveTA4354";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 -134.86101973364256 3 -146.84909142082705 
		30 -134.86101973364256;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTL -n "animCurveTL4537";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 0.013001829297552735 3 0.013001829297552735 
		30 0.013001829297552735;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTL -n "animCurveTL4538";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 0.18869766874714028 3 0.18869766874714028 
		30 0.18869766874714028;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTL -n "animCurveTL4539";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 -0.098961299571577524 3 -0.098961299571577524 
		30 -0.098961299571577524;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTU -n "animCurveTU2236";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 1 3 1 30 1;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  5 3;
createNode animCurveTU -n "animCurveTU2237";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 4;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4355";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 -40.411949019025116;
	setAttr ".roti" 3;
createNode animCurveTA -n "animCurveTA4356";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 -9.162415207370314;
	setAttr ".roti" 3;
createNode animCurveTA -n "animCurveTA4357";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 -37.63740736792812;
	setAttr ".roti" 3;
createNode animCurveTL -n "animCurveTL4540";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.4842349223785965;
createNode animCurveTL -n "animCurveTL4541";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.13228204759661807;
createNode animCurveTL -n "animCurveTL4542";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.18901856058043154;
createNode animCurveTU -n "animCurveTU2238";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2239";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 4;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4358";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 86.927330774274452;
createNode animCurveTA -n "animCurveTA4359";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 95.123223768045193;
createNode animCurveTA -n "animCurveTA4360";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 65.715212155585348;
createNode animCurveTL -n "animCurveTL4543";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.16272077418891739;
createNode animCurveTL -n "animCurveTL4544";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.14348121563773164;
createNode animCurveTL -n "animCurveTL4545";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.2961818785828581;
createNode animCurveTU -n "animCurveTU2240";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2241";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTA -n "animCurveTA4361";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -187.37772883281767;
createNode animCurveTA -n "animCurveTA4362";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 13.107973467750552;
createNode animCurveTA -n "animCurveTA4363";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -83.22250821359961;
createNode animCurveTL -n "animCurveTL4546";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.026678853391171344;
createNode animCurveTL -n "animCurveTL4547";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.010301323265660876;
createNode animCurveTL -n "animCurveTL4548";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.066739955283447644;
createNode animCurveTU -n "animCurveTU2242";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2243";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 4;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4364";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -13.868305021746231;
createNode animCurveTA -n "animCurveTA4365";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 82.875722586661226;
createNode animCurveTA -n "animCurveTA4366";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 101.65462181112589;
createNode animCurveTL -n "animCurveTL4549";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.094791345736591426;
createNode animCurveTL -n "animCurveTL4550";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.14845093250493735;
createNode animCurveTL -n "animCurveTL4551";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.13388933838305556;
createNode animCurveTU -n "animCurveTU2244";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2245";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 4;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4367";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.88648456031517453;
createNode animCurveTA -n "animCurveTA4368";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 97.002893006434391;
createNode animCurveTA -n "animCurveTA4369";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 70.687596651132566;
createNode animCurveTL -n "animCurveTL4552";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.094445367888397191;
createNode animCurveTL -n "animCurveTL4553";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.14789849796524912;
createNode animCurveTL -n "animCurveTL4554";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.12265632193298315;
createNode animCurveTU -n "animCurveTU2246";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2247";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4370";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 58.290652951333072;
createNode animCurveTA -n "animCurveTA4371";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 89.202644430010395;
createNode animCurveTA -n "animCurveTA4372";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 179.99999999999955;
createNode animCurveTL -n "animCurveTL4555";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.092078740514516033;
createNode animCurveTL -n "animCurveTL4556";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.14921226921143962;
createNode animCurveTL -n "animCurveTL4557";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.10927208749922893;
createNode animCurveTU -n "animCurveTU2248";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2249";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4373";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -90.867964797971439;
createNode animCurveTA -n "animCurveTA4374";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 90.616666388412767;
createNode animCurveTA -n "animCurveTA4375";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.83317549291585991;
createNode animCurveTL -n "animCurveTL4558";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.092059659888157383;
createNode animCurveTL -n "animCurveTL4559";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.17229569662952482;
createNode animCurveTL -n "animCurveTL4560";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -5.8295394463275137e-006;
createNode animCurveTU -n "animCurveTU2250";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2251";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4376";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -55.043592991908788;
createNode animCurveTA -n "animCurveTA4377";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 91.143535461851954;
createNode animCurveTA -n "animCurveTA4378";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 4.7109481410833448e-013;
createNode animCurveTL -n "animCurveTL4561";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.092078740514516033;
createNode animCurveTL -n "animCurveTL4562";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.14300260778747587;
createNode animCurveTL -n "animCurveTL4563";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.10565773088471279;
createNode animCurveTU -n "animCurveTU2252";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2253";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4379";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -32.944179631883848;
createNode animCurveTA -n "animCurveTA4380";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 95.324607963296828;
createNode animCurveTA -n "animCurveTA4381";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -211.65447381277886;
createNode animCurveTL -n "animCurveTL4564";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.086445046368000655;
createNode animCurveTL -n "animCurveTL4565";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.020305714530646057;
createNode animCurveTL -n "animCurveTL4566";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.16972326218266887;
createNode animCurveTU -n "animCurveTU2254";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2255";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4382";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 145.28430637455284;
createNode animCurveTA -n "animCurveTA4383";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 84.323169791981073;
createNode animCurveTA -n "animCurveTA4384";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 41.122861976945927;
createNode animCurveTL -n "animCurveTL4567";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.083590430119444337;
createNode animCurveTL -n "animCurveTL4568";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.091448079884016986;
createNode animCurveTL -n "animCurveTL4569";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.085576106421556733;
createNode animCurveTU -n "animCurveTU2256";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2257";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4385";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 32.610219777792729;
createNode animCurveTA -n "animCurveTA4386";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 85.440596802632385;
createNode animCurveTA -n "animCurveTA4387";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -39.273843515614992;
createNode animCurveTL -n "animCurveTL4570";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.083590430119444337;
createNode animCurveTL -n "animCurveTL4571";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.097071155479687726;
createNode animCurveTL -n "animCurveTL4572";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.073509714388873229;
createNode animCurveTU -n "animCurveTU2258";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU2259";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4388";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -4.9309846811920348;
createNode animCurveTA -n "animCurveTA4389";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 83.793639433990165;
createNode animCurveTA -n "animCurveTA4390";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -5.6229098223144662;
createNode animCurveTL -n "animCurveTL4573";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.090690922975573018;
createNode animCurveTL -n "animCurveTL4574";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.022682940295604748;
createNode animCurveTL -n "animCurveTL4575";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.17220369839616484;
createNode animCurveTU -n "animCurveTU2260";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA4391";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0 10 0 20 0 30 0;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4392";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0 10 0 20 0 30 0;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4393";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 138.07241099803983 10 138.07241099803983 
		20 138.07241099803983 30 138.07241099803983;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4394";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0 10 0 20 0 30 0;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4395";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0 10 0 20 0 30 0;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4576";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0 10 0 20 0 30 0;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4577";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.1564763852228141 10 -0.1564763852228141 
		20 -0.1564763852228141 30 -0.1564763852228141;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4578";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0.024526950963566954 10 0.024526950963566954 
		20 0.024526950963566954 30 0.024526950963566954;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4396";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 186.1564629807956 10 186.1564629807956 
		20 186.1564629807956 30 186.1564629807956;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4397";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0.89039438243229985 10 0.89039438243229985 
		20 0.89039438243229985 30 0.89039438243229985;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4398";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.13243663211320258 10 -0.13243663211320258 
		20 -0.13243663211320258 30 -0.13243663211320258;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4579";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0 10 0 20 0 30 0;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4580";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.072922062300977988 10 -0.072922062300977988 
		20 -0.072922062300977988 30 -0.072922062300977988;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4581";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.54799912886044588 10 -0.54799912886044588 
		20 -0.54799912886044588 30 -0.54799912886044588;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4582";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.035208445782383135 10 -0.035208445782383135 
		20 -0.035208445782383135 30 -0.035208445782383135;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4583";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0.56809068832002041 10 0.56809068832002041 
		20 0.56809068832002041 30 0.56809068832002041;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4584";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0.21026542838152262 10 0.21026542838152262 
		20 0.21026542838152262 30 0.21026542838152262;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4585";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.071241268614438913 10 -0.071241268614438913 
		20 -0.071241268614438913 30 -0.071241268614438913;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4586";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0.69087603666090092 10 0.69087603666090092 
		20 0.69087603666090092 30 0.69087603666090092;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4587";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.50179476372180642 10 -0.50179476372180642 
		20 -0.50179476372180642 30 -0.50179476372180642;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTU -n "animCurveTU2261";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 5 1 5 7 5 18 5 30 5;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTU -n "animCurveTU2262";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 2.1000000000000005 1 2.1000000000000005 
		7 2.1000000000000005 18 2.1000000000000005 30 2.1000000000000005;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTU -n "animCurveTU2263";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 1.0000000000000004 1 1.0000000000000004 
		7 1.0000000000000004 18 1.0000000000000004 30 1.0000000000000004;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTA -n "animCurveTA4399";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 167.7459723563517 1 164.15191959403694 
		7 165.45219268536601 18 168.08469924562996 30 167.7459723563517;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTA -n "animCurveTA4400";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 8.5644041599690599 1 32.974236649013882 
		7 17.720468218331739 18 5.2498634169721967 30 8.5644041599690599;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTA -n "animCurveTA4401";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -84.846021685778751 1 -87.035586619511577 
		7 -86.080799362973437 18 -84.252305834875273 30 -84.846021685778751;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4588";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 0.12012081896377635 1 0.14099350086560364 
		7 0.11658924557080955 18 0.097857632288469443 30 0.12012081896377635;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4589";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.065754286266072021 1 -0.065754286266072021 
		7 -0.065754286266072021 18 -0.065754286266072021 30 -0.065754286266072021;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4590";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 1.0447238210537579 1 1.0240811610457938 
		7 1.0242082394595151 18 1.0496706741061295 30 1.0447238210537579;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTU -n "animCurveTU2264";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 3.0000000000000004 8 3.0000000000000004 
		20 3.0000000000000004 30 3.0000000000000004;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTU -n "animCurveTU2265";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 3 8 3 20 3 30 3;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTU -n "animCurveTU2266";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.40000000000000013 8 -0.40000000000000013 
		20 -0.40000000000000013 30 -0.40000000000000013;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4402";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 85.182617738558719 8 85.182617738558719 
		20 85.182617738558719 30 85.182617738558719;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4403";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 79.764196180615713 8 79.764196180615713 
		20 79.764196180615713 30 79.764196180615713;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4404";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -42.495655665165934 8 -42.495655665165934 
		20 -42.495655665165934 30 -42.495655665165934;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4591";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -0.51580421496219919 8 -0.51580421496219919 
		20 -0.51580421496219919 30 -0.51580421496219919;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4592";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 0.13407559595011884 8 0.13407559595011884 
		20 0.13407559595011884 30 0.13407559595011884;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4593";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -1.2848573668101222 8 -1.2980051630957419 
		20 -1.2848573668101222 30 -1.2848573668101222;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4594";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.1103263293457013 5 -0.1103263293457013 
		10 -0.1103263293457013 20 -0.1103263293457013 30 -0.1103263293457013;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4595";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.63967128199006673 5 -0.63967128199006673 
		10 -0.63967128199006673 20 -0.63967128199006673 30 -0.63967128199006673;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4596";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 0.56339076283608791 5 0.55024296655046823 
		10 0.56339076283608791 20 0.56339076283608791 30 0.56339076283608791;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4597";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.29208931702808894 5 -0.29208931702808894 
		10 -0.29208931702808894 20 -0.29208931702808894 30 -0.29208931702808894;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4598";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.55360482081221174 5 -0.55360482081221174 
		10 -0.55360482081221174 20 -0.55360482081221174 30 -0.55360482081221174;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4599";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -1.1736044726230188 5 -1.1867522689086385 
		10 -1.1736044726230188 20 -1.1736044726230188 30 -1.1736044726230188;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTA -n "animCurveTA4405";
	setAttr ".tan" 10;
	setAttr -s 3 ".ktv[0:2]"  0 -3.3081705118823845 3 -3.3811627976598588 
		30 -3.3081705118823845;
	setAttr -s 3 ".kit[0:2]"  3 10 10;
	setAttr -s 3 ".kot[0:2]"  3 10 10;
createNode animCurveTA -n "animCurveTA4406";
	setAttr ".tan" 10;
	setAttr -s 3 ".ktv[0:2]"  0 17.164257302875196 3 23.678254842262103 
		30 17.164257302875196;
	setAttr -s 3 ".kit[0:2]"  3 10 10;
	setAttr -s 3 ".kot[0:2]"  3 10 10;
createNode animCurveTA -n "animCurveTA4407";
	setAttr ".tan" 10;
	setAttr -s 3 ".ktv[0:2]"  0 -0.58922276163762599 3 -0.6147324191182113 
		30 -0.58922276163762599;
	setAttr -s 3 ".kit[0:2]"  3 10 10;
	setAttr -s 3 ".kot[0:2]"  3 10 10;
createNode animCurveTA -n "animCurveTA4408";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 14.461181477117313 10 14.461181477117313 
		20 14.461181477117313 30 14.461181477117313;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4409";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 16.154447159053387 10 16.154447159053387 
		20 16.154447159053387 30 16.154447159053387;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTA -n "animCurveTA4410";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 -1.7383256329285628 10 -1.7383256329285628 
		20 -1.7383256329285628 30 -1.7383256329285628;
	setAttr -s 4 ".kit[0:3]"  3 10 10 10;
	setAttr -s 4 ".kot[0:3]"  3 10 10 10;
createNode animCurveTL -n "animCurveTL4600";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.015630409904486386 5 -0.015630409904486386 
		10 -0.015630409904486386 20 -0.015630409904486386 30 -0.015630409904486386;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4601";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.045844752295812037 5 -0.045844752295812037 
		10 -0.045844752295812037 20 -0.045844752295812037 30 -0.045844752295812037;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4602";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.26150770087782038 5 -0.27465549716344007 
		10 -0.26150770087782038 20 -0.26150770087782038 30 -0.26150770087782038;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTU -n "animCurveTU2267";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 -0.10000000000000142 5 -0.10000000000000142 
		10 -0.10000000000000142 20 -0.10000000000000142 30 -0.10000000000000142;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTU -n "animCurveTU2268";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 18 5 18 10 18 20 18 30 18;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4603";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 0.021846630578410187 5 0.021846630578410187 
		10 0.021846630578410187 20 0.021846630578410187 30 0.021846630578410187;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4604";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 0.59785634101176632 5 0.59785634101176632 
		10 0.59785634101176632 20 0.59785634101176632 30 0.59785634101176632;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTL -n "animCurveTL4605";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 0.16219632065703182 5 0.14904852437141214 
		10 0.16219632065703182 20 0.16219632065703182 30 0.16219632065703182;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 10;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 10;
createNode animCurveTA -n "animCurveTA4411";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 162.63420930771025 3 159.23729043801319 
		20 162.63420930771025 30 162.63420930771025;
	setAttr -s 4 ".kit[0:3]"  3 9 10 10;
	setAttr -s 4 ".kot[0:3]"  3 9 10 10;
createNode animCurveTA -n "animCurveTA4412";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 4.6487018156421476 3 5.5457317140511924 
		20 4.6487018156421476 30 4.6487018156421476;
	setAttr -s 4 ".kit[0:3]"  3 9 10 10;
	setAttr -s 4 ".kot[0:3]"  3 9 10 10;
createNode animCurveTA -n "animCurveTA4413";
	setAttr ".tan" 10;
	setAttr -s 4 ".ktv[0:3]"  0 1.8785667504778567 3 0.36342821325135161 
		20 1.8785667504778567 30 1.8785667504778567;
	setAttr -s 4 ".kit[0:3]"  3 9 10 10;
	setAttr -s 4 ".kot[0:3]"  3 9 10 10;
createNode animCurveTA -n "animCurveTA4414";
	setAttr ".tan" 10;
	setAttr -s 6 ".ktv[0:5]"  0 166.50689642387184 2 166.50689642387184 
		5 166.06113207628781 12 166.50689642387184 20 166.50689642387184 30 166.50689642387184;
	setAttr -s 6 ".kit[0:5]"  3 10 10 10 10 10;
	setAttr -s 6 ".kot[0:5]"  3 10 10 10 10 10;
createNode animCurveTA -n "animCurveTA4415";
	setAttr ".tan" 10;
	setAttr -s 6 ".ktv[0:5]"  0 2.2605619219581552 2 2.2605619219581552 
		5 4.2253592404604658 12 2.2605619219581552 20 2.2605619219581552 30 2.2605619219581552;
	setAttr -s 6 ".kit[0:5]"  3 10 10 10 10 10;
	setAttr -s 6 ".kot[0:5]"  3 10 10 10 10 10;
createNode animCurveTA -n "animCurveTA4416";
	setAttr ".tan" 10;
	setAttr -s 6 ".ktv[0:5]"  0 1.7775940858618917 2 1.7775940858618917 
		5 1.0888521342876012 12 1.7775940858618917 20 1.7775940858618917 30 1.7775940858618917;
	setAttr -s 6 ".kit[0:5]"  3 10 10 10 10 10;
	setAttr -s 6 ".kot[0:5]"  3 10 10 10 10 10;
createNode animCurveTA -n "animCurveTA4417";
	setAttr ".tan" 10;
	setAttr -s 6 ".ktv[0:5]"  0 166.73099976524983 3 166.73099976524983 
		6 166.36891637525991 15 166.73099976524983 24 166.73099976524983 30 166.73099976524983;
	setAttr -s 6 ".kit[0:5]"  3 10 10 10 10 10;
	setAttr -s 6 ".kot[0:5]"  3 10 10 10 10 10;
createNode animCurveTA -n "animCurveTA4418";
	setAttr ".tan" 10;
	setAttr -s 7 ".ktv[0:6]"  0 -3.3767943037299899 3 -3.3767943037299899 
		6 -2.3897655466328183 7 -2.8061919509669049 15 -3.3767943037299899 24 -3.3767943037299899 
		30 -3.3767943037299899;
	setAttr -s 7 ".kit[0:6]"  3 10 10 10 10 10 10;
	setAttr -s 7 ".kot[0:6]"  3 10 10 10 10 10 10;
createNode animCurveTA -n "animCurveTA4419";
	setAttr ".tan" 10;
	setAttr -s 6 ".ktv[0:5]"  0 -1.53321612253974 3 -1.53321612253974 
		6 -1.9757833914539686 15 -1.53321612253974 24 -1.53321612253974 30 -1.53321612253974;
	setAttr -s 6 ".kit[0:5]"  3 10 10 10 10 10;
	setAttr -s 6 ".kot[0:5]"  3 10 10 10 10 10;
createNode animCurveTL -n "animCurveTL4606";
	setAttr ".tan" 10;
	setAttr -s 6 ".ktv[0:5]"  0 -0.013240957131743336 3 -0.013240957131743336 
		6 -0.013240957131743336 15 -0.013240957131743336 24 -0.013240957131743336 
		30 -0.013240957131743336;
	setAttr -s 6 ".kit[0:5]"  3 10 10 10 10 10;
	setAttr -s 6 ".kot[0:5]"  3 10 10 10 10 10;
createNode animCurveTL -n "animCurveTL4607";
	setAttr ".tan" 10;
	setAttr -s 6 ".ktv[0:5]"  0 -0.050973859148174275 3 -0.050973859148174275 
		6 -0.050973859148174275 15 -0.050973859148174275 24 -0.050973859148174275 
		30 -0.050973859148174275;
	setAttr -s 6 ".kit[0:5]"  3 10 10 10 10 10;
	setAttr -s 6 ".kot[0:5]"  3 10 10 10 10 10;
createNode animCurveTL -n "animCurveTL4608";
	setAttr ".tan" 10;
	setAttr -s 7 ".ktv[0:6]"  0 -0.30453122591313464 3 -0.30708774185756071 
		6 -0.30778409558608183 9 -0.30810786668929058 17 -0.30299557177067282 25 
		-0.30423044650118136 30 -0.30453122591313464;
	setAttr -s 7 ".kit[0:6]"  3 9 10 10 10 10 10;
	setAttr -s 7 ".kot[0:6]"  3 9 10 10 10 10 10;
createNode clipLibrary -n "clipLibrary1";
	setAttr -s 191 ".cel[0].cev";
	setAttr ".cd[0].cm" -type "characterMapping" 207 "SkeletonGroup.scaleZ" 
		0 1 "SkeletonGroup.scaleY" 0 2 "SkeletonGroup.scaleX" 
		0 3 "SkeletonGroup.rotateZ" 2 1 "SkeletonGroup.rotateY" 
		2 2 "SkeletonGroup.rotateX" 2 3 "SkeletonGroup.translateZ" 
		1 1 "SkeletonGroup.translateY" 1 2 "SkeletonGroup.translateX" 
		1 3 "SubMachineGun.parent" 0 4 "SubMachineGun.rotateZ" 
		2 4 "SubMachineGun.rotateY" 2 5 "SubMachineGun.rotateX" 
		2 6 "SubMachineGun.translateZ" 1 4 "SubMachineGun.translateY" 
		1 5 "SubMachineGun.translateX" 1 6 "SubMachineGun.visibility" 
		0 5 "RocketLauncher.parent" 0 6 "RocketLauncher.rotateZ" 
		2 7 "RocketLauncher.rotateY" 2 8 "RocketLauncher.rotateX" 
		2 9 "RocketLauncher.translateZ" 1 7 "RocketLauncher.translateY" 
		1 8 "RocketLauncher.translateX" 1 9 "RocketLauncher.visibility" 
		0 7 "Rifle.parent" 0 8 "Rifle.rotateZ" 2 10 "Rifle.rotateY" 
		2 11 "Rifle.rotateX" 2 12 "Rifle.translateZ" 1 10 "Rifle.translateY" 
		1 11 "Rifle.translateX" 1 12 "Rifle.visibility" 0 
		9 "MachineGun.parent" 0 10 "MachineGun.rotateZ" 2 13 "MachineGun.rotateY" 
		2 14 "MachineGun.rotateX" 2 15 "MachineGun.translateZ" 
		1 13 "MachineGun.translateY" 1 14 "MachineGun.translateX" 
		1 15 "MachineGun.visibility" 0 11 "Item.parent" 0 
		12 "Item.rotateZ" 2 16 "Item.rotateY" 2 17 "Item.rotateX" 
		2 18 "Item.translateZ" 1 16 "Item.translateY" 1 17 "Item.translateX" 
		1 18 "Item.visibility" 0 13 "Slot1.parent" 0 14 "Slot1.rotateZ" 
		2 19 "Slot1.rotateY" 2 20 "Slot1.rotateX" 2 21 "Slot1.translateZ" 
		1 19 "Slot1.translateY" 1 20 "Slot1.translateX" 1 
		21 "Slot1.visibility" 0 15 "Slot2.parent" 0 16 "Slot2.rotateZ" 
		2 22 "Slot2.rotateY" 2 23 "Slot2.rotateX" 2 24 "Slot2.translateZ" 
		1 22 "Slot2.translateY" 1 23 "Slot2.translateX" 1 
		24 "Slot2.visibility" 0 17 "Slot3.parent" 0 18 "Slot3.rotateZ" 
		2 25 "Slot3.rotateY" 2 26 "Slot3.rotateX" 2 27 "Slot3.translateZ" 
		1 25 "Slot3.translateY" 1 26 "Slot3.translateX" 1 
		27 "Slot3.visibility" 0 19 "Slot4.parent" 0 20 "Slot4.rotateZ" 
		2 28 "Slot4.rotateY" 2 29 "Slot4.rotateX" 2 30 "Slot4.translateZ" 
		1 28 "Slot4.translateY" 1 29 "Slot4.translateX" 1 
		30 "Slot4.visibility" 0 21 "Slot5.parent" 0 22 "Slot5.rotateZ" 
		2 31 "Slot5.rotateY" 2 32 "Slot5.rotateX" 2 33 "Slot5.translateZ" 
		1 31 "Slot5.translateY" 1 32 "Slot5.translateX" 1 
		33 "Slot5.visibility" 0 23 "Slot6.parent" 0 24 "Slot6.rotateZ" 
		2 34 "Slot6.rotateY" 2 35 "Slot6.rotateX" 2 36 "Slot6.translateZ" 
		1 34 "Slot6.translateY" 1 35 "Slot6.translateX" 1 
		36 "Slot6.visibility" 0 25 "Slot7.parent" 0 26 "Slot7.rotateZ" 
		2 37 "Slot7.rotateY" 2 38 "Slot7.rotateX" 2 39 "Slot7.translateZ" 
		1 37 "Slot7.translateY" 1 38 "Slot7.translateX" 1 
		39 "Slot7.visibility" 0 27 "Slot8.parent" 0 28 "Slot8.rotateZ" 
		2 40 "Slot8.rotateY" 2 41 "Slot8.rotateX" 2 42 "Slot8.translateZ" 
		1 40 "Slot8.translateY" 1 41 "Slot8.translateX" 1 
		42 "Slot8.visibility" 0 29 "Slot9.parent" 0 30 "Slot9.rotateZ" 
		2 43 "Slot9.rotateY" 2 44 "Slot9.rotateX" 2 45 "Slot9.translateZ" 
		1 43 "Slot9.translateY" 1 44 "Slot9.translateX" 1 
		45 "Slot9.visibility" 0 31 "Corpse.parent" 0 32 "Corpse.rotateZ" 
		2 46 "Corpse.rotateY" 2 47 "Corpse.rotateX" 2 48 "Corpse.translateZ" 
		1 46 "Corpse.translateY" 1 47 "Corpse.translateX" 1 
		48 "Corpse.visibility" 0 33 "BackPack.parent" 0 34 "BackPack.rotateZ" 
		2 49 "BackPack.rotateY" 2 50 "BackPack.rotateX" 2 
		51 "BackPack.translateZ" 1 49 "BackPack.translateY" 1 50 "BackPack.translateX" 
		1 51 "BackPack.visibility" 0 35 "R_ToeLocator.rotateX" 
		2 55 "L_ToeLocator.rotateX" 2 56 "R_FootLocator.rotateZ" 
		2 57 "R_FootLocator.rotateY" 2 58 "R_FootLocator.rotateX" 
		2 59 "R_FootLocator.translateZ" 1 55 "R_FootLocator.translateY" 
		1 56 "R_FootLocator.translateX" 1 57 "L_FootLocator.rotateZ" 
		2 60 "L_FootLocator.rotateY" 2 61 "L_FootLocator.rotateX" 
		2 62 "L_FootLocator.translateZ" 1 58 "L_FootLocator.translateY" 
		1 59 "L_FootLocator.translateX" 1 60 "R_KneeLocator.translateZ" 
		1 61 "R_KneeLocator.translateY" 1 62 "R_KneeLocator.translateX" 
		1 63 "L_KneeLocator.translateZ" 1 64 "L_KneeLocator.translateY" 
		1 65 "L_KneeLocator.translateX" 1 66 "R_HandLocator.palm" 
		0 38 "R_HandLocator.point" 0 39 "R_HandLocator.thumb" 0 
		40 "R_HandLocator.rotateZ" 2 63 "R_HandLocator.rotateY" 2 
		64 "R_HandLocator.rotateX" 2 65 "R_HandLocator.translateZ" 1 
		67 "R_HandLocator.translateY" 1 68 "R_HandLocator.translateX" 
		1 69 "L_HandLocator.palm" 0 41 "L_HandLocator.point" 0 
		42 "L_HandLocator.thumb" 0 43 "L_HandLocator.rotateZ" 2 
		66 "L_HandLocator.rotateY" 2 67 "L_HandLocator.rotateX" 2 
		68 "L_HandLocator.translateZ" 1 70 "L_HandLocator.translateY" 
		1 71 "L_HandLocator.translateX" 1 72 "R_ElbowLocator.translateZ" 
		1 73 "R_ElbowLocator.translateY" 1 74 "R_ElbowLocator.translateX" 
		1 75 "L_ElbowLocator.translateZ" 1 76 "L_ElbowLocator.translateY" 
		1 77 "L_ElbowLocator.translateX" 1 78 "R_ShoulderLocator.rotateZ" 
		2 69 "R_ShoulderLocator.rotateY" 2 70 "R_ShoulderLocator.rotateX" 
		2 71 "L_ShoulderLocator.rotateZ" 2 72 "L_ShoulderLocator.rotateY" 
		2 73 "L_ShoulderLocator.rotateX" 2 74 "HeadPoleLocator.translateZ" 
		1 79 "HeadPoleLocator.translateY" 1 80 "HeadPoleLocator.translateX" 
		1 81 "HeadLocator.NeckRotateX" 0 44 "HeadLocator.NeckRotateY" 
		0 45 "HeadLocator.translateZ" 1 82 "HeadLocator.translateY" 
		1 83 "HeadLocator.translateX" 1 84 "ChestLocator.rotateZ" 
		2 75 "ChestLocator.rotateY" 2 76 "ChestLocator.rotateX" 
		2 77 "SpineLocator.rotateZ" 2 78 "SpineLocator.rotateY" 
		2 79 "SpineLocator.rotateX" 2 80 "HipLocator.rotateZ" 2 
		81 "HipLocator.rotateY" 2 82 "HipLocator.rotateX" 2 83 "HipLocator.translateZ" 
		1 85 "HipLocator.translateY" 1 86 "HipLocator.translateX" 
		1 87  ;
	setAttr ".cd[0].cim" -type "Int32Array" 207 0 1 2 3
		 4 5 6 7 8 9 10 11 12 13 14
		 15 16 17 18 19 20 21 22 23 24 25
		 26 27 28 29 30 31 32 33 34 35 36
		 37 38 39 40 41 42 43 44 45 46 47
		 48 49 50 51 52 53 54 55 56 57 58
		 59 60 61 62 63 64 65 66 67 68 69
		 70 71 72 73 74 75 76 77 78 79 80
		 81 82 83 84 85 86 87 88 89 90 91
		 92 93 94 95 96 97 98 99 100 101 102
		 103 104 105 106 107 108 109 110 111 112 113
		 114 115 116 117 118 119 120 121 122 123 124
		 125 126 127 128 129 130 131 132 133 134 135
		 136 145 146 147 148 149 150 151 152 153 154
		 155 156 157 158 159 160 161 162 163 164 165
		 166 167 168 169 170 171 172 173 174 175 176
		 177 178 179 180 181 182 183 184 185 186 187
		 188 189 190 191 192 193 194 195 196 197 198
		 199 200 201 202 203 204 205 206 207 208 209
		 210 211 212 213 214 ;
createNode lightLinker -n "MG_lightLinker1";
createNode lightLinker -n "PS_lightLinker1";
	setAttr ".ihi" 0;
	setAttr -s 10 ".lnk";
createNode lightLinker -n "RF_lightLinker1";
	setAttr ".ihi" 0;
	setAttr -s 22 ".lnk";
createNode lightLinker -n "SM_SMG1_lightLinker1";
createNode lightLinker -n "RL_lightLinker1";
	setAttr ".ihi" 0;
	setAttr -s 5 ".lnk";
createNode lightLinker -n "KN_lightLinker1";
createNode lightLinker -n "lightLinker1";
select -ne :time1;
	setAttr ".o" 25;
select -ne :renderPartition;
	setAttr -s 77 ".st";
select -ne :renderGlobalsList1;
select -ne :defaultShaderList1;
	setAttr -s 77 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :defaultRenderUtilityList1;
	setAttr -s 54 ".u";
select -ne :lightList1;
	setAttr -s 7 ".ln";
select -ne :defaultTextureList1;
	setAttr -s 75 ".tx";
select -ne :initialShadingGroup;
	addAttr -ci true -sn "materialIndex" -ln "materialIndex" -at "long";
	setAttr -s 493 ".dsm";
	setAttr ".ro" yes;
	setAttr -s 10 ".gn";
select -ne :initialParticleSE;
	addAttr -ci true -sn "materialIndex" -ln "materialIndex" -at "long";
	setAttr ".ro" yes;
select -ne :defaultRenderGlobals;
	setAttr ".fs" 1;
	setAttr ".ef" 10;
select -ne :characterPartition;
select -ne :hyperGraphLayout;
	setAttr ".cch" no;
	setAttr ".ihi" 2;
	setAttr ".nds" 0;
	setAttr ".img" -type "string" "";
	setAttr ".ims" 1;
select -ne :ikSystem;
connectAttr "clip00Source.st" "clipLibrary1.st[0]";
connectAttr "clip00Source.du" "clipLibrary1.du[0]";
connectAttr "animCurveTU2230.a" "clipLibrary1.cel[0].cev[0].cevr";
connectAttr "animCurveTU2231.a" "clipLibrary1.cel[0].cev[1].cevr";
connectAttr "animCurveTU2232.a" "clipLibrary1.cel[0].cev[2].cevr";
connectAttr "animCurveTA4346.a" "clipLibrary1.cel[0].cev[3].cevr";
connectAttr "animCurveTA4347.a" "clipLibrary1.cel[0].cev[4].cevr";
connectAttr "animCurveTA4348.a" "clipLibrary1.cel[0].cev[5].cevr";
connectAttr "animCurveTL4531.a" "clipLibrary1.cel[0].cev[6].cevr";
connectAttr "animCurveTL4532.a" "clipLibrary1.cel[0].cev[7].cevr";
connectAttr "animCurveTL4533.a" "clipLibrary1.cel[0].cev[8].cevr";
connectAttr "animCurveTU2233.a" "clipLibrary1.cel[0].cev[9].cevr";
connectAttr "animCurveTA4349.a" "clipLibrary1.cel[0].cev[10].cevr";
connectAttr "animCurveTA4350.a" "clipLibrary1.cel[0].cev[11].cevr";
connectAttr "animCurveTA4351.a" "clipLibrary1.cel[0].cev[12].cevr";
connectAttr "animCurveTL4534.a" "clipLibrary1.cel[0].cev[13].cevr";
connectAttr "animCurveTL4535.a" "clipLibrary1.cel[0].cev[14].cevr";
connectAttr "animCurveTL4536.a" "clipLibrary1.cel[0].cev[15].cevr";
connectAttr "animCurveTU2234.a" "clipLibrary1.cel[0].cev[16].cevr";
connectAttr "animCurveTU2235.a" "clipLibrary1.cel[0].cev[17].cevr";
connectAttr "animCurveTA4352.a" "clipLibrary1.cel[0].cev[18].cevr";
connectAttr "animCurveTA4353.a" "clipLibrary1.cel[0].cev[19].cevr";
connectAttr "animCurveTA4354.a" "clipLibrary1.cel[0].cev[20].cevr";
connectAttr "animCurveTL4537.a" "clipLibrary1.cel[0].cev[21].cevr";
connectAttr "animCurveTL4538.a" "clipLibrary1.cel[0].cev[22].cevr";
connectAttr "animCurveTL4539.a" "clipLibrary1.cel[0].cev[23].cevr";
connectAttr "animCurveTU2236.a" "clipLibrary1.cel[0].cev[24].cevr";
connectAttr "animCurveTU2237.a" "clipLibrary1.cel[0].cev[25].cevr";
connectAttr "animCurveTA4355.a" "clipLibrary1.cel[0].cev[26].cevr";
connectAttr "animCurveTA4356.a" "clipLibrary1.cel[0].cev[27].cevr";
connectAttr "animCurveTA4357.a" "clipLibrary1.cel[0].cev[28].cevr";
connectAttr "animCurveTL4540.a" "clipLibrary1.cel[0].cev[29].cevr";
connectAttr "animCurveTL4541.a" "clipLibrary1.cel[0].cev[30].cevr";
connectAttr "animCurveTL4542.a" "clipLibrary1.cel[0].cev[31].cevr";
connectAttr "animCurveTU2238.a" "clipLibrary1.cel[0].cev[32].cevr";
connectAttr "animCurveTU2239.a" "clipLibrary1.cel[0].cev[33].cevr";
connectAttr "animCurveTA4358.a" "clipLibrary1.cel[0].cev[34].cevr";
connectAttr "animCurveTA4359.a" "clipLibrary1.cel[0].cev[35].cevr";
connectAttr "animCurveTA4360.a" "clipLibrary1.cel[0].cev[36].cevr";
connectAttr "animCurveTL4543.a" "clipLibrary1.cel[0].cev[37].cevr";
connectAttr "animCurveTL4544.a" "clipLibrary1.cel[0].cev[38].cevr";
connectAttr "animCurveTL4545.a" "clipLibrary1.cel[0].cev[39].cevr";
connectAttr "animCurveTU2240.a" "clipLibrary1.cel[0].cev[40].cevr";
connectAttr "animCurveTU2241.a" "clipLibrary1.cel[0].cev[41].cevr";
connectAttr "animCurveTA4361.a" "clipLibrary1.cel[0].cev[42].cevr";
connectAttr "animCurveTA4362.a" "clipLibrary1.cel[0].cev[43].cevr";
connectAttr "animCurveTA4363.a" "clipLibrary1.cel[0].cev[44].cevr";
connectAttr "animCurveTL4546.a" "clipLibrary1.cel[0].cev[45].cevr";
connectAttr "animCurveTL4547.a" "clipLibrary1.cel[0].cev[46].cevr";
connectAttr "animCurveTL4548.a" "clipLibrary1.cel[0].cev[47].cevr";
connectAttr "animCurveTU2242.a" "clipLibrary1.cel[0].cev[48].cevr";
connectAttr "animCurveTU2243.a" "clipLibrary1.cel[0].cev[49].cevr";
connectAttr "animCurveTA4364.a" "clipLibrary1.cel[0].cev[50].cevr";
connectAttr "animCurveTA4365.a" "clipLibrary1.cel[0].cev[51].cevr";
connectAttr "animCurveTA4366.a" "clipLibrary1.cel[0].cev[52].cevr";
connectAttr "animCurveTL4549.a" "clipLibrary1.cel[0].cev[53].cevr";
connectAttr "animCurveTL4550.a" "clipLibrary1.cel[0].cev[54].cevr";
connectAttr "animCurveTL4551.a" "clipLibrary1.cel[0].cev[55].cevr";
connectAttr "animCurveTU2244.a" "clipLibrary1.cel[0].cev[56].cevr";
connectAttr "animCurveTU2245.a" "clipLibrary1.cel[0].cev[57].cevr";
connectAttr "animCurveTA4367.a" "clipLibrary1.cel[0].cev[58].cevr";
connectAttr "animCurveTA4368.a" "clipLibrary1.cel[0].cev[59].cevr";
connectAttr "animCurveTA4369.a" "clipLibrary1.cel[0].cev[60].cevr";
connectAttr "animCurveTL4552.a" "clipLibrary1.cel[0].cev[61].cevr";
connectAttr "animCurveTL4553.a" "clipLibrary1.cel[0].cev[62].cevr";
connectAttr "animCurveTL4554.a" "clipLibrary1.cel[0].cev[63].cevr";
connectAttr "animCurveTU2246.a" "clipLibrary1.cel[0].cev[64].cevr";
connectAttr "animCurveTU2247.a" "clipLibrary1.cel[0].cev[65].cevr";
connectAttr "animCurveTA4370.a" "clipLibrary1.cel[0].cev[66].cevr";
connectAttr "animCurveTA4371.a" "clipLibrary1.cel[0].cev[67].cevr";
connectAttr "animCurveTA4372.a" "clipLibrary1.cel[0].cev[68].cevr";
connectAttr "animCurveTL4555.a" "clipLibrary1.cel[0].cev[69].cevr";
connectAttr "animCurveTL4556.a" "clipLibrary1.cel[0].cev[70].cevr";
connectAttr "animCurveTL4557.a" "clipLibrary1.cel[0].cev[71].cevr";
connectAttr "animCurveTU2248.a" "clipLibrary1.cel[0].cev[72].cevr";
connectAttr "animCurveTU2249.a" "clipLibrary1.cel[0].cev[73].cevr";
connectAttr "animCurveTA4373.a" "clipLibrary1.cel[0].cev[74].cevr";
connectAttr "animCurveTA4374.a" "clipLibrary1.cel[0].cev[75].cevr";
connectAttr "animCurveTA4375.a" "clipLibrary1.cel[0].cev[76].cevr";
connectAttr "animCurveTL4558.a" "clipLibrary1.cel[0].cev[77].cevr";
connectAttr "animCurveTL4559.a" "clipLibrary1.cel[0].cev[78].cevr";
connectAttr "animCurveTL4560.a" "clipLibrary1.cel[0].cev[79].cevr";
connectAttr "animCurveTU2250.a" "clipLibrary1.cel[0].cev[80].cevr";
connectAttr "animCurveTU2251.a" "clipLibrary1.cel[0].cev[81].cevr";
connectAttr "animCurveTA4376.a" "clipLibrary1.cel[0].cev[82].cevr";
connectAttr "animCurveTA4377.a" "clipLibrary1.cel[0].cev[83].cevr";
connectAttr "animCurveTA4378.a" "clipLibrary1.cel[0].cev[84].cevr";
connectAttr "animCurveTL4561.a" "clipLibrary1.cel[0].cev[85].cevr";
connectAttr "animCurveTL4562.a" "clipLibrary1.cel[0].cev[86].cevr";
connectAttr "animCurveTL4563.a" "clipLibrary1.cel[0].cev[87].cevr";
connectAttr "animCurveTU2252.a" "clipLibrary1.cel[0].cev[88].cevr";
connectAttr "animCurveTU2253.a" "clipLibrary1.cel[0].cev[89].cevr";
connectAttr "animCurveTA4379.a" "clipLibrary1.cel[0].cev[90].cevr";
connectAttr "animCurveTA4380.a" "clipLibrary1.cel[0].cev[91].cevr";
connectAttr "animCurveTA4381.a" "clipLibrary1.cel[0].cev[92].cevr";
connectAttr "animCurveTL4564.a" "clipLibrary1.cel[0].cev[93].cevr";
connectAttr "animCurveTL4565.a" "clipLibrary1.cel[0].cev[94].cevr";
connectAttr "animCurveTL4566.a" "clipLibrary1.cel[0].cev[95].cevr";
connectAttr "animCurveTU2254.a" "clipLibrary1.cel[0].cev[96].cevr";
connectAttr "animCurveTU2255.a" "clipLibrary1.cel[0].cev[97].cevr";
connectAttr "animCurveTA4382.a" "clipLibrary1.cel[0].cev[98].cevr";
connectAttr "animCurveTA4383.a" "clipLibrary1.cel[0].cev[99].cevr";
connectAttr "animCurveTA4384.a" "clipLibrary1.cel[0].cev[100].cevr";
connectAttr "animCurveTL4567.a" "clipLibrary1.cel[0].cev[101].cevr";
connectAttr "animCurveTL4568.a" "clipLibrary1.cel[0].cev[102].cevr";
connectAttr "animCurveTL4569.a" "clipLibrary1.cel[0].cev[103].cevr";
connectAttr "animCurveTU2256.a" "clipLibrary1.cel[0].cev[104].cevr";
connectAttr "animCurveTU2257.a" "clipLibrary1.cel[0].cev[105].cevr";
connectAttr "animCurveTA4385.a" "clipLibrary1.cel[0].cev[106].cevr";
connectAttr "animCurveTA4386.a" "clipLibrary1.cel[0].cev[107].cevr";
connectAttr "animCurveTA4387.a" "clipLibrary1.cel[0].cev[108].cevr";
connectAttr "animCurveTL4570.a" "clipLibrary1.cel[0].cev[109].cevr";
connectAttr "animCurveTL4571.a" "clipLibrary1.cel[0].cev[110].cevr";
connectAttr "animCurveTL4572.a" "clipLibrary1.cel[0].cev[111].cevr";
connectAttr "animCurveTU2258.a" "clipLibrary1.cel[0].cev[112].cevr";
connectAttr "animCurveTU2259.a" "clipLibrary1.cel[0].cev[113].cevr";
connectAttr "animCurveTA4388.a" "clipLibrary1.cel[0].cev[114].cevr";
connectAttr "animCurveTA4389.a" "clipLibrary1.cel[0].cev[115].cevr";
connectAttr "animCurveTA4390.a" "clipLibrary1.cel[0].cev[116].cevr";
connectAttr "animCurveTL4573.a" "clipLibrary1.cel[0].cev[117].cevr";
connectAttr "animCurveTL4574.a" "clipLibrary1.cel[0].cev[118].cevr";
connectAttr "animCurveTL4575.a" "clipLibrary1.cel[0].cev[119].cevr";
connectAttr "animCurveTU2260.a" "clipLibrary1.cel[0].cev[120].cevr";
connectAttr "animCurveTA4391.a" "clipLibrary1.cel[0].cev[145].cevr";
connectAttr "animCurveTA4392.a" "clipLibrary1.cel[0].cev[146].cevr";
connectAttr "animCurveTA4393.a" "clipLibrary1.cel[0].cev[147].cevr";
connectAttr "animCurveTA4394.a" "clipLibrary1.cel[0].cev[148].cevr";
connectAttr "animCurveTA4395.a" "clipLibrary1.cel[0].cev[149].cevr";
connectAttr "animCurveTL4576.a" "clipLibrary1.cel[0].cev[150].cevr";
connectAttr "animCurveTL4577.a" "clipLibrary1.cel[0].cev[151].cevr";
connectAttr "animCurveTL4578.a" "clipLibrary1.cel[0].cev[152].cevr";
connectAttr "animCurveTA4396.a" "clipLibrary1.cel[0].cev[153].cevr";
connectAttr "animCurveTA4397.a" "clipLibrary1.cel[0].cev[154].cevr";
connectAttr "animCurveTA4398.a" "clipLibrary1.cel[0].cev[155].cevr";
connectAttr "animCurveTL4579.a" "clipLibrary1.cel[0].cev[156].cevr";
connectAttr "animCurveTL4580.a" "clipLibrary1.cel[0].cev[157].cevr";
connectAttr "animCurveTL4581.a" "clipLibrary1.cel[0].cev[158].cevr";
connectAttr "animCurveTL4582.a" "clipLibrary1.cel[0].cev[159].cevr";
connectAttr "animCurveTL4583.a" "clipLibrary1.cel[0].cev[160].cevr";
connectAttr "animCurveTL4584.a" "clipLibrary1.cel[0].cev[161].cevr";
connectAttr "animCurveTL4585.a" "clipLibrary1.cel[0].cev[162].cevr";
connectAttr "animCurveTL4586.a" "clipLibrary1.cel[0].cev[163].cevr";
connectAttr "animCurveTL4587.a" "clipLibrary1.cel[0].cev[164].cevr";
connectAttr "animCurveTU2261.a" "clipLibrary1.cel[0].cev[165].cevr";
connectAttr "animCurveTU2262.a" "clipLibrary1.cel[0].cev[166].cevr";
connectAttr "animCurveTU2263.a" "clipLibrary1.cel[0].cev[167].cevr";
connectAttr "animCurveTA4399.a" "clipLibrary1.cel[0].cev[168].cevr";
connectAttr "animCurveTA4400.a" "clipLibrary1.cel[0].cev[169].cevr";
connectAttr "animCurveTA4401.a" "clipLibrary1.cel[0].cev[170].cevr";
connectAttr "animCurveTL4588.a" "clipLibrary1.cel[0].cev[171].cevr";
connectAttr "animCurveTL4589.a" "clipLibrary1.cel[0].cev[172].cevr";
connectAttr "animCurveTL4590.a" "clipLibrary1.cel[0].cev[173].cevr";
connectAttr "animCurveTU2264.a" "clipLibrary1.cel[0].cev[174].cevr";
connectAttr "animCurveTU2265.a" "clipLibrary1.cel[0].cev[175].cevr";
connectAttr "animCurveTU2266.a" "clipLibrary1.cel[0].cev[176].cevr";
connectAttr "animCurveTA4402.a" "clipLibrary1.cel[0].cev[177].cevr";
connectAttr "animCurveTA4403.a" "clipLibrary1.cel[0].cev[178].cevr";
connectAttr "animCurveTA4404.a" "clipLibrary1.cel[0].cev[179].cevr";
connectAttr "animCurveTL4591.a" "clipLibrary1.cel[0].cev[180].cevr";
connectAttr "animCurveTL4592.a" "clipLibrary1.cel[0].cev[181].cevr";
connectAttr "animCurveTL4593.a" "clipLibrary1.cel[0].cev[182].cevr";
connectAttr "animCurveTL4594.a" "clipLibrary1.cel[0].cev[183].cevr";
connectAttr "animCurveTL4595.a" "clipLibrary1.cel[0].cev[184].cevr";
connectAttr "animCurveTL4596.a" "clipLibrary1.cel[0].cev[185].cevr";
connectAttr "animCurveTL4597.a" "clipLibrary1.cel[0].cev[186].cevr";
connectAttr "animCurveTL4598.a" "clipLibrary1.cel[0].cev[187].cevr";
connectAttr "animCurveTL4599.a" "clipLibrary1.cel[0].cev[188].cevr";
connectAttr "animCurveTA4405.a" "clipLibrary1.cel[0].cev[189].cevr";
connectAttr "animCurveTA4406.a" "clipLibrary1.cel[0].cev[190].cevr";
connectAttr "animCurveTA4407.a" "clipLibrary1.cel[0].cev[191].cevr";
connectAttr "animCurveTA4408.a" "clipLibrary1.cel[0].cev[192].cevr";
connectAttr "animCurveTA4409.a" "clipLibrary1.cel[0].cev[193].cevr";
connectAttr "animCurveTA4410.a" "clipLibrary1.cel[0].cev[194].cevr";
connectAttr "animCurveTL4600.a" "clipLibrary1.cel[0].cev[195].cevr";
connectAttr "animCurveTL4601.a" "clipLibrary1.cel[0].cev[196].cevr";
connectAttr "animCurveTL4602.a" "clipLibrary1.cel[0].cev[197].cevr";
connectAttr "animCurveTU2267.a" "clipLibrary1.cel[0].cev[198].cevr";
connectAttr "animCurveTU2268.a" "clipLibrary1.cel[0].cev[199].cevr";
connectAttr "animCurveTL4603.a" "clipLibrary1.cel[0].cev[200].cevr";
connectAttr "animCurveTL4604.a" "clipLibrary1.cel[0].cev[201].cevr";
connectAttr "animCurveTL4605.a" "clipLibrary1.cel[0].cev[202].cevr";
connectAttr "animCurveTA4411.a" "clipLibrary1.cel[0].cev[203].cevr";
connectAttr "animCurveTA4412.a" "clipLibrary1.cel[0].cev[204].cevr";
connectAttr "animCurveTA4413.a" "clipLibrary1.cel[0].cev[205].cevr";
connectAttr "animCurveTA4414.a" "clipLibrary1.cel[0].cev[206].cevr";
connectAttr "animCurveTA4415.a" "clipLibrary1.cel[0].cev[207].cevr";
connectAttr "animCurveTA4416.a" "clipLibrary1.cel[0].cev[208].cevr";
connectAttr "animCurveTA4417.a" "clipLibrary1.cel[0].cev[209].cevr";
connectAttr "animCurveTA4418.a" "clipLibrary1.cel[0].cev[210].cevr";
connectAttr "animCurveTA4419.a" "clipLibrary1.cel[0].cev[211].cevr";
connectAttr "animCurveTL4606.a" "clipLibrary1.cel[0].cev[212].cevr";
connectAttr "animCurveTL4607.a" "clipLibrary1.cel[0].cev[213].cevr";
connectAttr "animCurveTL4608.a" "clipLibrary1.cel[0].cev[214].cevr";
connectAttr ":defaultLightSet.msg" "MG_lightLinker1.lnk[2].llnk";
connectAttr ":defaultLightSet.msg" "MG_lightLinker1.lnk[3].llnk";
connectAttr ":defaultLightSet.msg" "MG_lightLinker1.lnk[5].llnk";
connectAttr ":defaultLightSet.msg" "MG_lightLinker1.lnk[6].llnk";
connectAttr ":defaultLightSet.msg" "MG_lightLinker1.lnk[7].llnk";
connectAttr ":defaultLightSet.msg" "MG_lightLinker1.lnk[8].llnk";
connectAttr ":defaultLightSet.msg" "MG_lightLinker1.lnk[9].llnk";
connectAttr ":defaultLightSet.msg" "MG_lightLinker1.lnk[10].llnk";
connectAttr ":defaultLightSet.msg" "PS_lightLinker1.lnk[1].llnk";
connectAttr ":defaultLightSet.msg" "PS_lightLinker1.lnk[3].llnk";
connectAttr ":defaultLightSet.msg" "PS_lightLinker1.lnk[5].llnk";
connectAttr ":defaultLightSet.msg" "PS_lightLinker1.lnk[6].llnk";
connectAttr ":defaultLightSet.msg" "PS_lightLinker1.lnk[7].llnk";
connectAttr ":defaultLightSet.msg" "PS_lightLinker1.lnk[8].llnk";
connectAttr ":defaultLightSet.msg" "PS_lightLinker1.lnk[9].llnk";
connectAttr ":defaultLightSet.msg" "PS_lightLinker1.lnk[11].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[1].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[5].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[7].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[8].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[9].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[10].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[11].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[12].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[13].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[14].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[16].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[17].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[18].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[19].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[35].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[38].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[53].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[74].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[75].llnk";
connectAttr ":defaultLightSet.msg" "RF_lightLinker1.lnk[77].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[2].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[3].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[4].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[5].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[6].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[7].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[8].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[9].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[11].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[12].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[13].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[14].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[15].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[16].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[17].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[18].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[20].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[21].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[22].llnk";
connectAttr ":defaultLightSet.msg" "SM_SMG1_lightLinker1.lnk[23].llnk";
connectAttr ":defaultLightSet.msg" "RL_lightLinker1.lnk[5].llnk";
connectAttr ":defaultLightSet.msg" "RL_lightLinker1.lnk[6].llnk";
connectAttr ":defaultLightSet.msg" "RL_lightLinker1.lnk[8].llnk";
connectAttr ":defaultLightSet.msg" "KN_lightLinker1.lnk[1].llnk";
connectAttr ":defaultLightSet.msg" "KN_lightLinker1.lnk[3].llnk";
connectAttr ":defaultLightSet.msg" "KN_lightLinker1.lnk[4].llnk";
connectAttr ":defaultLightSet.msg" "KN_lightLinker1.lnk[5].llnk";
connectAttr ":defaultLightSet.msg" "KN_lightLinker1.lnk[7].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[0].llnk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.lnk[0].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[1].llnk";
connectAttr ":initialParticleSE.msg" "lightLinker1.lnk[1].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[2].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[9].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[10].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[14].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[15].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[22].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[23].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[24].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[32].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[33].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[34].llnk";
connectAttr "MG_lightLinker1.msg" ":lightList1.ln" -na;
connectAttr "PS_lightLinker1.msg" ":lightList1.ln" -na;
connectAttr "RF_lightLinker1.msg" ":lightList1.ln" -na;
connectAttr "SM_SMG1_lightLinker1.msg" ":lightList1.ln" -na;
connectAttr "RL_lightLinker1.msg" ":lightList1.ln" -na;
connectAttr "KN_lightLinker1.msg" ":lightList1.ln" -na;
connectAttr "lightLinker1.msg" ":lightList1.ln" -na;
// End of shootFront00+.ma
