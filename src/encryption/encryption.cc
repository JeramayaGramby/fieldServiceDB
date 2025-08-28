#include <filesystem>
#include <fstream>
#include <vector>
#include <stdexcept>

// Directory for sensitive data
const std::string SENSITIVE_DIR = "src/temp/.2MX6u6bzBp!$";
const std::string KEY_FILE = SENSITIVE_DIR + "/hashkey.bin";

// Ensure sensitive directory exists
void ensureSensitiveDir() {
    std::filesystem::create_directories(SENSITIVE_DIR);
}

// Save key to file in sensitive directory
void saveKey(const std::vector<unsigned char>& key) {
    ensureSensitiveDir();
    std::ofstream file(KEY_FILE, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot write key file");
    file.write(reinterpret_cast<const char*>(key.data()), key.size());
}

// Load key from file in sensitive directory
std::vector<unsigned char> loadKey() {
    ensureSensitiveDir();
    std::ifstream file(KEY_FILE, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open key file");
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());
}

// Generate and save a random key for hashing
std::vector<unsigned char> generateAndStoreKey() {
    std::vector<unsigned char> key(HASH_KEY_SIZE);
    randombytes_buf(key.data(), HASH_KEY_SIZE);
    saveKey(key);
    return key;
}
