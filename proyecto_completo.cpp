/*
 * PROYECTO FINAL - ANALISIS DE ALGORITMOS
 * Analisis de la Red Mundial de Vuelos
 *
 * RETO 1: Alcance Personalizado (BFS con maximo 3 escalas)
 * RETO 2: Grupos y Aislamiento Aereo (Componentes Conexos + SCC)
 * RETO 3: La Maxima Eficiencia - Diametro del grafo (Dijkstra)
 * RETO 4: Red Minima de Sudamerica (MST - Prim)
 *
 * Compilacion: g++ -std=c++17 -O2 proyecto_completo.cpp -o proyecto_completo
 */

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

using namespace std;
using namespace chrono;

// ============================================================
// ESTRUCTURAS DE DATOS
// ============================================================

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
    double distancia; // km
};

struct ResultadoBFS {
    unordered_set<int> aeropuertosAlcanzables;
    unordered_map<int, int> vuelosMinimos;
    unordered_map<int, int> mapaPadres;
};

// Para Union-Find (Reto 4)
struct UnionFind {
    vector<int> padre;
    vector<int> rango;

    UnionFind(int n) {
        padre.resize(n);
        rango.resize(n, 0);
        for (int i = 0; i < n; i++) padre[i] = i;
    }

    int encontrar(int x) {
        if (padre[x] != x)
            padre[x] = encontrar(padre[x]);
        return padre[x];
    }

    void unir(int x, int y) {
        int rx = encontrar(x);
        int ry = encontrar(y);
        if (rx == ry) return;
        if (rango[rx] < rango[ry]) {
            padre[rx] = ry;
        } else if (rango[rx] > rango[ry]) {
            padre[ry] = rx;
        } else {
            padre[ry] = rx;
            rango[rx]++;
        }
    }
};

struct AristaMST {
    int u;
    int v;
    double peso;
    string nombreU;
    string nombreV;
    string iataU;
    string iataV;
};

// ============================================================
// FUNCIONES AUXILIARES
// ============================================================

vector<string> dividirCSV(const string& linea) {
    vector<string> valores;
    string valor;
    bool dentroDeComillas = false;
    for (char c : linea) {
        if (c == '"') {
            dentroDeComillas = !dentroDeComillas;
        } else if (c == ',' && !dentroDeComillas) {
            valores.push_back(valor);
            valor.clear();
        } else {
            valor += c;
        }
    }
    valores.push_back(valor);
    return valores;
}

string aMinusculas(string texto) {
    for (char& c : texto) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return texto;
}

double calcularHaversine(double lat1, double lon1, double lat2, double lon2) {
    const double radioTierra = 6371.0;
    const double pi = 3.141592653589793;
    double phi1 = lat1 * pi / 180.0;
    double phi2 = lat2 * pi / 180.0;
    double deltaPhi = (lat2 - lat1) * pi / 180.0;
    double deltaLam = (lon2 - lon1) * pi / 180.0;
    double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
               cos(phi1) * cos(phi2) *
               sin(deltaLam / 2) * sin(deltaLam / 2);
    return radioTierra * 2 * atan2(sqrt(a), sqrt(1 - a));
}

// ============================================================
// CARGA DE DATOS
// ============================================================

unordered_map<int, Aeropuerto> cargarAeropuertos(const string& rutaArchivo) {
    unordered_map<int, Aeropuerto> aeropuertos;
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        throw runtime_error("No se pudo abrir el archivo de aeropuertos: " + rutaArchivo);
    }
    string linea;
    getline(archivo, linea);
    int omitidos = 0;
    while (getline(archivo, linea)) {
        vector<string> fila = dividirCSV(linea);
        if (fila.size() < 8) { omitidos++; continue; }
        try {
            Aeropuerto a;
            a.id = stoi(fila[0]);
            a.nombre = fila[1];
            a.ciudad = fila[2];
            a.pais = fila[3];
            a.iata = fila[4];
            a.icao = fila[5];
            a.latitud = stod(fila[6]);
            a.longitud = stod(fila[7]);
            aeropuertos[a.id] = a;
        } catch (...) { omitidos++; }
    }
    if (omitidos > 0)
        cout << "  Aeropuertos omitidos: " << omitidos << "\n";
    return aeropuertos;
}

