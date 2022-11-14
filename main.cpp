#include <bits/stdc++.h>
#include "yens.cpp"
using namespace std;

// class g_nodes{
//     public:
//         int node;
//         double capacity;
//         g_nodes(){
//             node = 0;
//             capacity = 0;
//         }
//         g_nodes(int n,double c){
//             node = n;
//             capacity = c;
//         }
//         g_nodes(const g_nodes& a){
//             node = a.node;
//             capacity = a.capacity;
//         }

// };

// class node{
//     public:
//         int id;   // temporary int 
//         double link;   // cost 
//         double available_bandwidth;     // bandwith
//         node(int id,double link,double bw){
//             this->id = id;
//             this->link = link;
//             this->available_bandwidth = bw;
//         }
// };

// class f_attr{  //contains the attributes of a network function
//     public:
//     int id;
//     double processing_t;
// };

// class node_capacity{ //contains the list of NFs installed in node and the amount left of each of those instances.
//     public:
//     map<int,double> NF_left; //maps a function instance to the amount of processing power left for that function(think NF shareability)
//     map<int,double> deployed_NF; // tells how much processing time that NF requires.
//     node_capacity(){}
//     node_capacity(int id,double time){
//         NF_left[id] = 1;
//         deployed_NF[id] = time;
//     }
// };

// class Request{  //represents the parameters of the SFC request
//     public:
//     int src;
//     int dest;
//     vector<vector<vector<int>>> SFC;
//     double e2e;
//     double t_arrival_rate;
//     Request(){}
//     Request(int srd,int dest,vector<vector<vector<int>>> SFC,double e2e,double t_arrival_rate){
//         this->src = src;
//         this->dest = dest;
//         this->SFC = SFC;
//         this->e2e = e2e;
//         this->t_arrival_rate = t_arrival_rate;
//     }
// };

//SFC = {{{0}},{{1,2},{3}},{{4}}};

vector<vector<vector<int>>> bin(vector<vector<vector<int>>> SFC,map<int,double> NFs){
    vector<vector<vector<int>>> temp;

    for(int i=0;i<SFC.size();i++){ //for each Ci
        if(SFC[i].size() == 1){
            temp.push_back(SFC[i]);
            continue;;
        }
        double max_process = -1;
        int max_func;
        for(int j=0;j<SFC[i].size();j++){ // for each bx in Ci
            for(int k=0;k<SFC[i][j].size();k++){ //finding the max f_time
                if(NFs[SFC[i][j][k]] > max_process){
                    max_process = NFs[SFC[i][j][k]];
                    max_func = SFC[i][j][k];
                }
            }
        }
        cout<<"MAX FUNC:::::"<<max_func<<endl;
        double cap = max_process;
        vector<vector<int>> new_Ci;
        double cap_curr=0;
        vector<int> bin;
        for(int b=0;b<SFC[i].size();b++){
            for(int f=0;f<SFC[i][b].size();f++){
                if(SFC[i][b][f] == max_func)
                    continue;
                if(cap_curr + NFs[SFC[i][b][f]] < cap){
                    cap_curr += NFs[SFC[i][b][f]];
                    bin.push_back(SFC[i][b][f]);
                }
                else{
                    cap_curr = 0;
                    new_Ci.push_back(bin);
                    bin.clear();
                    cap_curr += NFs[SFC[i][b][f]];
                    bin.push_back(SFC[i][b][f]);
                }
            }
        }
        if(!bin.empty())
            new_Ci.push_back(bin);
        new_Ci.push_back({max_func});
        temp.push_back(new_Ci);

    }
    return temp;
}

