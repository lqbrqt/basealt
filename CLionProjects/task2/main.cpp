#include <iostream>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <cstdarg>
#include <string>
#include <memory>
#include <cstdio>

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

void run_RPM_verify(){

    const char* binaryPath = "/usr/bin/rpm";
    const char* arg1 = "-Va";

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
