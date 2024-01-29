// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: g++ -std=c++11 -pthread -o centrotatuajes_preg1 centrotatuajes_preg1.cpp scd.cpp

#include <iostream>
#include <thread>
#include <random>
#include "scd.h"
#include <atomic>

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

atomic<int>
    num_usados(0);		// Uso de tipo atomic para garantizar la atomicidad de los cambios en la variable

Semaphore
    pequeno = NUM_TATUADOR_PEQUENO,			// Semaforo que controla que no se tatuen a la vez más clientes que tatuadores haya para tatuajes pequeños
    mediano = NUM_TATUADOR_MEDIANO,			// Semaforo que controla que no se tatuen a la vez más clientes que tatuadores haya para tatuajes medianos
	grande = NUM_TATUADOR_GRANDE;			// Semaforo que controla que no se tatuen a la vez más clientes que tatuadores haya para tatuajes grandes

mutex mtx;					// Mutex para controlar el acceso concurrente para la salida por pantalla

//**********************************************************************

// Tatuar

void tatuar(int i, string tipo){

	// Indicamos que el cliente ha comenzado a tatuarse

    mtx.lock();
	cout << "Cliente numero " << i << " de tatuaje " << tipo << " comienza a tatuarse" << endl;
    mtx.unlock();

	// Tiempo que tarda en tatuarse aleatorio

	chrono::milliseconds duracion_produ( aleatorio<10,100>() );
	this_thread::sleep_for( duracion_produ );

	// Indicamos que el cliente ha terminado de tatuarse

    mtx.lock();
	cout << "Cliente numero " << i << " de tatuaje " << tipo << " termina de tatuarse" << endl;
    mtx.unlock();
	
}

// Funciones hebras
//----------------------------------------------------------------------

void funcion_hebra_pequeno(int i){

	while(true){
		sem_wait(pequeno);			// Esperamos que haya algún tatuador libre
		num_usados.fetch_add(1);	// Añadimos uno al número de tatuadores trabajando
		tatuar(i, PEQUENO);
		num_usados.fetch_add(-1);	// Termina el servicio, luego disminuimos los tatuadores trabajando
		sem_signal(pequeno);		// Mandamos una señal de que hay un tatuador otra vez disponible
		
		// Retraso aleatorio antes de volver al centro de tatuajes
		
		chrono::milliseconds duracion_produ( aleatorio<20,200>() );
		this_thread::sleep_for( duracion_produ );
	}
}
//----------------------------------------------------------------------

void funcion_hebra_mediano(int i){

	while(true){
		sem_wait(mediano);			// Esperamos que haya algún tatuador libre
		num_usados.fetch_add(1);	// Añadimos uno al número de tatuadores trabajando
		tatuar(i, MEDIANO);
		num_usados.fetch_add(-1);	// Termina el servicio, luego disminuimos los tatuadores trabajando
		sem_signal(mediano);		// Mandamos una señal de que hay un tatuador otra vez disponible
		
		// Retraso aleatorio antes de volver al centro de tatuajes
		
		chrono::milliseconds duracion_produ( aleatorio<20,200>() );
		this_thread::sleep_for( duracion_produ );
	}
}

//----------------------------------------------------------------------

void funcion_hebra_grande(int i){

	while(true){
		sem_wait(grande);			// Esperamos que haya algún tatuador libre
		num_usados.fetch_add(1);	// Añadimos uno al número de tatuadores trabajando
		tatuar(i, GRANDE);
		num_usados.fetch_add(-1);	// Termina el servicio, luego disminuimos los tatuadores trabajando
		sem_signal(grande);			// Mandamos una señal de que hay un tatuador otra vez disponible
		
		// Retraso aleatorio antes de volver al centro de tatuajes
		
		chrono::milliseconds duracion_produ( aleatorio<20,200>() );
		this_thread::sleep_for( duracion_produ );
	}
}

//----------------------------------------------------------------------

// Función que cada 100 milisegundos mostrará por pantalla el número total de tatuadores realizando un servicio en el momento
void test(){

	chrono::milliseconds tiempo(100);

	while(true){
		this_thread::sleep_for(tiempo);
		cout << "El numero de tatuadores trabajando ahora es " << num_usados << endl;
	}
}

//----------------------------------------------------------------------

int main(){
	
	// Declaramos los distintos thread que realizarán cada función

	thread cliente_pequeno[NUM_CLIENTE_PEQUENO];
	thread cliente_mediano[NUM_CLIENTE_MEDIANO];
	thread cliente_grande[NUM_CLIENTE_GRANDE];
    thread hebra_test(test);
	
	// Inicializamos cada uno

	for(int i = 0; i < NUM_CLIENTE_PEQUENO; i++){
		cliente_pequeno[i] = thread(funcion_hebra_pequeno, i);
	}
	
	for(int i = 0; i < NUM_CLIENTE_MEDIANO; i++){
		cliente_mediano[i] = thread(funcion_hebra_mediano, i);
	}

	for(int i = 0; i < NUM_CLIENTE_GRANDE; i++){
		cliente_grande[i] = thread(funcion_hebra_grande, i);
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

    hebra_test.join();


return 0;
}