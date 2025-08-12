#ifndef FIELD_SERVICE_DB_DBHEADER_H
#define FIELD_SERVICE_DB_DBHEADER_H

// Download the libsodium library from https://libsodium.gitbook.io/doc/ and follow the installation instructions
#include <sodium.h>

#include <string>
#include <vector>
#include <cstdint>

// Download the MySQL Connector/C++ from https://dev.mysql.com/downloads/connector/cpp/ and follow the installation instructions
#include <mysql_driver.h>
#include <mysqlx/xdevapi.h>

// Here are the functions needed to be implemented

// Function to define tabular data structure for 
struct GeneralServiceLog {
    uint64_t id;                // Unique identifier for entries inside the service log
    std::string robot_id;            // Identifier for the robot (botNumber-year) Example: "434-2023"
    uint64_t serviceTicketTimestamp; // Timestamp of the service ticket creation
    std::string serviceTicketNumber; // Service ticket number. It is an alphanumeric string with the bot ID and 8 digit timestamp
    char severityCode;   // Status of the service (e.g., "active", "inactive")
    std::string diagnosticCode;
    std::string fixCode; // Code for the determined fix (200-00976)
    std::string created_at;     // Timestamp when the service was created
    std::string comments;          // Additional comments or notes
};

struct FleetServiceLog {
    uint64_t id;                 // Unique identifier for entries inside the fleet service log
    std::string robot_id;        // Identifier for the robot (botNumber-year) Example: "434-2023"
    char eventType;             // Type of event ('C' for campaign, 'S' for service, 'D' for deployment)
    std::string deploymentDate; // Date of deployment
    std::string address;         // Address of the service location
    char topModConfigCode;       // Code for the top module configuration (A is 1 shelf, B is 2 shelves, C is 3 shelves and so on)
    char airTagCode;            // Code for the AirTag used (A is 1, B is 2, C is 3 and so on)
    std::string mainGroup;        // Main group of the problem area (e.g., "Top Module", "Platform","Preventative Maintenance", "Battery", "Top Module Platform")
    std::string subset;           // Subset of the problem area (e.g., "Handle", "Firmware", "SLA System", "Battery" "E-Stops", "Blue Safety Light", "RGo", "Motor", Safety System", "HMI", "PM")
    std::string conditionObserved; // Condition observed during the service
    std::string serviceRequestObjective; // Objective of the service request
    char completionStatus;    // Status of the service completion (e.g., 'C' for completed, 'P' for pending or in progress, 'S' for scheduled)
    char usabilityStatus;    // Usability status of the robot after service (e.g., 'I' for in service, 'O' for out of service)
    std::string comments;          // Additional comments or notes
    uint64_t elapsedTime;         // Elapsed time for the service in minutes
    std::string toolsUsed;          // Tools used during the service
    uint64_t serviceTicketTimestamp; // Timestamp of the service ticket creation
};


// Function for generating a cursor object
mysqlx::RowResult generateCursor(mysqlx::Session& session, const std::string& query);





#endif