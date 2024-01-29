// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: g++ -std=c++11 -pthread -o centrotatuajes_preg2 centrotatuajes_preg2.cpp scd.cpp

#include <iostream>
#include <thread>
#include <random>
#include "scd.h"
#include <atomic>
#include <mutex>
#include <string>

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const int
    NUM_CLIENTE_PEQUENO = 2,
    NUM_CLIENTE_MEDIANO = 5,
	NUM_CLIENTE_GRANDE = 3,
    NUM_TATUADOR_PEQUENO = 1,
    NUM_TATUADOR_MEDIANO = 3,
	NUM_TATUADOR_GRANDE = 1;

const string
    PEQUENO = "pequeno",
	MEDIANO = "mediano",
    GRANDE = "grande";

mutex mtx;					// Mutex para controlar el acceso concurrente para la salida por pantalla

//**********************************************************************
// Declaración del monitor CentroTatu

class CentroTatu : public HoareMonitor
{
	private:
	
	int tatuadores_pequenos;		// Número de tatuadores libres para tatuajes pequeños
	int tatuadores_medianos;		// Número de tatuadores libres para tatuajes medianos
	int tatuadores_grandes;			// Número de tatuadores libres para tatuajes grandes

	CondVar pequeno_libre;			// Variable de condición donde esperaran los "clientes de tatuaje pequeño" si no hay ningún tatuador libre
	CondVar mediano_libre;			// Variable de condición donde esperaran los "clientes de tatuaje mediano" si no hay ningún tatuador libre
	CondVar grande_libre;			// Variable de condición donde esperaran los "clientes de tatuaje grande" si no hay ningún tatuador libre
	
	public:
	
	CentroTatu();					// Constructor
	
	void entra_cliente_pequeno();

	void entra_cliente_mediano();

	void entra_cliente_grande();
	
	void sale_cliente_pequeno();
	
	void sale_cliente_mediano();

	void sale_cliente_grande();
};

//**********************************************************************
// Definición de los métodos

CentroTatu::CentroTatu(){

	// Inicializamos el número de tatuadores disponibles en el total de cada uno

	tatuadores_pequenos = NUM_TATUADOR_PEQUENO;
	tatuadores_medianos = NUM_TATUADOR_MEDIANO;
	tatuadores_grandes = NUM_CLIENTE_GRANDE;

	// Inicializamos las variables de condición

	pequeno_libre = newCondVar();
	mediano_libre = newCondVar();
	grande_libre = newCondVar();
}

void CentroTatu::entra_cliente_pequeno(){
	
	if(tatuadores_pequenos == 0){
		pequeno_libre.wait();		// Si no hay tatuadores libres, se quedará en cola esperando a que uno se libere
	}
	
	tatuadores_pequenos--;			// Reducimos en 1 el número de tatuadores disponibles
}

void CentroTatu::entra_cliente_mediano(){

	if(tatuadores_medianos == 0){
		mediano_libre.wait();		// Si no hay tatuadores libres, se quedará en cola esperando a que uno se libere
	}
	
	tatuadores_medianos--;			// Reducimos en 1 el número de tatuadores disponibles
}

void CentroTatu::entra_cliente_grande(){

	if(tatuadores_grandes == 0){
		grande_libre.wait();		// Si no hay tatuadores libres, se quedará en cola esperando a que uno se libere
	}
	
	tatuadores_grandes--;			// Reducimos en 1 el número de tatuadores disponibles
}

void CentroTatu::sale_cliente_pequeno(){

	tatuadores_pequenos++;			// Aumentamos en un el número de tatuadores disponibles
	
	pequeno_libre.signal();			// Mandamos una señal por si había algún cliente esperando en cola
}

void CentroTatu::sale_cliente_mediano(){

	tatuadores_medianos++;			// Aumentamos en un el número de tatuadores disponibles
	
	mediano_libre.signal();			// Mandamos una señal por si había algún cliente esperando en cola
}

void CentroTatu::sale_cliente_grande(){

	tatuadores_grandes++;			// Aumentamos en un el número de tatuadores disponibles
	
	grande_libre.signal();			// Mandamos una señal por si había algún cliente esperando en cola
}

//**********************************************************************
// Función tatuar

void tatuar(int i, string tipo){

	// Indicamos que el cliente ha comenzado a tatuarse

    mtx.lock();
	cout << "Cliente numero " << i << " de tatuaje " << tipo << " comienza a tatuarse" << endl;
    mtx.unlock();

	// Tiempo que tarda en tatuarse aleatorio

	chrono::milliseconds duracion_produ( aleatorio<20,200>() );
	this_thread::sleep_for( duracion_produ );

	// Indicamos que el cliente ha terminado de tatuarse

    mtx.lock();
	cout << "Cliente numero " << i << " de tatuaje " << tipo << " termina de tatuarse" << endl;
    mtx.unlock();
	
}

//**********************************************************************
// Funciones hebras

void funcion_hebra_pequeno(int i, MRef<CentroTatu> monitor){

	while(true){

		monitor->entra_cliente_pequeno();
		tatuar(i,PEQUENO);
		monitor->sale_cliente_pequeno();
		
		this_thread::sleep_for( chrono::milliseconds( aleatorio<10,100>() ) );	// Tiempo de espera aleatorio hasta tatuarse otra vez
	}
}

void funcion_hebra_mediano(int i, MRef<CentroTatu> monitor){

	while(true){

		monitor->entra_cliente_mediano();
		tatuar(i,MEDIANO);
		monitor->sale_cliente_mediano();
		
		this_thread::sleep_for( chrono::milliseconds( aleatorio<10,100>() ) );	// Tiempo de espera aleatorio hasta tatuarse otra vez
	}
}

void funcion_hebra_grande(int i, MRef<CentroTatu> monitor){

	while(true){

		monitor->entra_cliente_grande();
		tatuar(i,GRANDE);
		monitor->sale_cliente_grande();
		
		this_thread::sleep_for( chrono::milliseconds( aleatorio<10,100>() ) );	// Tiempo de espera aleatorio hasta tatuarse otra vez
	}
}

//**********************************************************************


int main(){

	MRef<CentroTatu> monitor = Create<CentroTatu>();
	
	// Declaramos los distintos thread que realizarán cada función

	thread cliente_pequeno[NUM_CLIENTE_PEQUENO];
	thread cliente_mediano[NUM_CLIENTE_MEDIANO];
	thread cliente_grande[NUM_CLIENTE_GRANDE];
	
	// Inicializamos cada uno

	for(int i = 0; i < NUM_CLIENTE_PEQUENO; i++){
		cliente_pequeno[i] = thread(funcion_hebra_pequeno, i, monitor);
	}
	
	for(int i = 0; i < NUM_CLIENTE_MEDIANO; i++){
		cliente_mediano[i] = thread(funcion_hebra_mediano, i, monitor);
	}

	for(int i = 0; i < NUM_CLIENTE_GRANDE; i++){
		cliente_grande[i] = thread(funcion_hebra_grande, i, monitor);
	}
	
	// Llamamos al método join para que la hebra principal espera la ejecución de cada una de estas
	
	for(int i = 0; i < NUM_CLIENTE_PEQUENO; i++){
		cliente_pequeno[i].join();
	}
	
	for(int i = 0; i < NUM_CLIENTE_MEDIANO; i++){
		cliente_mediano[i].join();
	}

	for(int i = 0; i < NUM_CLIENTE_GRANDE; i++){
		cliente_grande[i].join();
	}


	return 0;
}