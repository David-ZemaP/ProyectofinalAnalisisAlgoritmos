#include "common.h"

vector<string> dividirCSV(const string& linea) {
    vector<string> valores;
    string valor;
    bool dentroDeComillas = false;
    for (int i = 0; i < (int)linea.size(); i++) {
        char c = linea[i];
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
    for (int i = 0; i < (int)texto.size(); i++) {
        texto[i] = static_cast<char>(tolower(static_cast<unsigned char>(texto[i])));
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

unordered_map<int, Aeropuerto> cargarAeropuertos(const string& rutaArchivo) {
    unordered_map<int, Aeropuerto> aeropuertos;
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open())
        throw runtime_error("No se pudo abrir el archivo de aeropuertos: " + rutaArchivo);

    string linea;
    getline(archivo, linea);
    int omitidos = 0;
    while (getline(archivo, linea)) {
        vector<string> fila = dividirCSV(linea);
        if ((int)fila.size() < 8) { omitidos++; continue; }
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
    if (omitidos > 0) cout << "  Aeropuertos omitidos: " << omitidos << "\n";
    return aeropuertos;
}

unordered_map<int, vector<Arista>> cargarRutas(
    const string& rutaArchivo,
    const unordered_map<int, Aeropuerto>& aeropuertos
) {
    unordered_map<int, vector<Arista>> grafo;
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open())
        throw runtime_error("No se pudo abrir el archivo de rutas: " + rutaArchivo);

    string linea;
    getline(archivo, linea);
    int omitidas = 0, invalidas = 0;
    while (getline(archivo, linea)) {
        vector<string> fila = dividirCSV(linea);
        if ((int)fila.size() < 2) { omitidas++; continue; }
        try {
            int idOrigen = stoi(fila[0]);
            int idDestino = stoi(fila[1]);
            if (aeropuertos.count(idOrigen) > 0 && aeropuertos.count(idDestino) > 0) {
                const Aeropuerto& apO = aeropuertos.at(idOrigen);
                const Aeropuerto& apD = aeropuertos.at(idDestino);
                double dist = calcularHaversine(apO.latitud, apO.longitud,
                                                apD.latitud, apD.longitud);
                grafo[idOrigen].push_back({idDestino, dist});
            } else { invalidas++; }
        } catch (...) { omitidas++; }
    }
    if (omitidas > 0 || invalidas > 0)
        cout << "  Rutas omitidas: " << omitidas << " | Invalidas: " << invalidas << "\n";
    return grafo;
}

int buscarAeropuerto(const unordered_map<int, Aeropuerto>& aeropuertos, const string& consulta) {
    string consMin = aMinusculas(consulta);

    int id = -1;
    try { id = stoi(consulta); } catch (...) {}
    if (id != -1 && aeropuertos.count(id) > 0) return id;

    vector<int> ids;
    for (pair<const int, Aeropuerto> p : aeropuertos) ids.push_back(p.first);

    for (int i = 0; i < (int)ids.size(); i++) {
        const Aeropuerto& ap = aeropuertos.at(ids[i]);
        if (aMinusculas(ap.iata) == consMin) return ap.id;
    }

    int mejorId = -1;
    size_t menorPos = string::npos;
    for (int i = 0; i < (int)ids.size(); i++) {
        const Aeropuerto& ap = aeropuertos.at(ids[i]);
        size_t pos = aMinusculas(ap.nombre).find(consMin);
        if (pos != string::npos && (menorPos == string::npos || pos < menorPos)) {
            menorPos = pos;
            mejorId = ap.id;
        }
    }
    if (mejorId != -1) return mejorId;

    for (int i = 0; i < (int)ids.size(); i++) {
        const Aeropuerto& ap = aeropuertos.at(ids[i]);
        if (aMinusculas(ap.ciudad).find(consMin) != string::npos)
            return ap.id;
    }

    return -1;
}

static bool compararSCC(const vector<int>& a, const vector<int>& b) {
    return a.size() > b.size();
}

static void dfsTarjan(int u,
    const unordered_map<int, vector<int>>& g,
    unordered_map<int, int>& idx,
    unordered_map<int, int>& low,
    unordered_map<int, bool>& enPila,
    stack<int>& pila,
    int& contador,
    vector<vector<int>>& componentes)
{
    contador++;
    idx[u] = contador;
    low[u] = contador;
    pila.push(u);
    enPila[u] = true;

    if (g.count(u) > 0) {
        const vector<int>& vecinos = g.at(u);
        for (int i = 0; i < (int)vecinos.size(); i++) {
            int v = vecinos[i];
            if (idx.count(v) == 0) {
                dfsTarjan(v, g, idx, low, enPila, pila, contador, componentes);
                low[u] = min(low[u], low[v]);
            } else if (enPila[v]) {
                low[u] = min(low[u], idx[v]);
            }
        }
    }

    if (low[u] == idx[u]) {
        vector<int> comp;
        int v;
        do {
            v = pila.top(); pila.pop();
            enPila[v] = false;
            comp.push_back(v);
        } while (v != u);
        componentes.push_back(comp);
    }
}

vector<vector<int>> encontrarSCCs(const unordered_map<int, vector<Arista>>& grafo) {
    unordered_map<int, vector<int>> g;

    vector<int> origenes;
    for (pair<const int, vector<Arista>> p : grafo) origenes.push_back(p.first);

    for (int i = 0; i < (int)origenes.size(); i++) {
        int idOrigen = origenes[i];
        const vector<Arista>& aristas = grafo.at(idOrigen);
        for (int j = 0; j < (int)aristas.size(); j++) {
            g[idOrigen].push_back(aristas[j].idDestino);
        }
    }

    unordered_map<int, int> idx;
    unordered_map<int, int> low;
    unordered_map<int, bool> enPila;
    stack<int> pila;
    int contador = 0;
    vector<vector<int>> componentes;

    vector<int> nodos;
    for (pair<const int, vector<int>> p : g) nodos.push_back(p.first);

    for (int i = 0; i < (int)nodos.size(); i++) {
        int u = nodos[i];
        if (idx.count(u) == 0) {
            dfsTarjan(u, g, idx, low, enPila, pila, contador, componentes);
        }
    }

    sort(componentes.begin(), componentes.end(), compararSCC);
    return componentes;
}

unordered_map<int, Aerolinea> cargarAerolineas(const string& rutaArchivo) {
    unordered_map<int, Aerolinea> aerolineas;
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open())
        throw runtime_error("No se pudo abrir el archivo de aerolineas: " + rutaArchivo);

    string linea;
    getline(archivo, linea);
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string id_str, code;
        if (getline(ss, id_str, ',') && getline(ss, code, ',')) {
            try {
                int id = stoi(id_str);
                aerolineas[id] = {id, code};
            } catch (...) {}
        }
    }
    return aerolineas;
}

Multigrafo cargarMultigrafoAirline(const string& rutaArchivo) {
    Multigrafo grafo;
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open())
        throw runtime_error("No se pudo abrir el archivo multigrafo: " + rutaArchivo);

    string linea;
    getline(archivo, linea);
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string src_str, dest_str, airline_str;
        if (getline(ss, src_str, ',') && getline(ss, dest_str, ',') && getline(ss, airline_str, ',')) {
            try {
                int src = stoi(src_str);
                int dest = stoi(dest_str);
                int aid = stoi(airline_str);
                grafo[src].push_back({dest, aid});
            } catch (...) {}
        }
    }
    return grafo;
}
