<?php
    // thing to render live sensor data as a bitmap
    require __DIR__ . '/vendor/autoload.php';
    use PhpMqtt\Client\MqttClient;

    // callback invoked when the message is received, this does the
    // work of generating the bitmap, then aborting the loop
    $on_message = function ($topic, $message, $retained, $matchedWildcards)
    {
       global $client ;

      // decode
      $hwater_data = explode(" ", $message) ;
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
       
      $client->interrupt() ; 
    } ;
       
    // establish connection to mqtt broker
    $server = "localhost";
    $port = 1883 ;
    $client_id = "php-hotwater" ;
    $client = new MqttClient($server,$port,$client_id);
    $client->connect();
    // subscribe to topic - holds raw data from the sensor
    $client->subscribe('hotwater/data',$on_message) ;
    // start the poll
    $client->loop(true);
    // interrupt from the callback gets us here
    $client->disconnect();
?>
