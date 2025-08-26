#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <exception>
#include <map>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <windows.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include <sys/types.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <set>
#include <unordered_map>
#include <functional>
using namespace std;

vector<string> commands;





// =================== TCP SERVER ===================
class TCPServer {
private:
    int port;
    SOCKET serverSocket;
    SOCKET clientSocket;

public:
    TCPServer(int port) {
        this->port = port;
        this->serverSocket = INVALID_SOCKET;
        this->clientSocket = INVALID_SOCKET;
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed!\n";
        }
    }

    void start(vector<string>& voltagesources) {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed!\n";
            WSACleanup();
            return;
        }

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed!\n";
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed!\n";
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        std::cout << "Server listening on port " << port << "...\n";

        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed!\n";
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        std::cout << "Client connected!\n";
        handleClient(voltagesources);
    }

    void handleClient(vector<string> &voltagesources) {
        char buffer[1024] = {};
        int bytesReceived = 0;
        while(true) {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string msg(buffer);
                std::cout << "Received: " << msg << "\n";
                voltagesources.push_back(msg);

                if(msg.find("disconnect") != std::string::npos)
                    break;
                
            } else if (bytesReceived == 0) {
                std::cout << "Client disconnected.\n";
                break;
            } else {
                std::cerr << "recv failed with error: " << WSAGetLastError() << "\n";
                break;
            }
        }



        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
    }
};

// =================== TCP CLIENT ===================
class TCPClient {
private:
    std::string serverIP;
    int port;
    SOCKET sock;

public:
    TCPClient(const std::string& ip, int port) : serverIP(ip), port(port), sock(INVALID_SOCKET) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2,2), &wsaData);

        sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());

        if(connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
            std::cerr << "Connection failed\n";
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
    }

    void sendMessage(const std::string& message) {
        if(sock == INVALID_SOCKET) return;

        send(sock, message.c_str(), (int)message.size(), 0);
        char buffer[1024] = {};
        int bytesReceived = recv(sock, buffer, sizeof(buffer)-1, 0);
        if(bytesReceived>0){
            buffer[bytesReceived] = '\0';
            std::cout << "Server says: " << buffer << "\n";
        }
    }

    ~TCPClient(){
        if(sock != INVALID_SOCKET) closesocket(sock);
        WSACleanup();
    }
};












class matrix {
private:
    int n, m;
    vector<vector<double>> data;

public:
    matrix(int n, int m) : n(n), m(m), data(n, vector<double>(m)) {}
    int get_n ()const{
        return n ;
    }
    int get_m ()const{
        return m ;
    }
    void set_data(vector<vector<double>> in) {
        if (in.size() == n && in[0].size() == m) {
            for (int i = 0; i < n; i++)
                for (int j = 0; j < m; j++)
                    data[i][j] = in[i][j];
        }
    }
    vector<vector<double>>& get_data() {
        return data;
    }
    const vector<vector<double>>& get_data() const {

        return data;

    }
    void matrix_info() {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                cout << data[i][j] << ' ';
            }
            cout << endl;
        }
    }

    matrix inverse() {
        if (n == m) {
            matrix result(n, m);
            vector<vector<double>> aug(n, vector<double>(2 * m));

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j)
                    aug[i][j] = data[i][j];
                aug[i][m + i] = 1;
            }

            for (int i = 0; i < n; ++i) {
                if (aug[i][i] == 0) {
                    for (int j = i + 1; j < n; ++j) {
                        if (aug[j][i] != 0) {
                            swap(aug[i], aug[j]);
                            break;
                        }
                    }
                }
                double p = aug[i][i];
                if (p == 0) throw runtime_error("Matrix is not invertible");

                for (int j = 0; j < 2 * m; ++j)
                    aug[i][j] /= p;
                for (int k = 0; k < n; ++k) {
                    if (k == i) continue;
                    double factor = aug[k][i];
                    for (int j = 0; j < 2 * m; ++j)
                        aug[k][j] -= factor * aug[i][j];
                }
            }

            for (int i = 0; i < n; ++i)
                for (int j = 0; j < m; ++j)
                    result.data[i][j] = aug[i][j + m];
            return result;
        } else {
            throw runtime_error("Matrix is not square and cannot be inverted.");
        }
    }
    matrix operator * (matrix& other){
        matrix result (n,other.m) ;
        for(int i =0 ;i<n;i++){
            for(int j = 0 ; j<other.m;j++){
                double sum = 0;
                for (int k = 0; k<m;k++){
                    sum+=data[i][k]*other.data[k][j];
                }
                result.data[i][j]=sum;
            }
        }
        return result ;
    }
};
class invalid_command : public exception {
public:
    const char* what() const noexcept override {
        return "Error: Syntax error";
    }
};

class invalid_component : public exception {
public:
    const char* what() const noexcept override {
        return "Error: component does not exist";
    }
};

class inapropriate_input : public exception{
public:
    const char* what() const noexcept override {
        return "-Error : Inappropriate input";
    }
};

