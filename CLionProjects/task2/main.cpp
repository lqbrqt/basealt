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

#define CHANGES_LIST_FILENAME "changes.file"


std::string get_executable_name(){ //Получаем исполняемое имя для отображения подсказок при неверном количестве аргументов
    std::string sp;
    std::ifstream("/proc/self/comm") >> sp;
    return sp;
}

void check_params_count(int argc){  //Проверяем кол-во переданных параметров и выводим подсказку если необходимо
    if(argc == 1){
        std::cout << "Примеры использования:\n";
        std::cout << "\t" << get_executable_name() << " start <задача>\n";
        std::cout << "\t" << get_executable_name() << " stop\n";
        std::cout << "\t" << get_executable_name() << " print\n";
        std::cout << "\t" << get_executable_name() << " clear\n";
    }

    if(argc != 2 && argc != 3){
        std::cerr << "Неверное количество аргументов" << std::endl;
        exit(1);
    }
}

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
    bool isExist =  f.good();
    f.close();
    return isExist;
}

void clear(){ //Функция очистки журнал проектов
    if(is_file_exists(CHANGES_LIST_FILENAME)){
        std::remove(CHANGES_LIST_FILENAME);
    }
}

void generate_initial_rpm_changelist(){
    std::string str = exec("rpm -Va");
    std::ofstream output;

    output.open(CHANGES_LIST_FILENAME);

    if (output.is_open())
        output << str;

    output.close();
}

void parse_changes_line(const std::string& row, std::string& changes, std::string& modifier, std::string& path){
    std::istringstream iss(row);
    std::string s, tmp;

    getline( iss, changes, ' ' );
    getline( iss, tmp, ' ' );
    getline( iss, modifier, ' ' );
    if(!getline( iss, path, ' ' ) || path == ""){
        getline( iss, path, ' ' );
    };
}

void alert(std::string& outline){
    std::cerr << "Новые изменения: " << outline << std::endl;
    exec(("logger -p authpriv.warning -t sudo \"Новые изменения: " + outline + "\"").c_str());
}

void run_RPM_verify(){

    if(!is_file_exists(CHANGES_LIST_FILENAME)){
        std::cerr << "Файл с изменениями не найден!\n";
    }

    std::ifstream fs;
    fs.open(CHANGES_LIST_FILENAME);
    std::string line;

    std::vector<std::string> newOutChanges;


    if(fs.is_open())
        while (fs.peek()!='\n' && std::getline(fs, line)) {
            std::string changes, modifier, path;
            parse_changes_line(line, changes, modifier, path);

            if(changes == "missing") continue;

            std::string str = exec(("rpm -Vf " + path).c_str());
            std::string newChanges, newModifier, newPath;

            parse_changes_line(str, newChanges, newModifier, newPath);

            if(newChanges!=changes){
                alert(newChanges);
            }

            newOutChanges.push_back(str);
        }

    std::ofstream output;

    output.open(CHANGES_LIST_FILENAME);

    for(auto it : newOutChanges){
        output << it;
    }

    output.close();
}

int main(int argc, char **argv) {

    setlocale(LC_ALL, "Russian");
    check_params_count(argc);

    if(strcmp(argv[1], "INIT") == 0){
        generate_initial_rpm_changelist();
    }else if(strcmp(argv[1], "CHECK") == 0){
        run_RPM_verify();
    }else if(strcmp(argv[1], "CLEAR") == 0){
        clear();
    }
    return 0;
}
