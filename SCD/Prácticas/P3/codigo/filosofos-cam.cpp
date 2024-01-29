// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: mpicxx -std=c++11 -o filosofos-cam filosofos-cam.cpp
// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,              // número de filósofos 
   num_filo_ten  = 2*num_filosofos, // número de filósofos y tenedores
   id_cam        = num_filo_ten,
   num_procesos  = num_filo_ten + 1,   // número de procesos total (por ahora solo hay filo y ten)
   etiq_coger    = 0 ,
   etiq_soltar   = 1 ,
   etiq_sentarse = 2 ,
   etiq_levantarse = 3;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_filo_ten, //id. tenedor izq.
      id_ten_der = (id+num_filo_ten-1) % num_filo_ten; //id. tenedor der.

   int valor = 1;

  while ( true )
  {
      // Solicita sentarse al camarero
      cout <<"Filósofo " <<id << " solicita sentarse" << endl;
      MPI_Ssend( &valor, 1, MPI_INT, id_cam, etiq_sentarse, MPI_COMM_WORLD);

      // Solicita tenedor izquierda 
      cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
      MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, etiq_coger, MPI_COMM_WORLD);

      // Solicita tenedor derecha
      cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
      MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, etiq_coger, MPI_COMM_WORLD);

      // comiendo (espera bloqueada)
      cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
      sleep_for( milliseconds( aleatorio<10,100>() ) );

      // Suelta el tenedor izquierda
      cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
      MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, etiq_soltar, MPI_COMM_WORLD);

      // Suelta el tenedor derecha
      cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
      MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, etiq_soltar, MPI_COMM_WORLD);

      // Solicita levantarse al camarero
      cout <<"Filósofo " <<id << " solicita levantarse" << endl;
      MPI_Ssend( &valor, 1, MPI_INT, id_cam, etiq_levantarse, MPI_COMM_WORLD);

      // Piensa (espera bloqueada aleatorio del proceso)
      cout << "Filosofo " << id << " comienza a pensar" << endl;
      sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
      // Recibe la peticion del filosofo
      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_coger, MPI_COMM_WORLD, &estado );
      id_filosofo = estado.MPI_SOURCE;
      cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

      // Espera a que el filosofo suelte el tenedor
      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_soltar, MPI_COMM_WORLD, &estado );
      cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

void funcion_camarero()
{
   int filo_sentados = 0;
   int valor;
   int etiq_aceptable;
   MPI_Status estado;
   while ( true )
   {
      if ( filo_sentados < 4)
         etiq_aceptable = MPI_ANY_TAG;
      else
         etiq_aceptable = etiq_levantarse;
      
      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado);

      switch( estado.MPI_TAG )
      {
         case etiq_levantarse:
            cout << "Cam. deja que filo. " << estado.MPI_SOURCE << " se levante" << endl;
            filo_sentados--;
            break;

         case etiq_sentarse:
            cout << "Cam. deja que filo. " << estado.MPI_SOURCE << " se siente" << endl;
            filo_sentados++;
            break;
      }
   }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio == id_cam)              // si es el id del camarero
         funcion_camarero();
      else if ( id_propio % 2 == 0 )         // si es par
         funcion_filosofos( id_propio );     //   es un filósofo
      else                                   // si es impar
         funcion_tenedores( id_propio );     //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
