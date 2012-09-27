<?php
thaps_enable();

class EnKlasse {

	public function World() {
		echo "Hello World\n";
	}

	public static function StaticWorld() {
		echo "Hello static World\n";
	}

}

$rap = "EnKlasse";
$hello = new $rap;
$hello->World();

$rap::StaticWorld();
