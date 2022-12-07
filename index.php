<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);


function getSunsetAfter(DateTime $time) {
    $tz = $time->getTimezone();
    $lat = 42.198585005444905;
    $lon = -71.43879697549325;

    // Make sure the default time zone is set correctly before calling date_sun_info.
    // date_sun_info will determine the date based on the timestamp adjusted to the default time zone.
    $default_tz = date_default_timezone_get();
    date_default_timezone_set($tz->getName());

    // Get the sunrise/sunset data.
    $timestamp = $time->getTimestamp();
    $data = date_sun_info($timestamp, $lat, $lon);

    // Make sure that sunset occurs after the provided time.
    while ($time->getTimestamp() > $data["sunset"]) {
        // Add one day to the timestamp.
        $timestamp = $timestamp + 86400;
        $data = date_sun_info($timestamp, $lat, $lon);
    }

    // Restore the default timezone back to the default.
    date_default_timezone_set($default_tz);

    $sunrise = new DateTime('@' . $data["sunrise"], $tz);
    $sunset = new DateTime('@' . $data["sunset"], $tz);
    return (object) ([ "sunrise" => $sunrise, "sunset" => $sunset ]);
}

function getNextEvent($now) {
    $data = getSunsetAfter($now);

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
