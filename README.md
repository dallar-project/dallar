Dallar Wallet Core Codebase
=================================

Dallar Core is forked from Groestlcoin's Core, because we had to start somewhere.

Only major changes are related to the chain parameters and a new MSVC build toolchain.

This codebase hasn't been cleaned up since the fork yet, so there are references to Bitcoin/Groestlcoin/other coins when there shouldn't be, just as there are references to Dallar where there should be Bitcoin/Groestlcoin references, mainly in regards to copyright.

Build instructions are also behind and lacking.

Fixing these issues up will be a priority once more Dallar systems are stable.


What is Dallar?
-----------------

Its Dallar.

License
-------

Dallar Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see http://opensource.org/licenses/MIT.

Development Process
-------------------

@TODO

Testing
-------

@TODO

Development tips and tricks
---------------------------

**compiling for debugging**

Run configure with the --enable-debug option, then make. Or run configure with
CXXFLAGS="-g -ggdb -O0" or whatever debug flags you need.

**debug.log**

If the code is behaving strangely, take a look in the debug.log file in the data directory;
error and debugging message are written there.

The -debug=... command-line option controls debugging; running with just -debug will turn
on all categories (and give you a very large debug.log file).

The Qt code routes qDebug() output to debug.log under category "qt": run with -debug=qt
to see it.

**testnet and regtest modes** @TODO

Run with the -testnet option to run with "play dallars" on the test network, if you
are testing multi-machine code that needs to operate across the internet.

If you are testing something that can run on one machine, run with the -regtest option.
In regression test mode blocks can be created on-demand; see qa/rpc-tests/ for tests
that run in -regest mode.

**DEBUG_LOCKORDER**

Dallar Core is a multithreaded application, and deadlocks or other multithreading bugs
can be very difficult to track down. Compiling with -DDEBUG_LOCKORDER (configure
CXXFLAGS="-DDEBUG_LOCKORDER -g") inserts run-time checks to keep track of what locks
are held, and adds warning to the debug.log file if inconsistencies are detected.
