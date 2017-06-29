#include "classes_BPGC_HetBins.h"

bool
FitInSect(vector<EDGES> rp, vector<PUNTO> ptos_Sec);

vector<double> calculateInnerAngles(vector<EDGES> &ar) {
	vector<double> angles;
	double angle;
	double numer, denom;
	EDGES aux;
	//Obtain first angle
	long last = ar.size() - 1;
	//Set aux edge
	aux.ini = ar[last].ini;
	aux.fin = ar[0].fin;
	aux.set_mod();
	//Cosine Law a^2 = b^2 + c^2 - 2bccos(angle(bc)).
	numer = pow(ar[last].mod, 2) + pow(ar[0].mod, 2) - pow(aux.mod, 2);
	denom = 2 * ar[last].mod * ar[0].mod;
	angle = acos(numer / denom) * 180.0 / PI;
	angles.push_back(angle);

	//Obtain remaining angles
	for (int i = 1; i < ar.size(); i++) {
		//Set aux edge
		aux.ini = ar[i - 1].ini;
		aux.fin = ar[i].fin;
		aux.set_mod();
		//Cosine law for the remaining of the angles
		numer = pow(ar[i - 1].mod, 2) + pow(ar[i].mod, 2) - pow(aux.mod, 2);
		denom = 2 * ar[i - 1].mod * ar[i].mod;
		angle = acos(numer / denom) * 180.0 / PI;
		angles.push_back(angle);
	}
	return angles;
}

double calculate_rotation_match(EDGES &e1, EDGES &e2) {
	double alpha;
	double u1 = e1.fin.coordx - e1.ini.coordx;
	double u2 = e1.fin.coordy - e1.ini.coordy;
	double v1 = e2.ini.coordx - e2.fin.coordx;
	double v2 = e2.ini.coordy - e2.fin.coordy;

	// CALCULAMOS ANGULO que forman las aristas. (mediante el prod interior)
	double num = (u1 * v1) + (u2 * v2);
	double denom = sqrt((u1 * u1) + (u2 * u2)) * sqrt((v1 * v1) + (v2 * v2));
	double cos_alpha = num / denom;
	if (cos_alpha >= 1 - TOL2) //Tolerance problems if cos_alpha is close to 1, alpha = 0.
		alpha = 0;
	else {
		if (equal_double(cos_alpha, -1.0, TOL))
			cos_alpha = -1.0; //Avoid problems if cos_alpha very close to -1.
		alpha = acos(cos_alpha) * (180 / PI); //alpha is in degrees.
	}
	if (equal_double(alpha, 360.0, TOL2) || equal_double(alpha, 0.0, TOL2)) //e1 & e2 are already matched.
			{
		alpha = 0;
		return alpha;
	}

	// e1 is going to be rotated to match with e2
	double angulo_aux = 2 * PI * alpha / 360; //angulo_aux  = alpha rads.
	double cos_e1 = u1 / e1.mod; //cos of angle (e1, x-axis) (beta)
	double sin_e1 = u2 / e1.mod; //sin of angle (e1, x_axis) (beta)
	double cos_e2 = v1 / e2.mod; //cos of angle (e2, x-axis) 
	double sin_e2 = v2 / e2.mod; //sin of angle (e2, x_axis) 

	cos_alpha = cos(angulo_aux); //cos(alpha)
	double sin_alpha = sin(angulo_aux); //sin(alpha)

	double xx = (cos_e1 * cos_alpha) - (sin_e1 * sin_alpha); //cos(beta + alpha)
	double yy = (cos_e1 * sin_alpha) + (sin_e1 * cos_alpha); //sin(alpha + beta)

	if (equal_double(xx, cos_e2, TOL2) && equal_double(yy, sin_e2, TOL2)) //if alpha + beta = angle(e2, x-axis)  rotate clockwise
			{
		return alpha;
	} else
		//rotate anticlockwise.
		return (360 - alpha);
}

