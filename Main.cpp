#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <zmq.h>
#include <stdio.h>
#include "ThreadPool.cpp"
#include "hcl/huffmantool.h"


void getRecurs(std::queue<std::string>&, const std::string&);
void Arhivator(std::string);


int main(int argc, char** argv) {

    setlocale(LC_ALL, "rus");

    argc = 2;//////////////

    if (argc == 1) { // если в аргументах только имя программы
        std::cout << "Вы не ввели путь" << std::endl; // выводим, что нет аргументов
    }
    else {

        for (int i = 1; i < argc; i++) {

            std::string put = "D:\\testFlash"; //получаем путь

			std::cout << "Ваш путь: " << put << std::endl;

			std::queue<std::string> paths;

			getRecurs(paths, put); //рекурсивно получаем файлы

            void* context = zmq_ctx_new();
            void* sender = zmq_socket(context, ZMQ_PUSH);
            zmq_connect(sender, "tcp://localhost:5050");

            int count = 0;
            while (!paths.empty())
            {
                zmq_msg_t message;
                const char* ssend = "paths.";
                int t_length = strlen(ssend);
                zmq_msg_init_size(&message, t_length);
                memcpy(zmq_msg_data(&message), ssend, t_length);
                zmq_msg_send(&message, sender, 0);
                zmq_msg_close(&message);


            }
            zmq_close(sender);            


            void* receiver = zmq_socket(context, ZMQ_PULL);
            zmq_connect(receiver, "tcp://localhost:4040");

            for (int count = 0; count < 10; count++) {

                zmq_msg_t reply;
                zmq_msg_init(&reply);
                zmq_msg_recv(&reply, receiver, 0);
                int length = zmq_msg_size(&reply);
                char* value = new char[length + 1];
                memcpy(value, zmq_msg_data(&reply), length);

                printf("%s\n", value);
                free(value);

                zmq_msg_close(&reply);
            }

            zmq_close(receiver);
            zmq_ctx_destroy(context);

             /*
			if (!paths.empty()) {

				int cores_count = std::thread::hardware_concurrency(); //Узнаем кол-во ядер

				Thread_pool t(cores_count);
				while (!paths.empty()) {

					std::cout << paths.front() << std::endl;

					t.add_task(Arhivator, paths.front());  //потоки пошли в бой
					paths.pop();
				}

				t.wait_all();
				std::cout << "Готово!";
			}*/

        }

    }

    return 0;
}

void getRecurs(std::queue<std::string>& paths, const std::string& path) {

    try {
        for (const auto& file : std::filesystem::directory_iterator(path)) { //проходим по директории
            if (std::filesystem::is_directory(file)) { //если является директорией
                getRecurs(paths, file.path().string());
            }
            else {
                paths.push(file.path().string());
            }
        }
    }
    catch (std::filesystem::filesystem_error  const& ex) {
        std::cout << "Исключение: " << ex.what() << std::endl;
    }

}

void Arhivator(std::string path) { //выполняем архивирование файлов, поточная функция

    huffmantool ht;

    try {
        std::string compressedFile = path;
        size_t start{ compressedFile.find_first_of(".") }; // Находим конец подстроки
        compressedFile.insert(start, "_compress");    
        std::string output = ht.compressFile(path, compressedFile);
    }
    catch (const char* error_message) {
        std::cout << error_message << std::endl;
    }
    
}
