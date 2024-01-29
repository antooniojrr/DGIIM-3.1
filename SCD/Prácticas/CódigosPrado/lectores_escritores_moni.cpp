#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

mutex mx;

constexpr int 
	escritores = 5,
	lectores = 20,
	min_ms = 5,
	max_ms = 20;
int a;

class Lectores_Escritores : public HoareMonitor{
	private:
	bool escribiendo;
	int nlectores;
	CondVar lectura,
		escritura;
	int cuanto_leo;
	public:
	Lectores_Escritores();
	void lectura_ini();
	void lectura_fin();
	void escritura_ini();
	void escritura_fin();
	
	
};


Lectores_Escritores::Lectores_Escritores(){
	escribiendo = false;
	lectura = newCondVar();
	escritura = newCondVar();
	nlectores = 0;
	cuanto_leo = 3;

}

void Lectores_Escritores::lectura_ini(){
	
	if (escribiendo || cuanto_leo == 0 ) {
		lectura.wait();
	}
	
	nlectores++;
	if (!escritura.empty()){
		cuanto_leo--;
	}
	if (cuanto_leo > 0){
		lectura.signal();
	}

}
void Lectores_Escritores::lectura_fin(){
	nlectores--;

	
	if (nlectores == 0){
		cuanto_leo = 3;
		escritura.signal();
	}

}

void Lectores_Escritores::escritura_ini(){
	if (escribiendo || nlectores > 0){
		escritura.wait();
	}
	
	escribiendo = true;
}

void Lectores_Escritores::escritura_fin(){
	escribiendo = false;
	
	if (!lectura.empty()){
		lectura.signal();
	}
	else {
		escritura.signal();
	}
}



void funcion_hebra_escritora(MRef<Lectores_Escritores> monitor, int i){
	while (true){
		monitor->escritura_ini();
		
		mx.lock();
		cout << "Escritor: " << i << " escribiendo" << endl;
		cout << "Escritor: " << i << " termina de escribir" << endl;
		mx.unlock();
		chrono::milliseconds duracion(aleatorio < min_ms, max_ms > ());
		this_thread::sleep_for(duracion);

		monitor->escritura_fin();
		
		
	}	
	
}
void funcion_hebra_lectora(MRef<Lectores_Escritores> monitor, unsigned i){
	while (true){
		monitor->lectura_ini();
		
		mx.lock();
		cout << "Lector: " << i << " leyendo" << endl;
		cout << "Lector: " << i << " termina de termina de leer" << endl;
		
		mx.unlock();
		chrono::milliseconds duracion(aleatorio < min_ms, max_ms > ());
		this_thread::sleep_for(duracion);
		monitor->lectura_fin();
		
		
	}	
	
}



int main()
{
	MRef<Lectores_Escritores> monitor = Create<Lectores_Escritores>();
	
	thread hebras_escritoras[escritores],
	       hebras_lectoras[lectores];
	
	for (int i = 0; i < escritores; i++){
		hebras_escritoras[i] = thread(funcion_hebra_escritora,monitor,i);
	}
	
	for (int i = 0; i < lectores; i++){
		hebras_lectoras[i] = thread(funcion_hebra_lectora,monitor,i);
	}
	
	for (int i = 0; i < escritores; i++){
		hebras_escritoras[i].join();
	}
	
	for (int i = 0; i < lectores; i++){
		hebras_lectoras[i].join();
	}
	
	
	
	
}
