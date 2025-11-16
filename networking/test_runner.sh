#!/bin/sh
(cd kernel && make)
# insert module
(cd kernel && sudo insmod filter.ko) || exit 1

# listen for connections on localhost, port 60000 (run in background)
netcat -l -p 60000 &

# wait for netcat to start listening
sleep 1

# connect to localhost, port 60000, starting a connection using local
# port number 600001;
echo "Should show up in filter." | netcat -q 2 127.0.0.1 60000

# look for filter message in dmesg output
echo "dmesg output:"

sudo dmesg | tail -n 10


# set filter IP address to 127.0.0.1
(cd user && make)

(cd user && sudo ./test 127.0.0.1)

# listen for connections on localhost, port 60000 (run in background)
netcat -l -p 60000 &

# wait for netcat to start listening
sleep 1

# connect to localhost, port 60000, starting a connection using local
# port number 600001;
echo "Should show up in filter." | netcat -q 2 127.0.0.1 60000

# set filter IP address to 127.0.0.2

(cd user && sudo ./test 127.0.0.2)

# listen for connections on localhost, port 60000 (run in background)
netcat -l -p 60000 &

# wait for netcat to start listening
sleep 1

# connect to localhost, port 60000, starting a connection using local
# port number 600001;
echo "Should NOT show up in filter." | netcat -q 2 127.0.0.1 60000

# look for filter message in dmesg output
echo "dmesg output:"
sudo dmesg | tail -n 10

# remove module
sudo rmmod filter || exit 1

(cd user && make clean)
(cd kernel && make clean)