# daemon_cli

Simple C++ CLI client that connects to a Unix domain socket for the `Daemon_Socket` daemon.

Build (Linux with gcc):

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j
```

Default socket path: `/var/run/Daemon_Socket`

Usage:

- One-shot send:

```bash
./daemon_cli --socket /var/run/Daemon_Socket --send "STATUS"
```

- Interactive:

```bash
./daemon_cli --socket /var/run/Daemon_Socket
# then type lines; use `exit` or `quit` to leave
```
