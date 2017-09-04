# Process Control #

By Ross Smith

* `#include "rs-core/process.hpp"`

## Contents ##

[TOC]

## Spawned process channels ##

These classes call `popen()` to create a child process on construction, with
its standard output connected to the channel. Closing or destroying the
channel calls `pclose()`.

### Class StreamProcess ###

* `class` **`StreamProcess`**`: public StreamChannel`
    * `explicit StreamProcess::`**`StreamProcess`**`(const U8string& cmd)`
    * `virtual StreamProcess::`**`~StreamProcess`**`() noexcept`
    * `int StreamProcess::`**`status`**`() const noexcept`

This runs a process whose output is treated as an undifferentiated stream of
bytes. The `status()` function returns the value returned by `pclose()`, or -1
if the process has not been closed yet.

### Class TextProcess ###

* `class` **`TextProcess`**`: public MessageChannel<U8string>`
    * `explicit TextProcess::`**`TextProcess`**`(const U8string& cmd)`
    * `virtual TextProcess::`**`~TextProcess`**`() noexcept`
    * `std::string TextProcess::`**`read_all`**`()`
    * `int TextProcess::`**`status`**`() const noexcept`

This runs a process whose output is treated as a text stream. Each call to
`read()` yields one line of text (if any is available), with the terminating
LF or CRLF stripped.
