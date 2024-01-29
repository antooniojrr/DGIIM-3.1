// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: g++ -std=c++11 -pthread -o p1_pc_impr p1_pc_impr.cpp scd.cpp

// Comentarios:
// - Variables usadas: primera_libre, marcará la primera posición disponible para escribir un dato y, por tanto, la inmediatamente anterior será el último dato escrito
//                     primera_ocupada, marca la primera posición del vector ocupada por un dato
//                     num_ocupadas, guarda el número de casillas ocupadas hasta el momento
// - Semáforos: libres, controla que no se vaya a guardar un dato hasta que no haya espacio libre para este
//              ocupados, controla que no se vaya a consumir un dato hasta que no haya alguno disponible para consumir
//              impresora, controla que la impresora realice su trabajo cuando el dato producido es múltiplo de 5
//              multiplo, bloquea la hebra productora mientras la impresora hace su trabajo

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <atomic>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 50 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

unsigned
   primera_libre = 0 ,
   primera_ocupada = 0,
   vec[tam_vec] = {0};

Semaphore
   libres(tam_vec),
   ocupadas(0),
   impresora(0),
   multiplo(0);

atomic<unsigned> num_ocupadas(0);

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      sem_wait( libres );
      vec[primera_libre] = dato;
      primera_libre = ( primera_libre + 1 ) % tam_vec;
      num_ocupadas.fetch_add(1);
      if ( dato % 5 == 0 ) {
         sem_signal( impresora );
         sem_wait( multiplo );
      }
      sem_signal( ocupadas );
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
      sem_wait( ocupadas );
      dato = vec[primera_ocupada];
      primera_ocupada = ( primera_ocupada + 1 ) % tam_vec;
      consumir_dato( dato ) ;
      num_ocupadas.fetch_add(-1);
      sem_signal( libres );
    }
}
//----------------------------------------------------------------------

void funcion_hebra_impresora(  )
{
   while(true) {
      sem_wait( impresora );
      cout << "Numero de celdas ocupadas: " << num_ocupadas << endl;
      sem_signal( multiplo );
   }
}

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO )." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora ),
          hebra_impresora( funcion_hebra_impresora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;
   // hebra_impresora.join();


   test_contadores();

   cout << endl;
   cout << "-----------------------------------------------------------------" << endl
        << "                           FIN                                   " << endl
        << "------------------------------------------------------------------" << endl;
}
