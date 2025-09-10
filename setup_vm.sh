#!/bin/bash
set -e
IMG="lkm-dev.qcow2"
SIZE="10G"
ISO="ubuntu-24.04.3-live-server-amd64.iso"
HTTP_DIR="./ci"
HTTP_PORT=8000

qemu-img create -f qcow2 "$IMG" "$SIZE"
mkdir -p "$HTTP_DIR"

cat > "$HTTP_DIR/user-data" << 'EOF'
#cloud-config
autoinstall:
  version: 1
  identity:
    hostname: lkm-vm
    username: ubuntu
    password: sacp
  ssh:
    install-server: true
    allow-pw: true
  packages:
    - build-essential
    - kmod
    - curl
    - nano
  late-commands:
    - curtin in-target --target=/target -- apt update
    - curtin in-target --target=/target -- apt install -y linux-headers-generic
EOF

touch "$HTTP_DIR/meta-data"

(cd "$HTTP_DIR" && python3 -m http.server "$HTTP_PORT") &
HTTP_PID=$!

qemu-system-x86_64 \
  -enable-kvm -m 4G -smp 2 \
  -boot menu=on \
  -cdrom "$ISO" \
  -drive file="$IMG",format=qcow2,if=virtio \
  -netdev user,id=net0 -device virtio-net-pci,netdev=net0 \
  -nographic -serial mon:stdio

kill "$HTTP_PID"
