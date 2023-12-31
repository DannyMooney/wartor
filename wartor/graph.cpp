#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility> // For std::pair
#include <string>
#include <stdexcept>

int main() {
    // Read data from results.csv
    std::ifstream inputFile("results.csv");
    if (!inputFile.is_open()) {
        std::cout << "Failed to open the CSV file." << std::endl;
        return 1;
    }

    std::vector<std::pair<int, float> > results; // Vector to store threads and execution times
    std::string line;

    // Read and skip the header row
    std::getline(inputFile, line);

    while (std::getline(inputFile, line)) {
        std::string numThreadsStr, executionTimeStr;
        int numThreads;
        float executionTime;

        // Parse the CSV line and extract numeric data
        size_t pos = line.find(",");
        if (pos != std::string::npos) {
            numThreadsStr = line.substr(0, pos);
            executionTimeStr = line.substr(pos + 1);
            try {
                numThreads = std::stoi(numThreadsStr);
                executionTime = std::stof(executionTimeStr);
                results.push_back(std::make_pair(numThreads, executionTime));
            } catch (const std::invalid_argument& e) {
                std::cout << "Invalid numeric value in CSV line: " << line << std::endl;
            }
        }
    }
    inputFile.close();

    // Check if we have any data to plot
    if (results.empty()) {
        std::cout << "No data to plot." << std::endl;
        return 1;
    }

    // Create an SFML window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Threads and Execution Times");

    // Create SFML text for displaying data
    sf::Font font;
    if (!font.loadFromFile("times.ttf")) {
        std::cout << "Failed to load font." << std::endl;
        return 1;
    }

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);

    // Find the max number of threads and max execution time for scaling the graph
    int maxThreads = 0;
    float maxExecutionTime = 0.0f;
    for (size_t i = 0; i < results.size(); ++i) {
        if (results[i].first > maxThreads) maxThreads = results[i].first;
        if (results[i].second > maxExecutionTime) maxExecutionTime = results[i].second;
    }

    // Define the scale factors to fit the chart in the window
    float xScale = 800.0f / maxThreads;
    float yScale = 800.0f / maxExecutionTime;

    // Create and draw lines for execution times
    for (size_t i = 0; i < results.size(); ++i) {
        float x1 = results[i].first * xScale;
        float y1 = 600 - (results[i].second * yScale);
        
        if (i > 0) {
            float x2 = results[i - 1].first * xScale;
            float y2 = 600 - (results[i - 1].second * yScale);

            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x1, y1), sf::Color::Red),
                sf::Vertex(sf::Vector2f(x2, y2), sf::Color::Red)
            };

            window.draw(line, 2, sf::Lines);
        }
    }

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.display();
    }

    return 0;
}