class same_component : public exception {
private:
    string name;
    string message;
public:
    explicit same_component(const string& c_name) : name(c_name) {
        if(c_name[0]=='R') {
            message = "Error: Resistor " + name + " already exists in the circuit";
        }
        else if(c_name[0]=='C'){
            message = "Error: Capacitor " + name + " already exists in the circuit";
        }
        else if(c_name[0]=='L'){
            message = "Error: Inductor " + name + " already exists in the circuit";
        }
        else if(c_name[0]=='D'){
            message = "Error: Diode " + name + " already exists in the circuit";
        }
        else if(c_name[0]=='V'){

            message = "Error: Voltage source " + name + " already exists in the circuit";
        }
        else if(c_name[0]=='I'){
            message = "Error: Current source " + name + " already exists in the circuit";
        }
        else {
            message = "Error: Component " + name + " already exists in the circuit";
        }
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class invalid_node : public exception {
public:
    const char* what() const noexcept override {
        return "Error: invalid node";
    }
};

class negative_resistance : public exception {
public:
    const char* what() const noexcept override {
        return "Error: Resistance cannot be zero or negative";
    }
};

class element_not_found : public exception {
private:
    string name;
    string message;
public:
    explicit element_not_found(const string& elem_name) : name(elem_name) {
        message = "Error: Element " + name + " not found in library";
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class Node {
private:
    string name;
    double v;
public:
    Node(string n) : name(n), v(0.0) {}
    double getVoltage() const { return v; }
    void setVoltage(double voltage) { v = voltage; }
    string get_name() const { return name; }
    void set_name(string n_name)  {
        name = n_name ;
    }
};

class Component {
protected:
    string name;
    Node* node1;
    Node* node2;
public:
    Component(string n, Node* n1, Node* n2)
            : name(n), node1(n1), node2(n2) {}
    virtual double getVoltage() const = 0;
    virtual double getCurrent() const = 0;
    virtual ~Component() = default;
    virtual string get_name() const {
        return name;
    }
    virtual Node* get_node1() const {
        return node1;
    }

    virtual Node* get_node2() const {
        return node2;
    }
};

class Resistor : public Component {
private:
    double val;
    double v;
    double I;
public:
    Resistor(double val, string n, Node* n1, Node* n2)
            : Component(n, n1, n2), val(val), v(0), I(0) {}
    double getVoltage() const override { return v; }
    double getCurrent() const override { return I; }
    double get_r() const { return val; }
};

class Diode : public Component {
private:
    string model ;
    double v;
    double I;
public:
    Diode(string model, string n, Node* n1, Node* n2)
            : Component(n, n1, n2), model(model), v(0), I(0) {}
    double getVoltage() const override { return v; }
    double getCurrent() const override { return I; }
    string get_m() const { return model; }
};

class VoltageSource : public Component {
private:
    double val;
    double v;
    double I;
public:
    VoltageSource(string name, double val, Node* n1, Node* n2)
            : Component(name, n1, n2), val(val), v(0), I(0) {}
    double getVoltage() const override { return v; }
    double getCurrent() const override { return I; }
    double get_val() const { return val; }
    void set_value(double new_val) { val = new_val; }
};

class CurrentSource : public Component {
private:
    double val;
    double v;
    double I;
public:
    CurrentSource(string name, double val, Node* n1, Node* n2)
            : Component(name, n1, n2), val(val), v(0), I(val) {}
    double getVoltage() const override { return v; }
    double getCurrent() const override { return I; }
    void set_current(double new_current) { val = new_current; }
};

class Capacitor : public Component {
private:
    double cap;
    double v;
    double I;
public:
    Capacitor(string name, double val, Node* n1, Node* n2)
            : Component(name, n1, n2), cap(val), v(0), I(0) {}
    double getVoltage() const override { return v; }
    double getCurrent() const override { return I; }
    double getCap() const{return cap;}
};

class Inductor : public Component {
private:
    double ind;
    double v;
    double I;
public:
    Inductor(string name, double val, Node* n1, Node* n2)
            : Component(name, n1, n2), ind(val), v(0), I(0) {}
    double getVoltage() const override { return v; }
    double getCurrent() const override { return I; }
    double getInd() const {return ind;}
};
class Ground {
private:
    Node* node;
public:
    Ground(Node* node) :  node(node) {}
    Node* getNode() const { return node; }
};
class Circuit {
public:
    vector<Node*> nodes;
    vector<Resistor> resistors;
    vector<VoltageSource> voltage_s;
    vector<CurrentSource> current_s;
    vector<Capacitor> capacitors;
    vector<Inductor> inductors;
    vector<Diode> diodes;
    vector<Ground> grounds;
    ~Circuit() {
        for (Node* n : nodes) delete n;
    }
    Node* get_or_create_node(const string& nodeName) {
        for (Node* n : nodes) {
            if (n->get_name() == nodeName) return n;
        }
        Node* newNode = new Node(nodeName);
        nodes.push_back(newNode);
        return newNode;

    }
    void deleteAll(){
        nodes.clear();
        resistors.clear();
        voltage_s.clear();
        current_s.clear();
        capacitors.clear();
        inductors.clear();
        diodes.clear();
        grounds.clear();
    }
    bool component_exists(const string& name) const {
        for (const auto& r : resistors) if (r.get_name() == name) return true;
        for (const auto& c : capacitors) if (c.get_name() == name) return true;
        for (const auto& l : inductors) if (l.get_name() == name) return true;
        for (const auto& d : diodes) if (d.get_name() == name) return true;
        for (const auto& v : voltage_s) if (v.get_name() == name) return true;
        for (const auto& cur : current_s) if (cur.get_name() == name) return true;
        return false;
    }
};

template <typename T>
bool delete_element(vector<T>& elements, const string& name, const string& type_name) {
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if (it->get_name() == name) {
            elements.erase(it);
            cout << type_name << " " << name << " deleted successfully." << endl;
            return true;
        }
    }
    return false;
}
class node_not_found : public exception {
    std::string message;
public:
    explicit node_not_found(const string& node_name) {
        message = "ERROR: Node " + node_name + " does not exist in the circuit";
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class duplicate_node_name : public exception {
    std::string message;
public:
    explicit duplicate_node_name(const string& node_name) {
        message = "ERROR: Node name " + node_name + " already exists";
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};



void update_component_references(Circuit* circuit, const string& old_name, const string& new_name) {
    for (auto& r : circuit->resistors) {
        if (r.get_node1()->get_name() == old_name) r.get_node1()->set_name(new_name);
        if (r.get_node2()->get_name() == old_name) r.get_node2()->set_name(new_name);
    }
    for (auto& c : circuit->capacitors) {
        if (c.get_node1()->get_name() == old_name) c.get_node1()->set_name(new_name);
        if (c.get_node2()->get_name() == old_name) c.get_node2()->set_name(new_name);
    }
    for (auto& l : circuit->inductors) {
        if (l.get_node1()->get_name() == old_name) l.get_node1()->set_name(new_name);
        if (l.get_node2()->get_name() == old_name) l.get_node2()->set_name(new_name);
    }
    for (auto& v : circuit->voltage_s) {
        if (v.get_node1()->get_name() == old_name) v.get_node1()->set_name(new_name);
        if (v.get_node2()->get_name() == old_name) v.get_node2()->set_name(new_name);
    }
    for (auto& i : circuit->current_s) {
        if (i.get_node1()->get_name() == old_name) i.get_node1()->set_name(new_name);
        if (i.get_node2()->get_name() == old_name) i.get_node2()->set_name(new_name);
    }
}

class Run {
private:
    string line;
    Circuit* circuit;
public:
    Run(string input, Circuit* c) : line(input), circuit(c) {}
    void parse(bool replace,double* dt,double* tEnd,map<string,int>* underConsideration) {
        try {
            istringstream ss(line);
            string word;
            string cmd, c_name, node1_n, node2_n;
            ss>>cmd;
            if(cmd == ".list") {
                string element_n;
                ss >> element_n;
                if (!ss) {
                    cout << "Available resistors:" << endl;
                    for (int i = 0; i < circuit->resistors.size(); i++) {
                        if (i) cout << ',';
                        cout << circuit->resistors[i].get_name();
                    }
                    cout << endl << "Available capacitors:" << endl;
                    for (int i = 0; i < circuit->capacitors.size(); i++) {
                        if (i) cout << ',';
                        cout << circuit->capacitors[i].get_name();
                    }
                    cout << endl << "Available inductors:" << endl;
                    for (int i = 0; i < circuit->inductors.size(); i++) {
                        if (i) cout << ',';
                        cout << circuit->inductors[i].get_name();
                    }
                    cout << endl << "Available diodes:" << endl;
                    for (int i = 0; i < circuit->diodes.size(); i++) {
                        if (i) cout << ',';
                        cout << circuit->diodes[i].get_name();
                    }
                    cout << endl;
                } else {
                    if (element_n == "R") {
                        cout << "Available resistors:" << endl;
                        for (int i = 0; i < circuit->resistors.size(); i++) {
                            if (i) cout << ',';
                            cout << circuit->resistors[i].get_name();
                        }
                        cout << endl;
                    }
                    if (element_n == "C") {
                        cout << "Available capacitors:" << endl;
                        for (int i = 0; i < circuit->capacitors.size(); i++) {
                            if (i) cout << ',';
                            cout << circuit->capacitors[i].get_name();
                        }
                        cout << endl;
                    }
                    if (element_n == "L") {
                        cout << "Available inductors:" << endl;
                        for (int i = 0; i < circuit->inductors.size(); i++) {
                            if (i) cout << ',';
                            cout << circuit->inductors[i].get_name();
                        }
                        cout << endl;
                    }
                    if (element_n == "D") {
                        cout << "Available diodes:" << endl;
                        for (int i = 0; i < circuit->diodes.size(); i++) {
                            if (i) cout << ',';
                            cout << circuit->diodes[i].get_name();
                        }
                        cout << endl;
                    }
                }
                return;
            }
            if(cmd == ".nodes"){
                cout << "Available nodes:" << endl;
                for(int i = 0 ; i < circuit->nodes.size();i++){
                    if(i) cout << ',';
                    cout << circuit->nodes[i]->get_name();
                }
                cout << endl;
                return;
            }
            if(cmd == ".rename") {
                string what, old_name, new_name;
                ss>>what>>old_name>>new_name;
                if (what!="node" || old_name.empty() || new_name.empty()) throw invalid_command();
                bool node_found = false;
                for (const Node* n : circuit->nodes) {
                    if (n->get_name() == old_name) {
                        node_found = true;
                        break;
                    }
                }
                if (!node_found) throw node_not_found(old_name);
                for (const Node* n : circuit->nodes) {
                    if (n->get_name() == new_name) throw duplicate_node_name(new_name);
                }
                for (int i = 0; i < circuit->nodes.size(); i++) {
                    if (circuit->nodes[i]->get_name() == old_name) {
                        circuit->nodes[i]->set_name(new_name);
                        update_component_references(circuit, old_name, new_name);
                        cout << "SUCCESS: Node renamed from " << old_name << " to " << new_name << endl;
                        return;
                    }
                }
            }
            if(cmd == "delete") {
                if (!(ss >> c_name)) throw invalid_command();
                bool found = delete_element(circuit->resistors, c_name, "Resistor")
                             || delete_element(circuit->capacitors, c_name, "Capacitor")
                             || delete_element(circuit->inductors, c_name, "Inductor")
                             || delete_element(circuit->diodes, c_name, "Diode")
                             || delete_element(circuit->voltage_s, c_name, "Voltage source")
                             || delete_element(circuit->current_s, c_name, "Current source");
                if (!found) throw element_not_found(c_name);
                return;
            }
            ss>>c_name>>node1_n;

            if(cmd==".TRAN"){
                try {
                    double a=stod(c_name);
                    double b=stod(node1_n);
                    *dt = stod(c_name);
                    *tEnd = stod(node1_n);
                    commands.push_back(line);
                    return;
                }
                catch (const exception& e){
                    cout<<e.what()<<endl;
                    *dt=0;
                    *tEnd=0;
                    return;
                };
            }
            else if(cmd==".print" && c_name=="TRAN"){
                try {
                    ss >> node2_n;
                    double a=stod(node1_n);
                    double b=stod(node2_n);
                    string extra;
                    ss >> extra;
                    string err;
                    map<string, int> &tempo = *underConsideration;
                    tempo[extra.substr(2, extra.size() - 3)] = 0;
                    while (ss >> extra) {
                        string nameComp = extra.substr(2, extra.size() - 3);
                        bool node_found = false;
                        for (const Node *n: circuit->nodes) {
                            if (n->get_name() == nameComp) {
                                node_found = true;
                                break;
                            }
                        }
                        if (!circuit->component_exists(nameComp) && !node_found) {
                            cout << "component or node " << nameComp << " not found " << nameComp << endl;
                            continue;
                        }
                        tempo[extra.substr(2, extra.size() - 3)] = 0;
                    }
                    *underConsideration = tempo;
                    *dt = stod(node1_n);
                    *tEnd = stod(node2_n);
                    commands.push_back(line);
                    return;
                }
                catch (const exception& e){
                    cout<<e.what()<<endl;
                    *dt=0;
                    *tEnd=0;
                    return;
                };
            }

            if (cmd != "add" || c_name.size() < 2) {
                if(!replace) throw invalid_command();
            }

            if ((c_name == "VoltageSource" || c_name == "CurrentSource")) {
                string node1_n_ , node2_n;
                double value;
                if (!(ss >> node1_n_ >> node2_n >> value)) {
                    throw invalid_command();
                }
                Node* node1 = circuit->get_or_create_node(node1_n);
                Node* node2 = circuit->get_or_create_node(node2_n);
                if (circuit->component_exists(node1_n))  throw same_component(node1_n);
                if (c_name == "VoltageSource") {
                    circuit->voltage_s.emplace_back(node1_n, value, node1, node2);
                    if (!replace) cout << "Component " << node1_n << " of type VoltageSource added successfully." << endl;
                    string addToFile = "V " + node1_n + " " + node1->get_name() + " " + node2->get_name() + " " + to_string(value);
                    commands.push_back(addToFile);
                } else if (c_name == "CurrentSource") {
                    circuit->current_s.emplace_back(node1_n, value, node1, node2);
                    if (!replace) cout << "Component " << node1_n << " of type CurrentSource added successfully." << endl;
                    string addToFile = "I " + node1_n + " " + node1->get_name() + " " + node2->get_name() + " " + to_string(value);
                    commands.push_back(addToFile);
                }
                return;
            }

            char type = c_name[0];
            if (type == 'G' && c_name == "GND") {
                string extra;
                if (ss >> extra)
                    throw invalid_command();
                Node* node1 = circuit->get_or_create_node(node1_n);
                circuit->grounds.emplace_back(node1);
                if(!replace) cout << "Ground added at node " << node1_n << " successfully." << endl;
                string addToFile="G GND "+node1->get_name();
                commands.push_back(addToFile);
            }
            else if (type == 'D') {
                if (!(ss >> node2_n))
                    throw invalid_command();
                string extra;
                string model;
                if (ss >> extra) {
                    if(extra=="D"){
                        model="default_model";
                    }
                    else if(extra=="Z"){
                        model="zener_model";
                    }
                    else
                        throw invalid_command();
                }
                if(ss>>extra)
                    throw invalid_command();
                Node* node1 = circuit->get_or_create_node(node1_n);
                Node* node2 = circuit->get_or_create_node(node2_n);
                if (circuit->component_exists(c_name))
                    throw same_component(c_name);
                circuit->diodes.emplace_back(model, c_name, node1, node2);
                if(!replace) cout << "Component " << c_name << " of type Diode added successfully." << endl;
                string addToFile="D "+c_name+" "+node1->get_name()+" "+node2->get_name();
                commands.push_back(addToFile);
            }
            else if (type == 'R' || type == 'C' || type == 'L' || type == 'V' || type == 'I') {
                if (!(ss >> node2_n)) {
                    throw invalid_command();
                }
                double value;
                if (!(ss >> value)) {
                    throw invalid_command();
                }
                string extra;
                if (ss >> extra) {
                    if(extra=="k"){
                        value*=1000;
                    }
                    else if(extra=="M"){
                        value*=pow(10,6);
                    }
                    else if(extra=="m"){
                        value*=pow(10,-3);
                    }
                    else if(extra=="u"){
                        value*=pow(10,-6);
                    }
                    else if(extra=="n"){
                        value*=pow(10,-9);
                    }
                    else{
                        throw invalid_command();
                    }
                }

                Node* node1 = circuit->get_or_create_node(node1_n);
                Node* node2 = circuit->get_or_create_node(node2_n);
                if (circuit->component_exists(c_name))
                    throw same_component(c_name);
                if ((type == 'R' || type == 'C' || type == 'L') && value <= 0)
                    throw negative_resistance();
                if (type == 'R')
                    circuit->resistors.emplace_back(value, c_name, node1, node2);
                else if (type == 'C')
                    circuit->capacitors.emplace_back(c_name, value, node1, node2);
                else if (type == 'L')
                    circuit->inductors.emplace_back(c_name, value, node1, node2);
                else if (type == 'V')
                    circuit->voltage_s.emplace_back(c_name, value, node1, node2);

                else if (type == 'I')
                    circuit->current_s.emplace_back(c_name, value, node1, node2);
                if(!replace) cout << "Component " << c_name << " of type " << type << " added successfully." << endl;
                string addToFile="";
                string typeString(1,type);
                addToFile= typeString+" "+c_name+" "+node1->get_name()+" "+node2->get_name()+" "+to_string(value);
                commands.push_back(addToFile);
            }
            else {
                throw element_not_found(string(1, type));
            }
        }
        catch (const exception& e) {
            cout << e.what() << endl;
        }
    }

    void parse_netlist_line(const string& line, Circuit* circuit) {
        istringstream ss(line);
        string token;
        ss >> token;
        if(token.empty() || token[0] == '*' || token[0] == '.') return;
        if(token == "GND" || token == "0") {
            string nodeName = token == "GND" ? "" : token;
            if(ss >> nodeName || token == "GND") {
                Node* node = circuit->get_or_create_node(nodeName);
                circuit->grounds.emplace_back(node);
            }
            return;
        }
        char type = token[0];
        string name = token;
        string n1, n2;
        double value = 0;
        ss >> n1 >> n2;
        if(type == 'D') {
            Node* node1 = circuit->get_or_create_node(n1);
            Node* node2 = circuit->get_or_create_node(n2);
            string model = "default_model";
            circuit->diodes.emplace_back(model, name, node1, node2);
            return;
        }
        ss >> value;
        string suffix;
        ss >> suffix;
        if(suffix == "k") value *= 1e3;
        else if(suffix == "M") value *= 1e6;
        else if(suffix == "m") value *= 1e-3;
        else if(suffix == "u") value *= 1e-6;
        else if(suffix == "n") value *= 1e-9;
        Node* node1 = circuit->get_or_create_node(n1);
        Node* node2 = circuit->get_or_create_node(n2);
        switch(type) {
            case 'R': circuit->resistors.emplace_back(value, name, node1, node2); break;
            case 'C': circuit->capacitors.emplace_back(name, value, node1, node2); break;
            case 'L': circuit->inductors.emplace_back(name, value, node1, node2); break;
            case 'V': circuit->voltage_s.emplace_back(name, value, node1, node2); break;
            case 'I': circuit->current_s.emplace_back(name, value, node1, node2); break;
            default: break;
        }
    }


};
struct TransientResult {
    double time;
    double voltage;
    string node ;
};
struct CurrentResult {
    double time;
    double current;
    string component;
};
vector<TransientResult> transientResults;
vector<CurrentResult> currentResults;
class MNA {
private:
    Circuit* circuit;
    map<string, int> nodeIndex;
    int N;
    int M;
    bool is_ground(Node* node) {
        for (auto& g : circuit->grounds)
            if (g.getNode()->get_name() == node->get_name())
                return true;
        return false;
    }
    int node_pos(Node* node) {
        string name = node->get_name();
        if (is_ground(node)) return -1;
        return nodeIndex[name];
    }
    pair<VoltageSource*, CurrentSource*> findSource(const string& sourceName) {
        for (auto& vs : circuit->voltage_s) {
            if (vs.get_name() == sourceName) {
                return {&vs, nullptr};
            }
        }
        for (auto& cs : circuit->current_s) {
            if (cs.get_name() == sourceName) {
                return {nullptr, &cs};
            }
        }
        return {nullptr, nullptr};
    }
    void set_sVal(pair<VoltageSource*, CurrentSource*> source, double value) {
        if (source.first) {
            source.first->set_value(value);
        }  else if (source.second) {
            source.second->set_current(value);
        }

    }
    double get_sVal(pair<VoltageSource*, CurrentSource*> source) {
        if (source.first) {
            return source.first->get_val();
        } else if (source.second) {
            return source.second->getCurrent();
        }
        return 0.0;
    }
    void print_table(const string& sourceName,const vector<string>& printNodes,const vector<string>& printCurrents) {
        cout << "DC Sweep Results:" << endl;
        cout << setw(15) << sourceName << " Value";
        for (const auto& node : printNodes) {
            cout << setw(15) << "V(" + node + ")";
        }
        for (const auto& current : printCurrents) {
            cout << setw(15) << "I(" + current + ")";
        }
        cout << endl;
        cout << string(15 + 15*(printNodes.size() + printCurrents.size()), '-') << endl;
    }
    void print_results(double sourceValue,const matrix& x,const vector<string>& printNodes,const vector<string>& printCurrents) {
        cout << fixed << setprecision(4) << setw(15) << sourceValue;
        for (const auto& node : printNodes) {
            bool found = false;
            for (const auto& pair : nodeIndex) {
                if (pair.first == node) {
                    cout << setw(15) << x.get_data()[pair.second][0];
                    found = true;
                    break;
                }
            }
            if (!found && node == "0") {
                cout << setw(15) << "0.0000"; // Ground
            } else if (!found) {
                cout << setw(15) << "N/A";
            }
        }
        for (const auto& current : printCurrents) {
            bool found = false;
            for (size_t i = 0; i < circuit->voltage_s.size(); ++i) {
                if (circuit->voltage_s[i].get_name() == current) {
                    cout << setw(15) << x.get_data()[N + i][0];
                    found = true;
                    break;
                }
            }
            for (size_t i = 0; i < circuit->current_s.size(); ++i) {
                if (circuit->current_s[i].get_name() == current) {
                    cout << setw(15) << x.get_data()[N + i][0];
                    found = true;
                    break;
                }
            }
            for (size_t i = 0; i < circuit->resistors.size(); ++i) {
                if (circuit->resistors[i].get_name() == current) {
                    int n1 = node_pos(circuit->resistors[i].get_node1());
                    int n2 = node_pos(circuit->resistors[i].get_node2());
                    double V1=0,V2=0;
                    if(n1>=0) V1 = x.get_data()[n1][0];
                    if(n2>=0) V2 = x.get_data()[n2][0];
                    double I_R = (V1 - V2) / circuit->resistors[i].get_r();
                    cout << setw(15) << I_R;
                    found = true;
                    break;
                }
            }
            for (size_t i = 0; i < circuit->capacitors.size(); ++i) {
                if (circuit->capacitors[i].get_name() == current) {
                    double I_C = 0;
                    cout << setw(15) << I_C;
                    found = true;
                    break;
                }
            }


            if (!found) {
                cout << setw(15) << "N/A";
            }
        }
        cout << endl;
    }

public:
    explicit MNA(Circuit* c) : circuit(c), N(0), M(0) {}


    void prepare() {
        nodeIndex.clear();
        int idx = 0;
        for (auto n : circuit->nodes) {
            if (!is_ground(n)) {
                nodeIndex[n->get_name()] = idx++;
            }
        }
        N = idx;
        M = (int)circuit->voltage_s.size();
    }

    //------------------------------------------------------------
    void analyzeTransient(double dt, double t_end,map<string,int> underConsideration) {
        transientResults.clear();
        currentResults.clear();
        prepare();
        if (N == 0) {
            cout << "No non-ground nodes to analyze." << endl;
            return;
        }

        vector<double> node_voltages_guess(N, 0.0);
        vector<double> node_voltages_prev(N, 0.0);

        const int max_NewtonR_iter = 30;
        const double NRstop = 1e-5;
        for (double t = dt; t <= t_end; t += dt) {
            bool converged = false;

            for (int iter = 0; iter < max_NewtonR_iter; ++iter) {
                matrix A(N + M, N + M);
                matrix z(N + M, 1);
                for (int i = 0; i < N + M; ++i) {
                    for (int j = 0; j < N + M; ++j)
                        A.get_data()[i][j] = 0.0;
                    z.get_data()[i][0] = 0.0;
                }
                for (const auto &r: circuit->resistors) {
                    int n1 = node_pos(r.get_node1());
                    int n2 = node_pos(r.get_node2());
                    double g = 1.0 / r.get_r();

                    if (n1 >= 0) A.get_data()[n1][n1] += g;
                    if (n2 >= 0) A.get_data()[n2][n2] += g;
                    if (n1 >= 0 && n2 >= 0) {
                        A.get_data()[n1][n2] -= g;
                        A.get_data()[n2][n1] -= g;
                    }
                }
                for (const auto &cur: circuit->current_s) {
                    int n1 = node_pos(cur.get_node1());
                    int n2 = node_pos(cur.get_node2());
                    double I = cur.getCurrent();
                    if (n1 >= 0) z.get_data()[n1][0] -= I;
                    if (n2 >= 0) z.get_data()[n2][0] += I;
                }

                for (size_t i = 0; i < circuit->voltage_s.size(); ++i) {
                    auto &vs = circuit->voltage_s[i];
                    int n1 = node_pos(vs.get_node1());
                    int n2 = node_pos(vs.get_node2());
                    if (n1 >= 0) {
                        A.get_data()[n1][N + i] += 1;
                        A.get_data()[N + i][n1] += 1;
                    }
                    if (n2 >= 0) {
                        A.get_data()[n2][N + i] -= 1;
                        A.get_data()[N + i][n2] -= 1;
                    }
                    z.get_data()[N + i][0] = vs.get_val();
                }

                for (const auto &cap: circuit->capacitors) {
                    int n1 = node_pos(cap.get_node1());
                    int n2 = node_pos(cap.get_node2());
                    double Gc = cap.getCap() / dt;
                    double Ieq = 0;

                    double Vprev_n1 = (n1 >= 0) ? node_voltages_prev[n1] : 0.0;
                    double Vprev_n2 = (n2 >= 0) ? node_voltages_prev[n2] : 0.0;
                    Ieq = Gc * (Vprev_n1 - Vprev_n2);

                    if (n1 >= 0) {
                        A.get_data()[n1][n1] += Gc;
                        z.get_data()[n1][0] += Ieq;
                    }
                    if (n2 >= 0) {
                        A.get_data()[n2][n2] += Gc;
                        z.get_data()[n2][0] += -Ieq;
                    }
                    if (n1 >= 0 && n2 >= 0) {
                        A.get_data()[n1][n2] -= Gc;
                        A.get_data()[n2][n1] -= Gc;
                    }
                }

                for (const auto &ind: circuit->inductors) {
                    int n1 = node_pos(ind.get_node1());
                    int n2 = node_pos(ind.get_node2());
                    double Gl = dt / ind.getInd();

                    if (n1 >= 0) A.get_data()[n1][n1] += Gl;
                    if (n2 >= 0) A.get_data()[n2][n2] += Gl;
                    if (n1 >= 0 && n2 >= 0) {
                        A.get_data()[n1][n2] -= Gl;
                        A.get_data()[n2][n1] -= Gl;
                    }
                }
                for (const auto &dio: circuit->diodes) {
                    int n1 = node_pos(dio.get_node1());
                    int n2 = node_pos(dio.get_node2());
                    double g_diode = 1e9;
                    double Vd = 0.0;
                    if (n1 >= 0 && n2 >= 0) {
                        Vd = node_voltages_guess[n1] - node_voltages_guess[n2];
                    }
                    if (Vd > 0) {
                        if (n1 >= 0) A.get_data()[n1][n1] += g_diode;
                        if (n2 >= 0) A.get_data()[n2][n2] += g_diode;
                        if (n1 >= 0 && n2 >= 0) {
                            A.get_data()[n1][n2] -= g_diode;
                            A.get_data()[n2][n1] -= g_diode;
                        }
                        double Id = g_diode * Vd;
                        if (n1 >= 0) z.get_data()[n1][0] -= Id;
                        if (n2 >= 0) z.get_data()[n2][0] += Id;
                    } else {
                        if (n1 >= 0) A.get_data()[n1][n1] -= g_diode;
                        if (n2 >= 0) A.get_data()[n2][n2] -= g_diode;
                        if (n1 >= 0 && n2 >= 0) {
                            A.get_data()[n1][n2] += g_diode;
                            A.get_data()[n2][n1] += g_diode;
                        }
                        double Id = g_diode * Vd;
                        if (n1 >= 0) z.get_data()[n1][0] += Id;
                        if (n2 >= 0) z.get_data()[n2][0] -= Id;
                    }
                }

                matrix x(N + M, N + M);
                try {
                    x = A.inverse() * z;
                } catch (const exception &e) {
                    cout << "Matrix inversion failed at time " << t << " iteration " << iter << ": " << e.what()
                         << endl;
                    break;
                }
                double max_diff = 0.0;
                for (int i = 0; i < N; ++i) {
                    double diff = fabs(x.get_data()[i][0] - node_voltages_guess[i]);
                    if (diff > max_diff) max_diff = diff;
                    node_voltages_guess[i] = x.get_data()[i][0];
                }
                if (max_diff < NRstop) {
                    converged = true;
                    break;
                }
            }

            if (!converged) {
                cout << "Warning: Newton-Raphson did not converge at time " << t << endl;
            }

            for (const auto &pair: nodeIndex) {
                const string &name = pair.first;
                int idx = pair.second;
                for (auto n: circuit->nodes) {
                    if (n->get_name() == name) {
                        n->setVoltage(node_voltages_guess[idx]);
                        break;
                    }
                }
            }
            cout << fixed << setprecision(4);
            cout << "Time " << t << " s:\n";
            for (const auto &pair: nodeIndex) {
                if (underConsideration.size()==0|| underConsideration.find(pair.first)!=underConsideration.end()) {
                    if(node_voltages_guess[pair.second]!=0) {
                        cout << "  Voltage at node " << pair.first << " = " << -1 * node_voltages_guess[pair.second]<<endl;
                        TransientResult node = {t,-1 * node_voltages_guess[pair.second],pair.first };
                        transientResults.push_back(node);
                    }
                    else{
                        cout << "  Voltage at node " << pair.first << " = " << node_voltages_guess[pair.second]<<endl;
                        TransientResult node = {t,node_voltages_guess[pair.second],pair.first };
                        transientResults.push_back(node);

                    }
                }
            }

            for (const auto &r: circuit->resistors) {
                if(underConsideration.find(r.get_name())!=underConsideration.end() || underConsideration.size()==0) {
                    int n1 = node_pos(r.get_node1());
                    int n2 = node_pos(r.get_node2());
                    double V1=0,V2=0;
                    if(n1>=0) V1 = node_voltages_guess[n1];
                    if(n2>=0) V2 = node_voltages_guess[n2];
                    double I_R = (V1 - V2) / r.get_r();
                    if(I_R!=0) {
                        cout << "Current through resistor " << r.get_name() << " = " <<-1* I_R << " A" << endl;
                        CurrentResult res = {t , -1* I_R , r.get_name() };
                        currentResults.push_back(res);
                    }
                    else{
                        cout << "Current through resistor " << r.get_name() << " = " <<I_R << " A" << endl;
                        CurrentResult res = {t , I_R , r.get_name() };
                        currentResults.push_back(res);
                    }
                }
            }
            for (const auto &cap: circuit->capacitors) {
                if(underConsideration.find(cap.get_name())!=underConsideration.end() || underConsideration.size()==0) {
                    int n1 = node_pos(cap.get_node1());
                    int n2 = node_pos(cap.get_node2());
                    double V1=0,V2=0,V1_prev=0,V2_prev=0;
                    if(n1>=0) {V1 = node_voltages_guess[n1]; V1_prev = node_voltages_prev[n1];}
                    if(n2>=0) {V2 = node_voltages_guess[n2]; V2_prev = node_voltages_prev[n2];}
                    double I_C = cap.getCap() * ((V1 - V2) - (V1_prev - V2_prev)) / dt;
                    if(I_C!=0) {
                        cout << "Current through capacitor " << cap.get_name() << " = " <<-1* I_C << " A" << endl;
                        CurrentResult res = {t , -1*I_C , cap.get_name() };
                        currentResults.push_back(res);
                    }
                    else{
                        cout << "Current through capacitor " << cap.get_name() << " = " << I_C << " A" << endl;
                        CurrentResult res = {t ,I_C , cap.get_name() };
                        currentResults.push_back(res);
                    }
                }
            }
            for (const auto &ind: circuit->inductors) {
                if(underConsideration.find(ind.get_name())!=underConsideration.end() || underConsideration.size()==0) {
                    int n1 = node_pos(ind.get_node1());
                    int n2 = node_pos(ind.get_node2());
                    double V1=0,V2=0,V1_prev=0,V2_prev=0;
                    if(n1>=0) {V1 = node_voltages_guess[n1]; V1_prev = node_voltages_prev[n1];}
                    if(n2>=0) {V2 = node_voltages_guess[n2]; V2_prev = node_voltages_prev[n2];}
                    double I_L = (dt/ind.getInd()) * (V1 - V2) + (dt/ind.getInd())*(V1_prev-V2_prev);//I_l=1/L integral(Vdv) -> I_0+1/L integral = I_0 + 1/L V (dt->0)
                    if(I_L!=0) {
                        cout << "Current through inductor " << ind.get_name() << " = " <<-1* I_L << " A" << endl;
                        CurrentResult res = {t ,-1*I_L , ind.get_name() };
                        currentResults.push_back(res);
                    }
                    else{
                        cout << "Current through inductor " << ind.get_name() << " = " <<I_L << " A" << endl;
                        CurrentResult res = {t ,I_L , ind.get_name() };
                        currentResults.push_back(res);
                    }
                }
            }

            for (const auto &dio: circuit->diodes) {
                if(underConsideration.find(dio.get_name())!=underConsideration.end() || underConsideration.size()==0) {
                    int n1 = node_pos(dio.get_node1());
                    int n2 = node_pos(dio.get_node2());
                    double V1=0,V2=0;
                    if(n1>=0) V1 = node_voltages_guess[n1];
                    if(n2>=0) V2 = node_voltages_guess[n2];
                    double Vd = V1 - V2;
                    double g_diode = 1e9;
                    double I_diode = g_diode * Vd;
                    if(I_diode!=0) {
                        cout << "Current through diode " << dio.get_name() << " = " <<-1* I_diode << " A" << endl;
                    }
                    else{
                        cout << "Current through diode " << dio.get_name() << " = " <<I_diode << " A" << endl;
                    }
                }
            }
            cout << endl;
            node_voltages_prev = node_voltages_guess;
        }
    }


    //-------------------------------------
    void analyze() {
        prepare();
        if (N == 0) {
            cout << "No non-ground nodes to analyze." << endl;
            return;
        }
        /* for(int i=0;i<commands.size();i++){
             cout<<commands[i]<<endl;
         }*/
        matrix A(N + M, N + M);
        matrix z(N + M, 1);
        for (int i = 0; i < N + M; ++i) {
            for (int j = 0; j < N + M; ++j)
                A.get_data()[i][j] = 0.0;
            z.get_data()[i][0] = 0.0;
        }
        for (const auto &r: circuit->resistors) {
            int n1 = node_pos(r.get_node1());
            int n2 = node_pos(r.get_node2());
            double g = 1.0 / r.get_r();

            if (n1 >= 0) A.get_data()[n1][n1] += g;
            if (n2 >= 0) A.get_data()[n2][n2] += g;
            if (n1 >= 0 && n2 >= 0) {
                A.get_data()[n1][n2] -= g;
                A.get_data()[n2][n1] -= g;
            }
        }
        for (const auto &cur: circuit->current_s) {
            int n1 = node_pos(cur.get_node1());
            int n2 = node_pos(cur.get_node2());
            double I = cur.getCurrent();
            if (n1 >= 0) z.get_data()[n1][0] -= I;
            if (n2 >= 0) z.get_data()[n2][0] += I;
        }
        for (size_t i = 0; i < circuit->voltage_s.size(); ++i) {
            auto &vs = circuit->voltage_s[i];
            int n1 = node_pos(vs.get_node1());
            int n2 = node_pos(vs.get_node2());
            if (n1 >= 0) {
                A.get_data()[n1][N + i] += 1;
                A.get_data()[N + i][n1] += 1;
            }
            if (n2 >= 0) {
                A.get_data()[n2][N + i] -= 1;
                A.get_data()[N + i][n2] -= 1;
            }
            z.get_data()[N + i][0] = vs.get_val();
        }
        //adding Capacitor for DC after a long time---------------------------------------------------------
        for (const auto &cap: circuit->capacitors) {
            int n1 = node_pos(cap.get_node1());
            int n2 = node_pos(cap.get_node2());
            double g = 0;

            if (n1 >= 0) A.get_data()[n1][n1] += g;
            if (n2 >= 0) A.get_data()[n2][n2] += g;
            if (n1 >= 0 && n2 >= 0) {
                A.get_data()[n1][n2] -= g;
                A.get_data()[n2][n1] -= g;
            }
        }
        //adding inductor for DC after a long time-------------------------------------------------------------
        for (const auto &ind: circuit->inductors) {
            int n1 = node_pos(ind.get_node1());
            int n2 = node_pos(ind.get_node2());
            double g = 1e9;

            if (n1 >= 0) A.get_data()[n1][n1] += g;
            if (n2 >= 0) A.get_data()[n2][n2] += g;
            if (n1 >= 0 && n2 >= 0) {
                A.get_data()[n1][n2] -= g;
                A.get_data()[n2][n1] -= g;
            }
        }
        //adding Diode for DC---------------------------------------------------------------------------------
        matrix x0(N + M, N + M);
        try {
            x0 = A.inverse() * z;
        } catch (const exception &e) {
            cout << "MNA analysis failed:" << e.what() << endl;
            return;
        }

        for (const auto &dio: circuit->diodes) {
            int n1 = node_pos(dio.get_node1());
            int n2 = node_pos(dio.get_node2());
            double g = 1e9;

            double v_diode = 0.0;
            if (n1 >= 0) v_diode += x0.get_data()[n1][0];
            if (n2 >= 0) v_diode -= x0.get_data()[n2][0];

            if (v_diode > 0.0) {  // Forward-biased
                if (n1 >= 0) A.get_data()[n1][n1] += g;
                if (n2 >= 0) A.get_data()[n2][n2] += g;
                if (n1 >= 0 && n2 >= 0) {
                    A.get_data()[n1][n2] -= g;
                    A.get_data()[n2][n1] -= g;
                }
            }
        }

        try {
            matrix x = A.inverse() * z;
            cout << fixed << setprecision(4);
            for (const auto &pair: nodeIndex) {
                const auto &name = pair.first;
                const auto &index = pair.second;
                for (auto n: circuit->nodes) {
                    if (n->get_name() == name) {
                        n->setVoltage(x.get_data()[index][0]);
                        break;
                    }
                }
                cout << "Voltage at node " << name << " = " << -1 * x.get_data()[index][0] << " V\n";
            }
            for (size_t i = 0; i < circuit->voltage_s.size(); ++i) {
                double I_vs = x.get_data()[N + i][0];
                cout << "Current through voltage source " << circuit->voltage_s[i].get_name() << " = " << -1 * I_vs
                     << " A\n";
            }
        } catch (const exception &e) {
            cout << "MNA analysis failed: " << e.what() << endl;
        }
    }
    void analyzeDCSweep(const string& sourceName, double start, double end, double increment,
                        const vector<string>& printNodes, const vector<string>& printCurrents) {
        auto source = findSource(sourceName);
        if (!source.first && !source.second) {
            cout << "Error: Source " << sourceName << " not found." << endl;
            return;
        }
        double originalValue = get_sVal(source);
        print_table(sourceName, printNodes, printCurrents);
        for (double value = start; value <= end + 1e-9; value += increment) {
            set_sVal(source, value);
            prepare();
            if (N == 0) {
                cout << "No non-ground nodes to analyze." << endl;
                continue;
            }
            matrix A(N + M, N + M);
            matrix z(N + M, 1);
            for (int i = 0; i < N + M; ++i) {
                for (int j = 0; j < N + M; ++j)
                    A.get_data()[i][j] = 0.0;
                z.get_data()[i][0] = 0.0;
            }
            for (const auto &ind: circuit->inductors) {
                int n1 = node_pos(ind.get_node1());
                int n2 = node_pos(ind.get_node2());
                double g = 1e9;

                if (n1 >= 0) A.get_data()[n1][n1] += g;
                if (n2 >= 0) A.get_data()[n2][n2] += g;
                if (n1 >= 0 && n2 >= 0) {
                    A.get_data()[n1][n2] -= g;
                    A.get_data()[n2][n1] -= g;
                }
            }

            for (const auto &cap: circuit->capacitors) {
                int n1 = node_pos(cap.get_node1());
                int n2 = node_pos(cap.get_node2());
                double g = 0;

                if (n1 >= 0) A.get_data()[n1][n1] += g;
                if (n2 >= 0) A.get_data()[n2][n2] += g;
                if (n1 >= 0 && n2 >= 0) {
                    A.get_data()[n1][n2] -= g;
                    A.get_data()[n2][n1] -= g;
                }
            }

            for (const auto &r: circuit->resistors) {
                int n1 = node_pos(r.get_node1());
                int n2 = node_pos(r.get_node2());
                double g = 1.0 / r.get_r();
                if (n1 >= 0) A.get_data()[n1][n1] += g;
                if (n2 >= 0) A.get_data()[n2][n2] += g;
                if (n1 >= 0 && n2 >= 0) {
                    A.get_data()[n1][n2] -= g;
                    A.get_data()[n2][n1] -= g;
                }
            }
            for (const auto &cur: circuit->current_s) {
                int n1 = node_pos(cur.get_node1());
                int n2 = node_pos(cur.get_node2());
                double I = cur.getCurrent();
                if (n1 >= 0) z.get_data()[n1][0] -= I;
                if (n2 >= 0) z.get_data()[n2][0] += I;
            }
            for (size_t i = 0; i < circuit->voltage_s.size(); ++i) {
                auto &vs = circuit->voltage_s[i];
                int n1 = node_pos(vs.get_node1());
                int n2 = node_pos(vs.get_node2());
                if (n1 >= 0) {
                    A.get_data()[n1][N + i] += 1;
                    A.get_data()[N + i][n1] += 1;
                }
                if (n2 >= 0) {
                    A.get_data()[n2][N + i] -= 1;
                    A.get_data()[N + i][n2] -= 1;
                }
                z.get_data()[N + i][0] = vs.get_val();}
            try {
                matrix x = A.inverse() * z;
                print_results(value, x, printNodes, printCurrents);
            } catch (const exception& e) {
                cout << "Analysis failed at " << value << ": " << e.what() << endl;
            }
        }
        set_sVal(source, originalValue);
    }
};
SDL_Window* window;
SDL_Renderer* renderer;

const int WINDOW_WIDTH = 1100;
const int WINDOW_HEIGHT = 1000;
const int GRID_SIZE = 20;

enum Mode {
    NORMAL,
    PREVIEW,
    EDITING,
    PLACING_PROBES
};

enum ComponentType {
    NONE,
    RESISTOR,
    CAPACITOR,
    INDUCTOR,
    VOLTAGE_SOURCE,
    CURRENT_SOURCE,
    Ground,
    VOLTAGE_PROBE,
    CURRENT_PROBE
};

enum Orientation {
    RIGHT = 0,
    DOWN = 1,
    LEFT = 2,
    UP = 3
};

struct C {
    ComponentType type;
    SDL_Point position;
    Orientation orientation;
    string name;
    string value;

};




struct Probe {
    ComponentType type;
    SDL_Point position;
    string name;
    string target;
    bool isActive;
};
vector<Probe> voltageProbes;
vector<Probe> currentProbes;
vector<C> components;
vector<C> resistors;
vector<C> capacitors;
vector<C> inductors;
vector<C> voltageSources;
vector<C> currentSources;
vector<C> grounds ;
vector<pair<SDL_Point, SDL_Point>> wires;
C* selectedComponent = nullptr;
TTF_Font* font = nullptr;
void drawText(const string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
C* getComponentAt(int mx, int my) {
    for (auto &c : components) {
        int w = 40;
        int h = 20;
        SDL_Rect rect = { c.position.x - w/2, c.position.y - h/2, w, h };
        if (mx >= rect.x && mx <= rect.x + rect.w &&
            my >= rect.y && my <= rect.y + rect.h) {
            return &c;
        }
    }
    return nullptr;
}
int roundToGrid(int value, int gridSize) {
    return (int)(round((float)value / gridSize)) * gridSize;
}

void draw_grid() {
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    for (int x = 0; x < WINDOW_WIDTH; x += GRID_SIZE)
        SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
    for (int y = 0; y < WINDOW_HEIGHT; y += GRID_SIZE)
        SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);
}
struct ToolbarButton {
    SDL_Rect rect;
    string text ;
    ComponentType type;
};
vector<ToolbarButton> toolbarButtons;
void draw_toolbar() {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect toolbarRect = {0, 60, WINDOW_WIDTH, 80};
    SDL_RenderFillRect(renderer, &toolbarRect);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color black = {0, 0, 0, 255};
    for (auto &btn : toolbarButtons) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &btn.rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &btn.rect);
        int textX = btn.rect.x + (btn.rect.w / 2) - (btn.text.size() * 5);
        int textY = btn.rect.y + (btn.rect.h / 2) - 10;
        drawText(btn.text, textX, textY, black);
    }
}
void draw_resistor(SDL_Point pos, Orientation o) {
    int w = 80, h = 20;
    SDL_Rect rectR;
    SDL_Rect rectU;
    SDL_Rect rectD;
    SDL_Rect rectL;
    switch (o) {
        case RIGHT:
            SDL_RenderDrawLine(renderer, pos.x - w/2, pos.y, pos.x - w/4, pos.y);
            rectR = { pos.x - w/4, pos.y - h/2, w/2, h };
            SDL_RenderDrawRect(renderer, &rectR);
            SDL_RenderDrawLine(renderer, pos.x + w/4, pos.y, pos.x + w/2, pos.y);
            break;

        case LEFT:
            SDL_RenderDrawLine(renderer, pos.x + w/2, pos.y, pos.x + w/4, pos.y);
            rectL = { pos.x - w/4, pos.y - h/2, w/2, h };
            SDL_RenderDrawRect(renderer, &rectL);
            SDL_RenderDrawLine(renderer, pos.x - w/4, pos.y, pos.x - w/2, pos.y);
            break;

        case DOWN:
            SDL_RenderDrawLine(renderer, pos.x, pos.y - w/2, pos.x, pos.y - w/4);
            rectD = { pos.x - h/2, pos.y - w/4, h, w/2 };
            SDL_RenderDrawRect(renderer, &rectD);
            SDL_RenderDrawLine(renderer, pos.x, pos.y + w/4, pos.x, pos.y + w/2);
            break;

        case UP:
            SDL_RenderDrawLine(renderer, pos.x, pos.y + w/2, pos.x, pos.y + w/4);
            rectU = { pos.x - h/2, pos.y - w/4, h, w/2 };
            SDL_RenderDrawRect(renderer, &rectU);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - w/4, pos.x, pos.y - w/2);
            break;
    }
}
void draw_inductor(SDL_Point pos, Orientation o) {
    int radius = 5;
    int spacing = 10;

    switch (o) {
        case RIGHT: {
            int startX = pos.x - 40;
            SDL_RenderDrawLine(renderer, startX, pos.y, startX + 5, pos.y);

            for (int i = 0; i < 4; i++) {
                int cx = startX + 10 + i * spacing;
                for (int angle = 0; angle < 180; angle += 10) {
                    int x1 = cx + cos(angle * M_PI / 180.0) * radius;
                    int y1 = pos.y + sin(angle * M_PI / 180.0) * radius;
                    int x2 = cx + cos((angle + 10) * M_PI / 180.0) * radius;
                    int y2 = pos.y + sin((angle + 10) * M_PI / 180.0) * radius;
                    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
                }
            }

            SDL_RenderDrawLine(renderer, startX + 50, pos.y, startX + 60, pos.y);
            break;
        }
        case LEFT: {
            int startX = pos.x + 40;
            SDL_RenderDrawLine(renderer, startX, pos.y, startX - 5, pos.y);

            for (int i = 0; i < 4; i++) {
                int cx = startX - 10 - i * spacing;
                for (int angle = 0; angle < 180; angle += 10) {
                    int x1 = cx - cos(angle * M_PI / 180.0) * radius;
                    int y1 = pos.y + sin(angle * M_PI / 180.0) * radius;
                    int x2 = cx - cos((angle + 10) * M_PI / 180.0) * radius;
                    int y2 = pos.y + sin((angle + 10) * M_PI / 180.0) * radius;
                    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
                }
            }

            SDL_RenderDrawLine(renderer, startX - 50, pos.y, startX - 60, pos.y);
            break;
        }
        case UP: {
            int startY = pos.y + 40;
            SDL_RenderDrawLine(renderer, pos.x, startY, pos.x, startY - 5);

            for (int i = 0; i < 4; i++) {
                int cy = startY - 10 - i * spacing;
                for (int angle = 0; angle < 180; angle += 10) {
                    int x1 = pos.x + sin(angle * M_PI / 180.0) * radius;
                    int y1 = cy - cos(angle * M_PI / 180.0) * radius;
                    int x2 = pos.x + sin((angle + 10) * M_PI / 180.0) * radius;
                    int y2 = cy - cos((angle + 10) * M_PI / 180.0) * radius;
                    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
                }
            }

            SDL_RenderDrawLine(renderer, pos.x, startY - 50, pos.x, startY - 60);
            break;
        }
        case DOWN: {
            int startY = pos.y - 40;
            SDL_RenderDrawLine(renderer, pos.x, startY, pos.x, startY + 5);

            for (int i = 0; i < 4; i++) {
                int cy = startY + 10 + i * spacing;
                for (int angle = 0; angle < 180; angle += 10) {
                    int x1 = pos.x + sin(angle * M_PI / 180.0) * radius;
                    int y1 = cy + cos(angle * M_PI / 180.0) * radius;
                    int x2 = pos.x + sin((angle + 10) * M_PI / 180.0) * radius;
                    int y2 = cy + cos((angle + 10) * M_PI / 180.0) * radius;
                    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
                }
            }

            SDL_RenderDrawLine(renderer, pos.x, startY + 50, pos.x, startY + 60);
            break;
        }
    }
}
void draw_ground(SDL_Point pos, Orientation o) {
    int r = 15;

    switch (o) {
        case RIGHT:
            SDL_RenderDrawLine(renderer, pos.x - 30, pos.y, pos.x +30, pos.y);
            SDL_RenderDrawLine(renderer, pos.x -20, pos.y+10, pos.x + 20, pos.y+10);
            SDL_RenderDrawLine(renderer, pos.x - 10, pos.y+20, pos.x + 10, pos.y+20);
            break;

        case LEFT:
            SDL_RenderDrawLine(renderer, pos.x - 30, pos.y, pos.x +30, pos.y);
            SDL_RenderDrawLine(renderer, pos.x -20, pos.y-10, pos.x + 20, pos.y-10);
            SDL_RenderDrawLine(renderer, pos.x - 10, pos.y-20, pos.x + 10, pos.y-20);
            break;

        case DOWN:
            SDL_RenderDrawLine(renderer, pos.x , pos.y+30, pos.x, pos.y-30);
            SDL_RenderDrawLine(renderer, pos.x +10, pos.y-20, pos.x + 10, pos.y+20);
            SDL_RenderDrawLine(renderer, pos.x +20, pos.y+10, pos.x +20,pos.y-10);
            break;

        case UP:
            SDL_RenderDrawLine(renderer, pos.x , pos.y+30, pos.x, pos.y-30);
            SDL_RenderDrawLine(renderer, pos.x -10, pos.y-20, pos.x - 10, pos.y+20);
            SDL_RenderDrawLine(renderer, pos.x -20, pos.y+10, pos.x -20,pos.y-10);
            break;
    }
}
void draw_capacitor(SDL_Point pos, Orientation o) {
    int length = 40;
    int plateHalf = 15;

    switch (o) {
        case RIGHT:
            SDL_RenderDrawLine(renderer, pos.x - length, pos.y, pos.x - 10, pos.y);
            SDL_RenderDrawLine(renderer, pos.x - 10, pos.y - plateHalf, pos.x - 10, pos.y + plateHalf);
            SDL_RenderDrawLine(renderer, pos.x + 10, pos.y - plateHalf, pos.x + 10, pos.y + plateHalf);
            SDL_RenderDrawLine(renderer, pos.x + 10, pos.y, pos.x + length, pos.y);
            break;
        case LEFT:
            SDL_RenderDrawLine(renderer, pos.x + length, pos.y, pos.x + 10, pos.y);
            SDL_RenderDrawLine(renderer, pos.x + 10, pos.y - plateHalf, pos.x + 10, pos.y + plateHalf);
            SDL_RenderDrawLine(renderer, pos.x - 10, pos.y - plateHalf, pos.x - 10, pos.y + plateHalf);
            SDL_RenderDrawLine(renderer, pos.x - 10, pos.y, pos.x - length, pos.y);
            break;
        case UP:
            SDL_RenderDrawLine(renderer, pos.x, pos.y + length, pos.x, pos.y + 10);
            SDL_RenderDrawLine(renderer, pos.x - plateHalf, pos.y + 10, pos.x + plateHalf, pos.y + 10);
            SDL_RenderDrawLine(renderer, pos.x - plateHalf, pos.y - 10, pos.x + plateHalf, pos.y - 10);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - 10, pos.x, pos.y - length);
            break;
        case DOWN:
            SDL_RenderDrawLine(renderer, pos.x, pos.y - length, pos.x, pos.y - 10);
            SDL_RenderDrawLine(renderer, pos.x - plateHalf, pos.y - 10, pos.x + plateHalf, pos.y - 10);
            SDL_RenderDrawLine(renderer, pos.x - plateHalf, pos.y + 10, pos.x + plateHalf, pos.y + 10);
            SDL_RenderDrawLine(renderer, pos.x, pos.y + 10, pos.x, pos.y + length);
            break;
    }
}

void SDL_RenderDrawCircle(SDL_Renderer* renderer, int x, int y, int r) {
    for (int w = 0; w < r * 2; w++) {
        for (int h = 0; h < r * 2; h++) {
            int dx = r - w;
            int dy = r - h;
            if ((dx*dx + dy*dy) <= (r * r)&&(dx*dx + dy*dy) >= (r * r)-40) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}
void draw_voltage_source(SDL_Point pos, Orientation o) {
    int r = 15;

    switch (o) {
        case RIGHT:
            SDL_RenderDrawLine(renderer, pos.x - 40, pos.y, pos.x - r, pos.y);
            SDL_RenderDrawLine(renderer, pos.x + r, pos.y, pos.x + 40, pos.y);
            SDL_RenderDrawCircle(renderer, pos.x, pos.y, r);
            SDL_RenderDrawLine(renderer, pos.x + 5, pos.y, pos.x + 9, pos.y);
            SDL_RenderDrawLine(renderer, pos.x + 7, pos.y - 2, pos.x+7, pos.y + 2);
            SDL_RenderDrawLine(renderer,pos.x - 7, pos.y - 2, pos.x-7, pos.y + 2);
            break;

        case DOWN:
            SDL_RenderDrawLine(renderer, pos.x, pos.y + 40, pos.x, pos.y + r);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - r, pos.x, pos.y - 40);
            SDL_RenderDrawCircle(renderer, pos.x, pos.y, r);
            SDL_RenderDrawLine(renderer, pos.x - 3, pos.y - 7, pos.x + 3, pos.y - 7);
            SDL_RenderDrawLine(renderer, pos.x, pos.y + 7, pos.x, pos.y +3);
            SDL_RenderDrawLine(renderer, pos.x - 2, pos.y +7, pos.x + 2, pos.y +7);
            break;

        case LEFT:
            SDL_RenderDrawLine(renderer, pos.x + 40, pos.y, pos.x + r, pos.y);
            SDL_RenderDrawLine(renderer, pos.x - r, pos.y, pos.x - 40, pos.y);
            SDL_RenderDrawCircle(renderer, pos.x, pos.y, r);
            SDL_RenderDrawLine(renderer, pos.x - 5, pos.y, pos.x - 9, pos.y);
            SDL_RenderDrawLine(renderer, pos.x - 7, pos.y - 2, pos.x-7, pos.y + 2);
            SDL_RenderDrawLine(renderer,pos.x + 7, pos.y - 2, pos.x+7, pos.y + 2);
            break;

        case UP:
            SDL_RenderDrawLine(renderer, pos.x, pos.y - 40, pos.x, pos.y - r);
            SDL_RenderDrawLine(renderer, pos.x, pos.y + r, pos.x, pos.y + 40);
            SDL_RenderDrawCircle(renderer, pos.x, pos.y, r);
            SDL_RenderDrawLine(renderer, pos.x - 3, pos.y + 7, pos.x + 3, pos.y + 7);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - 7, pos.x, pos.y -3);
            SDL_RenderDrawLine(renderer, pos.x - 2, pos.y -7, pos.x + 2, pos.y -7);
            break;
    }
}

void draw_current_source(SDL_Point pos, Orientation o) {
    int r = 15;

    switch (o) {
        case RIGHT:
            SDL_RenderDrawLine(renderer, pos.x - 40, pos.y, pos.x - r, pos.y);
            SDL_RenderDrawLine(renderer, pos.x + r, pos.y, pos.x + 40, pos.y);
            SDL_RenderDrawCircle(renderer, pos.x, pos.y, r);
            SDL_RenderDrawLine(renderer, pos.x - 6, pos.y, pos.x + 6, pos.y);
            SDL_RenderDrawLine(renderer, pos.x - 6, pos.y, pos.x - 2, pos.y + 4);
            SDL_RenderDrawLine(renderer, pos.x - 6, pos.y, pos.x - 2, pos.y - 4);
            break;

        case LEFT:
            SDL_RenderDrawLine(renderer, pos.x + 40, pos.y, pos.x + r, pos.y);
            SDL_RenderDrawLine(renderer, pos.x - r, pos.y, pos.x - 40, pos.y);
            SDL_RenderDrawCircle(renderer, pos.x, pos.y, r);
            SDL_RenderDrawLine(renderer, pos.x - 6, pos.y, pos.x + 6, pos.y);
            SDL_RenderDrawLine(renderer, pos.x + 6, pos.y, pos.x + 2, pos.y + 4);
            SDL_RenderDrawLine(renderer, pos.x + 6, pos.y, pos.x + 2, pos.y - 4);
            break;

        case DOWN:
            SDL_RenderDrawLine(renderer, pos.x, pos.y - 40, pos.x, pos.y - r);
            SDL_RenderDrawLine(renderer, pos.x, pos.y + r, pos.x, pos.y + 40);
            SDL_RenderDrawCircle(renderer, pos.x, pos.y, r);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - 6, pos.x, pos.y + 6);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - 6, pos.x - 4, pos.y - 2);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - 6, pos.x + 4, pos.y - 2);
            break;

