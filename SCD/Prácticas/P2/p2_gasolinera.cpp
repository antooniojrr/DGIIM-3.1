// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: g++ -std=c++11 -pthread -o p2_gasolinera p2_gasolinera.cpp scd.cpp

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

const int NUM_GASOLINA = 6;
const int NUM_GASOIL = 4;
mutex mtx;

//**********************************************************************
// Monitor Gasolinera

class Gasolinera : public HoareMonitor
{
	private:
	
	int surtidores_gasolina;
	int surtidores_gasoil;
	CondVar gasoil;
	CondVar gasolina;
	
	public:
	
	Gasolinera();
	
	void entra_coche_gasolina();
	
	void entra_coche_gasoil();
	
	void sale_coche_gasolina();
	
	void sale_coche_gasoil();
};

Gasolinera::Gasolinera(){

	surtidores_gasolina = 3;
	surtidores_gasoil = 2;
	gasoil = newCondVar();
	gasolina = newCondVar();
}

void Gasolinera::entra_coche_gasolina(){
	
	if(surtidores_gasolina == 0){
		gasolina.wait();
	}
	
	surtidores_gasolina--;
}

void Gasolinera::entra_coche_gasoil(){

	if(surtidores_gasoil == 0){
		gasoil.wait();
	}
	
	surtidores_gasoil--;
}

void Gasolinera::sale_coche_gasolina(){

	surtidores_gasolina++;
	
	if(!gasolina.empty()){
		gasolina.signal();
	}
}

void Gasolinera::sale_coche_gasoil(){

	surtidores_gasoil++;
	
	if(!gasoil.empty()){
		gasoil.signal();
	}
}

//**********************************************************************
// Repostar

void repostar(int i, string cadena){
    mtx.lock();
	cout << "Coche " << i << " comienza a repostar " << cadena << endl;
    mtx.unlock();
	chrono::milliseconds duracion_produ( aleatorio<10,100>() );
	this_thread::sleep_for( duracion_produ );
    mtx.lock();
	cout << "Coche " << i << " termina de repostar " << cadena << endl;
    mtx.unlock();
	
}

//**********************************************************************
// Funciones hebras

void funcion_hebra_gasolina(int i, MRef<Gasolinera> monitor){

	while(true){
		monitor->entra_coche_gasolina();
		repostar(i,"gasolina");
		monitor->sale_coche_gasolina();
		
		this_thread::sleep_for( chrono::milliseconds( aleatorio<10,100>() ) );
	}
}

void funcion_hebra_gasoil(int i, MRef<Gasolinera> monitor){

	while(true){
		monitor->entra_coche_gasoil();
		repostar(i,"gasoil");
		monitor->sale_coche_gasoil();
		
		this_thread::sleep_for( chrono::milliseconds( aleatorio<10,100>() ) );
	}
}

//**********************************************************************


int main(){

	MRef<Gasolinera> monitor = Create<Gasolinera>();
	
	thread coches_gasolina[NUM_GASOLINA];
	thread coches_gasoil[NUM_GASOIL];
	
	for(int i = 0; i < NUM_GASOLINA; i++){
		coches_gasolina[i] = thread(funcion_hebra_gasolina, i, monitor);
	}
	
	for(int j = 0; j < NUM_GASOIL; j++){
		coches_gasoil[j] = thread(funcion_hebra_gasoil, j, monitor);
	}
	
	for(int i = 0; i < NUM_GASOLINA; i++){
		coches_gasolina[i].join();
	}
	
	for(int j = 0; j < NUM_GASOIL; j++){
		coches_gasoil[j].join();
	}

	return 0;
}