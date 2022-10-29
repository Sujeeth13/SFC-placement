#include <bits/stdc++.h>

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

class node{
    public:
        int id;   // temporary int 
        double link;   // cost 
        double bw;     // bandwith
        node(int a,double l,double b){
            id = a;
            link = l;
            bw = b;
        }
};

class f_attr{  //contains the attributes of a network function
    public:
    int id;
    double processing_t;
};

class node_capacity{ //contains the list of NFs installed in node and the amount left of each of those instances.
    public:
    map<int,int> NF; //maps a function instance to the amount of processing power left for that function(think NF shareability)
    map<int,double> NF_p_time; // tells how much processing time that NF requires.
    node_capacity(){}
    node_capacity(int id,double time){
        NF[id] = 1;
        NF[id] = time;
    }
};

class request{  //represents the parameters of the SFC request
    public:
    int src;
    int dest;
    vector<vector<f_attr>> SFC;
    double e2e;
    double t_arrival_rate;
};

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
    vector<request> requests(n_of_requests); //contains a batch of SFC requests
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
    //SFC = {{{0}},{{1,2},{3}},{{4}};
    for(int i=0;i<n_of_f_instances;i++){
        int f_id;
        double time;
        int node_id;
        fin>>node_id>>f_id>>time;
        // node_capacity temp(f_id,time);
        n_resource[node_id].NF[f_id] = 1;
        n_resource[node_id].NF_p_time[f_id] = time;
        NF_to_node[f_id].push_back(node_id);
    }
    cout<<"Node to funcs"<<endl;
    for(int i=0;i<N;i++){
        cout<<i<<":"<<endl;
        map<int,double>::iterator it;
        for(it=n_resource[i].NF_p_time.begin();it!=n_resource[i].NF_p_time.end();it++){
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

    return 0;
}