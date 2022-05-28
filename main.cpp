#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <mutex>
#include <atomic>
#include <condition_variable>

using namespace std;

mutex mtx;

template<int start = 10, int stop = 550>
int rand_int() {
    random_device rd;
    uniform_int_distribution<int> uid(start, stop);
    return uid(rd);
}

void incrementar(int &x) {
    // Inicio de seccion critica
    mtx.lock();
    x = x + 1;
    mtx.unlock();
    // Fin de seccion critica
}

void ejemplo_1_mutex() {
    int n_repeticion = 200;
    int n_hilos = 100;

    vector<thread> vhilos(n_hilos);

    for (int i = 0; i < n_repeticion; ++i) {
        int x = 0;
        for (auto &h: vhilos)
            h = thread(incrementar, ref(x));
        for (auto &h: vhilos)
            h.join();
        cout << x << " ";
    }
}

void ejemplo_2_atomic() {
    int n_repeticion = 200;
    int n_hilos = 100;
    vector<thread> vhilos(n_hilos);

    for (int i = 0; i < n_repeticion; ++i) {

        atomic<int> x = 0;
        for (auto &h: vhilos)
            h = thread([&x] { x++; });

        for (auto &h: vhilos)
            h.join();
        cout << x << " ";
    }
}

class cuenta_bancaria {
    condition_variable cv;
    mutex mtx;
    double saldo = 0;
public:
    void deposito(double importe) {
        unique_lock ul(mtx); // mtx.lock()
        saldo += importe;
        ul.unlock();
        cv.notify_one();    // La libere el saldo
    } // mtx.unlock()

    void retiro(double importe) {
        unique_lock ul(mtx); // mtx.lock()
        cv.wait(ul, [this] {return this->saldo > 0;});
        saldo -= importe;
    } // mtx.unlock()

    double get_saldo() { return saldo; }
};


void ejemplo_3() {
    int n_repeticiones = 100;
    for (int i = 0; i < n_repeticiones; ++i) {
        cuenta_bancaria b;
        thread t1(&cuenta_bancaria::deposito, &b, 100);
        thread t2(&cuenta_bancaria::retiro, &b, 70);
        t1.join();
        t2.join();
        cout << b.get_saldo() << " ";
    }
}

void agregar_valores(vector<int>& vec, int start, int n_size) {
    for (int i = start; i < start + n_size; ++i) {
        unique_lock ul(mtx);
        vec.push_back(i);
    }
}

void ejemplo_4_serial() {
    int n_size = 100;
    int start = 0;
    vector<int> vec;
    for (int i = start; i < n_size; ++i)
        vec.push_back(i);
    for (const auto& item: vec)
        cout << item << " ";
}

void ejemplo_4_paralela() {
    int n_size = 100;
    int start = 0;
    int n_hilos = 10;
    vector<int> vec;
    vector<thread> vhilos(n_hilos);
    auto r = (n_size - start) / n_hilos;
    auto i = start;
    for (auto& h: vhilos) {
        h = thread(agregar_valores, ref(vec), i, r);
        i += r;
    }
    for (auto& h: vhilos)
        h.join();
    cout << size(vec) << " ";
}

int main() {
//    ejemplo_1_mutex();
//    ejemplo_2_atomic();
//    ejemplo_3();
//    ejemplo_4_serial();
    int n_repeticiones = 100;
    for(int i = 0; i < n_repeticiones; ++i)
        ejemplo_4_paralela();
    return 0;
}
