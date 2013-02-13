<!DOCTYPE html>
<html>
  <head>
    <title>Pallet Planner</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="bootstrap/css/bootstrap.min.css" rel="stylesheet" media="screen">
    <style type="text/css">
      body {
        padding-top: 60px;
        padding-bottom: 40px;
      }
      .sidebar-nav {
        padding: 9px 0;
      }

      @media (max-width: 980px) {
        /* Enable use of floated navbar text */
        .navbar-text.pull-right {
          float: none;
          padding-left: 5px;
          padding-right: 5px;
        }
      }
      
     .sidebar-nav-fixed {
     position:fixed;
     top:60px;
     width:21.97%;
     }
     @media (max-width: 767px) {
     .sidebar-nav-fixed {
         width:auto;
     }
     }

     @media (max-width: 979px) {
     .sidebar-nav-fixed {
         position:static;
         width: auto;
     }
     }
     
  </style>
  
    <script src="bootstrap/js/bootstrap.js"></script>
    <script src="bootstrap/js/bootstrap-alert.js"></script>
    <script src="bootstrap/js/bootstrap-modal.js"></script>
    <script src="jquery-1.9.1.js"></script>
    <script type="text/javascript" src="jquery.timer.js"></script>   
  </head>
  <body>
  
    <div class="navbar navbar-inverse navbar-fixed-top">
      <div class="navbar-inner">
        <div class="container-fluid">
          <button type="button" class="btn btn-navbar" data-toggle="collapse" data-target=".nav-collapse">
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </button>
          <a class="brand" href="pallet.php">Pallet Planner</a>
          <div class="nav-collapse collapse">
            <ul class="nav">
              <li class="active"><a href="pallet.php">Home</a></li>
              <li><a href="mailto:pushkar@cc.gatech.edu">Contact</a></li>
              <li><a href="#">Help</a></li>
            </ul>
          </div>
        </div>
      </div>
   </div>
   
   <div id="con" class="container-fluid">
      <div class="row-fluid">
        <div class="span3">
        <div class="sidebar-nav-fixed">
          <div class="well sidebar-nav">
            <ul class="nav nav-list">
              <li class="nav-header">Options</li>
              <li class="disabled"><a href="#">Upload Order</a></li>
              <li><a href="#" id="load_basic">Plan Pallet</a></li>
              <li><a href="palletviewer/index.html">View Pallet</a></li>
              <li><a href="palletviewer/packlist.xml">Download Packlist</a></li>
            </ul> 
           </div>
         <div id="result" class="functions"></div>
         <div id="status" class="functions"></div>
        </div>
        </div>
              
       <div class="span9">
       <div id="bashout" class="functions" style="font-family: monospace;"></div>
       <div id="footer" class="functions"></div>
       </div>
    
      </div> 
   </div>
  
    <script type="text/javascript">
    
        status_div = function() {}
        status_div.warning = function(message) {
            $('#status').html('<div class="alert"><a class="close" data-dismiss="alert">&times;</a><span>'+message+'</span></div>')
        }
    
        var timer = $.timer(function() {
            $("#bashout")
               .load("bashout.php")
               .scrollTop($("#bashout")[0].scrollHeight);
;
        });
        timer.set({ time : 0, autostart : false });

         $.ajaxSetup ({  
            cache: false  
         });
         
         var ajax_load = "<img src='load.gif' alt='Loading...' /> Running... <br />";  

         var loadUrl = "load.php";
         $("#load_basic").click(function() {
            timer.play();
            status_div.warning("Timer Started.");
            $("#result")
               .html(ajax_load)
               .load(loadUrl, null, function() {
                   //timer.stop();
                   status_div.warning("Timer Stopped.");
               });
         });
         
                             
    </script> 
        
  </body>
</html>
