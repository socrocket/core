$( document ).ready(function() {

	//$("div.headertitle").addClass("page-header");
	//$("div.title").addClass("h1");
	var mainpage = $('li.current > a[href="index.html"]').length != 0;
  var header = $('.header').addClass('container').removeClass('header');
  var header_text = header.find('.title').html();
	var breadcrumbs = $('#nav-path > ul').addClass('breadcrumb').detach();
  breadcrumbs = breadcrumbs.find('a');
  breadcrumbs.append(" | ")
  var contents = $('.contents').addClass('container').removeClass('contents');
	
  var mainnav = $('#navrow1').addClass("mainnav navbar navbar-default").removeClass('tabs');
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
  mainnav.find('li > a[href="index.html"]').parent().after(
    '<li><a href="usermanual.html">User Manual</a></li>' +
    '<li><a href="https://github.com/socrocket/core">GitHub</a></li>'
  );
  //Related pages
  mainnav.find('li > a[href="pages.html"] > span').html('All Documents');
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
  var subnav = $('#navrow3').addClass("subnav navbar navbar-default").removeClass('tabs2'); //.detach();
  subnav.find('ul').addClass('nav navbar-nav').removeClass('tablist');
  header.html(
    '<header class="jumbotron subhead">' +
    '  <div class="breadcrumbs"></div>' +
    '  <h1>' + header_text + '</h1>' +
    //'  <p class="lead">Feature rich and performant</p>' +
    //'  <div class="subnav navbar navbar-default">' +
    //'    <ul class="nav nav-pills">' +
    //subnav.text() +
    //'    </ul>' +
    //'  </div>' +
    '</header>'
  );
  header.find('.breadcrumbs').html(breadcrumbs.text()+header_text);
  //var selector = $('#navrow4').addClass("subnav navbar navbar-default");
  //subnav.find('ul').addClass('nav navbar-nav').removeClass('tablist');

  if(mainpage) {
    header.remove();
    
  } else {
    $('.masthead').remove();
    $('body').removeClass('mainpage');
    mainnav.addClass('navbar-fixed-top');
    //$('body').attr('data-spy', 'scroll').attr('data-target', '.subnav');
  } 

	$('li > a[href="index.html"] > span').text("Start");
	$('img[src="ftv2ns.png"]').replaceWith('<span class="label label-danger">N</span> ');
	$('img[src="ftv2cl.png"]').replaceWith('<span class="label label-danger">C</span> ');
	
	//$("ul.tablist").addClass("nav nav-pills nav-justified");
	//$("ul.tablist").css("margin-top", "0.5em");
	//$("ul.tablist").css("margin-bottom", "0.5em");
	$("li.current").addClass("active");
	$("iframe").attr("scrolling", "yes");
  //$('#doc-content').addClass('container').attr('style', '').attr('id','');

  $('#nav-path').removeClass('navpath');
	
	$("table.doxtable").addClass('table table-striped table-hover').removeClass('doxtable');
	$("div.ingroups").wrapInner("<small></small>");
	$("div.levels").css("margin", "0.5em");
	$("div.levels > span").addClass("btn btn-default btn-xs");
	$("div.levels > span").css("margin-right", "0.25em");
  $('dl.reflist').removeClass('reflist');
	
	$("div.summary > a").addClass("btn btn-default btn-xs");
	$(".fragment").addClass("well");
  $('table.memberdecls').css('width', '100%').removeClass('table table-striped table-hover');
	$("table.mlabels").css("width", "100%").removeClass('table table-striped table-hover');
	$("table.memname").removeClass('table table-striped table-hover');
	$(".memitem").addClass("panel panel-default").removeClass('memitem');
	$(".memproto").addClass("panel-heading").removeClass('memproto');
	$(".memdoc").addClass("panel-body").removeClass('memdoc');
	$("span.mlabel").addClass("label label-info");
	
	$("[class^=memitem]").addClass("active");
	
	$("div.ah").addClass("btn btn-default");
	$("span.mlabels").addClass("pull-right");
	$("td.mlabels-right").addClass("pull-right");

	$("div.ttc").addClass("panel panel-info");
	$("div.ttname").addClass("panel-heading");
	$("div.ttdef,div.ttdoc,div.ttdeci").addClass("panel-body");

  if(mainpage) {
    // fix sub nav on scroll
    var $win = $(window)
      , $nav = $('.mainnav')
      , navTop = $('.mainnav').length && $('.mainnav').offset().top - 0 // 40
      , isFixed = 0;
      
    processMainScroll();
    
    // hack sad times - holdover until rewrite for 2.1
    $nav.on('click', function () {
      if (!isFixed) setTimeout(function () {  $win.scrollTop($win.scrollTop() - 47) }, 10);
    })
    
    $win.on('scroll', processMainScroll);
    $('.mainnav').scrollspy();
    
    function processMainScroll() {
      var i, scrollTop = $win.scrollTop();
      if (scrollTop >= navTop && !isFixed) {
        isFixed = 1;
        $nav.addClass('navbar-fixed-top');
      } else if (scrollTop <= navTop && isFixed) {
        isFixed = 0;
        $nav.removeClass('navbar-fixed-top');
      }
    }
  } else {
    if(subnav.length != 0) {
      // fix sub nav on scroll
      var $win = $(window)
        , $nav = $('.subnav')
        , navTop = $('.subnav').length && $('.subnav').offset().top - 64 // 40
        , isFixed = 0

      processSubScroll()

      // hack sad times - holdover until rewrite for 2.1
      $nav.on('click', function () {
        if (!isFixed) setTimeout(function () {  $win.scrollTop($win.scrollTop() - 47) }, 10)
      })

      $win.on('scroll', processSubScroll)
      $('.subnav').scrollspy()

      function processSubScroll() {
        var i, scrollTop = $win.scrollTop()
        if (scrollTop >= navTop && !isFixed) {
          isFixed = 1;
          $nav.addClass('subnav-fixed');
          $('body').css('padding-top', '160px');
        } else if (scrollTop <= navTop && isFixed) {
          isFixed = 0;
          $nav.removeClass('subnav-fixed');
          $('body').css('padding-top', '84px');
        }
      }
    }
  }
	
  /*$(function() {
      $(".fragment").each(function(i,node) {
          var $node = $(node);
          $node.html("<pre><code>"+$node.text()+"</code></pre>");
          hljs.highlightBlock(node);
      });
  });	*/
});
