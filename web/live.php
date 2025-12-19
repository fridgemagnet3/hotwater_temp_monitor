<?php

    // thing to render live sensor data as a bitmap
    // create socket to receive sensor readings
    $sock = socket_create(AF_INET, SOCK_DGRAM, 0) ;
    if ( !$sock )
      die("Failed to create socket") ;    
    if( !socket_bind($sock, "0.0.0.0" , 52004) )
      die("Failed to bind socket") ;
    // potential to collide with the logger that is running on the same machine
    // so allow multiple receivers on the same port
    if (!socket_set_option($sock, SOL_SOCKET, SO_REUSEPORT, 1))
      die("Failed to set socket option") ;
    // wait for the datagram to come in
    if ( !socket_recvfrom($sock, $buf, 512, 0, $remote_ip, $remote_port) )
      die("Failed to receive UDP data") ;
    // decode
    $hwater_data = explode(" ", $buf) ;
    $timestamp = $hwater_data[0] ;
    $tank_lower = $hwater_data[1] ;
    $tank_upper = $hwater_data[2] ;
        
    // start generating the bitmap...
    header("Content-type: image/png") ;
    // template image
    $im = imagecreatefrompng("cylinder.png") ;
    // draw on the temperatures
    $blue = imagecolorallocate($im,42,96,153);
    $font = "/usr/share/fonts/truetype/freefont/FreeSerif.ttf" ;
    imagettftext($im,18,0,400,278,$blue,$font, $tank_upper . " ºC") ;
    imagettftext($im,18,0,400,505,$blue,$font, $tank_lower . " ºC") ;
    // and last update time
    $updated = new DateTime();
    $updated->setTimestamp($timestamp) ;
    imagettftext($im,16,0,100,700,$blue,$font, "Last updated: " . $updated->format("D jS M g:i:s A")) ;
    // render it
    imagepng($im) ;
?>
