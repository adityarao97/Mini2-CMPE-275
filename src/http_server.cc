#include <crow.h>
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
#include "csv_processor.h"
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

//#include <omp.h>

using namespace std;
using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

ABSL_FLAG(double, result_size, 1.0, "Result size");

int main(int agrc, char** argv) {
    crow::SimpleApp app;
    absl::ParseCommandLine(agrc, argv);
    printf("result size %f\n", absl::GetFlag(FLAGS_result_size));

    loadAllCSVFiles(directoryPath, headers);
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

            // Limit the number of results
            int counter = absl::GetFlag(FLAGS_result_size) * results.size();
            printf("result size %d\n", counter);

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = end - start;

            // Prepare JSON response
            crow::json::wvalue jsonResponse;
            jsonResponse["execution_time"] = elapsed.count();
            jsonResponse["matches"] = crow::json::wvalue::list();
            for (const auto& result : results) {
                if (counter-- <= 0) {
                    break;
                }
                crow::json::wvalue rowJson;
                for (const auto& pair : result) {
                    rowJson[pair.first] = pair.second;
                }
                jsonResponse["matches"][jsonResponse["matches"].size()] = std::move(rowJson);
            }

            return crow::response(jsonResponse);
        });

    CROW_ROUTE(app, "/search_string")
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

            // Limit the number of results
            int counter = absl::GetFlag(FLAGS_result_size) * results.size();
            printf("result size %d\n", counter);

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = end - start;

            // Prepare JSON response
            crow::json::wvalue jsonResponse;
            jsonResponse["execution_time"] = elapsed.count();
            jsonResponse["matches"] = convert_result_to_string(results);

            return crow::response(jsonResponse);
        });
    // Start the Crow server
    app.port(8080).multithreaded().run();

    return 0;
}
