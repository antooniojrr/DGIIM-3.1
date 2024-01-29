// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: mpicxx -std=c++11 -o filosofo_1 filosofo_1.cpp
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
   num_procesos  = num_filo_ten ,   // número de procesos total (por ahora solo hay filo y ten)
   etiq_coger    = 0 ,
   etiq_soltar   = 1 ;

const string nombres_filo[num_filosofos] = {"Pitagoras","Socrates","Platon","Aristoteles","Epicuro"}; // Nombres filósofos
const string nombres_ten[num_filosofos] = {"T0","T1","T2","T3","T4"};                                 // Ident. Tenedores


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
   string nombre = nombres_filo[id/2];                         // Guardamos el nombre del filósofo para no hacer accesos demás al array
   string nombre_ten_der = nombres_ten[(id_ten_der-1)/2];      // Guardamos el identificador del tenedor de la derecha
   string nombre_ten_izq = nombres_ten[(id_ten_izq-1)/2];      // Guardamos el identificador del tenedor de la izquierda

   while ( true )
   {
      // ---------------------------------------------------------------------
      // Paso 1: Hambre
      cout << "Filosofo " << nombre << " tiene hambre y va a comenzar a comer." << endl << flush;

         // Solicita primer tenedor
      cout <<"Filosofo " << nombre << " solicita el 1er tenedor " << nombre_ten_izq << " (izq.)" << endl << flush;
      MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, etiq_coger, MPI_COMM_WORLD);

         // Solicita segundo tenedor
      cout <<"Filosofo " << nombre <<" solicita el 2do tenedor " << nombre_ten_der << " (der.)" <<endl << flush;
      MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, etiq_coger, MPI_COMM_WORLD);

      // ---------------------------------------------------------------------
      // Paso 2: Comer
      cout <<"Filosofo " << nombre <<" está comiendo" <<endl ;
      sleep_for( milliseconds( aleatorio<10,100>() ) );     // Bloqueo de duración aleatoria

      // ---------------------------------------------------------------------
      // Paso 3: Soltar tenedores

         // Suelta el primer tenedor
      cout <<"Filosofo " << nombre <<" suelta el 1er tenedor " << nombre_ten_izq << " (izq.)" <<endl << flush;
      MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, etiq_soltar, MPI_COMM_WORLD);

         // Suelta el segundo tenedor
      cout<< "Filosofo " << nombre <<" suelta el 2do tenedor " << nombre_ten_der << " (der.)" <<endl << flush;
      MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, etiq_soltar, MPI_COMM_WORLD);

      // ---------------------------------------------------------------------
      // Paso 4: Pensar
      cout << "Filosofo " << nombre << " esta pensando si tiene hambre" << endl << flush;
      sleep_for( milliseconds( aleatorio<10,100>() ) );     // Bloqueo de duración aleatoria
      // ---------------------------------------------------------------------
   }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  string nombre = nombres_ten[(id-1)/2];     // Guardamos el identificador del tenedor

  while ( true )
  {
      // Esperamos a que alguien solicite coger el tenedor
      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_coger, MPI_COMM_WORLD, &estado );
      id_filosofo = estado.MPI_SOURCE;     // Guardamos el filósofo que lo ha cogido
      cout <<"Ten. " << nombre <<" ha sido cogido por filo. " << nombres_filo[id_filosofo/2] <<endl << flush;

      // Esperamos a que lo suelte
      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_soltar, MPI_COMM_WORLD, &estado );
      cout <<"Ten. "<< nombre << " ha sido liberado por filo. " << nombres_filo[id_filosofo/2] <<endl << flush;
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
      if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
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
