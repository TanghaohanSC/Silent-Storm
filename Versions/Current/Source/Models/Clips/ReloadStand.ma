//Maya ASCII 4.0 scene
//Name: ReloadStand.ma
//Last modified: Tue, Feb 12, 2002 04:18:55 PM
requires maya "4.0";
currentUnit -l centimeter -a degree -t ntsc;
createNode animClip -n "clip00Source";
	setAttr ".ihi" 0;
	setAttr ".du" 80;
	setAttr ".ci" no;
createNode animCurveTU -n "animCurveTU297";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 4;
createNode animCurveTA -n "animCurveTA665";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -465.92496686923408;
createNode animCurveTA -n "animCurveTA666";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -81.339856873831991;
createNode animCurveTA -n "animCurveTA667";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 292.4483677477059;
createNode animCurveTL -n "animCurveTL694";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.42790223534792582;
createNode animCurveTL -n "animCurveTL695";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.17236352849116041;
createNode animCurveTL -n "animCurveTL696";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.20601379863348462;
createNode animCurveTU -n "animCurveTU298";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 0;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU299";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 0 9 0 9.1 1 57 1 57.100000000000001 
		0 67 0;
	setAttr -s 6 ".kot[0:5]"  5 5 5 5 5 5;
createNode animCurveTA -n "animCurveTA668";
	setAttr ".tan" 2;
	setAttr -s 6 ".ktv[0:5]"  0 209.25606360009732 9 209.25606360009732 
		9.1 282.75012330289326 57 282.75012330289326 57.100000000000001 209.32589479349505 
		67 209.25606360009732;
	setAttr -s 6 ".kit[5]"  10;
	setAttr -s 6 ".kot[5]"  10;
createNode animCurveTA -n "animCurveTA669";
	setAttr ".tan" 2;
	setAttr -s 6 ".ktv[0:5]"  0 6.0384967741173146 9 6.0384967741173146 
		9.1 -14.082871088102621 57 -14.082871088102621 57.100000000000001 6.0194545910188477 
		67 6.0384967741173146;
	setAttr -s 6 ".kit[5]"  10;
	setAttr -s 6 ".kot[5]"  10;
createNode animCurveTA -n "animCurveTA670";
	setAttr ".tan" 2;
	setAttr -s 6 ".ktv[0:5]"  0 -64.426617816645219 9 -64.426617816645219 
		9.1 -164.66835464017501 57 -164.66835464017501 57.100000000000001 -64.385053333062544 
		67 -64.426617816645219;
	setAttr -s 6 ".kit[5]"  10;
	setAttr -s 6 ".kot[5]"  10;
createNode animCurveTL -n "animCurveTL697";
	setAttr ".tan" 2;
	setAttr -s 6 ".ktv[0:5]"  0 -0.047320047367157989 9 -0.047320047367157989 
		9.1 -0.10004844784321193 57 -0.10004844784321193 57.100000000000001 -0.04797564920028323 
		67 -0.047320047367157989;
	setAttr -s 6 ".kit[5]"  10;
	setAttr -s 6 ".kot[5]"  10;
createNode animCurveTL -n "animCurveTL698";
	setAttr ".tan" 2;
	setAttr -s 6 ".ktv[0:5]"  0 0.001348939808841945 9 0.001348939808841945 
		9.1 0.41922525367342617 57 0.41922525367342617 57.100000000000001 0.0015942438261276615 
		67 0.001348939808841945;
	setAttr -s 6 ".kit[5]"  10;
	setAttr -s 6 ".kot[5]"  10;
createNode animCurveTL -n "animCurveTL699";
	setAttr ".tan" 2;
	setAttr -s 6 ".ktv[0:5]"  0 -0.07193308384154877 9 -0.07193308384154877 
		9.1 -0.041693071359783104 57 -0.041693071359783104 57.100000000000001 -0.072198639296221251 
		67 -0.07193308384154877;
	setAttr -s 6 ".kit[5]"  10;
	setAttr -s 6 ".kot[5]"  10;
createNode animCurveTU -n "animCurveTU300";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 1 9 1 9.1 1 57 1 57.100000000000001 
		1 67 1;
	setAttr -s 6 ".kot[0:5]"  5 5 5 5 5 5;
createNode animCurveTU -n "animCurveTU301";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 2;
createNode animCurveTA -n "animCurveTA671";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 175.63959866373224;
createNode animCurveTA -n "animCurveTA672";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 82.671540597353498;
createNode animCurveTA -n "animCurveTA673";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 236.95768315183463;
createNode animCurveTL -n "animCurveTL700";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.026093145395513644;
createNode animCurveTL -n "animCurveTL701";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.088084485238461366;
createNode animCurveTL -n "animCurveTL702";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.16887736870556289;
createNode animCurveTU -n "animCurveTU302";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 0;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "Skeleton_Slot9_parent";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "Skeleton_Slot9_rotateZ";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 2.3154399048039953;
	setAttr ".roti" 3;
createNode animCurveTA -n "Skeleton_Slot9_rotateY";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 37.902992356007296;
	setAttr ".roti" 3;
createNode animCurveTA -n "Skeleton_Slot9_rotateX";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 0.34481341447015323;
	setAttr ".roti" 3;
createNode animCurveTL -n "Skeleton_Slot9_translateZ";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.090690922975573018;
createNode animCurveTL -n "Skeleton_Slot9_translateY";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.022682940295604748;
createNode animCurveTL -n "Skeleton_Slot9_translateX";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.17220369839616484;
createNode animCurveTU -n "Skeleton_Slot9_visibility";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU303";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA674";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 32.610219777792729;
createNode animCurveTA -n "animCurveTA675";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 85.440596802632385;
createNode animCurveTA -n "animCurveTA676";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -39.273843515614992;
createNode animCurveTL -n "animCurveTL703";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.083590430119444337;
createNode animCurveTL -n "animCurveTL704";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.097071155479687726;
createNode animCurveTL -n "animCurveTL705";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.073509714388873229;
createNode animCurveTU -n "animCurveTU304";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU305";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA677";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 145.28430637455284;
createNode animCurveTA -n "animCurveTA678";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 84.323169791981073;
createNode animCurveTA -n "animCurveTA679";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 41.122861976945927;
createNode animCurveTL -n "animCurveTL706";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.083590430119444337;
createNode animCurveTL -n "animCurveTL707";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.091448079884016986;
createNode animCurveTL -n "animCurveTL708";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.085576106421556733;
createNode animCurveTU -n "animCurveTU306";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "Skeleton_Slot6_parent";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "Skeleton_Slot6_rotateZ";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 41.010315328440733;
	setAttr ".roti" 3;
createNode animCurveTA -n "Skeleton_Slot6_rotateY";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 9.8914343590187706;
	setAttr ".roti" 3;
createNode animCurveTA -n "Skeleton_Slot6_rotateX";
	setAttr ".tan" 2;
	setAttr ".ktv[0]"  0 -37.035768368166487;
	setAttr ".roti" 3;
