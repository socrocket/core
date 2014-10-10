$( document ).ready(function() {

	//$("div.headertitle").addClass("page-header");
	//$("div.title").addClass("h1");
	var mainpage = $('li.current > a[href="index.html"]').length != 0;
  var header = $('.header').addClass('container').removeClass('header');
  var contents = $('.contents').addClass('container').removeClass('contents');
	var breadcrumb = $('#nav-path > ul').addClass('breadcrumb');
	
  var mainnav = $('#navrow1').addClass("mainnav navbar navbar-default");
  var mainnav_ul = mainnav.html();
  mainnav.html(
    '<div class="container-fluid">' +
    '  <div class="navbar-header nav-justified">' +
    '    <button type="button" data-toggle="collapse" data-target="#mainnav-collapse" class="navbar-toggle collapsed">' +
    '      <span class="sr-only">Toggle navigation</span>' +
    '      <span class="icon-bar"></span>' +
    '      <span class="icon-bar"></span>' +
    '      <span class="icon-bar"></span>' +
    '    </button>' +
    '    <a href="/" class="navbar-brand">SoCRocket</a>' +
    '    <div id="mainnav-collapse" class="navbar-collapse collapse">' +
    mainnav_ul +
    '      <!--<input type="text" placeholder="Search" class="search-query span2"> <a href="" class="icon-search icon-white"></a>-->' +
    '    </div>' +
    '  </div>' +
    '</div>'
  );
  mainnav.find('ul').addClass('nav navbar-nav').removeClass('tablist');
  // Namespaces
  mainnav.find('li > a[href="namespaces.html"]').parent().replaceWith(
    '<li class="dropdown">' +
    '  <a href="#" class="dropdown-toggle" data-toggle="dropdown"><span>Namespaces <span class="caret"></span></span></a>' +
    '    <ul class="dropdown-menu" role="menu">' +
    '       <li><a href="namespaces.html">Namespace List</a></li>' +
    '       <li><a href="namespacemembers.html">Namespace Members</a></li>' +
    '     </ul>' +
    '   </li>'
  );
  // Classes
  mainnav.find('li > a[href="annotated.html"]').parent().replaceWith(
    '<li class="dropdown">' +
    '  <a href="#" class="dropdown-toggle" data-toggle="dropdown">Classes <span class="caret"></span></a>' +
    '    <ul class="dropdown-menu" role="menu">' +
    '       <li><a href="annotated.html">List</a></li>' +
    '       <li><a href="classes.html">Index</a></li>' +
    '       <li><a href="inherits.html">Hierarchy</a></li>' +
    '       <li><a href="functions.html">Members</a></li>' +
    '     </ul>' +
    '   </li>'
  );
  // Files
  mainnav.find('li > a[href="files.html"]').parent().replaceWith(
    '<li class="dropdown">' +
    '  <a href="#" class="dropdown-toggle" data-toggle="dropdown">Files <span class="caret"></span></a>' +
    '    <ul class="dropdown-menu" role="menu">' +
    '       <li><a href="files.html">List</a></li>' +
    '       <li><a href="globals.html">Members</a></li>' +
    '     </ul>' +
    '   </li>'
  );

  $('#navrow2').remove();
  var subnav = $('#navrow3').addClass("subnav navbar navbar-default");
  subnav.find('ul').addClass('nav navbar-nav').removeClass('tablist');
  //var selector = $('#navrow4').addClass("subnav navbar navbar-default");
  //subnav.find('ul').addClass('nav navbar-nav').removeClass('tablist');

	$('li > a[href="index.html"] > span').text("SoCRocket");
	$('img[src="ftv2ns.png"]').replaceWith('<span class="label label-danger">N</span> ');
	$('img[src="ftv2cl.png"]').replaceWith('<span class="label label-danger">C</span> ');
	
	//$("ul.tablist").addClass("nav nav-pills nav-justified");
	//$("ul.tablist").css("margin-top", "0.5em");
	//$("ul.tablist").css("margin-bottom", "0.5em");
	$("li.current").addClass("active");
	$("iframe").attr("scrolling", "yes");
  //$('#doc-content').addClass('container').attr('style', '').attr('id','');

  $('#nav-path').removeClass('navpath');
	
	$("table").addClass('table table-striped table-hover').removeClass('doxtable');
	$("div.ingroups").wrapInner("<small></small>");
	$("div.levels").css("margin", "0.5em");
	$("div.levels > span").addClass("btn btn-default btn-xs");
	$("div.levels > span").css("margin-right", "0.25em");
	
	$("div.summary > a").addClass("btn btn-default btn-xs");
	$(".fragment").addClass("well");
	$(".memitem").addClass("panel panel-default");
	$(".memproto").addClass("panel-heading");
	$(".memdoc").addClass("panel-body");
	$("span.mlabel").addClass("label label-info");
	
	$("[class^=memitem]").addClass("active");
	
	$("div.ah").addClass("btn btn-default");
	$("span.mlabels").addClass("pull-right");
	$("table.mlabels").css("width", "100%")
	$("td.mlabels-right").addClass("pull-right");

	$("div.ttc").addClass("panel panel-info");
	$("div.ttname").addClass("panel-heading");
	$("div.ttdef,div.ttdoc,div.ttdeci").addClass("panel-body");
	
	
});
