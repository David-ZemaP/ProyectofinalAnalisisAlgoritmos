#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cmath>
#include <stdexcept>
#include <queue>
#include <algorithm>
#include <cctype>
#include <limits>
#include <stack>
#include <iomanip>
#include <chrono>
#include <functional>

using namespace std;
using namespace chrono;

struct Aeropuerto {
    int id;
    string nombre;
    string ciudad;
    string pais;
    string iata;
    string icao;
    double latitud;
    double longitud;
};

struct Arista {
    int idDestino;
    double distancia;
};

struct ResultadoBFS {
    unordered_set<int> aeropuertosAlcanzables;
    unordered_map<int, int> vuelosMinimos;
    unordered_map<int, int> mapaPadres;
};

struct AristaMST {
    int u, v;
    double peso;
    string nombreU, nombreV, iataU, iataV;
};
vector<string> dividirCSV(const string& linea);
string aMinusculas(string texto);
double calcularHaversine(double lat1, double lon1, double lat2, double lon2);
int buscarAeropuerto(const unordered_map<int, Aeropuerto>& aeropuertos, const string& consulta);

unordered_map<int, Aeropuerto> cargarAeropuertos(const string& rutaArchivo);
unordered_map<int, vector<Arista>> cargarRutas(const string& rutaArchivo, const unordered_map<int, Aeropuerto>& aeropuertos);
vector<vector<int>> encontrarSCCs(const unordered_map<int, vector<Arista>>& grafo);

#endif
