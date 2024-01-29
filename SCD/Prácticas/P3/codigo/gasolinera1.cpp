// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: mpicxx -std=c++11 -o gasolinera1 gasolinera1.cpp
// -----------------------------------------------------------------------------

#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;


//**********************************************************************
// Variables globales

const int num_coches 		= 7;			// Número de coches
const int id_gasolinera 	= num_coches;	// Identificador del proceso gasolinera
const int etiq_adquirir 	= 0;			// Etiqueta para adquirir surtidor
const int etiq_liberar 		= 1;			// Etiqueta para liberar surtidor
const int num_surtidores	= 3;			// Número de surtidores
const int num_proc 			= num_coches + 1; // Número total de procesos

//----------------------------------------------------------------------
/// @brief plantilla de función para generar un entero aleatorio uniformemente
/// distribuido entre dos valores enteros, ambos incluidos
/// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
/// @tparam min - valor mínimo (int)
/// @tparam max - valor máximo (int)
/// @return número 'int' aleatorio uniformemente distribuido entew 'min' y 'max', ambos incluidos
///
template <int min, int max>
int aleatorio()
{
   static default_random_engine generador((random_device())());
   static uniform_int_distribution<int> distribucion_uniforme(min, max);
   return distribucion_uniforme(generador);
}


//**********************************************************************
// Funciones hebras

void funcion_gasolinera(){

	
	int surtidores_disponibles = num_surtidores;
	int etiq_permitida,
		valor;
	MPI_Status estado;
	
	while(true){
		
		if(surtidores_disponibles == 0)
			etiq_permitida = etiq_liberar;
		else
			etiq_permitida = MPI_ANY_TAG;
		
		MPI_Recv ( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_permitida, MPI_COMM_WORLD, &estado );
		
		switch( estado.MPI_TAG ){

			case etiq_adquirir:
				cout << "El coche " << estado.MPI_SOURCE << " ha empezado a repostar " << endl;
				surtidores_disponibles--;
				break;

			case etiq_liberar:
				cout << "El coche " << estado.MPI_SOURCE << " ha terminado de repostar " << endl;
				surtidores_disponibles++;
				break;
		}
	}
}

void funcion_coche(int id){
	
	int valor = 0;
	
	while( true ){
		
		cout << "El coche "<< id << " está solicitando entrar a algún surtidor " << endl;

      	MPI_Ssend( &valor, 1, MPI_INT, id_gasolinera, etiq_adquirir, MPI_COMM_WORLD );
      	this_thread::sleep_for( chrono::milliseconds( aleatorio<20,200>() ) );
      	MPI_Ssend( &valor, 1, MPI_INT, id_gasolinera, etiq_liberar, MPI_COMM_WORLD );
		
		// Retraso aleatorio antes de volver a la gasolinera
		chrono::milliseconds duracion_produ( aleatorio<10,100>() );
		this_thread::sleep_for( duracion_produ );
	}
}

//**********************************************************************

int main( int argc, char** argv )
{
   int id_propio, num_proc_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_proc_actual );


   if ( num_proc == num_proc_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio == id_gasolinera )          // si es la hebra 0
         funcion_gasolinera(); 					 //   es la gasolinera
      else                              		 // si no es la hebra 0
         funcion_coche( id_propio );			 //   es un coche de gasolina
   }

   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cerr << "el número de procesos esperados es:    " << num_proc << endl
             << "el número de procesos en ejecución es: " << num_proc_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}
