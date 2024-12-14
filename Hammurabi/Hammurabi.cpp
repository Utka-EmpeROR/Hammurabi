#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

/// <summary>
/// Hammurabi game state struct.
/// </summary>
struct GameState {
    int year = 1;
    int population = 100;
    int wheat = 2800;
    int land = 1000;
    int landPrice = 20;
    double deathPercentageSum = 0;
};

/// <summary>
/// Saves current game state to file.
/// </summary>
/// <param name="state"></param>
void saveGame(const GameState& state) {
    std::ofstream saveFile("savegame.txt");
    saveFile << state.year << " "
        << state.population << " "
        << state.wheat << " "
        << state.land << " "
        << state.deathPercentageSum << std::endl;
    saveFile.close();
}

/// <summary>
/// Loads saved game state from file.
/// </summary>
/// <param name="state"></param>
/// <returns>True if save file exist, False otherwise.</returns>
bool loadGame(GameState& state) {
    std::ifstream saveFile("savegame.txt");
    if (saveFile.is_open()) {
        saveFile >> state.year
            >> state.population
            >> state.wheat
            >> state.land
            >> state.deathPercentageSum;
        saveFile.close();
        return true;
    }
    return false;
}

/// <summary>
/// Displays current game state.
/// </summary>
/// <param name="state"></param>
void displayState(const GameState& state) {
    std::cout << "\nГод: " << state.year << std::endl;
    std::cout << "Население: " << state.population << std::endl;
    std::cout << "Бушелей пшеницы: " << state.wheat << std::endl;
    std::cout << "Акры земли: " << state.land << std::endl;
}

/// <summary>
/// Check for valid user input and outputs errors if its not correct input.
/// </summary>
/// <param name="prompt">: Message to appear.</param>
/// <param name="min">: Min value to check.</param>
/// <param name="max">: Max value to check.</param>
/// <param name="errorMessage">: Error message if value not in given range.</param>
/// <returns>True if ccoorect, False otherwise.</returns>
int getValidatedInput(const std::string& prompt, int min, int max, const std::string& errorMessage = "") {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail() || value < min || value > max) {
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');
            if (!errorMessage.empty()) {
                std::cout << "Ошибка: " << errorMessage << std::endl;
            }
            else {
                std::cout << "Ошибка: некорректный ввод. Попробуйте снова." << std::endl;
            }
        }
        else {
            break;
        }
    }
    return value;
}

bool nextRound(GameState& state) {
    bool isGameOver = false;
    std::srand(std::time(0));
    int landPrice = std::rand() % 10 + 17;

    std::cout << "Цена за акр земли: " << landPrice << " бушелей" << std::endl;

    int maxLandToBuy = state.wheat / landPrice;
    int landToBuy = getValidatedInput("Сколько акров земли хотите купить? (0 для пропуска): ", 0, maxLandToBuy, "Недостаточно пшеницы для покупки.");
    state.land += landToBuy;
    state.wheat -= landToBuy * landPrice;

    int landToSell = getValidatedInput("Сколько акров земли хотите продать? (0 для пропуска): ", 0, state.land, "Недостаточно акров для продажи.");
    state.land -= landToSell;
    state.wheat += landToSell * landPrice;

    int wheatToFeed = getValidatedInput("Сколько бушелей пшеницы хотите отдать на еду? ", 0, state.wheat, "Недостаточно пшеницы для распределения.");
    state.wheat -= wheatToFeed;

    int maxLandToPlant = std::min(state.land, std::min(state.wheat * 2, state.population * 10));
    int landToPlant = getValidatedInput("Сколько акров земли хотите засеять? ", 0, maxLandToPlant, "Недостаточно ресурсов для посева (пшеницы, населения или земли).");
    state.wheat -= landToPlant * 0.5;

    int harvestMultiplier = std::rand() % 6 + 1;
    int harvest = landToPlant * harvestMultiplier;
    state.wheat += harvest;
    std::cout << "Вы собрали " << harvest << " бушелей пшеницы. (по " << harvestMultiplier << " c каждого акра)" << std::endl;

    int rats = std::rand() % static_cast<int>(state.wheat * 0.07);
    state.wheat -= rats;
    std::cout << "Крысы съели " << rats << " бушелей пшеницы." << std::endl;

    int starvation = std::max(0, state.population - wheatToFeed / 20);
    double deathPercentage = static_cast<double>(starvation) / state.population;
    state.deathPercentageSum += deathPercentage;
    if (deathPercentage > 0.45) {
        isGameOver = true;
    }
    if (starvation > 0) {
        state.population -= starvation;
        std::cout << "От голода умерло " << starvation << " человек." << std::endl;
    }

    if (std::rand() % 100 < 15) {
        state.population /= 2;
        std::cout << "Чума уничтожила половину населения!" << std::endl;
    }

    int immigrants = (5 - (harvest / landToPlant)) * wheatToFeed / 600;
    immigrants = std::max(0, std::min(50, immigrants));
    if (immigrants > 0) {
        state.population += immigrants;
        std::cout << "В город прибыло " << immigrants << " человек." << std::endl;
    }

    state.year++;
    return !isGameOver;
}

int main() {
    setlocale(LC_ALL, "Russian");

    GameState state;
    char choice;

    if (loadGame(state)) {
        std::cout << "Найдена сохраненная игра. Продолжить? (y/n): ";
        std::cin >> choice;
        if (choice != 'y') {
            state = GameState();
        }
    }

    while (state.year <= 3) {
        displayState(state);
        std::cout << "Хотите выйти из игры? (y/n): ";
        std::cin >> choice;
        if (choice == 'y') {
            saveGame(state);
            std::cout << "Игра сохранена. До встречи!" << std::endl;
            break;
        }
        if (!nextRound(state)) {
            std::cout << "Погибло более 45% населения. Вы ужасный правитель, игра окончена!" << std::endl;
            return 0;
        }
    }

    if (state.year > 3) {
        std::remove("savegame.txt");
        double P = state.deathPercentageSum / 3;
        int L = state.land / state.population;
        if (P > 0.33 && L < 7) {
            std::cout << "Из-за вашей некомпетентности в управлении, народ устроил бунт, и изгнал вас их города.Теперь вы вынуждены влачить жалкое существование в изгнании" << std::endl;
        }
        else if (P > 0.1 && L < 9) {
            std::cout << "Вы правили железной рукой, подобно Нерону и Ивану Грозному.Народ вздохнул с облегчением, и никто больше не желает видеть вас правителем" << std::endl;
        }
        else if (P > 0.03 && L < 10) {
            std::cout << "«Вы справились вполне неплохо, у вас, конечно, есть недоброжелатели, но многие хотели бы увидеть вас во главе города снова" << std::endl;
        }
        else {
            std::cout << "Фантастика! Карл Великий, Дизраэли и Джефферсон вместе не справились бы лучше" << std::endl;
        }
    }

    return 0;
}
