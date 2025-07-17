#include "../logger/include/logger.hpp"
#include <thread>
#include <queue>
#include <condition_variable>
#include <chrono>

using namespace std;

class MessageQueue {
public:
    void push(const string& msg, LogLevel lvl) {
        unique_lock<mutex> lock(mtx);
        queue.push({msg, lvl});
        cv.notify_one();
    }

    pair<string, LogLevel> pop() {
        unique_lock<mutex> lock(mtx);
        while(queue.empty())
            cv.wait(lock);
        auto front = queue.front();
        queue.pop();
        return front;
    }

private:
    condition_variable cv;
    mutex mtx;
    queue<pair<string, LogLevel>> queue;
};

void workerThread(MessageQueue& q) {
    while(true) {
        auto [msg, lvl] = q.pop();
        Logger::log(msg, lvl);
    }
}

int main(int argc, char* argv[]) {
    if(argc < 3){
        cerr << "Usage: " << argv[0] << " <log_file_name> <default_log_level>\n";
        return EXIT_FAILURE;
    }

    string filename(argv[1]);
    LogLevel defLvl = LogLevel::INFO;
    try{
        defLvl = static_cast<LogLevel>(stoi(argv[2]));
    } catch(...) {}

    Logger::init(filename, defLvl);

    MessageQueue q;
    thread t(workerThread, ref(q)); // Запускаем фоновый поток обработки очереди

    string input;
    while(getline(cin, input)){
        size_t pos = input.find(' ');
        if(pos != string::npos){
            string msg = input.substr(pos+1);
            LogLevel lvl = LogLevel::INFO;
            try{
                lvl = static_cast<LogLevel>(stoi(input.substr(0,pos)));
            }catch(...){lvl=defLvl;}
            q.push(msg,lvl);
        } else {
            q.push(input, defLvl);
        }
    }

    t.join();
    return EXIT_SUCCESS;
}