unordered_map<int, vector<Arista>> cargarRutas(
    const string& rutaArchivo,
    const unordered_map<int, Aeropuerto>& aeropuertos
) {
    unordered_map<int, vector<Arista>> grafo;
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        throw runtime_error("No se pudo abrir el archivo de rutas: " + rutaArchivo);
    }
    string linea;
    getline(archivo, linea);
    int omitidas = 0;
    int invalidas = 0;
    while (getline(archivo, linea)) {
        vector<string> fila = dividirCSV(linea);
        if (fila.size() < 2) { omitidas++; continue; }
        try {
            int idOrigen = stoi(fila[0]);
            int idDestino = stoi(fila[1]);

            auto itOrigen = aeropuertos.find(idOrigen);
            auto itDestino = aeropuertos.find(idDestino);

            if (itOrigen != aeropuertos.end() && itDestino != aeropuertos.end()) {
                double dist = calcularHaversine(
                    itOrigen->second.latitud, itOrigen->second.longitud,
                    itDestino->second.latitud, itDestino->second.longitud
                );
                grafo[idOrigen].push_back({idDestino, dist});
            } else {
                invalidas++;
            }
        } catch (...) { omitidas++; }
    }
    if (omitidas > 0 || invalidas > 0)
        cout << "  Rutas omitidas: " << omitidas << " | Invalidas: " << invalidas << "\n";
    return grafo;
}

int buscarAeropuerto(
    const unordered_map<int, Aeropuerto>& aeropuertos,
    const string& consulta
) {
    string consMin = aMinusculas(consulta);
    int id = stoi(consulta);

    if (aeropuertos.find(id) != aeropuertos.end()) return id;
    for (const auto& par : aeropuertos) {
        if (aMinusculas(par.second.iata) == consMin) return par.second.id;
    }

    int mejorCoincidencia = -1;
    size_t menorPos = string::npos;
    for (const auto& par : aeropuertos) {
        size_t pos = aMinusculas(par.second.nombre).find(consMin);
        if (pos != string::npos) {
            if (menorPos == string::npos || pos < menorPos) {
                menorPos = pos;
                mejorCoincidencia = par.second.id;
            }
        }
    }
    if (mejorCoincidencia != -1) return mejorCoincidencia;

    for (const auto& par : aeropuertos) {
        if (aMinusculas(par.second.ciudad).find(consMin) != string::npos)
            return par.second.id;
    }
    return -1;
}

// ============================================================
// RETO 1: ALCANCE PERSONALIZADO (BFS con maximo 3 escalas)
// ============================================================

ResultadoBFS ejecutarBFS(
    const unordered_map<int, vector<Arista>>& grafo,
    int idOrigen,
    int maximoVuelos
) {
    ResultadoBFS resultado;
    queue<pair<int, int>> pendientes;
    pendientes.push({idOrigen, 0});
    resultado.vuelosMinimos[idOrigen] = 0;
    resultado.mapaPadres[idOrigen] = -1;

    while (!pendientes.empty()) {
        int idActual = pendientes.front().first;
        int vuelosActuales = pendientes.front().second;
        pendientes.pop();

        if (vuelosActuales == maximoVuelos) continue;
        if (grafo.find(idActual) == grafo.end()) continue;

        for (const Arista& arista : grafo.at(idActual)) {
            int idVecino = arista.idDestino;
            int sigVuelos = vuelosActuales + 1;

            if (resultado.vuelosMinimos.find(idVecino) == resultado.vuelosMinimos.end()) {
                resultado.vuelosMinimos[idVecino] = sigVuelos;
                resultado.mapaPadres[idVecino] = idActual;
                resultado.aeropuertosAlcanzables.insert(idVecino);
                pendientes.push({idVecino, sigVuelos});
            }
        }
    }
    return resultado;
}

void mostrarRutasExactas(
    const vector<pair<int, int>>& apsOrdenados,
    const unordered_map<int, Aeropuerto>& aeropuertos,
    const ResultadoBFS& bfsResult
) {
    cout << "\nDesea ver las rutas exactas de los primeros 5 aeropuertos? (s/n): ";
    char resp;
    cin >> resp;
    if (resp == 's' || resp == 'S') {
        cout << "\n=== RUTAS DE VUELO EXACTAS ===\n";
        int mostrados = 0;
        for (const auto& par : apsOrdenados) {
            if (mostrados == 5) break;
            int idDest = par.first;
            if (aeropuertos.find(idDest) == aeropuertos.end()) continue;

            vector<int> camino;
            int actual = idDest;
            while (actual != -1) {
                camino.push_back(actual);
                actual = bfsResult.mapaPadres.at(actual);
            }
            reverse(camino.begin(), camino.end());

            cout << (mostrados + 1) << ". Hacia " << aeropuertos.at(idDest).ciudad
                 << " (" << aeropuertos.at(idDest).iata << "):\n   Ruta: ";
            for (size_t i = 0; i < camino.size(); ++i) {
                cout << aeropuertos.at(camino[i]).iata;
                if (i < camino.size() - 1) cout << " -> ";
            }
            cout << "\n\n";
            mostrados++;
        }
    }
}

