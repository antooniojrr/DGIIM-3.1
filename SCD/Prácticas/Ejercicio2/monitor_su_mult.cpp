// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: g++ -std=c++11 -pthread -o monitor_su_mult monitor_su_mult.cpp scd.cpp

#include <iostream>
#include <thread>
#include <random>
#include "scd.h"
#include <atomic>
#include <mutex>
#include <string>

using namespace std ;
using namespace scd ;


const int
    NUM_RECURSOS = 5,
    NUM_PROC = 7   ;


class Monitor : public HoareMonitor
{

    private:
        CondVar asignando;
        CondVar proceso[NUM_PROC];
        bool buscando;
        bool libre[NUM_RECURSOS];
        int num_ocupados;

    public:
        
        Monitor() {

            for(int i = 0; i < NUM_RECURSOS; i++)
                libre[i] = true;
            asignando = newCondVar();

            for(int j = 0; j < NUM_PROC; j++)
                proceso[j]=newCondVar();
            
            buscando=false;
            num_ocupados = 0;

        }

        int pedir(int id) {

            cout << "Proceso " << id << " pide un recurso" << endl;

            int idRec = 0;

            if ( num_ocupados == NUM_RECURSOS ) proceso[id].wait();
            
            buscando = true;

            while(!libre[idRec] && idRec < NUM_RECURSOS) idRec++;

            if(idRec < NUM_RECURSOS) 
                libre[idRec] = false;
            num_ocupados++;
            if (!asignando.empty())
                asignando.signal();

            buscando = false;

            
            cout << "Proceso " << id << " accede a recurso " << idRec << endl;
            
            return idRec;
        }

        void devolver(int id) {

            cout << "Devuelto recurso " << id << endl;
            libre[id]=true;
            
            if(num_ocupados==NUM_RECURSOS) {

                int idProc=0;
                while(proceso[idProc].empty() && idProc < NUM_PROC) idProc++;

                if(buscando) asignando.wait();
                if (idProc < NUM_PROC) proceso[idProc].signal();

            }
            num_ocupados--;

        }

};

void proceso(MRef<Monitor> monitor, int id) {
    int idRec;
    while(true) {
        idRec = monitor->pedir(id);

        // Tiempo aleatorio de uso del recurso
        this_thread::sleep_for(chrono::milliseconds( aleatorio<10,100>() ));

        monitor->devolver(idRec);

        // Tiempo aleatorio hasta pedir otro recurso
        this_thread::sleep_for(chrono::milliseconds( aleatorio<20,200>() ));
    }
}

int main() {

    thread procesos[NUM_PROC];
    MRef<Monitor> monitor = Create<Monitor>();

    for ( int i = 0; i < NUM_PROC; i++)
        procesos[i] = thread(proceso,monitor,i);
    for ( int i = 0; i < NUM_PROC; i++)
        procesos[i].join();

    return 0;
}