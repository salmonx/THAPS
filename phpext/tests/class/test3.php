<?php
thaps_enable();

function __autoload($class) {
	include strtolower($class).".php";
}

new TestClass();