void Ejercicio1(
    const unordered_map<int, Aeropuerto>& aeropuertos,
    const unordered_map<int, vector<Arista>>& grafo
) {
    cin.ignore(10000, '\n');
    string consulta;
    cout << "\n======================================\n";
    cout << "   RETO 1: ALCANCE PERSONALIZADO\n";
    cout << "======================================\n";
    cout << "Ingrese ID, nombre o codigo IATA del aeropuerto: ";
    cout.flush();
    if (!getline(cin, consulta)) {
        cout << "Error de lectura.\n";
        cin.clear();
        return;
    }

    int idOrigen = buscarAeropuerto(aeropuertos, consulta);
    if (idOrigen == -1) {
        cout << "No se encontro el aeropuerto.\n";
        return;
    }

    const Aeropuerto& origen = aeropuertos.at(idOrigen);
    cout << "\nOrigen: " << origen.nombre << " (" << origen.iata << ") - "
         << origen.ciudad << ", " << origen.pais << "\n";

    int maxEscalas = 3;
    int maxVuelos = maxEscalas + 1;

    ResultadoBFS res = ejecutarBFS(grafo, idOrigen, maxVuelos);

    cout << "\nRESULTADO:\n";
    cout << "Aeropuertos distintos alcanzables con maximo " << maxEscalas
         << " escalas: " << res.aeropuertosAlcanzables.size() << "\n";

    vector<pair<int, int>> apsOrdenados;
    for (int id : res.aeropuertosAlcanzables) {
        apsOrdenados.push_back({id, res.vuelosMinimos.at(id)});
    }
    sort(apsOrdenados.begin(), apsOrdenados.end(),
         [](const pair<int, int>& a, const pair<int, int>& b) {
             return a.second < b.second;
         });

    cout << "\nPrimeros aeropuertos alcanzables (ordenados por cercania):\n";
    int mostrados = 0;
    for (const auto& par : apsOrdenados) {
        if (mostrados == 20) break;
        int id = par.first;
        if (aeropuertos.find(id) == aeropuertos.end()) continue;
        const Aeropuerto& ap = aeropuertos.at(id);
        cout << "- " << ap.nombre << " (" << ap.iata << ") | "
             << ap.ciudad << ", " << ap.pais << " | Vuelos: " << par.second << "\n";
        mostrados++;
    }
    if (res.aeropuertosAlcanzables.size() > 20)
        cout << "... y " << (res.aeropuertosAlcanzables.size() - 20) << " mas.\n";

    int con1 = 0, con2 = 0, con3 = 0;
    for (const auto& par : apsOrdenados) {
        if (par.second == 1) con1++;
        else if (par.second == 2) con2++;
        else if (par.second > 2) con3++;
    }
    cout << "\nDesglose por escalas:\n";
    cout << "  1 vuelo (0 escalas): " << con1 << "\n";
    cout << "  2 vuelos (1 escala): " << con2 << "\n";
    cout << "  3-4 vuelos (2-3 escalas): " << con3 << "\n";

    mostrarRutasExactas(apsOrdenados, aeropuertos, res);
}

// ============================================================
// Ejercicio 2: GRUPOS Y AISLAMIENTO AEREO
// ============================================================

void dfsComponente(
    int nodo,
    const unordered_map<int, vector<int>>& grafoNoDirigido,
    unordered_set<int>& visitados,
    vector<int>& componente
) {
    stack<int> pila;
    pila.push(nodo);
    visitados.insert(nodo);

    while (!pila.empty()) {
        int actual = pila.top();
        pila.pop();
        componente.push_back(actual);

        auto it = grafoNoDirigido.find(actual);
        if (it == grafoNoDirigido.end()) continue;

        for (int vecino : it->second) {
            if (visitados.find(vecino) == visitados.end()) {
                visitados.insert(vecino);
                pila.push(vecino);
            }
        }
    }
}

