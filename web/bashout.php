<?php

$file = @fopen("bash.out", "r");

echo "<small>";
while (!feof($file)) {
  echo "".stream_get_line($file, 100, "\n")."<br />";
}
echo "</small>";

fclose($file);

?>
