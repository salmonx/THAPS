<?php
thaps_enable();

function enFunktion($arg) {
	if ($arg != NULL)
		echo $arg."\n";
}

enFunktion("Et argument");
enFunktion(null);
