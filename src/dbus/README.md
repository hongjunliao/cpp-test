##tests for DBus

https://github.com/freedesktop/dbus
https://thebigdoc.readthedocs.io/en/latest/dbus/dbus.html

# send/recv signal
$./build/cpp-test send_signal

$./build/cpp-test listen_signal
Got Singal with value : Hello,world!

# method call

$./build/cpp-test method_call
[2022-11-04 17:38:55.720]/511881 ./build/cpp-test send_signal | listen_signal, listen_dbus | method_call
Got Reply: 1, 2010

$./build/cpp-test listen_dbus
[2022-11-04 17:37:20.333]/507782 ./build/cpp-test send_signal | listen_signal, listen_dbus | method_call
[2022-11-04 17:37:23.335]/507782 method_call: 'Hello, D-Bus'

# using xml 
$dbus-binding-tool --mode=glib-client --prefix=com_wei wei.xml > wei.h