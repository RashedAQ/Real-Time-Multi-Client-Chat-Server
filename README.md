# Real-Time Multi-Client Chat Server 🚀

A robust, feature-rich chat server implementation in C that demonstrates advanced networking concepts, multi-threading, and data integrity protocols. This project showcases real-time communication capabilities with comprehensive error detection and logging systems.

## 📋 Table of Contents

- [Features](#-features)
- [Technical Overview](#-technical-overview)
- [Installation](#-installation)
- [Usage](#-usage)
- [Project Structure](#-project-structure)
- [API Reference](#-api-reference)
- [Data Integrity](#-data-integrity)
- [Contributing](#-contributing)
- [License](#-license)

## ✨ Features

### Core Functionality
- **Real-time Communication**: Multi-client chat using TCP sockets
- **Concurrent Processing**: Multi-threaded architecture for handling multiple clients simultaneously
- **Private Messaging**: Direct message functionality between users
- **Session Management**: Unique session IDs with comprehensive logging
- **User Management**: Dynamic username changes and client listing

### Data Integrity & Security
- **Parity Checking**: Validates even/odd parity of received data
- **CRC32 Validation**: Cyclic redundancy check for data integrity
- **Error Simulation**: Random bit corruption for testing error handling
- **Session Logging**: Complete audit trail of all communications

### Advanced Features
- **Multi-threading**: Each client runs in its own thread for optimal performance
- **Error Detection**: Comprehensive error detection and handling mechanisms
- **Logging System**: Session-based log files with timestamps
- **Connection Management**: Graceful handling of client connections and disconnections

## 🔧 Technical Overview

### Architecture
The application follows a client-server architecture with the following components:

- **Server**: Central hub that manages all client connections
- **Clients**: Individual chat participants connected to the server
- **Threading**: Each client connection is handled by a separate thread
- **Logging**: Session-based logging for audit and debugging purposes

### Key Technologies
- **Language**: C (ANSI C)
- **Networking**: TCP/IP sockets
- **Concurrency**: POSIX threads (pthread)
- **Error Detection**: Parity bits and CRC32 checksums
- **File I/O**: Session logging and persistence

## 🚀 Installation

### Prerequisites
- GCC compiler (version 4.8 or higher)
- POSIX-compliant system (Linux/Unix/macOS)
- pthread library (usually included with GCC)

### Compilation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/real-time-chat-server.git
   cd real-time-chat-server
   ```

2. **Compile the server:**
   ```bash
   gcc -o server server.c -lpthread
   ```

3. **Compile the client:**
   ```bash
   gcc -o client client.c -lpthread
   ```

### For Windows Users
If you're using Windows with MinGW or similar:

```bash
gcc -o server server.c -lws2_32 -lpthread
gcc -o client client.c -lws2_32 -lpthread
```

## 📖 Usage

### Starting the Server

1. **Run the server on a specific port:**
   ```bash
   ./server 12121
   ```

2. **Server will display:**
   ```
   Server started on port 12121
   ```

### Connecting Clients

1. **Start the first client:**
   ```bash
   ./client 127.0.0.1 12121 alice
   ```

2. **Start additional clients in separate terminals:**
   ```bash
   ./client 127.0.0.1 12121 bob
   ./client 127.0.0.1 12121 charlie
   ```

3. **Each client will receive a welcome message:**
   ```
   Welcome to the server, alice!
   ```

### Available Commands

| Command | Description | Example |
|---------|-------------|---------|
| `list` | Show all connected clients | `list` |
| `MESG <username> <message>` | Send private message | `MESG bob Hello there!` |
| `/CONN <new_username>` | Change your username | `/CONN alice_new` |
| `GONE` | Disconnect from server | `GONE` |
| Any other text | Broadcast message to all clients | `Hello everyone!` |

### Example Session

```
Server: Welcome to the server, alice!
Server: New client connected: bob
alice: Hello everyone!
bob: Hi alice!
alice: MESG bob How are you?
[Private]alice: How are you?
bob: I'm doing great, thanks!
alice: list
Connected Clients
alice
bob
```

## 📁 Project Structure

```
Real-Time Multi-Client Chat Server/
├── README.md                 # This file
├── server.c                  # Main server implementation
├── client.c                  # Client implementation
├── startcmd.txt             # Quick start commands
├── server.exe               # Compiled server (Windows)
├── client.exe               # Compiled client (Windows)
└── logs/                    # Session log files
    ├── session_1704585600.log
    ├── session_1704585800.log
    └── ...
```

## 🔍 API Reference

### Server Functions

#### `void *client_socket_reader(void *usernr)`
Main thread function for handling individual client connections.

#### `unsigned long crc32(const unsigned char *data, unsigned int len)`
Calculates CRC32 checksum for data integrity validation.

#### `int countOnes(unsigned char byte)`
Counts the number of set bits in a byte for parity checking.

#### `void write_log(const char *username, const char *message)`
Writes timestamped messages to session log files.

### Client Functions

#### `int rndCorrupt(char clientname[], char *buffer)`
Simulates random message corruption for testing error handling.

#### `void generate_session_id(char *session_id, size_t session_id_size)`
Generates unique session identifiers for logging.

## 🛡️ Data Integrity

### Parity Checking
The system implements parity checking to detect transmission errors:

```c
int onesCount = 0;
for (int i = 0; i < n; ++i) {
    onesCount += countOnes(buffer[i]);
}

if (onesCount % 2 == 0) {
    printf("Data received with even parity\n");
} else {
    printf("Data received with odd parity. Possible error!\n");
}
```

### CRC32 Validation
Cyclic Redundancy Check for enhanced error detection:

```c
unsigned long crc = crc32((unsigned char *)buffer, n);
printf("Received Cyclic Redundancy Check: %lu\n", crc);
```

### Error Simulation
The client includes random message corruption for testing:

```c
int should_corrupt = rand() % 2; // 50% chance of corruption
if (should_corrupt) {
    // Flip random bits in the message
    for (int i = 0; i < num_bytes; i++) {
        int random_bit = rand() % 8;
        buffer[i] ^= (1 << random_bit);
    }
}
```

## 📊 Logging System

### Session Logs
Each client session creates a separate log file with format:
```
logs/session_<timestamp>.log
```

### Log Format
```
[HH:MM:SS] username: message
[14:30:25] alice: connected
[14:30:30] alice: Hello everyone!
[14:30:35] alice: disconnected
```

### Log Management
- Logs are automatically created when clients connect
- Each session has its own log file
- Timestamps are in local time
- Logs persist across server restarts

## 🧪 Testing

### Manual Testing
1. Start the server
2. Connect multiple clients
3. Test various commands
4. Verify error detection
5. Check log files

### Error Testing
The client includes built-in error simulation:
- Random message corruption
- Parity checking validation
- CRC32 verification

## 🤝 Contributing

We welcome contributions! Please follow these steps:

1. **Fork the repository**
2. **Create a feature branch:**
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. **Commit your changes:**
   ```bash
   git commit -m 'Add amazing feature'
   ```
4. **Push to the branch:**
   ```bash
   git push origin feature/amazing-feature
   ```
5. **Open a Pull Request**

### Contribution Guidelines
- Follow existing code style
- Add comments for complex logic
- Test your changes thoroughly
- Update documentation if needed

## 🐛 Known Issues

- Limited to 10 concurrent clients (configurable in `MAX_CLIENTS`)
- Username length limited to 15 characters
- No encryption for message transmission
- Windows compatibility requires additional libraries

## 🔮 Future Enhancements

- [ ] Message encryption
- [ ] File transfer capabilities
- [ ] Web-based client interface
- [ ] Database integration for persistent user data
- [ ] Message history and search
- [ ] User authentication system
- [ ] Room-based chat channels
- [ ] Message delivery confirmation

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Your Name**
- GitHub: [@yourusername](https://github.com/yourusername)
- LinkedIn: [Your LinkedIn](https://linkedin.com/in/yourprofile)

## 🙏 Acknowledgments

- POSIX threads documentation
- TCP/IP socket programming guides
- CRC32 implementation references
- Error detection and correction literature

---

⭐ **Star this repository if you found it helpful!**

For questions or support, please open an issue on GitHub.
