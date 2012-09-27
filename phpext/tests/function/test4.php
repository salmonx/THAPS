<?php
thaps_enable();

function enFunk($arg1, $arg2, $arg3) {
	echo "$arg1 $arg2 $arg3\n";
}

function enAndenFunk() {
	echo "hej hej\n";
}

$f = "enAndenFunk";

$f();

if (isset($argv[1]) && function_exists($argv[1]))
	call_user_func_array($argv[1], array("hej", "med", "dig"));

call_user_func("enAndenFunk");
call_user_func("enFunk", "rap", "anders", "and");


class TestClass {

	public function func() {
		echo "TestClass->func() executed\n";
	}

	public static function sFunc() {
		echo "TestClass::func() executed\n";
	}

}

call_user_func_array("TestClass::sFunc", array());

$t = new TestClass();
call_user_func_array(array($t, "func"), array());

call_user_func_array(array("TestClass", "sFunc"), array());