createNode animCurveTL -n "Skeleton_Slot6_translateZ";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.086445046368000655;
createNode animCurveTL -n "Skeleton_Slot6_translateY";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.020305714530646057;
createNode animCurveTL -n "Skeleton_Slot6_translateX";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.16972326218266887;
createNode animCurveTU -n "Skeleton_Slot6_visibility";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU307";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA680";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTA -n "animCurveTA681";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 90;
createNode animCurveTA -n "animCurveTA682";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 55.04359299190547;
createNode animCurveTL -n "animCurveTL709";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.092078740514516033;
createNode animCurveTL -n "animCurveTL710";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.13816814638046276;
createNode animCurveTL -n "animCurveTL711";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.10227808331933355;
createNode animCurveTU -n "animCurveTU308";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU309";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA683";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 3.0533324942049761e-013;
createNode animCurveTA -n "animCurveTA684";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 90;
createNode animCurveTA -n "animCurveTA685";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 89.999999999999289;
createNode animCurveTL -n "animCurveTL712";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.092078740514516033;
createNode animCurveTL -n "animCurveTL713";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.16270320199464611;
createNode animCurveTL -n "animCurveTL714";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTU -n "animCurveTU310";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU311";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 2;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA686";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTA -n "animCurveTA687";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 89.999999999999986;
createNode animCurveTA -n "animCurveTA688";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 121.7093470486665;
createNode animCurveTL -n "animCurveTL715";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.092078740514516033;
createNode animCurveTL -n "animCurveTL716";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.13818956154465087;
createNode animCurveTL -n "animCurveTL717";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.1024618401142183;
createNode animCurveTU -n "animCurveTU312";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU313";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 4;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA689";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 20.333183631937793;
createNode animCurveTA -n "animCurveTA690";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 100.90515844868014;
createNode animCurveTA -n "animCurveTA691";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 89.999999999999815;
createNode animCurveTL -n "animCurveTL718";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.094445367888397191;
createNode animCurveTL -n "animCurveTL719";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.14789849796524912;
createNode animCurveTL -n "animCurveTL720";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.12265632193298315;
createNode animCurveTU -n "animCurveTU314";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU315";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 4;
	setAttr ".kot[0]"  5;
createNode animCurveTA -n "animCurveTA692";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -30.725722245344027;
createNode animCurveTA -n "animCurveTA693";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 78.914277911490771;
createNode animCurveTA -n "animCurveTA694";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 89.999999999999872;
createNode animCurveTL -n "animCurveTL721";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0.094791345736591426;
createNode animCurveTL -n "animCurveTL722";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.14845093250493735;
createNode animCurveTL -n "animCurveTL723";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 -0.13388933838305556;
createNode animCurveTU -n "animCurveTU316";
	setAttr ".tan" 9;
	setAttr ".ktv[0]"  0 1;
	setAttr ".kot[0]"  5;
createNode animCurveTU -n "animCurveTU317";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 1;
createNode animCurveTU -n "animCurveTU318";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 1;
createNode animCurveTU -n "animCurveTU319";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 1;
createNode animCurveTA -n "animCurveTA695";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTA -n "animCurveTA696";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTA -n "animCurveTA697";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTL -n "animCurveTL724";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTL -n "animCurveTL725";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTL -n "animCurveTL726";
	setAttr ".tan" 10;
	setAttr ".ktv[0]"  0 0;
createNode animCurveTA -n "animCurveTA698";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 0 80 0;
createNode animCurveTA -n "animCurveTA699";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 0 80 0;
createNode animCurveTA -n "animCurveTA700";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 86.111703431999985 80 86.111703431999985;
createNode animCurveTA -n "animCurveTA701";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 0 80 0;
createNode animCurveTA -n "animCurveTA702";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 0 80 0;
createNode animCurveTL -n "animCurveTL727";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 -0.0016697872460390006 80 -0.0016697872460390006;
createNode animCurveTL -n "animCurveTL728";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 -0.10832795437875198 80 -0.10832795437875198;
createNode animCurveTL -n "animCurveTL729";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 0.025350618892568857 80 0.025350618892568857;
createNode animCurveTA -n "animCurveTA703";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 94.296819964999983 80 94.296819964999983;
createNode animCurveTA -n "animCurveTA704";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 0 80 0;
createNode animCurveTA -n "animCurveTA705";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 -0.90018898340000086 80 -0.90018898340000086;
createNode animCurveTL -n "animCurveTL730";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 -0.0020498860481951708 80 -0.0020498860481951708;
createNode animCurveTL -n "animCurveTL731";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 0.073456169030626448 80 0.073456169030626448;
createNode animCurveTL -n "animCurveTL732";
	setAttr ".tan" 3;
	setAttr -s 2 ".ktv[0:1]"  0 -0.15665197174057599 80 -0.15665197174057599;
