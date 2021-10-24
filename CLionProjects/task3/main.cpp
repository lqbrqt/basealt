#include <iostream>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <cstdarg>
#include <string>
#include <memory>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

#define STAT_FILENAME "stat.file"

std::string exec(const char* cmd) {
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

bool is_file_exists (const std::string& name) { //Функция проверки существования искомого файла
    std::ifstream f(name.c_str());
    return f.good();
}

void clear(){ //Функция очистки журнал проектов
    if(is_file_exists(STAT_FILENAME)){
        std::remove(STAT_FILENAME);
    }
}

std::vector<std::string> get_network_processes_pid(){
    std::string rawOutput, tmp;
    std::vector<std::string> out;

    rawOutput = exec("netstat -ltnp");

    std::istringstream istream(rawOutput);
    std::getline(istream, tmp);
    std::getline(istream, tmp);

    std::string line;

    while(istream.peek()!='\n' && std::getline(istream, line)){
        std::istringstream lineStream(line);
        std::string tmp, pid;
        for(int i = 0; i < 6; ++i){
            lineStream >> tmp;
        }

        lineStream >> pid;


        if(pid.find("/") == std::string::npos){
            continue;
        }
        pid = pid.substr(0, pid.find("/"));
        out.push_back(pid);
    }
    return out;
}

void analyze_process_files_by_pid(std::vector<std::string> pids){

    std::ofstream fout;
    fout.open (STAT_FILENAME, std::ios_base::app);

    for(auto it : pids){
        std::string rawOutput;
        rawOutput = exec(("lsof -p " + it).c_str());

        fout << rawOutput;
    }

    fout.close();
}

void analyze_processes_files(){
    clear();

    std::vector<std::string> processPIDs = get_network_processes_pid();
    analyze_process_files_by_pid(processPIDs);
}



int main(int argc, char **argv) {

    setlocale(LC_ALL, "Russian");

    while(true){
        analyze_processes_files();
        sleep(2000);
    }



    return 0;
}
