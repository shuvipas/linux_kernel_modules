# Netfilter module
The netfilter module monitors outbound TCP connections by implementing a netfilter hook on the NF_INET_LOCAL_OUT chain. It initially detects and logs TCP connection initiation packets (SYN flag set, ACK flag cleared) by displaying their source IP address and port. The module supports destination based filtering through an ioctl interface, allowing users to specify a target IP address only packets destined for that address will be logged while others are ignored. The module handles network byte order conversion for ports and uses kernel-appropriate functions for IP address comparison and user-space data transfer.
## Bulid procces
run test_runner.sh 
