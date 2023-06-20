

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

/**
 * @brief Интерфейс для чтения байтов из определенного источника.
 */
class SourceInterface {
public:
    virtual ~SourceInterface() {}
    
    /**
     * @brief Читает очередной байт из источника.
     * 
     * @param byte Ссылка на переменную, в которую будет записан очередной байт.
     * @return true, если байт успешно прочитан; false, если чтение не удалось.
     */
    virtual bool read(char& byte) = 0;
};

/**
 * @brief Интерфейс для записи данных в определенный приемник.
 */
class SinkInterface {
public:
    virtual ~SinkInterface() {}
    
    /**
     * @brief Записывает данные в приемник.
     * 
     * @param data Строка с данными для записи.
     */
    virtual void write(const std::string& data) = 0;
};

/**
 * @brief Класс, который в отдельном потоке читает байты из source,
 * конвертирует их в строковые символы и передает результат конвертации в sink.
 * Байт, прочитанный из source, состоит из битов type (6-ой и 7-ой), кодирующие
 * тип данных, хранящиеся в данном байте, и битов data (0-5), хранящие данные:
 * a. Если type = 0b00, то data – беззнаковое целое;
 * b. Если type = 0b01, то data – знаковое целое;
 * c. Если type = 0b10, то data – одина из букв латинского алфавита
 *    (‘a’ = 0b000000, ‘b’ = 0b000001, ‘c’ = 0b000010, ‘d’ = 0b000011 и т.д.).
 */
class Converter {
public:
    /**
     * @brief Конструктор класса Converter.
     * 
     * @param source Ссылка на объект, реализующий интерфейс SourceInterface.
     * @param sink Ссылка на объект, реализующий интерфейс SinkInterface.
     */
    Converter(SourceInterface& source, SinkInterface& sink) : 
        source_(source), sink_(sink), running_(false) {}
    
    /**
     * @brief Деструктор класса Converter.
     * 
     * Останавливает работу потока, если он еще не был остановлен.
     */
    ~Converter() {
        stop();
    }
    
    /**
     * @brief Запускает поток чтения и конвертации данных.
     * 
     * Если поток уже запущен, то метод ничего не делает.
     */
    void start() {
        if (running_) {
            return;
        }
        
        running_ = true;
        thread_ = std::thread([this]() { run(); });
    }
    
    /**
     * @brief Останавливает поток чтения и конвертации данных.
     * 
     * Если поток уже остановлен, то метод ничего не делает.
     */
    void stop() {
        if (!running_) {
            return;
        }
        
        running_ = false;
        cv_.notify_all();
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    
private:
    /**
     * @brief Основной цикл работы потока.
     * 
     * Читает очередной байт из источника, конвертирует его в строку и записывает
     * результат в приемник. Поток засыпает до тех пор, пока не будет вызван метод
     * notify_all() объекта условной переменной cv_, который указывает на то, что
     * поток может продолжить работу.
     */
    void run() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex);
// Чтение очередного байта из источника
        char byte;
        if (!source_.read(byte)) {
            break;
        }
        
        // Конвертация типа и данных из байта
        const int type = ((byte >> 6) & 0x03);
        const int data = (byte & 0x3f);
        std::string str;
        
        switch (type) {
            case 0b00: // беззнаковое целое
                str = std::to_string(data);
                break;
            case 0b01: // знаковое целое
                str = std::to_string(static_cast<int8_t>(data));
                break;
            case 0b10: // буква латинского алфавита
                str = std::string(1, 'a' + data);
                break;
            default:
                // Неподдерживаемый тип данных
                continue;
        }
        
        // Вывод результата конвертации в приемник
        sink_.write(str);
        
        cv_.wait(lock);
    }
}