//Calcula la rotación de una pieza por un ángulo alpha, pero solo guarda las aristas, no rota toda la pieza
vector<EDGES> rotation_piece(PIEZA &p, double alpha) {
	vector<PUNTO>::iterator it;
	vector<EDGES> sol;
	vector<PUNTO> *pp;
	pp = p.obtener_puntos();
	vector<PUNTO> ppp;

	double angulo_aux = 2 * PI * alpha / 360;

	for (it = pp->begin(); it != pp->end(); it++) {
		double modulo = sqrt(
				(*it).coordx * (*it).coordx + (*it).coordy * (*it).coordy);
		double tita = PI / 2; // SI x=0 :: y>0
		if (modulo < TOL && modulo > -TOL)
			tita = 0;
		else {
			if ((*it).coordx < TOL && (*it).coordx > -TOL && (*it).coordy < -TOL) // SI x>0 :: y>=0
			{
				tita = 3 * PI / 2;
			}
			if ((*it).coordx > TOL && (*it).coordy >= 0) // SI x>0 :: y>=0
					{
				tita = atan((*it).coordy / (*it).coordx);
			}
			if ((*it).coordx > TOL && (*it).coordy < -TOL) // SI x>0 :: y<0
			{
				tita = atan((*it).coordy / (*it).coordx) + 2 * PI;
			}
			if ((*it).coordx < -TOL) // SI x<0
			{
				tita = atan((*it).coordy / (*it).coordx) + PI;
			}
		}

		PUNTO p_aux;
		p_aux.coordx = modulo * cos(angulo_aux + tita);
		p_aux.coordy = modulo * sin(angulo_aux + tita);
		ppp.push_back(p_aux);
	}
	for (int i = 0; i < ppp.size(); i++) {
		EDGES e_aux;
		e_aux.ini = ppp[i];
		if (i < ppp.size() - 1)
			e_aux.fin = ppp[i + 1];
		else
			e_aux.fin = ppp[0];
		sol.push_back(e_aux);
	}
	return sol;
}

//Calcula un vector de posibles rotaciones para que coincida con los lados de un bin.

vector<double> calculate_rotations_FitSection_Mirror(vector<EDGES> &ed_irrbin,
		PIEZA &p, int num_rot, double epsilon) {
	if (p.get_angulo_usado() != 0) //if piece is rotated, then place piece in original position.
			{
		p.obtener_rotacion(0);
	}
	vector<double> g_angles; //Vector of possible rotations
	vector<int> g_matchings; //number of edges that match
	vector<double> breaking_ties; //lenght of matching edges (sum)
	g_angles.assign(num_rot, -100);
	g_matchings.assign(num_rot, -1);
	breaking_ties.assign(num_rot, -1);
	int tot_ang = 0;
	vector<PIEZA*>::iterator it_p;

	vector<EDGES> *edges_p;
	vector<EDGES>::iterator it_e3, it_e4;
	edges_p = p.getEdges();
	vector<EDGES>::iterator it_ep, it_es;
//	for(int m = 0; m <= 0; m++)//For the rectangle instances, no need to mirror pieces

	for (int m = 0; m <= 1; m++) {
		if (m == 1 && p.get_mirror() == false)
			p.set_mirror(true); //Trabajamos con la pieza reflejada.
		for (it_ep = edges_p->begin(); it_ep != edges_p->end(); it_ep++) {
			for (it_es = ed_irrbin.begin(); it_es != ed_irrbin.end(); it_es++) {

				// CALCULATE ROTATION TO MATCH EDGES OF BIN
				double alpha = calculate_rotation_match((*it_ep), (*it_es));
				double alpha_mir = alpha; //This angle indicates if the alpha calculated is the for the mirrrored piece
				if (m == 1) {
					if (equal_double(alpha, 0, epsilon))
						alpha_mir = -1000;
					else
						alpha_mir = -1 * alpha;
				}

				bool ya_probado = false;
				for (int compr_disp = 0; compr_disp < num_rot; compr_disp++) {
					if (alpha_mir >= g_angles[compr_disp] - epsilon
							&& alpha_mir <= g_angles[compr_disp] + epsilon) {
						ya_probado = true;
						break;
					}
				}
				if (ya_probado)
					continue;

				// MATCHING:
				int cont_match = 0;
				double bt2 = 0;
				// CALCULATING ROTATED EDGES OF p:
				vector<EDGES> rp;
				rp = rotation_piece(p, alpha);
				for (it_e3 = rp.begin(); it_e3 != rp.end(); it_e3++) {
					for (it_e4 = ed_irrbin.begin(); it_e4 != ed_irrbin.end();
							it_e4++) {
						(*it_e3).set_mod();
						if (calculate_rotation_match((*it_e3), (*it_e4))
								<= epsilon) {
							cont_match++;
							(*it_e3).set_mod();
							(*it_e4).set_mod();
							double min_length = (*it_e3).mod;
							double aux_d = (*it_e4).mod;
							if (aux_d < min_length)
								min_length = aux_d;
							bt2 += min_length; //break ties by matching edges. (maybe change this later).
						}
					}
				}
				//VEMOS SI ES MEJOR QUE LOS num_rot primeros:
				int pos = 0;
				while (pos < g_matchings.size()
						&& (cont_match < g_matchings[pos]
								|| (cont_match == g_matchings[pos]
										&& bt2 < breaking_ties[pos] - TOL))) {
					pos++;
				}

				if (pos < g_matchings.size()) {
					// GUARDAMOS ANGULO
					for (int j = g_matchings.size() - 1; j > pos; j--) {
						g_matchings[j] = g_matchings[j - 1];
						g_angles[j] = g_angles[j - 1];
						breaking_ties[j] = breaking_ties[j - 1];
					}
					g_matchings[pos] = cont_match;
					if (m == 0)
						g_angles[pos] = alpha; //Piece is not mirrored
					else if (alpha == 0)
						g_angles[pos] = -1000; //Piece is mirrored and rotation angle is 0.
					else
						g_angles[pos] = -1 * alpha; //piece is mirrored.
					breaking_ties[pos] = bt2;
				}
			}
		}
	}
	p.set_mirror(false);
	int i = 0;
	tot_ang = 0;
	while (i < g_angles.size()) {
		if (g_angles[i] != -100)
			tot_ang++;
		i++;
	}
	g_angles.resize(tot_ang);
	return g_angles;
}

