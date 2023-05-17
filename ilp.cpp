#include <iostream>
#include <fstream>
#include "ilcplex/ilocplex.h"
#include <vector>
#include <map>
#include <set>
#include <random>
#include <queue>

#define INF 100000

typedef IloArray<IloArray<IloArray<IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>>>>> NumVar8d;
typedef IloArray<IloArray<IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>>>> NumVar7d;
typedef IloArray<IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>>> NumVar6d;
typedef IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>> NumVar5d;
typedef IloArray<IloArray<IloArray<IloNumVarArray>>> NumVar4d;
typedef IloArray<IloArray<IloNumVarArray>> NumVar3d;
typedef IloArray<IloNumVarArray> NumVar2d;

using namespace std;

class node {   //  graph node
public:
    int id;   // temporary int 
    double link;   // cost 
    double available_bandwidth;     // bandwith
    double init_bw;
    node(int id, double link, double bw) {
        this->id = id;
        this->link = link;
        this->available_bandwidth = bw;
        this->init_bw = bw;
    }
};

class inst_node {  // layer graph node
public:
    string id; /// id of the node
    vector<double> links; // k shortest links in the instance graph betweem two nodes
    vector<bool> vis_links; // has the link been visited or not
    inst_node(string id, vector<double> links, vector<bool> vis_links) {
        this->id = id;
        this->links = links;
        this->vis_links = vis_links;
    }
};

class node_capacity { //contains the list of NFs installed in node and the amount left of each of those instances.
public:
    map<int, double> NF_left; //maps a function instance to the amount of processing power left for that function(think NF shareability)
    map<int, double> deployed_NF; // tells how much processing time that NF requires.
    node_capacity() {}
    node_capacity(int id, double time) {
        NF_left[id] = 1;
        deployed_NF[id] = time;
    }
};

class Request {  //represents the parameters of the SFC request
public:
    int src;
    int dest;
    vector<vector<vector<int>>> SFC;
    double e2e;
    double t_arrival_rate;
    Request() {}
    Request(int src, int dest, vector<vector<vector<int>>> SFC, double e2e, double t_arrival_rate) {
        this->src = src;
        this->dest = dest;
        this->SFC = SFC;
        this->e2e = e2e;
        this->t_arrival_rate = t_arrival_rate;
    }
    Request(const Request& r) {
        this->src = r.src;
        this->dest = r.dest;
        this->SFC = r.SFC;
        this->e2e = r.e2e;
        this->t_arrival_rate = r.t_arrival_rate;
    }
};

class Request_ilp {  //represents the parameters of the SFC request
public:
    int src;
    int dest;
    vector<vector<int>> SFC;
    double e2e;
    double t_arrival_rate;
    Request_ilp() {}
    Request_ilp(int src, int dest, vector<vector<int>> SFC, double e2e, double t_arrival_rate) {
        this->src = src;
        this->dest = dest;
        this->SFC = SFC;
        this->e2e = e2e;
        this->t_arrival_rate = t_arrival_rate;
    }
    Request_ilp(const Request_ilp& r) {
        this->src = r.src;
        this->dest = r.dest;
        this->SFC = r.SFC;
        this->e2e = r.e2e;
        this->t_arrival_rate = r.t_arrival_rate;
    }
};

class Result {
public:
    double mean_latency;
    double mean_PD;
    vector<double> nodes_util;
    Result(int N) {
        this->mean_latency = 0;
        this->mean_PD = 0;
        for (int i = 0;i < N;i++)
            this->nodes_util.push_back(0);
    }
};

