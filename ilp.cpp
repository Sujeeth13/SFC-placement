#include <iostream>
#include <fstream>
#include "ilcplex/ilocplex.h"
#include <vector>

#define INF 100000

typedef IloArray<IloArray<IloArray<IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>>>>> NumVar8d;
typedef IloArray<IloArray<IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>>>> NumVar7d;
typedef IloArray<IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>>> NumVar6d;
typedef IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>> NumVar5d;
typedef IloArray<IloArray<IloArray<IloNumVarArray>>> NumVar4d;
typedef IloArray<IloArray<IloNumVarArray>> NumVar3d;
typedef IloArray<IloNumVarArray> NumVar2d;

using namespace std;

int main() {
    ifstream fin("graph_config.txt");
    ifstream rin("request.txt");

    int N, M;
    int funcs, n_of_requests, n_of_f_instances;

    //rin>>n_of_requests;

    fin >> funcs >> n_of_f_instances;
    fin >> N >> M;
    fin >> n_of_requests;

    //vector<vector<int>> SFC{ {10}, {0}, {1},{4},{11} };
    vector<vector<int>> SFC{ {10}, {0}, {1,2},{4},{11} };
    int src = 0;
    int dest = 2;
    int a = 2;
    vector<double> t_f(funcs + 2+(SFC[a].size()*(SFC[a].size() - 1)), 0);
    vector<vector<int>> g(N,vector<int>(N,0));
    vector<vector<double>> bw(N, vector<double>(N, 0));
    vector<vector<double>> t_uv(N, vector<double>(N, 0));

    double copy_time = 2, merge_time = 2;
    double T_l = 500;

    // creating the graph
    for (int i = 0;i < N;i++) {
        int s, d;
        double l, b;
        fin >> s >> d >> l >> b;
        g[s][d] = 1;
        g[d][s] = 1;
        t_uv[s][d] = l;
        t_uv[d][s] = l;
        bw[s][d] = b;
        bw[d][s] = b;
    }

    //getting func time value
    for (int i = 0;i < funcs;i++) {
        int f_id;
        double p_time;
        rin>>f_id>>p_time;
        t_f[f_id] = p_time;
    }

    std::cout << "Graph\n";
    for (int i = 0;i < N;i++) {
        for (int j = 0;j < N;j++) {
            std::cout << g[i][j] << " ";
        }
        std::cout << endl;
    }

    std::cout << "Links\n";
    for (int i = 0;i < N;i++) {
        for (int j = 0;j < N;j++) {
            std::cout << t_uv[i][j] << " ";
        }
        std::cout << endl;
    }

    std::cout << "BW\n";
    for (int i = 0;i < N;i++) {
        for (int j = 0;j < N;j++) {
            std::cout << bw[i][j] << " ";
        }
        std::cout << endl;
    }

    std::cout << "Funcs\n";
    for (int i = 0;i < t_f.size();i++) {
        std::cout << i << ": " << t_f[i] << endl;
    }

    int temp = (SFC[a].size() * (SFC[a].size() - 1));
    vector<vector<int>> dep(N, vector<int>(funcs+2+temp, 0));
    vector<vector<int>> cap(N, vector<int>(funcs+2+temp, 0));

    //function deployed instance
    //dep[0][10] = 1;
    //dep[6][11] = 1;
    //dep[1][2] = 1;
    //dep[1][4] = 1;
    //dep[2][0] = 1;
    //dep[2][3] = 1;
    //dep[3][0] = 1;
    //dep[4][1] = 1;
    //dep[4][2] = 1;
    //dep[5][1] = 1;
    dep[0][10] = 1;
    dep[0][0] = 1;
    dep[1][1] = 1;
    dep[2][2] = 1;
    dep[2][11] = 1;
    dep[3][4] = 1;

    //deployed instance capacity
    //cap[1][2] = 10;
    //cap[1][4] = 10;
    //cap[2][0] = 10;
    //cap[2][3] = 10;
    //cap[3][0] = 10;
    //cap[4][1] = 10;
    //cap[4][2] = 10;
    //cap[5][1] = 10;

    std::cout << "SFC before: \n";
    for (int i = 0;i < SFC.size();i++) {
        for (int j = 0;j < SFC[i].size();j++) {
            std::cout << SFC[i][j] << ", ";
        }
        std::cout << endl;
    }

    vector<int> B(SFC.size(),1);
    B[a] = SFC[a].size();
    double arrival_SFC = 1;
    
    //pushing the dummy functions into the SFC
    for (int i = 0;i < temp;i++) {
        SFC[a].push_back(funcs + 2 + i);
    }

    std::cout << "SFC after: \n";
    for (int i = 0;i < SFC.size();i++) {
        for (int j = 0;j < SFC[i].size();j++) {
            std::cout << SFC[i][j] << ", ";
        }
        std::cout << endl;
    }

    //making the dummy functions available everywhere
    for (int i = 0;i < temp;i++) {
        for (int u = 0;u < N;u++) {
            dep[u][funcs + 2 + i] = 1;
            cap[u][funcs + 2 + i] = INF;
        }
    }

    IloEnv env;
    try {
        IloModel model(env);
        # pragma region Node Variable
        NumVar5d NODE(env,SFC.size());
        for (int i = 0;i < SFC.size();i++) {
            NumVar4d t1(env, B[i]);
            for (int b = 0;b < B[i];b++) {
                NumVar3d t2(env, B[i]);
                for (int s = 0;s < B[i];s++) {
                    NumVar2d t3(env, funcs + 2+temp);
                    for (int f = 0;f < funcs + 2+temp;f++) {
                        IloNumVarArray t4(env, N,0,1,ILOINT);
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

        IloNumVarArray theta(env, 1,0,IloInfinity, ILOFLOAT);
        double param = 1;
        #pragma region Objective function
            IloExpr node_expr(env);
            IloExpr edge_expr(env);
            for (int b = 0;b < B[a];b++) {
                for (int s = 0;s < B[a];s++) {
                    for (int f = 0;f < funcs + 2;f++) {
                        for (int u = 0;u < N;u++) {
                            node_expr += (t_f[f] * NODE[a][b][s][f][u]);
                        }
                    }
                }
                for (int s = 0;s < B[a]-1;s++) {
                    //for (int s1 = s+1;s1 < B[a];s1++) {
                    //    for (int u = 0;u < N;u++) {
                    //        for (int v = 0;v < N;v++) {
                    //            edge_expr += (t_uv[u][v] * EDGE[a][a][b][b][s][s1][u][v]);
                    //        }
                    //    }
                    //}
                    for (int u = 0;u < N;u++) {
                        for (int v = 0;v < N;v++) {
                            edge_expr += (t_uv[u][v] * EDGE[a][a][b][b][s][s+1][u][v]);
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
            for (int i = 0;i < a - 1;i++) {
                for (int f = 0;f < B[i];f++) {
                    for (int u = 0;u < N;u++) {
                        e2e_expr += NODE[i][0][0][SFC[i][f]][u] * t_f[SFC[i][f]];
                    }
                }
                for (int u = 0;u < N;u++) {
                    for (int v = 0;v < N;v++) {
                        e2e_expr += EDGE[i][i + 1][0][0][0][0][u][v] * t_uv[u][v];
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
                        e2e_expr += EDGE[i][i + 1][0][0][0][0][u][v] * t_uv[u][v];
                    }
                }
            }
            e2e_expr += theta[0];

            IloExpr obj(env);
            obj += param*((B[a] * theta[0]) - (node_expr + edge_expr)) + (1-param)*e2e_expr;
            model.add(obj >= 0);
            model.add(IloMinimize(env,obj));

        #pragma endregion

        #pragma region Constraints
            // setting constraint for the auxilary varaible theta
            for (int b = 0;b < B[a];b++) {
                IloExpr node_c(env);
                IloExpr edge_c(env);
                for (int s = 0;s < B[a];s++) {
                    for (int f = 0;f < B[a];f++) {
                        for (int u = 0;u < N;u++) {
                            node_c += (t_f[SFC[a][f]] * NODE[a][b][s][f][u]);
                        }
                    }
                }
                for (int s = 0;s < B[a]-1;s++) {
                    //for (int s1 = s + 1;s1 < B[a];s1++) {
                    //    for (int u = 0;u < N;u++) {
                    //        for (int v = 0;v < N;v++) {
                    //            edge_c += (t_uv[u][v] * EDGE[a][a][b][b][s][s1][u][v]);
                    //        }
                    //    }
                    //}
                    for (int u = 0;u < N;u++) {
                        for (int v = 0;v < N;v++) {
                            edge_c += (t_uv[u][v] * EDGE[a][a][b][b][s][s + 1][u][v]);
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
                        for (int f = 0;f < funcs + 2+temp;f++) {
                            for (int u = 0;u < N;u++) {
                                c += NODE[i][b][s][f][u];
                            }
                        }
                    }
                }
                //model.add(c == SFC[i].size());
            }
            //ensures each function is picked only once
            for (int f = 0;f < funcs + 2+temp;f++) {
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
                vector<bool> fs(funcs + 2+temp, false);
                for (int t = 0;t < SFC[i].size();t++) {
                    fs[SFC[i][t]] = true;
                }
                for (int b = 0;b < B[i];b++) {
                    for (int s = 0;s < B[i];s++) {
                        for (int f = 0;f < funcs + 2+temp;f++) {
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
                    for (int f = 0;f < SFC[a].size();f++) {
                        for (int f1 = 0;f1 < SFC[a].size();f1++) {
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
                                model.add((c - c1) - (NODE[a][b][s + 1][SFC[a][f1]][u] - NODE[a][b][s][SFC[a][f]][u]) == 0);
                            }
                        }
                    }
                }
            }
            //flow conservation between PEs
            for (int i = 0;i < SFC.size()-1;i++) {
                for (int b = 0;b < B[i];b++) {
                    for (int b1 = 0;b1 < B[i + 1];b1++) {
                        for (int f = 0;f < SFC[i].size();f++) {
                            for (int f1 = 0;f1 < SFC[i+1].size();f1++) {
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
                                    model.add((c - c1) - (NODE[i + 1][b1][0][SFC[i + 1][f1]][u] - NODE[i][b][B[i] - 1][SFC[i][f]][u]) == 0);
                                }
                            }
                        }
                    }
                }
            }

            for (int b = 0;b < B[0];b++) {
                for (int s = 0;s < B[0];s++) {
                    for (int f = 0; f < funcs + 2 + temp; f++) {
                        IloExpr f_expr(env);
                        for (int u = 0;u < N;u++) {
                            f_expr += NODE[0][b][s][f][u];
                            if (f == 10) {
                                if (u == src) {
                                    model.add(NODE[0][b][s][f][u] == 1);
                                }
                                else {
                                    model.add(NODE[0][b][s][f][u] == 0);
                                }
                            }
                        }
                        if (f != 10)
                            model.add(f_expr == 0);
                    }
                }
            }

            for (int b = 0;b < B[SFC.size()-1];b++) {
                for (int s = 0;s < B[SFC.size()-1];s++) {
                    for (int f = 0; f < funcs + 2 + temp; f++) {
                        IloExpr f_expr(env);
                        for (int u = 0;u < N;u++) {
                            f_expr += NODE[SFC.size()-1][b][s][f][u];
                            if (f == 11) {
                                if (u == dest) {
                                    model.add(NODE[SFC.size()-1][b][s][f][u] == 1);
                                }
                                else {
                                    model.add(NODE[SFC.size()-1][b][s][f][u] == 0);
                                }
                            }
                        }
                        if (f != 11)
                            model.add(f_expr == 0);
                    }
                }
            }
            //for (int b = 0;b < B[a];b++) {
            //    for (int i = 0;i < SFC.size()-1;i++) {
            //        if (i == a - 1) {
            //            for (int f = 0;f < SFC[i].size();f++) {
            //                for (int f1 = 0;f1 < SFC[i + 1].size();f1++) {
            //                    for (int u = 0;u < N;u++) {
            //                        IloExpr c(env);
            //                        IloExpr c1(env);
            //                        for (int v = 0;v < N;v++) {
            //                            if (g[u][v] == 1) {
            //                                c += EDGE[i][i + 1][0][b][0][0][v][u];
            //                            }
            //                        }
            //                        for (int w = 0;w < N;w++) {
            //                            if (g[u][w] == 1) {
            //                                c1 += EDGE[i][i + 1][0][b][0][0][u][w];
            //                            }
            //                        }
            //                        model.add((c - c1) - (NODE[i + 1][b][0][SFC[i + 1][f1]][u] - NODE[i][0][0][SFC[i][f]][u]) == 0);
            //                    }
            //                }
            //            }
            //        }
            //        else  if (i == a) {
            //            for (int s = 0;s < B[a] - 1;s++) {
            //                for (int f = 0;f < SFC[a].size();f++) {
            //                    for (int f1 = 0;f1 < SFC[a].size();f1++) {
            //                        for (int u = 0;u < N;u++) {
            //                            IloExpr c(env);
            //                            IloExpr c1(env);
            //                            for (int v = 0;v < N;v++) {
            //                                if (g[u][v] == 1) {
            //                                    c += EDGE[a][a][b][b][s][s + 1][v][u];
            //                                }
            //                            }
            //                            for (int w = 0;w < N;w++) {
            //                                if (g[u][w] == 1) {
            //                                    c1 += EDGE[a][a][b][b][s][s + 1][u][w];
            //                                }
            //                            }
            //                            model.add((c - c1) - (NODE[a][b][s + 1][SFC[a][f1]][u] - NODE[a][b][s][SFC[a][f]][u]) == 0);
            //                        }
            //                    }
            //                }
            //            }
            //            for (int f = 0;f < SFC[i].size();f++) {
            //                for (int f1 = 0;f1 < SFC[i + 1].size();f1++) {
            //                    for (int u = 0;u < N;u++) {
            //                        IloExpr c(env);
            //                        IloExpr c1(env);
            //                        for (int v = 0;v < N;v++) {
            //                            if (g[u][v] == 1) {
            //                                c += EDGE[i][i + 1][b][0][B[i] - 1][0][v][u];
            //                            }
            //                        }
            //                        for (int w = 0;w < N;w++) {
            //                            if (g[u][w] == 1) {
            //                                c1 += EDGE[i][i + 1][b][0][B[i] - 1][0][u][w];
            //                            }
            //                        }
            //                        model.add((c - c1) - (NODE[i + 1][0][0][SFC[i + 1][f1]][u] - NODE[i][b][B[i] - 1][SFC[i][f]][u]) == 0);
            //                    }
            //                }
            //            }
            //        }
            //        else {
            //            for (int f = 0;f < SFC[i].size();f++) {
            //                for (int f1 = 0;f1 < SFC[i + 1].size();f1++) {
            //                    for (int u = 0;u < N;u++) {
            //                        IloExpr c(env);
            //                        IloExpr c1(env);
            //                        for (int v = 0;v < N;v++) {
            //                            if (g[u][v] == 1) {
            //                                c += EDGE[i][i + 1][0][0][0][0][v][u];
            //                            }
            //                        }
            //                        for (int w = 0;w < N;w++) {
            //                            if (g[u][w] == 1) {
            //                                c1 += EDGE[i][i + 1][0][0][0][0][u][w];
            //                            }
            //                        }
            //                        model.add((c - c1) - (NODE[i + 1][0][0][SFC[i + 1][f1]][u] - NODE[i][0][0][SFC[i][f]][u]) == 0);
            //                    }
            //                }
            //            }
            //        }
            //    }
            //}
            ////BW constraints within branch
            //
            //ensuring function f on u is picked only if f is deployed on u
            for (int i = 0;i < SFC.size();i++) {
                for (int b = 0;b < B[i];b++) {
                    for (int s = 0;s < B[i];s++) {
                        for (int f = 0;f < funcs + 2+temp;f++) {
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
                for (int i1 = i+2;i1 < SFC.size();i1++) {
                    for (int b = 0;b < B[i];b++) {
                        for (int b1 = 0;b1 < B[i1];b1++) {
                            for (int s = 0;s < B[i];s++) {
                                for (int s1 = 0;s1 < B[i1];s1++) {
                                    for (int u = 0;u < N;u++) {
                                        for (int v = 0;v < N;v++) {
                                            model.add(EDGE[i][i1][b][b1][s][s1][u][v] == 0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            for (int i = SFC.size()-1;i >= 0;i--) {
                for (int i1 = i - 1;i1 >= 0;i1--) {
                    for (int b = 0;b < B[i];b++) {
                        for (int b1 = 0;b1 < B[i1];b1++) {
                            for (int s = 0;s < B[i];s++) {
                                for (int s1 = 0;s1 < B[i1];s1++) {
                                    for (int u = 0;u < N;u++) {
                                        for (int v = 0;v < N;v++) {
                                            model.add(EDGE[i][i1][b][b1][s][s1][u][v] == 0);
                                        }
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
                    for (int b1 = b + 1;b1 < B[i];b1++) {
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
            for (int i = 0;i < SFC.size();i++) {
                for (int b = 0;b < B[i];b++) {
                    for (int s = 0;s < B[i];s++) {
                        for (int s1 = s + 2;s1 < B[i];s1++) {
                            for (int u = 0;u < N;u++) {
                                for (int v = 0;v < N;v++) {
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
            cplex.setOut(env.getNullStream());
            cplex.solve();
            std::cout << cplex.getStatus() << endl;
            //if (!cplex.solve()) {
            //    env.error() << "Failed to optimize the Master Problem!!!" << endl;
            //    throw(-1);
            //}

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
            std::cout << "Theta: " << B[a]*cplex.getValue(theta[0]) << endl;
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
                        e += (t_uv[u][v] * (cplex.getValue(EDGE[a - 1][a][0][b][0][0][u][v]) + cplex.getValue(EDGE[a][a + 1][b][0][B[a]-1][0][u][v])));
                    }
                }
            }
            std::cout << "Diff: " << e << endl;

            double e2e_val = 0;
            e2e_val += (B[a] - 1) * (copy_time + merge_time);
            for (int i = 0;i < a - 1;i++) {
                for (int f = 0;f < B[i];f++) {
                    for (int u = 0;u < N;u++) {
                        e2e_val += cplex.getValue(NODE[i][0][0][SFC[i][f]][u]) * t_f[SFC[i][f]];
                    }
                }
                for (int u = 0;u < N;u++) {
                    for (int v = 0;v < N;v++) {
                        e2e_val += cplex.getValue(EDGE[i][i + 1][0][0][0][0][u][v]) * t_uv[u][v];
                    }
                }
            }
            for (int i = a + 1;i < SFC.size() - 1;i++) {
                for (int f = 0;f < B[i];f++) {
                    for (int u = 0;u < N;u++) {
                        e2e_val += cplex.getValue(NODE[i][0][0][SFC[i][f]][u]) * t_f[SFC[i][f]];
                    }
                }
                for (int u = 0;u < N;u++) {
                    for (int v = 0;v < N;v++) {
                        e2e_val += cplex.getValue(EDGE[i][i + 1][0][0][0][0][u][v]) * t_uv[u][v];
                    }
                }
            }
            e2e_val += cplex.getValue(theta[0]);
            std::cout << "Final e2e latency::::::" << e2e_val << endl;

            for (int i = 0;i < SFC.size();i++) {
                for (int i1 = 0;i1 < SFC.size();i1++) {
                    for (int b = 0;b < B[i];b++) {
                        for (int b1 = 0;b1 < B[i1];b1++) {
                            for (int s = 0;s < B[i];s++) {
                                for (int s1 = 0;s1 < B[i1];s1++) {
                                    for (int u = 0;u < N;u++) {
                                        for (int v = 0;v < N;v++) {
                                            if(g[u][v] == 1)
                                            if (cplex.getValue(EDGE[i][i1][b][b1][s][s1][u][v])) {
                                                std::cout << "(" << i << "," << i1 << "," << b << "," << b1 << "," << s << "," << s1 << ","<<u << ","<<v << ") " << cplex.getValue(EDGE[i][i1][b][b1][s][s1][u][v]) << endl;
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
    }
    catch (IloException& e) {
        cerr << "Concert exception caught: " << e << endl;
    }
    catch (...) {
        cerr << "Unknown exception caught" << endl;
    }

    env.end();
    return 0;
}