createNode animCurveTL -n "animCurveTL733";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 -0.049358564484432765 20 -0.049358564484432765 
		45 -0.049358564484432765 80 -0.049358564484432765;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTL -n "animCurveTL734";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 0.3021643468469557 20 0.28168730020412408 
		45 0.3021643468469557 80 0.3021643468469557;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTL -n "animCurveTL735";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 0.34516920356298542 20 0.39560290856739205 
		45 0.34516920356298542 80 0.34516920356298542;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTL -n "animCurveTL736";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 -0.044738945874433278 20 -0.044738945874433278 
		45 -0.044738945874433278 80 -0.044738945874433278;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTL -n "animCurveTL737";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 0.5150070836563363 20 0.53635646741777865 
		45 0.5150070836563363 80 0.5150070836563363;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTL -n "animCurveTL738";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 0.16745478835360703 20 0.17939643145049214 
		45 0.16745478835360703 80 0.16745478835360703;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTU -n "animCurveTU320";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 5 9 5 13 2.7000000000000002 22 
		2.7000000000000002 27 6.2000000000000002 30 6.2000000000000002 32 6.2986394729280599 
		37 6.2000000000000002 42 3.2999999999999989 47 3.3200000122070303 57 5 61 
		5.1209599962234496 67 5 80 5;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTU -n "animCurveTU321";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 2.1000000000000005 9 2.1000000000000005 
		13 -0.099999999999999645 22 1.6000000000000005 27 5.4000000000000004 30 5.4000000000000004 
		32 5.5088435587313018 37 5.4000000000000004 42 2.2000000000000002 47 1.9112000549793247 
		57 2.1000000000000005 61 2.1135935951471332 67 2.1000000000000005 80 2.1000000000000005;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTU -n "animCurveTU322";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 1.6000000000000005 9 1.6000000000000005 
		13 -0.2999999999999996 22 2.2000000000000002 27 4.1000000000000005 30 4.1000000000000005 
		32 3.6791060154361563 37 2.2000000000000002 42 -0.29999999999999982 47 -0.21439999237060525 
		57 1.6000000000000005 61 1.7306367969512946 67 1.6000000000000005 80 1.6000000000000005;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTA -n "animCurveTA706";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 -118.86844155632033 9 -129.15041404615457 
		13 -129.15041404615457 22 -118.49502486415017 27 -145.24971433155832 30 -145.24971433155832 
		32 -137.614476144154 37 -127.6224650132379 42 -116.57576831160264 47 -117.48114281221541 
		57 -129.15041404615457 61 -126.4875374769541 67 -118.30459836350217 80 -118.86844155632033;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTA -n "animCurveTA707";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 6.6312945973673276 9 -18.25889608481959 
		13 -18.25889608481959 22 4.5344069658057986 27 1.5044377388304251 30 1.5044377388304251 
		32 3.9141165915220268 37 1.405270732202486 42 10.553926308881245 47 8.4794030759211285 
		57 -18.25889608481959 61 -17.796677464346978 67 -7.4305725028984453 80 6.6312945973673276;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTA -n "animCurveTA708";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 -65.480443573845548 9 -64.243058185740495 
		13 -64.243058185740495 22 -65.545499772325414 27 -114.66825348345481 30 -114.66825348345481 
		32 -102.12236101383434 37 -110.24986387981522 42 -67.552469155312167 47 -67.314191568652035 
		57 -64.243058185740495 61 -65.880400448274813 67 -70.292380633408882 80 -65.480443573845548;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTL -n "animCurveTL739";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 -0.25821113567068532 9 -0.20713539499734201 
		13 -0.15235280289143693 22 -0.096974738904481939 27 -0.15266286016542052 
		30 -0.16257063902987978 32 -0.14594953384637982 37 -0.13257003205114803 42 
		-0.12697475213755305 47 -0.13274631850920754 57 -0.20713539499734201 61 -0.21822799695332451 
		67 -0.25956305014393966 80 -0.25821113567068532;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTL -n "animCurveTL740";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 -0.15182254516888952 9 -0.22021934184671568 
		13 -0.20318283032010168 22 -0.038488975277624543 27 -0.052989969487894167 
		30 -0.15560330446683127 32 -0.14998633277211257 37 -0.056710495151189394 
		42 -0.05491913732519535 47 -0.066820752024235958 57 -0.22021934184671568 
		61 -0.23669879793093895 67 -0.19824518838199603 80 -0.15182254516888952;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTL -n "animCurveTL741";
	setAttr ".tan" 9;
	setAttr -s 14 ".ktv[0:13]"  0 0.85671088606950629 9 0.81382771203465765 
		13 0.87100968189444283 22 0.90151637195308454 27 0.96928586314978982 30 0.94795935907478968 
		32 0.94471038885647862 37 0.96563686678397231 42 0.97151710154683113 47 0.9601634655996989 
		57 0.81382771203465765 61 0.81070249636520253 67 0.84596577585711308 80 0.85671088606950629;
	setAttr -s 14 ".kit[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
	setAttr -s 14 ".kot[0:13]"  3 9 9 9 9 9 9 
		9 9 9 9 10 10 3;
createNode animCurveTU -n "animCurveTU323";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 4.7 9 4.7 22 4.7 32 4.7 52 4.7 
		57 4.7 61 4.7 67 4.7 80 4.7;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTU -n "animCurveTU324";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 3.5 9 3.5 22 3.5 32 3.5 52 3.5 
		57 3.5 61 3.5 67 3.5 80 3.5;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTU -n "animCurveTU325";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 2.9000000000000004 9 2.9000000000000004 
		22 2.9000000000000004 32 2.9000000000000004 52 2.9000000000000004 57 2.9000000000000004 
		61 2.9000000000000004 67 2.9000000000000004 80 2.9000000000000004;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTA -n "animCurveTA709";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 -7.3924559138835884 9 -10.126416172155734 
		22 -13.621204362228859 32 -15.681960597272633 52 -10.126416172155734 57 -10.126416172155734 
		61 -10.494296862520294 67 4.6895172870169342 80 -7.3924559138835884;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTA -n "animCurveTA710";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 -41.311825077366123 9 -19.003398645360068 
		22 -18.363639914963375 32 -17.123612955780459 52 -19.003398645360068 57 -19.003398645360068 
		61 -19.052990868885932 67 -18.718517465862416 80 -41.311825077366123;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTA -n "animCurveTA711";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 -148.00687982094675 9 -163.66906676402843 
		22 -166.59662849009774 32 -164.66206614871027 52 -163.66906676402843 57 -163.66906676402843 
		61 -161.49793345596265 67 -157.40405515878288 80 -148.00687982094675;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTL -n "animCurveTL742";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 -0.087190477370387764 9 -0.21610200259934031 
		22 -0.23633845050072866 32 -0.24861191091981497 52 -0.21610200259934031 57 
		-0.21610200259934031 61 -0.21018312105676676 67 -0.15109510384336189 80 -0.087190477370387764;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTL -n "animCurveTL743";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 0.29525993216332275 9 0.2720603349196356 
		22 0.2645008135994516 32 0.24125792608743379 52 0.2720603349196356 57 0.2720603349196356 
		61 0.27145584329047201 67 0.27455390375254479 80 0.29525993216332275;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTL -n "animCurveTL744";
	setAttr ".tan" 9;
	setAttr -s 9 ".ktv[0:8]"  0 -0.52218742048400935 9 -0.45836377115697008 
		22 -0.45677998074326392 32 -0.4629907942039504 52 -0.45836377115697008 57 
		-0.45836377115697008 61 -0.47865622081405507 67 -0.4990890812293387 80 -0.52218742048400935;
	setAttr -s 9 ".kit[0:8]"  3 9 9 9 9 9 10 
		10 3;
	setAttr -s 9 ".kot[0:8]"  3 9 9 9 9 9 10 
		10 3;
createNode animCurveTL -n "animCurveTL745";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 -0.31541086303721388 9 -0.21811209898181927 
		22 -0.20427721513078123 32 -0.2128313918787533 52 -0.21811209898181927 80 
		-0.31541086303721388;
	setAttr -s 6 ".kit[0:5]"  3 9 9 9 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 9 9 3;
createNode animCurveTL -n "animCurveTL746";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 -0.90756201132097147 9 -0.93444642195663774 
		22 -0.90666925834933632 32 -0.91872312656084876 52 -0.93444642195663774 80 
		-0.90756201132097147;
	setAttr -s 6 ".kit[0:5]"  3 9 9 9 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 9 9 3;
createNode animCurveTL -n "animCurveTL747";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 0.33624250204514528 9 0.37148545856658699 
		22 0.61449019433845864 32 0.55148896654575119 52 0.37148545856658699 80 0.33624250204514528;
	setAttr -s 6 ".kit[0:5]"  3 9 9 9 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 9 9 3;
