<?php
thaps_enable();

function enfunktion() {
	echo "hej hej funktion her\n";
}

function cb($a, $b) {
	return ($a > $b);
}

$ar = array(2,3,1);
usort($ar, "cb");

$rap="enfunktion";
$rap(); // denne feiler ikke længere :-)

usort($ar, function ($a, $b) {
	return 1;
});

enfunktion();
