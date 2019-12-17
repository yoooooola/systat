# smapshot

smapshot is a program monitoring memory usage and finding memory leak

## How to use

### monitoring
$gcc memoryUse.c -o memoryUse

$./memoryUse

$./memoryUse -p bash

$./memoryUse -p bash -m

### memory leak
$ gcc leak.c -lpthread -o leak

$ gcc make_leak.c -o make_leak

$ mkdir ps_file

$./leak -p

$./leak -c