createNode animCurveTL -n "animCurveTL748";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -0.35391374804527564 9 -0.26861703459771169 
		32 -0.26592587464964723 52 -0.26861703459771169 80 -0.35391374804527564;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTL -n "animCurveTL749";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -0.064535898032066052 9 -0.041042300579107849 
		32 -0.043785268031918442 52 -0.041042300579107849 80 -0.064535898032066052;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTL -n "animCurveTL750";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -0.58071027263732677 9 -0.39953271844906013 
		32 -0.3832653746453718 52 -0.39953271844906013 80 -0.58071027263732677;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTA -n "animCurveTA712";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 5 9 5.2572301934075645 22 10.220619800277635 
		32 3.7997930078629341 80 5;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTA -n "animCurveTA713";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -5 9 -0.085703967819726765 22 
		12.113760555263514 32 4.0633836865624602 80 -5;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTA -n "animCurveTA714";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 3.0000000000000004 9 2.9885770564617502 
		22 5.3739295801215432 32 4.0469467916413029 80 3.0000000000000004;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTA -n "animCurveTA715";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 -5 9 -5.3820331635589866 80 -5;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTA -n "animCurveTA716";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 5 9 -2.3035562681093351 80 5;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTA -n "animCurveTA717";
	setAttr ".tan" 3;
	setAttr -s 3 ".ktv[0:2]"  0 3.0000000000000004 9 2.9909929198857661 
		80 3.0000000000000004;
	setAttr -s 3 ".kit[1:2]"  9 3;
	setAttr -s 3 ".kot[1:2]"  9 3;
createNode animCurveTL -n "animCurveTL751";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 0 17 0 35 0 55 0 80 0;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 3;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 3;
createNode animCurveTL -n "animCurveTL752";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 0 17 -0.0059839640815252393 35 
		-0.044337297551123554 55 -0.00085623975249033216 80 0;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 3;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 3;
createNode animCurveTL -n "animCurveTL753";
	setAttr ".tan" 10;
	setAttr -s 5 ".ktv[0:4]"  0 0 17 0 35 0 55 0 80 0;
	setAttr -s 5 ".kit[0:4]"  3 10 10 10 3;
	setAttr -s 5 ".kot[0:4]"  3 10 10 10 3;
createNode animCurveTU -n "animCurveTU326";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -5 17 17.021426939337669 32 14.074440761664395 
		63 3.5 80 -5;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTU -n "animCurveTU327";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 0 9 0 17 0 32 0 63 0 80 0;
	setAttr -s 6 ".kit[0:5]"  3 9 9 9 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 9 9 3;
createNode animCurveTL -n "animCurveTL754";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 0.035220830356047284 9 -0.086513136843691107 
		17 -0.11441571968549535 32 -0.102410589658779 63 -0.086513136843691107 80 
		0.035220830356047284;
	setAttr -s 6 ".kit[0:5]"  3 9 9 9 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 9 9 3;
createNode animCurveTL -n "animCurveTL755";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 0.56396493947465143 9 0.56396493947465143 
		17 0.56391905776226992 32 0.55911255812429117 63 0.56396493947465143 80 0.56396493947465143;
	setAttr -s 6 ".kit[0:5]"  3 9 9 9 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 9 9 3;
createNode animCurveTL -n "animCurveTL756";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 0.3446828801293973 9 0.29244285445548202 
		17 0.28493610415587522 32 0.28775239203105069 63 0.29244285445548202 80 0.3446828801293973;
	setAttr -s 6 ".kit[0:5]"  3 9 9 9 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 9 9 3;
createNode animCurveTA -n "animCurveTA718";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 91.732908780903117 9 91.732908780903131 
		22 91.732908780903131 30 87.80460130549514 52 91.732908780903131 80 91.732908780903117;
	setAttr -s 6 ".kit[0:5]"  3 9 9 10 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 10 9 3;
createNode animCurveTA -n "animCurveTA719";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 -1.1455029386183242 9 -1.145502938618326 
		22 -1.145502938618326 30 -3.3963445575695541 52 -1.145502938618326 80 -1.1455029386183242;
	setAttr -s 6 ".kit[0:5]"  3 9 9 10 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 10 9 3;
createNode animCurveTA -n "animCurveTA720";
	setAttr ".tan" 9;
	setAttr -s 6 ".ktv[0:5]"  0 -0.64957549402554438 9 7.3771147892880826 
		22 7.3771147892880826 30 10.287255833977625 52 7.3771147892880826 80 -0.64957549402554438;
	setAttr -s 6 ".kit[0:5]"  3 9 9 10 9 3;
	setAttr -s 6 ".kot[0:5]"  3 9 9 10 9 3;
createNode animCurveTA -n "animCurveTA721";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 89.999999999999986 9 90 52 90 
		80 89.999999999999986;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTA -n "animCurveTA722";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 0 9 0 52 0 80 0;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTA -n "animCurveTA723";
	setAttr ".tan" 9;
	setAttr -s 4 ".ktv[0:3]"  0 0 9 4.7725859937567439 52 4.7725859937567439 
		80 0;
	setAttr -s 4 ".kit[0:3]"  3 9 9 3;
	setAttr -s 4 ".kot[0:3]"  3 9 9 3;
createNode animCurveTA -n "animCurveTA724";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 90 9 89.906796390166846 32 89.999999999999986 
		52 89.999999999999986 80 90;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTA -n "animCurveTA725";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 0 9 1.6045428820765624 32 1.6045428820765624 
		52 0 80 0;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTA -n "animCurveTA726";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -2.3904092277547475 9 -3.3248596402368178 
		32 -3.5033411741490368 52 -3.323554491781906 80 -2.3904092277547475;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTL -n "animCurveTL757";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -0.0047355411366871714 9 -0.0048210463554358984 
		32 -0.0098048754335770278 52 -0.0048210463554358984 80 -0.0047355411366871714;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTL -n "animCurveTL758";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -0.0015698951559620807 9 -0.0015698951559620807 
		32 -0.0064222765063223393 52 -0.0015698951559620807 80 -0.0015698951559620807;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode animCurveTL -n "animCurveTL759";
	setAttr ".tan" 9;
	setAttr -s 5 ".ktv[0:4]"  0 -0.021752286305616472 9 -0.073992311979531752 
		32 -0.084057262981884276 52 -0.073992311979531752 80 -0.021752286305616472;
	setAttr -s 5 ".kit[0:4]"  3 9 9 9 3;
	setAttr -s 5 ".kot[0:4]"  3 9 9 9 3;
