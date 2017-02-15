#include <string>
#include <tuple>

//##############################################
//########## DEFINICION DE CLASES ##############
//##########     Y ESTRUCTURAS    ##############
//##############################################
#define _CRT_SECURE_NO_WARNINGS//I think this is only necessary in Windows. 

#include <tuple>
#include <time.h>
#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <assert.h>
#include <algorithm>

#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>

//Define constants
#define GRANDE 99999999
#define TOL  0.000001 //Tolerancia para evitar errores numéricos
#define TOL2 0.0001  //0.005 para albano // 0.00001 para dados_t2
#define ALPHA 2.5 // Umbral para definir angulos rectos [87.5, 92.5]
#define PI 3.14159265358979323846
#define NH 3 //Número de tipos de bin. 
#define THRES 0.5 //This is the acceptance threshold for a good match. If both edges of pieces match in length in more than 50% we accept the match. (This will be reviewed)
#define THRES2 0.8 //This is the acceptance threshold for placing a piece in the candidate list. We have different thresholds, because the final acceptance criterion is based in convex hull utilization, and maybe by matching a "not so good" edge gives better utilization. In any case it will always try the edge with the max matching distance, so wer are only allowing for more tests to be made.

#define slide_ep 10//This value we will have to explore as I don't have a clue on how much to slide a piece along other piece...
#define PARAM_LAST_BIN 0.05 //We will try with 0.2, 0.1, 0.05 y 0.01

using namespace std;
//Para no tener que repetir siempre std:: con los comandos standar
using std::vector;
//Para poder usar vector sin tener que escribir std::vector::...

// To compare in the global eval
class GE_COMP
{
public:
  double fractional_n_bins;
  double fractional_util;
  double prop_used_current_bin;
};

//PUNTO
class PUNTO
{
public:
  double coordx;
  double coordy;
};

class EDGES
{
public:
  PUNTO ini;
  PUNTO fin;
  double mod; //Stores the lenght of the edge.
  void
  set_mod ();
};

class NUMERO
{
public:
  int parte_entera;
  int decimal_1; // digito del 0 al 9
  int decimal_2; // digito del 0 al 9
};

class PIEZA
{
private:
  int ID; //ID de la pieza
  int bin; // numero de bin al que pertenece
  double area; //área de la pieza
  bool placed; //True if piece is placed in a bin, false if not.
  vector<PUNTO> puntos_angulo_inicial; // puntos del polígono inicial
  vector<PUNTO> puntos_angulo_usado; // puntos del polígono rotado. Van cambiando a medida que hacemos las diferentes rotaciones
  vector<EDGES> aristas; //aristas de la pieza
  vector<double> angulos_pieza; //Contiene los ángulos de la pieza en sentido antihorario
  double base; //lenght of the longest edge of the piece.
  double ancho; // ancho de la pieza
  double largo; //largo de la pieza
  int no_vertices; //numero de vertices de la pieza
  PUNTO Coord; //Coordenadas del punto de referencia de la pieza. Inicialmente será el (0,0) en todas, luego depende de la posición de la pieza en el bin.
  bool mirror; //True si la pieza es simétrica (no hay necesidad de hacer una reflexión), false si no lo es. Inicializa a False
  double angulo_usado; // guarda el ángulo que se utilizó en la rotación
public:
  //Funciones de la clase PIEZA (publicas)
  void
  setID (int id); //asigna el ID de la pieza
  int
  getID (); //Obtiene el ID de la pieza
  void
  setbin (int i); //Asigna el id del bin en el que está la pieza
  int
  getBIN (); //Obtiene el ID del bin en el que está la pieza
  void
  add_point (PUNTO p); //añade los puntos de la pieza
  int
  get_noVert (); //Gets the number of vertices of the piece.
  double
  getl ();
  double
  getw ();
  double
  getArea ();
  double
  get_angulo_usado ();
  void
  setCoord (PUNTO p);
  PUNTO
  getCoord ();
  vector<EDGES> *
  getEdges ();
  void
  ini_edges ();
  vector<double>
  get_angles ();
  void
  clear_points (); //Borra todos los puntos de la pieza para poder iniciar otra.
  void
  initialize_piece (); //Inicializa una pieza
  vector<PUNTO> *
  obtener_puntos ();
  bool
  obtener_rotacion (double angulo_a_rotar); // devuelve la rotación (devuelve el vector de vértices rotados de la pieza) y false si coinciden con los originales (rotación no hace nada)
  void
  translate (double x, double y); //This function translate the piece (x,y) units.
  void
  write_coords ();
  bool
  get_mirror (); //Obiene el valor de mirror.
  bool
  set_mirror (bool v); //Realiza la reflexión. Devuelve false si el mirror no cambia la orientación de la pieza.
  bool
  Fits_Bin (double L, double W); //Checa si la pieza entra en el rectángulo del bin.
  void
  setbase (vector<EDGES> aristas); //Set the longest edge as base.
  double
  getbase ();
  void
  set_placed (int b);
  bool
  is_placed ();
};

