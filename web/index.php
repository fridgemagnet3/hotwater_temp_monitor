<?php

    // thing to render last set of readings as a bitmap
    // connect to db
    $mysqli = new mysqli("localhost", "hotwater", "mysql-password", "hotwater") ;
    if (mysqli_connect_errno()) 
    {
      printf("Connect failed: %s\n", mysqli_connect_error());
      exit();
    }
    // fetch last readings
    $query = "select Timestamp, TankUpper, TankLower FROM hotwater ORDER By Id DESC LIMIT 1" ;
    $result = $mysqli->query($query) ;
    $row = $result->fetch_assoc();
    // start generating the bitmap...
    header("Content-type: image/png") ;
    // template image
    $im = imagecreatefrompng("cylinder.png") ;
    // draw on the temperatures
    $blue = imagecolorallocate($im,42,96,153);
    $font = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf" ;
    imagettftext($im,18,0,400,278,$blue,$font, $row['TankUpper'] . " ºC") ;
    imagettftext($im,18,0,400,505,$blue,$font, $row['TankLower'] . " ºC") ;
    // and last update time
    $updated = DateTime::createFromFormat('Y-m-d H:i:s',$row['Timestamp']) ;
    imagettftext($im,16,0,100,700,$blue,$font, "Last updated: " . $updated->format("D jS M g:i:s A")) ;
    // render it
    imagepng($im) ;
?>