void Ejercicio2(
    const unordered_map<int, Aeropuerto>& aeropuertos,
    const unordered_map<int, vector<Arista>>& grafo
) {
    unordered_map<int, vector<int>> grafoND;
    for (const auto& par : grafo) {
        int origen = par.first;
        for (const Arista& arista : par.second) {
            int destino = arista.idDestino;
            grafoND[origen].push_back(destino);
            grafoND[destino].push_back(origen);
        }
    }

    unordered_set<int> visitados;
    vector<vector<int>> componentes;
    int nodosEnGrafo = 0;

    for (const auto& par : grafoND) {
        nodosEnGrafo++;
        if (visitados.find(par.first) == visitados.end()) {
            vector<int> comp;
            dfsComponente(par.first, grafoND, visitados, comp);
            componentes.push_back(comp);
        }
    }

    sort(componentes.begin(), componentes.end(),
         [](const vector<int>& a, const vector<int>& b) {
             return a.size() > b.size();
         });

    cout << "Nodos en el grafo: " << nodosEnGrafo << "\n";
    cout << "Componentes conexos: " << componentes.size() << "\n";
    cout << "Componente mas grande: " << componentes[0].size() << " aeropuertos\n";

    int gruposAislados = 0;
    for (const auto& comp : componentes) {
        if (comp.size() >= 2) gruposAislados++;
    }
    cout << "Grupos cerrados (2+ aeropuertos): " << gruposAislados << "\n";

    cout << "\nComponentes aislados (islas):\n";
    int mostradosIslas = 0;
    for (const auto& comp : componentes) {
        if (comp.size() < 2) continue;
        if (comp.size() == componentes[0].size()) continue;
        if (mostradosIslas >= 15) {
            cout << "... y " << (gruposAislados - 1 - mostradosIslas) << " grupos mas.\n";
            break;
        }
        cout << "  Grupo #" << (mostradosIslas + 1) << " (" << comp.size() << " aeropuertos):\n";
        for (size_t i = 0; i < min(comp.size(), size_t(5)); i++) {
            int id = comp[i];
            if (aeropuertos.find(id) != aeropuertos.end()) {
                const auto& ap = aeropuertos.at(id);
                cout << "    - " << ap.nombre << " (" << ap.iata << ") - " << ap.pais << "\n";
            }
        }
        if (comp.size() > 5) {
            cout << "    ... y " << (comp.size() - 5) << " mas.\n";
        }
        mostradosIslas++;
    }

    cout << "Aeropuertos en componente gigante: " << componentes[0].size() << "\n";
    int totalIslas = 0;
    for (size_t i = 1; i < componentes.size(); i++) {
        totalIslas += componentes[i].size();
    }
    cout << "Aeropuertos en grupos aislados: " << totalIslas << "\n";
    cout << "Aeropuertos sin conexion (solos): ";
    int solos = 0;
    for (const auto& comp : componentes) {
        if (comp.size() == 1) solos++;
    }
    cout << solos << "\n";
}

// ============================================================
// Ejercicio 3: LA MAXIMA EFICIENCIA (DIAMETRO DEL GRAFO)
// ============================================================

