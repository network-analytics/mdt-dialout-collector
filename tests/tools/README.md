# (External) ZMQ Poller

Standalone helper for poking at the daemon's ZMQ socket while it runs
(zmq delivery mode). Not wired into CMake or ctest — build manually:

```SHELL
cd <main-prj-folder>/tests/tools
g++ -O2 -Wall -pedantic zmq_pull.cc -o ../../bin/zmq_pull -lzmq
```

