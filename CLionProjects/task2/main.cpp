#include <iostream>
#include <fstream>
#include <unistd.h>

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

void run_RPM_verify(){

    const char* binaryPath = "/usr/bin/rpm";
    const char* arg1 = "-Va";

    execl(binaryPath, binaryPath, arg1, NULL);
}

int main() {
    run_RPM_verify();
    return 0;
}