void reto3Diametro(
    const unordered_map<int, Aeropuerto>& aeropuertos,
    const unordered_map<int, vector<Arista>>& grafo
) {
    unordered_map<int, vector<int>> grafoND;
    for (const auto& par : grafo) {
        int orig = par.first;
        for (const Arista& arista : par.second) {
            int dest = arista.idDestino;
            grafoND[orig].push_back(dest);
            grafoND[dest].push_back(orig);
        }
    }

    unordered_set<int> visitados;
    vector<int> componenteGigante;

    for (const auto& par : grafoND) {
        if (visitados.find(par.first) == visitados.end()) {
            queue<int> q;
            q.push(par.first);
            visitados.insert(par.first);
            vector<int> comp;

            while (!q.empty()) {
                int actual = q.front();
                q.pop();
                comp.push_back(actual);

                auto it = grafoND.find(actual);
                if (it == grafoND.end()) continue;
                for (int vecino : it->second) {
                    if (visitados.find(vecino) == visitados.end()) {
                        visitados.insert(vecino);
                        q.push(vecino);
                    }
                }
            }

            if (comp.size() > componenteGigante.size()) {
                componenteGigante = comp;
            }
        }
    }

    cout << "\nComponente gigante: " << componenteGigante.size() << " aeropuertos\n";

    if (componenteGigante.size() < 2) {
        cout << "No hay suficientes aeropuertos conectados.\n";
        return;
    }

    auto inicio = high_resolution_clock::now();

    auto dijkstraAMax = [&](int origen, int& nodoLejano) -> double {
        unordered_map<int, double> distancias;
        using Par = pair<double, int>;
        priority_queue<Par, vector<Par>, greater<Par>> pq;

        distancias[origen] = 0;
        pq.push({0, origen});

        while (!pq.empty()) {
            auto [distAct, nodoAct] = pq.top();
            pq.pop();

            if (distAct > distancias[nodoAct]) continue;

            auto it = grafo.find(nodoAct);
            if (it == grafo.end()) continue;

            for (const Arista& arista : it->second) {
                int vecino = arista.idDestino;
                double nuevaDist = distAct + arista.distancia;

                auto itDist = distancias.find(vecino);
                if (itDist == distancias.end() || nuevaDist < itDist->second) {
                    distancias[vecino] = nuevaDist;
                    pq.push({nuevaDist, vecino});
                }
            }
        }
        double maxD = 0;
        nodoLejano = origen;
        for (const auto& p : distancias) {
            if (p.first != origen && p.second > maxD) {
                maxD = p.second;
                nodoLejano = p.first;
            }
        }
        return maxD;
    };

    vector<int> semillas;

    int idNorte = componenteGigante[0], idSur = componenteGigante[0];
    int idEste = componenteGigante[0], idOeste = componenteGigante[0];
    double latNorte = -90, latSur = 90;
    double lonEste = -180, lonOeste = 180;

    for (int id : componenteGigante) {
        auto it = aeropuertos.find(id);
        if (it == aeropuertos.end()) continue;
        double lat = it->second.latitud;
        double lon = it->second.longitud;

        if (lat > latNorte) { latNorte = lat; idNorte = id; }
        if (lat < latSur)   { latSur = lat;   idSur = id; }
        if (lon > lonEste)  { lonEste = lon;  idEste = id; }
        if (lon < lonOeste) { lonOeste = lon; idOeste = id; }
    }

    semillas.push_back(componenteGigante[0]);
    semillas.push_back(idNorte);
    semillas.push_back(idSur);
    semillas.push_back(idEste);
    semillas.push_back(idOeste);

    sort(semillas.begin(), semillas.end());
    semillas.erase(unique(semillas.begin(), semillas.end()), semillas.end());

    cout << "Usando " << semillas.size() << " semillas geograficamente diversas\n";

    double distMaxGlobal = 0;
    int mejorOrigen = -1, mejorDestino = -1;
    int totalDijkstra = 0;

    for (size_t i = 0; i < semillas.size(); i++) {
        int semilla = semillas[i];

        int nodoA = -1;
        double distA = dijkstraAMax(semilla, nodoA);
        totalDijkstra++;

        if (distA > distMaxGlobal) {
            distMaxGlobal = distA;
            mejorOrigen = semilla;
            mejorDestino = nodoA;
        }

        int nodoB = -1;
        double distB = dijkstraAMax(nodoA, nodoB);
        totalDijkstra++;

        if (distB > distMaxGlobal) {
            distMaxGlobal = distB;
            mejorOrigen = nodoA;
            mejorDestino = nodoB;
        }

        cout << "  Semilla " << (i + 1) << "/" << semillas.size()
             << ": " << aeropuertos.at(semilla).iata
             << " -> extremo " << aeropuertos.at(nodoA).iata
             << " (" << fixed << setprecision(0) << distA << " km)"
             << " -> " << aeropuertos.at(nodoB).iata
             << " (" << fixed << setprecision(0) << distB << " km)"
             << " | Mejor: " << fixed << setprecision(0) << distMaxGlobal << " km\n";
    }

    cout << "\nRefinando desde el mejor origen...\n";
    int nodoExtra = -1;
    double distExtra = dijkstraAMax(mejorOrigen, nodoExtra);
    totalDijkstra++;
    if (distExtra > distMaxGlobal) {
        distMaxGlobal = distExtra;
        mejorDestino = nodoExtra;
    }

    nodoExtra = -1;
    distExtra = dijkstraAMax(mejorDestino, nodoExtra);
    totalDijkstra++;
    if (distExtra > distMaxGlobal) {
        distMaxGlobal = distExtra;
        mejorOrigen = mejorDestino;
        mejorDestino = nodoExtra;
    }

    cout << "  Mejor origen  -> extremo: " << fixed << setprecision(0) << distMaxGlobal << " km\n";

    unordered_map<int, double> distDesdeMejor;
    {
        int temp;
        auto dijkstraCompleto = [&](int origen) -> unordered_map<int, double> {
            unordered_map<int, double> dists;
            using Par = pair<double, int>;
            priority_queue<Par, vector<Par>, greater<Par>> pq;
            dists[origen] = 0;
            pq.push({0, origen});
            while (!pq.empty()) {
                auto [d, n] = pq.top(); pq.pop();
                if (d > dists[n]) continue;
                auto it = grafo.find(n);
                if (it == grafo.end()) continue;
                for (const Arista& ar : it->second) {
                    double nd = d + ar.distancia;
                    auto itd = dists.find(ar.idDestino);
                    if (itd == dists.end() || nd < itd->second) {
                        dists[ar.idDestino] = nd;
                        pq.push({nd, ar.idDestino});
                    }
                }
            }
            return dists;
        };
        distDesdeMejor = dijkstraCompleto(mejorOrigen);
        totalDijkstra++;
    }

    vector<pair<int, double>> candidatos;
    for (const auto& p : distDesdeMejor) {
        if (p.first != mejorOrigen) candidatos.push_back(p);
    }
    sort(candidatos.begin(), candidatos.end(),
         [](const auto& a, const auto& b) { return a.second > b.second; });

    int maxCand = min((int)candidatos.size(), 30);
    cout << "Refinando con " << maxCand << " candidatos lejanos...\n";

    for (int i = 0; i < maxCand; i++) {
        int cand = candidatos[i].first;
        int nodoLej = -1;
        double dist = dijkstraAMax(cand, nodoLej);
        totalDijkstra++;

        if (dist > distMaxGlobal) {
            distMaxGlobal = dist;
            mejorOrigen = cand;
            mejorDestino = nodoLej;
            cout << "  ! Nuevo maximo encontrado: " << fixed << setprecision(0)
                 << distMaxGlobal << " km (" << aeropuertos.at(cand).iata
                 << " -> " << aeropuertos.at(nodoLej).iata << ")\n";
        }

        if ((i + 1) % 10 == 0) {
            cout << "  Progreso: " << (i + 1) << "/" << maxCand
                 << " (mejor: " << fixed << setprecision(0) << distMaxGlobal << " km)\n";
        }
    }

    auto fin = high_resolution_clock::now();
    auto duracion = duration_cast<seconds>(fin - inicio).count();

    // --- Resultados ---
    cout << "\n=== RESULTADO FINAL ===\n";
    cout << "Total de Dijkstra ejecutados: " << totalDijkstra << "\n";
    cout << "Tiempo de computo: " << duracion << " segundos\n\n";

    if (mejorOrigen != -1 && mejorDestino != -1 &&
        aeropuertos.find(mejorOrigen) != aeropuertos.end() &&
        aeropuertos.find(mejorDestino) != aeropuertos.end()) {

        const Aeropuerto& ap1 = aeropuertos.at(mejorOrigen);
        const Aeropuerto& ap2 = aeropuertos.at(mejorDestino);

        cout << "PAR DE AEROPUERTOS CON MAYOR DISTANCIA DE VIAJE:\n";
        cout << "  Origen:  " << ap1.nombre;
        if (!ap1.iata.empty() && ap1.iata != "\\N")
            cout << " (" << ap1.iata << ")";
        cout << "\n           " << ap1.ciudad << ", " << ap1.pais << "\n";

        cout << "  Destino: " << ap2.nombre;
        if (!ap2.iata.empty() && ap2.iata != "\\N")
            cout << " (" << ap2.iata << ")";
        cout << "\n           " << ap2.ciudad << ", " << ap2.pais << "\n";

        cout << "  Distancia total del viaje mas corto: "
             << fixed << setprecision(0) << distMaxGlobal << " km\n";

        // Distancia geografica directa para comparacion
        double distDirecta = calcularHaversine(
            ap1.latitud, ap1.longitud,
            ap2.latitud, ap2.longitud
        );
        cout << "  Distancia geografica directa: " << fixed << setprecision(0)
             << distDirecta << " km\n";
        if (distMaxGlobal > distDirecta) {
            cout << "  Diferencia (rutas vs directo): +"
                 << fixed << setprecision(0) << (distMaxGlobal - distDirecta) << " km\n";
        }
    }

    // Estadisticas de la red
    cout << "\n--- ESTADISTICAS DE LA RED ---\n";
    double distPromedio = 0;
    long long contPares = 0;
    for (const auto& par : distDesdeMejor) {
        if (par.second < numeric_limits<double>::max() && par.second > 0) {
            distPromedio += par.second;
            contPares++;
        }
    }
    if (contPares > 0) {
        distPromedio /= contPares;
        cout << "Distancia promedio de viaje: " << fixed << setprecision(0)
             << distPromedio << " km\n";
        cout << "Pares evaluados: " << contPares << "\n";
    }
}

