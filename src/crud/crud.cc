#include "crudheader.h"
#include <mysql/mysql.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>


// Helper to get sorted list of JSON files by timestamp
std::vector<std::filesystem::path> get_sorted_json_files(const std::string& dir) {
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".json") {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
        return a.filename().string() > b.filename().string();
    });
    return files;
}

// Helper to load JSON from file
nlohmann::json load_json(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Failed to open JSON file: " + path.string());
    nlohmann::json j;
    f >> j;
    return j;
}

// Helper to connect to MySQL
MYSQL* connect_mysql(const CrudConfig& config) {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) throw std::runtime_error("mysql_init failed");
    if (!mysql_real_connect(conn, config.host.c_str(), config.user.c_str(), config.password.c_str(),
                            config.db.c_str(), config.port, nullptr, 0)) {
        throw std::runtime_error(mysql_error(conn));
    }
    return conn;
}

// Compare two JSONs and return changes
struct ChangeSet {
    std::vector<nlohmann::json> to_create;
    std::vector<nlohmann::json> to_update;
    std::vector<nlohmann::json> to_delete;
};

ChangeSet jsonChangeFinder(const nlohmann::json& old_json, const nlohmann::json& new_json, const std::string& key_field) {
    ChangeSet changes;
    std::map<std::string, nlohmann::json> old_map, new_map;
    for (const auto& item : old_json) old_map[item[key_field]] = item;
    for (const auto& item : new_json) new_map[item[key_field]] = item;

    // Detect creates and updates
    for (const auto& pair : new_map) {
        const std::string& key = pair.first;
        const nlohmann::json& new_item = pair.second;
        if (old_map.find(key) == old_map.end()) {
            changes.to_create.push_back(new_item);
        } else if (old_map[key] != new_item) {
            changes.to_update.push_back(new_item);
        }
    }
    // Detect deletes
    for (const auto& pair : old_map) {
        const std::string& key = pair.first;
        const nlohmann::json& old_item = pair.second;
        if (new_map.find(key) == new_map.end()) {
            changes.to_delete.push_back(old_item);
        }
    }
    return changes;
}

// CRUD Implementation
void Crud::json_synchronizer(const CrudConfig& config, const std::string& table, const std::string& key_field) {
    try {
        // Wait asynchronously until at least 2 files are present
        while (true) {
            std::vector<std::filesystem::path> files = get_sorted_json_files("src/temp");
            if (files.size() >= 2) {
                // Proceed when enough files are available
                nlohmann::json old_json = load_json(files[1]);
                nlohmann::json new_json = load_json(files[0]);
                ChangeSet changes = jsonChangeFinder(old_json, new_json, key_field);

                MYSQL* conn = connect_mysql(config);

                // Create
                for (const auto& item : changes.to_create) {
                    std::string fields, values;
                    for (auto it = item.begin(); it != item.end(); ++it) {
                        if (!fields.empty()) { fields += ","; values += ","; }
                        fields += "`" + it.key() + "`";
                        values += "'" + it.value().dump() + "'";
                    }
                    std::string query = "INSERT INTO " + table + " (" + fields + ") VALUES (" + values + ")";
                    if (mysql_query(conn, query.c_str())) {
                        std::cerr << "Create Error: " << mysql_error(conn) << std::endl;
                        mysql_close(conn);
                        exit(EXIT_FAILURE);
                    }
                }

                // Update
                for (const auto& item : changes.to_update) {
                    std::string sets;
                    for (auto it = item.begin(); it != item.end(); ++it) {
                        if (it.key() == key_field) continue;
                        if (!sets.empty()) sets += ",";
                        sets += "`" + it.key() + "`='" + it.value().dump() + "'";
                    }
                    std::string query = "UPDATE " + table + " SET " + sets + " WHERE `" + key_field + "`='" + item[key_field].dump() + "'";
                    if (mysql_query(conn, query.c_str())) {
                        std::cerr << "Update Error: " << mysql_error(conn) << std::endl;
                        mysql_close(conn);
                        exit(EXIT_FAILURE);
                    }
                }

                // Delete
                for (const auto& item : changes.to_delete) {
                    std::string query = "DELETE FROM " + table + " WHERE `" + key_field + "`='" + item[key_field].dump() + "'";
                    if (mysql_query(conn, query.c_str())) {
                        std::cerr << "Delete Error: " << mysql_error(conn) << std::endl;
                        mysql_close(conn);
                        exit(EXIT_FAILURE);
                    }
                }

                mysql_close(conn);
                break; // Exit loop after processing
            }
            // Sleep for a short duration before checking again
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& ex) {
        std::cerr << "Fatal Error: " << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Example read method
nlohmann::json Crud::read_all(const CrudConfig& config, const std::string& table) {
    try {
        MYSQL* conn = connect_mysql(config);
        std::string query = "SELECT * FROM " + table;
        if (mysql_query(conn, query.c_str())) {
            std::cerr << "Read Error: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            exit(EXIT_FAILURE);
        }
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;
        unsigned int num_fields = mysql_num_fields(res);
        nlohmann::json result = nlohmann::json::array();
        while ((row = mysql_fetch_row(res))) {
            nlohmann::json obj;
            for (unsigned int i = 0; i < num_fields; ++i) {
                obj[mysql_fetch_fields(res)[i].name] = row[i] ? row[i] : "";
            }
            result.push_back(obj);
        }
        mysql_free_result(res);
        mysql_close(conn);
        return result;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal Error: " << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}