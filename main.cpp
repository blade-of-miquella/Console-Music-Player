#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <mmsystem.h>
#include <xpath.h>
#include <HTMLparser.h>
#include <curl/curl.h>
#include <algorithm>
#include <random>
#include <conio.h> 
#include <tree.h>
#pragma comment(lib, "winmm.lib")

using namespace std;

vector<string> GetFileListing(string directory, string fileFilter, bool recursively = true) {
    vector<string> files;
    directory += "\\";
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    string filter = directory + (recursively ? "*" : fileFilter);
    hFind = FindFirstFile(filter.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return files;
    }
    else {
        do {
            if (!recursively) {
                string fileName = FindFileData.cFileName;
                if (fileName != "." && fileName != "..") {
                    files.push_back(directory + fileName);
                }
            }
            else {
                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && FindFileData.cFileName[0] != '.') {
                    vector<string> subFiles = GetFileListing(directory + string(FindFileData.cFileName), fileFilter);
                    files.insert(files.end(), subFiles.begin(), subFiles.end());
                }
                else {
                    string fileName = FindFileData.cFileName;
                    if (fileName.find(fileFilter) != string::npos) {
                        files.push_back(directory + fileName);
                    }
                }
            }
        } while (FindNextFile(hFind, &FindFileData) != 0);
        DWORD dwError = GetLastError();
        FindClose(hFind);
        if (dwError != ERROR_NO_MORE_FILES) {
            cout << "Error: " << dwError << endl;
        }
    }
    return files;
}

void playMusic(string trackname) {
    PlaySound(TEXT(trackname.c_str()), NULL, SND_FILENAME | SND_ASYNC);
}

void printTracks(vector<string> mp3Files) {
    int trackPos = 0;
    cout << "List of tracks in the directory:" << endl;
    for (const auto& file : mp3Files) {
        cout << "#" << trackPos << " " << file << endl;
        trackPos++;
    }
}

std::size_t WriteCallback(void* contents, std::size_t size, std::size_t nmemb, string* buffer) {
    std::size_t total_size = size * nmemb;
    buffer->append((char*)contents, total_size);
    return total_size;
}

void ExtractText(xmlNodePtr node) {
    xmlNodePtr cur = node;
    while (cur) {
        if (cur->type == XML_TEXT_NODE) {
            cout << cur->content << endl;
        }
        else if (cur->type == XML_ELEMENT_NODE) {
            ExtractText(cur->children);
        }
        cur = cur->next;
    }
}

void PrintText(string urls) {
    CURL* curl = curl_easy_init();
    if (curl) {
        string url = urls;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "No internet connection." << endl;
            curl_easy_cleanup(curl);
        }
        curl_easy_cleanup(curl);
        htmlDocPtr doc = htmlReadDoc((xmlChar*)response_string.c_str(), NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
        if (doc != NULL) {
            xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
            if (xpathCtx != NULL) {
                xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar*)"//div[@data-lyrics-container='true']", xpathCtx);
                if (xpathObj != NULL) {
                    xmlNodeSetPtr nodes = xpathObj->nodesetval;
                    if (nodes->nodeNr > 0) {
                        for (int i = 0; i < nodes->nodeNr; ++i) {
                            xmlNodePtr node = nodes->nodeTab[i];
                            ExtractText(node->children);
                        }
                    }
                    else {
                        cout << "Incorrect track name or the track is not in the site's database." << endl;
                    }
                    xmlXPathFreeObject(xpathObj);
                }
                xmlXPathFreeContext(xpathCtx);
            }
            xmlFreeDoc(doc);
            xmlCleanupParser();
        }
    }
}

string transformString(string input) {
    string output = input;
    output[0] = toupper(output[0]);
    for (size_t i = 1; i < output.size(); ++i) {
        output[i] = tolower(output[i]);
    }
    size_t dotPos = output.find_last_of('.');
    if (dotPos != string::npos) {
        output.erase(dotPos);
    }
    replace(output.begin(), output.end(), ' ', '-');
    output += "-lyrics";
    string url = "https://genius.com/";
    url += output;
    return url;
}

vector<string> checkDirectory() {
    cout << "Enter the full path to the music folder: " << endl;
    string path;
    string substring = "\\";
    cin >> path;
    while (path.find(substring) == string::npos) {
        cout << "Invalid choice. Try again: " << endl;
        cin >> path;
        system("CLS");
    }
    vector<string> mp3FilesPath = GetFileListing(path, ".wav");
    if (mp3FilesPath.size() == 0) {
        system("CLS");
        cout << "Directory is empty. Check the input or enter another path." << endl;
        checkDirectory();
    }
    return mp3FilesPath;
}

void stopMusic() {
    PlaySound(NULL, 0, 0);
}

int main() {
    vector<string> mp3FilesPath = checkDirectory();
    vector<string> mp3Files;
    for (const auto& file : mp3FilesPath) {
        size_t pose = file.find_last_of("\\");
        string fileName = file.substr(pose + 1);
        mp3Files.push_back(fileName);
    }
    while (true) {
        system("CLS");
        printTracks(mp3Files);
        int menuChoice;
        cout << " ============================ " << endl;
        cout << "|         WAV Player         |" << endl;
        cout << "|----------------------------|" << endl;
        cout << "| 1) Select track            |" << endl;
        cout << "| 2) Stop track              |" << endl;
        cout << "| 3) Random track            |" << endl;
        cout << "| 4) Search lyrics by number |" << endl;
        cout << "| 5) Search lyrics manually  |" << endl;
        cout << "| 6) Exit                    |" << endl;
        cout << " ============================ " << endl;
        cin >> menuChoice;
        if (menuChoice == 1) {
            cout << "Enter the track number to listen: ";
            int trackChoice = 0;
            cin >> trackChoice;
            while (trackChoice > mp3Files.size()) {
                cout << "Invalid choice, try again: ";
                cin >> trackChoice;
            }
            playMusic(mp3FilesPath[trackChoice]);
        }
        else if (menuChoice == 2) {
            PlaySound(0, 0, 0);
        }
        else if (menuChoice == 3) {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> distr(0, mp3Files.size() - 1);
            playMusic(mp3FilesPath[distr(gen)]);
        }
        else if (menuChoice == 4) {
            int trackChoice = 0;
            cout << "Enter the track number: ";
            cin >> trackChoice;
            while (trackChoice > mp3Files.size()) {
                cout << "Invalid choice, try again: ";
                cin >> trackChoice;
            }
            string url;
            url = transformString(mp3Files[trackChoice]);
            PrintText(url);
            cout << "Press any key to continue...";
            char ch;
            ch = _getch();
            if (ch >= 0) {
                continue;
            }
        }
        else if (menuChoice == 5) {
            cout << "Enter the artist and track name separated by a space (in English): " << endl;
            string handUrl;
            cin.ignore();
            getline(cin, handUrl);
            string url;
            url = transformString(handUrl);
            PrintText(url);
            cout << "Press any key to continue...";
            char ch;
            ch = _getch();
            if (ch >= 0) {
                continue;
            }
        }
        else if (menuChoice == 6) {
            stopMusic();
            exit(0);
        }
    }
}
