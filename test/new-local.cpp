#include "crow.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <omp.h>

using namespace std;
using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

std::mutex data_mutex; // Mutex for thread-safe access to the global data container
vector<map<string, string>> globalData; // Global container to store loaded data

// Function to split a string by a delimiter
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Check if file has .csv extension
bool hasCSVExtension(const string& filename) {
    return filename.size() > 4 && filename.substr(filename.size() - 4) == ".csv";
}

// Process a single CSV file and add its data to the global container
void loadCSVFile(const string& filePath, const vector<string>& headers) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error: File " << filePath << " could not be opened" << endl;
        return;
    }

    string line;

    vector<map<string, string>> localData; // Thread-local container

    // Read rows and map them to headers
    while (getline(file, line)) {
        vector<string> row = split(line, ',');
        map<string, string> rowMap;
        for (size_t i = 0; i < headers.size(); i++) {
            if (i < row.size()) {
                rowMap[headers[i]] = row[i];
            }
        }
        localData.push_back(rowMap);
    }

    file.close();

    // Merge local data into the global container
    std::lock_guard<std::mutex> lock(data_mutex);
    globalData.insert(globalData.end(), localData.begin(), localData.end());
}

// Load all CSV files into memory
void loadAllCSVFiles(const string& directoryPath, const vector<string>& headers) {
    vector<string> filePaths;

    // Recursively get all file paths
    for (const auto& entry : recursive_directory_iterator(directoryPath)) {
        if (hasCSVExtension(entry.path().string())) {
            filePaths.push_back(entry.path().string());
        }
    }

    // Parallel processing of files using OpenMP
    //#pragma omp parallel for
    for (int i = 0; i < filePaths.size(); ++i) {
        loadCSVFile(filePaths[i], headers);
    }
}

// Search the global data for matching rows
vector<map<string, string>> searchLoadedData(const string& headerKey, const string& headerValue) {
    vector<map<string, string>> results;

    // Thread-local storage for results
    #pragma omp parallel
    {
        vector<map<string, string>> localResults;

        // Parallel loop over globalData
        #pragma omp for nowait
        for (size_t i = 0; i < globalData.size(); ++i) {
            const auto& row = globalData[i];
            if (row.find(headerKey) != row.end() && row.at(headerKey) == headerValue) {
                localResults.push_back(row);
            }
        }

        // Merge local results into global results
        #pragma omp critical
        results.insert(results.end(), localResults.begin(), localResults.end());
    }

    return results;
}


int main() {
    crow::SimpleApp app;

    // Define the directory path and headers
    const string directoryPath = "../Data2 - AirNow 2020 California Complex Fire";
    const vector<string> headers = {
        "lat", "lon", "time", "measurement_ozone", "measurement_PM2.5",
        "measurement_PM10", "measurement_CO", "measurement_NO2",
        "measurement_SO2", "location1", "location2", "data1", "data2"
    };

    // API endpoint to load data into memory
    CROW_ROUTE(app, "/load")
        .methods("POST"_method)([&](const crow::request& /*req*/) {
            auto start = chrono::high_resolution_clock::now();

            globalData.clear(); // Clear existing data
            loadAllCSVFiles(directoryPath, headers);

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = end - start;

            crow::json::wvalue response;
            response["status"] = "Data loaded successfully";
            response["records_loaded"] = globalData.size();
            response["execution_time"] = elapsed.count();

            return crow::response(response);
        });

    // API endpoint to search loaded data
    CROW_ROUTE(app, "/search")
        .methods("GET"_method)([&](const crow::request& req) {
            // Extract query parameters
            auto headerKey = req.url_params.get("headerKey");
            auto headerValue = req.url_params.get("headerValue");

            // Validate query parameters
            if (!headerKey || !headerValue) {
                return crow::response(400, "Missing required query parameters: headerKey and headerValue");
            }

            auto start = chrono::high_resolution_clock::now();

            // Perform the search
            vector<map<string, string>> results = searchLoadedData(headerKey, headerValue);

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = end - start;

            // Prepare JSON response
            crow::json::wvalue jsonResponse;
            jsonResponse["execution_time"] = elapsed.count();
            jsonResponse["matches"] = crow::json::wvalue::list();
            for (const auto& result : results) {
                crow::json::wvalue rowJson;
                for (const auto& pair : result) {
                    rowJson[pair.first] = pair.second;
                }
                jsonResponse["matches"][jsonResponse["matches"].size()] = std::move(rowJson);
            }

            return crow::response(jsonResponse);
        });

    // Start the Crow server
    app.port(8080).multithreaded().run();

    return 0;
}
