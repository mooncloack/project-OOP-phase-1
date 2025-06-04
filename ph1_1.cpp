#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <exception>
#include <map>
#include <iomanip>
using namespace std;
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

class same_component : public exception {
private:
    string name;
    string message;
public:
    explicit same_component(const string& c_name) : name(c_name) {
        message = "Error: Resistor " + name + " already exists in the circuit";
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

class Run {
private:
    string line;
    Circuit* circuit;
public:
    Run(string input, Circuit* c) : line(input), circuit(c) {}
    void parse() {
        try {
            stringstream ss(line);
            string cmd, c_name, node1_n, node2_n;
            ss >> cmd >> c_name >> node1_n;
            if (cmd != "add" || c_name.size() < 2)
                throw invalid_command();
            char type = c_name[0];
            if (type == 'G' && c_name == "GND") {
                string extra;
                if (ss >> extra)
                    throw invalid_command();
                Node* node1 = circuit->get_or_create_node(node1_n);
                circuit->grounds.emplace_back(node1);
                cout << "Ground added at node " << node1_n << " successfully." << endl;
            }
            else if (type == 'D') {
                if (!(ss >> node2_n))
                    throw invalid_command();
                string extra;
                if (ss >> extra)
                    throw invalid_command();
                Node* node1 = circuit->get_or_create_node(node1_n);
                Node* node2 = circuit->get_or_create_node(node2_n);
                if (circuit->component_exists(c_name))
                    throw same_component(c_name);
                circuit->diodes.emplace_back("default_model", c_name, node1, node2);
                cout << "Component " << c_name << " of type Diode added successfully." << endl;
            }
            else if (type == 'R' || type == 'C' || type == 'L' || type == 'V' || type == 'I') {
                if (!(ss >> node2_n))
                    throw invalid_command();
                double value;
                if (!(ss >> value))
                    throw invalid_command();
                string extra;
                if (ss >> extra)
                    throw invalid_command();
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
                cout << "Component " << c_name << " of type " << type << " added successfully." << endl;
            }
            else {
                throw element_not_found(string(1, type));
            }
        }
        catch (const exception& e) {
            cout << e.what() << endl;
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
    void analyze() {
        prepare();
        if (N == 0) {
            cout << "No non-ground nodes to analyze." <<endl;
            return;
        }
        matrix A(N + M, N + M);
        matrix z(N + M, 1);
        for (int i = 0; i < N + M; ++i) {
            for (int j = 0; j < N + M; ++j)
                A.get_data()[i][j] = 0.0;
            z.get_data()[i][0] = 0.0;
        }
        for (const auto& r : circuit->resistors) {
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
        for (const auto& cur : circuit->current_s) {
            int n1 = node_pos(cur.get_node1());
            int n2 = node_pos(cur.get_node2());
            double I = cur.getCurrent();
            if (n1 >= 0) z.get_data()[n1][0] -= I;
            if (n2 >= 0) z.get_data()[n2][0] += I;
        }
        for (size_t i = 0; i < circuit->voltage_s.size(); ++i) {
            auto& vs = circuit->voltage_s[i];
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
        try {
            matrix x = A.inverse() * z;
            cout << fixed << setprecision(4);
            for (const auto& [name, index] : nodeIndex) {
                for (auto n : circuit->nodes) {
                    if (n->get_name() == name) {
                        n->setVoltage(x.get_data()[index][0]);
                        break;
                    }
                }
                cout << "Voltage at node " << name << " = " << x.get_data()[index][0] << " V\n";
            }
            for (size_t i = 0; i < circuit->voltage_s.size(); ++i) {
                double I_vs = x.get_data()[N + i][0];
                cout << "Current through voltage source " << circuit->voltage_s[i].get_name() << " = " << I_vs << " A\n";
            }
        }
        catch (const exception& e) {
            cout << "MNA analysis failed: " << e.what() <<endl;
        }
    }
};
int main() {
    Circuit circuit;
    string line;
    cout << "Enter commands (type 'run' to analyze):" << endl;
    while (true) {
        getline(cin, line);
        if (line == "run") {
            break;
        }
        Run runner(line, &circuit);
        runner.parse();
    }
    MNA analyzer(&circuit);
    analyzer.analyze();
    return 0;
}