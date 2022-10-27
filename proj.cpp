#include <bits/stdc++.h>

using namespace std;

class g_node{
    public:
    int n_id;
    double link;
    double available_bandwidth;
    g_node(int id,double link,double bw){
        this->n_id = id;
        this->link = link;
        this->available_bandwidth = bw;
    }
};

class node_capacity{
    public:
    map<int,double> deployed_NF;
    map<int,double> NF_left;
    node_capacity(){};
};

class Request{
    public:
    int src;
    int dest;
    vector<vector<vector<int>>> SFC;
    double e2e;
    double arrival_rate;
    Request(int src,int dest,vector<vector<vector<int>>> SFC,double e2e,double arrival_rate){
        this->src = src;
        this->dest = dest;
        this->SFC = SFC;
        this->e2e = e2e;
        this->arrival_rate = arrival_rate;
    }
};

void SFC_embedding(vector<vector<g_node>>& g,vector<node_capacity>& g_resources,vector<vector<int>>& NF_to_node,map<int,int>& NFs,Request request){
    int src,dest;
    vector<vector<vector<int>>> SFC;
    double e2e;
    double arrival_rate;
    src = request.src;
    dest = request.dest;
    SFC = request.SFC;
    e2e = request.e2e;
    arrival_rate = request.arrival_rate;
    
}

int main(){
    ifstream fin("graph_config.txt");
    ifstream rin("func.txt");
    
    int N,M,no_of_inst,funcs;
    fin>>N>>M;
    fin>>funcs>>no_of_inst;
    map<int,double> NFs;
    vector<node_capacity> g_resources(N);
    vector<vector<int>> NF_to_node(funcs);

    for(int i=0;i<funcs;i++){
        int f_id;
        double p_time;
        rin>>f_id>>p_time;
        NFs[f_id] = p_time;
    }

    for(int i=0;i<no_of_inst;i++){
        int n_id,f_id;
        double processing_time,func_cap_left;
        fin>>n_id>>f_id>>processing_time>>func_cap_left;
        g_resources[n_id].deployed_NF[f_id] = processing_time;
        g_resources[n_id].NF_left[f_id] = func_cap_left;
        NF_to_node[f_id].push_back(n_id);
    }
    cout<<"node to NF:"<<endl;
    for(int i=0;i<N;i++){
        cout<<i<<":"<<endl;
        map<int,double>::iterator it;
        for(it=g_resources[i].deployed_NF.begin();it!=g_resources[i].deployed_NF.end();it++){
            cout<<it->first<<"->"<<it->second<<endl;
        }
    }
    cout<<"NF to node:"<<endl;
    for(int i=0;i<funcs;i++){
        cout<<i<<":";
        for(int j=0;j<NF_to_node[i].size();j++){
            cout<<NF_to_node[i][j]<<" ";
        }
        cout<<endl;
    }

    vector<vector<g_node>> g(N);
    vector<vector<vector<int>>> SFC{{{0}},{{1,2},{3}},{{4}}};\

    for(int i=0;i<M;i++){
        int src,dest;
        double link,bw;
        fin>>src>>dest>>link>>bw;
        g_node node1(dest,link,bw);
        g_node node2(src,link,bw);
        g[src].push_back(node1);
        g[dest].push_back(node2);
    }

    cout<<"Graph Topology:"<<endl;
    for(int i=0;i<N;i++){
        for(int j=0;j<g[i].size();j++){
            cout<<i<<"->"<<g[i][j].n_id<<"("<<g[i][j].link<<")"<<endl;
        }
    }
    Request request(0,6,SFC,270,1);
    SFC_embedding(g,g_resources,NF_to_node,NFs,request);
    return 0;
}