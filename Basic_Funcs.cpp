#include "classes_BPGC_HetBins.h"

vector<PIEZA>
read_pieces (char* name)
{
  vector<PIEZA> sol;
  char name_sol[80];
  strcpy (name_sol, name);
  strcat (name_sol, ".dat");

  char cadena[128];

  ifstream p_file (name_sol);
  if (!p_file.is_open ())
    {
      cout << "ERROR: Unable to open " << name_sol << " file\n";
      exit (1);
    }

  //Lee la primera fila de los archivos de datos, que contiene una línea de información
  PIEZA p_actual;
  p_file >> cadena; // polyNo
  p_file >> cadena; // pointNo
  p_file >> cadena; // X
  p_file >> cadena; // Y
  int pieza_anterior = -1;
  p_actual.setID (1); //Crea el ID de la pieza, inicialmente =1
  int v_anterior = -1;
  while (!p_file.eof ()) //Mientras no llegue al fin del archivo (end_of_file)
    {
      int pieza;
      p_file >> cadena;
      sscanf (cadena, "%d", &pieza); //Número de pieza
      if (pieza_anterior == -1)
	pieza_anterior = pieza;
      if (pieza == pieza_anterior)
	{
	  PUNTO ppp;
	  int v_actual;
	  p_file >> cadena;
	  sscanf (cadena, "%d", &v_actual); //Numero de punto de la pieza
	  p_file >> cadena;
	  sscanf (cadena, "%lf", &ppp.coordx);
	  p_file >> cadena;
	  sscanf (cadena, "%lf", &ppp.coordy);
	  if (v_actual != v_anterior)
	    {
	      v_anterior = v_actual;
	      p_actual.add_point (ppp);
	    }
	}
      else
	{
	  // AÑADIMOS pieza al vector solución:
	  sol.push_back (p_actual);
	  // COMENZAMOS CON LA SIGUIENTE
	  p_actual.clear_points ();
	  p_actual.setID (pieza);
	  PUNTO ppp;
	  p_file >> cadena;
	  p_file >> cadena;
	  sscanf (cadena, "%lf", &ppp.coordx);
	  p_file >> cadena;
	  sscanf (cadena, "%lf", &ppp.coordy);
	  p_actual.add_point (ppp);
	  pieza_anterior++;
	}
    }
  // AñADIMOS LA ULTIMA pieza
  sol.push_back (p_actual);
  for (int i = 0; i < sol.size (); i++)
    {
      // INICIALIZAMOS PIEZAS:
      sol[i].initialize_piece ();
    }
  return sol;
}

int
argmax_mod (vector<EDGES> &a)
{
  int argmax = 0;
  double max = a[0].mod;
  for (int i = 1; i < a.size (); i++)
    {
      if (max < a[i].mod)
	{
	  max = a[i].mod;
	  argmax = i;
	}
    }
  return argmax;
}

bool
equal_double (double a, double b, double prec)
{
  if (a >= b - prec && a <= b + prec)
    return true;
  else
    return false;
}

bool
equal_pto (PUNTO a, PUNTO b)
{
  if (equal_double (a.coordx, b.coordx, TOL)
      && equal_double (a.coordy, b.coordy, TOL))
    return true;
  else
    return false;
}

double
distancia (PUNTO p1, PUNTO p2)
{
  double dist;
  dist = sqrt (pow (p2.coordy - p1.coordy, 2) + pow (p2.coordx - p1.coordx, 2));
  return dist;
}

int
Position (PUNTO A, PUNTO B, PUNTO C)
{
  int pos = 100;
  double crosprod;
  //Avoid numerical errors, by rounding for comparison  pourposes
  //=============================================================
  if (fabs (A.coordx - C.coordx) < 0.01 && fabs (A.coordy - C.coordy) < 0.01)//A & C can be considered the same point
    C = A;
  if (fabs (B.coordx - C.coordx) < 0.01 && fabs (B.coordy - C.coordy) < 0.01)//B & C can be considered the same point
    C = B;
  //=============================================================
  crosprod = (B.coordx - A.coordx) * (C.coordy - A.coordy)
      - (B.coordy - A.coordy) * (C.coordx - A.coordx);
  if (crosprod > 0 + TOL)
    return 1;	//C is left of vector AB
  if (crosprod < 0 - TOL)
    return -1;	//C is right of vector AB
  if (equal_double (crosprod, 0, TOL))
    return 0;	//Alignded
  return pos;	//if returns 100 something is wrong.
}

bool
right_angles (vector<double> angle)
{
  bool has_right = false;
  int count = 0;
  while (count < angle.size () && !has_right)
    {
      if (equal_double (angle[count], 90, ALPHA))
	has_right = true;
      count++;
    }

  return has_right;
}

bool
lexicographic (PUNTO P1, PUNTO P2)
{
  return (P1.coordx < P2.coordx
      || (P1.coordx == P2.coordx && P1.coordy < P2.coordy));
}
