// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: g++ -std=c++11 -pthread -o p1_gasolinera p1_gasolinera.cpp scd.cpp

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
    NUM_GASOIL = 3,
    NUM_GASOLINA = 7,
    NUM_SURTIDORES_GASOIL = 2,
    NUM_SURTIDORES_GASOLINA = 3;

const string
    GASOLINA = "gasolina",
    GASOIL = "gasoil";

atomic<int>
    num_usados(0);

Semaphore
    gasoil = NUM_SURTIDORES_GASOIL,
    gasolina = NUM_SURTIDORES_GASOLINA;

//**********************************************************************

// Repostar

void repostar(int i, string gas){

	cout << "Coche numero " << i << " de " << gas << " comienza a repostar " << endl;
	this_thread::sleep_for( chrono::milliseconds(aleatorio<10,100>()) );
	cout << "Coche numero " << i << " de " << gas << " termina de repostar " << endl;
	
}

// Funciones hebras
//----------------------------------------------------------------------

void funcion_hebra_gasolina(int i){

	while(true){
		sem_wait(gasolina);
		num_usados.fetch_add(1);
		repostar(i, GASOLINA);
		num_usados.fetch_add(-1);
		sem_signal(gasolina);
		
		// Retraso aleatorio antes de volver a la gasolinera
		
		chrono::milliseconds duracion_produ( aleatorio<20,200>() );
		this_thread::sleep_for( duracion_produ );
	}
}
//----------------------------------------------------------------------

void funcion_hebra_gasoil(int i){

	while(true){
		sem_wait(gasoil);
		num_usados.fetch_add(1);
		repostar(i, GASOIL);
		num_usados.fetch_add(-1);
		sem_signal(gasoil);
		
		// Retraso aleatorio antes de volver a la gasolinera
		
		chrono::milliseconds duracion_produ( aleatorio<20,200>() );
		this_thread::sleep_for( duracion_produ );
	}
}

//----------------------------------------------------------------------

void test(){

	chrono::milliseconds tiempo(100);

	while(true){
		this_thread::sleep_for(tiempo);
		cout << "El numero de surtidores en uso ahora es " << num_usados << endl;
	}
}

//----------------------------------------------------------------------

int main(){
	
	thread coches_gasolina[NUM_GASOLINA];
	thread coches_gasoil[NUM_GASOIL];
    thread hebra_test(test);
	
	for(int i = 0; i < NUM_GASOLINA; i++){
		coches_gasolina[i] = thread(funcion_hebra_gasolina, i);
	}
	
	for(int j = 0; j < NUM_GASOIL; j++){
		coches_gasoil[j] = thread(funcion_hebra_gasoil, j);
	}
	
	for(int i = 0; i < NUM_GASOLINA; i++){
		coches_gasolina[i].join();
	}
	
	for(int j = 0; j < NUM_GASOIL; j++){
		coches_gasoil[j].join();
	}

    hebra_test.join();


return 0;
}