        case UP:
            SDL_RenderDrawLine(renderer, pos.x, pos.y + 40, pos.x, pos.y + r);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - r, pos.x, pos.y - 40);
            SDL_RenderDrawCircle(renderer, pos.x, pos.y, r);
            SDL_RenderDrawLine(renderer, pos.x, pos.y - 6, pos.x, pos.y + 6);
            SDL_RenderDrawLine(renderer, pos.x, pos.y + 6, pos.x - 4, pos.y + 2);
            SDL_RenderDrawLine(renderer, pos.x, pos.y + 6, pos.x + 4, pos.y + 2);
            break;
    }
}
void draw_voltage_probe(SDL_Point pos) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(renderer, pos.x - 10, pos.y - 10, pos.x + 10, pos.y + 10);
    SDL_RenderDrawLine(renderer, pos.x - 10, pos.y + 10, pos.x + 10, pos.y - 10);
    SDL_RenderDrawCircle(renderer, pos.x, pos.y, 8);
}

void draw_current_probe(SDL_Point pos) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(renderer, pos.x, pos.y - 12, pos.x, pos.y + 12);
    SDL_RenderDrawLine(renderer, pos.x - 6, pos.y - 6, pos.x + 6, pos.y + 6);
    SDL_RenderDrawLine(renderer, pos.x - 6, pos.y + 6, pos.x + 6, pos.y - 6);
}
void draw_component(const C& c) {
    switch (c.type) {
        case RESISTOR: draw_resistor(c.position, c.orientation); break;
        case CAPACITOR: draw_capacitor(c.position, c.orientation); break;
        case INDUCTOR: draw_inductor(c.position, c.orientation); break;
        case VOLTAGE_SOURCE: draw_voltage_source(c.position, c.orientation); break;
        case CURRENT_SOURCE: draw_current_source(c.position, c.orientation); break;
        case Ground: draw_ground(c.position,c.orientation);break;
        case VOLTAGE_PROBE: draw_voltage_probe(c.position); break;
        case CURRENT_PROBE: draw_current_probe(c.position); break;
        default: break;
    }
}
SDL_Point firstPoint;
bool waitingForSecondClick = false;
Mode mode = NORMAL;
Orientation placing = LEFT ;
ComponentType placingType = NONE;
void handle_toolbar_click(int mx, int my) {
    for (auto &btn : toolbarButtons) {
        if (mx >= btn.rect.x && mx <= btn.rect.x + btn.rect.w &&
            my >= btn.rect.y && my <= btn.rect.y + btn.rect.h) {
            placingType = btn.type;
            mode = PREVIEW;
            placing = LEFT;
            break;
        }
    }
}
class NodeManager {
private:
    map<pair<int, int>, string> nodeMap;
    map<string, set<pair<int, int>>> connectedPoints;
    int nextNodeNumber = 1;

public:
    string getNodeName(pair<int, int> point) {
        if (nodeMap.find(point) != nodeMap.end()) {
            return nodeMap[point];
        }
        return "";
    }

