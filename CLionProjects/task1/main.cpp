#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <map>

#define TIMETABLE_FILENAME "timetable.time"

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

std::string get_last_line_of_file(std::string filename){  // Получение последней строки из файла
    std::ifstream fs;
    fs.open(filename.c_str(), std::fstream::in);
    std::string line;

    if (fs.is_open()) {
        while (fs.peek()!='\n' &&std::getline(fs, line)) {}
        fs.close();
    }
    return line;
}

void start_task(const std::string& taskName){ // Функция записи информации о запуске проекта в файл
    std::ofstream fout;
    fout.open (TIMETABLE_FILENAME, std::ios_base::app);
    fout << "START " << taskName << " " << std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now())) << std::endl;
    fout.close();
}

void stop_task(const std::string& taskName){// Функция записи информации о остановке проекта в файл
    std::ofstream fout;
    fout.open (TIMETABLE_FILENAME, std::ios_base::app);
    fout << "STOP " << taskName << " " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl;
    fout.close();
}

void parse_table_line(const std::string& row, std::string& action, std::string& process, std::string& dateTime){ // Функция парсинга строки из файла на название процесса, действия (СТОП/СТАРТ) и времени выполнения действия
    std::istringstream iss(row);
    std::string s;

    getline( iss, action, ' ' );
    getline( iss, process, ' ' );
    getline( iss, dateTime, ' ' );

}

void start_new_process(const std::string& procName){ // Абстрактная функция начала проекта. Избегает конфликтных ситуаций
    std::string lastFileLine = get_last_line_of_file(TIMETABLE_FILENAME);

    if(lastFileLine.length() == 0){
        start_task(procName);
        return;
    }

    std::string action, process, rawDateTime;
    parse_table_line(lastFileLine, action, process, rawDateTime);

    if(action.compare("STOP") == 0){
        start_task(procName);
    }else if(action.compare("START") == 0){
        stop_task(process);
        start_task(procName);
    }
}

void stop_old_process(){ // Абстрактная функция остановки проекта. Избегает конфликтных ситуаций
    std::string lastFileLine = get_last_line_of_file(TIMETABLE_FILENAME);

    if(lastFileLine.length() == 0){
        return;
    }

    std::string action, process, rawDateTime;
    parse_table_line(lastFileLine, action, process, rawDateTime);

    if(action.compare("STOP") == 0){
        return;
    }else if(action.compare("START") == 0){
        stop_task(process);
    }
}

bool is_file_exists (const std::string& name) { //Функция проверки существования искомого файла
    std::ifstream f(name.c_str());
    return f.good();
}

void clear(){ //Функция очистки журнал проектов
    if(is_file_exists(TIMETABLE_FILENAME)){
        std::remove(TIMETABLE_FILENAME);
    }
}

void analyze(){ // Функция анализа и вывода информации о проектах в журнале
    std::map<std::string, long long> screenTime; // хранилище информации о суммарном времени работы проекта


    std::ifstream fs;
    fs.open(TIMETABLE_FILENAME, std::fstream::in);
    std::string line;

    if (fs.is_open()) {
        while (fs.peek()!='\n' && std::getline(fs, line)) { //Поочередно считываем строки из файла
            std::string currentAction, currentProcess, currentDateTime;
            parse_table_line(line, currentAction, currentProcess, currentDateTime);

            std::string nextLine;
            std::string nextAction, nextProcess, nextDateTime;

            if(!(fs.peek()!='\n' && std::getline(fs, nextLine))){ // Пытаемся считать еще одну строку (противоположную по действию ) из файла
                long long nowSeconds = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); //Обрабатываем неостановленные проекты (считаем что они идут по настоящее время)
                long timeDelta = nowSeconds - stoll(currentDateTime);

                if(screenTime.find(currentProcess) != screenTime.end()){
                    screenTime[currentProcess] += timeDelta;
                }else{
                    screenTime[currentProcess] = timeDelta;
                }

                break;
            }

            parse_table_line(nextLine, nextProcess, nextProcess, nextDateTime);



            long timeDelta = std::stoll(nextDateTime) - std::stoll(currentDateTime);

            if(screenTime.find(currentProcess) != screenTime.end()){
                screenTime[currentProcess] += timeDelta;
            }else{
                screenTime[currentProcess] = timeDelta;
            }
        }
        fs.close();

        for(auto it = screenTime.begin(); it != screenTime.end(); it++){ // Выводим информацию о проектах
            std::cout << "Проект: " << it->first << " Затраченное время: " << it->second << " секунд\n";
        }
    }else{
        std::cerr << "Файл с записями не найден\n";
    }
}

int main(int argc, char **argv) {
    setlocale(LC_ALL, "Russian");
    check_params_count(argc);


    if(strcmp(argv[1], "START") == 0){
        if(argv[2] == NULL) std::cerr << "Недостаточное количество аргументов\n";
        start_new_process(argv[2]);
    }else if(strcmp(argv[1], "STOP") == 0){
        stop_old_process();
    }else if(strcmp(argv[1], "PRINT") == 0){
        analyze();
    }else if(strcmp(argv[1], "CLEAR") == 0){
        clear();
    }

    return 0;

}
