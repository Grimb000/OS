#ifndef COMMON_H
#define COMMON_H

#include <windows.h> // For DWORD, etc.

// Max length for employee name
const int NAME_MAX_LENGTH = 10;
// Max length for pipe names
const int PIPE_NAME_MAX_LENGTH = 80;

// Structure for employee data
struct Employee {
    int    num;              // Employee ID
    char   name[NAME_MAX_LENGTH]; // Employee name
    double hours;            // Hours worked
};

// Types of messages between client and server
enum MessageType {
    // Client to Server
    REQ_READ,                // Request to read a record
    REQ_MODIFY,              // Request to lock a record for modification
    MODIFIED_RECORD,         // Client sends modified record data
    RELEASE_LOCK_MODIFY,     // Client releases write lock after modification
    FINISH_READ,             // Client releases read interest
    CLIENT_EXITING,          // Client is shutting down

    // Server to Client
    DATA_RECORD,             // Server sends record data
    OPERATION_SUCCESS,       // Operation was successful
    OPERATION_FAIL_LOCKED,   // Operation failed because record is locked by another
    OPERATION_FAIL_NOT_FOUND,// Operation failed because record not found
    OPERATION_FAIL_OTHER     // Other general failure
};

// Message structure for client requests
struct RequestMessage {
    MessageType type;
    DWORD       client_pid; // Process ID of the client
    char        client_response_pipe_name[PIPE_NAME_MAX_LENGTH]; // Pipe name for server's response
    int         record_id;  // Employee ID for the operation
    Employee    emp_data;   // Employee data (used for MODIFIED_RECORD)
};

// Message structure for server responses
struct ResponseMessage {
    MessageType type;
    Employee    emp_data;        // Employee data (used for DATA_RECORD)
    char        error_message[100]; // Optional error message detail
};

// Name of the main pipe created by the server for clients to connect
const char SERVER_REQUEST_PIPE_NAME[] = "\\\\.\\pipe\\EmployeeServerRequestPipe";

#endif // COMMON_H