    string getOrCreateNode(pair<int, int> point) {
        string nodeName = getNodeName(point);
        if (!nodeName.empty()) {
            return nodeName;
        }
        nodeName = "N" + to_string(nextNodeNumber++);
        nodeMap[point] = nodeName;
        connectedPoints[nodeName].insert(point);
        return nodeName;
    }

    void connectPoints(pair<int, int> p1, pair<int, int> p2) {
        string node1 = getOrCreateNode(p1);
        string node2 = getOrCreateNode(p2);

        if (node1 != node2) {
            for (auto& point : connectedPoints[node2]) {
                nodeMap[point] = node1;
                connectedPoints[node1].insert(point);
            }
            connectedPoints.erase(node2);
        }
    }

    void renameNodesSequentially() {
        map<string, string> oldToNewName;
        int counter = 1;

        for (auto& entry : connectedPoints) {
            string oldName = entry.first;
            string newNodeName = "N" + to_string(counter++);
            oldToNewName[oldName] = newNodeName;
        }

        for (auto& entry : nodeMap) {
            string oldName = entry.second;
            entry.second = oldToNewName[oldName];
        }

        map<string, set<pair<int, int>>> newConnectedPoints;
        for (auto& entry : connectedPoints) {
            string oldName = entry.first;
            newConnectedPoints[oldToNewName[oldName]] = entry.second;
        }
        connectedPoints = newConnectedPoints;

        nextNodeNumber = counter;
    }