vector<double> calculate_rotations_FitBin_Mirror(NODE &b, PIEZA &p, int num_rot,
		double epsilon) {
	if (p.get_angulo_usado() != 0) {
		p.obtener_rotacion(0);
	}
	vector<double> g_angles; //Vector of possible rotations
	vector<int> g_matchings; //number of edges that match
	vector<double> breaking_ties; //lenght of matching edges (sum)
	g_angles.assign(num_rot, -100);
	g_matchings.assign(num_rot, -1);
	breaking_ties.assign(num_rot, -1);

	vector<PIEZA*>::iterator it_p;

	vector<EDGES> *edges_p;
	vector<EDGES>::iterator it_e, it_e2, it_e3, it_e4;
	edges_p = p.getEdges();

	// PRIMERO AÑADIMOS LOS LADOS DEL BIN:
	// CONSTRUIMOS EDGES DEL BIN EN SENTIDO HORARIO (para que la pieza quede dentro):
	vector<EDGES> edges_bin;
	PUNTO pp1, pp2, pp4;
	pp1.coordx = 0;
	pp1.coordy = 0;
	pp2.coordx = 0;
	pp2.coordy = b.getW();
	pp4.coordx = b.getL();
	pp4.coordy = 0;
	EDGES eb1, eb4;    //Considerar solo dos aristas del bin.
	eb1.ini = pp1;
	eb1.fin = pp2;
	eb4.ini = pp4;
	eb4.fin = pp1;
	eb1.set_mod();
	eb4.set_mod();
	edges_bin.push_back(eb1);
	edges_bin.push_back(eb4);
	//=======================
	//	for(int m = 0; m<= 0; m++)//For the rectangle instances, no need of mirroring.
	for (int m = 0; m <= 1; m++) {
		if (m == 1 && p.get_mirror() == false)
			p.set_mirror(true);    //Trabajamos con la pieza reflejada.
		for (it_e = edges_p->begin(); it_e != edges_p->end(); it_e++) {
			for (it_e2 = edges_bin.begin(); it_e2 != edges_bin.end(); it_e2++) {
				// CALCULATE ROTATION TO MATCH EDGES OF BIN
				double alpha = calculate_rotation_match((*it_e), (*it_e2));
				double alpha_mir = alpha; //This angle indicates if the alpha calculated is the for the mirrrored piece
				if (m == 1) {
					if (equal_double(alpha, 0, epsilon))
						alpha_mir = -1000;
					else
						alpha_mir = -1 * alpha;
				}

				bool ya_probado = false;
				for (int compr_disp = 0; compr_disp < num_rot; compr_disp++) {
					if (alpha_mir >= g_angles[compr_disp] - epsilon
							&& alpha_mir <= g_angles[compr_disp] + epsilon) {
						ya_probado = true;
						break;
					}
				}
				if (ya_probado)
					continue;

				// MATCHING:
				int cont_match = 0;
				double bt2 = 0;
				// CALCULATING ROTATED EDGES OF p:
				vector<EDGES> rp;
				rp = rotation_piece(p, alpha);

				for (it_e3 = rp.begin(); it_e3 != rp.end(); it_e3++) {
					for (it_e4 = edges_bin.begin(); it_e4 != edges_bin.end();
							it_e4++) {
						if (calculate_rotation_match((*it_e3), (*it_e4))
								<= epsilon) {
							cont_match++;
							(*it_e3).set_mod();
							(*it_e4).set_mod();
							double min_length = (*it_e3).mod;
							double aux_d = (*it_e4).mod;
							if (aux_d < min_length)
								min_length = aux_d;
							bt2 += min_length;
						}
					}
				}

				//VEMOS SI ES MEJOR QUE LOS num_rot primeros:
				int pos = 0;
				while (pos < g_matchings.size()
						&& (cont_match < g_matchings[pos]
								|| (cont_match == g_matchings[pos]
										&& bt2 < breaking_ties[pos] - TOL))) {
					pos++;
				}

				if (pos < g_matchings.size()) {
					// GUARDAMOS ANGULO
					for (int j = g_matchings.size() - 1; j > pos; j--) {
						g_matchings[j] = g_matchings[j - 1];
						g_angles[j] = g_angles[j - 1];
						breaking_ties[j] = breaking_ties[j - 1];
					}
					g_matchings[pos] = cont_match;
					if (m == 0)
						g_angles[pos] = alpha; //Piece is not mirrored
					else if (alpha == 0)
						g_angles[pos] = -1000; //Piece is mirrored and rotation angle is 0.
					else
						g_angles[pos] = -1 * alpha; //piece is mirrored. 
					breaking_ties[pos] = bt2;
				}
			}
		}
	}
	p.set_mirror(false);
	return g_angles;
}

