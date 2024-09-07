/*
Marco Antonio Riascos Salguero
Sistemas operativos 
Parcial práctico
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>

using namespace std;

class Process {
private:
    int id;        // ID del proceso
    int at;        // Arrival Time (Tiempo de llegada)
    int bt;        // Burst Time (Tiempo de ráfaga)
    int ct;        // Completion Time (Tiempo de finalización)
    int rt;        // Response Time (Tiempo restante)
    int wt;        // Waiting Time (Tiempo de espera)
    int priority;  // Prioridad del proceso
    int queuelevel;// cola a que pertenece
    int tat;       // Turnaround Time (Tiempo de respuesta)

public:
    // Constructor por defecto
    Process() : id(0), at(0), bt(0), ct(0), rt(0), wt(0), priority(0), tat(0), queuelevel(0) {}

    // Constructor con parámetros
    Process(int id, int at, int bt, int priority, int queue) 
        : id(id), at(at), bt(bt), ct(0), rt(bt), wt(0), priority(priority), tat(0), queuelevel(queue) {}

    // Getters
    int getId() const { return id; }
    int getArrivalTime() const { return at; }
    int getBurstTime() const { return bt; }
    int getCompletionTime() const { return ct; }
    int getResponseTime() const { return rt; }
    int getWaitingTime() const { return wt; }
    int getPriority() const { return priority; }
    int getTurnaroundTime() const { return tat; }
    int getQueue() const { return queuelevel; }

    // Setters
    void setId(int id) { this->id = id; }
    void setArrivalTime(int at) { this->at = at; }
    void setBurstTime(int bt) { this->bt = bt; }
    void setCompletionTime(int ct) { this->ct = ct; }
    void setResponseTime(int rt) { this->rt = rt; }
    void setWaitingTime(int wt) { this->wt = wt; }
    void setPriority(int priority) { this->priority = priority; }
    void setTurnaroundTime(int tat) { this->tat = tat; }
    void setQueuelevel(int queuelevel) { this->queuelevel = queuelevel; }
};

// Función para leer los datos del archivo y almacenar los procesos
void readInput(const string& filename, vector<Process>& processes, string& schedulingPolicy, int& defaultPriority, int& numberqueues) {
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo" << endl;
        return;
    }

    int numProcesses;
    file >> numProcesses;  // Leer el número de procesos

    file >> numberqueues; // leer el número de colas 

    // Leer la política de planificación (ej: FCFS, Round Robin, MLQ, MLFQ)
    file >> schedulingPolicy;

    // Leer la prioridad por defecto (en caso de ser necesario para más configuraciones)
    file >> defaultPriority;

    // Leer cada proceso (id, at, bt, priority, queuelevel) y almacenarlos en la lista de procesos
    for (int i = 0; i < numProcesses; ++i) {
        int id, at, bt, priority, queuelevel;
        file >> id >> at >> bt >> priority >> queuelevel;
        processes.push_back(Process(id, at, bt, priority, queuelevel));
    }

    file.close();
}


// Algoritmo First-Come, First-Served (FCFS)
void firstComeFirstServed(queue<Process>& q, int& counter) {
    if (!q.empty()) {
        Process currentProcess = q.front();
        q.pop();

        // Ejecutar el proceso
        int burst = currentProcess.getBurstTime();
        counter += burst;

        // Establecer tiempos en el proceso
        currentProcess.setCompletionTime(counter);
        currentProcess.setTurnaroundTime(currentProcess.getCompletionTime() - currentProcess.getArrivalTime());
        currentProcess.setWaitingTime(currentProcess.getTurnaroundTime() - burst);

        cout << "Proceso " << currentProcess.getId() << " ejecutado completamente. "
             << "CT: " << currentProcess.getCompletionTime() << ", "
             << "TAT: " << currentProcess.getTurnaroundTime() << ", "
             << "WT: " << currentProcess.getWaitingTime() << "\n";
    }
}

// Algoritmo Round Robin (RR)
void roundRobin(queue<Process>& q, int& counter, int timeQuantum) {
    if (!q.empty()) {
        Process currentProcess = q.front();
        q.pop();

        int burst = currentProcess.getBurstTime();
        int timeToExecute = min(burst, timeQuantum);
        counter += timeToExecute;

        // Actualizar el tiempo restante y el burst time
        currentProcess.setBurstTime(burst - timeToExecute);

        if (currentProcess.getBurstTime() > 0) {
            q.push(currentProcess);
        } else {
            currentProcess.setCompletionTime(counter);
            currentProcess.setTurnaroundTime(currentProcess.getCompletionTime() - currentProcess.getArrivalTime());
            currentProcess.setWaitingTime(currentProcess.getTurnaroundTime() - burst);
            cout << "Proceso " << currentProcess.getId() << " ejecutado completamente. "
                 << "TC: " << currentProcess.getCompletionTime() << ", "
                 << "TAT: " << currentProcess.getTurnaroundTime() << ", "
                 << "TW: " << currentProcess.getWaitingTime() << "\n";
        }
    }
}

// Algoritmo Multi-Level Feedback Queue (MLFQ)
void mlfq(vector<queue<Process>>& priorityQueues, int& counter, vector<int>& timeQuantums) {
    for (int i = 0; i < priorityQueues.size(); ++i) {
        if (!priorityQueues[i].empty()) {
            Process currentProcess = priorityQueues[i].front();
            priorityQueues[i].pop();

            int burst = currentProcess.getBurstTime();
            int timeQuantum = timeQuantums[i];
            int timeToExecute = min(burst, timeQuantum);
            counter += timeToExecute;

            // Actualizar el tiempo restante
            currentProcess.setBurstTime(burst - timeToExecute);

            // Si el proceso aún tiene tiempo restante, se degrada a una cola más baja
            if (currentProcess.getBurstTime() > 0) {
                int newQueueLevel = min(currentProcess.getQueue() + 1, static_cast<int>(priorityQueues.size()) - 1);
                currentProcess.setQueuelevel(newQueueLevel);
                priorityQueues[newQueueLevel].push(currentProcess);
            } else {
                currentProcess.setCompletionTime(counter);
                currentProcess.setTurnaroundTime(currentProcess.getCompletionTime() - currentProcess.getArrivalTime());
                currentProcess.setWaitingTime(currentProcess.getTurnaroundTime() - burst);
                cout << "Proceso " << currentProcess.getId() << " ejecutado completamente. "
                     << "TC: " << currentProcess.getCompletionTime() << ", "
                     << "TAT: " << currentProcess.getTurnaroundTime() << ", "
                     << "WT: " << currentProcess.getWaitingTime() << "\n";
            }
            // Salir después de ejecutar un proceso
            break;
        }
    }
}

// Función para ejecutar el sistema con múltiples colas (MLQ y MLFQ)
void os(vector<Process>& processes, int numQueues, const string& schedulingPolicy, int timeQuantum) {
    int counter = 0; // Tiempo actual
    vector<queue<Process>> priorityQueues(numQueues);
    vector<int> timeQuantums(numQueues, timeQuantum); // Inicializar los timeQuantums para cada cola

    // Mover procesos que han llegado al tiempo actual a la cola de listos
    while (!processes.empty() || any_of(priorityQueues.begin(), priorityQueues.end(), [](const queue<Process>& q) { return !q.empty(); })) {
        // Mover procesos que han llegado al tiempo actual a la cola correspondiente
        for (int i = 0; i < processes.size(); ++i) {
            if (processes[i].getArrivalTime() <= counter) {
                int queueLevel = processes[i].getQueue();
                if (queueLevel >= 1 && queueLevel <= numQueues) {
                    priorityQueues[queueLevel - 1].push(processes[i]);
                }
                processes.erase(processes.begin() + i);
                --i; // Decrementar i porque se eliminó un elemento del vector
            }
        }

        if (schedulingPolicy == "MLQ") {
            for (auto& queue : priorityQueues) {
                if (!queue.empty()) {
                    firstComeFirstServed(queue, counter);
                    break;
                }
            }
        } else if (schedulingPolicy == "RR") {
            for (auto& queue : priorityQueues) {
                if (!queue.empty()) {
                    roundRobin(queue, counter, timeQuantum);
                    break;
                }
            }
        } else if (schedulingPolicy == "MLFQ") {
            mlfq(priorityQueues, counter, timeQuantums);
        }
    }

    cout << "No hay mas procesos en la cola de listos." << endl;
}

int main() {
    vector<Process> processes;
    string schedulingPolicy;
    int defaultPriority;
    int numberqueues;
    int timeQuantum = 2; // Valor por defecto para el Round Robin

    // Llamar a la función para leer los procesos desde el archivo
    readInput("input2.txt", processes, schedulingPolicy, defaultPriority, numberqueues);

    // Ejecutar la política de planificación
    os(processes, numberqueues, schedulingPolicy, timeQuantum);

    return 0;
}