//Irregular bin
class IRR_BIN
{
private:
  int ID; //Independent identifier for the irregular bin
  int binID; //Bin ID to which the irregular bin is associated.
  double area_ib; //Total area of the irregular bin.
  double waste_ib; //free area
  double prop_used; //proportion of the bin used.
  vector<PUNTO> ib_ptos; //vertices of the bin in clockwise order.
  vector<double> ib_angles;
  vector<PIEZA*> ib_pzas; //Pieces that are included in the irr_bin
  vector<EDGES> ib_GC; //Guillotine cuts inside the irregular bin
public:
  void
  set_ID (int i);
  int
  getID ();
  void
  setBin (int b);
  void
  setArea ();
  double
  getArea ();
  int
  getBin (); //Returns the value of binID;
  void
  set_prop_used ();
  double
  get_prop_used ();
  void
  set_waste (PIEZA &p);
  void
  update_waste ();
  void
  empty_irrbin ();
  double
  get_waste ();
  void
  add_piece (PIEZA &p);
  void
  add_points (PUNTO &pto);
  void
  add_GC (EDGES &e);
  vector<double>
  get_angles ();
  vector<PUNTO> *
  get_ptos ();
  vector<PIEZA*> *
  get_pzas ();
  vector<EDGES> *
  get_ibGC ();
};

// NODOS
class NODE
{
private:
  int ID;
  int level;
  NODE *predecessor;
  int num_piezas;
  double global_eval;
  double local_eval;
  string SizeID; //Identifier for the Size of the bin
  double L; //largo del bin. Se asume L>W.
  double W; //Ancho del bin
  double waste; // area libre
  double prop_utilizada; //proporción del bin utilizada
  bool bin_open; //True if there is the possibility of placing more items in this bin. False otherwise.
  int num_sect; //number of sections in which the bin is being divided (0 = no divisions)
  PIEZA piezas_incluidas[100]; //piezas incluidas en el bin.
  vector<EDGES> GC;
  vector<IRR_BIN> section;
  vector<int> ID_pzas_disp; //Piezas disponibles para colocar en ese nodo.

public:
  void
  setID (int i); //crea el ID del bin
  int
  getID () const; //recupera el ID del bin

  void
  set_globaleval (double of);
  double
  get_globaleval () const;

  void
  set_localevaluation (double of);
  double
  get_localevaluation () const;

  double
  get_waste () const;

  void
  set_SizeID (string s);
  void
  setDim (const char*name, string s); //Asigna largo y ancho del bin
  void
  initialize_node (const char*name, int ID, string s);
  void
  IniWaste ();
  double
  getL ();
  double
  getW ();
  string
  getSize ();
  void
  setOpen (bool op);
  bool
  getOpen ();
  double
  getL_used (); //recupera la longitud utilizada
  double
  getW_used (); //recupera el ancho utilizado
  void
  setWaste (PIEZA &p); //calcula el area libre
  void
  setPropUtil (); //Calcula la proporción utilizada
  double
  getPropUtil ();
  int
  getNumPiezas ();
  void
  include_piece (PIEZA p); //Incluye una copia de la pieza, pues la misma pieza se va a incluir en varios nodos.
  void
  add_GC (EDGES e);
  vector<EDGES> *
  getGC ();
  vector<PIEZA*>
  getPI ();
  void
  empty_bin ();
  void
  add_IB (IRR_BIN &ib);
  int
  getNumSect ();
  IRR_BIN
  get_ib (int i);
  void
  update_section (IRR_BIN &ib); //Updates the pieces of section(ib) already  in  the bin.
  void
  update_ptos_sect (IRR_BIN &ib); //update the points of sectino ib already in the bin.
  void
  add_piece_SectToBin (IRR_BIN &ib); //Adds a piece from a given section (ib) to the bin
  //=========================
  //Functions related to NODE as a node in a tree, not as a bin object
  void
  set_pred (NODE &pred);
  void
  set_ID_pzas_disp (vector<PIEZA> &pz);
  void
  update_pzas_disp (int ID);
  vector<int>
  get_IDdisp ();
  NODE *
  get_Pred ();
  void
  set_level (int i);
  int
  get_level ();
  void
  FixUtil (double d);
  void
  FixDim (double w, double l);
  void
  CopyPiecesInSect ();
};

//TREE
class TREE
{
private:
  list<NODE> BS_tree;
  int alpha;
  int beta;

  //
  // setting up the best global solution
  //

  // the list of computed nodes in the best solution
  //
  list<NODE> best_current_solution;

  // the list of estimated nodes in the best solution
  //
  list<NODE> best_estimated_solution;

  // the global evaluation of the best solution
  //
  double best_global_evaluation = 0;

  // the tuple holding the best solution
  //
  std::tuple<list<NODE> &, list<NODE> &, double &> best_solution;

public:

  TREE () :
      alpha (0), beta (0), best_solution (
	  std::tie (best_current_solution, best_estimated_solution,
		    best_global_evaluation)) {};

  void
  build_solution (const char *argv, vector<PIEZA> &p);

  list<NODE>
  get_tree ();

  void
  del_node (list<NODE>::iterator &node);

  void
  create_child (const char *argv, int NoChild, int Rots, NODE &current_node,
		vector<PIEZA> &pzas, double &A);

  bool
  local_eval (vector<NODE> &c, NODE &c_node, int t, double &A);	//

  void
  global_eval (const char *argv, vector<PIEZA> &p);

  // return the best solution
  //
  const std::tuple<list<NODE>, list<NODE>, double>&
  getBestSolution () const
  {
    return best_solution;
  }
};

extern int
argmax_mod (vector<EDGES> &a);
extern bool
equal_double (double a, double b, double prec);
extern double
distancia (PUNTO p1, PUNTO p2);
extern bool
equal_pto (PUNTO p1, PUNTO p2);
extern double
calcular_area_exacta (vector<PUNTO> &poligono);
extern int
Position (PUNTO A, PUNTO B, PUNTO C);
extern vector<double>
calculateInnerAngles (vector<EDGES> &ar);
