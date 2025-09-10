#!/bin/bash

# LKM Development VM Runner Script
# This script runs the LKM development VM and provides file transfer capabilities

set -e

# Configuration
VM_NAME="lkm-dev"
IMG_PATH="${VM_NAME}.qcow2"
RAM="2G"
CORES="2"
SSH_PORT="2222"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_menu() {
    echo -e "${BLUE}[MENU]${NC} $1"
}

check_vm_image() {
    if [ ! -f "$IMG_PATH" ]; then
        print_error "VM image $IMG_PATH not found!"
        print_error "Please run setup-lkm-vm.sh first to create the VM."
        exit 1
    fi
}

start_vm() {
    print_status "Starting LKM development VM..."
    print_status "SSH will be available on localhost:$SSH_PORT"
    print_status "Press Ctrl+Alt+G to release mouse/keyboard from QEMU"
    
    # Try with KVM acceleration first, fall back to TCG if not available
    qemu-system-x86_64 \
        -machine accel=kvm:tcg \
        -cpu host \
        -m "$RAM" \
        -smp "$CORES" \
        -hda "$IMG_PATH" \
        -netdev user,id=net0,hostfwd=tcp::${SSH_PORT}-:22 \
        -device e1000,netdev=net0 \
        -display gtk \
        -enable-kvm \
        -daemonize \
        -pidfile vm.pid 2>/dev/null || \
    qemu-system-x86_64 \
        -cpu max \
        -m "$RAM" \
        -smp "$CORES" \
        -hda "$IMG_PATH" \
        -netdev user,id=net0,hostfwd=tcp::${SSH_PORT}-:22 \
        -device e1000,netdev=net0 \
        -display gtk \
        -daemonize \
        -pidfile vm.pid
    
    print_status "VM started in background. Waiting for SSH to become available..."
    
    # Wait for SSH to be available
    local attempts=0
    local max_attempts=30
    
    while [ $attempts -lt $max_attempts ]; do
        if nc -z localhost $SSH_PORT 2>/dev/null; then
            print_status "SSH is now available!"
            break
        fi
        attempts=$((attempts + 1))
        echo -n "."
        sleep 2
    done
    
    if [ $attempts -eq $max_attempts ]; then
        print_warning "SSH connection timeout. VM might still be booting."
        print_warning "Try connecting manually: ssh -p $SSH_PORT username@localhost"
    fi
}

get_vm_credentials() {
    if [ -z "$VM_USER" ]; then
        read -p "Enter VM username: " VM_USER
    fi
}

test_ssh_connection() {
    print_status "Testing SSH connection..."
    if ssh -p $SSH_PORT -o ConnectTimeout=5 -o StrictHostKeyChecking=no ${VM_USER}@localhost "echo 'SSH connection successful'" 2>/dev/null; then
        print_status "SSH connection to VM established successfully"
        return 0
    else
        print_error "Could not establish SSH connection to VM"
        print_error "Make sure the VM is running and SSH service is active"
        return 1
    fi
}

copy_files_to_vm() {
    print_menu "File Transfer to VM"
    echo "Options:"
    echo "1. Copy current directory"
    echo "2. Copy specific file/directory"
    echo "3. Skip file transfer"
    
    read -p "Choose option (1-3): " choice
    
    case $choice in
        1)
            local current_dir=$(basename "$PWD")
            print_status "Copying current directory to VM:~/host-files/$current_dir"
            ssh -p $SSH_PORT ${VM_USER}@localhost "mkdir -p ~/host-files"
            scp -P $SSH_PORT -r . ${VM_USER}@localhost:~/host-files/$current_dir/
            print_status "Files copied successfully!"
            ;;
        2)
            read -p "Enter path to file/directory to copy: " source_path
            if [ ! -e "$source_path" ]; then
                print_error "Source path does not exist: $source_path"
                return 1
            fi
            
            local basename_path=$(basename "$source_path")
            read -p "Enter destination path on VM (default: ~/host-files/$basename_path): " dest_path
            dest_path=${dest_path:-~/host-files/$basename_path}
            
            print_status "Copying $source_path to VM:$dest_path"
            ssh -p $SSH_PORT ${VM_USER}@localhost "mkdir -p $(dirname $dest_path)"
            
            if [ -d "$source_path" ]; then
                scp -P $SSH_PORT -r "$source_path" ${VM_USER}@localhost:"$dest_path"
            else
                scp -P $SSH_PORT "$source_path" ${VM_USER}@localhost:"$dest_path"
            fi
            
            print_status "Files copied successfully!"
            ;;
        3)
            print_status "File transfer skipped"
            ;;
        *)
            print_error "Invalid choice"
            ;;
    esac
}

