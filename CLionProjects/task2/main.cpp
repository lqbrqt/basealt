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
    return f.good();
}

void clear(){ //Функция очистки журнал проектов
    if(is_file_exists(CHANGES_LIST_FILENAME)){
        std::remove(CHANGES_LIST_FILENAME);
    }
}

void generate_initial_rpm_changelist(){
    std::string str = exec("rpm -Va");
    std::ofstream output;

    output.open("changelist.file");

    if (output.is_open())
        output << str;

    output.close();
}

void parse_changes_line(const std::string& row, std::string& changes, std::string& modifier, std::string& path){
    std::istringstream iss(row);
    std::string s;

    getline( iss, changes, ' ' );
    getline( iss, modifier, ' ' );
    if(!getline( iss, path, ' ' )){
        path = modifier;
        modifier = "";
    };
}

void alert(std::string& outline){
    std::cerr << "Новые изменения: " << outline << std::endl;
}

void run_RPM_verify(){

    if(!is_file_exists(CHANGES_LIST_FILENAME)){
        std::cerr << "Файл с изменениями не найден!\n";
    }


    std::ifstream fs;
    fs.open(CHANGES_LIST_FILENAME, std::fstream::in);
    std::string line;

    while (fs.peek()!='\n' &&std::getline(fs, line)) {
        std::string changes, modifier, path;
        parse_changes_line(line, changes, modifier, path);

        std::string str = exec(("rpm -Vf " + path).c_str());
        std::string newChanges, newModifier, newPath;

        if(newChanges!=changes){
            alert(newChanges)
        }


    }


    std::string str = exec("brew list");
    std::ofstream output;

    output.open("log.txt");

    if (output.is_open())
        output << str;

    output.close();
}

int main(int argc, char **argv) {

    setlocale(LC_ALL, "Russian");
    //check_params_count(argc);

    if(strcmp(argv[1], "INIT") == 0){
        run_RPM_verify();
        //if(argv[2] == NULL) std::cerr << "Недостаточное количество аргументов\n";
        //start_new_process(argv[2]);
    }else if(strcmp(argv[1], "CHECK") == 0){
        //stop_old_process();
    }else if(strcmp(argv[1], "CLEAR") == 0){
        clear();
    }


    run_RPM_verify();
    return 0;
}
