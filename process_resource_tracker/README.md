# Process Resource Tracker

## General Overview of the Project

This project aims to monitor and display the resource usage of system processes,
focusing on CPU, memory, and power consumption. Processes that originate from the
same application will be grouped together—for instance, all processes run by the
root user or all instances of VSCode will appear as unified categories.
The output will be presented in the terminal, showing both real-time usage and
cumulative statistics collected since the system booted.


## Core Features

### Intelligent Process Categorization
    - Group processes by:
    * Root processes (all UID 0 processes aggregated)
    * Application groups (all Firefox processes as "firefox")
    * Service categories (systemd, docker, database processes)
    * User sessions (group by user ID)
    * Custom regex patterns for custom grouping

    - Smart process naming:
    * Resolve binary paths to friendly names
    * Handle child processes intelligently
    * Detect containerized processes (Docker, LXC)

### Comprehensive Resource Tracking
    - CPU Usage:
    * Current percentage per category
    * Total CPU time since boot
    * Per-core utilization breakdown

    - Memory Consumption:
    * Current RSS (Resident Set Size)
    * Virtual memory usage
    * Shared memory accounting
    * Memory pressure indicators

    - Power Consumption (Advanced):
    * Estimated power usage per process category
    * RAPL-based energy tracking (Intel/AMD)
    * Battery impact estimation
    * Energy usage since boot

    - I/O Statistics:
    * Disk read/write operations
    * Network bandwidth usage
    * Total I/O since system start
    -Read CPU temperature, fan speeds

### Dual Timeframe Display
    - Real-time (Current):
    * Instant resource usage
    * Refreshing display (1-5 second intervals)
    * Peak usage indicators

    - Cumulative (Since Boot):
    * Total CPU time consumed
    * Aggregate memory hours
    * Total energy consumed
    * Lifetime I/O operations

