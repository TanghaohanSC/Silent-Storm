var arTitle=Array('img/title1.gif','img/title2.gif','img/title3.gif','img/title4.gif','img/title5.gif','img/title6.gif','img/title7.gif','img/title8.gif','img/title9.gif','img/title10.gif','img/title11.gif');
var arAlt=Array('Prologue','System requirements','Installing and launching the game','Quick start','Game screens, options','Base and character screens','Campaign screens','Combat screen','RPG and combat systems','Credits','Controls');
var arLinks=Array('part1.html','part2.html','part3.html','part4.html','part5.html','part6.html','part7.html','part8.html','part9.html','part10.html','part11.html');



function openTree(_name){
closeTree();
document.all[_name].style.display="block";

}
function closeTree(){

document.all["m1"].style.display="none";
document.all["m2"].style.display="none";
document.all["m3"].style.display="none";
document.all["m4"].style.display="none";
document.all["m5"].style.display="none";
document.all["m6"].style.display="none";
document.all["m7"].style.display="none";
document.all["m8"].style.display="none";
}


	function newTitle(_name){
	var id=Math.round(_name);
	
parent.document.images.title.src=arTitle[id];
parent.document.images.title.alt=arAlt[id];


}

	
function loadDocument(docurl,frname){
	frames(frname).location.href=docurl;
}


function openWindow(url_, width, height) {
        var GloWin = window.open(url_,"text",'width=' + width + ',height=' + height + ',resizable=1,scrollbars=1,menubar=yes,toolbar=yes, status=yes' );
		GloWin.focus();
}

function changeImages() {
    if (document.images) {
	    for (var i=0; i<changeImages.arguments.length; i+=2) {
            document[changeImages.arguments[i]].src = changeImages.arguments[i+1];
        }
    }
}

function parentNavigate(url_) {
    window.opener.document.location=url_; 
    window.close();
}








