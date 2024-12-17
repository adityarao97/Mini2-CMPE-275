#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <chrono>
#include <filesystem>

using namespace std;
using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;


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

//return fileName if it has .csv as extension
bool hasCSVExtension(const string& filename) {
    return filename.substr(filename.size() - 4) == ".csv";
}

int main(int argc, char *argv[]) {
    auto start = chrono::high_resolution_clock::now();
    std::string directoryPath = "../Data Sets/Data2 - AirNow 2020 California Complex Fire";

    if (argc < 3) {
        cout << "No search keyword entered" << endl;
        return 1;
    }

    string headerKey = argv[1];
    string headerValue = argv[2];
    //custom header as headers were not defined in file
    vector<string> headers = {"lat", "lon", "time", "measurement_ozone", "measurement_PM2.5", "measurement_PM10", "measurement_CO", "measurement_NO2", "measurement_SO2", "location1", "location2", "data1", "data2"};

    //recursively obtain all files within the directoryPath 
    for (const auto& entry : recursive_directory_iterator(directoryPath)){
        string filePath = entry.path().string();
        if (hasCSVExtension(filePath)) {
            ifstream file(filePath);
            if (!file.is_open()) {
                cerr << "Error: File " << filePath << " could not be opened" << endl;
                continue;
            }

            string line;
            vector<map<string, string>> data;
            
            //map each cell based on headers and store in rowMap
            while (getline(file, line)) {
                vector<string> row = split(line, ',');
                map<string, string> rowMap;
                for (size_t i = 0; i < headers.size(); i++) {
                    if (i < row.size()) {
                        rowMap[headers[i]] = row[i];
                    }
                }
                data.push_back(rowMap);
            }
            file.close();

            bool found = false;
            for (const auto &rowMap : data) {
                if (rowMap.find(headerKey) != rowMap.end() && rowMap.at(headerKey) == headerValue) {
                    found = true;
                    // Print the matching row
                    if(rowMap.size() == headers.size()){
                        for (const auto &header : headers) {
                            cout << header << ": " << rowMap.at(header) << " | ";
                        }
                    }
                    else{
                        cout << "Row with mismatched size:" << endl;
                        for (const auto &pair : rowMap) {
                            cout << pair.first << ": " << pair.second << " | ";
                        }
                    }
                    cout << endl;
                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double> executionTime = end - start;
                    cout << "Time spent: " << executionTime.count() << " seconds" << endl;
                    break;
                }
            }
        }
    }
    return 0;
}