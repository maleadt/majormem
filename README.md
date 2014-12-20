majormem
========

A small tool to generate a sequence of word combinations according to the
[mnemonic major system](https://en.wikipedia.org/wiki/Mnemonic_major_system).

Currently does only a simple mapping, mapping the digits 0-9 onto respectively
s, t, n, m, r, l, d, k, f and p.


Compiling
---------

Basic compilation:
```
$ mkdir build
$ qmake $CHECKOUT_DIR
$ make
```

In order to use Clang, call qmake with option variables `QMAKE_CC` and
`QMAKE_CXX` set to respectively `clang` and `clang++`.

Usage
-----

* Call the binary with the sequence-to-remember on the command line, or enter it
  when prompted for.
* Dictionaries should be present in the current directory, have the file
  extension `dic`, and contain a single word per line.