void SFC_embedding(vector<vector<node>> g,vector<node_capacity>& n_resource,vector<vector<int>>& NF_to_node,map<int,double>& NFs,Request request){

    // vector<vector<int>> k_paths = YenKSP(g,request,3);
    // for(int i=0;i<k_paths.size();i++){
    //     cout<<"Path"<<i<<":"<<endl;
    //     for(int j=0;j<k_paths[i].size();j++){
    //         cout<<k_paths[i][j]<<"->";
    //     }
    //     cout<<endl;
    // }
    map<int,int> deployed_inst;
    map<int,double> time; //map that contains the reach time to a NF in the chain
    double dest_time;
    double pkt_copy=2,pkt_merge=2;
    //initializing reach time to that NF in the chain to 0
    vector<vector<vector<int>>> SFC = request.SFC;
    int src = request.src;
    int dest = request.dest;
    double e2e = request.e2e;
    double t_arrival_rate = request.t_arrival_rate;

    for(int i=0;i<g[src].size();i++){
        for(int j=0;j<g[g[src][i].id].size();j++){
            if(g[g[src][i].id][j].id == src)
                g[g[src][i].id].erase(g[g[src][i].id].begin()+j);
        }
    }

    g[dest].clear();
    
    int K = 2; //the number of K shortest paths
    vector<vector<vector<vector<int>>>> paths(g.size(),vector<vector<vector<int>>>(g.size()));
    cout<<paths.size()<<paths[0].size()<<endl;
    for(int i=0;i<g.size();i++){
        for(int j=0;j<g.size();j++){
            vector<vector<int>> k_paths;
            if(i == j){
                for(int k=0;k<K;k++){
                    vector<int> temp;
                    temp.push_back(i);
                    k_paths.push_back(temp);
                }
            }
            else{
                //cout<<"here\n";
                k_paths = YenKSP(g,i,j,request,K);
                //cout<<"here1\n";
            }
            paths[i][j] = k_paths;
        }
    }
    // cout<<paths.size()<<paths[0].size()<<endl;
    // paths[0][0]= k_paths;
    vector<vector<vector<double>>> time_of_paths = calc_time(g,paths);
    // cout<<"Time:"<<endl;
    // for(int i=0;i<time_of_paths[0][0].size();i++){
    //     cout<<time_of_paths[0][0][i]<<" ";
    // }
    // cout<<endl;



    for(int i=0;i<SFC.size();i++){
        for(int j=0;j<SFC[i].size();j++){
            for(int k=0;k<SFC[i][j].size();k++){
                time[SFC[i][j][k]] = 0;
            }
        }
    }
    for(int i=0;i<time_of_paths.size();i++){
        for(int j=0;j<time_of_paths[i].size();j++){
            for(int k=0;k<time_of_paths[i][j].size();k++){
                cout<<i<<","<<j<<","<<k<<"::::\n"<<time_of_paths[i][j][k]<<endl;
                for(int p=0;p<paths[i][j][k].size();p++){
                    cout<<paths[i][j][k][p]<<"->";
                }
                cout<<endl;
            }
        }
    }
    // {{{0}},{{1,2},{3}},{{4}}}
    //find critical branches delay from src to the C0
    vector<int> critical_branch_node(SFC.size());
    for(int i=0;i<SFC.size();i++){
        critical_branch_node[i] = SFC[i][SFC[i].size()-1][SFC[i][SFC[i].size()-1].size()-1];
    }
    critical_branch_node[0] = SFC[0][SFC[0].size()-1][0];
    int inst_0;
    int min_dist = INT_MAX;
    cout<<"Error::::"<<NF_to_node[critical_branch_node[0]].size()<<endl;
    for(int i=0;i<NF_to_node[critical_branch_node[0]].size();i++){
        if(n_resource[NF_to_node[critical_branch_node[0]][i]].NF_left[critical_branch_node[0]] > t_arrival_rate){
            if(time_of_paths[src][NF_to_node[critical_branch_node[0]][i]][0] < min_dist){
                min_dist = time_of_paths[src][NF_to_node[critical_branch_node[0]][i]][0];
                inst_0 = NF_to_node[critical_branch_node[0]][i];
                cout<<"All inst::::"<<inst_0<<endl;
            }
        }
    }
    deployed_inst[critical_branch_node[0]] = inst_0;
    time[critical_branch_node[0]] = min_dist/*link delay*/ + NFs[critical_branch_node[0]]/*function delay*/ +(SFC[1].size()-1)*(pkt_copy)/*copy delay*/ ;
    cout<<"inst:::"<<inst_0<<endl;
    cout<<"Min dis:::"<<min_dist<<endl;
    cout<<"F delay:::"<<NFs[critical_branch_node[0]]<<endl;
    cout<<"Copy delay:::"<<(SFC[1].size()-1)*(pkt_copy)<<endl;
    cout<<"initial::::"<<time[critical_branch_node[0]]<<endl;
    for(int i=0;i<SFC.size()-1;i++){
        int inst;
        int min_dist = INT_MAX;
        int next_branch_node;
        for(int b=0;b<SFC[i+1].size();b++){
            if(SFC[i].size() == 1 && b!=SFC[i+1].size()-1)
                continue;
            next_branch_node = SFC[i+1][b][0];
            min_dist = INT_MAX;
            for(int j=0;j<NF_to_node[next_branch_node].size();j++){
                if(n_resource[NF_to_node[next_branch_node][j]].NF_left[next_branch_node] > t_arrival_rate){
                    if(time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0] < min_dist){
                        min_dist = time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0];
                        // for(int a=0;a<paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0].size();a++){
                        //     cout<<paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0][a]<<"->";
                        // }
                        // cout<<endl;
                        inst = NF_to_node[next_branch_node][j];
                    }
                }
            }
            deployed_inst[next_branch_node] = inst;
            time[next_branch_node] = time[critical_branch_node[i]] + min_dist/*link delay*/ + NFs[next_branch_node]/*function delay*/ +(SFC[i].size()-1)*(pkt_merge)/*merge delay*/ ;
                cout<<"Min dis:::"<<min_dist<<endl;
                cout<<"F delay:::"<<NFs[next_branch_node]<<endl;
                cout<<"Copy delay:::"<<(SFC[i].size()-1)*(pkt_merge)<<endl;
                cout<<"initial::::"<<time[next_branch_node]<<endl;
            if(i+2 <= SFC.size()-1 && SFC[i+1][b].size() == 1)
                time[next_branch_node] += (SFC[i+2].size()-1)*(pkt_copy);
        }
    }
    dest_time = time[critical_branch_node[SFC.size()-1]] + time_of_paths[deployed_inst[critical_branch_node[SFC.size()-1]]][dest][0] + (SFC[SFC.size()-1].size()-1)*(pkt_merge);
    for(int i=0;i<SFC.size();i++){
        for(int j=0;j<SFC[i].size();j++){
            for(int k=0;k<SFC[i][j].size();k++){
                cout<<i<<","<<j<<","<<k<<":::"<<time[SFC[i][j][k]]<<endl;
            }
        }
    }
    cout<<"e2e latency:::::::::::"<<dest_time<<endl;

    // layer graph step......
    // for(int i=0;i<SFC.size();i++){
    //     if(SFC[i].size() == 1)
    //         continue;
    //     for(int b=0;b<SFC[i].size()-1;b++){ //checking all branches except critical branch since all function instances have been deployed in the critical branch
    //         if(deployed_inst.find(SFC[i][b][0]) == deployed_inst.end()){
    //             if(i == 0){
    //                 for(int b_n=0;b_n<SFC[i+1].size();b_n++){
    //                     layer_graph(l_id(0,-1,src),SFC[i][b],l_id(0,SFC[i+1][b_n][0],deployed_inst[SFC[i+1][b_n][0]]),time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
    //                 }
    //             }
    //             else{
    //                 if(i == SFC.size()-1){
    //                     layer_graph(l_id(0,SFC[i-1][0][0],deployed_inst[SFC[i-1][0][0]]),SFC[i][b],l_id(0,-1,dest),time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
    //                 }
    //                 else{
    //                     for(int b_n=0;b_n<SFC[i+1].size();b_n++)
    //                         layer_graph(l_id(0,SFC[i-1][0][0],deployed_inst[SFC[i-1][0][0]]),SFC[i][b],l_id(0,SFC[i+1][b_n][0],deployed_inst[SFC[i+1][b_n][0]]),time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
    //                 }
    //             }
    //         }
    //         else{
    //             if(i == SFC.size()-1){
    //                 layer_graph(l_id(0,SFC[i][b][0],deployed_inst[SFC[i][b][0]]),SFC[i][b],l_id(0,-1,dest),time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
    //             }
    //             else{
    //                 for(int b_n=0;b_n<SFC[i+1].size();b_n++)
    //                     layer_graph(l_id(0,SFC[i][b][0],deployed_inst[SFC[i][b][0]]),SFC[i][b],l_id(0,SFC[i+1][b_n][0],deployed_inst[SFC[i+1][b_n][0]]),time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
    //             }
    //         }
    //     }
    // }
    for(int i=0;i<SFC.size();i++){
        if(SFC[i].size() == 1)
            continue;
        for(int b=0;b<SFC[i].size()-1;b++){ //checking all branches except critical branch since all function instances have been deployed in the critical branch
            if(deployed_inst.find(SFC[i][b][0]) == deployed_inst.end()){
                if(i == 0){
                    for(int b_n=0;b_n<SFC[i+1].size();b_n++){
                        layer_graph_2(src,SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
                    }
                }
                else{
                    if(i == SFC.size()-1){
                        layer_graph_2(deployed_inst[SFC[i-1][0][0]],SFC[i][b],dest,time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
                    }
                    else{
                        for(int b_n=0;b_n<SFC[i+1].size();b_n++)
                            layer_graph_2(deployed_inst[SFC[i-1][0][0]],SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
                    }
                }
            }
            else{
                if(i == SFC.size()-1){
                    layer_graph_2(deployed_inst[SFC[i][b][0]],SFC[i][b],dest,time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
                }
                else{
                    for(int b_n=0;b_n<SFC[i+1].size();b_n++)
                        layer_graph_2(deployed_inst[SFC[i][b][0]],SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],time,deployed_inst,NF_to_node,NFs,n_resource,paths,time_of_paths);
                }
            }
        }
    }
    cout<<"final e2e latency:::::::::::"<<dest_time<<endl;
}

int main(){

    ifstream fin("graph_config.txt");
    ifstream rin("request.txt");

    int N,M;
    int funcs,n_of_requests,n_of_f_instances;

    //rin>>n_of_requests;

    fin>>funcs>>n_of_f_instances;
    fin>>N>>M;

    vector<vector<node>> g(N); //network topology
    vector<node_capacity> n_resource(N); //tells all the function instances deployed in a node in the topology
    vector<vector<int>> NF_to_node(funcs); //tells in which node each function is deployed in
    vector<Request> requests(n_of_requests); //contains a batch of SFC requests
    map<int,double> NFs;
    for(int i=0;i<funcs;i++){
        int f_id;
        double p_time;
        rin>>f_id>>p_time;
        NFs[f_id] = p_time;
    }
    vector<vector<vector<int>>> SFC{{{0}},{{1},{2},{3}},{{4}}};

    SFC = bin(SFC,NFs);
    for(int i=0;i<SFC.size();i++){
        for(int j=0;j<SFC[i].size();j++){
            for(int k=0;k<SFC[i][j].size();k++){
                cout<<SFC[i][j][k]<<"->";
            }
            cout<<endl;
        }
        cout<<endl;
    }
    //SFC = {{{0}},{{1,2},{3}},{{4}}};
    for(int i=0;i<n_of_f_instances;i++){
        int f_id;
        double time;
        int node_id;
        fin>>node_id>>f_id>>time;
        // node_capacity temp(f_id,time);
        n_resource[node_id].NF_left[f_id] = 50;
        n_resource[node_id].deployed_NF[f_id] = time;
        NF_to_node[f_id].push_back(node_id);
    }
    cout<<"Node to funcs"<<endl;
    for(int i=0;i<N;i++){
        cout<<i<<":"<<endl;
        map<int,double>::iterator it;
        for(it=n_resource[i].deployed_NF.begin();it!=n_resource[i].deployed_NF.end();it++){
            cout<<it->first+1<<":"<<it->second<<endl;
        }
        cout<<endl;
    }
    cout<<"Funcs to Node"<<endl;
    for(int i=0;i<funcs;i++){
        cout<<i+1<<":"<<endl;
        for(int j=0;j<NF_to_node[i].size();j++){
            cout<<NF_to_node[i][j]<<" ";
        }
        cout<<endl;
    }

    for(int i=0;i<M;i++){
        int s,d;
        double l,b;
        fin>>s>>d>>l>>b;
        int temp1 =d ;
        int temp2 = s;
        node n1(temp1,l,b);
        node n2(temp2,l,b);
        g[s].push_back(n1);
        g[d].push_back(n2);
    }

    for(int i=0;i<g.size();i++){
        for(int j=0;j<g[i].size();j++){
            cout<<i<<"->"<<g[i][j].id<<"("<<g[i][j].link<<")"<<endl;
        }
    }
    Request request(0,6,SFC,270,12);
    SFC_embedding(g,n_resource,NF_to_node,NFs,request);

    // int K = 2; //the number of K shortest paths
    // vector<vector<vector<vector<int>>>> paths(g.size(),vector<vector<vector<int>>>(g.size()));
    // cout<<paths.size()<<paths[0].size()<<endl;
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
    //             k_paths = YenKSP(g,i,j,request,K);
    //             //cout<<"here1\n";
    //         }
    //         paths[i][j] = k_paths;
    //     }
    // }

    // vector<vector<vector<double>>> time_of_paths = calc_time(g,paths);

    // for(int i=0;i<time_of_paths.size();i++){
    //     for(int j=0;j<time_of_paths[i].size();j++){
    //         for(int k=0;k<time_of_paths[i][j].size();k++){
    //             cout<<i<<","<<j<<","<<k<<"::::"<<time_of_paths[i][j][k]<<endl;
    //         }
    //     }
    // }
    // vector<int> paths = dijkstra(0,2,g,12);
    // for(int i=0;i<paths.size();i++){
    //     cout<<paths[i]<<"->";
    // }
    // cout<<endl;

    // vector<vector<int>> k_paths = YenKSP(g,request,3);
    // for(int i=0;i<k_paths.size();i++){
    //     cout<<"Path"<<i<<":"<<endl;
    //     for(int j=0;j<k_paths[i].size();j++){
    //         cout<<k_paths[i][j]<<"->";
    //     }
    //     cout<<endl;
    // }
    return 0;
}