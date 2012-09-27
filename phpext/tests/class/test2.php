<?php
thaps_enable();

class Test {

	public function __call($name, $args) {
		echo "You called function: $name on me..\n";
	}
}

$test = new Test;
$test->hello();
