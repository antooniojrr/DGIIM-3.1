// Nombre y apellidos: Antonio Javier Rodríguez Romero
// DNI: 77432800B
// Compilado con: mpicxx -std=c++11 -o gasolinera2 gasolinera2.cpp
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

const int num_coches 						= 14;				// Número de coches 
const int id_gasolinera 					= num_coches;		// Identificador del proceso gasolinera
const int num_tipos_com 					= 3;				// Tipos de combustible
const int surtidores[num_tipos_com] 		= {3,2,1};			// Número de surtidores de cada tipo
constexpr int etiq_adquirir[num_tipos_com] 	= {0,1,2};			// Etiquetas para adquirir cada tipo de surtidor
const int etiq_liberar 						= num_tipos_com;	// Etiqueta para liberar un surtidor
const int num_procesos 						= num_coches + 1;	// Número de procesos total

int surtidores_libres[ num_tipos_com ] = { surtidores[0], surtidores[1], surtidores[2] };
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
// Funciones de cada proceso

void funcion_gasolinera(){

	int flag,
		valor,
		contador_mensajes;

	MPI_Status estado;
	
	while(true){

		contador_mensajes = 0;

		// Comprueba si hay mensajes de fin de repostar disponibles
		MPI_Iprobe( MPI_ANY_SOURCE, etiq_liberar, MPI_COMM_WORLD, &flag, &estado );
		
		if(flag > 0){
		
			MPI_Recv ( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_liberar, MPI_COMM_WORLD, &estado );
			
			surtidores_libres[valor]++;
			contador_mensajes++;
			cout << "Gasolinera: procesado 'fin de repostar' de coche " << estado.MPI_SOURCE << " para surtidor tipo " << valor << endl ;
		}
		
		// Comprueba si hay mensajes de comenzar a repostar disponibles
		for(int i = 0; i < num_tipos_com; i++){
			if (surtidores_libres[i] > 0) {

				MPI_Iprobe( MPI_ANY_SOURCE, etiq_adquirir[i], MPI_COMM_WORLD, &flag, &estado );
				
				if(flag > 0){
				
					MPI_Recv ( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_adquirir[i], MPI_COMM_WORLD, &estado );
					surtidores_libres[i]--;
					contador_mensajes++;
					cout << "Gasolinera: procesado 'inicio de repostar' de coche " << estado.MPI_SOURCE << " para surtidor tipo " << i << endl ;
				}
			}
		}
		
		if(contador_mensajes == 0){
			
			// Si no hay mensajes aceptables, espera 20 milisegundos
			
			chrono::milliseconds duracion_produ( 20 );
			this_thread::sleep_for( duracion_produ );
		}
		
	}
}

void funcion_coche(int id){
	
	// Asignamos el tipo de combustible que utilizará el coche
	const int tipo_combustible = aleatorio<0,num_tipos_com-1>();
	
	while( true ){
		
		// Mandamos un mensaje síncrono para adquirir un surtidor
		MPI_Ssend( &tipo_combustible, 1, MPI_INT, id_gasolinera, etiq_adquirir[tipo_combustible], MPI_COMM_WORLD );

		cout << "El coche "<< id << " inicia repostaje en surtidor tipo " << tipo_combustible << endl;
      	this_thread::sleep_for( chrono::milliseconds( aleatorio<20,200>() ) );
      	cout << "El coche "<< id << " acaba repostaje en surtidor tipo " << tipo_combustible << endl;
		
		// Mandamos un mensaje síncrono para liberar un surtidor
		MPI_Ssend( &tipo_combustible, 1, MPI_INT, id_gasolinera, etiq_liberar, MPI_COMM_WORLD );
		

		// Retraso aleatorio antes de volver a la gasolinera
		chrono::milliseconds duracion_produ( aleatorio<10,100>() );
		this_thread::sleep_for( duracion_produ );
	}
}

//**********************************************************************

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio == id_gasolinera )          // si es la hebra 0
         funcion_gasolinera(); 					 //   es la gasolinera
      else                               		 // si no es la hebra 0
         funcion_coche( id_propio ); 			 //   es un coche de gasolina
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
