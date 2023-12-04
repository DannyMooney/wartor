/**
 * @file main.cpp
 * @brief  Wa-Tor simulation.
 */
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <omp.h>
/**
 * @brief the variables
 */
int grid = 4;
int Width = 800;
int Height = 800;
int numFish = 200;
int numSharks = 200;
int fishBreeding = 20;
int sharkBreeding = 20;
int starve = 20;
int maxThreads = 24;
/**
 * @brief The SimulationResult is for running the simulation.
 */
struct Results {
    int numThreads;
    double Time;
};
/**
 * @brief The Fish class for everything the fish needs 
 */
class Fish {
public:
    sf::RectangleShape shape;
    sf::Vector position;
    int Timer;
      /**
     * @brief Constructor for the Fish class.
     */
    Fish(int x, int y, int grid, int fishBreed) {
        shape.setSize(sf::Vector2f(grid, grid));
        shape.setFillColor(sf::Color::Blue);
        position.x = x;
        position.y = y;
        shape.setPosition(x * grid, y * grid);
        Timer = fishBreed;
    }
/**
     * @brief Move the fish to random position
     */
    void move(std::vector<Fish>& all, int Width, int Height, int grid) {
        int randomx = rand() % 3 - 1;
        int randomy = rand() % 3 - 1; 

        int X = (position.x + randomx + Width / grid) % (Width / grid);
        int Y = (position.y + randomy + Height / grid) % (Height / grid);

        #pragma omp parallel for
        for (int i = 0; i < all.size(); i++) {
            if (all[i].position.x == X && all[i].position.y == Y) {
                return; 
            }
        }
        position.x = X;
        position.y = Y;
        shape.setPosition(position.x * grid, position.y * grid);
        Timer--;
    }
};
    /**
     * @brief Shark class.
     */

class Shark {
public:
    sf::RectangleShape shape;
    sf::Vector position;
    int breedTimer;
    int starve;
    int energy;
      /**
     * @brief Constructor for the Shark class.
     */

    Shark(int x, int y, int grid, int sharkBreed, int starve) {
        shape.setSize(sf::Vector2f(grid, grid));
        shape.setFillColor(sf::Color::Red);
        position.x = x;
        position.y = y;
        shape.setPosition(x * grid, y * grid);
        breedTimer = sharkBreed;
        starve = starve;
        energy = starve;
    }
    void move(std::vector<Fish>& fishes, int Width, int Height, int grid) {
        int randx = rand() % 3 - 1; 
        int randy = rand() % 3 - 1;
        position.x = (position.x + randx + Width / grid) % (Width / grid);
        position.y = (position.y + randy + Height / grid) % (Height / grid);
        shape.setPosition(position.x * grid, position.y * grid);
        breedTimer--;
        starve--;
        energy--; 
        if (breedTimer <= 0) {
            breedTimer = sharkBreeding;
        }
        #pragma omp parallel for
        for (int i = 0; i < fishes.size(); ++i) {
            if (position.x == fishes[i].position.x && position.y == fishes[i].position.y) {
                #pragma omp critical
                {
                    starve = starve;
                    energy += 10; 
                    fishes.erase(fishes.begin() + i);
                }
                break;
            }
        }
        if (starve <= 0 || energy <= 0) {
            starve = starve;
            position.x = -1;
        }
    }
};
/**
 * @brief handling the save of the results and the file name 
 */
void saveResultsToCSV(const std::vector<SimulationResult>& results) {
    std::ofstream outputFile("results.csv");
    outputFile << "Threads,ExecutionTime\n";
    for (const Results& result : results) {
        outputFile << result.numThreads << "," << result.Time << "\n";
    }
    outputFile.close();
}
/**
 * @brief Update the fish for the position time and death
 */
void updateFish(std::vector<Fish>& fishes, int fishBreeding, int Width, int Height, int grid)  {
    std::vector<Fish> newFishes;
    #pragma omp parallel for
    for (int i = 0; i < fishes.size(); i++) {
        fishes[i].move(fishes, windowWidth, windowHeight, gridSize);
        if (fishes[i].breedTimer <= 0) {
            fishes[i].breedTimer = fishBreed;
            Fish newFish(fishes[i].position.x, fishes[i].position.y, gridSize, fishBreed);
            #pragma omp critical
            newFishes.push_back(newFish);
        }
    }
    fishes.insert(fishes.end(), newFishes.begin(), newFishes.end());
}
/**
 * @brief Update the sharks for timer and position and breed time 
 */
void updateSharks(std::vector<Shark>& sharks, std::vector<Fish>& fishes, int sharkBreed, int Width, int Height, int grid, int starve) {
    std::vector<Shark> newSharks;
    #pragma omp parallel for
    for (int i = 0; i < sharks.size(); i++) {
        sharks[i].move(fishes, Width, Height, grid);
        if (sharks[i].breedTimer <= 0) {
            sharks[i].breedTimer = sharkBreed;
            int randx = rand() % 3 - 1;
            int randy = rand() % 3 - 1; 
            int newX = (sharks[i].position.x + randx + Width / gridSize) % (Width / gridSize);
            int newY = (sharks[i].position.y + randy + Height / grid) % (Height / grid);
            bool empty = true;
            for (int j = 0; j < sharks.size(); j++) {
                if (sharks[j].position.x == newX && sharks[j].position.y == newY) {
                    empty = false;
                    break;
                }
            }
            if (empty) {
                Shark newShark(newX, newY, grid, sharkBreed, starve);
                #pragma omp critical
                newSharks.push_back(newShark);
            }
        }
    }
    sharks.insert(sharks.end(), newSharks.begin(), newSharks.end());
}
/**
 * @brief the main part of code to show the SFML and saveing the file and using the code for Fish and Sharks
 */
int main() {
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Wator Simulation");
    std::vector<Fish> fishes;
    std::vector<Shark> sharks;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::vector<Results> results; 
    for (int numThreads = 1; numThreads <= maxThreads; numThreads++)  {
        omp_set_num_threads(numThreads);
        for (int i = 0; i < numFish; ++i) {
            int ranx = std::rand() % (Width / grid);
            int rany = std::rand() % (Height / grid);
            fishes.push_back(Fish(ranx, rany, grid, fishBreeding)); 
        }
        for (int i = 0; i < numSharks; ++i) {
            int x = std::rand() % (Width / grid);
            int y = std::rand() % (Height / grid);
            sharks.push_back(Shark(x, y, grid, sharkBreeding, starve)); 
        }
        double startTime = omp_get_wtime();
        while (!fishes.empty() && !sharks.empty() && window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            window.clear();
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    updateFish(fishes, fishBreeding, Width, Height, grid);
                }
                #pragma omp section
                {
                    updateSharks(sharks, fishes, sharkBreeding, Width, Height, grid, starve);
                }
            }
            sharks.erase(std::remove_if(sharks.begin(), sharks.end(), [](Shark& shark) {
                return shark.position.x == -1;
            }), sharks.end());
            for (Fish& fish : fishes) {
                window.draw(fish.shape);
            }
            for (Shark& shark : sharks) {
                window.draw(shark.shape);
            }
            window.display();
        }
        double endTime = omp_get_wtime();
        double executionTime = endTime - startTime;
        std::cout << "Threads: " << numThreads << ", Execution Time: " << executionTime << " seconds" << std::endl;
        Results result;
        result.numThreads = numThreads;
        result.executionTime = executionTime;
        results.push_back(result);
        fishes.clear();
        sharks.clear();
    }
    saveResultsToCSV(results);
    return 0;
}