bool ILPsolve(vector<vector<int>>& g, vector<vector<double>>& bw, vector<vector<double>>& t_uv, vector<double> t_f,vector<vector<int>> dep, vector<vector<double>>& cap,Request_ilp request,Result& result,int funcs) {
    
    int N = g.size();

    int src = request.src;
    int dest = request.dest;
    vector<vector<int>> SFC = request.SFC;
    SFC.insert(SFC.begin(), vector<int>{10});
    SFC.insert(SFC.end(), vector<int>{11});
    int a=1;
    for (int i = 0;i < SFC.size();i++) {
        if (SFC[i].size() > 1) {
            a = i;
            break;
        }
    }

    cout << "SRC::" << src << " " << "DEST::" << dest << endl;

    for (int i = 0;i < SFC.size();i++) {
        for (int j = 0;j < SFC[i].size();j++) {
            cout << SFC[i][j] << "\t";
        }
        cout << endl;
    }

    double copy_time = 2, merge_time = 2;
    double T_l = request.e2e;
    double arrival_SFC = request.t_arrival_rate;

    int temp = (SFC[a].size() * (SFC[a].size() - 1));

    vector<int> B(SFC.size(), 1);
    B[a] = SFC[a].size();


    //resizing the vector to account for dummy functions;
    vector<double> t(temp, 0);
    t_f.insert(t_f.begin(), t.begin(), t.end());

    for (int i = 0;i < N;i++) {
        vector<int> d(temp, 0);
        vector<double> c(temp, 0);
        dep[i].insert(dep[i].end(), d.begin(), d.end());
        cap[i].insert(cap[i].end(), c.begin(), c.end());
    }

    //pushing the dummy functions into the SFC
    for (int i = 0;i < temp;i++) {
        SFC[a].push_back(funcs + 2 + i);
    }

    //making the dummy functions available everywhere
    for (int i = 0;i < temp;i++) {
        for (int u = 0;u < N;u++) {
            dep[u][funcs + 2 + i] = 1;
            cap[u][funcs + 2 + i] = INF;
        }
    }
    dep[src][10] = 1;
    dep[dest][11] = 1;
    cap[src][10] = INF;
    cap[dest][11] = INF;

    IloEnv env;
    try {
        IloModel model(env);
# pragma region Node Variable
        NumVar5d NODE(env, SFC.size());
        for (int i = 0;i < SFC.size();i++) {
            NumVar4d t1(env, B[i]);
            for (int b = 0;b < B[i];b++) {
                NumVar3d t2(env, B[i]);
                for (int s = 0;s < B[i];s++) {
                    NumVar2d t3(env, funcs + 2 + temp);
                    for (int f = 0;f < funcs + 2 + temp;f++) {
                        IloNumVarArray t4(env, N, 0, 1, ILOINT);
                        t3[f] = t4;
                    }
                    t2[s] = t3;
                }
                t1[b] = t2;
            }
            NODE[i] = t1;
        }
#pragma endregion

# pragma region EDGE Variable
        NumVar8d EDGE(env, SFC.size());
        for (int i = 0;i < SFC.size();i++) {
            NumVar7d t1(env, SFC.size());
            for (int i1 = 0;i1 < SFC.size();i1++) {
                NumVar6d t2(env, B[i]);
                for (int b = 0;b < B[i];b++) {
                    NumVar5d t3(env, B[i1]);
                    for (int b1 = 0;b1 < B[i1];b1++) {
                        NumVar4d t4(env, B[i]);
                        for (int s = 0;s < B[i];s++) {
                            NumVar3d t5(env, B[i1]);
                            for (int s1 = 0;s1 < B[i1];s1++) {
                                NumVar2d t6(env, N);
                                for (int v = 0;v < N;v++) {
                                    IloNumVarArray t7(env, N, 0, 1, ILOINT);
                                    t6[v] = t7;
                                }
                                t5[s1] = t6;
                            }
                            t4[s] = t5;
                        }
                        t3[b1] = t4;
                    }
                    t2[b] = t3;
                }
                t1[i1] = t2;
            }
            EDGE[i] = t1;
        }
#pragma endregion

        IloNumVarArray theta(env, 1, 0, IloInfinity, ILOFLOAT);
        double param1 = 0.8;
        double param2 = 0.1;
        double param3 = 0.1;

#pragma region Objective function
        IloExpr node_expr(env);
        IloExpr edge_expr(env);
        for (int b = 0;b < B[a];b++) {
            for (int s = 0;s < B[a];s++) {
                for (int f = 0;f < B[a];f++) {   //funcs is enough
                    for (int u = 0;u < N;u++) {
                        node_expr += (t_f[SFC[a][f]] * NODE[a][b][s][SFC[a][f]][u]);
                    }
                }
            }

            for (int s = 0;s < B[a] - 1;s++) {
                //for (int s1 = s+1;s1 < B[a];s1++) {
                //    for (int u = 0;u < N;u++) {
                //        for (int v = 0;v < N;v++) {
                //            edge_expr += (t_uv[u][v] * EDGE[a][a][b][b][s][s1][u][v]);
                //        }
                //    }
                //}
                for (int u = 0;u < N;u++) {
                    for (int v = 0;v < N;v++) {
                        if (g[u][v]) {
                            edge_expr += (t_uv[u][v] * EDGE[a][a][b][b][s][s + 1][u][v]);
                        }
                    }
                }
                //for (int u = 0;u < N;u++) {
                //    for (int v = 0;v < N;v++) {
                //        edge_expr += (t_uv[u][v] * (EDGE[a - 1][a][0][b][0][s][u][v] + EDGE[a][a + 1][b][0][s][0][u][v]));
                //    }
                //}
            }

            for (int u = 0;u < N;u++) {
                for (int v = 0;v < N;v++) {
                    if (g[u][v]) {
                        edge_expr += (t_uv[u][v] * (EDGE[a - 1][a][0][b][0][0][u][v] + EDGE[a][a + 1][b][0][B[a] - 1][0][u][v]));
                    }
                }
            }
        }

        IloExpr e2e_expr(env);
        e2e_expr += (B[a] - 1) * (copy_time + merge_time);
        for (int i = 0;i < a;i++) {
            for (int f = 0;f < B[i];f++) {
                for (int u = 0;u < N;u++) {
                    e2e_expr += NODE[i][0][0][SFC[i][f]][u] * t_f[SFC[i][f]];
                }
            }
            if (i < a - 1) {
                for (int u = 0;u < N;u++) {
                    for (int v = 0;v < N;v++) {
                        if (g[u][v]) {
                            e2e_expr += EDGE[i][i + 1][0][0][0][0][u][v] * t_uv[u][v];
                        }
                    }
                }
            }
        }
        for (int i = a + 1;i < SFC.size() - 1;i++) {
            for (int f = 0;f < B[i];f++) {
                for (int u = 0;u < N;u++) {
                    e2e_expr += NODE[i][0][0][SFC[i][f]][u] * t_f[SFC[i][f]];
                }
            }
            for (int u = 0;u < N;u++) {
                for (int v = 0;v < N;v++) {
                    if (g[u][v]) {
                        e2e_expr += EDGE[i][i + 1][0][0][0][0][u][v] * t_uv[u][v];
                    }
                }
            }
        }
        e2e_expr += theta[0];

        IloExpr bw_cons(env);
        for (int i = 0;i < SFC.size() - 1;i++) {
            for (int b = 0;b < B[i];b++) {
                for (int b1 = 0;b1 < B[i + 1];b1++) {
                    for (int u = 0;u < N;u++) {
                        for (int v = 0;v < N;v++) {
                            bw_cons += EDGE[i][i + 1][b][b1][B[i] - 1][0][u][v] * arrival_SFC;
                        }
                    }
                }
            }
            if (i == a) {
                for (int b = 0;b < B[i];b++) {
                    for (int s = 0;s < B[i] - 1;s++) {
                        for (int u = 0;u < N;u++) {
                            for (int v = 0;v < N;v++) {
                                bw_cons += EDGE[a][a][b][b][s][s + 1][u][v] * arrival_SFC;
                            }
                        }
                    }
                }
            }
        }

        IloExpr obj(env);
        obj += param1 * ((B[a] * theta[0]) - (node_expr + edge_expr)) + (param2)*e2e_expr + (param2)*bw_cons;
        //model.add(obj >= 0);
        model.add(IloMinimize(env, obj));

#pragma endregion

#pragma region Constraints
        // setting constraint for the auxilary varaible theta
        for (int b = 0;b < B[a];b++) {
            IloExpr node_c(env);
            IloExpr edge_c(env);
            for (int s = 0;s < B[a];s++) {
                for (int f = 0;f < B[a];f++) {
                    for (int u = 0;u < N;u++) {
                        node_c += (t_f[SFC[a][f]] * NODE[a][b][s][SFC[a][f]][u]);
                    }
                }
            }
            for (int s = 0;s < B[a] - 1;s++) {
                //for (int s1 = s + 1;s1 < B[a];s1++) {
                //    for (int u = 0;u < N;u++) {
                //        for (int v = 0;v < N;v++) {
                //            edge_c += (t_uv[u][v] * EDGE[a][a][b][b][s][s1][u][v]);
                //        }
                //    }
                //}
                for (int u = 0;u < N;u++) {
                    for (int v = 0;v < N;v++) {
                        if (g[u][v]) {
                            edge_c += (t_uv[u][v] * EDGE[a][a][b][b][s][s + 1][u][v]);
                        }
                    }
                }
                //for (int u = 0;u < N;u++) {
                //    for (int v = 0;v < N;v++) {
                //        edge_c += (t_uv[u][v] * (EDGE[a - 1][a][0][b][0][s][u][v] + EDGE[a][a + 1][b][0][s][0][u][v]));
                //    }
                //}
            }
            for (int u = 0;u < N;u++) {
                for (int v = 0;v < N;v++) {
                    if (g[u][v]) {
                        edge_c += (t_uv[u][v] * (EDGE[a - 1][a][0][b][0][0][u][v] + EDGE[a][a + 1][b][0][B[a] - 1][0][u][v]));
                    }
                }
            }
            model.add(theta[0] >= (node_c + edge_c));
        }
        // ensures each slot in a branch has only one function picked
        for (int i = 0;i < SFC.size();i++) {
            for (int b = 0;b < B[i];b++) {
                for (int s = 0;s < B[i];s++) {
                    IloExpr c(env);
                    for (int f = 0;f < funcs + 2 + temp;f++) {
                        for (int u = 0;u < N;u++) {
                            c += NODE[i][b][s][f][u];
                        }
                    }
                    model.add(c == 1);
                }
            }
        }
        //ensures that total nodes picked in PE_i is same as the number of parallel functions
        for (int i = 0;i < SFC.size();i++) {
            IloExpr c(env);
            for (int b = 0;b < B[i];b++) {
                for (int s = 0;s < B[i];s++) {
                    for (int f = 0;f < funcs + 2 + temp;f++) {
                        for (int u = 0;u < N;u++) {
                            c += NODE[i][b][s][f][u];
                        }
                    }
                }
            }
            //model.add(c == SFC[i].size());
        }
        //ensures each function is picked atmost once
        for (int f = 0;f < funcs + 2 + temp;f++) {
            IloExpr c(env);
            for (int i = 0;i < SFC.size();i++) {
                for (int b = 0;b < B[i];b++) {
                    for (int s = 0;s < B[i];s++) {
                        for (int u = 0;u < N;u++) {
                            c += NODE[i][b][s][f][u];
                        }
                    }
                }
            }
            model.add(c <= 1);
        }
        //for (int i = 0;i < SFC.size();i++) {
        //    for (int f = 0;f < SFC[i].size();f++) {
        //        IloExpr c(env);
        //        for (int b = 0;b < B[i];b++) {
        //            for (int s = 0;s < B[i];s++) {
        //                for (int u = 0;u < N;u++) {
        //                    c += NODE[i][b][s][SFC[i][f]][u];
        //                }
        //            }
        //        }
        //        model.add(c == 1);
        //    }
        //}
        // ensures that only those functions are selected that belong to the respective PE_i  
        for (int i = 0;i < SFC.size();i++) {
            IloExpr c(env);
            vector<bool> fs(funcs + 2 + temp, false);
            for (int t = 0;t < SFC[i].size();t++) {
                fs[SFC[i][t]] = true;
            }
            for (int b = 0;b < B[i];b++) {
                for (int s = 0;s < B[i];s++) {
                    for (int f = 0;f < funcs + 2 + temp;f++) {
                        if (!fs[f]) {
                            for (int u = 0;u < N;u++) {
                                model.add(NODE[i][b][s][f][u] == 0);
                            }
                        }
                    }
                }
            }
        }
        //flow conservation within parallel branch
        for (int b = 0;b < B[a];b++) {
            for (int s = 0;s < B[a] - 1;s++) {
                for (int u = 0;u < N;u++) {
                    IloExpr c(env);
                    IloExpr c1(env);
                    for (int v = 0;v < N;v++) {
                        if (g[u][v] == 1) {
                            c += EDGE[a][a][b][b][s][s + 1][v][u];
                        }
                    }
                    for (int w = 0;w < N;w++) {
                        if (g[u][w] == 1) {
                            c1 += EDGE[a][a][b][b][s][s + 1][u][w];
                        }
                    }
                    IloExpr n1(env);
                    IloExpr n(env);
                    for (int f = 0;f < SFC[a].size();f++) {
                        n1 += NODE[a][b][s + 1][SFC[a][f]][u];
                        n += NODE[a][b][s][SFC[a][f]][u];
                    }
                    model.add((c - c1) - (n1 - n) == 0);
                    //model.add((c - c1) /* - (NODE[a][b][s + 1][SFC[a][f1]][u] - NODE[a][b][s][SFC[a][f]][u]) */ == 0);
                }

            }
        }
        //flow conservation between PEs
        for (int i = 0;i < SFC.size() - 1;i++) {
            for (int b = 0;b < B[i];b++) {
                for (int b1 = 0;b1 < B[i + 1];b1++) {
                    for (int u = 0;u < N;u++) {
                        IloExpr c(env);
                        IloExpr c1(env);
                        for (int v = 0;v < N;v++) {
                            if (g[u][v] == 1) {
                                c += EDGE[i][i + 1][b][b1][B[i] - 1][0][v][u];
                            }
                        }
                        for (int w = 0;w < N;w++) {
                            if (g[u][w] == 1) {
                                c1 += EDGE[i][i + 1][b][b1][B[i] - 1][0][u][w];
                            }
                        }
                        if (i == 0 && u == src) {
                            model.add(c == 0);
                            model.add(c1 == 1);
                        }
                        else if (i == SFC.size() - 2 and u == dest) {
                            model.add(c == 1);
                            model.add(c1 == 0);
                        }
                        else {
                            IloExpr n1(env);
                            IloExpr n(env);
                            for (int f1 = 0;f1 < SFC[i + 1].size();f1++) {
                                n1 += NODE[i + 1][b1][0][SFC[i + 1][f1]][u];
                            }
                            for (int f = 0;f < SFC[i].size();f++) {
                                n += NODE[i][b][B[i] - 1][SFC[i][f]][u];
                            }
                            model.add((c - c1) - (n1 - n) == 0);
                        }
                        //model.add((c - c1) /* - (NODE[i + 1][b1][0][SFC[i + 1][f1]][u] - NODE[i][b][B[i] - 1][SFC[i][f]][u]) */ == 0);
                    }


                }
            }
        }

        //BW constraints
        for (int u = 0;u < N;u++) {
            for (int v = 0;v < N;v++) {
                IloExpr bw_expr(env);

                //intra PE BW constraint
                for (int b = 0;b < B[a];b++) {
                    for (int s = 0;s < B[a] - 1;s++) {
                        bw_expr += (arrival_SFC * EDGE[a][a][b][b][s][s + 1][u][v]);
                    }
                }

                //inter PE BW constraint
                for (int i = 0;i < SFC.size() - 1;i++) {
                    for (int b = 0;b < B[i];b++) {
                        for (int b1 = 0;b1 < B[i + 1];b1++) {
                            bw_expr += (arrival_SFC * EDGE[i][i + 1][b][b1][B[i] - 1][0][u][v]);
                        }
                    }
                }

                model.add(bw_expr <= bw[u][v]);
            }
        }

        //node resource constraint
        for (int u = 0;u < N;u++) {
            for (int f = 0;f < funcs + 2 + temp;f++) {
                IloExpr cap_expr(env);
                for (int i = 0;i < SFC.size();i++) {
                    for (int b = 0;b < B[i];b++) {
                        for (int s = 0;s < B[i];s++) {
                            cap_expr += (arrival_SFC * NODE[i][b][s][f][u]);
                        }
                    }
                }
                model.add(cap_expr <= cap[u][f]);
            }
        }

        //ensuring function f on u is picked only if f is deployed on u
        for (int i = 0;i < SFC.size();i++) {
            for (int b = 0;b < B[i];b++) {
                for (int s = 0;s < B[i];s++) {
                    for (int f = 0;f < funcs + 2 + temp;f++) {
                        for (int u = 0;u < N;u++) {
                            model.add(NODE[i][b][s][f][u] <= dep[u][f]);
                        }
                    }
                }
            }
        }
        // ensuring edges are picked which exist in the topology
        for (int i = 0;i < SFC.size();i++) {
            for (int i1 = 0;i1 < SFC.size();i1++) {
                for (int b = 0;b < B[i];b++) {
                    for (int b1 = 0;b1 < B[i1];b1++) {
                        for (int s = 0;s < B[i];s++) {
                            for (int s1 = 0;s1 < B[i1];s1++) {
                                for (int u = 0;u < N;u++) {
                                    for (int v = 0;v < N;v++) {
                                        if (g[u][v] == 0) {
                                            model.add(EDGE[i][i1][b][b1][s][s1][u][v] == 0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0;i < SFC.size();i++) {
            for (int i1 = 0;i1 < SFC.size();i1++) {
                for (int b = 0;b < B[i];b++) {
                    for (int b1 = 0;b1 < B[i1];b1++) {
                        for (int s = 0;s < B[i];s++) {
                            for (int s1 = 0;s1 < B[i1];s1++) {
                                for (int u = 0;u < N;u++) {
                                    for (int v = 0;v < N;v++) {
                                        if (i1 != i && i1 != i + 1)
                                            model.add(EDGE[i][i1][b][b1][s][s1][u][v] == 0);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        //for (int i = SFC.size()-1;i >= 0;i--) {
        //    for (int i1 = i - 1;i1 >= 0;i1--) {
        //        for (int b = 0;b < B[i];b++) {
        //            for (int b1 = 0;b1 < B[i1];b1++) {
        //                for (int s = 0;s < B[i];s++) {
        //                    for (int s1 = 0;s1 < B[i1];s1++) {
        //                        for (int u = 0;u < N;u++) {
        //                            for (int v = 0;v < N;v++) {
        //                                model.add(EDGE[i][i1][b][b1][s][s1][u][v] == 0);
        //                            }
        //                        }
        //                    }
        //                }
        //            }
        //        }
        //    }
        //}
        for (int i = 0;i < SFC.size();i++) {
            for (int b = 0;b < B[i];b++) {
                for (int s = 0;s < B[i];s++) {
                    for (int u = 0;u < N;u++) {
                        for (int v = 0;v < N;v++) {
                            model.add(EDGE[i][i][b][b][s][s][u][v] == 0);
                        }
                    }
                }
            }
        }
        for (int i = 0;i < SFC.size();i++) {
            for (int b = 0;b < B[i];b++) {
                for (int b1 = 0;b1 < B[i];b1++) {
                    if (b1 != b) {
                        for (int s = 0;s < B[i];s++) {
                            for (int s1 = 0;s1 < B[i];s1++) {
                                for (int u = 0;u < N;u++) {
                                    for (int v = 0;v < N;v++) {
                                        model.add(EDGE[i][i][b][b1][s][s1][u][v] == 0);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        for (int i = 0;i < SFC.size() - 1;i++) {
            for (int b = 0;b < B[i];b++) {
                for (int b1 = 0;b1 < B[i + 1];b1++) {
                    for (int s = 0;s < B[i];s++) {
                        for (int s1 = 0;s1 < B[i + 1];s1++) {
                            for (int u = 0;u < N;u++) {
                                for (int v = 0;v < N;v++) {
                                    if (s != B[i] - 1 && s1 != 0)
                                        model.add(EDGE[i][i + 1][b][b1][s][s1][u][v] == 0);
                                }
                            }
                        }
                    }
                }
            }
        }
        for (int i = 0;i < SFC.size();i++) {
            for (int b = 0;b < B[i];b++) {
                for (int s = 0;s < B[i];s++) {
                    for (int s1 = 0;s1 < B[i];s1++) {
                        for (int u = 0;u < N;u++) {
                            for (int v = 0;v < N;v++) {
                                if (s1 != s && s1 != s + 1)
                                    model.add(EDGE[i][i][b][b][s][s1][u][v] == 0);
                            }
                        }
                    }
                }
            }
        }
        // e2e latency constraint
        model.add(e2e_expr <= T_l);
#pragma endregion
        IloCplex cplex(model);
        cplex.setParam(IloCplex::RootAlg, IloCplex::MIP);
        cplex.setOut(env.getNullStream());
        cout << "MIP:" << cplex.isMIP() << endl;
        cout << "QO:" << cplex.isQO() << endl;
        cout << "QC:" << cplex.isQC() << endl;
        //if (!cplex.solve()) {
        //    env.error() << "Failed to optimize the Master Problem!!!" << endl;
        //    throw(-1);
        //}
        if(cplex.solve() == IloTrue) {
            double Objval = cplex.getObjValue();
            std::cout << "Objective::::" << Objval << endl;
            vector<int> ctr(SFC.size(), 0);
            for (int i = 0;i < SFC.size();i++) {
                for (int b = 0;b < B[i];b++) {
                    for (int s = 0;s < B[i];s++) {
                        for (int f = 0;f < funcs + 2 + temp;f++) {
                            for (int u = 0;u < N;u++) {
                                double NODEval = cplex.getValue(NODE[i][b][s][f][u]);
                                if (NODEval == 1) {
                                    ctr[i]++;
                                    std::cout << "(" << i << "," << b << "," << s << "," << f << "," << u << ": " << NODEval << endl;
                                }
                            }
                        }
                    }
                }
            }
            for (int i = 0;i < ctr.size();i++) {
                std::cout << i << ": " << ctr[i] << endl;
            }
            std::cout << "Theta: " << B[a] * cplex.getValue(theta[0]) << endl;
            double e = 0;
            for (int b = 0;b < B[a];b++) {
                for (int s = 0;s < B[a];s++) {
                    for (int f = 0;f < funcs + 2 + temp;f++) {
                        for (int u = 0;u < N;u++) {
                            e += (t_f[f] * cplex.getValue(NODE[a][b][s][f][u]));
                        }
                    }
                }
                for (int s = 0;s < B[a] - 1;s++) {
                    //for (int s1 = s+1;s1 < B[a];s1++) {
                    //    for (int u = 0;u < N;u++) {
                    //        for (int v = 0;v < N;v++) {
                    //            edge_expr += (t_uv[u][v] * EDGE[a][a][b][b][s][s1][u][v]);
                    //        }
                    //    }
                    //}
                    for (int u = 0;u < N;u++) {
                        for (int v = 0;v < N;v++) {
                            e += (t_uv[u][v] * cplex.getValue(EDGE[a][a][b][b][s][s + 1][u][v]));
                        }
                    }
                    //for (int u = 0;u < N;u++) {
                    //    for (int v = 0;v < N;v++) {
                    //        e += (t_uv[u][v] * (cplex.getValue(EDGE[a - 1][a][0][b][0][s][u][v]) + cplex.getValue(EDGE[a][a + 1][b][0][s][0][u][v])));
                    //    }
                    //}
                }
                for (int u = 0;u < N;u++) {
                    for (int v = 0;v < N;v++) {
                        e += (t_uv[u][v] * (cplex.getValue(EDGE[a - 1][a][0][b][0][0][u][v]) + cplex.getValue(EDGE[a][a + 1][b][0][B[a] - 1][0][u][v])));
                    }
                }
            }
            std::cout << "Diff: " << e << endl;

            std::cout << "Final e2e latency::::::" << cplex.getValue(e2e_expr) << endl;

            for (int i = 0;i < SFC.size();i++) {
                for (int i1 = 0;i1 < SFC.size();i1++) {
                    for (int b = 0;b < B[i];b++) {
                        for (int b1 = 0;b1 < B[i1];b1++) {
                            for (int s = 0;s < B[i];s++) {
                                for (int s1 = 0;s1 < B[i1];s1++) {
                                    for (int u = 0;u < N;u++) {
                                        for (int v = 0;v < N;v++) {
                                            try {
                                                if (cplex.getValue(EDGE[i][i1][b][b1][s][s1][u][v])) {
                                                    std::cout << "(" << i << "," << i1 << "," << b << "," << b1 << "," << s << "," << s1 << "," << u << "," << v << ") " << cplex.getValue(EDGE[i][i1][b][b1][s][s1][u][v]) << endl;
                                                }
                                            }
                                            catch (IloException e) {
                                                continue;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            //for (int i = 0;i < SFC.size() - 1;i++) {
            //    for (int b = 0;b < B[i];b++) {
            //        for (int b1 = 0;b1 < B[i + 1];b1++) {
            //            for (int f = 0;f < SFC[i].size();f++) {
            //                for (int f1 = 0;f1 < SFC[i + 1].size();f1++) {
            //                    for (int u = 0;u < N;u++) {
            //                        IloExpr c(env);
            //                        IloExpr c1(env);
            //                        for (int v = 0;v < N;v++) {
            //                            if (g[u][v] == 1) {
            //                                cout << "(" << i << "," << i+1 << "," << b << "," << b1 << "," << B[i] - 1 << "," << 0 << "," << u << "," << v << ") " << cplex.getValue(EDGE[i][i + 1][b][b1][B[i] - 1][0][v][u]) << endl;
            //                            }
            //                        }
            //                    }
            //                }
            //            }
            //        }
            //    }
            //}
            //network update
                //VM cap update
            for (int i = 0;i < SFC.size();i++) {
                for (int b = 0;b < B[i];b++) {
                    for (int s = 0;s < B[i];s++) {
                        for (int f = 0;f < funcs + 2 + temp;f++) {
                            for (int u = 0;u < N;u++) {
                                double NODEval = cplex.getValue(NODE[i][b][s][f][u]);
                                if (NODEval == 1) {
                                    cap[u][f] -= arrival_SFC;
                                }
                            }
                        }
                    }
                }
            }
            // BW update
            for (int i = 0;i < SFC.size();i++) {
                for (int i1 = 0;i1 < SFC.size();i1++) {
                    for (int b = 0;b < B[i];b++) {
                        for (int b1 = 0;b1 < B[i1];b1++) {
                            for (int s = 0;s < B[i];s++) {
                                for (int s1 = 0;s1 < B[i1];s1++) {
                                    for (int u = 0;u < N;u++) {
                                        for (int v = 0;v < N;v++) {
                                            try {
                                                if (cplex.getValue(EDGE[i][i1][b][b1][s][s1][u][v])) {
                                                    bw[u][v] -= arrival_SFC;
                                                    bw[v][u] -= arrival_SFC;
                                                }
                                            }
                                            catch (IloException e) {
                                                continue;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            for (int u = 0;u < N;u++) {
                for (int i = 0;i < temp;i++) {
                    cap[u].pop_back();
                }
            }
            result.mean_latency += cplex.getValue(e2e_expr);
            result.mean_PD += (B[a] * cplex.getValue(theta[0]) - e);
            return true;
    }
        else{
            for (int u = 0;u < N;u++) {
                for (int i = 0;i < temp;i++) {
                    cap[u].pop_back();
                }
            }
            return false;
        }
    }
    catch (IloException& e) {
        for (int u = 0;u < N;u++) {
            for (int i = 0;i < temp;i++) {
                cap[u].pop_back();
            }
        }
        return false;
    }
    catch (...) {
        cerr << "Unknown exception caught" << endl;
        for (int u = 0;u < N;u++) {
            for (int i = 0;i < temp;i++) {
                cap[u].pop_back();
            }
        }
        return false;
    }

    env.end();
}

void print(std::vector <int> const& a) {
    std::cout << "The vector elements are : ";

    for (int i = 0; i < a.size(); i++)
        std::cout << a.at(i) << ' ';
}

// Structure to represent a min heap node 
struct MinHeapNode
{
    int  v;
    float dist;
};

// Structure to represent a min heap 
struct MinHeap
{
    int size;      // Number of heap nodes present currently 
    int capacity;  // Capacity of min heap 
    int* pos;     // This is needed for decreaseKey() 
    struct MinHeapNode** array;
};

// A utility function to create a new Min Heap Node 
struct MinHeapNode* newMinHeapNode(int v, float dist)
{
    struct MinHeapNode* minHeapNode = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
    minHeapNode->v = v;
    minHeapNode->dist = dist;
    return minHeapNode;
}

// A utility function to create a Min Heap 
struct MinHeap* createMinHeap(int capacity)
{
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->pos = (int*)malloc(capacity * sizeof(int));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array =
        (struct MinHeapNode**)malloc(capacity * sizeof(struct MinHeapNode*));
    return minHeap;
}

void deleteHeap(struct MinHeap*& minHeap, int capacity) {
    //for(int i=0;i<capacity;i++){
    free(minHeap->pos);
    //}
    //for(int i=0;i<capacity;i++){
    free(minHeap->array);
    //}
}

// A utility function to swap two nodes of min heap. Needed for min heapify 
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b)
{
    struct MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

// A standard function to heapify at given idx 
// This function also updates position of nodes when they are swapped. 
// Position is needed for decreaseKey() 
void minHeapify(struct MinHeap*& minHeap, int idx)
{
    int smallest, left, right;
    smallest = idx;
    left = 2 * idx + 1;
    right = 2 * idx + 2;

    if (left < minHeap->size &&
        minHeap->array[left]->dist < minHeap->array[smallest]->dist)
        smallest = left;

    if (right < minHeap->size &&
        minHeap->array[right]->dist < minHeap->array[smallest]->dist)
        smallest = right;

    if (smallest != idx)
    {
        // The nodes to be swapped in min heap 
        MinHeapNode* smallestNode = minHeap->array[smallest];
        MinHeapNode* idxNode = minHeap->array[idx];

        // Swap positions 
        minHeap->pos[smallestNode->v] = idx;
        minHeap->pos[idxNode->v] = smallest;

        // Swap nodes 
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);

        minHeapify(minHeap, smallest);
    }
}

// A utility function to check if the given minHeap is ampty or not 
int isEmpty(struct MinHeap*& minHeap)
{
    return minHeap->size == 0;
}

// Standard function to extract minimum node from heap 
struct MinHeapNode* extractMin(struct MinHeap*& minHeap)
{
    if (isEmpty(minHeap))
        return NULL;

    // Store the root node 
    struct MinHeapNode* root = minHeap->array[0];

    // Replace root node with last node 
    struct MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;

    // Update position of last node 
    minHeap->pos[root->v] = minHeap->size - 1;
    minHeap->pos[lastNode->v] = 0;

    // Reduce heap size and heapify root 
    --minHeap->size;
    minHeapify(minHeap, 0);

    return root;
}

// Function to decreasy dist value of a given vertex v. This function 
// uses pos[] of min heap to get the current index of node in min heap 
void decreaseKey(struct MinHeap*& minHeap, int v, float dist)
{
    // Get the index of v in  heap array 
    int i = minHeap->pos[v];

    // Get the node and update its dist value 
    minHeap->array[i]->dist = dist;

    // Travel up while the complete tree is not hepified. 
    // This is a O(Logn) loop 
    while (i && minHeap->array[i]->dist < minHeap->array[(i - 1) / 2]->dist)
    {
        // Swap this node with its parent 
        minHeap->pos[minHeap->array[i]->v] = (i - 1) / 2;
        minHeap->pos[minHeap->array[(i - 1) / 2]->v] = i;
        swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);

        // move to parent index 
        i = (i - 1) / 2;
    }
}

// A utility function to check if a given vertex 
// 'v' is in min heap or not 
bool isInMinHeap(struct MinHeap*& minHeap, int v)
{
    if (minHeap->pos[v] < minHeap->size)
        return true;
    return false;
}

ofstream w("test.txt");
ofstream s("status.txt");
double add_links(vector<vector<node>>& g, vector<int>& path, vector<vector<vector<int>>>& SFC, vector<node_capacity>& n_r, int source, int destination) { //calculates the link delay in a given path in the graph
    // cout<<"h\n";
    double time = 0;
    if (path.size() == 0)
        return time;
    // cout<<"he\n";
    int src = path[0];
    if (path.size() > 10000) {
        w << "Path Size::::::" << path.size() << endl;
        w << "Path::::::" << path[0] << endl;
        for (int i = 0;i < path.size();i++) {
            w << path[i] << "->";
        }
        w << endl;
        w << endl;
        w << endl;
        w << "End soter" << endl;
    }
    // cout<<"hel\n";
    // cout<<"hell\n";
    for (int i = 1;i < path.size();i++) {
        if (path[i - 1] < 0 || path[i - 1] > 7) {
            cout << "Errorrrrrr\n";
            s << "Graph\n";
            for (int j = 0;j < g.size();j++) {
                for (int k = 0;k < g[j].size();k++) {
                    s << j << "->" << g[j][k].id << "(" << g[j][k].link << ")" << "(" << g[j][k].available_bandwidth << ")" << endl;
                }
            }

            s << "SFC\n";
            for (int l = 0;l < SFC.size();l++) {
                for (int j = 0;j < SFC[l].size();j++) {
                    for (int k = 0;k < SFC[l][j].size();k++) {
                        cout << SFC[l][j][k];
                    }
                    cout << "-->";
                }
                cout << endl;
            }

            cout << "Node to funcs" << endl;
            for (int l = 0;l < g.size();l++) {
                cout << l << ":" << endl;
                map<int, double>::iterator it;
                for (it = n_r[l].NF_left.begin();it != n_r[l].NF_left.end();it++) {
                    cout << it->first << ":" << it->second << endl;
                }
                cout << endl;
            }
            cout << "SRC::::" << source << endl;
            cout << "DEST:::" << destination << endl;

        }
        for (auto x : g[path[i - 1]]) {
            if (x.id == path[i] && path[i - 1] == path[i])
                continue;
            if (x.id == path[i]) {
                time += x.link;
            }
        }
    }
    // cout<<"hello\n";
    return time;
}

double add_links(vector<vector<node>>& g, vector<int>& path) { //calculates the link delay in a given path in the graph
    double time = 0;
    if (path.size() == 0)
        return 0;
    int src = path[0];
    if (path.size() > 10000) {
        for (int i = 0;i < path.size();i++) {
            w << path[i] << "->";
        }
        w << endl;
        w << endl;
        w << endl;
        w << "Enddd" << endl;
    }
    for (int i = 1;i < path.size();i++) {
        if (path[i - 1] < 0 || path[i - 1] > 7) {
            cout << "Errorrrrrr\n";
        }
        for (auto x : g[path[i - 1]]) {
            if (x.id == path[i] && path[i - 1] == path[i])
                continue;
            if (x.id == path[i]) {
                time += x.link;
            }
        }
    }
    return time;
}
vector<vector<vector<double>>> calc_time(vector<vector<node>>& g, vector<vector<vector<vector<int>>>>& paths) { //calculates the time taken to traverse each of the k shortest path for all src-dest pairs in graph
    vector<vector<vector<double>>> times(paths.size());
    for (int i = 0;i < paths.size();i++) {
        times[i].resize(paths[i].size());
        for (int j = 0;j < paths[i].size();j++) {
            times[i][j].resize(paths[i][j].size());
            for (int k = 0;k < paths[i][j].size();k++) {
                times[i][j][k] = add_links(g, paths[i][j][k]);
            }
        }
    }
    return times;
}

// The main function that calulates distances of shortest paths from src to all 
// vertices. It is a O(ELogV) function 
vector <int> dijkstra(int src, int dest, vector<vector<node>>& graph, int throughput)
{

    // request has source, destination, NF{vector<pair<int, int>>}, throughput, delay 
    int V = graph.size();// Get the number of vertices in graph 
    //float dist[V];      // dist values used to pick minimum weight edge in cut 
    vector<double> dist;
    dist.resize(V);
    //int src = request.source;
    //int dest = request.destination;
    //int throughput = request.throughput;
    vector<vector<int>> paths;
    paths.resize(V);

    // minHeap represents set E 
    struct MinHeap* minHeap = createMinHeap(V);
    // Initialize min heap with all vertices. dist value of all vertices  
    for (int v = 0; v < V; ++v)
    {
        dist[v] = FLT_MAX;
        minHeap->array[v] = newMinHeapNode(v, dist[v]);
        minHeap->pos[v] = v;
    }
    // Make dist value of src vertex as 0 so that it is extracted first 
    minHeap->array[src] = newMinHeapNode(src, dist[src]);
    paths[src].push_back(src);
    minHeap->pos[src] = src;
    dist[src] = 0;
    decreaseKey(minHeap, src, dist[src]);

    // Initially size of min heap is equal to V
    minHeap->size = V;

    // In the followin loop, min heap contains all nodes 
    // whose shortest distance is not yet finalized. 
    // cout<<"Calc dist\n";
    while (!isEmpty(minHeap))
    {
        // Extract the vertex with minimum distance value 
        struct MinHeapNode* minHeapNode = extractMin(minHeap);
        int u = minHeapNode->v; // Store the extracted vertex number 
        // Traverse through all adjacent vertices of u (the extracted 
        // vertex) and update their distance values  
        for (auto pCrawl : graph[u])
        {
            int v = pCrawl.id;

            // If shortest distance to v is not finalized yet, and distance to v 
            // through u is less than its previously calculated distance and u-v link throughput is not a bottleneck for us
            if (isInMinHeap(minHeap, v) && dist[u] != FLT_MAX &&
                pCrawl.link + dist[u] < dist[v] && pCrawl.available_bandwidth >= throughput)
            {
                paths[v].clear(); // clear the path to the destination node, add the nodes from the node just visited
                for (auto& vertex : paths[u])
                    paths[v].push_back(vertex);
                paths[v].push_back(v);
                dist[v] = dist[u] + pCrawl.link;
                // //cout<<"delay is "<<pCrawl.delay<<endl;
                // update distance value in min heap also 
                decreaseKey(minHeap, v, dist[v]);
            }
            // if destination is finalized, we are done!
            else if (v == dest && !isInMinHeap(minHeap, v))
                break;
        }
    }

    // print the calculated shortest distances 
    // //cout<<"The distance from source "<<src<<" to destination "<<dest<<" is: "<<dist[dest]<<endl;
    // //cout<<"The path of the shortest distance is: ";
    // print the shortest distance

    // for(auto &vertex: paths[dest])
    //     //cout<<vertex<<" ";
    // //cout<<endl;
    // float vnf_delay = compute_vnf_delay(request);

    // if(dist[dest]+vnf_delay>request.delay)
    //     paths[dest].clear();

    vector <int> selected_path;
    // if(dist[dest]!=FLT_MAX)
    //    selected_path.delay = dist[dest] + vnf_delay;
    // else
    //     selected_path.delay = dist[dest];
    if (paths[dest].size() != 0)
        selected_path = paths[dest];

    deleteHeap(minHeap, V);
    return selected_path;
}

// const int INF = std::numeric_limits<int>::max();

// // a struct to represent a vertex and its distance from the source vertex
// struct Vertex {
//     int id;
//     int distance;
//     std::vector<int> path;
//     bool operator<(const Vertex& other) const {
//         return distance > other.distance;
//     }
// };

// // function to run Dijkstra's algorithm
// std::vector<int> dijkstra(int source, int dest,std::vector<std::vector<node>>& graph,int throughput) {
//     int n = graph.size();
//     std::vector<int> distance(n, INF);
//     std::vector<bool> visited(n, false);
//     std::priority_queue<Vertex> pq;

//     // set the distance to the source vertex to zero and add it to the priority queue
//     distance[source] = 0;
//     pq.push({source, 0, {source}});

//     while (!pq.empty()) {
//         Vertex curr = pq.top();
//         pq.pop();

//         // if we've already visited this vertex, continue to the next one
//         if (visited[curr.id]) {
//             continue;
//         }

//         // mark the current vertex as visited
//         visited[curr.id] = true;

//         // if we've reached the destination vertex, return its path
//         if (curr.id == dest) {
//             return curr.path;
//         }

//         // update the distances and paths to the neighboring vertices
//         for (auto i: graph[curr.id]) {
//             int dist = curr.distance + graph[curr.id][i.id].link;
//             if (dist < distance[i.id] && graph[curr.id][i.id].available_bandwidth >= throughput) {
//                 distance[i.id] = dist;
//                 std::vector<int> path = curr.path;
//                 path.push_back(i.id);
//                 pq.push({i.id, dist, path});
//             }
//         }
//     }

//     // if we couldn't reach the destination vertex, return an empty path
//     return {};
// }

struct comparePaths {
    vector<vector<node>> g;
    comparePaths(vector<vector<node>> g) {
        this->g = g;
    }
    bool operator() (vector<int>& P1, vector<int>& P2) {
        return add_links(this->g, P1) <= add_links(this->g, P2);
    }
};

vector<vector<int>> YenKSP(vector<vector<node>>& graph, int s, int d, Request request, int K)
{
    int src = s;
    int dest = d;
    vector<vector<int>> A(K + 1);
    int throughput = request.t_arrival_rate;
    // cout<<s<<" "<<d<<endl;
    A[0] = dijkstra(src, dest, graph, throughput);
    if (A[0].size() == 0)
    {
        vector<vector<int>> empty;
        return empty;
    }
    //struct k_shortest temp;
    //temp.path.push_back(A[0]);
    //return temp;
    // cout<<"Here1\n";
    vector <vector<int>> B;
    for (int k = 1;k <= K;k++)
    {
        //cout<<"Im here"<<endl;
        for (int i = 0;i < A[k - 1].size();i++)
        {
            int spurNode = A[k - 1][i];
            //cout<<spurNode<<endl;
            vector <int> rootpath(A[k - 1].begin(), A[k - 1].begin() + i);
            vector <int> removepath(A[k - 1].begin() + i, A[k - 1].end());
            //cout<<"Root path"<<endl;
            //print(rootpath);
            //cout<<endl;
            //cout<<"Remove path "<<endl;
            //print(removepath);
            //cout<<endl;
            vector <vector<node>> graph_copy = graph;
            for (int j = 0;j < removepath.size() - 1;j++)
            {
                int node1 = removepath[j];
                int node2 = removepath[j + 1];
                for (auto a = graph_copy[node1].begin();a != graph_copy[node1].end();a++)
                {
                    if ((*a).id == node2)
                    {
                        graph_copy[node1].erase(a);
                        break;
                    }
                }
                for (auto a = graph_copy[node2].begin();a != graph_copy[node2].end();a++)
                {
                    if ((*a).id == node1)
                    {
                        graph_copy[node2].erase(a);
                        break;
                    }
                }
                //if(i==spurNode)
                //continue;
                // else{
                //     for(int j=0;j<graph_copy.size();j++)
                //     {
                //         for(auto a = graph_copy[j].begin();a!=graph_copy[j].end();a++)
                //         {
                //             if((*a).node2==i)
                //             {
                //                 graph_copy[j].erase(a);
                //                 break;
                //             }
                //         }
                //     }
                // }

            }
            for (int l = 0;l < rootpath.size();l++)
            {
                int node1 = rootpath[l];
                for (auto j = graph_copy[node1].begin();j != graph_copy[node1].end();j++)
                {
                    int node2 = (*j).id;
                    for (auto a = graph_copy[node2].begin();a != graph_copy[node2].end();a++)
                    {
                        if ((*a).id == node1)
                        {
                            graph_copy[node2].erase(a);
                            break;
                        }
                    }
                }
                // for(auto j = graph_copy[node1].begin();j!=graph_copy[node1].end();j++)
                // graph_copy[node1].erase(j);
                graph_copy[node1].clear();

            }
            int n = graph_copy.size();

            // for(int i=0;i<n;i++)
            // {
            //     cout<<"Node "<<i<<" ";
            //     for(auto j=graph_copy[i].begin();j!=graph_copy
            //     [i].end();j++)
            //     {
            //         cout<<(*j).node2<<" ";

            //     }
            //     cout<<endl;
            // }
            vector<int> spurPath;
            spurPath = dijkstra(spurNode, dest, graph_copy, throughput);
            //print(spurPath);
            vector <int> totalPath;
            totalPath.insert(totalPath.end(), rootpath.begin(), rootpath.end());
            totalPath.insert(totalPath.end(), spurPath.begin(), spurPath.end());

            int flag = 0;
            //cout<<"totalPath"<<endl;
            //print(totalPath);
            //cout<<endl;
            if (totalPath.size() != 0) {
                if (totalPath[totalPath.size() - 1] != dest)
                    flag = 1;
                for (int j = 0;j < B.size();j++)
                {
                    if (B[j] == totalPath)
                        flag = 1;
                }
                for (int j = 0;j < k;j++)
                {
                    if (A[j] == totalPath)
                        flag = 1;
                }
            }
            if (totalPath.size() == 0)
                flag = 1;
            if (flag == 0)
                B.push_back(totalPath);
        }
        if (B.size() == 0) {
            break;
        }
        //B.sort();
        // vector <int> temp;
        // for(int i=0;i<B.size();i++)
        // {
        //     int sum=0;
        //     for(int j=0;j<B[i].size();j++)
        //     sum+=B[i][j];
        //     temp.push_back(sum);
        // }

        // cout<<"Next Path"<<endl;
        // for(int i=0;i<B.size();i++)
        // {
        //     for(int j=0;j<B[i].size();j++)
        //     cout<<B[i][j]<<" ";
        //     cout<<endl;
        // }
        sort(B.begin(), B.end(), comparePaths(graph));
        A[k] = B[0];
        B.erase(B.begin());
    }
    //sort(B.begin(),B.end(),comparePaths);
    // cout<<"Printing B"<<endl;
    // for(int i=0;i<B.size();i++)
    // {
    //     //cout<<"Im here"<<endl;
    //     print(B[i]);
    //     cout<<endl;
    // }
    vector<vector<int>> temp;
    //cout<<"Here\n";
    // if(B.size() != 0){
    //     for(int i=1;i<K;i++){
    //         cout<<B[i].size()<<endl;
    //         A[i] = B[i];
    //     }
    // }
    // cout<<"Here2\n";
    for (int i = 0;i < K;i++)
    {
        if (A[i].size() != 0)
            temp.push_back(A[i]);
        // for(int j=0;j<A[i].size();j++){
        //     cout<<A[i][j]<<"->";
        // }
        // cout<<endl;
    }
    //cout<<"Test:::::::::::::::::::"<<K<<"   "<<temp.size()<<endl;
    B.clear();
    sort(temp.begin(), temp.end(), comparePaths(graph));
    // cout<<"Here3\n";
    //cout<<"K::::::::::"<<temp.size()<<endl;
    return temp;

}

struct info {
public:
    vector<vector<int>> paths;
    vector<vector<int>> dep;
    info(vector<vector<int>> paths, vector<vector<int>> dep) {
        this->paths = paths;
        this->dep = dep;
    }
};

vector<int> parse_id(string s) {
    vector<int> ids;
    string temp = "";
    for (int i = 0;i < s.size();i++) {
        if (s[i] == ';') {
            ids.push_back(stoi(temp));
            temp = "";
        }
        else {
            temp += s[i];
        }
    }
    ids.push_back(stoi(temp));
    return ids;
}
int cc = 0;
void dfs(string u, string d, int lev, map<string, vector<inst_node>>& layer_g, vector<vector<vector<vector<int>>>>& paths, vector<vector<int>>& dep, vector<int> d_temp, vector<vector<int>>& res, vector<int> temp) {

    if (u == d) {
        if (temp.size() == 0)
            return;
        res.push_back(temp);
        dep.push_back(d_temp);
        if (temp.size() > 10000) {
            s << "Error:::::dfs path size" << u << " " << d << endl;
            map<string, vector<inst_node>>::iterator it;
            for (it = layer_g.begin();it != layer_g.end();it++) {
                s << it->first << ":\n";
                for (int j = 0;j < it->second.size();j++) {
                    inst_node t = it->second[j];
                    for (int i = 0;i < t.links.size();i++) {
                        s << " " << t.id << "(" << t.links[i] << ")\n";
                    }
                    s << endl;
                }
            }
            exit(0);
        }
        for (int i = 0;i < temp.size();i++) {
            if (temp[i] < 0 || temp[i]>7) {
                s << "Error::::dfs\n";
                map<string, vector<inst_node>>::iterator it;
                for (it = layer_g.begin();it != layer_g.end();it++) {
                    s << it->first << ":\n";
                    for (int j = 0;j < it->second.size();j++) {
                        inst_node t = it->second[j];
                        for (int i = 0;i < t.links.size();i++) {
                            s << " " << t.id << "(" << t.links[i] << ")\n";
                        }
                        s << endl;
                    }
                }
                exit(0);
            }
        }
        cc = 0;
        return;
    }
    else // If current vertex is not destination
    {
        // Recur for all the vertices adjacent to current
        // vertex
        vector<inst_node>::iterator i;
        for (i = layer_g[u].begin(); i != layer_g[u].end(); i++) {
            for (int l = 0;l < i->links.size();l++) {
                vector<int> temp2 = temp;
                vector<int> d_temp2 = d_temp;
                vector<int> src_id = parse_id(u);
                vector<int> dest_id = parse_id(i->id);
                d_temp2.push_back(dest_id.back());
                if (temp2.size() != 0 && paths[src_id.back()][dest_id.back()][l].begin() + 1 != paths[src_id.back()][dest_id.back()][l].end()) {
                    temp2.insert(temp2.end(), paths[src_id.back()][dest_id.back()][l].begin() + 1, paths[src_id.back()][dest_id.back()][l].end());
                }
                else
                    temp2.insert(temp2.end(), paths[src_id.back()][dest_id.back()][l].begin(), paths[src_id.back()][dest_id.back()][l].end());
                dfs(i->id, d, lev + 1, layer_g, paths, dep, d_temp2, res, temp2);
                cc++;
            }
        }
    }

}

info get_all_paths(string s, string d, map<string, vector<inst_node>>& layer_g, vector<vector<vector<vector<int>>>>& paths, vector<vector<vector<double>>>& time_of_paths) {
    vector<vector<int>> res;
    vector<int> temp;
    vector<vector<int>> dep;
    vector<int> d_temp;
    vector<int> ids = parse_id(s);
    int lev = 0;
    cout << "source::" << s << " " << "Des::" << d << endl;
    dfs(s, d, lev, layer_g, paths, dep, d_temp, res, temp);
    cout << "Size:::::::::::" << res.size() << ":::" << cc << endl;
    info i(res, dep);
    return i;
}

class comp {
public:
    double time;
    vector<vector<node>> g;
    Request request;
    vector<node_capacity> n_r;
    comp(vector<vector<node>>& g, double time, Request request, vector<node_capacity> n_r) {
        this->g = g;
        this->time = time;
        this->request = request;
        this->n_r = n_r;
    }
    bool operator()(pair<vector<int>, vector<int>>& a, pair<vector<int>, vector<int>>& b) {
        // cout<<"Test1\n";
        double time_a = add_links(g, a.first, request.SFC, n_r, request.src, request.dest);
        // cout<<"Test1.5\n";
        double time_b = add_links(g, b.first, request.SFC, n_r, request.src, request.dest);
        // cout<<"Test2\n";
        if (abs(time_a - time) == abs(time_b - time)) {
            // cout<<"Test3\n";
            return (time_a - time) < (time_b - time);
        }
        // cout<<"Test4\n";
        return abs(time_a - time) < abs(time_b - time);
    }
};

void update_BW(vector<vector<node>>& g, vector<int> path, double arrival) {
    if (path.size() <= 1)
        return;
    int prev, curr;
    prev = path[0];

    // for(int i=0;i<path.size();i++){
    //     cout<<path[i]<<"->";
    // }
    // cout<<endl;

    for (int i = 1;i < path.size();i++) {
        curr = path[i];
        int j = 0;
        while (j < g[prev].size() && g[prev][j].id != curr)
            j++;
        g[prev][j].available_bandwidth -= arrival;
        j = 0;
        while (j < g[curr].size() && g[curr][j].id != prev)
            j++;
        if (j != g[curr].size())
            g[curr][j].available_bandwidth -= arrival;
        prev = curr;
    }
}

bool is_available_BW(vector<vector<node>>& g, vector<int> path, double arrival) {
    if (path.size() <= 1)
        return true;
    int prev, curr;
    prev = path[0];
    for (int i = 1;i < path.size();i++) {
        curr = path[i];
        int j = 0;
        while (j < g[prev].size() && g[prev][j].id != curr)
            j++;
        if (g[prev][j].available_bandwidth < arrival) {
            return false;
        }
        prev = curr;
    }
    return true;
}

void update_resource(vector<node_capacity>& n_resource, vector<int> funcs, vector<int> dep, double arrival) {
    for (int i = 0;i < dep.size() - 1;i++) {
        n_resource[dep[i]].NF_left[funcs[i]] -= arrival;
    }
}

bool is_available_resource(vector<node_capacity>& n_resource, vector<int> funcs, vector<int> dep, double arrival) {
    for (int i = 0;i < dep.size() - 1;i++) {
        if (n_resource[dep[i]].NF_left[funcs[i]] < arrival) {
            return false;
        }
    }
    return true;
}

bool layer_graph_2(int src, vector<int> funcs, int dest, vector<vector<node>>& g, map<int, double>& time, map<int, int>& deployed_inst,
    vector<vector<int>>& NF_to_node, map<int, double>& NFs, vector<node_capacity>& n_resource, vector<vector<vector<vector<int>>>>& paths,
    vector<vector<vector<double>>>& time_of_paths, Request request, Result& result, double diff, double& max_tt) {
    cout << "L_Fnc3\n";
    double temp_diff = diff;
    for (int i = 0;i < funcs.size();i++) {
        diff -= NFs[funcs[i]];
    }
    // int a;
    // for(int i=0;i<request.SFC.size();i++){
    //     if(request.SFC[i].size() > 1){
    //         a = i;
    //         break;
    //     }
    // }
    // if(diff < 0){
    //     for(int i=0;i<request.SFC.size();i++){
    //         for(int j=0;j<request.SFC[i].size();j++){
    //             for(int k=0;k<request.SFC[i][j].size();k++){
    //                 s<<i<<","<<j<<","<<k<<":::"<<time[request.SFC[i][j][k]]<<endl;
    //             }
    //         }
    //     }
    //     if(a != request.SFC.size()-1)
    //         s<<time[request.SFC[a+1][request.SFC[a+1].size()-1][0]]<<"\n"<<time[request.SFC[a-1][request.SFC[a-1].size()-1][0]]<<"\n"<<NFs[request.SFC[a+1][request.SFC[a+1].size()-1][0]]<<"\n"<<(request.SFC[a].size()-1)<<endl;
    //     s<<"INIT_DIFF::::::"<<temp_diff<<endl;
    //     s<<"FINAL_DIFF::::::"<<diff<<endl;
    //     s<<"DIFF::::Graph\n";
    //     for(int j=0;j<g.size();j++){
    //         for(int k=0;k<g[j].size();k++){
    //             s<<j<<"->"<<g[j][k].id<<"("<<g[j][k].link<<")"<<"("<<g[j][k].available_bandwidth<<")"<<endl;
    //         }
    //     }

    //     cout<<"Node to funcs"<<endl;
    //     for(int l=0;l<g.size();l++){
    //         cout<<l<<":"<<endl;
    //         map<int,double>::iterator it;
    //         for(it=n_resource[l].NF_left.begin();it!=n_resource[l].NF_left.end();it++){
    //             cout<<it->first<<":"<<it->second<<endl;
    //         }
    //         cout<<endl;
    //     }
    //     map<int,double>::iterator it;
    //     for(it = NFs.begin();it!=NFs.end();it++){
    //         s<<it->first<<": "<<it->second<<endl;
    //     }
    //     for(int l=0;l<request.SFC.size();l++){
    //         for(int j=0;j<request.SFC[l].size();j++){
    //             for(int k=0;k<request.SFC[l][j].size();k++){
    //                 s<<"("<<request.SFC[l][j][k]<<")";
    //             }
    //             s<<"-->";
    //         }
    //         s<<endl;
    //     }

    //     exit(0);
    // }
    // cout<<"FINAL_DIFF::::::"<<diff<<endl;
    map<string, vector<inst_node>> layer_g;
    queue<string> q;
    vector<int> nodes(10, 0);
    int cnt = 1;
    //forming the multi graph
    if (NF_to_node[funcs[0]].size() == 0)
        return false;
    for (int j = 0;j < NF_to_node[funcs[0]].size();j++) {
        int id = funcs[0];
        int node_id = NF_to_node[funcs[0]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(node_id);

        nodes[cnt]++;

        string source = to_string(cnt - 1) + ";" + to_string(src);
        if (paths[src][node_id].size() == 0)
            continue;
        q.push(u_id);
        vector<bool> vis(time_of_paths[src][node_id].size(), false);
        inst_node temp_node(u_id, time_of_paths[src][node_id], vis);
        layer_g[source].push_back(temp_node);
    }
    if (q.empty())
        return false;
    cout << "Maybe here\n";
    for (int i = 1;i < funcs.size();i++) {
        queue<string> next_q;
        cnt++;
        bool pushed = false;
        while (!q.empty()) {
            string prev_id = q.front();
            vector<int> ids = parse_id(prev_id);
            q.pop();
            if (NF_to_node[funcs[i]].size() == 0)
                return false;
            for (int j = 0;j < NF_to_node[funcs[i]].size();j++) {
                int id = funcs[i];
                int node_id = NF_to_node[funcs[i]][j];
                string u_id;
                u_id = to_string(cnt) + ";" + to_string(node_id);

                nodes[cnt]++;

                if (paths[ids.back()][node_id].size() == 0)
                    continue;
                if (!pushed)
                    next_q.push(u_id);
                vector<bool> vis(time_of_paths[ids.back()][node_id].size(), false);
                inst_node temp_node(u_id, time_of_paths[ids.back()][node_id], vis);
                layer_g[prev_id].push_back(temp_node);
            }
            if (!pushed)
                pushed = true;
            if (next_q.empty())
                return false;
        }
        q = next_q;
    }
    cout << "Maybe here1\n";
    // cnt++;
    if (NF_to_node[funcs[funcs.size() - 1]].size() == 0)
        return false;
    int ctrr = 0;
    for (int j = 0;j < NF_to_node[funcs[funcs.size() - 1]].size();j++) {
        int id = funcs[funcs.size() - 1];
        int node_id = NF_to_node[funcs[funcs.size() - 1]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(node_id);

        nodes[cnt]++;

        if (paths[node_id][dest].size() == 0)
            continue;
        ctrr++;
        vector<bool> vis(time_of_paths[node_id][dest].size(), false);
        string destination = to_string(cnt + 1) + ";" + to_string(dest);
        inst_node temp_node(destination, time_of_paths[node_id][dest], vis);
        layer_g[u_id].push_back(temp_node);
    }
    if (ctrr == 0)
        return false;

    map<string, vector<inst_node>>::iterator it;
    // for(it=layer_g.begin();it!=layer_g.end();it++){
    //     cout<<it->first<<":\n";
    //     for(int j=0;j<it->second.size();j++){
    //         inst_node t = it->second[j];
    //         for(int i=0;i<t.links.size();i++){
    //             cout<<" "<<t.id<<"("<<t.links[i]<<")\n";
    //         }
    //         cout<<endl;
    //     }
    // }

    // cout<<"Layer Node size"<<endl;
    // for(int i=0;i<nodes.size();i++){
    //     cout<<i<<":"<<nodes[i]<<endl;
    // }

    cout << "Maybe here2\n";
    string source = to_string(0) + ";" + to_string(src);
    string destination = to_string(cnt + 1) + ";" + to_string(dest);
    // vector<vector<int>> layer_paths = get_all_paths(source,destination,layer_g,paths,time_of_paths);
    info i = get_all_paths(source, destination, layer_g, paths, time_of_paths);
    vector<pair<vector<int>, vector<int> > > sorter;
    // cout<<"Here1\n";
    cout << "i.paths:::::" << i.paths.size() << endl;
    if (i.paths.size() == 0)
        return false;
    for (int k = 0;k < i.paths.size();k++) {
        // cout<<"counter::;:"<<i.paths[k].size()<<endl;
        sorter.push_back(make_pair(i.paths[k], i.dep[k]));
    }
    // wil have to update the time;;;;;;;;;;;;;;;;;;;;;;;;
    // cout<<"Here sub1\n";
    sort(sorter.begin(), sorter.end(), comp(g, diff, request, n_resource));
    // sort(sorter.begin(),sorter.end(),[](pair<vector<int>,vector<int>>& a,pair<vector<int>,vector<int>>& b,vector<vector<node>> g,vector<node_capacity> n_r,Request request) -> bool {
    //     cout<<"Test1\n";
    //     double time_a = add_links(g,a.first,request.SFC,n_r,request.src,request.dest);
    //     cout<<"Test1.5\n";
    //     double time_b = add_links(g,b.first,request.SFC,n_r,request.src,request.dest);
    //     cout<<"Test2\n";
    //     if(abs(time_a-diff) == abs(time_b-diff)){
    //         cout<<"Test3\n";
    //         return (time_a-diff) <= (time_b - diff);
    //     }
    //     cout<<"Test4\n";
    //     return abs(time_a-diff) < abs(time_b-diff);
    // });
    // cout<<"Here sub2\n";

    vector<vector<int>> layer_paths;
    vector<vector<int>> layer_dep;
    // cout<<"Here2\n";
    for (int i = 0;i < sorter.size();i++) {
        layer_paths.push_back(sorter[i].first);
        layer_dep.push_back(sorter[i].second);
    }
    i.paths = layer_paths;
    i.dep = layer_dep;

    // for(int i=0;i<layer_paths.size();i++){
    //     for(int j=0;j<layer_paths[i].size();j++){
    //         cout<<layer_paths[i][j]<<"->";
    //     }
    //     cout<<endl;
    // }

    vector<int> layer_paths_time(layer_paths.size());

    // for(int i=0;i<layer_paths.size();i++){
    //     layer_paths_time[i] = add_links(g,layer_paths[i]);
    //     cout<<"Time::::"<<layer_paths_time[i]<<endl;
    // }

    // for(int i=0;i<layer_dep.size();i++){
    //     for(int j=0;j<layer_dep[i].size();j++){
    //         cout<<layer_dep[i][j]<<"->";
    //     }
    //     cout<<endl;
    // }

    //vector<vector<node>> temp_g = g;
    // cout<<"Here3\n";
    int p;
    for (p = 0;p < i.paths.size();p++) {
        i.dep[p].pop_back();
        if (is_available_BW(g, i.paths[p], request.t_arrival_rate) && is_available_resource(n_resource, funcs, i.dep[p], request.t_arrival_rate)) {
            //do the check for time update
            break;
        }
    }
    if (p == i.paths.size()) {
        //somehow drop the request
        return false;
    }
    double tt = add_links(g, i.paths[p]);
    result.mean_PD += (request.t_arrival_rate * abs(tt - diff));
    // cout<<"TTT:::::::"<<tt<<endl;
    if (tt > diff) {
        if (tt - diff > max_tt)
            max_tt = tt - diff;
    }
    // cout<<"Here4\n";
    update_BW(g, i.paths[p], request.t_arrival_rate);
    update_resource(n_resource, funcs, i.dep[p], request.t_arrival_rate);
    // cout<<"Here5\n";
    for (int k = 0;k < i.dep[p].size();k++) {
        deployed_inst[funcs[k]] = i.dep[p][k];
        //cout<<funcs[k]<<":::::::::"<<i.dep[p][k]<<endl;
    }
    return true;
}

bool layer_graph(int src, vector<int> funcs, int dest, vector<vector<node>>& g, map<int, double>& time, map<int, int>& deployed_inst,
    vector<vector<int>>& NF_to_node, map<int, double>& NFs, vector<node_capacity>& n_resource, vector<vector<vector<vector<int>>>>& paths,
    vector<vector<vector<double>>>& time_of_paths, Request request, Result& result, double& diff, double& max_tt) {
    cout << "L_Func3\n";
    double temp_diff = diff;
    for (int i = 0;i < funcs.size();i++) {
        diff -= NFs[funcs[i]];
    }
    // cout<<"FINAL_DIFF::::::"<<diff<<endl;
    map<string, vector<inst_node>> layer_g;
    queue<string> q;
    vector<int> nodes(10, 0);
    int cnt = 1;
    //forming the multi graph
    if (NF_to_node[funcs[0]].size() == 0)
        return false;
    for (int j = 0;j < NF_to_node[funcs[0]].size();j++) {
        int id = funcs[0];
        int node_id = NF_to_node[funcs[0]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(node_id);

        nodes[cnt]++;

        string source = to_string(cnt - 1) + ";" + to_string(src);
        if (paths[src][node_id].size() == 0)
            continue;
        q.push(u_id);
        vector<bool> vis(time_of_paths[src][node_id].size(), false);
        inst_node temp_node(u_id, time_of_paths[src][node_id], vis);
        layer_g[source].push_back(temp_node);
    }
    if (q.empty())
        return false;
    cout << "Maybe here\n";
    for (int i = 1;i < funcs.size();i++) {
        queue<string> next_q;
        cnt++;
        bool pushed = false;
        while (!q.empty()) {
            string prev_id = q.front();
            vector<int> ids = parse_id(prev_id);
            q.pop();
            if (NF_to_node[funcs[i]].size() == 0)
                return false;
            for (int j = 0;j < NF_to_node[funcs[i]].size();j++) {
                int id = funcs[i];
                int node_id = NF_to_node[funcs[i]][j];
                string u_id;
                u_id = to_string(cnt) + ";" + to_string(node_id);

                nodes[cnt]++;

                if (paths[ids.back()][node_id].size() == 0)
                    continue;
                if (!pushed)
                    next_q.push(u_id);
                vector<bool> vis(time_of_paths[ids.back()][node_id].size(), false);
                inst_node temp_node(u_id, time_of_paths[ids.back()][node_id], vis);
                layer_g[prev_id].push_back(temp_node);
            }
            if (!pushed)
                pushed = true;
            if (next_q.empty())
                return false;
        }
        q = next_q;
    }
    cout << "Maybe Here1\n";
    // cnt++;
    if (NF_to_node[funcs[funcs.size() - 1]].size() == 0)
        return false;
    int ctrr = 0;
    for (int j = 0;j < NF_to_node[funcs[funcs.size() - 1]].size();j++) {
        int id = funcs[funcs.size() - 1];
        int node_id = NF_to_node[funcs[funcs.size() - 1]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(node_id);

        nodes[cnt + 1]++;

        if (paths[node_id][dest].size() == 0)
            continue;
        ctrr++;
        vector<bool> vis(time_of_paths[node_id][dest].size(), false);
        string destination = to_string(cnt + 1) + ";" + to_string(dest);
        inst_node temp_node(destination, time_of_paths[node_id][dest], vis);
        layer_g[u_id].push_back(temp_node);
    }
    if (ctrr == 0)
        return false;

    map<string, vector<inst_node>>::iterator it;
    // for(it=layer_g.begin();it!=layer_g.end();it++){
    //     cout<<it->first<<":\n";
    //     for(int j=0;j<it->second.size();j++){
    //         inst_node t = it->second[j];
    //         for(int i=0;i<t.links.size();i++){
    //             cout<<" "<<t.id<<"("<<t.links[i]<<")\n";
    //         }
    //         cout<<endl;
    //     }
    // }
    bool pok = false;
    // cout<<"Layer Node size"<<endl;

    cout << "Maybe\n";
    string source = to_string(0) + ";" + to_string(src);
    string destination = to_string(cnt + 1) + ";" + to_string(dest);
    // vector<vector<int>> layer_paths = get_all_paths(source,destination,layer_g,paths,time_of_paths);
    info i = get_all_paths(source, destination, layer_g, paths, time_of_paths);
    vector<pair<vector<int>, vector<int> > > sorter;
    // cout<<"Here1\n";
    cout << "i.paths:::::" << i.paths.size() << endl;
    if (i.paths.size() == 0)
        return false;
    // if(i.paths.size() > 100000){
    //     for(int i=0;i<nodes.size();i++){
    //         s<<i<<":"<<nodes[i]<<endl;
    //     }
    //     for(it=layer_g.begin();it!=layer_g.end();it++){
    //         s<<it->first<<":\n";
    //         for(int j=0;j<it->second.size();j++){
    //             inst_node t = it->second[j];
    //             for(int i=0;i<t.links.size();i++){
    //                 s<<" "<<t.id<<"("<<t.links[i]<<")\n";
    //             }
    //             s<<endl;
    //         }
    //     }
    //     for(int i=0;i<funcs.size();i++){
    //         s<<i<<":"<<funcs[i]<<endl;
    //         for(int j=0;j<NF_to_node[funcs[i]].size();j++){
    //             s<<"\t"<<j<<":"<<NF_to_node[funcs[i]][j]<<endl;
    //         }
    //     }
    //     exit(0);
    // }
    for (int k = 0;k < i.paths.size();k++) {
        sorter.push_back(make_pair(i.paths[k], i.dep[k]));
    }
    // wil have to update the time;;;;;;;;;;;;;;;;;;;;;;;;
    sort(sorter.begin(), sorter.end(), comp(g, 0, request, n_resource));
    // sort(sorter.begin(),sorter.end(),[](pair<vector<int>,vector<int>>& a,pair<vector<int>,vector<int>>& b,vector<vector<node>> g,vector<node_capacity> n_r,Request request) -> bool {
    //     cout<<"Test1\n";
    //     double time_a = add_links(g,a.first,request.SFC,n_r,request.src,request.dest);
    //     cout<<"Test1.5\n";
    //     double time_b = add_links(g,b.first,request.SFC,n_r,request.src,request.dest);
    //     cout<<"Test2\n";
    //     if(abs(time_a-0) == abs(time_b-0)){
    //         cout<<"Test3\n";
    //         return (time_a-0) <= (time_b - 0);
    //     }
    //     cout<<"Test4\n";
    //     return abs(time_a-0) < abs(time_b-0);
    // });

    vector<vector<int>> layer_paths;
    vector<vector<int>> layer_dep;
    // cout<<"Here2\n";
    for (int i = 0;i < sorter.size();i++) {
        layer_paths.push_back(sorter[i].first);
        layer_dep.push_back(sorter[i].second);
    }
    i.paths = layer_paths;
    i.dep = layer_dep;

    // for(int i=0;i<layer_paths.size();i++){
    //     for(int j=0;j<layer_paths[i].size();j++){
    //         cout<<layer_paths[i][j]<<"->";
    //     }
    //     cout<<endl;
    // }

    vector<int> layer_paths_time(layer_paths.size());

    // for(int i=0;i<layer_paths.size();i++){
    //     layer_paths_time[i] = add_links(g,layer_paths[i]);
    //     cout<<"Time::::"<<layer_paths_time[i]<<endl;
    // }

    // for(int i=0;i<layer_dep.size();i++){
    //     for(int j=0;j<layer_dep[i].size();j++){
    //         cout<<layer_dep[i][j]<<"->";
    //     }
    //     cout<<endl;
    // }

    //vector<vector<node>> temp_g = g;
    // cout<<"Here3\n";
    int p;
    for (p = 0;p < i.paths.size();p++) {
        i.dep[p].pop_back();
        if (is_available_BW(g, i.paths[p], request.t_arrival_rate) && is_available_resource(n_resource, funcs, i.dep[p], request.t_arrival_rate)) {
            //do the check for time update
            break;
        }
    }
    if (p == i.paths.size()) {
        //somehow drop the request
        return false;
    }
    double tt = add_links(g, i.paths[p]);
    result.mean_PD += (request.t_arrival_rate * abs(tt - diff));
    // cout<<"TTT:::::::"<<tt<<endl;
    if (tt > diff) {
        if (tt - diff > max_tt)
            max_tt = tt - diff;
    }
    // cout<<"Here4\n";
    update_BW(g, i.paths[p], request.t_arrival_rate);
    update_resource(n_resource, funcs, i.dep[p], request.t_arrival_rate);
    // cout<<"Here5\n";
    for (int k = 0;k < i.dep[p].size();k++) {
        deployed_inst[funcs[k]] = i.dep[p][k];
        //cout<<funcs[k]<<":::::::::"<<i.dep[p][k]<<endl;
    }
    return true;
}

vector<vector<vector<int>>> bin_FP(vector<vector<vector<int>>> SFC, map<int, double> NFs) {
    vector<vector<vector<int>>> temp;

    // cout<<"Initial SFC:::::::\n";
    // for(int l=0;l<SFC.size();l++){
    //     for(int j=0;j<SFC[l].size();j++){
    //         for(int k=0;k<SFC[l][j].size();k++){
    //             cout<<"("<<SFC[l][j][k]<<")";
    //         }
    //         cout<<"-->";
    //     }
    //     cout<<endl;
    // }

    for (int i = 0;i < SFC.size();i++) { //for each Ci
        if (SFC[i].size() == 1) {
            temp.push_back(SFC[i]);
            continue;
        }
        double max_process = -1;
        int loc = 0;
        int max_func;
        for (int j = 0;j < SFC[i].size();j++) { // for each bx in Ci
            for (int k = 0;k < SFC[i][j].size();k++) { //finding the max f_time
                if (NFs[SFC[i][j][k]] > max_process) {
                    max_process = NFs[SFC[i][j][k]];
                    max_func = SFC[i][j][k];
                    loc = j;
                }
            }
        }
        if (loc == SFC[i].size() - 1)
            return SFC;
        else {
            int temper = SFC[i][SFC[i].size() - 1][0];
            SFC[i][SFC[i].size() - 1][0] = SFC[i][loc][0];
            SFC[i][loc][0] = temper;
            temp.push_back(SFC[i]);
        }

    }
    // cout<<"Final SFC:::::::\n";
    // for(int l=0;l<temp.size();l++){
    //     for(int j=0;j<temp[l].size();j++){
    //         for(int k=0;k<temp[l][j].size();k++){
    //             cout<<"("<<temp[l][j][k]<<")";
    //         }
    //         cout<<"-->";
    //     }
    //     cout<<endl;
    // }
    return temp;
}

//SFC = {{{0}},{{1,2},{3}},{{4}}};

vector<vector<vector<int>>> bin(vector<vector<vector<int>>> SFC, map<int, double> NFs) {
    vector<vector<vector<int>>> temp;

    for (int i = 0;i < SFC.size();i++) { //for each Ci
        if (SFC[i].size() == 1) {
            temp.push_back(SFC[i]);
            continue;
        }
        double max_process = -1;
        int max_func;
        for (int j = 0;j < SFC[i].size();j++) { // for each bx in Ci
            for (int k = 0;k < SFC[i][j].size();k++) { //finding the max f_time
                if (NFs[SFC[i][j][k]] > max_process) {
                    max_process = NFs[SFC[i][j][k]];
                    max_func = SFC[i][j][k];
                }
            }
        }
        cout << "MAX FUNC:::::" << max_func << endl;
        for (int l = 0;l < SFC.size();l++) {
            for (int j = 0;j < SFC[l].size();j++) {
                for (int k = 0;k < SFC[l][j].size();k++) {
                    if (SFC[l][j].size() > 1) {
                        cout << "It is there\n";
                        exit(0);
                    }
                    cout << "(" << SFC[l][j][k] << ")";
                }
                cout << "-->";
            }
            cout << endl;
        }

        double cap = max_process;
        vector<vector<int>> new_Ci;
        double cap_curr = 0;
        vector<int> bin;
        for (int b = 0;b < SFC[i].size();b++) {
            for (int f = 0;f < SFC[i][b].size();f++) {
                if (SFC[i][b][f] == max_func)
                    continue;
                else if (cap_curr + NFs[SFC[i][b][f]] <= cap) {
                    cap_curr += NFs[SFC[i][b][f]];
                    bin.push_back(SFC[i][b][f]);
                }
                else {
                    cap_curr = 0;
                    new_Ci.push_back(bin);
                    if (bin.size() == 0) {
                        cout << "2222222222222222222\n";
                        map<int, double>::iterator it;
                        for (it = NFs.begin();it != NFs.end();it++) {
                            s << it->first << ": " << it->second << endl;
                        }
                        exit(0);
                    }
                    bin.clear();
                    cap_curr += NFs[SFC[i][b][f]];
                    bin.push_back(SFC[i][b][f]);
                }
            }
        }
        if (!bin.empty())
            new_Ci.push_back(bin);
        new_Ci.push_back({ max_func });
        temp.push_back(new_Ci);

    }
    return temp;
}

bool check(vector<vector<vector<vector<int>>>>& paths) {
    for (int i = 0;i < paths.size();i++) {
        for (int j = 0;j < paths[i].size();j++) {
            for (int k = 0;k < paths[i][j].size();k++) {
                for (int l = 0;l < paths[i][j][k].size();l++) {
                    if (paths[i][j][k][l] < 0 || paths[i][j][k][l] > 7)
                        return false;
                }
            }
        }
    }
    cout << "check\n";
    return true;
}

//ofstream s("status.txt");

bool SFC_embedding_PD(vector<vector<node>>& g, vector<node_capacity>& n_resource, vector<vector<int>>& NF_to_node, map<int, double>& NFs, Request request, Result& result) {
    cout << "Enter PD aware algo\n";
    map<int, int> deployed_inst;
    map<int, double> time; //map that contains the reach time to a NF in the chain
    double dest_time;
    double pkt_copy = 2, pkt_merge = 2;
    //initializing reach time to that NF in the chain to 0
    vector<vector<vector<int>>> SFC = request.SFC;
    int src = request.src;
    int dest = request.dest;
    double e2e = request.e2e;
    double t_arrival_rate = request.t_arrival_rate;

    // for(int i=0;i<g[src].size();i++){
    //     for(int j=0;j<g[g[src][i].id].size();j++){
    //         if(g[g[src][i].id][j].id == src)
    //             g[g[src][i].id].erase(g[g[src][i].id].begin()+j);
    //     }
    // }

    // g[dest].clear();

    int K = 3; //the number of K shortest paths
    vector<vector<vector<vector<int>>>> paths(g.size(), vector<vector<vector<int>>>(g.size()));
    cout << paths.size() << paths[0].size() << endl;
    for (int i = 0;i < g.size();i++) {
        for (int j = 0;j < g.size();j++) {
            vector<vector<int>> k_paths;
            if (i == j) {
                for (int k = 0;k < K;k++) {
                    vector<int> temp;
                    temp.push_back(i);
                    k_paths.push_back(temp);
                }
            }
            else {
                //cout<<"here\n";
                k_paths = YenKSP(g, i, j, request, K);
                if (k_paths.size() == 0)
                    return false;
                //cout<<"here1\n";
            }
            paths[i][j] = k_paths;
        }
    }

    if (!check(paths)) {
        cout << "error:::::::::PD_paths" << endl;
        exit(0);
    }

    cout << paths.size() << paths[0].size() << endl;
    // paths[0][0]= k_paths;
    vector<vector<vector<double>>> time_of_paths = calc_time(g, paths);
    // cout<<"Time:"<<endl;
    // for(int i=0;i<time_of_paths[0][0].size();i++){
    //     cout<<time_of_paths[0][0][i]<<" ";
    // }
    // cout<<endl;



    for (int i = 0;i < SFC.size();i++) {
        for (int j = 0;j < SFC[i].size();j++) {
            for (int k = 0;k < SFC[i][j].size();k++) {
                time[SFC[i][j][k]] = 0;
            }
        }
    }

    // for(int i=0;i<time_of_paths.size();i++){
    //     for(int j=0;j<time_of_paths[i].size();j++){
    //         for(int k=0;k<time_of_paths[i][j].size();k++){
    //             cout<<i<<","<<j<<","<<k<<"::::\n"<<time_of_paths[i][j][k]<<endl;
    //             for(int p=0;p<paths[i][j][k].size();p++){
    //                 cout<<paths[i][j][k][p]<<"->";
    //             }
    //             cout<<endl;
    //         }
    //     }
    // }

    // {{{0}},{{1,2},{3}},{{4}}}
    //find critical branches delay from src to the C0
    vector<int> critical_branch_node(SFC.size());
    for (int i = 0;i < SFC.size();i++) {
        critical_branch_node[i] = SFC[i][SFC[i].size() - 1][SFC[i][SFC[i].size() - 1].size() - 1];
    }
    critical_branch_node[0] = SFC[0][SFC[0].size() - 1][0];
    int inst_0;
    int min_dist = INT_MAX;
    vector<int> min_path;
    // cout<<"Error::::"<<NF_to_node[critical_branch_node[0]].size()<<endl;
    for (int i = 0;i < NF_to_node[critical_branch_node[0]].size();i++) {
        if (n_resource[NF_to_node[critical_branch_node[0]][i]].NF_left[critical_branch_node[0]] > t_arrival_rate) {
            if (time_of_paths[src][NF_to_node[critical_branch_node[0]][i]][0] < min_dist) {
                min_dist = time_of_paths[src][NF_to_node[critical_branch_node[0]][i]][0];
                min_path = paths[src][NF_to_node[critical_branch_node[0]][i]][0];
                inst_0 = NF_to_node[critical_branch_node[0]][i];
                // cout<<"All inst::::"<<inst_0<<endl;
            }
        }
    }
    if (min_dist == INT_MAX) // could not find a path
        return false;
    vector<vector<node>> temp_g = g;
    vector<node_capacity> temp_n_resource = n_resource;
    deployed_inst[critical_branch_node[0]] = inst_0;
    time[critical_branch_node[0]] = min_dist/*link delay*/ + NFs[critical_branch_node[0]]/*function delay*/ + (SFC[1].size() - 1) * (pkt_copy)/*copy delay*/;
    update_BW(temp_g, min_path, request.t_arrival_rate);
    update_resource(temp_n_resource, vector<int>(1, critical_branch_node[0]), vector<int>(1, inst_0), request.t_arrival_rate);
    //call the shortest paths again since network got changed
   // for(int i=0;i<g.size();i++){
   //     for(int j=0;j<g.size();j++){
   //         vector<vector<int>> k_paths;
   //         if(i == j){
   //             for(int k=0;k<K;k++){
   //                 vector<int> temp;
   //                 temp.push_back(i);
   //                 k_paths.push_back(temp);
   //             }
   //         }
   //         else{
   //             //cout<<"here\n";
   //             k_paths = YenKSP(temp_g,i,j,request,K);
   //             if(k_paths.size() == 0)
   //                 return false;
   //             //cout<<"here1\n";
   //         }
   //         paths[i][j] = k_paths;
   //     }
   // }
   // if(!check(paths)){
   //     cout<<"error:::::::::PD_paths2"<<endl;
   //     exit(0);
   // }
   // time_of_paths = calc_time(temp_g,paths);

   // cout<<"inst:::"<<inst_0<<endl;
   // cout<<"Min dis:::"<<min_dist<<endl;
   // cout<<"F delay:::"<<NFs[critical_branch_node[0]]<<endl;
   // cout<<"Copy delay:::"<<(SFC[1].size()-1)*(pkt_copy)<<endl;
   // cout<<"initial::::"<<time[critical_branch_node[0]]<<endl;

    for (int i = 0;i < SFC.size() - 1;i++) {
        int inst;
        int min_dist = INT_MAX;
        vector<int> min_path;
        int next_branch_node;
        for (int b = 0;b < SFC[i + 1].size();b++) {
            if (SFC[i].size() == 1 && b != SFC[i + 1].size() - 1)
                continue;
            next_branch_node = SFC[i + 1][b][0];
            min_dist = INT_MAX;
            for (int j = 0;j < NF_to_node[next_branch_node].size();j++) {
                if (n_resource[NF_to_node[next_branch_node][j]].NF_left[next_branch_node] > t_arrival_rate) {
                    if (time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]].size() == 0)
                        continue;
                    if (time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0] < min_dist) {
                        min_dist = time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0];
                        // for(int a=0;a<paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0].size();a++){
                        //     cout<<paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0][a]<<"->";
                        // }
                        // cout<<endl;
                        min_path = paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0];
                        inst = NF_to_node[next_branch_node][j];
                    }
                }
            }
            if (min_dist == INT_MAX)
                return false;
            deployed_inst[next_branch_node] = inst;
            time[next_branch_node] = time[critical_branch_node[i]] + min_dist/*link delay*/ + NFs[next_branch_node]/*function delay*/ + (SFC[i].size() - 1) * (pkt_merge)/*merge delay*/;
            update_BW(temp_g, min_path, request.t_arrival_rate);
            update_resource(temp_n_resource, vector<int>(1, next_branch_node), vector<int>(1, inst), request.t_arrival_rate);
            // for(int i=0;i<g.size();i++){
            //     for(int j=0;j<g.size();j++){
            //         vector<vector<int>> k_paths;
            //         if(i == j){
            //             for(int k=0;k<K;k++){
            //                 vector<int> temp;
            //                 temp.push_back(i);
            //                 k_paths.push_back(temp);
            //             }
            //         }
            //         else{
            //             //cout<<"here\n";
            //             k_paths = YenKSP(temp_g,i,j,request,K);
            //             if(k_paths.size() == 0)
            //                 return false;
            //             //cout<<"here1\n";
            //         }
            //         paths[i][j] = k_paths;
            //     }
            // }
            // if(!check(paths)){
            //     cout<<"error:::::::::PD_paths3"<<endl;
            //     exit(0);
            // }
            // time_of_paths = calc_time(temp_g,paths);

                // cout<<"Min dis:::"<<min_dist<<endl;
                // cout<<"F delay:::"<<NFs[next_branch_node]<<endl;
                // cout<<"Copy delay:::"<<(SFC[i].size()-1)*(pkt_merge)<<endl;
                // cout<<"initial::::"<<time[next_branch_node]<<endl;
            if (i + 2 <= SFC.size() - 1 && SFC[i + 1][b].size() == 1)
                time[next_branch_node] += (SFC[i + 2].size() - 1) * (pkt_copy);
        }
    }
    if (time_of_paths[deployed_inst[critical_branch_node[SFC.size() - 1]]][dest].size() == 0)
        return false;
    dest_time = time[critical_branch_node[SFC.size() - 1]] + time_of_paths[deployed_inst[critical_branch_node[SFC.size() - 1]]][dest][0] + (SFC[SFC.size() - 1].size() - 1) * (pkt_merge);
    // for(int i=0;i<SFC.size();i++){
    //     for(int j=0;j<SFC[i].size();j++){
    //         for(int k=0;k<SFC[i][j].size();k++){
    //             cout<<i<<","<<j<<","<<k<<":::"<<time[SFC[i][j][k]]<<endl;
    //         }
    //     }
    // }
    cout << "e2e latency:::::::::::" << dest_time << endl;

    // layer graph step......
    for (int i = 0;i < SFC.size();i++) {
        if (SFC[i].size() == 1)
            continue;

        // UPDATE NEEDED!!!!!
        double max_tt = 0;
        double diff = 0;
        if (i == SFC.size() - 1) {
            diff = dest_time - time[SFC[i - 1][SFC[i - 1].size() - 1][0]];
            diff -= (SFC[i].size() - 1) * pkt_merge;
            // cout<<"DIFFFFFFFF:::::::::"<<diff<<endl;
        }
        else {
            diff = time[SFC[i + 1][SFC[i + 1].size() - 1][0]] - time[SFC[i - 1][SFC[i - 1].size() - 1][0]];
            diff -= NFs[SFC[i + 1][SFC[i + 1].size() - 1][0]] + (SFC[i].size() - 1) * pkt_merge;
            // cout<<"DIFFFFFFFF:::::::::"<<diff<<endl;
            // cout<<time[SFC[i+1][SFC[i+1].size()-1][0]]<<"\n"<<time[SFC[i-1][SFC[i-1].size()-1][0]]<<"\n"<<NFs[SFC[i+1][SFC[i+1].size()-1][0]]<<"\n"<<(SFC[i].size()-1)*pkt_merge<<endl;
        }
        for (int b = 0;b < SFC[i].size() - 1;b++) { //checking all branches except critical branch since all function instances have been deployed in the critical branch
            // if(SFC[i][b].size() == 0){
            //     map<int,double>::iterator it;
            //     for(it = NFs.begin();it!=NFs.end();it++){
            //         s<<it->first<<": "<<it->second<<endl;
            //     }
            //     for(int l=0;l<SFC.size();l++){
            //         for(int j=0;j<SFC[l].size();j++){
            //             for(int k=0;k<SFC[l][j].size();k++){
            //                 s<<"("<<SFC[l][j][k]<<")";
            //             }
            //             s<<"-->";
            //         }
            //         s<<endl;
            //     }
            // }
            if (deployed_inst.find(SFC[i][b][0]) == deployed_inst.end()) {
                if (i == 0) {
                    cout << "Fnc1\n";
                    for (int b_n = 0;b_n < SFC[i + 1].size();b_n++) {
                        if (!layer_graph_2(src, SFC[i][b], deployed_inst[SFC[i + 1][b_n][0]], temp_g, time, deployed_inst, NF_to_node, NFs, temp_n_resource, paths, time_of_paths, request, result, diff, max_tt))
                            return false;
                    }
                }
                else {
                    if (i == SFC.size() - 1) {
                        cout << "CHECKER FOR DEPLOYED::::::" << deployed_inst[SFC[i - 1][0][0]] << " " << SFC[i][b].size() << endl;
                        if (!layer_graph_2(deployed_inst[SFC[i - 1][0][0]], SFC[i][b], dest, temp_g, time, deployed_inst, NF_to_node, NFs, temp_n_resource, paths, time_of_paths, request, result, diff, max_tt))
                            return false;
                    }
                    else {
                        cout << "Fnc2\n";
                        for (int b_n = 0;b_n < SFC[i + 1].size();b_n++) {
                            // cout<<"HHHHHHHHHHHHHHHHHHHHHHHH:"<<b_n<<endl;
                            if (!layer_graph_2(deployed_inst[SFC[i - 1][0][0]], SFC[i][b], deployed_inst[SFC[i + 1][b_n][0]], temp_g, time, deployed_inst, NF_to_node, NFs, temp_n_resource, paths, time_of_paths, request, result, diff, max_tt))
                                return false;
                            // cout<<"SSSSSSSSSSSSSS::"<<deployed_inst[SFC[i-1][0][0]]<<"DDDDDDDDDDDDDDDD::"<<deployed_inst[SFC[i+1][b_n][0]]<<endl;
                        }
                    }
                }
            }
            else {
                if (i == SFC.size() - 1) {
                    cout << "Fnc4\n";
                    if (!layer_graph_2(deployed_inst[SFC[i][b][0]], SFC[i][b], dest, temp_g, time, deployed_inst, NF_to_node, NFs, temp_n_resource, paths, time_of_paths, request, result, diff, max_tt))
                        return false;
                }
                else {
                    cout << "Fnc5\n";
                    for (int b_n = 0;b_n < SFC[i + 1].size();b_n++) {
                        if (!layer_graph_2(deployed_inst[SFC[i][b][0]], SFC[i][b], deployed_inst[SFC[i + 1][b_n][0]], temp_g, time, deployed_inst, NF_to_node, NFs, temp_n_resource, paths, time_of_paths, request, result, diff, max_tt))
                            return false;
                    }
                }
            }
        }
        dest_time += max_tt;
    }
    if (dest_time > request.e2e)
        return false;
    g = temp_g;
    n_resource = temp_n_resource;

    // cout<<"Check:::::::::"<<deployed_inst[0]<<endl;

    // for(int i=0;i<SFC.size();i++){
    //     for(int j=0;j<SFC[i].size();j++){
    //         for(int k=0;k<SFC[i][j].size();k++){
    //             cout<<i<<","<<j<<","<<k<<":::"<<deployed_inst[SFC[i][j][k]]<<endl;
    //         }
    //     }
    // }
    for (int i = 0;i < SFC.size();i++) {
        for (int j = 0;j < SFC[i].size();j++) {
            for (int k = 0;k < SFC[i][j].size();k++) {
                result.nodes_util[deployed_inst[SFC[i][j][k]]]++;
            }
        }
    }

    cout << "final e2e latency:::::::::::" << dest_time << endl;
    result.mean_latency += dest_time;
    return true;
}

pair<vector<Request>, vector<Request_ilp>> generate_SFC(int n, int num_of_funcs, int g_low, int g_high) {
    vector<Request> requests;
    vector<Request_ilp> requests_ilp;
    for (int i = 0;i < n;i++) {
        Request request;
        Request_ilp request_ilp;
        int start, end;
        double e2e, arrival;
        int PE_size, SFC_length;
        vector<vector<vector<int>>> SFC;
        vector<vector<int>> SFC_ilp;

        std::set<int> numbers;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, num_of_funcs - 1);
        std::uniform_int_distribution<> dis_SFC(4, 8);

        // randomly generating the SFC length (3 <= length <= num_of_funcs)
        //SFC_length = dis_SFC(gen);
        SFC_length = 6;
        // cout<<"SFC Length::::::"<<SFC_length<<endl;

        // randomly generating the PE size (2 <= PE_size <= SFC_length-1)
        std::uniform_int_distribution<> dis_PE(2, SFC_length - 1);
        PE_size = dis_PE(gen);
        // cout<<"PE Size::::::"<<PE_size<<endl;

        // randomly generating the SFC by forming a non repeating sequence
        while (numbers.size() < SFC_length) {
            numbers.insert(dis(gen));
        }

        vector<int> temp(numbers.begin(), numbers.end());

        std::uniform_int_distribution<> dis_part(1, temp.size() - PE_size);
        int low = dis_part(gen);
        for (int i = 0;i < low;i++) {
            vector<vector<int>> t(1, vector<int>(1, temp[i]));

            SFC_ilp.push_back(vector<int>(1, temp[i]));
            SFC.push_back(t);
        }
        vector<vector<int>> t1;
        vector<int> t2;
        for (int i = low;i < low + PE_size;i++) {
            t1.push_back(vector<int>(1, temp[i]));
            t2.push_back(temp[i]);

        }
        SFC.push_back(t1);
        SFC_ilp.push_back(t2);

        for (int i = low + PE_size;i < temp.size();i++) {
            vector<vector<int>> t2(1, vector<int>(1, temp[i]));
            SFC.push_back(t2);
            SFC_ilp.push_back(vector<int>(1, temp[i]));
        }

        std::uniform_real_distribution<> dis_e2e(140, 180);
        request.SFC = SFC;
        request_ilp.SFC = SFC_ilp;

        request.e2e = dis_e2e(gen);
        request_ilp.e2e = request.e2e;

        std::uniform_real_distribution<> dis_arrival(1, 1.5);
        request.t_arrival_rate = dis_arrival(gen);
        request_ilp.t_arrival_rate = request.t_arrival_rate;

        set<int> nodes;
        std::uniform_int_distribution<> dis_nodes(g_low, g_high);
        while (nodes.size() < 2) {
            nodes.insert(dis_nodes(gen));
        }
        vector<int> temp_nu(nodes.begin(), nodes.end());
        request.src = temp_nu[0];
        request_ilp.src = temp_nu[0];

        request.dest = temp_nu[1];
        request_ilp.dest = temp_nu[1];
        // for(int i=0;i<request.SFC.size();i++){
        //     for(int j=0;j<request.SFC[i].size();j++){
        //         for(int k=0;k<request.SFC[i][j].size();k++){
        //             if(request.SFC[i][j].size() > 1){
        //                 cout<<"Maybe here\n";
        //                 exit(0);
        //             }
        //             cout<<"("<<request.SFC[i][j][k]<<")";
        //         }
        //         cout<<"-->";
        //     }
        //     cout<<endl;
        // }
        // cout<<"SFC::::::"<<request.SFC.size()<<endl;
        // cout<<"Nodes::::::"<<request.src<<"    "<<request.dest<<endl;
        requests.push_back(request);
        requests_ilp.push_back(request_ilp);
    }
    return make_pair(requests, requests_ilp);
}

double BW_used(vector<vector<node>>& g) {
    double bw = 0;
    for (int i = 0;i < g.size();i++) {
        for (int j = 0;j < g[i].size();j++) {
            if (i < g[i][j].id)
                bw += (g[i][j].init_bw - g[i][j].available_bandwidth);
        }
    }
    return bw;
}

class comparator {
public:
    bool operator () (Request& a, Request& b) {
        return a.t_arrival_rate < b.t_arrival_rate;
    }
};

int main() {
    ifstream fin("graph_config.txt");
    ifstream rin("request.txt");

    int N, M;
    int funcs, n_of_requests, n_of_f_instances;
    double total_BW = 0;

    //rin>>n_of_requests;

    fin >> funcs >> n_of_f_instances;
    fin >> N >> M;
    fin >> n_of_requests;

    
    vector<double> t_f(funcs + 2, 0);
    vector<vector<int>> g(N,vector<int>(N,0));
    vector<vector<double>> bw(N, vector<double>(N, INF)); //make it 0 when there is no edge
    vector<vector<double>> t_uv(N, vector<double>(N, 0));

    std::random_device rd;
    std::mt19937 gen(rd());

    vector<vector<node>> g_1(N); //network topology
    vector<node_capacity> n_resource_1(N); //tells all the function instances deployed in a node in the topology
    vector<vector<int>> NF_to_node(funcs);
    map<int, double> NFs;


    vector<vector<int>> dep(N, vector<int>(funcs + 2, 0));
    vector<vector<double>> cap(N, vector<double>(funcs + 2, 0));

    for (int i = 0;i < funcs;i++) {
        int f_id;
        double p_time;
        std::uniform_int_distribution<> func_time(1, 20);
        //rin>>f_id>>p_time;
        f_id = i;
        p_time = func_time(gen);
        NFs[f_id] = p_time;

        t_f[f_id] = p_time;
    }

    int VMs = 4;  // max number of VMs per server
    double VM_cap = 5;
    for (int i = 0;i < N;i++) {
        std::uniform_int_distribution<> VM(1, VMs);
        int node_vm = VM(gen);
        std::set<int> numbers;
        std::uniform_int_distribution<> node_funcs(0, funcs - 1);
        while (numbers.size() < node_vm) {
            numbers.insert(node_funcs(gen));
        }
        vector<int> f_in_node(numbers.begin(), numbers.end());
        for (int j = 0;j < node_vm;j++) {
            int f_id = f_in_node[j];
            double time;
            int node_id = i;
            //fin>>node_id>>f_id>>time;
            node_id = i;
            f_id = f_in_node[j];
            // node_capacity temp(f_id,time);
            n_resource_1[node_id].NF_left[f_id] = VM_cap;
            n_resource_1[node_id].deployed_NF[f_id] = NFs[f_id];

            cap[node_id][f_id] = 10;//VM_cap;
            dep[node_id][f_id] = 1;

            NF_to_node[f_id].push_back(node_id);
        }
    }

    // creating the graph
    for (int i = 0;i < M;i++) {
        int s, d;
        double l, b;
        fin >> s >> d >> l >> b;
        std::uniform_int_distribution<> bw_gen(20, 30);
        std::uniform_int_distribution<> link(4, 8);
        int temp1 = d;
        int temp2 = s;
        //b = bw_gen(gen);
        total_BW += b;
        //l = link(gen);
        node n1(temp1, l, b);
        node n2(temp2, l, b);
        g_1[s].push_back(n1);
        g_1[d].push_back(n2);

        g[s][d] = 1;
        g[d][s] = 1;
        t_uv[s][d] = l;
        t_uv[d][s] = l;
        bw[s][d] = b;
        bw[d][s] = b;   //update both the edges bw simultaneously
    }
    //making bw between non existant edges 0
    for (int u = 0;u < N;u++) {
        for (int v = 0;v < N;v++) {
            if (g[u][v] == 0) {
                bw[u][v] = 0;
            }
        }
    }
    auto req = generate_SFC(5,funcs, 0, 6);
    auto ilp_requests = req.second;
    Result ilp_result(N);
    int ilp_AR = 0;
    for (int i = 0;i < ilp_requests.size();i++) {
        bool res = ILPsolve(g, bw, t_uv, t_f, dep, cap, ilp_requests[i],ilp_result, funcs);
        if (res)
            ilp_AR++;
    }

    return 0;
}

//int main() {
//    ifstream fin("graph_config.txt");
//    ifstream rin("request.txt");
//
//    int N, M;
//    int funcs, n_of_requests, n_of_f_instances;
//
//    //rin>>n_of_requests;
//
//    fin >> funcs >> n_of_f_instances;
//    fin >> N >> M;
//    fin >> n_of_requests;
//
//    //vector<vector<int>> SFC{ {10}, {0}, {1},{4},{11} };
//    vector<vector<int>> SFC{ {10}, {0}, {1,2,3},{4},{11} };
//    //vector<vector<int>> SFC{ {10}, {1,2 }, { 11 } };
//    int src = 0;
//    int dest = 6;
//    int a = 2;
//    vector<double> t_f(funcs + 2 + (SFC[a].size() * (SFC[a].size() - 1)), 0);
//    vector<vector<int>> g(N, vector<int>(N, 0));
//    vector<vector<double>> bw(N, vector<double>(N, INF)); //make it 0 when there is no edge
//    vector<vector<double>> t_uv(N, vector<double>(N, 0));
//    // creating the graph
//    for (int i = 0;i < M;i++) {
//        int s, d;
//        double l, b;
//        fin >> s >> d >> l >> b;
//        g[s][d] = 1;
//        g[d][s] = 1;
//        t_uv[s][d] = l;
//        t_uv[d][s] = l;
//        bw[s][d] = b;
//        bw[d][s] = b;   //update both the edges bw simultaneously
//    }
//    //making bw between non existant edges 0
//    for (int u = 0;u < N;u++) {
//        for (int v = 0;v < N;v++) {
//            if (g[u][v] == 0) {
//                bw[u][v] = 0;
//            }
//        }
//    }
//    //getting func time value
//    for (int i = 0;i < funcs;i++) {
//        int f_id;
//        double p_time;
//        rin >> f_id >> p_time;
//        t_f[f_id] = p_time;
//    }
//
//    std::cout << "Graph\n";
//    for (int i = 0;i < N;i++) {
//        for (int j = 0;j < N;j++) {
//            std::cout << g[i][j] << " ";
//        }
//        std::cout << endl;
//    }
//
//    std::cout << "Links\n";
//    for (int i = 0;i < N;i++) {
//        for (int j = 0;j < N;j++) {
//            std::cout << t_uv[i][j] << " ";
//        }
//        std::cout << endl;
//    }
//
//    std::cout << "BW\n";
//    for (int i = 0;i < N;i++) {
//        for (int j = 0;j < N;j++) {
//            std::cout << bw[i][j] << " ";
//        }
//        std::cout << endl;
//    }
//
//    std::cout << "Funcs\n";
//    for (int i = 0;i < t_f.size();i++) {
//        std::cout << i << ": " << t_f[i] << endl;
//    }
//
//    int temp = (SFC[a].size() * (SFC[a].size() - 1));
//    vector<vector<int>> dep(N, vector<int>(funcs + 2 + temp, 0));
//    vector<vector<int>> cap(N, vector<int>(funcs + 2 + temp, 0));
//
//    //function deployed instance
//    dep[0][10] = 1;
//    dep[6][11] = 1;
//    dep[1][2] = 1;
//    dep[1][4] = 1;
//    dep[2][0] = 1;
//    dep[2][3] = 1;
//    dep[3][0] = 1;
//    dep[4][1] = 1;
//    dep[4][2] = 1;
//    dep[5][1] = 1;
//
//    //dep[0][10] = 1;
//    //dep[0][0] = 1;
//    //dep[1][1] = 1;
//    //dep[2][2] = 1;
//    //dep[2][11] = 1;
//    //dep[3][4] = 1;
//
//    //dep[0][10] = 1;
//    //dep[1][0] = 1;
//    //dep[1][1] = 1;
//    //dep[1][2] = 1;
//    //dep[1][4] = 1;
//    //dep[2][11] = 1;
//
//    //deployed instance capacity
//    cap[0][10] = 1;
//    cap[6][11] = 1;
//    cap[1][2] = 10;
//    cap[1][4] = 10;
//    cap[2][0] = 10;
//    cap[2][3] = 10;
//    cap[3][0] = 10;
//    cap[4][1] = 10;
//    cap[4][2] = 10;
//    cap[5][1] = 10;
//
//    std::cout << "SFC before: \n";
//    for (int i = 0;i < SFC.size();i++) {
//        for (int j = 0;j < SFC[i].size();j++) {
//            std::cout << SFC[i][j] << ", ";
//        }
//        std::cout << endl;
//    }
//
//    vector<int> B(SFC.size(), 1);
//    B[a] = SFC[a].size();
//    double arrival_SFC = 1;
//
//    //pushing the dummy functions into the SFC
//    for (int i = 0;i < temp;i++) {
//        SFC[a].push_back(funcs + 2 + i);
//    }
//
//    std::cout << "SFC after: \n";
//    for (int i = 0;i < SFC.size();i++) {
//        for (int j = 0;j < SFC[i].size();j++) {
//            std::cout << SFC[i][j] << ", ";
//        }
//        std::cout << endl;
//    }
//
//    //making the dummy functions available everywhere
//    for (int i = 0;i < temp;i++) {
//        for (int u = 0;u < N;u++) {
//            dep[u][funcs + 2 + i] = 1;
//            cap[u][funcs + 2 + i] = INF;
//        }
//    }
//
//    double copy_time = 2, merge_time = 2;
//    double T_l = 1000;
//    IloEnv env;
//    try {
//        IloModel model(env);
//# pragma region Node Variable
//        NumVar5d NODE(env, SFC.size());
//        for (int i = 0;i < SFC.size();i++) {
//            NumVar4d t1(env, B[i]);
//            for (int b = 0;b < B[i];b++) {
//                NumVar3d t2(env, B[i]);
//                for (int s = 0;s < B[i];s++) {
//                    NumVar2d t3(env, funcs + 2 + temp);
//                    for (int f = 0;f < funcs + 2 + temp;f++) {
//                        IloNumVarArray t4(env, N, 0, 1, ILOINT);
//                        t3[f] = t4;
//                    }
//                    t2[s] = t3;
//                }
//                t1[b] = t2;
//            }
//            NODE[i] = t1;
//        }
//#pragma endregion
//
//# pragma region EDGE Variable
//        NumVar8d EDGE(env, SFC.size());
//        for (int i = 0;i < SFC.size();i++) {
//            NumVar7d t1(env, SFC.size());
//            for (int i1 = 0;i1 < SFC.size();i1++) {
//                NumVar6d t2(env, B[i]);
//                for (int b = 0;b < B[i];b++) {
//                    NumVar5d t3(env, B[i1]);
//                    for (int b1 = 0;b1 < B[i1];b1++) {
//                        NumVar4d t4(env, B[i]);
//                        for (int s = 0;s < B[i];s++) {
//                            NumVar3d t5(env, B[i1]);
//                            for (int s1 = 0;s1 < B[i1];s1++) {
//                                NumVar2d t6(env, N);
//                                for (int v = 0;v < N;v++) {
//                                    IloNumVarArray t7(env, N, 0, 1, ILOINT);
//                                    t6[v] = t7;
//                                }
//                                t5[s1] = t6;
//                            }
//                            t4[s] = t5;
//                        }
//                        t3[b1] = t4;
//                    }
//                    t2[b] = t3;
//                }
//                t1[i1] = t2;
//            }
//            EDGE[i] = t1;
//        }
//#pragma endregion
//
//        IloNumVarArray theta(env, 1, 0, IloInfinity, ILOFLOAT);
//        double param1 = 0.8;
//        double param2 = 0.1;
//        double param3 = 0.1;
//
//#pragma region Objective function
//        IloExpr node_expr(env);
//        IloExpr edge_expr(env);
//        for (int b = 0;b < B[a];b++) {
//            for (int s = 0;s < B[a];s++) {
//                for (int f = 0;f < B[a];f++) {   //funcs is enough
//                    for (int u = 0;u < N;u++) {
//                        node_expr += (t_f[SFC[a][f]] * NODE[a][b][s][SFC[a][f]][u]);
//                    }
//                }
//            }
//            for (int s = 0;s < B[a] - 1;s++) {
//                //for (int s1 = s+1;s1 < B[a];s1++) {
//                //    for (int u = 0;u < N;u++) {
//                //        for (int v = 0;v < N;v++) {
//                //            edge_expr += (t_uv[u][v] * EDGE[a][a][b][b][s][s1][u][v]);
//                //        }
//                //    }
//                //}
//                for (int u = 0;u < N;u++) {
//                    for (int v = 0;v < N;v++) {
//                        if (g[u][v]) {
//                            edge_expr += (t_uv[u][v] * EDGE[a][a][b][b][s][s + 1][u][v]);
//                        }
//                    }
//                }
//                //for (int u = 0;u < N;u++) {
//                //    for (int v = 0;v < N;v++) {
//                //        edge_expr += (t_uv[u][v] * (EDGE[a - 1][a][0][b][0][s][u][v] + EDGE[a][a + 1][b][0][s][0][u][v]));
//                //    }
//                //}
//            }
//            for (int u = 0;u < N;u++) {
//                for (int v = 0;v < N;v++) {
//                    if (g[u][v]) {
//                        edge_expr += (t_uv[u][v] * (EDGE[a - 1][a][0][b][0][0][u][v] + EDGE[a][a + 1][b][0][B[a] - 1][0][u][v]));
//                    }
//                }
//            }
//        }
//
//        IloExpr e2e_expr(env);
//        e2e_expr += (B[a] - 1) * (copy_time + merge_time);
//        for (int i = 0;i < a;i++) {
//            for (int f = 0;f < B[i];f++) {
//                for (int u = 0;u < N;u++) {
//                    e2e_expr += NODE[i][0][0][SFC[i][f]][u] * t_f[SFC[i][f]];
//                }
//            }
//            if (i < a - 1) {
//                for (int u = 0;u < N;u++) {
//                    for (int v = 0;v < N;v++) {
//                        if (g[u][v]) {
//                            e2e_expr += EDGE[i][i + 1][0][0][0][0][u][v] * t_uv[u][v];
//                        }
//                    }
//                }
//            }
//        }
//        for (int i = a + 1;i < SFC.size() - 1;i++) {
//            for (int f = 0;f < B[i];f++) {
//                for (int u = 0;u < N;u++) {
//                    e2e_expr += NODE[i][0][0][SFC[i][f]][u] * t_f[SFC[i][f]];
//                }
//            }
//            for (int u = 0;u < N;u++) {
//                for (int v = 0;v < N;v++) {
//                    if (g[u][v]) {
//                        e2e_expr += EDGE[i][i + 1][0][0][0][0][u][v] * t_uv[u][v];
//                    }
//                }
//            }
//        }
//        e2e_expr += theta[0];
//
//        IloExpr bw_cons(env);
//        for (int i = 0;i < SFC.size() - 1;i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int b1 = 0;b1 < B[i + 1];b1++) {
//                    for (int u = 0;u < N;u++) {
//                        for (int v = 0;v < N;v++) {
//                            bw_cons += EDGE[i][i + 1][b][b1][B[i] - 1][0][u][v] * arrival_SFC;
//                        }
//                    }
//                }
//            }
//            if (i == a) {
//                for (int b = 0;b < B[i];b++) {
//                    for (int s = 0;s < B[i] - 1;s++) {
//                        for (int u = 0;u < N;u++) {
//                            for (int v = 0;v < N;v++) {
//                                bw_cons += EDGE[a][a][b][b][s][s + 1][u][v] * arrival_SFC;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//
//        IloExpr obj(env);
//        obj += param1 * ((B[a] * theta[0]) - (node_expr + edge_expr)) + (param2)*e2e_expr + (param2)*bw_cons;
//        //model.add(obj >= 0);
//        model.add(IloMinimize(env, obj));
//
//#pragma endregion
//
//#pragma region Constraints
//        // setting constraint for the auxilary varaible theta
//        for (int b = 0;b < B[a];b++) {
//            IloExpr node_c(env);
//            IloExpr edge_c(env);
//            for (int s = 0;s < B[a];s++) {
//                for (int f = 0;f < B[a];f++) {
//                    for (int u = 0;u < N;u++) {
//                        node_c += (t_f[SFC[a][f]] * NODE[a][b][s][SFC[a][f]][u]);
//                    }
//                }
//            }
//            for (int s = 0;s < B[a] - 1;s++) {
//                //for (int s1 = s + 1;s1 < B[a];s1++) {
//                //    for (int u = 0;u < N;u++) {
//                //        for (int v = 0;v < N;v++) {
//                //            edge_c += (t_uv[u][v] * EDGE[a][a][b][b][s][s1][u][v]);
//                //        }
//                //    }
//                //}
//                for (int u = 0;u < N;u++) {
//                    for (int v = 0;v < N;v++) {
//                        if (g[u][v]) {
//                            edge_c += (t_uv[u][v] * EDGE[a][a][b][b][s][s + 1][u][v]);
//                        }
//                    }
//                }
//                //for (int u = 0;u < N;u++) {
//                //    for (int v = 0;v < N;v++) {
//                //        edge_c += (t_uv[u][v] * (EDGE[a - 1][a][0][b][0][s][u][v] + EDGE[a][a + 1][b][0][s][0][u][v]));
//                //    }
//                //}
//            }
//            for (int u = 0;u < N;u++) {
//                for (int v = 0;v < N;v++) {
//                    if (g[u][v]) {
//                        edge_c += (t_uv[u][v] * (EDGE[a - 1][a][0][b][0][0][u][v] + EDGE[a][a + 1][b][0][B[a] - 1][0][u][v]));
//                    }
//                }
//            }
//            model.add(theta[0] >= (node_c + edge_c));
//        }
//        // ensures each slot in a branch has only one function picked
//        for (int i = 0;i < SFC.size();i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int s = 0;s < B[i];s++) {
//                    IloExpr c(env);
//                    for (int f = 0;f < funcs + 2 + temp;f++) {
//                        for (int u = 0;u < N;u++) {
//                            c += NODE[i][b][s][f][u];
//                        }
//                    }
//                    model.add(c == 1);
//                }
//            }
//        }
//        //ensures that total nodes picked in PE_i is same as the number of parallel functions
//        for (int i = 0;i < SFC.size();i++) {
//            IloExpr c(env);
//            for (int b = 0;b < B[i];b++) {
//                for (int s = 0;s < B[i];s++) {
//                    for (int f = 0;f < funcs + 2 + temp;f++) {
//                        for (int u = 0;u < N;u++) {
//                            c += NODE[i][b][s][f][u];
//                        }
//                    }
//                }
//            }
//            //model.add(c == SFC[i].size());
//        }
//        //ensures each function is picked atmost once
//        for (int f = 0;f < funcs + 2 + temp;f++) {
//            IloExpr c(env);
//            for (int i = 0;i < SFC.size();i++) {
//                for (int b = 0;b < B[i];b++) {
//                    for (int s = 0;s < B[i];s++) {
//                        for (int u = 0;u < N;u++) {
//                            c += NODE[i][b][s][f][u];
//                        }
//                    }
//                }
//            }
//            model.add(c <= 1);
//        }
//        //for (int i = 0;i < SFC.size();i++) {
//        //    for (int f = 0;f < SFC[i].size();f++) {
//        //        IloExpr c(env);
//        //        for (int b = 0;b < B[i];b++) {
//        //            for (int s = 0;s < B[i];s++) {
//        //                for (int u = 0;u < N;u++) {
//        //                    c += NODE[i][b][s][SFC[i][f]][u];
//        //                }
//        //            }
//        //        }
//        //        model.add(c == 1);
//        //    }
//        //}
//        // ensures that only those functions are selected that belong to the respective PE_i  
//        for (int i = 0;i < SFC.size();i++) {
//            IloExpr c(env);
//            vector<bool> fs(funcs + 2 + temp, false);
//            for (int t = 0;t < SFC[i].size();t++) {
//                fs[SFC[i][t]] = true;
//            }
//            for (int b = 0;b < B[i];b++) {
//                for (int s = 0;s < B[i];s++) {
//                    for (int f = 0;f < funcs + 2 + temp;f++) {
//                        if (!fs[f]) {
//                            for (int u = 0;u < N;u++) {
//                                model.add(NODE[i][b][s][f][u] == 0);
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        //flow conservation within parallel branch
//        for (int b = 0;b < B[a];b++) {
//            for (int s = 0;s < B[a] - 1;s++) {
//                for (int u = 0;u < N;u++) {
//                    IloExpr c(env);
//                    IloExpr c1(env);
//                    for (int v = 0;v < N;v++) {
//                        if (g[u][v] == 1) {
//                            c += EDGE[a][a][b][b][s][s + 1][v][u];
//                        }
//                    }
//                    for (int w = 0;w < N;w++) {
//                        if (g[u][w] == 1) {
//                            c1 += EDGE[a][a][b][b][s][s + 1][u][w];
//                        }
//                    }
//                    IloExpr n1(env);
//                    IloExpr n(env);
//                    for (int f = 0;f < SFC[a].size();f++) {
//                        n1 += NODE[a][b][s + 1][SFC[a][f]][u];
//                        n += NODE[a][b][s][SFC[a][f]][u];
//                    }
//                    model.add((c - c1) - (n1 - n) == 0);
//                    //model.add((c - c1) /* - (NODE[a][b][s + 1][SFC[a][f1]][u] - NODE[a][b][s][SFC[a][f]][u]) */ == 0);
//                }
//
//            }
//        }
//        //flow conservation between PEs
//        for (int i = 0;i < SFC.size() - 1;i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int b1 = 0;b1 < B[i + 1];b1++) {
//                    for (int u = 0;u < N;u++) {
//                        IloExpr c(env);
//                        IloExpr c1(env);
//                        for (int v = 0;v < N;v++) {
//                            if (g[u][v] == 1) {
//                                c += EDGE[i][i + 1][b][b1][B[i] - 1][0][v][u];
//                            }
//                        }
//                        for (int w = 0;w < N;w++) {
//                            if (g[u][w] == 1) {
//                                c1 += EDGE[i][i + 1][b][b1][B[i] - 1][0][u][w];
//                            }
//                        }
//                        if (i == 0 && u == src) {
//                            model.add(c == 0);
//                            model.add(c1 == 1);
//                        }
//                        else if (i == SFC.size() - 2 and u == dest) {
//                            model.add(c == 1);
//                            model.add(c1 == 0);
//                        }
//                        else {
//                            IloExpr n1(env);
//                            IloExpr n(env);
//                            for (int f1 = 0;f1 < SFC[i + 1].size();f1++) {
//                                n1 += NODE[i + 1][b1][0][SFC[i + 1][f1]][u];
//                            }
//                            for (int f = 0;f < SFC[i].size();f++) {
//                                n += NODE[i][b][B[i] - 1][SFC[i][f]][u];
//                            }
//                            model.add((c - c1) - (n1 - n) == 0);
//                        }
//                        //model.add((c - c1) /* - (NODE[i + 1][b1][0][SFC[i + 1][f1]][u] - NODE[i][b][B[i] - 1][SFC[i][f]][u]) */ == 0);
//                    }
//
//
//                }
//            }
//        }
//
//        //BW constraints
//        for (int u = 0;u < N;u++) {
//            for (int v = 0;v < N;v++) {
//                IloExpr bw_expr(env);
//
//                //intra PE BW constraint
//                for (int b = 0;b < B[a];b++) {
//                    for (int s = 0;s < B[a] - 1;s++) {
//                        bw_expr += (arrival_SFC * EDGE[a][a][b][b][s][s + 1][u][v]);
//                    }
//                }
//
//                //inter PE BW constraint
//                for (int i = 0;i < SFC.size() - 1;i++) {
//                    for (int b = 0;b < B[i];b++) {
//                        for (int b1 = 0;b1 < B[i + 1];b1++) {
//                            bw_expr += (arrival_SFC * EDGE[i][i + 1][b][b1][B[i] - 1][0][u][v]);
//                        }
//                    }
//                }
//
//                model.add(bw_expr <= bw[u][v]);
//            }
//        }
//
//        //node resource constraint
//        for (int u = 0;u < N;u++) {
//            for (int f = 0;f < funcs + 2 + temp;f++) {
//                IloExpr cap_expr(env);
//                for (int i = 0;i < SFC.size();i++) {
//                    for (int b = 0;b < B[i];b++) {
//                        for (int s = 0;s < B[i];s++) {
//                            cap_expr += (arrival_SFC * NODE[i][b][s][f][u]);
//                        }
//                    }
//                }
//                model.add(cap_expr <= cap[u][f]);
//            }
//        }
//
//        //ensuring function f on u is picked only if f is deployed on u
//        for (int i = 0;i < SFC.size();i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int s = 0;s < B[i];s++) {
//                    for (int f = 0;f < funcs + 2 + temp;f++) {
//                        for (int u = 0;u < N;u++) {
//                            model.add(NODE[i][b][s][f][u] <= dep[u][f]);
//                        }
//                    }
//                }
//            }
//        }
//        // ensuring edges are picked which exist in the topology
//        for (int i = 0;i < SFC.size();i++) {
//            for (int i1 = 0;i1 < SFC.size();i1++) {
//                for (int b = 0;b < B[i];b++) {
//                    for (int b1 = 0;b1 < B[i1];b1++) {
//                        for (int s = 0;s < B[i];s++) {
//                            for (int s1 = 0;s1 < B[i1];s1++) {
//                                for (int u = 0;u < N;u++) {
//                                    for (int v = 0;v < N;v++) {
//                                        if (g[u][v] == 0) {
//                                            model.add(EDGE[i][i1][b][b1][s][s1][u][v] == 0);
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//
//        for (int i = 0;i < SFC.size();i++) {
//            for (int i1 = 0;i1 < SFC.size();i1++) {
//                for (int b = 0;b < B[i];b++) {
//                    for (int b1 = 0;b1 < B[i1];b1++) {
//                        for (int s = 0;s < B[i];s++) {
//                            for (int s1 = 0;s1 < B[i1];s1++) {
//                                for (int u = 0;u < N;u++) {
//                                    for (int v = 0;v < N;v++) {
//                                        if (i1 != i && i1 != i + 1)
//                                            model.add(EDGE[i][i1][b][b1][s][s1][u][v] == 0);
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        //for (int i = SFC.size()-1;i >= 0;i--) {
//        //    for (int i1 = i - 1;i1 >= 0;i1--) {
//        //        for (int b = 0;b < B[i];b++) {
//        //            for (int b1 = 0;b1 < B[i1];b1++) {
//        //                for (int s = 0;s < B[i];s++) {
//        //                    for (int s1 = 0;s1 < B[i1];s1++) {
//        //                        for (int u = 0;u < N;u++) {
//        //                            for (int v = 0;v < N;v++) {
//        //                                model.add(EDGE[i][i1][b][b1][s][s1][u][v] == 0);
//        //                            }
//        //                        }
//        //                    }
//        //                }
//        //            }
//        //        }
//        //    }
//        //}
//        for (int i = 0;i < SFC.size();i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int s = 0;s < B[i];s++) {
//                    for (int u = 0;u < N;u++) {
//                        for (int v = 0;v < N;v++) {
//                            model.add(EDGE[i][i][b][b][s][s][u][v] == 0);
//                        }
//                    }
//                }
//            }
//        }
//        for (int i = 0;i < SFC.size();i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int b1 = 0;b1 < B[i];b1++) {
//                    if (b1 != b) {
//                        for (int s = 0;s < B[i];s++) {
//                            for (int s1 = 0;s1 < B[i];s1++) {
//                                for (int u = 0;u < N;u++) {
//                                    for (int v = 0;v < N;v++) {
//                                        model.add(EDGE[i][i][b][b1][s][s1][u][v] == 0);
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        for (int i = 0;i < SFC.size() - 1;i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int b1 = 0;b1 < B[i + 1];b1++) {
//                    for (int s = 0;s < B[i];s++) {
//                        for (int s1 = 0;s1 < B[i + 1];s1++) {
//                            for (int u = 0;u < N;u++) {
//                                for (int v = 0;v < N;v++) {
//                                    if (s != B[i] - 1 && s1 != 0)
//                                        model.add(EDGE[i][i + 1][b][b1][s][s1][u][v] == 0);
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        for (int i = 0;i < SFC.size();i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int s = 0;s < B[i];s++) {
//                    for (int s1 = 0;s1 < B[i];s1++) {
//                        for (int u = 0;u < N;u++) {
//                            for (int v = 0;v < N;v++) {
//                                if (s1 != s && s1 != s + 1)
//                                    model.add(EDGE[i][i][b][b][s][s1][u][v] == 0);
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        // e2e latency constraint
//        model.add(e2e_expr <= T_l);
//#pragma endregion
//
//        IloCplex cplex(model);
//        cplex.setParam(IloCplex::RootAlg, IloCplex::MIP);
//        cplex.setOut(env.getNullStream());
//        cout << "MIP:" << cplex.isMIP() << endl;
//        cout << "QO:" << cplex.isQO() << endl;
//        cout << "QC:" << cplex.isQC() << endl;
//        cplex.solve();
//        std::cout << cplex.getStatus() << endl;
//        //if (!cplex.solve()) {
//        //    env.error() << "Failed to optimize the Master Problem!!!" << endl;
//        //    throw(-1);
//        //}
//
//        double Objval = cplex.getObjValue();
//        std::cout << "Objective::::" << Objval << endl;
//        vector<int> ctr(SFC.size(), 0);
//        for (int i = 0;i < SFC.size();i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int s = 0;s < B[i];s++) {
//                    for (int f = 0;f < funcs + 2 + temp;f++) {
//                        for (int u = 0;u < N;u++) {
//                            double NODEval = cplex.getValue(NODE[i][b][s][f][u]);
//                            if (NODEval == 1) {
//                                ctr[i]++;
//                                std::cout << "(" << i << "," << b << "," << s << "," << f << "," << u << ": " << NODEval << endl;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        for (int i = 0;i < ctr.size();i++) {
//            std::cout << i << ": " << ctr[i] << endl;
//        }
//        std::cout << "Theta: " << B[a] * cplex.getValue(theta[0]) << endl;
//        double e = 0;
//        for (int b = 0;b < B[a];b++) {
//            for (int s = 0;s < B[a];s++) {
//                for (int f = 0;f < funcs + 2 + temp;f++) {
//                    for (int u = 0;u < N;u++) {
//                        e += (t_f[f] * cplex.getValue(NODE[a][b][s][f][u]));
//                    }
//                }
//            }
//            for (int s = 0;s < B[a] - 1;s++) {
//                //for (int s1 = s+1;s1 < B[a];s1++) {
//                //    for (int u = 0;u < N;u++) {
//                //        for (int v = 0;v < N;v++) {
//                //            edge_expr += (t_uv[u][v] * EDGE[a][a][b][b][s][s1][u][v]);
//                //        }
//                //    }
//                //}
//                for (int u = 0;u < N;u++) {
//                    for (int v = 0;v < N;v++) {
//                        e += (t_uv[u][v] * cplex.getValue(EDGE[a][a][b][b][s][s + 1][u][v]));
//                    }
//                }
//                //for (int u = 0;u < N;u++) {
//                //    for (int v = 0;v < N;v++) {
//                //        e += (t_uv[u][v] * (cplex.getValue(EDGE[a - 1][a][0][b][0][s][u][v]) + cplex.getValue(EDGE[a][a + 1][b][0][s][0][u][v])));
//                //    }
//                //}
//            }
//            for (int u = 0;u < N;u++) {
//                for (int v = 0;v < N;v++) {
//                    e += (t_uv[u][v] * (cplex.getValue(EDGE[a - 1][a][0][b][0][0][u][v]) + cplex.getValue(EDGE[a][a + 1][b][0][B[a] - 1][0][u][v])));
//                }
//            }
//        }
//        std::cout << "Diff: " << e << endl;
//
//        std::cout << "Final e2e latency::::::" << cplex.getValue(e2e_expr) << endl;
//
//        for (int i = 0;i < SFC.size();i++) {
//            for (int i1 = 0;i1 < SFC.size();i1++) {
//                for (int b = 0;b < B[i];b++) {
//                    for (int b1 = 0;b1 < B[i1];b1++) {
//                        for (int s = 0;s < B[i];s++) {
//                            for (int s1 = 0;s1 < B[i1];s1++) {
//                                for (int u = 0;u < N;u++) {
//                                    for (int v = 0;v < N;v++) {
//                                        try {
//                                            if (cplex.getValue(EDGE[i][i1][b][b1][s][s1][u][v])) {
//                                                std::cout << "(" << i << "," << i1 << "," << b << "," << b1 << "," << s << "," << s1 << "," << u << "," << v << ") " << cplex.getValue(EDGE[i][i1][b][b1][s][s1][u][v]) << endl;
//                                            }
//                                        }
//                                        catch (IloException e) {
//                                            continue;
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        //for (int i = 0;i < SFC.size() - 1;i++) {
//        //    for (int b = 0;b < B[i];b++) {
//        //        for (int b1 = 0;b1 < B[i + 1];b1++) {
//        //            for (int f = 0;f < SFC[i].size();f++) {
//        //                for (int f1 = 0;f1 < SFC[i + 1].size();f1++) {
//        //                    for (int u = 0;u < N;u++) {
//        //                        IloExpr c(env);
//        //                        IloExpr c1(env);
//        //                        for (int v = 0;v < N;v++) {
//        //                            if (g[u][v] == 1) {
//        //                                cout << "(" << i << "," << i+1 << "," << b << "," << b1 << "," << B[i] - 1 << "," << 0 << "," << u << "," << v << ") " << cplex.getValue(EDGE[i][i + 1][b][b1][B[i] - 1][0][v][u]) << endl;
//        //                            }
//        //                        }
//        //                    }
//        //                }
//        //            }
//        //        }
//        //    }
//        //}
//
//        //network update
//            //VM cap update
//        for (int i = 0;i < SFC.size();i++) {
//            for (int b = 0;b < B[i];b++) {
//                for (int s = 0;s < B[i];s++) {
//                    for (int f = 0;f < funcs + 2 + temp;f++) {
//                        for (int u = 0;u < N;u++) {
//                            double NODEval = cplex.getValue(NODE[i][b][s][f][u]);
//                            if (NODEval == 1) {
//                                cap[u][f] -= arrival_SFC;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//            // BW update
//        for (int i = 0;i < SFC.size();i++) {
//            for (int i1 = 0;i1 < SFC.size();i1++) {
//                for (int b = 0;b < B[i];b++) {
//                    for (int b1 = 0;b1 < B[i1];b1++) {
//                        for (int s = 0;s < B[i];s++) {
//                            for (int s1 = 0;s1 < B[i1];s1++) {
//                                for (int u = 0;u < N;u++) {
//                                    for (int v = 0;v < N;v++) {
//                                        try {
//                                            if (cplex.getValue(EDGE[i][i1][b][b1][s][s1][u][v])) {
//                                                bw[u][v] -= arrival_SFC;
//                                                bw[v][u] -= arrival_SFC;
//                                            }
//                                        }
//                                        catch (IloException e) {
//                                            continue;
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//
//        cout << "CAP::::\n";
//        for (int u = 0;u < N;u++) {
//            for (int f = 0;f < funcs + 2 + temp;f++) {
//                cout << f << "," << u << ": " << cap[u][f] << endl;
//            }
//        }
//        cout << "BW:::::\n";
//        for (int u = 0;u < N;u++) {
//            for (int v = 0;v < N;v++) {
//                cout << u << "," << v << ": " << bw[u][v] << endl;
//            }
//        }
//    }
//    catch (IloException& e) {
//        cerr << "Concert exception caught: " << e << endl;
//    }
//    catch (...) {
//        cerr << "Unknown exception caught" << endl;
//    }
//
//    env.end();
//
//
//    return 0;
//}
