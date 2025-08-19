Real-Time Multi-Client Chat Server ��
A robust, feature-rich chat server implementation in C that demonstrates advanced networking concepts, multi-threading, and data integrity protocols.
✨ Features
Real-time Communication: Multi-client chat using TCP sockets
Concurrent Processing: Multi-threaded architecture for handling multiple clients simultaneously
Data Integrity: Parity checking and CRC32 validation for error detection
Private Messaging: Direct message functionality between users
Session Management: Unique session IDs with comprehensive logging
Error Handling: Random message corruption simulation for testing
User Management: Dynamic username changes and client listing
🛠️ Technical Implementation
Core Components
Server (server.c): Main server implementation with client management
Client (client.c): Client-side implementation with error simulation
Logging System: Session-based log files with timestamps
Error Detection: Parity bits and CRC32 checksums
Key Functions
Apply
// Error detection and validation
unsigned long crc32(const unsigned char *data, unsigned int len);
int countOnes(unsigned char byte);

// Session management
void generate_session_id(char *session_id, size_t session_id_size);
void write_log(const char *username, const char *message);

// Multi-threaded client handling
void *client_socket_reader(void *usernr);
