xydiff
======

## Table of Contents

* [About](#about)
* [Requirements](#requirements)
* [Installation](#installation)
  * [Building from source](#source-installation)
  * [Building with autotools](#autotools-installation)
  * [Building on Windows](#win-build)
* [License](#license)
* [Authors](#authors)

About
-----

XyDiff is a set of tools for creating and applying deltas from XML documents (similar to `diff` / `patch`). It was originally developed by the [GEMO team](http://www.inria.fr/en/teams/gemo) at [INRIA-Rocquencourt](https://www.inria.fr/centre/paris-rocquencourt), near Paris. It was forked by [Frankie Dintino](http://frankiedintino.com/) to add features and to support more recent versions of gcc, clang, and Xerces-C++.

The algorithm used in XyDiff is described in the following research papers:

* [Change-Centric Management of Versions in an XML Warehouse](http://www.vldb.org/conf/2001/P581.pdf).
  A Marian, S Abiteboul, G Cobéna, L Mignet. *VLDB 2001*.
* [Detecting changes in XML documents](http://gregory.cobena.free.fr/www/Publications/%5BICDE2002%5D%20XyDiff%20-%20published%20version.pdf).
  G Cobéna, S Abiteboul, A Marian. *ICDE 2002 (San Jose)*.

XyDiff includes two command-line applications: `xydiff`, a program which compares two XML files and creates a “delta” that describes the differences between the two documents, and `xydelta`, which can apply that delta to the first document in order to reconstruct the second. It also includes a shared library, which is used by [php_xydiff](https://github.com/fdintino/php_xydiff).

Requirements
------------

xydiff requires that Xerces-C++ 3.x be installed. Xerces-C++ can be built
[from source](http://xerces.apache.org/xerces-c/download.cgi), or it can be
installed with the binaries and headers for your platform:

* **Ubuntu / Debian:**
  <pre>apt-get install libxerces-c-dev</pre>
* **RedHat / CentOS / Fedora**:
  <pre>yum install xerces-c-devel</pre>
* **Mac OS X ([Homebrew](http://mxcl.github.com/homebrew/))**
  <pre>brew install xerces-c</pre>

Installation
-------------

<a name="source-installation"></a>
### Building from source

1. Download [xydiff-3.0.0.tar.gz](https://github.com/fdintino/xydiff/releases/download/v3.0.0/xydiff-3.0.0.tar.gz),
   extract it, and cd into the directory `xydiff-3.0.0`.

2. Run a typical configure / make. If Xerces-C++ was installed into an unusual
   location, you may need to pass its prefix to `configure` using, for
   example, `--with-xercesc=/opt/xerces`.
   
        ./configure
        make
        sudo make install
        sudo ldconfig

<a name="autotools-installation"></a>
### Building with autotools (Linux / Unix / BSD / Mac OS X)

 1. Clone the xydiff repository:

        git clone https://github.com/fdintino/xydiff

 2. Change directory to where you cloned the repository and generate the
    files from automake (≥ 1.10), autoconf (≥ 2.61) and m4 (≥ 1.4.6):

        ./autogen.sh

 3. Follow step (2) above in "Building from source"

<a name="win-build"></a>
### Building on windows

 1. Install Visual Studio (windows builds have only been tested on VC9).
 2. Install the Microsoft Windows SDK for your OS if it is not already installed.
    Restart after installing.
 3. Create a working directory for the code (denoted below by `%XYDIFF_DIR%`).
 4. Download the Xerces-C++ binaries. For Visual Studio 2008 (VC9) and Visual
    Studio 2010 (VC10), install the Xerces-C++ binaries:
    ([VC9](http://archive.apache.org/dist/xerces/c/3/binaries/xerces-c-3.1.1-x86_64-windows-vc-9.0.zip),
    [VC10](http://archive.apache.org/dist/xerces/c/3/binaries/xerces-c-3.1.1-x86_64-windows-vc-10.0.zip)).
    For Visual Studio 2012, you will need to download Xerces-C++
    [from source](http://xerces.apache.org/xerces-c/download.cgi)
    and build the bundled VC11 solution file. Extract to `%XYDIFF_DIR%`.
 5. Check out xydiff into `%XYDIFF_DIR%\xydiff`:
    <pre lang="bash">git clone https://github.com/fdintino/xydiff</pre>
 6. Open `%XYDIFF_DIR%\xydiff\vc9.0\xydiff.sln`. Choose "Release" as the active
    configuration, and build the solution

License
-------

This code is licensed under the Lesser GPL. See LICENSE.TXT for the full license.

Authors
-------

* Grégory Cobena
* Serge Abiteboul
* Amélie Marian
* Frankie Dintino
