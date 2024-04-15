# Battleship Game Project

## Project Overview
The Battleship game is a classic board game where players take turns guessing the locations of their opponent's ships on a grid. In this version of the game, the opponent is a computer that randomly places ships and waits for the player's moves. The computer doesn't make moves, but only processes the player's moves. It's a game of strategy, deduction, and a bit of luck. 

### Key Features
- **TCP**: The game uses the TCP protocol for reliable communication between the client and the server.
- **Multiprocess**: The server uses multiple processes to handle multiple clients simultaneously, ensuring smooth gameplay.
- **Structured Programming**: The code is written using structured programming principles for better readability and maintainability.
- **MVC**: The game follows the Model-View-Controller (MVC) design pattern, separating the game logic, user interface, and control flow.

## Installation and Running

1. Clone the repository:
```bash
git clone https://github.com/aagavrish/battleship.git
cd battleship
```

2. Build the project:
```bash
make server_compile // for server
make client_compile // for client
```

3. Run the server:
```bash
./LaunchServer
```

4. Run the client:
```bash
./LaunchClient -h <host> -p <port> -n <username>
    - <host> is the server host address
    - <port> is the server port
    - <username]> is your username in the game
```


> **Note:** Server configuration is located in the `config.cfg` file.


## System and Utility Requirements

- **Operating System**: Compatible with Unix systems.
- **Compiler**: Requires a C compiler that supports the C standard (e.g., GCC, Clang).
- **Libraries**: Requires the standard C library and POSIX libraries for networking and threading (on Unix-based systems).
- **Doxygen**: For generating project documentation from annotated source code.
- **Git**: For version control management.

## Additional Information

### Documentation

Available on [aagavrish.github.io/battleship](https://aagavrish.github.io/battleship/)

### License

This project is licensed under the MIT License. See the LICENSE file for details.

### Contact

If you have any questions, issues, or suggestions, please feel free to open an issue in the repository or contact us on Telegram: `aagavrish`.