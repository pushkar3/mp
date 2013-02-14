<?php

if($_GET['order'] == "score") {
    exec("palletviewer/score.sh /var/www/bash.out");
    echo "<small><b>Scoring Complete.</b></small><br /><br />";
}
else {
    exec("mp/run.sh plan /var/www/mp/data/".$_GET['order']." /var/www/bash.out");
    echo "<small><b>Execution Complete.</b></small><br /><br />";
}

# include_once "bashout.php";

?>
