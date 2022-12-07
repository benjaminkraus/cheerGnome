<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

function querySunset(Datetime $date, float $lat, float $lon) {
    $data = date_sun_info($date->getTimestamp(), $lat, $lon);
    return (object) ([ "sunrise" => $data["sunrise"], "sunset" => $data["sunset"] ]);
}

function readCache($cacheFile) {
    $json = false;
    if (file_exists($cacheFile)) {
        $json = file_get_contents($cacheFile);
    }
    if ($json == false) {
        return false;
    }

    $data = json_decode($json, false);
    $sunrise = new DateTime($data->{'sunrise'});
    $sunset = new DateTime($data->{'sunset'});
    return (object) ([ "sunrise" => $sunrise, "sunset" => $sunset ]);
}

function writeCache($cacheFile, $data) {
    $sunriseStr = $data->{'sunrise'}->format("c");
    $sunsetStr = $data->{'sunset'}->format("c");
    $json = json_encode(["sunrise" => $sunriseStr, "sunset" => $sunsetStr]);
    file_put_contents($cacheFile, $json);
}

function getNextEvent($now) {
    $cacheFile = "sunsetdata.txt";
    $lat = 42.198585005444905;
    $lon = -71.43879697549325;
    $tz = $now->getTimeZone();
    $now = new DateTime("now", $tz);
    $today = new DateTime("today", $tz);

    // Check first to see if we have sunrise/sunset cached.
    $data = readCache($cacheFile);
    if ($data == false) {
        // No cache file, so read the data from the server.
        echo "Server<br>";
        $data = querySunset($today, $lat, $lon);
    } else {
        echo "Cache<br>";
    }
    if ($data == false) {
        // Failed to read cache or server.
        return false;
    }

    if ($data->{'sunset'} < $now) {
        // After sunset. Get tomorrow's data.
        echo "After sunset<br>";
        $tomorrow = new DateTime("tomorrow", $tz);
        $data = querySunset($tomorrow, $lat, $lon);
        if ($data == false) {
            // Failed to get new data from server.
            return false;
        }
    }
    writeCache($cacheFile, $data);

    if ($data->{'sunrise'} > $now) {
        // Early morning. Next event is to shut off LED at sunrise.
        return (object) ([ "LEDon" => true, "next" => $data->{'sunrise'} ]);
    } else {
        // Mid-day. Next event is to turn LED on at sunset.
        return (object) ([ "LEDon" => false, "next" => $data->{'sunset'} ]);
    }
}

$tz = new DateTimeZone('America/New_York');
$now = new DateTime("now", $tz);
$nextEvent = getNextEvent($now);

echo $nextEvent->{'next'}->getTimestamp() - $now->getTimeStamp();

//header('Content-Type: application/json; charset=utf-8');

?>
