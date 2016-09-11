# One eventloop per thread

### [server.cpp](./server.cpp)
Each worker thread has an eventloop(io_service), workers are designed for register
incoming I/O event which pass from manager thread, and the manager thread also has
an eventloop for accepting new connection.

Manager thread assign new I/O event to worker thread by invoking worker's interface
`void enqueue(SessionPtr session)`, then worker thread will register new event in
the same thread. Now that enqueue register new event and process event are both in
the same thread, we can avoid using mutex.

the following is a simple test with [client.cpp](./client.cpp):

**Environment**

|CPU|RAM|Kernel| Complier| Target|
|--|--|--|--|--|
|Intel Core i5-6300HQ CPU @ 4x 2.30GHz| 4GB x 2| x86_64 Linux 4.7.2-2-default| GCC 6.2.1| release |

**server**

```
#cat /proc/${pid}/status
Name:   server
Umask:  0022
State:  S (sleeping)
Tgid:   10068
Ngid:   0
Pid:    10068
PPid:   9820
TracerPid:      0
Uid:    1000    1000    1000    1000
Gid:    100     100     100     100
FDSize: 128
Groups: 100 477
NStgid: 10068
NSpid:  10068
NSpgid: 10068
NSsid:  9820
VmPeak:   377892 kB
VmSize:   312356 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:      1756 kB
VmRSS:      1756 kB
RssAnon:             164 kB
RssFile:            1592 kB
RssShmem:              0 kB
VmData:    33616 kB
VmStk:       136 kB
VmExe:        88 kB
VmLib:      4524 kB
VmPTE:        84 kB
VmPMD:        12 kB
VmSwap:        0 kB
HugetlbPages:          0 kB
Threads:        5
SigQ:   0/31258
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000000000
SigCgt: 0000000180004006
CapInh: 0000000000000000
CapPrm: 0000000000000000
CapEff: 0000000000000000
CapBnd: 0000003fffffffff
CapAmb: 0000000000000000
Seccomp:        0
Cpus_allowed:   ff
Cpus_allowed_list:      0-7
Mems_allowed:   00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000001
Mems_allowed_list:      0
voluntary_ctxt_switches:        5944
nonvoluntary_ctxt_switches:     1
```
maximum CPU usage never exceed 50%.

**client**
```
#time ./client
./client  1.92s user 3.20s system 194% cpu 2.624 total
```

### [server1.cpp](./server1.cpp)
There's only one eventloop, and all threads share the same loop, that's to say, accepting new connection and handling I/O event are happen in the same loop.
One thing to mention, we don't know which thread is currently accepting new connection or handling I/O event, it's managed by the operating system.

the following is a simple test with [client.cpp](./client.cpp):  

**Same environment**

**server1**
```
#cat /proc/${pid}/status
Name:   server1
Umask:  0022
State:  S (sleeping)
Tgid:   11659
Ngid:   0
Pid:    11659
PPid:   9820
TracerPid:      0
Uid:    1000    1000    1000    1000
Gid:    100     100     100     100
FDSize: 1024
Groups: 100 477
NStgid: 11659
NSpid:  11659
NSpgid: 11659
NSsid:  9820
VmPeak:   377880 kB
VmSize:   312344 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:      2508 kB
VmRSS:      2508 kB
RssAnon:             944 kB
RssFile:            1564 kB
RssShmem:              0 kB
VmData:    34224 kB
VmStk:       136 kB
VmExe:        76 kB
VmLib:      4524 kB
VmPTE:        80 kB
VmPMD:        16 kB
VmSwap:        0 kB
HugetlbPages:          0 kB
Threads:        5
SigQ:   0/31258
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000000000
SigCgt: 0000000180000000
CapInh: 0000000000000000
CapPrm: 0000000000000000
CapEff: 0000000000000000
CapBnd: 0000003fffffffff
CapAmb: 0000000000000000
Seccomp:        0
Cpus_allowed:   ff
Cpus_allowed_list:      0-7
Mems_allowed:   00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000001
Mems_allowed_list:      0
voluntary_ctxt_switches:        1
nonvoluntary_ctxt_switches:     1
```
maximum CPU usage is 95%, range from 90% to 95%, and when client crashed, it still consume 90%~95% CPU.

**client**

```
#time ./client
connect: Connection timed out
terminate called after throwing an instance of 'boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::system::system_error> >'
  what():  shutdown: Transport endpoint is not connected
[2]    11664 abort (core dumped)  ./client
./client  0.71s user 0.77s system 1% cpu 2:07.83 total
```

### Conclusion
Obviously, `server` is better than `server1`. Maybe this test is buggy, who care?
