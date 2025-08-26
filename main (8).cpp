#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <exception>
#include <map>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <cmath>
#include <SDL2/SDL.h>
#include <algorithm>
#include <sys/types.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

vector<string> commands;
double zoommultiplier=1;
pair<int,int> selectedx={0,1100};
pair<int,int> selectedy={0,1000};








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

    void start() {
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
        handleClient();
    }

    void handleClient() {
        char buffer[1024] = {};
        int bytesReceived = 0;
        while(true) {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string msg(buffer);
                std::cout << "Received: " << msg << "\n";

                if(msg.find("disconnect") != std::string::npos)
                    break;

                std::string reply = "Hello from server!";
                send(clientSocket, reply.c_str(), (int)reply.size(), 0);
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
                    }
                    else{
                        cout << "  Voltage at node " << pair.first << " = " << node_voltages_guess[pair.second]<<endl;
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
                    }
                    else{
                        cout << "Current through resistor " << r.get_name() << " = " <<I_R << " A" << endl;
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
                    }
                    else{
                        cout << "Current through capacitor " << cap.get_name() << " = " << I_C << " A" << endl;
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
                    }
                    else{
                        cout << "Current through inductor " << ind.get_name() << " = " <<I_L << " A" << endl;
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
    PREVIEW
};

enum ComponentType {
    NONE,
    RESISTOR,
    CAPACITOR,
    INDUCTOR,
    VOLTAGE_SOURCE,
    CURRENT_SOURCE,
    Ground
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
    string value="N/A";
    string name="N/A";
};

vector<C> components;
vector<pair<SDL_Point, SDL_Point>> wires;

int roundToGrid(int value, int gridSize) {
    return (int)(round((float)value / gridSize)) * gridSize;
}

void draw_grid() {
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    for (int x = 0; x < WINDOW_WIDTH; x += GRID_SIZE*zoommultiplier)
        SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
    for (int y = 0; y < WINDOW_HEIGHT; y += GRID_SIZE*zoommultiplier)
        SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);
}

void draw_resistor(SDL_Point pos, Orientation o,C csorce) {
    int w = 80*zoommultiplier, h = 20*zoommultiplier;
    SDL_Rect rectR;
    SDL_Rect rectU;
    SDL_Rect rectD;
    SDL_Rect rectL;




    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return ;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 10*zoommultiplier); // Replace with the path to your .ttf font
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return ;
    }




    SDL_Color a={0,0,0,200};
    SDL_Surface *wordSurface =TTF_RenderText_Blended(font, csorce.name.c_str(),a);

    SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
    int wordWidth = wordSurface->w;
    int wordHeight = wordSurface->h;
    SDL_Rect renderQuad = { pos.x,  pos.y, wordWidth, wordHeight};
    SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(wordTexture);
    SDL_FreeSurface(wordSurface);







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
void draw_inductor(SDL_Point pos, Orientation o,C csorce) {
    int radius = 5*zoommultiplier;
    int spacing = 10*zoommultiplier;



    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return ;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 10*zoommultiplier); // Replace with the path to your .ttf font
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return ;
    }




    SDL_Color a={0,0,0,200};
    SDL_Surface *wordSurface =TTF_RenderText_Blended(font, csorce.name.c_str(),a);

    SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
    int wordWidth = wordSurface->w;
    int wordHeight = wordSurface->h;
    SDL_Rect renderQuad = { pos.x,  pos.y, wordWidth, wordHeight};
    SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(wordTexture);
    SDL_FreeSurface(wordSurface);


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
void draw_capacitor(SDL_Point pos, Orientation o,C csorce) {
    int length = 40*zoommultiplier;
    int plateHalf = 15*zoommultiplier;




    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return ;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 10*zoommultiplier); // Replace with the path to your .ttf font
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return ;
    }




    SDL_Color a={0,0,0,200};
    SDL_Surface *wordSurface =TTF_RenderText_Blended(font, csorce.name.c_str(),a);

    SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
    int wordWidth = wordSurface->w;
    int wordHeight = wordSurface->h;
    SDL_Rect renderQuad = { pos.x,  pos.y, wordWidth, wordHeight};
    SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(wordTexture);
    SDL_FreeSurface(wordSurface);


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
void draw_voltage_source(SDL_Point pos, Orientation o,C csorce) {
    int r = 15*zoommultiplier;


    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return ;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 10*zoommultiplier); // Replace with the path to your .ttf font
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return ;
    }




    SDL_Color a={0,0,0,200};
    SDL_Surface *wordSurface =TTF_RenderText_Blended(font, csorce.name.c_str(),a);

    SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
    int wordWidth = wordSurface->w;
    int wordHeight = wordSurface->h;
    SDL_Rect renderQuad = { pos.x,  pos.y, wordWidth, wordHeight};
    SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(wordTexture);
    SDL_FreeSurface(wordSurface);


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

