#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <curl/curl.h>
#include <nlohmann/json.hpp> // For JSON parsing

//Needs libcurl and nlohmann json

using json = nlohmann::json;

// Global variable for your Gemini API key (replace with your actual key)
const std::string GEMINI_API_KEY = "AIzaSyAjgCmrM0EZVrPcB_Z_RvQVIemd_fieMis"; // VERY IMPORTANT! DO NOT HARDCODE IN PRODUCTION

// Callback function for curl to write response data
size_t writeCallback(char* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(contents, totalSize);
    return totalSize;
}

std::string getGeminiResponse(const std::string& prompt) {
    CURL* curl;
    CURLcode res;
    std::string responseString;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=" + GEMINI_API_KEY;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Construct the JSON payload for the Gemini API
        json payload = {
            {"prompt", {
                {"text", prompt}
            }}
        };
        std::string payloadString = payload.dump();

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            try {
                json responseJson = json::parse(responseString);
                if (responseJson.contains("candidates") && !responseJson["candidates"].empty() &&
                    responseJson["candidates"][0].contains("output") &&
                    responseJson["candidates"][0]["output"].contains("text")) {
                    return responseJson["candidates"][0]["output"]["text"].get<std::string>();
                } else {
                    return "Error: Could not extract response from Gemini API.";
                }
            } catch (const json::parse_error& e) {
                return "Error: Failed to parse Gemini API response.";
            }
        } else {
            return "Error: Failed to connect to Gemini API: " + std::string(curl_easy_strerror(res));
        }

        curl_slist_free_all(headers); // Free the headers
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return "Error: Failed to initialize curl.";
}

int main() {
    std::cout << "Chatbot: Hello! How can I help you?" << std::endl;

    std::string userInput;
    while (true) {
        std::cout << "You: ";
        std::getline(std::cin, userInput);

        if (userInput == "exit" || userInput == "quit") {
            std::cout << "Chatbot: Goodbye!" << std::endl;
            break;
        }

        std::string response = getGeminiResponse(userInput); // Use Gemini API
        std::cout << "Chatbot: " << response << std::endl;
    }

    return 0;
}