createNode clipLibrary -n "clipLibrary1";
	setAttr -s 175 ".cel[0].cev";
	setAttr ".cd[0].cm" -type "characterMapping" 191 "SubMachineGun.parent" 
		0 1 "SubMachineGun.rotateZ" 2 1 "SubMachineGun.rotateY" 
		2 2 "SubMachineGun.rotateX" 2 3 "SubMachineGun.translateZ" 
		1 1 "SubMachineGun.translateY" 1 2 "SubMachineGun.translateX" 
		1 3 "SubMachineGun.visibility" 0 2 "Rifle.parent" 0 
		3 "Rifle.rotateZ" 2 4 "Rifle.rotateY" 2 5 "Rifle.rotateX" 
		2 6 "Rifle.translateZ" 1 4 "Rifle.translateY" 1 
		5 "Rifle.translateX" 1 6 "Rifle.visibility" 0 4 "Item.parent" 
		0 5 "Item.rotateZ" 2 7 "Item.rotateY" 2 8 "Item.rotateX" 
		2 9 "Item.translateZ" 1 7 "Item.translateY" 1 8 "Item.translateX" 
		1 9 "Item.visibility" 0 6 "Cap.parent" 0 7 "Cap.rotateZ" 
		2 10 "Cap.rotateY" 2 11 "Cap.rotateX" 2 12 "Cap.translateZ" 
		1 10 "Cap.translateY" 1 11 "Cap.translateX" 1 12 "Cap.visibility" 
		0 8 "BackPack.parent" 0 9 "BackPack.rotateZ" 2 13 "BackPack.rotateY" 
		2 14 "BackPack.rotateX" 2 15 "BackPack.translateZ" 1 
		13 "BackPack.translateY" 1 14 "BackPack.translateX" 1 15 "BackPack.visibility" 
		0 10 "Slot9.parent" 0 11 "Slot9.rotateZ" 2 16 "Slot9.rotateY" 
		2 17 "Slot9.rotateX" 2 18 "Slot9.translateZ" 1 16 "Slot9.translateY" 
		1 17 "Slot9.translateX" 1 18 "Slot9.visibility" 0 
		12 "Slot8.parent" 0 13 "Slot8.rotateZ" 2 19 "Slot8.rotateY" 
		2 20 "Slot8.rotateX" 2 21 "Slot8.translateZ" 1 19 "Slot8.translateY" 
		1 20 "Slot8.translateX" 1 21 "Slot8.visibility" 0 
		14 "Slot7.parent" 0 15 "Slot7.rotateZ" 2 22 "Slot7.rotateY" 
		2 23 "Slot7.rotateX" 2 24 "Slot7.translateZ" 1 22 "Slot7.translateY" 
		1 23 "Slot7.translateX" 1 24 "Slot7.visibility" 0 
		16 "Slot6.parent" 0 17 "Slot6.rotateZ" 2 25 "Slot6.rotateY" 
		2 26 "Slot6.rotateX" 2 27 "Slot6.translateZ" 1 25 "Slot6.translateY" 
		1 26 "Slot6.translateX" 1 27 "Slot6.visibility" 0 
		18 "Slot5.parent" 0 19 "Slot5.rotateZ" 2 28 "Slot5.rotateY" 
		2 29 "Slot5.rotateX" 2 30 "Slot5.translateZ" 1 28 "Slot5.translateY" 
		1 29 "Slot5.translateX" 1 30 "Slot5.visibility" 0 
		20 "Slot4.parent" 0 21 "Slot4.rotateZ" 2 31 "Slot4.rotateY" 
		2 32 "Slot4.rotateX" 2 33 "Slot4.translateZ" 1 31 "Slot4.translateY" 
		1 32 "Slot4.translateX" 1 33 "Slot4.visibility" 0 
		22 "Slot3.parent" 0 23 "Slot3.rotateZ" 2 34 "Slot3.rotateY" 
		2 35 "Slot3.rotateX" 2 36 "Slot3.translateZ" 1 34 "Slot3.translateY" 
		1 35 "Slot3.translateX" 1 36 "Slot3.visibility" 0 
		24 "Slot2.parent" 0 25 "Slot2.rotateZ" 2 37 "Slot2.rotateY" 
		2 38 "Slot2.rotateX" 2 39 "Slot2.translateZ" 1 37 "Slot2.translateY" 
		1 38 "Slot2.translateX" 1 39 "Slot2.visibility" 0 
		26 "Slot1.parent" 0 27 "Slot1.rotateZ" 2 40 "Slot1.rotateY" 
		2 41 "Slot1.rotateX" 2 42 "Slot1.translateZ" 1 40 "Slot1.translateY" 
		1 41 "Slot1.translateX" 1 42 "Slot1.visibility" 0 
		28 "SkeletonGroup.scaleZ" 0 29 "SkeletonGroup.scaleY" 0 
		30 "SkeletonGroup.scaleX" 0 31 "SkeletonGroup.rotateZ" 2 
		43 "SkeletonGroup.rotateY" 2 44 "SkeletonGroup.rotateX" 2 
		45 "SkeletonGroup.translateZ" 1 43 "SkeletonGroup.translateY" 
		1 44 "SkeletonGroup.translateX" 1 45 "R_ToeLocator.rotateX" 
		2 46 "L_ToeLocator.rotateX" 2 47 "R_FootLocator.rotateZ" 
		2 48 "R_FootLocator.rotateY" 2 49 "R_FootLocator.rotateX" 
		2 50 "R_FootLocator.translateZ" 1 46 "R_FootLocator.translateY" 
		1 47 "R_FootLocator.translateX" 1 48 "L_FootLocator.rotateZ" 
		2 51 "L_FootLocator.rotateY" 2 52 "L_FootLocator.rotateX" 
		2 53 "L_FootLocator.translateZ" 1 49 "L_FootLocator.translateY" 
		1 50 "L_FootLocator.translateX" 1 51 "R_KneeLocator.translateZ" 
		1 52 "R_KneeLocator.translateY" 1 53 "R_KneeLocator.translateX" 
		1 54 "L_KneeLocator.translateZ" 1 55 "L_KneeLocator.translateY" 
		1 56 "L_KneeLocator.translateX" 1 57 "R_HandLocator.palm" 
		0 32 "R_HandLocator.point" 0 33 "R_HandLocator.thumb" 0 
		34 "R_HandLocator.rotateZ" 2 54 "R_HandLocator.rotateY" 2 
		55 "R_HandLocator.rotateX" 2 56 "R_HandLocator.translateZ" 1 
		58 "R_HandLocator.translateY" 1 59 "R_HandLocator.translateX" 
		1 60 "L_HandLocator.palm" 0 35 "L_HandLocator.point" 0 
		36 "L_HandLocator.thumb" 0 37 "L_HandLocator.rotateZ" 2 
		57 "L_HandLocator.rotateY" 2 58 "L_HandLocator.rotateX" 2 
		59 "L_HandLocator.translateZ" 1 61 "L_HandLocator.translateY" 
		1 62 "L_HandLocator.translateX" 1 63 "R_ElbowLocator.translateZ" 
		1 64 "R_ElbowLocator.translateY" 1 65 "R_ElbowLocator.translateX" 
		1 66 "L_ElbowLocator.translateZ" 1 67 "L_ElbowLocator.translateY" 
		1 68 "L_ElbowLocator.translateX" 1 69 "R_ShoulderLocator.rotateZ" 
		2 60 "R_ShoulderLocator.rotateY" 2 61 "R_ShoulderLocator.rotateX" 
		2 62 "L_ShoulderLocator.rotateZ" 2 63 "L_ShoulderLocator.rotateY" 
		2 64 "L_ShoulderLocator.rotateX" 2 65 "HeadPoleLocator.translateZ" 
		1 70 "HeadPoleLocator.translateY" 1 71 "HeadPoleLocator.translateX" 
		1 72 "HeadLocator.NeckRotateX" 0 38 "HeadLocator.NeckRotateY" 
		0 39 "HeadLocator.translateZ" 1 73 "HeadLocator.translateY" 
		1 74 "HeadLocator.translateX" 1 75 "ChestLocator.rotateZ" 
		2 66 "ChestLocator.rotateY" 2 67 "ChestLocator.rotateX" 
		2 68 "SpineLocator.rotateZ" 2 69 "SpineLocator.rotateY" 
		2 70 "SpineLocator.rotateX" 2 71 "HipLocator.rotateZ" 2 
		72 "HipLocator.rotateY" 2 73 "HipLocator.rotateX" 2 74 "HipLocator.translateZ" 
		1 76 "HipLocator.translateY" 1 77 "HipLocator.translateX" 
		1 78  ;
	setAttr ".cd[0].cim" -type "Int32Array" 191 0 1 2 3
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
		 136 137 138 139 140 141 142 143 144 145 146
		 147 148 149 150 151 152 153 154 155 156 157
		 158 159 160 161 162 163 164 165 166 167 168
		 169 170 171 172 173 174 175 176 177 178 179
		 180 181 182 183 184 185 186 187 188 189 190 ;
