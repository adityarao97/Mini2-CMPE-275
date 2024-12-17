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

const string directoryPath = "../data/airnow-2020";
const vector<string> headers = {
    "lat", "lon", "time", "measurement_ozone", "measurement_PM2.5",
    "measurement_PM10", "measurement_CO", "measurement_NO2",
    "measurement_SO2", "location1", "location2", "data1", "data2"
};

// Function to split a string by a delimiter
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        // remove double quote "aa" -> aa
        /*if (token.size() > 0) {
            token = token.substr(1, token.size() - 2);
        }*/
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

    printf("n_file %lu\n", filePaths.size());
    // Parallel processing of files using OpenMP
    //#pragma omp parallel for
    for (int i = 0; i < filePaths.size(); ++i) {
        loadCSVFile(filePaths[i], headers);
    }
    printf("finished loading file\n");
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
            /*
            for (auto &record: row) {
                printf("%s: %s, ", record.first.c_str(), record.second.c_str());
            }
            printf("\n");
            */
            
            if (row.find(headerKey) != row.end() && row.at(headerKey) == headerValue) {
                //cout << row.at(headerKey) << endl;
                localResults.push_back(row);
            }
        }

        // Merge local results into global results
        #pragma omp critical
        results.insert(results.end(), localResults.begin(), localResults.end());
    }

    return results;
}

std::string convert_result_to_string(const vector<map<string, string>>& result) {
    std::string result_str;
    for (auto &record: result) {
        for (auto &field: record) {
            result_str += field.first + ":" + field.second + " ";
        }
        result_str += "\n";
    }
    return result_str;
}