    void checkWireIntersections(const vector<pair<SDL_Point, SDL_Point>>& wires) {
        for (size_t i = 0; i < wires.size(); ++i) {
            for (size_t j = i + 1; j < wires.size(); ++j) {
                pair<int, int> intersection = findIntersection(wires[i], wires[j]);
                if (intersection.first != -1 && intersection.second != -1) {
                    connectPoints({wires[i].first.x, wires[i].first.y}, intersection);
                    connectPoints({wires[j].first.x, wires[j].first.y}, intersection);
                }
            }
        }
    }

    pair<int, int> findIntersection(const pair<SDL_Point, SDL_Point>& wire1, const pair<SDL_Point, SDL_Point>& wire2) {
        if (wire1.first.y == wire1.second.y && wire2.first.y == wire2.second.y && wire1.first.y == wire2.first.y) {
            int minX1 = min(wire1.first.x, wire1.second.x);
            int maxX1 = max(wire1.first.x, wire1.second.x);
            int minX2 = min(wire2.first.x, wire2.second.x);
            int maxX2 = max(wire2.first.x, wire2.second.x);
            if (minX1 <= maxX2 && minX2 <= maxX1) {
                return {max(minX1, minX2), wire1.first.y};
            }
        }
        else if (wire1.first.x == wire1.second.x && wire2.first.x == wire2.second.x && wire1.first.x == wire2.first.x) {
            int minY1 = min(wire1.first.y, wire1.second.y);
            int maxY1 = max(wire1.first.y, wire1.second.y);
            int minY2 = min(wire2.first.y, wire2.second.y);
            int maxY2 = max(wire2.first.y, wire2.second.y);
            if (minY1 <= maxY2 && minY2 <= maxY1) {
                return {wire1.first.x, max(minY1, minY2)};
            }
        }
        else if ((wire1.first.y == wire1.second.y && wire2.first.x == wire2.second.x) ||
                 (wire1.first.x == wire1.second.x && wire2.first.y == wire2.second.y)) {
            SDL_Point hStart, hEnd, vStart, vEnd;
            if (wire1.first.y == wire1.second.y) {
                hStart = wire1.first;
                hEnd = wire1.second;
                vStart = wire2.first;
                vEnd = wire2.second;
            } else {
                hStart = wire2.first;
                hEnd = wire2.second;
                vStart = wire1.first;
                vEnd = wire1.second;
            }
            int hMinX = min(hStart.x, hEnd.x);
            int hMaxX = max(hStart.x, hEnd.x);
            int vMinY = min(vStart.y, vEnd.y);
            int vMaxY = max(vStart.y, vEnd.y);
            if (vStart.x >= hMinX && vStart.x <= hMaxX && hStart.y >= vMinY && hStart.y <= vMaxY) {
                return {vStart.x, hStart.y};
            }
        }
        return {-1, -1};
    }
    void printAllNodes() {
        cout << "All Nodes:" << endl;
        for (const auto& entry : connectedPoints) {
            cout << "Node " << entry.first << " connects to points: ";
            for (const auto& point : entry.second) {
                cout << "(" << point.first << "," << point.second << ") ";
            }
            cout << endl;
        }
    }
    map<string, vector<pair<int, int>>> getAllNodes() const {
        map<string, vector<pair<int, int>>> result;
        for (const auto& entry : connectedPoints) {
            result[entry.first] = vector<pair<int, int>>(entry.second.begin(), entry.second.end());
        }
        return result;
    }

    string findClosestNodeToPoint(pair<int, int> point, double& minDistance) const {
        string closestNode = "";
        minDistance = 10;
        for (const auto& nodeEntry : connectedPoints) {
            for (const auto& nodePoint : nodeEntry.second) {
                double dist = sqrt(pow(nodePoint.first - point.first, 2) +
                                   pow(nodePoint.second - point.second, 2));
                if (dist < minDistance) {
                    minDistance = dist;
                    closestNode = nodeEntry.first;
                }
            }
        }

        return closestNode;
    }

    string getNodeNameAtPoint(pair<int, int> point) const {
        auto it = nodeMap.find(point);
        if (it != nodeMap.end()) {
            return it->second;
        }
        return "";
    }
};
double distanceToLine(pair<int, int> point, pair<int, int> lineStart, pair<int, int> lineEnd) {
    int x0 = point.first, y0 = point.second;
    int x1 = lineStart.first, y1 = lineStart.second;
    int x2 = lineEnd.first, y2 = lineEnd.second;


    if (x1 == x2 && y1 == y2) {
        return sqrt(pow(x0 - x1, 2) + pow(y0 - y1, 2));
    }

    double numerator = abs((y2 - y1) * x0 - (x2 - x1) * y0 + x2 * y1 - y2 * x1);
    double denominator = sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2));

    return abs(numerator / denominator);
}

string findClosestNode(SDL_Point probePos, const vector<pair<SDL_Point, SDL_Point>>& wires, NodeManager& nodeManager) {
    double minDistance = 10;
    string closestNode = "";
    nodeManager.checkWireIntersections(wires);

    for (const auto& wire : wires) {
        pair<int, int> p1 = {wire.first.x, wire.first.y};
        pair<int, int> p2 = {wire.second.x, wire.second.y};
        nodeManager.connectPoints(p1, p2);
    }
    nodeManager.renameNodesSequentially();
    pair<int, int> probePoint = {probePos.x, probePos.y};

    for (const auto& wire : wires) {
        double dist = distanceToLine(probePoint, {wire.first.x, wire.first.y}, {wire.second.x, wire.second.y});

        if (dist < minDistance && dist <= 10.0) {
            minDistance = dist;

            string node1 = nodeManager.getNodeNameAtPoint({wire.first.x, wire.first.y});
            string node2 = nodeManager.getNodeNameAtPoint({wire.second.x, wire.second.y});

            if (!node1.empty()) closestNode = node1;
            else if (!node2.empty()) closestNode = node2;
        }
    }

    if (closestNode.empty()) {
        for (const auto& nodeEntry : nodeManager.getAllNodes()) {
            for (const auto& nodePoint : nodeEntry.second) {
                double dist = sqrt(pow(nodePoint.first - probePoint.first, 2) +
                                   pow(nodePoint.second - probePoint.second, 2));

                if (dist < minDistance && dist <= 30.0) {
                    minDistance = dist;
                    closestNode = nodeEntry.first;
                }
            }
        }
    }

    if (closestNode.empty()) {
        cout << "Warning: No node found near voltage probe position" << endl;
    }

    return closestNode;
}
string findClosestComponent(SDL_Point probePos, const vector<C>& components) {
    string closestComponent = "";
    double minDistance = numeric_limits<double>::max();
    const double MAX_PROBE_DISTANCE = 40.0;

    for (const auto& comp : components) {
        if (comp.type == RESISTOR || comp.type == CAPACITOR || comp.type == INDUCTOR ||
            comp.type == VOLTAGE_SOURCE || comp.type == CURRENT_SOURCE) {

            double dist = sqrt(pow(comp.position.x - probePos.x, 2) +
                               pow(comp.position.y - probePos.y, 2));

            double maxAllowedDistance = MAX_PROBE_DISTANCE;
            if (comp.type == RESISTOR || comp.type == INDUCTOR) {
                maxAllowedDistance = 50.0;
            }

            if (dist < minDistance && dist <= maxAllowedDistance) {
                minDistance = dist;
                closestComponent = comp.name;
            }
        }
    }

    if (minDistance > MAX_PROBE_DISTANCE) {
        cout << "Warning: Current probe is too far from any component (" << minDistance << " pixels)" << endl;
        return "";
    }

    return closestComponent;
}
string generateNetlist(const vector<C>& components, const vector<pair<SDL_Point, SDL_Point>>& wires) {
    NodeManager nodeManager;

    nodeManager.checkWireIntersections(wires);

    for (const auto& wire : wires) {
        pair<int, int> p1 = {wire.first.x, wire.first.y};
        pair<int, int> p2 = {wire.second.x, wire.second.y};
        nodeManager.connectPoints(p1, p2);
    }
    nodeManager.renameNodesSequentially();
    nodeManager.printAllNodes();

    ostringstream netlist;
    for (const auto& comp : components) {
        pair<int, int> pos;
        if (comp.type != Ground && ( comp.orientation == RIGHT)) {
            pos = {comp.position.x - 40, comp.position.y};
        } else if (comp.type != Ground &&( comp.orientation == DOWN) ) {
            pos = {comp.position.x, comp.position.y - 40};
        }
        else if (comp.type != Ground &&( comp.orientation == UP) ) {
            pos = {comp.position.x, comp.position.y + 40};
        }
        else if (comp.type != Ground &&( comp.orientation == LEFT) ) {
            pos = {comp.position.x+40, comp.position.y };
        }
        if (comp.type == Ground) {
            pos = {comp.position.x, comp.position.y};
        }
        string node1 = nodeManager.getNodeName(pos);

        pair<int, int> pos2;
        if(comp.type != INDUCTOR) {
            switch (comp.orientation) {
                case RIGHT:
                    pos2 = {pos.first + 80, pos.second};
                    break;
                case LEFT:
                    pos2 = {pos.first - 80, pos.second};
                    break;
                case UP:
                    pos2 = {pos.first, pos.second - 80};
                    break;
                case DOWN:
                    pos2 = {pos.first, pos.second + 80};
                    break;
            }
        }
        else{
            switch (comp.orientation) {
                case RIGHT:
                    pos2 = {pos.first + 60, pos.second};
                    break;
                case LEFT:
                    pos2 = {pos.first - 60, pos.second};
                    break;
                case UP:
                    pos2 = {pos.first, pos.second - 60};
                    break;
                case DOWN:
                    pos2 = {pos.first, pos.second + 60};
                    break;
            }
        }
        string node2 = nodeManager.getNodeName(pos2);

        if (comp.type == Ground) {
            netlist << "GND " << node1 << endl;
            continue;
        }

        switch (comp.type) {
            case RESISTOR:
                netlist << comp.name << " " << node1 << " " << node2 << " " << comp.value << endl;
                break;
            case CAPACITOR:
                netlist << comp.name << " " << node1 << " " << node2 << " " << comp.value << endl;
                break;
            case VOLTAGE_SOURCE:
                netlist << comp.name << " " << node1 << " " << node2 << " " << comp.value << endl;
                break;
            case INDUCTOR:
                netlist << comp.name << " " << node1 << " " << node2 << " " << comp.value << endl;
                break;
            case CURRENT_SOURCE:
                netlist << comp.name << " " << node1 << " " << node2 << " " << comp.value << endl;
                break;
            default:
                break;
        }
    }

    return netlist.str();
}
enum AnalysisMenuState { MENU_CLOSED, MENU_OPEN, MENU_DC, MENU_TRAN, MENU_SWEEP, MENU_PLACING_PROBES  };
AnalysisMenuState menuState = MENU_CLOSED;

void draw_analysis_menu() {
    if (menuState == MENU_OPEN) {
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_Rect menu = {200, 150, 250, 150};
        SDL_RenderFillRect(renderer, &menu);

        drawText("1) DC Analysis", 210, 160, {0, 0, 0, 255});
        drawText("2) Transient", 210, 190, {0, 0, 0, 255});
        drawText("3) DC Sweep", 210, 220, {0, 0, 0, 255});
    } else if (menuState == MENU_DC) {
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_Rect menu = {200, 150, 250, 100};
        SDL_RenderFillRect(renderer, &menu);
        drawText("DC Analysis Selected", 210, 160, {0, 0, 0, 255});
        drawText("Press ENTER to run", 210, 190, {0, 0, 0, 255});
    } else if (menuState == MENU_TRAN) {
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_Rect menu = {200, 150, 250, 150};
        SDL_RenderFillRect(renderer, &menu);
        drawText("Transient Analysis", 210, 160, {0, 0, 0, 255});
        drawText("Enter dt:", 210, 190, {0, 0, 0, 255});
        drawText("Enter tEnd:", 210, 220, {0, 0, 0, 255});
    } else if (menuState == MENU_SWEEP) {
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_Rect menu = {200, 150, 250, 200};
        SDL_RenderFillRect(renderer, &menu);
        drawText("DC Sweep Analysis", 210, 160, {0, 0, 0, 255});
        drawText("Source:", 210, 190, {0, 0, 0, 255});
        drawText("Start:", 210, 220, {0, 0, 0, 255});
        drawText("End:", 210, 250, {0, 0, 0, 255});
        drawText("Step:", 210, 280, {0, 0, 0, 255});
    }
    else if (menuState == MENU_PLACING_PROBES) {
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_Rect menu = {200, 150, 350, 200};
        SDL_RenderFillRect(renderer, &menu);

        drawText("Placing Probes Mode", 210, 160, {0, 0, 0, 255});
        drawText("Left-click: Place voltage probe on wires", 210, 190, {0, 0, 0, 255});
        drawText("Right-click: Place current probe on components", 210, 220, {0, 0, 0, 255});
        drawText("ENTER: Finish and run analysis", 210, 250, {0, 0, 0, 255});
        drawText("ESC: Cancel analysis", 210, 280, {0, 0, 0, 255});

        drawText("Voltage probes: " + to_string(voltageProbes.size()), 210, 310, {0, 0, 0, 255});
        drawText("Current probes: " + to_string(currentProbes.size()), 210, 340, {0, 0, 0, 255});
    }
}
void saveNetlistToFile(const string& filename, const string& netlist) {
    ofstream file(filename);
    if (file.is_open()) {
        file << netlist;
        file.close();
        cout << "Netlist saved to " << filename << endl;
    } else {
        cerr << "Error saving netlist!" << endl;
    }
}
bool enteringTransientParams = false;
string transientParamInput;
bool isEnteringDt = true;
double dtValue = 0.0;
double tEndValue = 0.0;
Uint32 cursorBlinkTime = 0;
bool showCursor = false;
SDL_Rect transientInputBox = {300, 200, 400, 40};

void drawTransientInput() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &transientInputBox);

    string prompt = isEnteringDt ? "Enter dt value: " : "Enter tEnd value: ";
    string displayText = prompt + transientParamInput;

    if (showCursor) {
        displayText += "_";
    }

    drawText(displayText, transientInputBox.x + 10, transientInputBox.y + 10, {255, 255, 255, 255});
}
class MultiGraph {
private:
    SDL_Window* graphWindow;
    SDL_Renderer* graphRenderer;

    vector<TransientResult> voltageResults;
    vector<CurrentResult> currentResults;

