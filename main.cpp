#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <mmsystem.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <algorithm>
#include <random>
#include <conio.h>
#pragma comment(lib, "winmm.lib")

std::vector<std::string> GetFileListing(std::string directory, std::string fileFilter, bool recursively = true){
  std::vector<std::string> files;
  directory += "\\";
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  std::string filter = directory + (recursively ? "*" : fileFilter);
  hFind = FindFirstFile(filter.c_str(), &FindFileData);
  if (hFind == INVALID_HANDLE_VALUE){
    return files;}
  else{
    do{
        if (!recursively){
            std::string fileName = FindFileData.cFileName;
            if (fileName != "." && fileName != ".."){
                files.push_back(directory + fileName);}}
            else{
                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && FindFileData.cFileName[0] != '.'){
                std::vector<std::string> subFiles = GetFileListing(directory + std::string(FindFileData.cFileName), fileFilter);
                files.insert(files.end(), subFiles.begin(), subFiles.end());}
                else{
                    std::string fileName = FindFileData.cFileName;
                if (fileName.find(fileFilter) != std::string::npos){
                    files.push_back(directory + fileName);}}}} 
        while (FindNextFile(hFind, &FindFileData) != 0);
        DWORD dwError = GetLastError();
        FindClose(hFind);
        if (dwError != ERROR_NO_MORE_FILES){
            std::cout << "Ошибка: " << dwError << std::endl;}}
    return files;}

void playMusic(std::string trackname) {
    PlaySound(TEXT(trackname.c_str()), NULL, SND_FILENAME | SND_ASYNC);}

void printTracks(std::vector<std::string> mp3Files) {
    int trackPos = 0;
    std::cout << "Список треков в данной директории:" << std::endl;
    for (const auto& file : mp3Files) {
        std::cout << "#" << trackPos << " " << file << std::endl;
        trackPos++;}}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* buffer) {
    size_t total_size = size * nmemb;
    buffer->append((char*)contents, total_size);
    return total_size;}

void ExtractText(xmlNodePtr node) {
    xmlNodePtr cur = node;
    while (cur) {
        if (cur->type == XML_TEXT_NODE) {
            std::cout << cur->content << std::endl;
        }
        else if (cur->type == XML_ELEMENT_NODE) {
            ExtractText(cur->children);}
        cur = cur->next;
    }}

void PrintText(std::string urls) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = urls;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Отсутствует подключение к интернету."<< std::endl;
            curl_easy_cleanup(curl);
        }
        curl_easy_cleanup(curl);
        htmlDocPtr doc = htmlReadDoc((xmlChar*)response_string.c_str(), NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
        if (doc != NULL) {
            xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
            if (xpathCtx != NULL) {
                xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar*)"//div[contains(@class, 'Lyrics__Container-sc-1ynbvzw-1 kUgSbL')]", xpathCtx);
                if (xpathObj != NULL) {
                    xmlNodeSetPtr nodes = xpathObj->nodesetval;
                    if (nodes->nodeNr > 0) {
                        for (int i = 0; i < nodes->nodeNr; ++i) {
                            xmlNodePtr node = nodes->nodeTab[i];
                            ExtractText(node->children);
                        }}
                    else {
                        std::cout << "Некорректное имя трека либо же трек отсутствует в БД сайта." << std::endl;}
                    xmlXPathFreeObject(xpathObj);}
                xmlXPathFreeContext(xpathCtx);}
            xmlFreeDoc(doc);
            xmlCleanupParser();}}}

std::string transformString(std::string input) {
    std::string output = input;
    output[0] = std::toupper(output[0]);
    for (size_t i = 1; i < output.size(); ++i) {
        output[i] = std::tolower(output[i]);
    }
    size_t dotPos = output.find_last_of('.');
    if (dotPos != std::string::npos) {
        output.erase(dotPos);
    }
    std::replace(output.begin(), output.end(), ' ', '-');
    output += "-lyrics";
    std::string url = "https://genius.com/";
    url += output;
    return url;
}

std::vector<std::string> checkDirectory() {
    std::cout << "Введите полный путь к папке с музыкой: " << std::endl;
    std::string path;
    std::string substring = "\\";
    std::cin >> path;
    while (path.find(substring) == std::string::npos) {
        std::cout << "Неверный выбор. Попробуйте ещё раз: " << std::endl;
        std::cin >> path;
        system("CLS");
    }
    std::vector<std::string> mp3FilesPath = GetFileListing(path, ".wav");
    if (mp3FilesPath.size() == 0) {
        system("CLS");
        std::cout << "Директория пуста. Проверьте правильность ввода, либо введите другой путь." << std::endl;
        checkDirectory();
    }
    return mp3FilesPath;
}

int main() {
    std::vector<std::string> mp3FilesPath = checkDirectory();
    std::vector<std::string> mp3Files;
    for (const auto& file : mp3FilesPath) {
        size_t pose = file.find_last_of("\\");
        std::string fileName = file.substr(pose + 1);
        mp3Files.push_back(fileName);
    }
    while (true) {
        system("CLS");
        printTracks(mp3Files);
        int menuChoice;
        std::cout << "┌============================┐" << std::endl;
        std::cout << "│         WAV Player         │" << std::endl;
        std::cout << "│────────────────────────────│" << std::endl;
        std::cout << "│ 1) Выбрать трек            │" << std::endl;
        std::cout << "│ 2) Остановить трек         │" << std::endl;
        std::cout << "│ 3) Случайный трек          │" << std::endl;
        std::cout << "│ 4) Поиск текста по номеру  │" << std::endl;
        std::cout << "│ 5) Поиск текста вручную    │" << std::endl;
        std::cout << "│ 6) Выход                   │" << std::endl;
        std::cout << "└============================┘" << std::endl;
        std::cin >> menuChoice;
        if (menuChoice == 1) {
            std::cout << "Для прослушивания введите номер трека: ";
            int trackChoice = 0;
            std::cin >> trackChoice;
            while (trackChoice > mp3Files.size()) {
                std::cout << "Неверный выбор, попробуйте снова: ";
                std::cin >> trackChoice;
            }
            playMusic(mp3FilesPath[trackChoice]);
        }
        else if (menuChoice == 2) {
            PlaySound(0, 0, 0);
        }
        else if (menuChoice == 3) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distr(0, mp3Files.size()-1);
            playMusic(mp3FilesPath[distr(gen)]);
        }
        else if (menuChoice == 4) {
            int trackChoice = 0;
            std::cout << "Введите номер трека: ";
            std::cin >> trackChoice;
            while (trackChoice > mp3Files.size()) {
                std::cout << "Неверный выбор, попробуйте снова: ";
                std::cin >> trackChoice;
            }
            std::string url;
            url = transformString(mp3Files[trackChoice]);
            PrintText(url);
            std::cout << "Нажмите любую кнопку чтобы продолжить...";
            char ch;
            ch = _getch();
            if (ch >= 0) {
                continue;
            }}
        else if (menuChoice == 5) {
                std::cout << "Введите имя автора и название трека через пробел(на английском языке): " << std::endl;
                std::string handUrl;
                std::cin.ignore();
                std::getline(std::cin, handUrl);
                std::string url; 
                url = transformString(handUrl);
                PrintText(url);
                std::cout << "Нажмите любую кнопку чтобы продолжить...";
                char ch;
                ch = _getch();
                if (ch >= 0) {
                    continue;
                }}
        else if (menuChoice == 6) {
            exit(0);}}}