void draw_ground(SDL_Point pos, Orientation o, C csorce) {
    int r = 15*zoommultiplier;

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


void draw_current_source(SDL_Point pos, Orientation o,C csorce) {
    int r = 15*zoommultiplier;


    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return ;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 10*zoommultiplier); // Replace with the path to your .ttf font
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return ;
    }




    SDL_Color a={0,0,0,200};
    SDL_Surface *wordSurface =TTF_RenderText_Blended(font, csorce.name.c_str(),a);

    SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
    int wordWidth = wordSurface->w;
    int wordHeight = wordSurface->h;
    SDL_Rect renderQuad = { pos.x,  pos.y, wordWidth, wordHeight};
    SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(wordTexture);
    SDL_FreeSurface(wordSurface);

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
void draw_component(const C& c) {
    if(c.position.x>selectedx.first && c.position.x<selectedx.second && c.position.y>selectedy.first && c.position.y<selectedy.second ) {
        switch (c.type) {
            case RESISTOR:
                draw_resistor(c.position, c.orientation,c);
                break;
            case CAPACITOR:
                draw_capacitor(c.position, c.orientation,c);
                break;
            case INDUCTOR:
                draw_inductor(c.position, c.orientation,c);
                break;
            case VOLTAGE_SOURCE:
                draw_voltage_source(c.position, c.orientation,c);
                break;
            case CURRENT_SOURCE:
                draw_current_source(c.position, c.orientation,c);
                break;
            case Ground:
                draw_ground(c.position, c.orientation,c);
                break;
            default:
                break;
        }
    }
}

