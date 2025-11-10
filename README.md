# linux_kernel_modules


## How to build & run  
```bash
cd <example_folder>
make
sudo insmod example_module.ko
dmesg | tail
sudo rmmod example_module
dmesg | tail
make clean
