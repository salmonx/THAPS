<?php
thaps_enable();

function funcOne() {
	return 2*funcTwo();
}

function funcTwo() {
	return 3*funcTree();
}

function funcTree() {
	return 3;
}

$theFunction="funcOne";
echo "Result: " . $theFunction() . "\n";
