// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: g++ -std=c++11 -pthread -o prodconsmult-fifo prodconsmult-fifo.cpp scd.cpp

// Comentarios:
// - Variables usadas: primera_libre, marcará la primera posición disponible para escribir un dato y, por tanto, la inmediatamente anterior será el último dato escrito
//                     primera_ocupada, marca la primera posición del vector ocupada por un dato
// - Semáforos: libres, controla que no se vaya a guardar un dato hasta que no haya espacio libre para este
//              ocupados, controla que no se vaya a consumir un dato hasta que no haya alguno disponible para consumir

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"
#include <atomic>

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 50 ,   // número de items
	tam_vec   = 10 ,   // tamaño del buffer
   num_prod  = 5  ,
   num_cons  = 5  ;

unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato[num_prod]   ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

atomic<unsigned>
   primera_libre(0) ,
   primera_ocupada(0);

unsigned
   vec[tam_vec] = {0};

Semaphore
   libres(tam_vec),
   ocupadas(0);

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(int i)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato[i] ;
   siguiente_dato[i]++ ;
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

void  funcion_hebra_productora( int id )
{
   int fin = num_items/num_prod;
   for( unsigned i = 0 ; i < fin ; i++ )
   {
      int dato = producir_dato(id) ;
      sem_wait( libres );
      vec[primera_libre] = dato;
      primera_libre = ( primera_libre + 1 ) % tam_vec;
      sem_signal( ocupadas );
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   int fin = num_items/num_cons;
   for( unsigned i = 0 ; i < fin ; i++ )
   {
      int dato ;
      sem_wait( ocupadas );
      dato = vec[primera_ocupada];
      primera_ocupada = ( primera_ocupada + 1 ) % tam_vec;
      consumir_dato( dato ) ;
      sem_signal( libres );
    }
}
//----------------------------------------------------------------------

int main()
{

   for(int i = 0; i < num_prod; i++)
   	siguiente_dato[i] = (num_items/num_prod)*i;

   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores multiples (solución FIFO )." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebras_productora[num_prod],
          hebras_consumidora[num_cons];

   for ( int i = 0; i < num_prod; i++) 
      hebras_productora[i] = thread(funcion_hebra_productora,i);
   
   for ( int i = 0; i < num_cons; i++) 
      hebras_consumidora[i] = thread(funcion_hebra_consumidora);

   for ( int i = 0; i < num_prod; i++) 
      hebras_productora[i].join();
   
   for ( int i = 0; i < num_cons; i++) 
      hebras_consumidora[i].join();

   test_contadores();

   cout << endl;
   cout << "-----------------------------------------------------------------" << endl
        << "                           FIN                                   " << endl
        << "------------------------------------------------------------------" << endl;
}
