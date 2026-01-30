/* * Gemini generated - 2026-01-27
 * Task: Filter PGN files by minimum ELO rating for both players.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

// Function to display usage/help
void displayHelp() {
    cout << "Description:\n";
    cout << "  Command line tool filters stronger chess games.\n";
    cout << "  The program scans a pgn-file and writes to a new pgn-file only games played by stronger players.\n\n";
    cout << "Syntax:\n";
    cout << "  pgn_filter_elo from=<pgn-source> to=<pgn-target> elo=<ELO minimum>\n";
}

// Helper to extract value from "key=value" arguments
string getParam(int argc, char* argv[], string key) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg.find(key + "=") == 0) {
            return arg.substr(key.length() + 1);
        }
    }
    return "";
}

int main(int argc, char* argv[]) {
    // Check for help flags or lack of arguments
    if (argc < 2 || string(argv[1]) == "/h") {
        displayHelp();
        return 0;
    }

    // Parse parameters
    string sourceFile = getParam(argc, argv, "from");
    string targetFile = getParam(argc, argv, "to");
    string eloMinStr = getParam(argc, argv, "elo");

    if (sourceFile.empty() || targetFile.empty() || eloMinStr.empty()) {
        displayHelp();
        return 1;
    }

    int eloMin = stoi(eloMinStr);
    ifstream pgnSource(sourceFile);
    ofstream pgnTarget(targetFile);

    if (!pgnSource.is_open()) {
        cerr << "Error: Could not open source file: " << sourceFile << endl;
        return 1;
    }

    string line, currentGame;
    int whiteElo = 0, blackElo = 0;
    bool processingGame = false;

    // Counters for reporting
    int totalGamesCount = 0;
    int writtenGamesCount = 0;

    while (getline(pgnSource, line)) {
        // Detect the start of a new game via [Event
        if (line.find("[Event ") == 0) {
            if (processingGame) {
                totalGamesCount++;
                // Check ELO requirements for the completed buffer
                if (whiteElo >= eloMin && blackElo >= eloMin) {
                    pgnTarget << currentGame;
                    writtenGamesCount++;
                }
            }
            // Reset for next game
            currentGame = "";
            whiteElo = 0;
            blackElo = 0;
            processingGame = true;
        }

        if (processingGame) {
            currentGame += line + "\n";

            // Extract ELO ratings from tags
            if (line.find("[WhiteElo \"") == 0) {
                size_t first = line.find("\"") + 1;
                size_t last = line.find_last_of("\"");
                try { whiteElo = stoi(line.substr(first, last - first)); }
                catch (...) { whiteElo = 0; }
            }
            else if (line.find("[BlackElo \"") == 0) {
                size_t first = line.find("\"") + 1;
                size_t last = line.find_last_of("\"");
                try { blackElo = stoi(line.substr(first, last - first)); }
                catch (...) { blackElo = 0; }
            }
        }
    }

    // Handle the final game in the file
    if (processingGame) {
        totalGamesCount++;
        if (whiteElo >= eloMin && blackElo >= eloMin) {
            pgnTarget << currentGame;
            writtenGamesCount++;
        }
    }

    pgnSource.close();
    pgnTarget.close();

    // Final Report
    cout << "--------------------------------------" << endl;
    cout << "Filtering complete." << endl;
    cout << "Total games scanned:   " << totalGamesCount << endl;
    cout << "Games written (Elo >= " << eloMin << "): " << writtenGamesCount << endl;
    cout << "--------------------------------------" << endl;

    return 0;
}