SDL_Point firstPoint;
bool waitingForSecondClick = false;
void redraw_all(const C* preview = nullptr) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    draw_grid();
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (const auto& w : wires) {
        if(w.first.x>selectedx.first && w.second.x<selectedx.second && w.first.y>selectedy.first && w.second.y<selectedy.second ) {
            SDL_RenderDrawLine(renderer, w.first.x, w.first.y, w.second.x, w.second.y);
        }
    }
    if (waitingForSecondClick) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        SDL_Point current = { roundToGrid(mx, GRID_SIZE*zoommultiplier), roundToGrid(my, GRID_SIZE*zoommultiplier) };
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

    SDL_RenderPresent(renderer);
}
Orientation placing = LEFT ;
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    window = SDL_CreateWindow("SPICE Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return -1;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 18);
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }


    bool run = true;
    Mode mode = NORMAL;
    ComponentType placingType = NONE;
    SDL_Event e;

    while (run) {
        C preview = { NONE, {-1, -1} };

        while (SDL_PollEvent(&e)) {

            //------------------------------------
            if(e.type==SDL_KEYDOWN && (e.key.keysym.sym==SDLK_1 || e.key.keysym.sym==2)) {
                int choice;
                std::cout << "Run as (1) Server or (2) Client? ";
                std::cin >> choice;

                if (choice == 1) {
                    TCPServer server(8080);
                    server.start();
                } else {
                    std::string ip;
                    std::cout << "Enter server IP (use 127.0.0.1 for local test): ";
                    std::cin >> ip;
                    TCPClient client(ip, 8080);
                    client.sendMessage("Hello from client!");
                    string a;
                    cin.ignore();
                    do {
                        getline(cin, a);
                        client.sendMessage(a);
                    } while (a.find("disconnect") == string::npos);
                }
            }


            //-----------------------------------
            if (e.type == SDL_QUIT) run = false;

            if(e.type==SDL_MOUSEWHEEL){
                if(e.wheel.y>0 && zoommultiplier<4){
                    zoommultiplier+=0.5;
                    int xpos,ypos;
                    SDL_GetMouseState(&xpos,&ypos);

                    selectedx={max(0,(int)(xpos-550/zoommultiplier)),min(1100,(int) (xpos+550/zoommultiplier))};
                    selectedy={max(0,(int)(ypos-500/zoommultiplier)),min(1000,(int) (ypos+500/zoommultiplier))};
                }
                if(e.wheel.y<0 && zoommultiplier>0.5){
                    zoommultiplier-=0.5;
                    int xpos,ypos;
                    SDL_GetMouseState(&xpos,&ypos);
                    selectedx={max(0,(int)(xpos-550/zoommultiplier)),min(1100,(int) (xpos+550/zoommultiplier))};
                    selectedy={max(0,(int)(ypos-500/zoommultiplier)),min(1000,(int) (ypos+500/zoommultiplier))};
                }

                if(zoommultiplier==1){
                    selectedx={0,1100};
                    selectedy={0,1000};
                }

                string zoomedin="X"+to_string(zoommultiplier);
                SDL_Color a={0,0,0,200};
                SDL_Surface *wordSurface =TTF_RenderText_Blended(font, zoomedin.c_str(),a);


                SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                int wordWidth = wordSurface->w;
                int wordHeight = wordSurface->h;
                SDL_Rect renderQuad = { 40,  30, wordWidth, wordHeight};
                SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                SDL_DestroyTexture(wordTexture);
                SDL_FreeSurface(wordSurface);
                SDL_RenderPresent(renderer);
                SDL_Delay(250);



            }

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
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                if (mode == PREVIEW) {
                    mode = NORMAL;
                    placingType = NONE;
                } //else if (waitingForSecondClick) {
                  //  waitingForSecondClick = false;
                  //coming back to here debug required
                  else{
                      C check;
                      int indexcomp=0;
                      bool found=false;
                      for(auto x:components) {
                          if (e.button.x < x.position.x + 10 && e.button.x > x.position.x - 10) {
                              if (e.button.y < x.position.y + 10 && e.button.y > x.position.y - 10) {
                                  check = x;
                                  found = true;
                                  break;
                              }
                          }
                          indexcomp++;
                      }
                      //------------------------------------------------------
                      if(found) {
                          int xcord=e.button.x;
                          int ycord=e.button.y;
                          bool enteringfilename = true;
                          SDL_Rect nameplace = {xcord, ycord, 200, 120};
                          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                          SDL_RenderFillRect(renderer, &nameplace);
                          Uint8 RBB =255, GBB = 255, BBB = 255;
                          SDL_Color namecolor = {RBB, GBB, BBB, 255};
                          SDL_RenderPresent(renderer);
                          string setname;
                          string setvalue;
                          bool ending=false;
                          while(!(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE)) {
                              SDL_Color showenternamecolor = {RBB, GBB, BBB, 100};
                              SDL_Surface *wordSurface ;
                              if(setname.size()>1){
                                  wordSurface = TTF_RenderText_Blended(font, setname.c_str(),showenternamecolor);
                              }
                              else{wordSurface=TTF_RenderText_Blended(font, "Enter name",showenternamecolor);}
                              SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                              int wordWidth = wordSurface->w;
                              int wordHeight = wordSurface->h;
                              SDL_Rect renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};
                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                              SDL_DestroyTexture(wordTexture);
                              SDL_FreeSurface(wordSurface);
                              SDL_RenderPresent(renderer);

                              SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                              SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180, ycord + 60);
                              SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

                              if(setvalue.size()>1){
                                  wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),showenternamecolor);
                              }
                              else {
                                  wordSurface = TTF_RenderText_Blended(font, "Enter value", showenternamecolor);
                              }
                              wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                              wordWidth = wordSurface->w;
                              wordHeight = wordSurface->h;
                              renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                              SDL_DestroyTexture(wordTexture);
                              SDL_FreeSurface(wordSurface);
                              SDL_RenderPresent(renderer);
                              while (!(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT &&
                                       e.button.x <= xcord + 200 && e.button.x >= xcord && e.button.y >= ycord &&
                                       e.button.y <= ycord + 120)) {
                                  if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE){
                                      ending=true;
                                      break;
                                  }
                                  SDL_WaitEvent(&e);
                              }
                              if(ending){break;}
                              SDL_Color selectedcolor = {50, 255, 255, 255};
                              if (e.button.y <= ycord + 60 && e.button.y >= ycord && e.button.x >= xcord &&
                                  e.button.x <= xcord + 200) {
                                  SDL_Color showenternamecolor = {RBB, GBB, BBB, 100};
                                  if(setname.size()>1){
                                      wordSurface = TTF_RenderText_Blended(font, setname.c_str(),
                                                                           selectedcolor);
                                  }
                                  else{
                                      wordSurface = TTF_RenderText_Blended(font, "Enter name",
                                                                                        selectedcolor);
                                  }
                                  cout<<"in";
                                  SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                  int wordWidth = wordSurface->w;
                                  int wordHeight = wordSurface->h;
                                  SDL_Rect renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};
                                  SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                  SDL_DestroyTexture(wordTexture);
                                  SDL_FreeSurface(wordSurface);
                                  SDL_RenderPresent(renderer);


                                  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                  SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180, ycord + 60);
                                  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

                                  if(setvalue.size()>1){
                                      wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),
                                                                           showenternamecolor);
                                  }
                                  else{
                                      wordSurface = TTF_RenderText_Blended(font, "Enter value", showenternamecolor);
                                  }
                                  wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                  wordWidth = wordSurface->w;
                                  wordHeight = wordSurface->h;
                                  renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                                  SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                  SDL_DestroyTexture(wordTexture);
                                  SDL_FreeSurface(wordSurface);
                                  SDL_RenderPresent(renderer);

                                  while (SDL_WaitEvent(&e)) {
                                      if (e.type == SDL_KEYDOWN &&
                                          (e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_RETURN)) {
                                          break;
                                      }
                                      if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE) {
                                          if (setname.size() > 1) {
                                              setname.pop_back();
                                              SDL_RenderFillRect(renderer,
                                                                 &nameplace); // Render the updated filename
                                              SDL_Surface *wordSurface = TTF_RenderText_Blended(font,setname.c_str(),
                                                                                                selectedcolor);
                                              SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer,
                                                                                                      wordSurface);
                                              wordWidth = wordSurface->w;
                                              wordHeight = wordSurface->h;
                                              int x = xcord + 100 - wordWidth / 2;
                                              renderQuad = {x, ycord + 30, wordWidth, wordHeight};
                                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                              SDL_DestroyTexture(wordTexture);
                                              SDL_FreeSurface(wordSurface);

                                              SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                              SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180,
                                                                 ycord + 60);
                                              SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


                                              if(setvalue.size()>1){
                                                  wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),
                                                                                       showenternamecolor);
                                              }
                                              else{
                                                  wordSurface = TTF_RenderText_Blended(font, "Enter value",
                                                                                       showenternamecolor);
                                              }
                                              wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                              wordWidth = wordSurface->w;
                                              wordHeight = wordSurface->h;
                                              renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                              SDL_DestroyTexture(wordTexture);
                                              SDL_FreeSurface(wordSurface);

                                              SDL_RenderPresent(renderer);


                                              SDL_RenderPresent(renderer);
                                          } else if (setname.size() == 1) {
                                              setname.clear();
                                              SDL_RenderFillRect(renderer, &nameplace);
                                              wordSurface = TTF_RenderText_Blended(font, "Enter name",
                                                                                   selectedcolor);
                                              wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                              wordWidth = wordSurface->w;
                                              wordHeight = wordSurface->h;
                                              renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};

                                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                              SDL_DestroyTexture(wordTexture);
                                              SDL_FreeSurface(wordSurface);


                                              SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                              SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180,
                                                                 ycord + 60);
                                              SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);



                                              if(setvalue.size()>1){
                                                  wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),
                                                                                       showenternamecolor);
                                              }
                                              else{
                                                  wordSurface = TTF_RenderText_Blended(font, "Enter value",
                                                                                       showenternamecolor);
                                              }
                                              wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                              wordWidth = wordSurface->w;
                                              wordHeight = wordSurface->h;
                                              renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                              SDL_DestroyTexture(wordTexture);
                                              SDL_FreeSurface(wordSurface);

                                              SDL_RenderPresent(renderer);
                                          }
                                      }
                                      if (e.type == SDL_TEXTINPUT) {// Add the typed character to the filename
                                          setname += e.text.text; // Clear the previous text rendering
                                          SDL_RenderFillRect(renderer, &nameplace); // Render the updated filename
                                          wordSurface = TTF_RenderText_Blended(font, setname.c_str(),
                                                                               selectedcolor);
                                          wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                          wordWidth = wordSurface->w;
                                          wordHeight = wordSurface->h;
                                          int x = xcord + 100 - wordWidth / 2;
                                          renderQuad = {x, ycord + 30, wordWidth, wordHeight};
                                          SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                          SDL_DestroyTexture(wordTexture);
                                          SDL_FreeSurface(wordSurface);

                                          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                          SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180,
                                                             ycord + 60);
                                          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


                                          if(setvalue.size()>1){
                                              wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),
                                                                                   showenternamecolor);
                                          }
                                          else{
                                              wordSurface = TTF_RenderText_Blended(font, "Enter value",
                                                                                   showenternamecolor);
                                          }
                                          wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                          wordWidth = wordSurface->w;
                                          wordHeight = wordSurface->h;
                                          renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                                          SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                          SDL_DestroyTexture(wordTexture);
                                          SDL_FreeSurface(wordSurface);

                                          SDL_RenderPresent(renderer);
                                      }
                                      SDL_RenderPresent(renderer);
                                  }


                                  SDL_Rect nameplace = {xcord, ycord, 200, 120};
                                  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                  SDL_RenderFillRect(renderer, &nameplace);
                                  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                  SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180, ycord + 60);
                                  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

                                  if(setname.size()>1){
                                      wordSurface = TTF_RenderText_Blended(font, setname.c_str(),
                                                                           showenternamecolor);
                                  }
                                  else{
                                      wordSurface = TTF_RenderText_Blended(font, "Enter name",
                                                                           showenternamecolor);
                                  }
                                  wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                  wordWidth = wordSurface->w;
                                  wordHeight = wordSurface->h;
                                  renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};
                                  SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                  SDL_DestroyTexture(wordTexture);
                                  SDL_FreeSurface(wordSurface);

                                  if(setvalue.size()>1){
                                      wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),
                                                                           showenternamecolor);
                                  }
                                  else{
                                      wordSurface = TTF_RenderText_Blended(font, "Enter value",
                                                                           showenternamecolor);
                                  }
                                  wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                  wordWidth = wordSurface->w;
                                  wordHeight = wordSurface->h;
                                  renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                                  SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                  SDL_DestroyTexture(wordTexture);
                                  SDL_FreeSurface(wordSurface);

                                  SDL_RenderPresent(renderer);
                              }



                              else if (e.type==SDL_MOUSEBUTTONDOWN && e.button.y >= ycord + 60 && e.button.y <= ycord+120 && e.button.x >= xcord &&
                                  e.button.x <= xcord + 200) {
                                  SDL_Color showenternamecolor = {RBB, GBB, BBB, 100};
                                  SDL_Surface *wordSurface ;

                                  if(setname.size()>1){
                                      wordSurface = TTF_RenderText_Blended(font, setname.c_str(),
                                                                           showenternamecolor);
                                  }
                                  else{
                                      wordSurface=TTF_RenderText_Blended(font, "Enter name",
                                                                         showenternamecolor);
                                  }
                                  SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                  int wordWidth = wordSurface->w;
                                  int wordHeight = wordSurface->h;
                                  SDL_Rect renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};
                                  SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                  SDL_DestroyTexture(wordTexture);
                                  SDL_FreeSurface(wordSurface);
                                  SDL_RenderPresent(renderer);

                                  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                  SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180, ycord + 60);
                                  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                  if(setvalue.size()>1){
                                      wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),showenternamecolor);
                                  }
                                  else {
                                      wordSurface = TTF_RenderText_Blended(font, "Enter value", showenternamecolor);
                                  }
                                  wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                  wordWidth = wordSurface->w;
                                  wordHeight = wordSurface->h;
                                  renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                                  SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                  SDL_DestroyTexture(wordTexture);
                                  SDL_FreeSurface(wordSurface);
                                  SDL_RenderPresent(renderer);

                                  while (SDL_WaitEvent(&e)) {
                                      if (e.type == SDL_KEYDOWN &&
                                          (e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_RETURN)) {
                                          break;
                                      }
                                      if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE) {
                                          if (setvalue.size() > 1) {
                                              setvalue.pop_back();
                                              SDL_RenderFillRect(renderer,
                                                                 &nameplace); // Render the updated filename
                                              SDL_Surface *wordSurface = TTF_RenderText_Blended(font,
                                                                                                setvalue.c_str(),
                                                                                                selectedcolor);
                                              SDL_Texture *wordTexture = SDL_CreateTextureFromSurface(renderer,
                                                                                                      wordSurface);
                                              wordWidth = wordSurface->w;
                                              wordHeight = wordSurface->h;
                                              int x = xcord + 100 - wordWidth / 2;
                                              renderQuad = {x, ycord + 70, wordWidth, wordHeight};
                                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                              SDL_DestroyTexture(wordTexture);
                                              SDL_FreeSurface(wordSurface);

                                              SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                              SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180,
                                                                 ycord + 60);
                                              SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


                                              if(setname.size()>1){
                                                  wordSurface = TTF_RenderText_Blended(font, setname.c_str(),
                                                                                       showenternamecolor);
                                              }
                                              else{
                                                  wordSurface = TTF_RenderText_Blended(font, "Enter name",
                                                                                       showenternamecolor);
                                              }
                                              wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                              wordWidth = wordSurface->w;
                                              wordHeight = wordSurface->h;
                                              renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};
                                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                              SDL_DestroyTexture(wordTexture);
                                              SDL_FreeSurface(wordSurface);

                                              SDL_RenderPresent(renderer);


                                              SDL_RenderPresent(renderer);
                                          } else if (setvalue.size() == 1) {
                                              setvalue.clear();
                                              SDL_RenderFillRect(renderer, &nameplace);

                                              if(setname.size()>1){
                                                  wordSurface = TTF_RenderText_Blended(font, setname.c_str(),
                                                                                       showenternamecolor);
                                              }
                                              else{
                                                  wordSurface = TTF_RenderText_Blended(font, "Enter name",
                                                                                       showenternamecolor);
                                              }
                                              wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                              wordWidth = wordSurface->w;
                                              wordHeight = wordSurface->h;
                                              renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};

                                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                              SDL_DestroyTexture(wordTexture);
                                              SDL_FreeSurface(wordSurface);


                                              SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                              SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180,
                                                                 ycord + 60);
                                              SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


                                              wordSurface = TTF_RenderText_Blended(font, "Enter value",
                                                                                   selectedcolor);
                                              wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                              wordWidth = wordSurface->w;
                                              wordHeight = wordSurface->h;
                                              renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                                              SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                              SDL_DestroyTexture(wordTexture);
                                              SDL_FreeSurface(wordSurface);

                                              SDL_RenderPresent(renderer);
                                          }
                                      }
                                      if (e.type == SDL_TEXTINPUT) {// Add the typed character to the filename
                                          setvalue += e.text.text; // Clear the previous text rendering
                                          SDL_RenderFillRect(renderer, &nameplace); // Render the updated filename
                                          wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),
                                                                               selectedcolor);
                                          wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                          wordWidth = wordSurface->w;
                                          wordHeight = wordSurface->h;
                                          int x = xcord + 100 - wordWidth / 2;
                                          renderQuad = {x, ycord + 70, wordWidth, wordHeight};
                                          SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                          SDL_DestroyTexture(wordTexture);
                                          SDL_FreeSurface(wordSurface);

                                          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                          SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180,
                                                             ycord + 60);
                                          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


                                          if(setname.size()>1){
                                              wordSurface = TTF_RenderText_Blended(font, setname.c_str(),
                                                                                   showenternamecolor);
                                          }
                                          else{
                                              wordSurface = TTF_RenderText_Blended(font, "Enter name",
                                                                                   showenternamecolor);
                                          }
                                          wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                          wordWidth = wordSurface->w;
                                          wordHeight = wordSurface->h;
                                          renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};
                                          SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                          SDL_DestroyTexture(wordTexture);
                                          SDL_FreeSurface(wordSurface);

                                          SDL_RenderPresent(renderer);
                                      }

                                  }

                                  SDL_Rect nameplace = {xcord, ycord, 200, 120};
                                  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                  SDL_RenderFillRect(renderer, &nameplace);
                                  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                                  SDL_RenderDrawLine(renderer, xcord + 10, ycord + 60, xcord + 180, ycord + 60);
                                  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


                                  if(setname.size()>1){
                                      wordSurface = TTF_RenderText_Blended(font, setname.c_str(),
                                                                           showenternamecolor);
                                  }
                                  else{
                                      wordSurface = TTF_RenderText_Blended(font, "Enter name",
                                                                           showenternamecolor);
                                  }
                                  wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                  wordWidth = wordSurface->w;
                                  wordHeight = wordSurface->h;
                                  renderQuad = {xcord + 100-wordWidth/2, ycord + 30, wordWidth, wordHeight};
                                  SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                  SDL_DestroyTexture(wordTexture);
                                  SDL_FreeSurface(wordSurface);

                                  if(setvalue.size()>1){
                                      wordSurface = TTF_RenderText_Blended(font, setvalue.c_str(),
                                                                           showenternamecolor);
                                  }
                                  else{
                                      wordSurface = TTF_RenderText_Blended(font, "Enter value",
                                                                           showenternamecolor);
                                  }
                                  wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);
                                  wordWidth = wordSurface->w;
                                  wordHeight = wordSurface->h;
                                  renderQuad = {xcord + 100-wordWidth/2, ycord + 70, wordWidth, wordHeight};
                                  SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                                  SDL_DestroyTexture(wordTexture);
                                  SDL_FreeSurface(wordSurface);

                                  SDL_RenderPresent(renderer);
                              }




                          }
                          check.name=setname;
                          check.value=setvalue;
                          components[indexcomp]=check;
                      }
                    //---------------------------------------------------
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int x = roundToGrid(e.button.x, GRID_SIZE*zoommultiplier);
                int y = roundToGrid(e.button.y, GRID_SIZE*zoommultiplier);

                if (mode == PREVIEW && placingType != NONE) {
                    components.push_back({ placingType, { x, y }, placing ,"N/A"});
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
    string folderpath="C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/newsfolder"; //creating the files required
    if (CreateDirectory(folderpath.c_str(), NULL)) {
        cout << "Folder created successfully: " << folderpath << endl;
    }
    ifstream file("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/newsfolder/filenames ke be aghle jen ham nmirese.txt",ios::in);//making sure the name file exists
    if(!file.is_open()) {
        cout<<"File created successfuly"<<endl;
        ofstream file("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/newsfolder/filenames ke be aghle jen ham nmirese.txt",ios::out);//making sure the name file exists)
    }
    vector<string> allfilenames;
    ifstream filein("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/newsfolder/filenames ke be aghle jen ham nmirese.txt",ios::in);
    while(!filein.eof()) {
        string name;
        getline(filein,name);
        string check="C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/newsfolder/"+name+".txt";
        fstream filecheck(check,ios::in);
        if(filecheck.is_open()) {
            allfilenames.push_back(name);
        }
        filecheck.close();
    }
    filein.close();
    ofstream fileout("C:/Users/Dear User/OneDrive/Desktop/OOP project phase 2/newsfolder/filenames ke be aghle jen ham nmirese.txt",ios::out);
    for(string i:allfilenames) {
        fileout<<i<<endl;
    }
    fileout.close();
    string saving="draft"+to_string(allfilenames.size());
    allfilenames.push_back(saving);

    double dt,tEnd;
    map<string , int> underConsideration;
    Circuit circuit;
    string line;
    bool inwhile=false;
    cout << "Enter commands (type 'run' to analyze):" << endl;
//    while (true) {
//        getline(cin, line);
//        if (line == "run") {
//            inwhile=true;
//            break;
//        }
//        if (line.substr(0, 7) == ".PRINT ") {
//            istringstream ss(line);
//            string cmd, analysisType;
//            vector<string> printNodes;
//            vector<string> printCurrents;
//            ss >> cmd >> analysisType;
//            if (analysisType != "DC") {
//                cout << "Error: Only DC analysis is supported for .PRINT command" << endl;
//                continue;
//            }string token;
//            while (ss >> token) {
//                if (token[0] == 'V' && token[1] == '(') {
//                    string node = token.substr(2, token.size() - 3);
//                    printNodes.push_back(node);
//                }
//                else if (token[0] == 'I' && token[1] == '(') {
//                    string source = token.substr(2, token.size() - 3);
//                    printCurrents.push_back(source);
//                }
//            }
//            bool dcFound = false;
//            string dcLine;
//            for (const auto& cmd : commands) {
//                if (cmd.substr(0, 3) == "DC ") {
//                    dcLine = cmd;
//                    dcFound = true;
//                    break;
//                }
//            }
//            if (!dcFound) {
//                cout << "Error: No DC sweep command found before .PRINT" << endl;
//                continue;
//            }
//            istringstream dcSS(dcLine);
//            string dcCmd, sourceName;
//            double start, end, increment;
//            dcSS >> dcCmd >> sourceName >> start >> end >> increment;
//            MNA analyzer(&circuit);
//            analyzer.analyzeDCSweep(sourceName, start, end, increment, printNodes, printCurrents);
//            continue;
//        }
//        if (line.substr(0, 3) == "DC ") {
//            commands.push_back(line);
//            continue;
//        }
//
//        if(line=="-show existing schematics"){
//            while(true){
//                int j=1;
//                cout<<"-choose existing schematic:"<<endl;
//                for(string i:allfilenames){
//                    cout<<j++<<"-"<<i<<endl;
//                }
//                string inMenuCmd;
//                cin>>inMenuCmd;
//                if(inMenuCmd=="return"){cin.ignore(); break; }
//                else if(inMenuCmd=="NewFile"){
//                    cin.ignore();
//                    string name;
//                    getline(cin,name);
//                    allfilenames.push_back(name);
//                    string filename=folderpath+"/"+name+".txt";
//                    saving=name;
//                    ofstream schematic(filename,ios::out);
//                    for(int i=0;i<commands.size();i++){
//                        schematic<<commands[i];
//                        if(i!=commands.size()-1){
//                            schematic<<endl;
//                        }
//                    }
//                    schematic.close();
//                    filename="C:/Users/Asus/OneDrive/Desktop/OOP project/newsfolder/filenames ke be aghle jen ham nmirese.txt";
//                    ofstream filenames(filename,ios::out);
//                    for(int i=0;i<allfilenames.size();i++){
//                        filenames<<allfilenames[i]<<endl;
//                    }
//                    continue;
//                }
//                try {
//                    try {
//                        int fileToOpen = stoi(inMenuCmd);
//                        if (fileToOpen > j - 1 || fileToOpen < 1) {
//                            throw inapropriate_input();
//                        }
//                        string openedFile = folderpath + "/" + allfilenames[fileToOpen - 1] + ".txt";
//                        saving=allfilenames[fileToOpen-1];
//                        ifstream file(openedFile, ios::in);
//                        cout << allfilenames[fileToOpen - 1] << ":" << endl;
//                        commands.clear();
//                        circuit.deleteAll();
//                        while (!file.eof()) {
//                            string name;
//                            getline(file, name);
//                            commands.push_back(name);
//                            cout << name << endl;
//                            Run runner(name, &circuit);
//                            runner.parse(true,&dt,&tEnd,&underConsideration);
//                        }
//                        file.close();
//                    } catch (const invalid_argument &e) {
//                        throw inapropriate_input();
//
//                    }
//                }catch (const exception &e) {
//                    cout << e.what() << endl;
//                }
//            }
//            continue;
//        }
//
//        if (line.substr(0, 12) == ".readnetlist") {
//            string filename = line.substr(13);
//            ifstream netlist(filename);
//            if (!netlist.is_open()) {
//                cout << "Cannot open netlist file: " << filename << endl;
//                continue;
//            }
//            string netline;
//            while (getline(netlist, netline)) {
//                if (netline.empty() || netline[0] == '*') continue;
//                Run runner("", &circuit);
//                runner.parse_netlist_line(netline, &circuit);
//            }
//            netlist.close();
//            cout << "Netlist loaded and parsed." << endl;
//            continue;
//        }
//
//        if(inwhile){continue;}
//        Run runner(line, &circuit);
//        runner.parse(false,&dt,&tEnd,&underConsideration);
//        if(dt && tEnd){
//            MNA analyzer(&circuit);
//            analyzer.analyzeTransient(dt,tEnd,underConsideration);
//            underConsideration.clear();
//            dt=0;tEnd=0;
//            continue;
//        }
//        string filename=folderpath+"/"+saving+".txt";
//        ofstream schematic(filename,ios::out);
//        for(int i=0;i<commands.size();i++){
//            schematic<<commands[i];
//            if(i!=commands.size()-1){
//                schematic<<endl;
//            }
//        }
//        schematic.close();
//        filename="C:/Users/Asus/OneDrive/Desktop/OOP project/newsfolder/filenames ke be aghle jen ham nmirese.txt";
//        ofstream filenames(filename,ios::out);
//        for(int i=0;i<allfilenames.size();i++){
//            filenames<<allfilenames[i]<<endl;
//        }
//        filenames.close();
//    }
//    MNA analyzer(&circuit);
//    analyzer.analyze();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}