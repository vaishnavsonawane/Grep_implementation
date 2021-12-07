#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <unistd.h>
#include <regex>
#include <dirent.h>
#include <cstdlib>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <limits.h>
#include <stdlib.h>

using namespace std;

// Holds the user-given settings that modify grep behavior.
class Settings {
	public:
	bool invert;
	bool verbose;
	bool isFile;
	bool extra;
	int numExtra;
	string file;
	string term;
};

// Checks if the user asks for help.
// Parameters: argv (char* []) holds the arguments from the user.
void helpCheck(char *argv[]) {
	if (argv[1] == string("-h") || argv[1] == string("--help") || argv[1] == string("-help")) {
		cout << "\ngrepg8\n" << endl;
		cout << "    Usage:\n";
		cout << "    grepg8 -A <#>|-f <file> -v|-V| <search term>\n" << endl;
		cout << "    Modes:\n";
		cout << "    -A    After Context         grepg8 will grab a number of lines after the line containing the <search term>." << endl;
		cout << "    -f    Single File Search    Signals grepg8 to only search the <file> for the <search term>" << endl;
		cout << "    -v    Search Inversion      Search for every line that does not include the <search term>.\n" << endl;
		cout << "    -V    Enable Verbose        The file path to the file will be printed along with the search result.\n" << endl;
		exit(0);
	}
}

// Gets the settings given by the user.
// Parameters: argc (int) number of user arguments.
//             argv (char *[]) user arguments
//             instance (Settings *) user argument container.
void setSettings(int argc, char *argv[], Settings *instance) {
	queue<string> settings;
	for (int i = 1; i < argc; i++) {
		settings.push(argv[i]);
		// cout << argv[i] << endl;
	}
	
	// Sets settings based off of arguments received.
	while (!settings.empty()) {
		string arg = settings.front();
		if (arg == "-v") {
			(*instance).invert = true;
		} else if (arg == "-V") {
			(*instance).verbose = true;
		} else if (arg == "-f") {
			(*instance).isFile = true;
			settings.pop();
			if (!settings.empty()) {
				arg = settings.front();
			} else {
				cout << "ERROR: The path to the file was not given. \"grep -h\" for help." << endl;
				exit(0);
			}
			if (arg.compare(0, 1, "-") == 0) {
				cout << "ERROR: The path to the file was not given. \"grep -h\" for help." << endl;
				exit(0);
			}
			(*instance).file = arg;
		} else if (arg == "-A") {
			(*instance).extra = true;
			settings.pop();
			if (!settings.empty()) {
				arg = settings.front();
			} 
			else {
				cout << "ERROR: The number of after context lines was not given. \"grep -h\" for help." << endl;
				exit(0);
			}
			if (arg.compare(0, 1, "-") == 0) {
				cout << "ERROR: The number of after context lines was not given. \"grep -h\" for help." << endl;
				exit(0);
			}
			(*instance).numExtra = stoi(arg);
		} else {
			if (settings.size() > 1) {
				cout << "ERROR: grep was called incorrectly. \"grep -h\" for command syntax." << endl;
				exit(0);
			}
			(*instance).term = arg;
		}
		settings.pop();
	}

	// Check that the search term has been given.
	if ((*instance).term == "") {
		cout << "ERROR: Search term not given. \"grep -h\" for help." << endl;
		exit(0);
	}
}

void printWords(string str, Settings *instance)
{
    string word;
    regex rgx((*instance).term);
	// cout << (*instance).term << endl;
    stringstream iss(str);
    while (iss >> word) {
        if(regex_match(word.begin(), word.end(), rgx)) {
            cout << "\033[1;34m" << word << "\033[0m ";
        }
        else cout << word << " ";
    }
	cout << endl;
}

// Searches and prints results in a file-wise manner.
void printMultiple(queue<string> *filePaths, Settings *instance) {
	// while(!(filePaths->empty())){
	// 	cout << (filePaths->front()) << endl;
	// 	filePaths->pop();
	// }
	// cout << (instance->term) << endl << (instance->numExtra) << endl;
	regex rgx((*instance).term);
	while (!(filePaths->empty())) {
		string fileName;
		string output;

		fileName = filePaths->front();
		filePaths->pop();

		ifstream file(fileName);
		string line;

	// 	// Check each line and print results.
		while (getline(file, line)) {
			output = "";
			if ((*instance).verbose) {
				if (!regex_search(line.begin(), line.end(), rgx) && (*instance).invert) {
					output += fileName + ": " + line + "\n";
				} 
				else if (regex_search(line.begin(), line.end(), rgx) && !(*instance).invert) {
					output += fileName + ": " + line + "\n";
					if ((*instance).extra) {
						try {
							for (int j = 0; j < (*instance).numExtra; ++j) {
								if(!getline(file, line)) break;
								output += line + "\n";
								if (regex_search(line.begin(), line.end(), rgx)) {
									j = 0;
								}
							}
						} 
						catch (...) {
							cout << "ERROR: Could not grab line because it did not exist.\n";
						}
					}
				}
			} 
			else {
				if (!regex_search(line.begin(), line.end(), rgx) && (*instance).invert) {
					output += line + "\n";
				} 
				else if (regex_search(line.begin(), line.end(), rgx) && !(*instance).invert) {
					output += line + "\n";
					if ((*instance).extra) {
						try {
							for (int j = 0; j < (*instance).numExtra; ++j) {
								if(!getline(file, line)) break;
								output += line + "\n";
								if (regex_search(line.begin(), line.end(), rgx)) {
									j = 0;
								}
							}
						} 
						catch (...) {
							cout << "ERROR: Could not grab line because it did not exist.\n";
						}
					}
				}
			}
			if ((*instance).extra) {
				if (output != "") {
					output += "--\n";
				}
				printWords(output, instance);
			} 
			else {
				printWords(output, instance);
			}
		}
	}
}

char cwd[PATH_MAX];

// Parameters: argc (int) number of user arguments.
//             argv (char *[]) user given arguments.
int main(int argc, char *argv[]) {
	Settings *instance = new Settings;
	queue<string> *filePaths = new queue<string>;

	helpCheck(argv);
	setSettings(argc, argv, instance);
	getcwd(cwd, PATH_MAX);

	(*filePaths).push(string(cwd) + "/" + (*instance).file);
	printMultiple(filePaths, instance);

	delete(filePaths);
	delete(instance);
	return 0;
}
