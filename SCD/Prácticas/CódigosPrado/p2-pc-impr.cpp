#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int
   num_items = 30 ;   // número de items a producir/consumir
int
   siguiente_dato = 0 ; // siguiente valor a devolver en 'producir_dato'
   
constexpr int               
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for

mutex    mtx ;                 // mutex de escritura en pantalla


 const int num_consumidoras=3, // Número de hebras consumidora
           num_productoras=2; // Número de hebras productoras


//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------



//**********************************************************************
unsigned producir_dato(const int num_hebra, int j)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   const int dato=num_hebra*num_items/num_productoras+j;
   mtx.lock();
   cout << "producido: " << dato << endl << flush ;
   mtx.unlock();
   return dato ;
}


//----------------------------------------------------------------------
void consumir_dato( const int num_hebra, unsigned dato )
{
 
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
   cout << "                  hebra consumidora " <<num_hebra<<" consume: " << dato << endl<<flush ;
   mtx.unlock();
}

//----------------------------------------------------------------------


// *****************************************************************************
// clase para monitor buffer, version FIFO, semántica SU, multiples prod/cons
class Buffer : public HoareMonitor
{
 private:
 static const int           // constantes ('static' ya que no dependen de la instancia)
   num_celdas_total = 5;   //   núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//   buffer de tamaño fijo, con los datos
   primera_libre ,        
   primera_ocupada,	        
   n;		     //   Contador de items disponibles para leer
   int num_multiplos_5;
   int num_multiplos_5_impresora;

 CondVar                    // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres,                 //  cola donde espera el productor  (n<num_celdas_total)
   cola_impresora;         // (num_multiplos_5>num_multiplos_5_impresora)
 public:                    // constructor y métodos públicos
   Buffer() ;             // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
   bool metodo_impresora();
} ;
// -----------------------------------------------------------------------------

Buffer::Buffer(  )
{
   primera_libre = 0 ;
   primera_ocupada = 0;
   n = 0;
   num_multiplos_5=0;
   num_multiplos_5_impresora=0;
   ocupadas      = newCondVar();
   libres        = newCondVar();
   cola_impresora= newCondVar();  
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int Buffer::leer(  )
{
   // esperar bloqueado hasta que 0 < primera_libre
   //Cambiamos la condicion para el funcionamiento correcto
   if ( n == 0 )
      ocupadas.wait();

   //cout << "leer: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   //assert( 0 < primera_libre  );

   // hacer la operación de lectura, actualizando estado del monitor
   const int valor = buffer[primera_ocupada] ;
   primera_ocupada = (primera_ocupada + 1) % num_celdas_total;;
   n--;
   
   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void Buffer::escribir( int valor )
{
   // esperar bloqueado hasta que primera_libre < num_celdas_total
   //Cambiamos la condicion para el funcionamiento correcto
   if( n == num_items)
      libres.wait();

   // Insercion, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   primera_libre = (primera_libre + 1) % num_celdas_total;
   n++;
   if (valor%5==0) {
      num_multiplos_5++;
      cola_impresora.signal();
   }
   // Avisar de que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}



bool Buffer::metodo_impresora( )
{
   // esperar bloqueado hasta que primera_libre < num_celdas_total
   //Cambiamos la condicion para el funcionamiento correcto
   if(num_multiplos_5==num_multiplos_5_impresora)
      cola_impresora.wait();
   cout<<"********************* Se han detectado "
      <<num_multiplos_5-num_multiplos_5_impresora 
      <<" nuevos múltiplos"<<endl<<flush;
   num_multiplos_5_impresora=num_multiplos_5;

   return(num_multiplos_5<num_items/5);

}


// *****************************************************************************
// funciones de hebras
// *****************************************************************************

// -----------------------------------------------------------------------------
void funcion_hebra_productora( MRef<Buffer> monitor, const int num )
{
   for( unsigned j = 0 ; j < num_items/num_productoras ; j++ )
   {
      int dato = producir_dato(num,j) ;
      monitor->escribir( dato );
   }
}


// -----------------------------------------------------------------------------
void funcion_hebra_consumidora( MRef<Buffer>  monitor, const int num )
{
   for( unsigned j = 0 ; j < num_items/num_consumidoras ; j++ )
   {
      int valor = monitor->leer();
      consumir_dato( num,valor ) ;
   }
}


// -----------------------------------------------------------------------------
void funcion_hebra_impresora( MRef<Buffer>  monitor )
{  bool valor=true;
   while(valor)
   {
      this_thread::sleep_for( chrono::milliseconds( aleatorio<50,100>() ));
      valor = monitor->metodo_impresora();
   }
}

// -----------------------------------------------------------------------------

int main()
{
  
   cout << "--------------------------------------------------------------------" << endl
        << "   Problema de productores-consumidores  (Monitor SU, buffer FIFO).    " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   MRef<Buffer> monitor = Create<Buffer>() ;
   thread hebra_prod[num_productoras], hebra_cons[num_consumidoras]; 
   thread hebra_impresora;
   
   for( unsigned i = 0 ; i < num_productoras ; i++ )
      hebra_prod[i]=thread ( funcion_hebra_productora, monitor,i );

   for( unsigned i = 0 ; i < num_consumidoras ; i++ )
      hebra_cons[i]=thread (funcion_hebra_consumidora, monitor,i );
   hebra_impresora=thread( funcion_hebra_impresora, monitor);


   // esperar a que terminen las hebras
   for( unsigned i = 0 ; i < num_productoras ; i++ )
      hebra_prod[i].join();
   for( unsigned i = 0 ; i < num_consumidoras ; i++ ) 
      hebra_cons[i].join();
   hebra_impresora.join();

}
