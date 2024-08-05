# Air Traffic Control System

## Description

This project simulates a complex Air Traffic Control System (ATCS) using C programming. The system manages up to 10 unique planes and handles 100 concurrent processes, ensuring seamless communication and synchronization between them.

## Features

- **POSIX-compliant C program** for efficient process management.
- **Handles up to 10 unique planes** and 100 concurrent processes.
- **Manages up to 10 runways** with diverse load capacities (1,000 to 15,000 kgs).
- **Optimizes resource allocation** for efficient boarding/loading and departure operations.
- **Implements a single message queue system** for real-time communication between air traffic controllers, planes, and airport processes.
- **Accurately tracks and documents** the journey of planes across 10 airports.
- **Maintains system integrity** during termination protocols.

## Technologies Used

- C programming language
- POSIX standard for process management and inter-process communication

## Getting Started

1. Clone the repository:
    ```sh
    git clone https://github.com/vedantagr17/AirTrafficControlSystem.git
    ```

2. Set up the environment:
    - Ensure you have a C compiler (e.g., GCC) installed.
    - Make sure your system supports POSIX standards.

3. Build and run the application:
    ```sh
    gcc -o atcs main.c -lpthread
    ./atcs
    ```

## Project Structure

```plaintext
src: Contains the core C code for the application.
include: Contains header files.