createNode lightLinker -n "lightLinker1";
select -ne :time1;
	setAttr ".o" 77;
select -ne :renderPartition;
	setAttr -s 15 ".st";
select -ne :renderGlobalsList1;
select -ne :defaultShaderList1;
	setAttr -s 15 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :defaultRenderUtilityList1;
	setAttr -s 13 ".u";
select -ne :lightList1;
select -ne :defaultTextureList1;
	setAttr -s 13 ".tx";
select -ne :initialShadingGroup;
	addAttr -ci true -sn "materialIndex" -ln "materialIndex" -at "long";
	setAttr ".ro" yes;
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
connectAttr "animCurveTU297.a" "clipLibrary1.cel[0].cev[0].cevr";
connectAttr "animCurveTA665.a" "clipLibrary1.cel[0].cev[1].cevr";
connectAttr "animCurveTA666.a" "clipLibrary1.cel[0].cev[2].cevr";
connectAttr "animCurveTA667.a" "clipLibrary1.cel[0].cev[3].cevr";
connectAttr "animCurveTL694.a" "clipLibrary1.cel[0].cev[4].cevr";
connectAttr "animCurveTL695.a" "clipLibrary1.cel[0].cev[5].cevr";
connectAttr "animCurveTL696.a" "clipLibrary1.cel[0].cev[6].cevr";
connectAttr "animCurveTU298.a" "clipLibrary1.cel[0].cev[7].cevr";
connectAttr "animCurveTU299.a" "clipLibrary1.cel[0].cev[8].cevr";
connectAttr "animCurveTA668.a" "clipLibrary1.cel[0].cev[9].cevr";
connectAttr "animCurveTA669.a" "clipLibrary1.cel[0].cev[10].cevr";
connectAttr "animCurveTA670.a" "clipLibrary1.cel[0].cev[11].cevr";
connectAttr "animCurveTL697.a" "clipLibrary1.cel[0].cev[12].cevr";
connectAttr "animCurveTL698.a" "clipLibrary1.cel[0].cev[13].cevr";
connectAttr "animCurveTL699.a" "clipLibrary1.cel[0].cev[14].cevr";
connectAttr "animCurveTU300.a" "clipLibrary1.cel[0].cev[15].cevr";
connectAttr "animCurveTU301.a" "clipLibrary1.cel[0].cev[16].cevr";
connectAttr "animCurveTA671.a" "clipLibrary1.cel[0].cev[17].cevr";
connectAttr "animCurveTA672.a" "clipLibrary1.cel[0].cev[18].cevr";
connectAttr "animCurveTA673.a" "clipLibrary1.cel[0].cev[19].cevr";
connectAttr "animCurveTL700.a" "clipLibrary1.cel[0].cev[20].cevr";
connectAttr "animCurveTL701.a" "clipLibrary1.cel[0].cev[21].cevr";
connectAttr "animCurveTL702.a" "clipLibrary1.cel[0].cev[22].cevr";
connectAttr "animCurveTU302.a" "clipLibrary1.cel[0].cev[23].cevr";
connectAttr "Skeleton_Slot9_parent.a" "clipLibrary1.cel[0].cev[40].cevr"
		;
connectAttr "Skeleton_Slot9_rotateZ.a" "clipLibrary1.cel[0].cev[41].cevr"
		;
connectAttr "Skeleton_Slot9_rotateY.a" "clipLibrary1.cel[0].cev[42].cevr"
		;
connectAttr "Skeleton_Slot9_rotateX.a" "clipLibrary1.cel[0].cev[43].cevr"
		;
connectAttr "Skeleton_Slot9_translateZ.a" "clipLibrary1.cel[0].cev[44].cevr"
		;
connectAttr "Skeleton_Slot9_translateY.a" "clipLibrary1.cel[0].cev[45].cevr"
		;
connectAttr "Skeleton_Slot9_translateX.a" "clipLibrary1.cel[0].cev[46].cevr"
		;
connectAttr "Skeleton_Slot9_visibility.a" "clipLibrary1.cel[0].cev[47].cevr"
		;
connectAttr "animCurveTU303.a" "clipLibrary1.cel[0].cev[48].cevr";
connectAttr "animCurveTA674.a" "clipLibrary1.cel[0].cev[49].cevr";
connectAttr "animCurveTA675.a" "clipLibrary1.cel[0].cev[50].cevr";
connectAttr "animCurveTA676.a" "clipLibrary1.cel[0].cev[51].cevr";
connectAttr "animCurveTL703.a" "clipLibrary1.cel[0].cev[52].cevr";
connectAttr "animCurveTL704.a" "clipLibrary1.cel[0].cev[53].cevr";
connectAttr "animCurveTL705.a" "clipLibrary1.cel[0].cev[54].cevr";
connectAttr "animCurveTU304.a" "clipLibrary1.cel[0].cev[55].cevr";
connectAttr "animCurveTU305.a" "clipLibrary1.cel[0].cev[56].cevr";
connectAttr "animCurveTA677.a" "clipLibrary1.cel[0].cev[57].cevr";
connectAttr "animCurveTA678.a" "clipLibrary1.cel[0].cev[58].cevr";
connectAttr "animCurveTA679.a" "clipLibrary1.cel[0].cev[59].cevr";
connectAttr "animCurveTL706.a" "clipLibrary1.cel[0].cev[60].cevr";
connectAttr "animCurveTL707.a" "clipLibrary1.cel[0].cev[61].cevr";
connectAttr "animCurveTL708.a" "clipLibrary1.cel[0].cev[62].cevr";
connectAttr "animCurveTU306.a" "clipLibrary1.cel[0].cev[63].cevr";
connectAttr "Skeleton_Slot6_parent.a" "clipLibrary1.cel[0].cev[64].cevr"
		;
