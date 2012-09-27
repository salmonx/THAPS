<?php
thaps_enable();

function hello_1() {
	echo "hello_1 called\n";
}

function hello_2() {
	echo "hello_2 called\n";
}

if (isset($argv[1]) && function_exists($argv[1]))
	$argv[1]();
