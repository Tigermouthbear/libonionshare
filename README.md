# libonionshare
A reimplementation of the [OnionShare](https://github.com/onionshare/onionshare/) library and cli, written in C. The goal of this is to eventually port OnionShare to a [flutter](https://flutter.dev/) app using [dart FFI](https://docs.flutter.dev/development/platform-integration/c-interop).

## This is a work in progress
Not all the features from the original are implemented yet. Here's the status of each OnionShare service:
- [ ] Send
- [ ] Receive
- [x] Chat
- [ ] Website

## Building
### Dependencies
- CMake
- OpenSSL

### Building library and cli
Currently only supports POSIX systems.
```
git clone --recurse-submodules https://github.com/Tigermouthbear/libonionshare.git
cd libonionshare

mkdir build && cd build
cmake ..
make
```

## Using the CLI
A tor process must be running with an open control port.
```
Options:
-s, Send files
-r, Receive files
-c, Chat with others
-w, Host a website
-t [title], Title of your anonymous service (optional)
-a [address], Address of tor control port to use (optional)
-p [port], Port of tor control port to use (optional)
```