connectAttr "Skeleton_Slot6_rotateZ.a" "clipLibrary1.cel[0].cev[65].cevr"
		;
connectAttr "Skeleton_Slot6_rotateY.a" "clipLibrary1.cel[0].cev[66].cevr"
		;
connectAttr "Skeleton_Slot6_rotateX.a" "clipLibrary1.cel[0].cev[67].cevr"
		;
connectAttr "Skeleton_Slot6_translateZ.a" "clipLibrary1.cel[0].cev[68].cevr"
		;
connectAttr "Skeleton_Slot6_translateY.a" "clipLibrary1.cel[0].cev[69].cevr"
		;
connectAttr "Skeleton_Slot6_translateX.a" "clipLibrary1.cel[0].cev[70].cevr"
		;
connectAttr "Skeleton_Slot6_visibility.a" "clipLibrary1.cel[0].cev[71].cevr"
		;
connectAttr "animCurveTU307.a" "clipLibrary1.cel[0].cev[72].cevr";
connectAttr "animCurveTA680.a" "clipLibrary1.cel[0].cev[73].cevr";
connectAttr "animCurveTA681.a" "clipLibrary1.cel[0].cev[74].cevr";
connectAttr "animCurveTA682.a" "clipLibrary1.cel[0].cev[75].cevr";
connectAttr "animCurveTL709.a" "clipLibrary1.cel[0].cev[76].cevr";
connectAttr "animCurveTL710.a" "clipLibrary1.cel[0].cev[77].cevr";
connectAttr "animCurveTL711.a" "clipLibrary1.cel[0].cev[78].cevr";
connectAttr "animCurveTU308.a" "clipLibrary1.cel[0].cev[79].cevr";
connectAttr "animCurveTU309.a" "clipLibrary1.cel[0].cev[80].cevr";
connectAttr "animCurveTA683.a" "clipLibrary1.cel[0].cev[81].cevr";
connectAttr "animCurveTA684.a" "clipLibrary1.cel[0].cev[82].cevr";
connectAttr "animCurveTA685.a" "clipLibrary1.cel[0].cev[83].cevr";
connectAttr "animCurveTL712.a" "clipLibrary1.cel[0].cev[84].cevr";
connectAttr "animCurveTL713.a" "clipLibrary1.cel[0].cev[85].cevr";
connectAttr "animCurveTL714.a" "clipLibrary1.cel[0].cev[86].cevr";
connectAttr "animCurveTU310.a" "clipLibrary1.cel[0].cev[87].cevr";
connectAttr "animCurveTU311.a" "clipLibrary1.cel[0].cev[88].cevr";
connectAttr "animCurveTA686.a" "clipLibrary1.cel[0].cev[89].cevr";
connectAttr "animCurveTA687.a" "clipLibrary1.cel[0].cev[90].cevr";
connectAttr "animCurveTA688.a" "clipLibrary1.cel[0].cev[91].cevr";
connectAttr "animCurveTL715.a" "clipLibrary1.cel[0].cev[92].cevr";
connectAttr "animCurveTL716.a" "clipLibrary1.cel[0].cev[93].cevr";
connectAttr "animCurveTL717.a" "clipLibrary1.cel[0].cev[94].cevr";
connectAttr "animCurveTU312.a" "clipLibrary1.cel[0].cev[95].cevr";
connectAttr "animCurveTU313.a" "clipLibrary1.cel[0].cev[96].cevr";
connectAttr "animCurveTA689.a" "clipLibrary1.cel[0].cev[97].cevr";
connectAttr "animCurveTA690.a" "clipLibrary1.cel[0].cev[98].cevr";
connectAttr "animCurveTA691.a" "clipLibrary1.cel[0].cev[99].cevr";
connectAttr "animCurveTL718.a" "clipLibrary1.cel[0].cev[100].cevr";
connectAttr "animCurveTL719.a" "clipLibrary1.cel[0].cev[101].cevr";
connectAttr "animCurveTL720.a" "clipLibrary1.cel[0].cev[102].cevr";
connectAttr "animCurveTU314.a" "clipLibrary1.cel[0].cev[103].cevr";
connectAttr "animCurveTU315.a" "clipLibrary1.cel[0].cev[104].cevr";
connectAttr "animCurveTA692.a" "clipLibrary1.cel[0].cev[105].cevr";
connectAttr "animCurveTA693.a" "clipLibrary1.cel[0].cev[106].cevr";
connectAttr "animCurveTA694.a" "clipLibrary1.cel[0].cev[107].cevr";
connectAttr "animCurveTL721.a" "clipLibrary1.cel[0].cev[108].cevr";
connectAttr "animCurveTL722.a" "clipLibrary1.cel[0].cev[109].cevr";
connectAttr "animCurveTL723.a" "clipLibrary1.cel[0].cev[110].cevr";
connectAttr "animCurveTU316.a" "clipLibrary1.cel[0].cev[111].cevr";
connectAttr "animCurveTU317.a" "clipLibrary1.cel[0].cev[112].cevr";
connectAttr "animCurveTU318.a" "clipLibrary1.cel[0].cev[113].cevr";
connectAttr "animCurveTU319.a" "clipLibrary1.cel[0].cev[114].cevr";
connectAttr "animCurveTA695.a" "clipLibrary1.cel[0].cev[115].cevr";
connectAttr "animCurveTA696.a" "clipLibrary1.cel[0].cev[116].cevr";
connectAttr "animCurveTA697.a" "clipLibrary1.cel[0].cev[117].cevr";
connectAttr "animCurveTL724.a" "clipLibrary1.cel[0].cev[118].cevr";
connectAttr "animCurveTL725.a" "clipLibrary1.cel[0].cev[119].cevr";
connectAttr "animCurveTL726.a" "clipLibrary1.cel[0].cev[120].cevr";
connectAttr "animCurveTA698.a" "clipLibrary1.cel[0].cev[121].cevr";
connectAttr "animCurveTA699.a" "clipLibrary1.cel[0].cev[122].cevr";
connectAttr "animCurveTA700.a" "clipLibrary1.cel[0].cev[123].cevr";
connectAttr "animCurveTA701.a" "clipLibrary1.cel[0].cev[124].cevr";
connectAttr "animCurveTA702.a" "clipLibrary1.cel[0].cev[125].cevr";
connectAttr "animCurveTL727.a" "clipLibrary1.cel[0].cev[126].cevr";
connectAttr "animCurveTL728.a" "clipLibrary1.cel[0].cev[127].cevr";
connectAttr "animCurveTL729.a" "clipLibrary1.cel[0].cev[128].cevr";
connectAttr "animCurveTA703.a" "clipLibrary1.cel[0].cev[129].cevr";
connectAttr "animCurveTA704.a" "clipLibrary1.cel[0].cev[130].cevr";
connectAttr "animCurveTA705.a" "clipLibrary1.cel[0].cev[131].cevr";
connectAttr "animCurveTL730.a" "clipLibrary1.cel[0].cev[132].cevr";
connectAttr "animCurveTL731.a" "clipLibrary1.cel[0].cev[133].cevr";
connectAttr "animCurveTL732.a" "clipLibrary1.cel[0].cev[134].cevr";
connectAttr "animCurveTL733.a" "clipLibrary1.cel[0].cev[135].cevr";
connectAttr "animCurveTL734.a" "clipLibrary1.cel[0].cev[136].cevr";
connectAttr "animCurveTL735.a" "clipLibrary1.cel[0].cev[137].cevr";
connectAttr "animCurveTL736.a" "clipLibrary1.cel[0].cev[138].cevr";
connectAttr "animCurveTL737.a" "clipLibrary1.cel[0].cev[139].cevr";
connectAttr "animCurveTL738.a" "clipLibrary1.cel[0].cev[140].cevr";
connectAttr "animCurveTU320.a" "clipLibrary1.cel[0].cev[141].cevr";
connectAttr "animCurveTU321.a" "clipLibrary1.cel[0].cev[142].cevr";
connectAttr "animCurveTU322.a" "clipLibrary1.cel[0].cev[143].cevr";
connectAttr "animCurveTA706.a" "clipLibrary1.cel[0].cev[144].cevr";
connectAttr "animCurveTA707.a" "clipLibrary1.cel[0].cev[145].cevr";
connectAttr "animCurveTA708.a" "clipLibrary1.cel[0].cev[146].cevr";
connectAttr "animCurveTL739.a" "clipLibrary1.cel[0].cev[147].cevr";
connectAttr "animCurveTL740.a" "clipLibrary1.cel[0].cev[148].cevr";
connectAttr "animCurveTL741.a" "clipLibrary1.cel[0].cev[149].cevr";
connectAttr "animCurveTU323.a" "clipLibrary1.cel[0].cev[150].cevr";
connectAttr "animCurveTU324.a" "clipLibrary1.cel[0].cev[151].cevr";
connectAttr "animCurveTU325.a" "clipLibrary1.cel[0].cev[152].cevr";
connectAttr "animCurveTA709.a" "clipLibrary1.cel[0].cev[153].cevr";
connectAttr "animCurveTA710.a" "clipLibrary1.cel[0].cev[154].cevr";
connectAttr "animCurveTA711.a" "clipLibrary1.cel[0].cev[155].cevr";
connectAttr "animCurveTL742.a" "clipLibrary1.cel[0].cev[156].cevr";
connectAttr "animCurveTL743.a" "clipLibrary1.cel[0].cev[157].cevr";
connectAttr "animCurveTL744.a" "clipLibrary1.cel[0].cev[158].cevr";
connectAttr "animCurveTL745.a" "clipLibrary1.cel[0].cev[159].cevr";
connectAttr "animCurveTL746.a" "clipLibrary1.cel[0].cev[160].cevr";
connectAttr "animCurveTL747.a" "clipLibrary1.cel[0].cev[161].cevr";
connectAttr "animCurveTL748.a" "clipLibrary1.cel[0].cev[162].cevr";
connectAttr "animCurveTL749.a" "clipLibrary1.cel[0].cev[163].cevr";
connectAttr "animCurveTL750.a" "clipLibrary1.cel[0].cev[164].cevr";
connectAttr "animCurveTA712.a" "clipLibrary1.cel[0].cev[165].cevr";
connectAttr "animCurveTA713.a" "clipLibrary1.cel[0].cev[166].cevr";
connectAttr "animCurveTA714.a" "clipLibrary1.cel[0].cev[167].cevr";
connectAttr "animCurveTA715.a" "clipLibrary1.cel[0].cev[168].cevr";
connectAttr "animCurveTA716.a" "clipLibrary1.cel[0].cev[169].cevr";
connectAttr "animCurveTA717.a" "clipLibrary1.cel[0].cev[170].cevr";
connectAttr "animCurveTL751.a" "clipLibrary1.cel[0].cev[171].cevr";
connectAttr "animCurveTL752.a" "clipLibrary1.cel[0].cev[172].cevr";
connectAttr "animCurveTL753.a" "clipLibrary1.cel[0].cev[173].cevr";
connectAttr "animCurveTU326.a" "clipLibrary1.cel[0].cev[174].cevr";
connectAttr "animCurveTU327.a" "clipLibrary1.cel[0].cev[175].cevr";
connectAttr "animCurveTL754.a" "clipLibrary1.cel[0].cev[176].cevr";
connectAttr "animCurveTL755.a" "clipLibrary1.cel[0].cev[177].cevr";
connectAttr "animCurveTL756.a" "clipLibrary1.cel[0].cev[178].cevr";
connectAttr "animCurveTA718.a" "clipLibrary1.cel[0].cev[179].cevr";
connectAttr "animCurveTA719.a" "clipLibrary1.cel[0].cev[180].cevr";
connectAttr "animCurveTA720.a" "clipLibrary1.cel[0].cev[181].cevr";
connectAttr "animCurveTA721.a" "clipLibrary1.cel[0].cev[182].cevr";
connectAttr "animCurveTA722.a" "clipLibrary1.cel[0].cev[183].cevr";
connectAttr "animCurveTA723.a" "clipLibrary1.cel[0].cev[184].cevr";
connectAttr "animCurveTA724.a" "clipLibrary1.cel[0].cev[185].cevr";
connectAttr "animCurveTA725.a" "clipLibrary1.cel[0].cev[186].cevr";
connectAttr "animCurveTA726.a" "clipLibrary1.cel[0].cev[187].cevr";
connectAttr "animCurveTL757.a" "clipLibrary1.cel[0].cev[188].cevr";
connectAttr "animCurveTL758.a" "clipLibrary1.cel[0].cev[189].cevr";
connectAttr "animCurveTL759.a" "clipLibrary1.cel[0].cev[190].cevr";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[0].llnk";
connectAttr ":initialShadingGroup.msg" "lightLinker1.lnk[0].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[1].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[2].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[3].llnk";
connectAttr ":initialParticleSE.msg" "lightLinker1.lnk[3].olnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[4].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[5].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[6].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[7].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[8].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[9].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[10].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[11].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[12].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[13].llnk";
connectAttr ":defaultLightSet.msg" "lightLinker1.lnk[14].llnk";
connectAttr "lightLinker1.msg" ":lightList1.ln" -na;
// End of ReloadStand.ma