    unordered_map<string, bool> visibleVoltageNodes;
    unordered_map<string, bool> visibleCurrentComponents;

    map<string, SDL_Color> colorMap;
    SDL_Color colors[12] = {
            {255, 0, 0, 255},      {0, 0, 255, 255},
            {0, 255, 0, 255},      {255, 255, 0, 255},
            {255, 0, 255, 255},    {0, 255, 255, 255},
            {255, 128, 0, 255},    {128, 0, 255, 255},
            {0, 255, 128, 255},    {255, 0, 128, 255},
            {128, 255, 0, 255},    {0, 128, 255, 255}
    };

    double minTime, maxTime;
    double minVoltage, maxVoltage;
    double minCurrent, maxCurrent;

    bool showVoltageAxis = true;
    bool showCurrentAxis = true;

    int colorIndex = 0;

    enum ButtonType {
        BTN_VOLTAGE_AXIS,
        BTN_CURRENT_AXIS,
        BTN_CLEAR_ALL,
        BTN_EXIT,
        BTN_ADD_SIGNALS,
        BUTTON_COLOR,
        BUTTON_CLOSE_COLOR_MENU
    };
    enum GraphMode {
        MODE_TIME_DOMAIN,
        MODE_ADD_SIGNALS
    };
    struct AddedSignal {
        string name;
        vector<double> values;
        vector<double> times;
        SDL_Color color;
    };
    struct Button {
        SDL_Rect rect;
        string text;
        ButtonType type;
    };
    GraphMode graphMode = MODE_TIME_DOMAIN;
    vector<AddedSignal> addedSignals;
    vector<string> selectedSignalsForAddition;
    unordered_map<string, SDL_Color> userSelectedColors;
    string rightClickedItem;
    SDL_Point rightClickPosition;
    bool showColorMenu = false;
    vector<Button> colorMenuButtons;
    vector<Button> buttons;
    SDL_Point mousePosition;
    bool mouseInGraphArea;
    string hoverInfo;
    double hoverTime;
    double hoverValue;
    string hoverSignalName;
    vector<SDL_Color> presetColors = {
            {255, 0, 0, 255},
            {0, 0, 255, 255},
            {0, 255, 0, 255},
            {255, 255, 0, 255},
            {255, 0, 255, 255},
            {0, 255, 255, 255},
            {255, 128, 0, 255},
            {128, 0, 255, 255},
            {0, 255, 128, 255},
            {255, 0, 128, 255},
            {128, 255, 0, 255},
            {0, 128, 255, 255},
            {255, 255, 255, 255},
            {0, 0, 0, 255}
    };

public:
    MultiGraph(const vector<TransientResult>& vResults, const vector<CurrentResult>& cResults)
            : voltageResults(vResults), currentResults(cResults) {

        graphWindow = SDL_CreateWindow("SPICE Analysis Results",
                                       100, 100, 1400, 900,
                                       SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        graphRenderer = SDL_CreateRenderer(graphWindow, -1, SDL_RENDERER_ACCELERATED);

        int yPos = 400;
        int xPos = 1050;

        buttons.push_back({{xPos, yPos, 120, 30}, "Voltage Axis", BTN_VOLTAGE_AXIS});
        yPos += 40;
        buttons.push_back({{xPos, yPos, 120, 30}, "Current Axis", BTN_CURRENT_AXIS});
        yPos += 40;
        buttons.push_back({{xPos, yPos, 120, 30}, "Clear All", BTN_CLEAR_ALL});
        yPos += 40;
        buttons.push_back({{xPos, yPos, 120, 30}, "Exit", BTN_EXIT});

        calculateAutoZoom();
        yPos += 40;

        for (const auto& res : voltageResults) {
            visibleVoltageNodes[res.node] = false;
        }
        for (const auto& res : currentResults) {
            visibleCurrentComponents[res.component] = false;
        }
    }

    ~MultiGraph() {
        voltageResults.clear();
        visibleVoltageNodes.clear();
        visibleCurrentComponents.clear();
        SDL_DestroyRenderer(graphRenderer);
        SDL_DestroyWindow(graphWindow);
    }
    SDL_Color getColor(const string& name) {
        if (userSelectedColors.find(name) != userSelectedColors.end()) {
            return userSelectedColors[name];
        }

        if (colorMap.find(name) == colorMap.end()) {
            colorMap[name] = colors[colorIndex % 12];
            colorIndex++;
        }
        return colorMap[name];
    }

    void handleRightClick(int mx, int my) {

        int yPos = 120;
        for (const auto& nodePair : visibleVoltageNodes) {
            SDL_Rect itemRect = {1050, yPos, 200, 20};
            if (mx >= itemRect.x && mx <= itemRect.x + itemRect.w &&
                my >= itemRect.y && my <= itemRect.y + itemRect.h) {
                rightClickedItem = "V_" + nodePair.first;
                rightClickPosition = {mx, my};
                showColorMenu = true;
                createColorMenu();
                return;
            }
            yPos += 20;
        }

        yPos = 160 + visibleVoltageNodes.size() * 20;
        for (const auto& compPair : visibleCurrentComponents) {
            SDL_Rect itemRect = {1050, yPos, 200, 20};
            if (mx >= itemRect.x && mx <= itemRect.x + itemRect.w &&
                my >= itemRect.y && my <= itemRect.y + itemRect.h) {
                rightClickedItem = "I_" + compPair.first;
                rightClickPosition = {mx, my};
                showColorMenu = true;
                createColorMenu();
                return;
            }
            yPos += 20;
        }

        showColorMenu = false;
    }

    void createColorMenu() {
        colorMenuButtons.clear();
        int buttonSize = 20;
        int spacing = 5;
        int startX = rightClickPosition.x;
        int startY = rightClickPosition.y;

        for (size_t i = 0; i < presetColors.size(); i++) {
            int row = i / 7;
            int col = i % 7;
            SDL_Rect rect = {
                    startX + col * (buttonSize + spacing),
                    startY + row * (buttonSize + spacing),
                    buttonSize,
                    buttonSize
            };
            colorMenuButtons.push_back({rect, "", BUTTON_COLOR});
        }

        SDL_Rect closeRect = {
                startX + 7 * (buttonSize + spacing),
                startY,
                buttonSize,
                buttonSize
        };
        colorMenuButtons.push_back({closeRect, "X", BUTTON_CLOSE_COLOR_MENU});
    }

    void handleColorMenuClick(int mx, int my) {
        for (size_t i = 0; i < colorMenuButtons.size(); i++) {
            if (mx >= colorMenuButtons[i].rect.x && mx <= colorMenuButtons[i].rect.x + colorMenuButtons[i].rect.w &&
                my >= colorMenuButtons[i].rect.y && my <= colorMenuButtons[i].rect.y + colorMenuButtons[i].rect.h) {

                if (colorMenuButtons[i].type == BUTTON_CLOSE_COLOR_MENU) {
                    showColorMenu = false;
                    return;
                }

                if (i < presetColors.size()) {
                    userSelectedColors[rightClickedItem] = presetColors[i];
                    showColorMenu = false;
                }
                break;
            }
        }
    }

    void drawColorMenu() {
        if (!showColorMenu) return;


        SDL_SetRenderDrawColor(graphRenderer, 240, 240, 240, 255);
        SDL_Rect menuBackground = {
                rightClickPosition.x - 5,
                rightClickPosition.y - 5,
                7 * 25 + 30,
                ((14 + 6) / 7) * 25 + 10
        };
        SDL_RenderFillRect(graphRenderer, &menuBackground);
        SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(graphRenderer, &menuBackground);
        for (size_t i = 0; i < presetColors.size(); i++) {
            SDL_SetRenderDrawColor(graphRenderer,
                                   presetColors[i].r,
                                   presetColors[i].g,
                                   presetColors[i].b,
                                   presetColors[i].a);
            SDL_RenderFillRect(graphRenderer, &colorMenuButtons[i].rect);
            SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(graphRenderer, &colorMenuButtons[i].rect);
        }

        SDL_SetRenderDrawColor(graphRenderer, 255, 100, 100, 255);
        SDL_RenderFillRect(graphRenderer, &colorMenuButtons.back().rect);
        SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(graphRenderer, &colorMenuButtons.back().rect);
        drawText("X", colorMenuButtons.back().rect.x + 7,
                 colorMenuButtons.back().rect.y + 2, {0, 0, 0, 255});

        string itemName = rightClickedItem.substr(2);
        drawText("Color for " + itemName, rightClickPosition.x,
                 rightClickPosition.y - 20, {0, 0, 0, 255});
    }
    void calculateAutoZoom() {
        if (voltageResults.empty() && currentResults.empty()) return;

        minTime = maxTime = 0;
        if (!voltageResults.empty()) {
            minTime = maxTime = voltageResults[0].time;
            for (const auto& res : voltageResults) {
                if (res.time < minTime) minTime = res.time;
                if (res.time > maxTime) maxTime = res.time;
            }
        }
        if (!currentResults.empty()) {
            minTime = maxTime = currentResults[0].time;
            for (const auto& res : currentResults) {
                if (res.time < minTime) minTime = res.time;
                if (res.time > maxTime) maxTime = res.time;
            }
        }

        minVoltage = maxVoltage = 0;
        if (!voltageResults.empty()) {
            minVoltage = maxVoltage = voltageResults[0].voltage;
            for (const auto& res : voltageResults) {
                if (res.voltage < minVoltage) minVoltage = res.voltage;
                if (res.voltage > maxVoltage) maxVoltage = res.voltage;
            }
        }

        minCurrent = maxCurrent = 0;
        if (!currentResults.empty()) {
            minCurrent = maxCurrent = currentResults[0].current;
            for (const auto& res : currentResults) {
                if (res.current < minCurrent) minCurrent = res.current;
                if (res.current > maxCurrent) maxCurrent = res.current;
            }
        }

        double voltageRange = maxVoltage - minVoltage;
        double currentRange = maxCurrent - minCurrent;

        if (voltageRange < 1e-9) {
            minVoltage -= 1.0;
            maxVoltage += 1.0;
        } else {
            minVoltage -= voltageRange * 0.1;
            maxVoltage += voltageRange * 0.1;
        }

        if (currentRange < 1e-9) {
            minCurrent -= 1.0;
            maxCurrent += 1.0;
        } else {
            minCurrent -= currentRange * 0.1;
            maxCurrent += currentRange * 0.1;
        }
    }

    void drawText(const string& text, int x, int y, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(graphRenderer, surface);
            SDL_Rect dst = {x, y, surface->w, surface->h};
            SDL_RenderCopy(graphRenderer, texture, NULL, &dst);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }
    }

    void drawAxes() {
        SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);

        SDL_RenderDrawLine(graphRenderer, 100, 700, 1000, 700);

        if (showVoltageAxis) {
            SDL_RenderDrawLine(graphRenderer, 100, 100, 100, 700);
            drawText("Voltage (V)", 30, 400, {0, 0, 0, 255});
        }

        if (showCurrentAxis) {
            SDL_RenderDrawLine(graphRenderer, 1000, 100, 1000, 700);
            drawText("Current (A)", 1020, 400, {0, 0, 0, 255});
        }

        drawText("Time (s)", 500, 720, {0, 0, 0, 255});
        for (int i = 0; i <= 10; i++) {
            double time = minTime + i * (maxTime - minTime) / 10;
            int x = 100 + i * 90;
            SDL_RenderDrawLine(graphRenderer, x, 695, x, 705);
            drawText(to_string(time).substr(0, 6), x - 15, 710, {0, 0, 0, 255});
        }

        if (showVoltageAxis) {
            for (int i = 0; i <= 10; i++) {
                double voltage = minVoltage + i * (maxVoltage - minVoltage) / 10;
                int y = 700 - i * 60;
                SDL_RenderDrawLine(graphRenderer, 95, y, 105, y);
                drawText(to_string(voltage).substr(0, 6), 50, y - 10, {0, 0, 0, 255});
            }
        }

