#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <omp.h>

int gridSize = 4;
int windowWidth = 800;
int windowHeight = 800;
int numFish = 200;
int numSharks = 200;
int fishBreed = 20;
int sharkBreed = 20;
int starve = 20;
int maxThreads = 24;

struct SimulationResult {
    int numThreads;
    double executionTime;
};

class Fish {
public:
    sf::RectangleShape shape;
    sf::Vector2i position;
    int breedTimer;

    Fish(int x, int y, int gridSize, int fishBreed) {
        shape.setSize(sf::Vector2f(gridSize, gridSize));
        shape.setFillColor(sf::Color::Blue);
        position.x = x;
        position.y = y;
        shape.setPosition(x * gridSize, y * gridSize);
        breedTimer = fishBreed;
    }

    void move(std::vector<Fish>& allFish, int windowWidth, int windowHeight, int gridSize) {
        int dx = rand() % 3 - 1;
        int dy = rand() % 3 - 1; 

        int newX = (position.x + dx + windowWidth / gridSize) % (windowWidth / gridSize);
        int newY = (position.y + dy + windowHeight / gridSize) % (windowHeight / gridSize);

        #pragma omp parallel for
        for (int i = 0; i < allFish.size(); i++) {
            if (allFish[i].position.x == newX && allFish[i].position.y == newY) {
                return; 
            }
        }
        position.x = newX;
        position.y = newY;
        shape.setPosition(position.x * gridSize, position.y * gridSize);
        breedTimer--;
    }
};

class Shark {
public:
    sf::RectangleShape shape;
    sf::Vector2i position;
    int breedTimer;
    int starveTimer;
    int energy;
  
    Shark(int x, int y, int gridSize, int sharkBreed, int starve) {
        shape.setSize(sf::Vector2f(gridSize, gridSize));
        shape.setFillColor(sf::Color::Red);
        position.x = x;
        position.y = y;
        shape.setPosition(x * gridSize, y * gridSize);
        breedTimer = sharkBreed;
        starveTimer = starve;
        energy = starve;
    }
    void move(std::vector<Fish>& fishes, int windowWidth, int windowHeight, int gridSize) {
        int dx = rand() % 3 - 1; 
        int dy = rand() % 3 - 1;
        position.x = (position.x + dx + windowWidth / gridSize) % (windowWidth / gridSize);
        position.y = (position.y + dy + windowHeight / gridSize) % (windowHeight / gridSize);
        shape.setPosition(position.x * gridSize, position.y * gridSize);
        breedTimer--;
        starveTimer--;
        energy--; 
        if (breedTimer <= 0) {
            breedTimer = sharkBreed;
        }
        #pragma omp parallel for
        for (int i = 0; i < fishes.size(); ++i) {
            if (position.x == fishes[i].position.x && position.y == fishes[i].position.y) {
                #pragma omp critical
                {
                    starveTimer = starve;
                    energy += 10; 
                    fishes.erase(fishes.begin() + i);
                }
                break;
            }
        }
        if (starveTimer <= 0 || energy <= 0) {
            starveTimer = starve;
            position.x = -1;
        }
    }
};
void saveResultsToCSV(const std::vector<SimulationResult>& results) {
    std::ofstream outputFile("simulation_results.csv");
    outputFile << "Threads,ExecutionTime\n";
    for (const SimulationResult& result : results) {
        outputFile << result.numThreads << "," << result.executionTime << "\n";
    }
    outputFile.close();
}
void updateFish(std::vector<Fish>& fishes, int fishBreed, int windowWidth, int windowHeight, int gridSize)  {
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
void updateSharks(std::vector<Shark>& sharks, std::vector<Fish>& fishes, int sharkBreed, int windowWidth, int windowHeight, int gridSize, int starve) {
    std::vector<Shark> newSharks;
    #pragma omp parallel for
    for (int i = 0; i < sharks.size(); i++) {
        sharks[i].move(fishes, windowWidth, windowHeight, gridSize);
        if (sharks[i].breedTimer <= 0) {
            sharks[i].breedTimer = sharkBreed;
            int dx = rand() % 3 - 1;
            int dy = rand() % 3 - 1; 
            int newX = (sharks[i].position.x + dx + windowWidth / gridSize) % (windowWidth / gridSize);
            int newY = (sharks[i].position.y + dy + windowHeight / gridSize) % (windowHeight / gridSize);
            bool empty = true;
            for (int j = 0; j < sharks.size(); j++) {
                if (sharks[j].position.x == newX && sharks[j].position.y == newY) {
                    empty = false;
                    break;
                }
            }
            if (empty) {
                Shark newShark(newX, newY, gridSize, sharkBreed, starve);
                #pragma omp critical
                newSharks.push_back(newShark);
            }
        }
    }
    sharks.insert(sharks.end(), newSharks.begin(), newSharks.end());
}
int main() {
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Wator Simulation");
    std::vector<Fish> fishes;
    std::vector<Shark> sharks;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::vector<SimulationResult> results; 
    for (int numThreads = 1; numThreads <= maxThreads; numThreads++)  {
        omp_set_num_threads(numThreads);
        for (int i = 0; i < numFish; ++i) {
            int x = std::rand() % (windowWidth / gridSize);
            int y = std::rand() % (windowHeight / gridSize);
            fishes.push_back(Fish(x, y, gridSize, fishBreed)); 
        }
        for (int i = 0; i < numSharks; ++i) {
            int x = std::rand() % (windowWidth / gridSize);
            int y = std::rand() % (windowHeight / gridSize);
            sharks.push_back(Shark(x, y, gridSize, sharkBreed, starve)); 
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
                    updateFish(fishes, fishBreed, windowWidth, windowHeight, gridSize);
                }
                #pragma omp section
                {
                    updateSharks(sharks, fishes, sharkBreed, windowWidth, windowHeight, gridSize, starve);
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
        SimulationResult result;
        result.numThreads = numThreads;
        result.executionTime = executionTime;
        results.push_back(result);
        fishes.clear();
        sharks.clear();
    }
    saveResultsToCSV(results);
    return 0;
}