// ============================================================
// RETO 4: RED MINIMA DE SUDAMERICA (MST con Prim)
// ============================================================

void reto4MSTSudamerica(
    const unordered_map<int, Aeropuerto>& aeropuertos,
    const unordered_map<int, vector<Arista>>& grafo
) {
    cout << "\n========================================\n";
    cout << "   RETO 4: RED MINIMA DE SUDAMERICA\n";
    cout << "   (Arbol de Expansion Minima)\n";
    cout << "========================================\n";

    // Lista de paises sudamericanos
    unordered_set<string> paisesSA = {
        "Argentina", "Bolivia", "Brazil", "Chile", "Colombia", "Ecuador",
        "Guyana", "Paraguay", "Peru", "Suriname", "Uruguay", "Venezuela",
        "French Guiana"
    };

    // --- Paso 1: Filtrar aeropuertos de Sudamerica con datos validos ---
    vector<int> aeropuertosSA;
    for (const auto& par : aeropuertos) {
        if (paisesSA.find(par.second.pais) != paisesSA.end()) {
            // Solo incluir si tiene IATA valido
            if (!par.second.iata.empty() && par.second.iata != "\\N") {
                aeropuertosSA.push_back(par.first);
            }
        }
    }

    cout << "Aeropuertos en Sudamerica: " << aeropuertosSA.size() << "\n";

    if (aeropuertosSA.size() < 2) {
        cout << "No hay suficientes aeropuertos en Sudamerica.\n";
        return;
    }

    // Mostrar cuantos por pais
    unordered_map<string, int> conteoPaises;
    for (int id : aeropuertosSA) {
        conteoPaises[aeropuertos.at(id).pais]++;
    }
    cout << "\nAeropuertos por pais:\n";
    for (const auto& par : conteoPaises) {
        cout << "  " << par.first << ": " << par.second << "\n";
    }

    // --- Paso 2: Construir grafo completo con distancias Haversine ---
    int n = aeropuertosSA.size();

    // Matriz de distancias (solo triangulo superior, para ahorrar memoria)
    // Usamos Prim O(n^2) directamente sobre la malla de distancias

    cout << "\nConstruyendo Arbol de Expansion Minima (Prim)...\n";
    auto inicio = high_resolution_clock::now();

    // Prim: desde el nodo 0, expandir con la arista mas cercana
    vector<bool> enMST(n, false);
    vector<double> minDist(n, numeric_limits<double>::max());
    vector<int> padre(n, -1);

    minDist[0] = 0;
    double pesoTotal = 0;

    for (int i = 0; i < n; i++) {
        // Encontrar nodo no incluido con distancia minima
        int u = -1;
        double menor = numeric_limits<double>::max();
        for (int j = 0; j < n; j++) {
            if (!enMST[j] && minDist[j] < menor) {
                menor = minDist[j];
                u = j;
            }
        }

        if (u == -1) {
            cout << "ERROR: Grafo no conectado (algo salio mal).\n";
            return;
        }

        enMST[u] = true;
        pesoTotal += minDist[u];

        // Actualizar distancias de vecinos (calcular Haversine sobre la marcha)
        const Aeropuerto& apU = aeropuertos.at(aeropuertosSA[u]);
        for (int v = 0; v < n; v++) {
            if (!enMST[v]) {
                const Aeropuerto& apV = aeropuertos.at(aeropuertosSA[v]);
                double dist = calcularHaversine(
                    apU.latitud, apU.longitud,
                    apV.latitud, apV.longitud
                );
                if (dist < minDist[v]) {
                    minDist[v] = dist;
                    padre[v] = u;
                }
            }
        }
    }

    auto fin = high_resolution_clock::now();
    auto duracion = duration_cast<milliseconds>(fin - inicio).count();

    // --- Paso 3: Mostrar resultados ---
    cout << "\n=== RESULTADOS DEL MST ===\n";
    cout << "Kilometraje total de la red minima: "
         << fixed << setprecision(0) << pesoTotal << " km\n";
    cout << "Tiempo de computo: " << duracion << " ms\n";
    cout << "Rutas en el arbol: " << (n - 1) << "\n";

    // Recolectar aristas del MST para mostrar
    vector<AristaMST> aristasMST;
    for (int v = 1; v < n; v++) {
        if (padre[v] != -1) {
            int u = padre[v];
            const Aeropuerto& apU = aeropuertos.at(aeropuertosSA[u]);
            const Aeropuerto& apV = aeropuertos.at(aeropuertosSA[v]);
            double dist = calcularHaversine(
                apU.latitud, apU.longitud,
                apV.latitud, apV.longitud
            );
            aristasMST.push_back({
                aeropuertosSA[u], aeropuertosSA[v], dist,
                apU.nombre, apV.nombre, apU.iata, apV.iata
            });
        }
    }

    // Ordenar por distancia descendente para mostrar
    sort(aristasMST.begin(), aristasMST.end(),
         [](const AristaMST& a, const AristaMST& b) { return a.peso > b.peso; });

    // Mostrar las rutas mas largas del MST
    cout << "\nRutas mas largas del arbol (top 15):\n";
    for (size_t i = 0; i < min(aristasMST.size(), size_t(15)); i++) {
        const auto& arista = aristasMST[i];
        cout << "  " << (i + 1) << ". " << arista.nombreU << " (" << arista.iataU << ")"
             << "  <-->  " << arista.nombreV << " (" << arista.iataV << ")"
             << "  |  " << fixed << setprecision(0) << arista.peso << " km\n";
    }

    if (aristasMST.size() > 15) {
        cout << "  ... y " << (aristasMST.size() - 15) << " rutas mas.\n";
    }

    // Mostrar rutas mas cortas del MST
    sort(aristasMST.begin(), aristasMST.end(),
         [](const AristaMST& a, const AristaMST& b) { return a.peso < b.peso; });

    cout << "\nRutas mas cortas del arbol (top 10):\n";
    for (size_t i = 0; i < min(aristasMST.size(), size_t(10)); i++) {
        const auto& arista = aristasMST[i];
        cout << "  " << (i + 1) << ". " << arista.nombreU << " (" << arista.iataU << ")"
             << "  <-->  " << arista.nombreV << " (" << arista.iataV << ")"
             << "  |  " << fixed << setprecision(0) << arista.peso << " km\n";
    }

    // Estadisticas
    double distProm = 0;
    for (const auto& arista : aristasMST) distProm += arista.peso;
    distProm /= aristasMST.size();

    double distMax = aristasMST.empty() ? 0 : aristasMST.front().peso;
    double distMin = aristasMST.empty() ? 0 : aristasMST.back().peso;

    cout << "\n--- ESTADISTICAS DEL MST ---\n";
    cout << "Ruta mas larga: " << fixed << setprecision(0) << distMax << " km\n";
    cout << "Ruta mas corta: " << fixed << setprecision(0) << distMin << " km\n";
    cout << "Distancia promedio por ruta: " << fixed << setprecision(0) << distProm << " km\n";
    cout << "Total de kilometros de red: " << fixed << setprecision(0) << pesoTotal << " km\n";
}

