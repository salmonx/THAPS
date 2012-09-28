# Welcome to the THAPS project

This project consists of three parts:

* PHP Scanner
* PHP extension
* Java GUI crawler

## Requirements

 * PHP >= 5.3.0
 * MongoDB >= 0.4
 * PHP MongoDB extension >= 1.2.10

# THAPS parts

### PHP Scanner
The scanner is a standalone scanner written in PHP. Usage:

    php Main.php

### PHP extension
The PHP extension is written in C and can be compiled with:

    ./setup
    make
    make install

To clean up, simply put:

    ./clean

### Java GUI crawler
The GUI is a crawler written in Java. We use a modified version of crawler4j, in order to receive HTTP headers and cookies. To compile:

    ant -Darch=32 jar (for 32-bit)
    ant -Darch=64 jar (for 64-bit)
    
# Putting it all together

The PHP extension should be enabled in the php.ini file. See the bundled php-thaps.ini file for guidance.
The scanner and crawler uses a configuration.ini file, which is located in their respective folders.
They should all point at the same MongoDB instance.
The 'local.thaps' should be created in MongoDB, where the configured user has read and write access.

Good luck! :-)
