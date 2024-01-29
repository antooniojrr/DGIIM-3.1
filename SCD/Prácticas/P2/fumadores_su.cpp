// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: g++ -std=c++11 -pthread -o fumadores_su fumadores_su.cpp scd.cpp

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 

const int num_fumadores = 3 ;
mutex mtx;

class Estanco : public HoareMonitor
{
   private:

   static const int
      num_fumadores = 3;
   int
      ingrediente ;
   
   CondVar
      mostr_vacio ,
      ingr_disp[num_fumadores]   ;
   
   public:

   Estanco()   ;
   void obtenerIngrediente( int i)  ;
   int producirIngrediente() ;
   void ponerIngrediente(int i)  ;
   void esperarRecogidaIngrediente()   ;


};

Estanco::Estanco() {

   mostr_vacio = newCondVar();
   for ( int i = 0; i < num_fumadores; i++) 
      ingr_disp[i] = newCondVar();
   ingrediente = -1;

}

void Estanco::obtenerIngrediente( int i ) {
   if (i != ingrediente )
      ingr_disp[i].wait();
   mostr_vacio.signal();
}

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)
   

int Estanco::producirIngrediente() {

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   mtx.lock();
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   mtx.lock();
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;
   mtx.unlock();

   return num_ingrediente ;
}

void Estanco::ponerIngrediente(int i )
{
   if (!ingr_disp[i].empty()) {
      ingr_disp[i].signal();
   }
   mtx.lock();
   cout << "En mostrador ingrediente: " << i << endl;
   mtx.unlock();
}

void Estanco::esperarRecogidaIngrediente()
{
   if ( ingrediente >= 0 )
      mostr_vacio.wait();
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
   
   int i;
   while(true){
      
      i = monitor->producirIngrediente();
      monitor->ponerIngrediente(i);
      monitor->esperarRecogidaIngrediente();
      
   } 
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    mtx.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )
{
   while( true )
   {
      monitor->obtenerIngrediente(num_fumador);
      mtx.lock();
      cout << "Retirado de mostrador ingrediente: " << num_fumador << endl;
      mtx.unlock();
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   MRef<Estanco> monitor = Create<Estanco>();
   thread estanquero(funcion_hebra_estanquero, monitor);
   thread fumador[num_fumadores];
   
   for(int i = 0; i < num_fumadores; i++){
      fumador[i] = thread(funcion_hebra_fumador,monitor,i);
   }
   
   estanquero.join();
   for(int i = 0; i < num_fumadores; i++){
      fumador[i].join();
   }
   
   return 0;
}
