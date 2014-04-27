codeunsign
==========

A Mach-O binary codesign remover.

[Project Site](https://github.com/x43x61x69/codeunsign)


Description
-----------

The source code served as an example of how LC_CODE_SIGNATURE can be 
used, it is, by no means, a commercial grade product. It might contain 
errors or flaws, and it was created for demonstration purpose only.

The following architectures are supported by codeunsign:

* i386
* x86_64
* Fat (i386 + x86_64)

*PowerPC (PPC) is not support by this tool.*

To compile:

`clang codeunsign.c -O2 -o codeunsign`


Changelog
---------

v0.1:
* Initial release.


License
-------

Copyright (C) 2014  Cai, Zhi-Wei.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
