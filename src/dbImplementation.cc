// These imports are just commented out for now, but you can uncomment them once you run the build in ninja
#include <iostream>
#include <stdexcept>
/*
#include <mysql_driver.h>
#include <mysqlx/xdevapi.h>
#include <sodium.h>
*/
#include <string>
#include <vector>
#include <cstdint>
#include "dbHeader.h"
#include <ctime>


void databaseStateIdentifier(mysqlx::Session& session){
    try {
        // Check if the GeneralServiceLog table exists

        if (!session.getSchema("test").getTable("GeneralServiceLog").existsInDatabase()) {
            std::cout << "GeneralServiceLog table does not exist. Creating it..." << std::endl;
            
            std::string createTableQuery = "CREATE TABLE GeneralServiceLog ("
                                           "id BIGINT AUTO_INCREMENT PRIMARY KEY, "
                                           "robot_id VARCHAR(255), "
                                           "serviceTicketTimestamp BIGINT, "
                                           "serviceTicketNumber VARCHAR(255), "
                                           "severityCode CHAR(1), "
                                           "diagnosticCode VARCHAR(255), "
                                           "fixCode VARCHAR(255), "
                                           "created_at VARCHAR(20), "
                                           "comments TEXT)";
            session.sql(createTableQuery).execute();

        } else {
            std::cout << "GeneralServiceLog table currently exists." << std::endl;
        }
        if (!session.getSchema("test").getTable("FleetServiceLog").existsInDatabase()) {
            std::cout << "FleetServiceLog table does not exist. Creating it..." << std::endl;
            
            std::string createTableQuery = "CREATE TABLE FleetServiceLog ("
                                           "id BIGINT AUTO_INCREMENT PRIMARY KEY, "
                                           "robot_id VARCHAR(255), "
                                           "eventType CHAR(1), "
                                           "deploymentDate VARCHAR(20), "
                                           "address TEXT, "
                                           "topModConfigCode CHAR(1), "
                                           "airTagCode CHAR(1), "
                                           "mainGroup VARCHAR(255), "
                                           "subset VARCHAR(255), "
                                           "conditionObserved TEXT, "
                                           "serviceRequestObjective TEXT, "
                                           "completionStatus CHAR(1), "
                                           "usabilityStatus CHAR(1), "
                                           "comments TEXT, "
                                           "elapsedTime BIGINT, "
                                           "toolsUsed TEXT, "
                                           "serviceTicketTimestamp BIGINT)";
            session.sql(createTableQuery).execute();

        } else {
            std::cout << "FleetServiceLog table currently exists." << std::endl;
        }

    } catch (const mysqlx::Error &err) {
        std::cerr << "Error: " << err.what() << std::endl;
    } catch (const std::exception &ex) {
        std::cerr << "STD Exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred while fetching database version." << err.what() << std::endl;
    }
};