//Calcula un vector de posibles rotaciones

vector<double> calculate_rotations_FitNode(NODE &n, PIEZA &p, int num_rot,
		double epsilon) {
	if (p.get_angulo_usado() != 0) //Coloca la pieza en su posición original.
			{
		p.obtener_rotacion(0);
	}
	vector<double> g_angles; //Vector of possible rotations
	vector<int> g_matchings; //non increasing order
	vector<double> breaking_ties;
	g_angles.assign(num_rot, -100);
	g_matchings.assign(num_rot, -1);
	breaking_ties.assign(num_rot, -1);

	vector<EDGES> *edges_p;
	vector<EDGES>::iterator it_e, it_e2, it_e3, it_e4;
	edges_p = p.getEdges();

	// PRIMERO AÑADIMOS LAS DEL BIN:
	// CONSTRUIMOS EDGES DEL BIN EN SENTIDO HORARIO (para que la pieza quede dentro):
	vector<EDGES> edges_bin;
	PUNTO pp1, pp2, pp4;
	pp1.coordx = 0;
	pp1.coordy = 0;
	pp2.coordx = 0;
	pp2.coordy = n.getW();
	pp4.coordx = n.getL();
	pp4.coordy = 0;
	EDGES eb1, eb4;	//Considerar solo dos aristas del bin.
	eb1.ini = pp1;
	eb1.fin = pp2;
	eb4.ini = pp4;
	eb4.fin = pp1;
	eb1.set_mod();
	eb4.set_mod();
	edges_bin.push_back(eb1);
	edges_bin.push_back(eb4);
	//=======================
	for (it_e = edges_p->begin(); it_e != edges_p->end(); it_e++) {
		for (it_e2 = edges_bin.begin(); it_e2 != edges_bin.end(); it_e2++) {
			// CALCULATE ROTATION TO MATCH EDGES OF BIN
			double alpha = calculate_rotation_match((*it_e), (*it_e2));
			bool ya_probado = false;
			for (int compr_disp = 0; compr_disp < num_rot; compr_disp++) {
				if (alpha >= g_angles[compr_disp] - epsilon
						&& alpha <= g_angles[compr_disp] + epsilon) {
					ya_probado = true;
					break;
				}
			}
			if (ya_probado)
				continue;

			// MATCHING:
			int cont_match = 0;
			double bt2 = 0;
			// CALCULATING ROTATED EDGES OF p:
			vector<EDGES> rp;
			rp = rotation_piece(p, alpha);

			for (it_e3 = rp.begin(); it_e3 != rp.end(); it_e3++) {
				for (it_e4 = edges_bin.begin(); it_e4 != edges_bin.end();
						it_e4++) {
					(*it_e3).set_mod();
					if (calculate_rotation_match((*it_e3), (*it_e4))
							<= epsilon) {
						cont_match++;
						(*it_e3).set_mod();
						(*it_e4).set_mod();
						double min_length = (*it_e3).mod;
						double aux_d = (*it_e4).mod;
						if (aux_d < min_length)
							min_length = aux_d;
						bt2 += min_length;
					}
				}
			}

			//VEMOS SI ES MEJOR QUE LOS num_rot primeros:
			int pos = 0;
			while (pos < g_matchings.size()
					&& (cont_match < g_matchings[pos]
							|| (cont_match == g_matchings[pos]
									&& bt2 < breaking_ties[pos] - TOL))) {
				pos++;
			}

			if (pos < g_matchings.size()) {
				// GUARDAMOS ANGULO
				for (int j = g_matchings.size() - 1; j > pos; j--) {
					g_matchings[j] = g_matchings[j - 1];
					g_angles[j] = g_angles[j - 1];
					breaking_ties[j] = breaking_ties[j - 1];
				}
				g_matchings[pos] = cont_match;
				g_angles[pos] = alpha;
				breaking_ties[pos] = bt2;
			}
		}
	}

	return g_angles;
}

