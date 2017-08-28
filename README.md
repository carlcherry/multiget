# multiget
Multiget is a sample program that will pull a file from the internet in chunks.  By default it will download 4MiB in 4 chunks.
There are options to download the chunks simultaneously, as well as modifying the number of bytes, the chunk size, and number of chunks.
Use the -h option to see the full list of options.

## Requirement

Multiget has currently been built and tested on Ubuntu 16.04 using gcc 5.4.0

It requires the following libraries:
boost_system
boost_program_options
boost_regex
boost_thread

## Build Information

Required libraries to build are:

gcc automake autoconf build-essential
libboost-dev libboost-system-dev libboost-program-options-dev libboost-regex-dev libboost-thread-dev

In order to build type the following from the command line:

./autogen.sh

./configure

make

## Usage

./multiget [OPTIONS] url

To see the full list of options use the -h command line argument

## Documentation

If you want to create documentation then do the following:

1. Install doxygen

2. Create a directory name "doc" in the root multiget directory

3. Run this command "doxygen doxygen.config".  Open up index.htlm from doc/html