        if (showCurrentAxis) {
            for (int i = 0; i <= 10; i++) {
                double current = minCurrent + i * (maxCurrent - minCurrent) / 10;
                int y = 700 - i * 60;
                SDL_RenderDrawLine(graphRenderer, 995, y, 1005, y);
                drawText(to_string(current).substr(0, 6), 1010, y - 10, {0, 0, 0, 255});
            }
        }
    }

    void drawVoltageGraph() {
        if (voltageResults.empty()) return;

        for (const auto& nodePair : visibleVoltageNodes) {
            if (!nodePair.second) continue;

            const string& nodeName = nodePair.first;
            SDL_Color color = getColor("V_" + nodeName);
            SDL_SetRenderDrawColor(graphRenderer, color.r, color.g, color.b, color.a);

            bool firstPoint = true;
            int prevX = 0, prevY = 0;

            for (const auto& res : voltageResults) {
                if (res.node == nodeName) {
                    int x = 100 + (int)((res.time - minTime) / (maxTime - minTime) * 900);
                    int y = 700 - (int)((res.voltage - minVoltage) / (maxVoltage - minVoltage) * 600);

                    if (!firstPoint) {
                        SDL_RenderDrawLine(graphRenderer, prevX, prevY, x, y);
                    }

                    prevX = x;
                    prevY = y;
                    firstPoint = false;
                }
            }
        }
    }

    void drawCurrentGraph() {
        if (currentResults.empty()) return;

        for (const auto& compPair : visibleCurrentComponents) {
            if (!compPair.second) continue;

            const string& compName = compPair.first;
            SDL_Color color = getColor("I_" + compName);
            SDL_SetRenderDrawColor(graphRenderer, color.r, color.g, color.b, color.a);

            bool firstPoint = true;
            int prevX = 0, prevY = 0;

            for (const auto& res : currentResults) {
                if (res.component == compName) {
                    int x = 100 + (int)((res.time - minTime) / (maxTime - minTime) * 900);
                    int y = 700 - (int)((res.current - minCurrent) / (maxCurrent - minCurrent) * 600);

                    if (!firstPoint) {
                        SDL_RenderDrawLine(graphRenderer, prevX, prevY, x, y);
                    }

                    prevX = x;
                    prevY = y;
                    firstPoint = false;
                }
            }
        }
    }
    void findClosestPointToMouse(int mouseX, int mouseY) {
        if (!mouseInGraphArea) {
            hoverInfo = "";
            return;
        }

        double time = minTime + (mouseX - 100) * (maxTime - minTime) / 900.0;

        double minDistance = numeric_limits<double>::max();
        hoverSignalName = "";
        hoverTime = 0;
        hoverValue = 0;

        for (const auto& nodePair : visibleVoltageNodes) {
            if (!nodePair.second) continue;

            const string& nodeName = nodePair.first;
            for (const auto& res : voltageResults) {
                if (res.node == nodeName) {
                    double distance = abs(res.time - time);
                    if (distance < minDistance) {
                        minDistance = distance;
                        hoverTime = res.time;
                        hoverValue = res.voltage;
                        hoverSignalName = "V(" + nodeName + ")";
                    }
                }
            }
        }

        for (const auto& compPair : visibleCurrentComponents) {
            if (!compPair.second) continue;

            const string& compName = compPair.first;
            for (const auto& res : currentResults) {
                if (res.component == compName) {
                    double distance = abs(res.time - time);
                    if (distance < minDistance) {
                        minDistance = distance;
                        hoverTime = res.time;
                        hoverValue = res.current;
                        hoverSignalName = "I(" + compName + ")";
                    }
                }
            }
        }

        if (!hoverSignalName.empty() && minDistance < (maxTime - minTime) / 50.0) {
            stringstream ss;
            ss << fixed << setprecision(4);
            ss << hoverSignalName << " at " << hoverTime << "s = " << hoverValue;
            hoverInfo = ss.str();
        } else {
            hoverInfo = "";
        }
    }

    void drawMouseInfo() {
        if (hoverInfo.empty() || !mouseInGraphArea) return;

        int x = 100 + (int)((hoverTime - minTime) / (maxTime - minTime) * 900);
        SDL_SetRenderDrawColor(graphRenderer, 150, 150, 150, 100);
        SDL_RenderDrawLine(graphRenderer, x, 100, x, 700);

        SDL_Rect infoBox = {mousePosition.x + 10, mousePosition.y - 30, 250, 25};
        SDL_SetRenderDrawColor(graphRenderer, 255, 255, 220, 255);
        SDL_RenderFillRect(graphRenderer, &infoBox);
        SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(graphRenderer, &infoBox);

        drawText(hoverInfo, infoBox.x + 5, infoBox.y + 5, {0, 0, 0, 255});
    }


    void drawLegend() {
        int yPos = 120;
        int xPos = 1050;

        for (const auto& nodePair : visibleVoltageNodes) {
            if (nodePair.second) {
                SDL_Color color = getColor("V_" + nodePair.first);
                SDL_Rect colorRect = {xPos+150, yPos, 20, 20};
                SDL_SetRenderDrawColor(graphRenderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(graphRenderer, &colorRect);

                drawText("V(" + nodePair.first + ")", xPos + 180, yPos, {0, 0, 0, 255});
                yPos += 30;
            }
        }

        yPos += 20;

        for (const auto& compPair : visibleCurrentComponents) {
            if (compPair.second) {
                SDL_Color color = getColor("I_" + compPair.first);
                SDL_Rect colorRect = {xPos+150, yPos, 20, 20};
                SDL_SetRenderDrawColor(graphRenderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(graphRenderer, &colorRect);

                drawText("I(" + compPair.first + ")", xPos + 180, yPos, {0, 0, 0, 255});
                yPos += 30;
            }
        }
    }

    void drawControlPanel() {
        for (const auto& btn : buttons) {
            SDL_SetRenderDrawColor(graphRenderer, 200, 200, 200, 255);
            SDL_RenderFillRect(graphRenderer, &btn.rect);
            SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(graphRenderer, &btn.rect);

            drawText(btn.text, btn.rect.x + 10, btn.rect.y + 5, {0, 0, 0, 255});
        }
        if(graphMode==MODE_TIME_DOMAIN) {
            int yPos = 120;
            drawText("Select Nodes:", 1050, 100, {0, 0, 0, 255});

            for (const auto &node: visibleVoltageNodes) {
                SDL_Rect chkRect = {1050, yPos, 15, 15};
                SDL_SetRenderDrawColor(graphRenderer, 255, 255, 255, 255);
                SDL_RenderFillRect(graphRenderer, &chkRect);
                SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(graphRenderer, &chkRect);

                if (node.second) {
                    SDL_RenderDrawLine(graphRenderer, 1050, yPos, 1065, yPos + 15);
                    SDL_RenderDrawLine(graphRenderer, 1065, yPos + 15, 1080, yPos);
                }

                drawText(node.first, 1070, yPos, {0, 0, 0, 255});
                yPos += 20;
            }

            yPos += 20;
            drawText("Select Components:", 1050, yPos, {0, 0, 0, 255});
            yPos += 20;

            for (const auto &comp: visibleCurrentComponents) {
                SDL_Rect chkRect = {1050, yPos, 15, 15};
                SDL_SetRenderDrawColor(graphRenderer, 255, 255, 255, 255);
                SDL_RenderFillRect(graphRenderer, &chkRect);
                SDL_SetRenderDrawColor(graphRenderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(graphRenderer, &chkRect);

                if (comp.second) {
                    SDL_RenderDrawLine(graphRenderer, 1050, yPos, 1065, yPos + 15);
                    SDL_RenderDrawLine(graphRenderer, 1065, yPos + 15, 1080, yPos);
                }

                drawText(comp.first, 1070, yPos, {0, 0, 0, 255});
                yPos += 20;
            }
        }
    }

    void handleButtonClick(ButtonType btnType) {
        switch (btnType) {
            case BTN_VOLTAGE_AXIS:
                showVoltageAxis = !showVoltageAxis;
                break;
            case BTN_CURRENT_AXIS:
                showCurrentAxis = !showCurrentAxis;
                break;
            case BTN_CLEAR_ALL:
                for (auto& pair : visibleVoltageNodes) pair.second = false;
                for (auto& pair : visibleCurrentComponents) pair.second = false;
                break;
            case BTN_EXIT:
                break;

        }
    }

    void handleEvents(bool& running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                return;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                if (e.button.button == SDL_BUTTON_RIGHT) {
                    handleRightClick(mx, my);
                } else if (e.button.button == SDL_BUTTON_LEFT) {
                    if (showColorMenu) {
                        handleColorMenuClick(mx, my);
                    }
                }
            }
            if (e.type == SDL_MOUSEMOTION) {
                mousePosition.x = e.motion.x;
                mousePosition.y = e.motion.y;

                mouseInGraphArea = (mousePosition.x >= 100 && mousePosition.x <= 1000 &&
                                    mousePosition.y >= 100 && mousePosition.y <= 700);

                if (mouseInGraphArea) {
                    findClosestPointToMouse(mousePosition.x, mousePosition.y);
                } else {
                    hoverInfo = "";
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                int yPos = 120;
                for (auto &nodePair: visibleVoltageNodes) {
                    SDL_Rect chkRect = {1050, yPos, 15, 15};
                    if (mx >= chkRect.x && mx <= chkRect.x + chkRect.w &&
                        my >= chkRect.y && my <= chkRect.y + chkRect.h) {
                        nodePair.second = !nodePair.second;
                        break;
                    }
                    yPos += 20;
                }

                yPos = 160 + visibleVoltageNodes.size() * 20;
                for (auto &compPair: visibleCurrentComponents) {
                    SDL_Rect chkRect = {1050, yPos, 15, 15};
                    if (mx >= chkRect.x && mx <= chkRect.x + chkRect.w &&
                        my >= chkRect.y && my <= chkRect.y + chkRect.h) {
                        compPair.second = !compPair.second;
                        break;
                    }
                    yPos += 20;
                }

                for (const auto& btn : buttons) {
                    if (mx >= btn.rect.x && mx <= btn.rect.x + btn.rect.w &&
                        my >= btn.rect.y && my <= btn.rect.y + btn.rect.h) {

                        if (btn.type == BTN_EXIT) {
                            running = false;
                        } else {
                            handleButtonClick(btn.type);
                        }
                        break;
                    }
                }
            }
        }
    }

    void run() {
        bool running = true;

        while (running) {
            SDL_SetRenderDrawColor(graphRenderer, 255, 255, 255, 255);
            SDL_RenderClear(graphRenderer);


            drawAxes();
            drawColorMenu();
            drawVoltageGraph();
            drawCurrentGraph();
            drawControlPanel();
            drawMouseInfo();
            drawLegend();

            SDL_RenderPresent(graphRenderer);
            handleEvents(running);

            SDL_Delay(10);
        }
    }
};
string placeVoltageProbe(int x, int y) {
    NodeManager nodeManager;
    nodeManager.checkWireIntersections(wires);
    nodeManager.renameNodesSequentially();

    string targetNode = findClosestNode({x, y}, wires, nodeManager);

    if (!targetNode.empty()) {
        Probe newProbe;
        newProbe.type = VOLTAGE_PROBE;
        newProbe.position = {x, y};
        newProbe.target = targetNode;
        newProbe.name = "VP" + to_string(voltageProbes.size() + 1);
        newProbe.isActive = true;
        voltageProbes.push_back(newProbe);


        C visualProbe = {VOLTAGE_PROBE, {x, y}};
        components.push_back(visualProbe);

        cout << "Voltage probe placed on node: " << targetNode << endl;
        return targetNode;
    } else {
        cout << "Cannot place voltage probe here - no wire detected" << endl;
        return "";
    }
}

void placeCurrentProbe(int x, int y) {
    string targetComponent = findClosestComponent({x, y}, components);

    if (!targetComponent.empty()) {
        Probe newProbe;
        newProbe.type = CURRENT_PROBE;
        newProbe.position = {x, y};
        newProbe.target = targetComponent;
        newProbe.name = "CP" + to_string(currentProbes.size() + 1);
        newProbe.isActive = true;
        currentProbes.push_back(newProbe);

        C visualProbe = {CURRENT_PROBE, {x, y}};
        components.push_back(visualProbe);

        cout << "Current probe placed on component: " << targetComponent << endl;
    } else {
        cout << "Cannot place current probe here - no component detected" << endl;
    }
}
void runAnalysisWithProbes() {
    transientResults.clear();
    currentResults.clear();
    cout << "Running analysis with " << voltageProbes.size()
         << " voltage probes and " << currentProbes.size()
         << " current probes" << endl;

    string netlist = generateNetlist(components, wires);
    cout << "Generated Netlist:\n" << netlist << endl;

    string filename = "circuit_netlist.txt";
    saveNetlistToFile(filename, netlist);

    Circuit circuit;
    ifstream netlistFile(filename);
    string line;
    while (getline(netlistFile, line)) {
        Run run(line, &circuit);
        run.parse_netlist_line(line, &circuit);
    }

    MNA mna(&circuit);

    map<string, int> probeConsideration;
    for (const auto& probe : voltageProbes) {
        if (probe.isActive) {
            probeConsideration[probe.target] = 0;
        }
    }
    for (const auto& probe : currentProbes) {
        if (probe.isActive) {
            probeConsideration[probe.target] = 0;
        }
    }


    mna.analyzeTransient(dtValue, tEndValue, probeConsideration);

    menuState = MENU_CLOSED;
    mode = NORMAL;
    components.erase(
            remove_if(components.begin(), components.end(),
                      [](const C& comp) {
                          return comp.type == VOLTAGE_PROBE || comp.type == CURRENT_PROBE;
                      }),
            components.end()
    );
    if (!transientResults.empty() || !currentResults.empty()) {
        MultiGraph multiGraph(transientResults, currentResults);
        multiGraph.run();
    }

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                             "Analysis Complete",
                             "Circuit analysis completed successfully!",
                             window);
}
void redraw_all(const C* preview = nullptr) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    draw_grid();
    draw_toolbar();
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (const auto& w : wires)
        SDL_RenderDrawLine(renderer, w.first.x, w.first.y, w.second.x, w.second.y);
    if (waitingForSecondClick) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        SDL_Point current = { roundToGrid(mx, GRID_SIZE), roundToGrid(my, GRID_SIZE) };
        if (abs(current.x - firstPoint.x) > abs(current.y - firstPoint.y))
            current.y = firstPoint.y;
        else
            current.x = firstPoint.x;

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
        SDL_RenderDrawLine(renderer, firstPoint.x, firstPoint.y, current.x, current.y);
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (const auto& c : components)
        draw_component(c);
    if (preview) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
        draw_component(*preview);
    }
    draw_analysis_menu();
    if (enteringTransientParams) {
        drawTransientInput();
    }
    if (menuState == MENU_PLACING_PROBES) {

        SDL_Color textColor = {0, 0, 0, 255};
        drawText("PLACING PROBES MODE - Left-click: Voltage probe, Right-click: Current probe",
                 10, 10, textColor);
        drawText("Press ENTER to run analysis, ESC to cancel",
                 10, 30, textColor);
    }
    SDL_RenderPresent(renderer);
}
int main(int argc, char* argv[]) {


    //-----------------------------------------------------------------------------------------------------------------

    string libpath="C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/lib";
    if(CreateDirectory(libpath.c_str(),NULL)){
        cout << "Folder created successfully: " << libpath << endl;
    }

    ifstream file("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/lib/filenames ke be aghle jen ham nmirese.txt",ios::in);//making sure the name file exists
    if(!file.is_open()) {
        cout<<"File created successfuly"<<endl;
        ofstream file("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/lib/filenames ke be aghle jen ham nmirese.txt",ios::out);//making sure the name file exists)
    }
    vector<string> allfilenames;
    ifstream filein("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/lib/filenames ke be aghle jen ham nmirese.txt",ios::in);
    while(!filein.eof()) {
        string name;
        getline(filein,name);
        string check="C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/lib/"+name+".txt";
        fstream filecheck(check,ios::in);
        if(filecheck.is_open()) {
            allfilenames.push_back(name);
        }
        filecheck.close();
    }
    filein.close();
    ofstream fileout("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/lib/filenames ke be aghle jen ham nmirese.txt",ios::out);
    for(string i:allfilenames) {
        fileout<<i<<endl;
    }
    fileout.close();
    string saving="element"+to_string(allfilenames.size());
    allfilenames.push_back(saving);




    //-----------------------------------------------------------------------------------------------------------------

    toolbarButtons.push_back({ {10, 82, 20, 20}, "R", RESISTOR });
    toolbarButtons.push_back({ {40, 82, 20, 20}, "C", CAPACITOR });
    toolbarButtons.push_back({ {70, 82, 20, 20}, "L", INDUCTOR });
    toolbarButtons.push_back({ {100, 82, 20, 20}, "V", VOLTAGE_SOURCE });
    toolbarButtons.push_back({ {130, 82, 20, 20}, "I", CURRENT_SOURCE });
    toolbarButtons.push_back({ {200, 82, 40, 20}, "RUN", NONE });
    toolbarButtons.push_back({{1000,82,40,20},"lib",NONE});

    bool c = false ;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    TTF_Init();
    font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 18);
    if (!font) {
        cerr << "Font load error\n";
        return 1;
    }
    window = SDL_CreateWindow("SPICE Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return -1;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool run = true;
    SDL_Event e;

    while (run) {
        C preview = { placingType, {-1, -1} };

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) run = false;

            else if (e.type == SDL_KEYDOWN) {
                const Uint8* keyState = SDL_GetKeyboardState(NULL);
                bool ctrl = keyState[SDL_SCANCODE_LCTRL] || keyState[SDL_SCANCODE_RCTRL];

                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        run = false;
                        break;
                    case SDLK_r:
                        if (ctrl && mode == PREVIEW && placingType != NONE) {
                            placing = static_cast<Orientation>((placing + 1) % 4);
                        } else {
                            placingType = RESISTOR;
                            mode = PREVIEW;
                            placing = RIGHT;
                        }
                        break;
                    case SDLK_c:
                        placingType = CAPACITOR;
                        mode = PREVIEW;
                        placing = RIGHT;
                        break;
                    case SDLK_l:
                        placingType = INDUCTOR;
                        mode = PREVIEW;
                        placing = RIGHT;
                        break;
                    case SDLK_v:
                        placingType = VOLTAGE_SOURCE;
                        mode = PREVIEW;
                        placing = RIGHT;
                        break;
                    case SDLK_i:
                        placingType = CURRENT_SOURCE;
                        mode = PREVIEW;
                        placing = RIGHT;
                        break;
                    case SDLK_g:
                        placingType = Ground;
                        mode = PREVIEW;
                        placing = RIGHT;
                        break;
                    case SDLK_s: {
                        vector<string> forward;
                        int choice;
                        cout << "Run as (1) Server or (2) Client? ";
                        cin >> choice;
                        if (choice == 1) {
                            TCPServer server(8080);
                            server.start(forward);
                        } else {
                            string ip;
                            cout << "Enter server IP (use 127.0.0.1 for local test): ";
                            cin >> ip;
                            TCPClient client(ip, 8080);
                            string a;
                            cin.ignore();
                            bool once = false;
                            do {
                                if (!once) {
                                    once = true;
                                    for (const auto &c: components) {
                                        if (c.type == 4) {
                                            string toadd = "";
                                            toadd += to_string(c.type) + " " + to_string(c.position.x) + " " +
                                                     to_string(c.position.y) + " " + to_string(c.orientation) + " " +
                                                     c.value;
                                            client.sendMessage(toadd);
                                        }
                                    }
                                }
                                getline(cin, a);
                            } while (a.find("disconnect") == string::npos);
                        }
                        for(int i=0;i<forward.size();i++){
                            string positions[5];
                            int flag=0;
                            for(int j=0;j<forward[i].length();j++){
                                if(forward[i][j]==' '){
                                    flag++;
                                    continue;
                                }
                                positions[flag]+=forward[i][j];
                            }
                            C component;
                            int xplace=stoi(positions[1]);
                            int yplace=stoi(positions[2]);
                            component.position={xplace,yplace};
                            //editing right now not to lose
                            component.type = static_cast<ComponentType>(stoi(positions[0]));
                            component.orientation = static_cast<Orientation>(stoi(positions[3]));
                            component.value=positions[4];
                            components.push_back(component);
                        }

                        break;
                    }
                    case SDLK_b:

                        vector<string> elements;
                        for (const auto& w : wires) {
                            string toadd="";
                            toadd+="w"+to_string(w.first.x)+" "+to_string(w.first.y)+" "+to_string(w.second.x)+" "+to_string(w.second.y);
                            elements.push_back(toadd);
                        }

                        for (const auto& c : components){
                            string toadd="";
                            toadd+=to_string(c.type)+" "+to_string(c.position.x)+" "+to_string(c.position.y)+" "+to_string(c.orientation)+" "+c.value;
                            elements.push_back(toadd);
                        }


                        string totalpath=libpath+"/"+saving+".txt";
                        ofstream fileout(totalpath);
                         for(int i=0;i<elements.size();i++){
                             fileout<<elements[i]<<endl;
                         }
                         fileout.close();
                         allfilenames.push_back(saving);
                        ofstream filenames("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/lib/filenames ke be aghle jen ham nmirese.txt",ios::out);
                        for(string i:allfilenames) {
                            filenames<<i<<endl;
                        }
                        filenames.close();

                        break;
                }
            }
            if (menuState == MENU_OPEN && e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_1:
                        menuState = MENU_DC;
                        break;
                    case SDLK_2:
                        menuState = MENU_TRAN;
                        SDL_StartTextInput();
                        break;
                    case SDLK_3:
                        menuState = MENU_SWEEP;
                        break;
                    case SDLK_4:
                        menuState = MENU_CLOSED;
                        break;
                }
            } else if (menuState == MENU_DC && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                menuState = MENU_CLOSED;
            }
            else if (menuState == MENU_TRAN) {
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        if (!enteringTransientParams) {
                            enteringTransientParams = true;
                            isEnteringDt = true;
                            transientParamInput.clear();
                            SDL_StartTextInput();
                        } else {
                            try {
                                if (isEnteringDt) {
                                    dtValue = stod(transientParamInput);
                                    isEnteringDt = false;
                                    transientParamInput.clear();
                                } else {
                                    tEndValue = stod(transientParamInput);
                                    enteringTransientParams = false;
                                    isEnteringDt = true;
                                    SDL_StopTextInput();

                                    cout << "Running transient analysis with dt=" << dtValue
                                         << ", tEnd=" << tEndValue << endl;
                                    menuState = MENU_PLACING_PROBES;
                                    mode = PLACING_PROBES;

                                    voltageProbes.clear();
                                    currentProbes.clear();


                                    components.erase(
                                            remove_if(components.begin(), components.end(),
                                                      [](const C& comp) {
                                                          return comp.type == VOLTAGE_PROBE || comp.type == CURRENT_PROBE;
                                                      }),
                                            components.end()
                                    );

                                }
                                c = true ;
                            } catch (const exception& e) {
                                cout << "Invalid input: " << e.what() << endl;
                                transientParamInput.clear();
                            }
                        }
                    }
                    else if (e.key.keysym.sym == SDLK_BACKSPACE && enteringTransientParams) {
                        if (!transientParamInput.empty()) {
                            transientParamInput.pop_back();
                        }
                    }
                    else if (e.key.keysym.sym == SDLK_ESCAPE && enteringTransientParams) {
                        enteringTransientParams = false;
                        isEnteringDt = true;
                        transientParamInput.clear();
                        SDL_StopTextInput();
                    }
                }
                else if (e.type == SDL_TEXTINPUT && enteringTransientParams) {
                    if (isdigit(e.text.text[0]) || e.text.text[0] == '.' || e.text.text[0] == '-') {
                        transientParamInput += e.text.text;
                    }
                }
                if (enteringTransientParams) {
                    Uint32 currentTime = SDL_GetTicks();
                    if (currentTime - cursorBlinkTime > 500) {
                        showCursor = !showCursor;
                        cursorBlinkTime = currentTime;
                    }
                }
            }

            else if (menuState == MENU_TRAN) {
                SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
                SDL_Rect menu = {200, 150, 300, 150};
                SDL_RenderFillRect(renderer, &menu);

                drawText("Transient Analysis", 210, 160, {0, 0, 0, 255});

                if (!enteringTransientParams) {
                    drawText("Press ENTER to input", 210, 190, {0, 0, 0, 255});
                    drawText("parameters", 210, 220, {0, 0, 0, 255});
                }
            }
            else if (menuState == MENU_SWEEP) {

            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                if (mode == NORMAL) {
                    int mx = e.button.x;
                    int my = e.button.y;
                    selectedComponent = getComponentAt(mx, my);
                    if (selectedComponent) {
                        mode = EDITING;
                    }
                }
                if (mode == PREVIEW) {
                    mode = NORMAL;
                    placingType = NONE;
                }
            }
            if (mode == EDITING) {
                int xcord = e.button.x;
                int ycord = e.button.y;
                bool enteringfilename = true;
                SDL_Rect nameplace = {xcord, ycord, 200, 120};
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &nameplace);
                Uint8 RBB = 255, GBB = 255, BBB = 255;
                SDL_Color namecolor = {RBB, GBB, BBB, 255};
                SDL_RenderPresent(renderer);

                string setname = selectedComponent->name;
                string setvalue = selectedComponent->value;
                bool ending = false;
                bool editingName = true;
                bool refreshDisplay = true;

                SDL_StartTextInput();

                while (!ending) {
                    SDL_Event localEvent;

                    if (refreshDisplay) {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_RenderFillRect(renderer, &nameplace);

                        SDL_Color nameFieldColor = editingName ? (SDL_Color){50, 255, 255, 255} : (SDL_Color){RBB, GBB, BBB, 100};
                        SDL_Surface* nameSurface = TTF_RenderText_Blended(font,
                                                                          setname.empty() ? "Enter name" : setname.c_str(), nameFieldColor);
                        SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer, nameSurface);
                        SDL_Rect nameRect = {xcord + 100 - nameSurface->w/2, ycord + 30, nameSurface->w, nameSurface->h};
                        SDL_RenderCopy(renderer, nameTexture, nullptr, &nameRect);
                        SDL_FreeSurface(nameSurface);
                        SDL_DestroyTexture(nameTexture);

                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                        SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 190, ycord + 60);

                        SDL_Color valueFieldColor = !editingName ? (SDL_Color){50, 255, 255, 255} : (SDL_Color){RBB, GBB, BBB, 100};
                        SDL_Surface* valueSurface = TTF_RenderText_Blended(font,
                                                                           setvalue.empty() ? "Enter value" : setvalue.c_str(), valueFieldColor);
                        SDL_Texture* valueTexture = SDL_CreateTextureFromSurface(renderer, valueSurface);
                        SDL_Rect valueRect = {xcord + 100 - valueSurface->w/2, ycord + 70, valueSurface->w, valueSurface->h};
                        SDL_RenderCopy(renderer, valueTexture, nullptr, &valueRect);
                        SDL_FreeSurface(valueSurface);
                        SDL_DestroyTexture(valueTexture);

                        SDL_RenderPresent(renderer);
                        refreshDisplay = false;
                    }

                    if (SDL_WaitEvent(&localEvent)) {
                        if (localEvent.type == SDL_QUIT) {
                            ending = true;
                            run = false;
                        }
                        else if (localEvent.type == SDL_KEYDOWN) {
                            if (localEvent.key.keysym.sym == SDLK_ESCAPE) {
                                ending = true;
                            }
                            else if (localEvent.key.keysym.sym == SDLK_RETURN) {
                                ending = true;
                            }
                            else if (localEvent.key.keysym.sym == SDLK_TAB) {
                                editingName = !editingName;
                                refreshDisplay = true;
                            }
                            else if (localEvent.key.keysym.sym == SDLK_BACKSPACE) {
                                if (editingName && !setname.empty()) {
                                    setname.pop_back();
                                }
                                else if (!editingName && !setvalue.empty()) {
                                    setvalue.pop_back();
                                }
                                refreshDisplay = true;
                            }
                        }
                        else if (localEvent.type == SDL_TEXTINPUT) {
                            if (editingName) {
                                setname += localEvent.text.text;
                            }
                            else {
                                setvalue += localEvent.text.text;
                            }
                            refreshDisplay = true;
                        }
                        else if (localEvent.type == SDL_MOUSEBUTTONDOWN) {
                            int mx = localEvent.button.x;
                            int my = localEvent.button.y;


                            if (mx >= xcord && mx <= xcord + 200 && my >= ycord && my <= ycord + 120) {
                                if (my <= ycord + 60) {
                                    editingName = true;
                                }
                                else {
                                    editingName = false;
                                }
                                refreshDisplay = true;
                            }
                            else {
                                ending = true;
                            }
                        }
                    }
                }

                SDL_StopTextInput();
                if (!setname.empty() && !setvalue.empty()) {
                    try {
                        cout << "Updating component: " << selectedComponent->name << " -> " << setname
                             << " with value: " << setvalue << endl;

                        for (int j = 0; j < components.size(); j++) {
                            if (selectedComponent->position.x == components[j].position.x &&
                                selectedComponent->position.y == components[j].position.y &&
                                selectedComponent->type == components[j].type) {
                                components[j].value = setvalue;
                                components[j].name = setname;
                                break;
                            }
                        }

                        if (selectedComponent->type == RESISTOR) {
                            for (int j = 0; j < resistors.size(); j++) {
                                if (selectedComponent->position.x == resistors[j].position.x &&
                                    selectedComponent->position.y == resistors[j].position.y) {
                                    resistors[j].value = setvalue;
                                    resistors[j].name = setname;
                                    break;
                                }
                            }
                        }
                        else if (selectedComponent->type == INDUCTOR) {
                            for (int j = 0; j < inductors.size(); j++) {
                                if (selectedComponent->position.x == inductors[j].position.x &&
                                    selectedComponent->position.y == inductors[j].position.y) {
                                    inductors[j].value = setvalue;
                                    inductors[j].name = setname;
                                    break;
                                }
                            }
                        }
                        else if (selectedComponent->type == CAPACITOR) {
                            for (int j = 0; j < capacitors.size(); j++) {
                                if (selectedComponent->position.x == capacitors[j].position.x &&
                                    selectedComponent->position.y == capacitors[j].position.y) {
                                    capacitors[j].value = setvalue;
                                    capacitors[j].name = setname;
                                    break;
                                }
                            }
                        }
                        else if (selectedComponent->type == VOLTAGE_SOURCE) {
                            for (int j = 0; j < voltageSources.size(); j++) {
                                if (selectedComponent->position.x == voltageSources[j].position.x &&
                                    selectedComponent->position.y == voltageSources[j].position.y) {
                                    voltageSources[j].value = setvalue;
                                    voltageSources[j].name = setname;
                                    break;
                                }
                            }
                        }
                        else if (selectedComponent->type == CURRENT_SOURCE) {
                            for (int j = 0; j < currentSources.size(); j++) {
                                if (selectedComponent->position.x == currentSources[j].position.x &&
                                    selectedComponent->position.y == currentSources[j].position.y) {
                                    currentSources[j].value = setvalue;
                                    currentSources[j].name = setname;
                                    break;
                                }
                            }
                        }

                    } catch (const exception& ex) {
                        cout << "Error updating component: " << ex.what() << endl;
                    }
                }

                mode = NORMAL;
            }
            else if (mode == PLACING_PROBES) {
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int x = roundToGrid(e.button.x, GRID_SIZE);
                    int y = roundToGrid(e.button.y, GRID_SIZE);

                    if (e.button.button == SDL_BUTTON_LEFT) {
                        placeVoltageProbe(x, y);
                    } else if (e.button.button == SDL_BUTTON_RIGHT) {
                        placeCurrentProbe(x, y);
                    }
                }

                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_o) {
                        runAnalysisWithProbes();
                    } else if (e.key.keysym.sym == SDLK_ESCAPE) {

                        menuState = MENU_CLOSED;
                        mode = NORMAL;
                        cout << "Analysis cancelled" << endl;
                    }
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int x = roundToGrid(e.button.x, GRID_SIZE);
                int y = roundToGrid(e.button.y, GRID_SIZE);
                C newComp = { placingType, { x, y }, placing };
                if(x>=1000 && x<=1040 && y>=84 && y<=104) {
                    bool running = true;
                    int selectedIndex = 0;

                    SDL_Event e;
                    while (running) {
                        while (SDL_PollEvent(&e)) {
                            if (e.type == SDL_QUIT) running = false;
                            if (e.type == SDL_KEYDOWN) {
                                switch (e.key.keysym.sym) {
                                    case SDLK_UP:
                                        selectedIndex = (selectedIndex - 1 + allfilenames.size()) % allfilenames.size();
                                        break;
                                    case SDLK_DOWN:
                                        selectedIndex = (selectedIndex + 1) % allfilenames.size();
                                        break;
                                    case SDLK_RETURN:
                                        std::cout << "Selected file: " << allfilenames[selectedIndex] << std::endl;
                                        running = false; // exit menu after selection
                                        break;
                                }
                            }
                        }

                        // clear screen
                        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
                        SDL_RenderClear(renderer);

                        // draw each filename
                        for (size_t i = 0; i < allfilenames.size(); i++) {
                            SDL_Color color = (i == selectedIndex) ? SDL_Color{255, 255, 0} : SDL_Color{200, 200, 200};

                            SDL_Surface *surface = TTF_RenderText_Solid(font, allfilenames[i].c_str(), color);
                            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

                            SDL_Rect destRect = {50, 50 + (int) i * 40, surface->w, surface->h};
                            SDL_RenderCopy(renderer, texture, NULL, &destRect);

                            SDL_FreeSurface(surface);
                            SDL_DestroyTexture(texture);
                        }

                        SDL_RenderPresent(renderer);
                    }
                    string fullpath=libpath+"/"+allfilenames[selectedIndex]+".txt";
                    ifstream filein(fullpath);
                    vector<string> a;
                    string line;
                    while (std::getline(filein, line)) {
                        a.push_back(line);
                    }
                    for(int i=0;i<a.size();i++){
                        if(a[i][0]=='w'){
                            string positions[4];
                            int flag=0;
                            for(int j=1;j<a[i].length();j++){
                               if(a[i][j]==' '){
                                   flag++;
                                   continue;
                               }
                               positions[flag]+=a[i][j];
                            }
                            int xfirst=stoi(positions[0]);
                            int yfirst=stoi(positions[1]);
                            int xsecond=stoi(positions[2]);
                            int ysecond=stoi(positions[3]);
                            SDL_Point pointno1={xfirst,yfirst};
                            SDL_Point pointno2={xsecond,ysecond};
                            wires.push_back ({pointno1,pointno2});
                        }
                        else{
                            string positions[5];
                            int flag=0;
                            for(int j=0;j<a[i].length();j++){
                                if(a[i][j]==' '){
                                    flag++;
                                    continue;
                                }
                                positions[flag]+=a[i][j];
                            }
                            C component;
                            int xplace=stoi(positions[1]);
                            int yplace=stoi(positions[2]);
                            component.position={xplace,yplace};
                            //editing right now not to lose
                            component.type = static_cast<ComponentType>(stoi(positions[0]));
                            component.orientation = static_cast<Orientation>(stoi(positions[3]));
                            component.value=positions[4];
                            components.push_back(component);
                        }
                    }

                }
                    if (y < 140) {
                    for (auto &btn : toolbarButtons) {
                        if (x >= btn.rect.x && x <= btn.rect.x + btn.rect.w &&
                            y >= btn.rect.y && y <= btn.rect.y + btn.rect.h) {
                            if (btn.text == "RUN") {
                                menuState = MENU_OPEN;
                            } else {
                                handle_toolbar_click(x, y);
                            }
                        }
                    }
                }
                else if (mode == PREVIEW && placingType != NONE) {
                    switch (placingType) {
                        case RESISTOR:       resistors.push_back(newComp);
                            resistors[resistors.size()-1].name="R"+ to_string(resistors.size());
                            newComp.name="R"+ to_string(resistors.size());
                            break;
                        case CAPACITOR:      capacitors.push_back(newComp);
                            capacitors[capacitors.size()-1].name="C"+ to_string(capacitors.size());
                            newComp.name="C"+ to_string(capacitors.size());
                            break;
                        case INDUCTOR:       inductors.push_back(newComp);
                            inductors[inductors.size()-1].name="L"+ to_string(inductors.size());
                            newComp.name="L"+ to_string(inductors.size());
                            break;
                        case VOLTAGE_SOURCE: voltageSources.push_back(newComp);
                            voltageSources[voltageSources.size()-1].name="V"+ to_string(voltageSources.size());
                            newComp.name="V"+ to_string(voltageSources.size());
                            break;
                        case CURRENT_SOURCE: currentSources.push_back(newComp);
                            currentSources[currentSources.size()-1].name="I"+ to_string(currentSources.size());
                            newComp.name="I"+ to_string(currentSources.size());
                            break;
                        case Ground: grounds.push_back(newComp);
                            grounds[grounds.size()-1].name="G"+ to_string(grounds.size());
                            newComp.name="I"+ to_string(grounds.size());
                            break;
                        default: break;
                    }
                    components.push_back(newComp);
                    mode = NORMAL;
                    placingType = NONE;
                }  else {
                    if (!waitingForSecondClick) {
                        firstPoint = { x, y };
                        waitingForSecondClick = true;
                    } else {
                        SDL_Point second = { x, y };
                        if (abs(second.x - firstPoint.x) >= abs(second.y - firstPoint.y))
                            second.y = firstPoint.y;
                        else
                            second.x = firstPoint.x;
                        wires.push_back({ firstPoint, second });
                        waitingForSecondClick = false;
                    }
                }
            }
        }
        if (mode == PREVIEW) {
            int mx, my;
            SDL_GetMouseState(&mx, &my);

            preview.type = placingType;
            preview.position = { roundToGrid(mx, GRID_SIZE), roundToGrid(my, GRID_SIZE) };
            preview.orientation = placing;
            redraw_all(&preview);
        } else {
            redraw_all();
        }

        SDL_Delay(10);
    }


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    SDL_Quit();
    return 0;
}