// ============================================================
// MAIN - MENU PRINCIPAL
// ============================================================

int main() {
    string archivoAeropuertos = "airports_clean_final.csv";
    string archivoRutas = "routes_graph_validated.csv";

    unordered_map<int, Aeropuerto> aeropuertos;
    unordered_map<int, vector<Arista>> grafoVuelos;

    cout << "============================================\n";
    cout << "  ANALISIS DE LA RED MUNDIAL DE VUELOS\n";
    cout << "  Proyecto Final - Analisis de Algoritmos\n";
    cout << "============================================\n\n";

    // --- CARGA DE DATOS ---
    cout << "[1/2] Cargando aeropuertos...\n";
    try {
        aeropuertos = cargarAeropuertos(archivoAeropuertos);
        cout << "  Aeropuertos cargados: " << aeropuertos.size() << "\n";
    } catch (const exception& e) {
        cout << "ERROR FATAL: " << e.what() << "\n";
        cout << "Asegurate de que " << archivoAeropuertos << " existe en el directorio.\n";
        return 1;
    }

    cout << "[2/2] Construyendo grafo de rutas...\n";
    try {
        grafoVuelos = cargarRutas(archivoRutas, aeropuertos);
        // Contar aristas
        int totalAristas = 0;
        for (const auto& par : grafoVuelos) {
            totalAristas += par.second.size();
        }
        cout << "  Nodos con rutas: " << grafoVuelos.size() << "\n";
        cout << "  Rutas cargadas: " << totalAristas << "\n";
    } catch (const exception& e) {
        cout << "ERROR FATAL: " << e.what() << "\n";
        cout << "Asegurate de que " << archivoRutas << " existe en el directorio.\n";
        return 1;
    }

    // --- MENU INTERACTIVO ---
    int opcion = 0;
    do {
        cout << "\n========================================\n";
        cout << "  MENU PRINCIPAL\n";
        cout << "========================================\n";
        cout << "  1. RETO 1: Alcance Personalizado (BFS)\n";
        cout << "  2. RETO 2: Grupos y Aislamiento Aereo\n";
        cout << "  3. RETO 3: La Maxima Eficiencia (Diametro)\n";
        cout << "  4. RETO 4: Red Minima de Sudamerica (MST)\n";
        cout << "  0. Salir\n";
        cout << "========================================\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> opcion)) {
            // Si la entrada falla (EOF, pipe cerrado, o caracter invalido)
            if (cin.eof()) break;
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Entrada invalida. Use un numero.\n";
            continue;
        }

        switch (opcion) {
            case 1:
                iniciarReto1(aeropuertos, grafoVuelos);
                break;
            case 2:
                reto2ComponentesConexos(aeropuertos, grafoVuelos);
                break;
            case 3:
                reto3Diametro(aeropuertos, grafoVuelos);
                break;
            case 4:
                reto4MSTSudamerica(aeropuertos, grafoVuelos);
                break;
            case 0:
                cout << "Saliendo...\n";
                break;
            default:
                cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);

    return 0;
}