copy_files_from_vm() {
    print_menu "File Transfer from VM"
    
    read -p "Enter path on VM to copy from: " vm_path
    read -p "Enter local destination path (default: ./from-vm/): " local_path
    local_path=${local_path:-./from-vm/}
    
    mkdir -p "$local_path"
    
    print_status "Copying from VM:$vm_path to $local_path"
    scp -P $SSH_PORT -r ${VM_USER}@localhost:"$vm_path" "$local_path"
    print_status "Files copied successfully!"
}

show_vm_info() {
    print_status "VM Information:"
    echo "- Image: $IMG_PATH"
    echo "- SSH: localhost:$SSH_PORT"
    echo "- RAM: $RAM"
    echo "- CPU Cores: $CORES"
    
    if [ -f "vm.pid" ]; then
        local pid=$(cat vm.pid)
        echo "- Process ID: $pid"
        if ps -p $pid > /dev/null 2>&1; then
            echo "- Status: Running"
        else
            echo "- Status: Not running"
            rm vm.pid
        fi
    else
        echo "- Status: Unknown"
    fi
}

stop_vm() {
    if [ -f "vm.pid" ]; then
        local pid=$(cat vm.pid)
        if ps -p $pid > /dev/null 2>&1; then
            print_status "Stopping VM (PID: $pid)..."
            kill $pid
            rm vm.pid
            print_status "VM stopped"
        else
            print_warning "VM process not found"
            rm vm.pid
        fi
    else
        print_warning "VM PID file not found. VM might not be running or started manually."
    fi
}

interactive_menu() {
    while true; do
        echo
        print_menu "LKM Development VM Manager"
        echo "1. Connect to VM via SSH"
        echo "2. Copy files TO VM"
        echo "3. Copy files FROM VM"
        echo "4. Show VM info"
        echo "5. Stop VM"
        echo "6. Exit"
        
        read -p "Choose option (1-6): " choice
        
        case $choice in
            1)
                print_status "Connecting to VM..."
                ssh -p $SSH_PORT ${VM_USER}@localhost
                ;;
            2)
                copy_files_to_vm
                ;;
            3)
                copy_files_from_vm
                ;;
            4)
                show_vm_info
                ;;
            5)
                stop_vm
                ;;
            6)
                print_status "Goodbye!"
                exit 0
                ;;
            *)
                print_error "Invalid choice"
                ;;
        esac
    done
}

main() {
    echo "=== LKM Development VM Runner ==="
    echo
    
    check_vm_image
    
    # Check if VM is already running
    if [ -f "vm.pid" ]; then
        local pid=$(cat vm.pid)
        if ps -p $pid > /dev/null 2>&1; then
            print_warning "VM appears to be already running (PID: $pid)"
            read -p "Continue anyway? (y/N): " continue_choice
            if [[ ! $continue_choice =~ ^[Yy]$ ]]; then
                exit 0
            fi
        else
            rm vm.pid
        fi
    fi
    
    start_vm
    get_vm_credentials
    
    if test_ssh_connection; then
        read -p "Do you want to copy files from host to VM? (y/N): " copy_choice
        if [[ $copy_choice =~ ^[Yy]$ ]]; then
            copy_files_to_vm
        fi
        
        interactive_menu
    else
        print_error "Could not establish SSH connection. Check VM manually."
        exit 1
    fi
}

# Cleanup function for Ctrl+C
cleanup() {
    echo
    print_warning "Received interrupt signal"
    stop_vm
    exit 0
}

trap cleanup INT TERM

main "$@"