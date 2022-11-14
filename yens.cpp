#include <bits/stdc++.h>

using namespace std;

class node{   //  graph node
    public:
        int id;   // temporary int 
        double link;   // cost 
        double available_bandwidth;     // bandwith
        node(int id,double link,double bw){
            this->id = id;
            this->link = link;
            this->available_bandwidth = bw;
        }
};

class l_id{  // layer graph node details
    public:
        int level;
        int func;
        int node_id;
        l_id(){}
        l_id(int level,int func,int node_id){
            this->level = level;
            this->func = func;
            this->node_id = node_id;
        }
        l_id(const l_id& a){
            this->level = a.level;
            this->func = a.func;
            this->node_id = a.node_id;
        }
        bool operator < (l_id const &a) const{
            if(node_id == a.node_id){
                return func < a.func;
            }
            return node_id<a.node_id;
        }
};

class inst_node{  // layer graph node
    public:
        string id; /// id of the node
        vector<double> links; // k shortest links in the instance graph betweem two nodes
        vector<bool> vis_links; // has the link been visited or not
        inst_node(string id,vector<double> links,vector<bool> vis_links){
            this->id = id;
            this->links = links;
            this->vis_links = vis_links;
        }
};

class node_capacity{ //contains the list of NFs installed in node and the amount left of each of those instances.
    public:
    map<int,double> NF_left; //maps a function instance to the amount of processing power left for that function(think NF shareability)
    map<int,double> deployed_NF; // tells how much processing time that NF requires.
    node_capacity(){}
    node_capacity(int id,double time){
        NF_left[id] = 1;
        deployed_NF[id] = time;
    }
};

class Request{  //represents the parameters of the SFC request
    public:
    int src;
    int dest;
    vector<vector<vector<int>>> SFC;
    double e2e;
    double t_arrival_rate;
    Request(){}
    Request(int src,int dest,vector<vector<vector<int>>> SFC,double e2e,double t_arrival_rate){
        this->src = src;
        this->dest = dest;
        this->SFC = SFC;
        this->e2e = e2e;
        this->t_arrival_rate = t_arrival_rate;
    }
};

void print(std::vector <int> const &a) {
   std::cout << "The vector elements are : ";

   for(int i=0; i < a.size(); i++)
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
    int *pos;     // This is needed for decreaseKey() 
    struct MinHeapNode **array; 
}; 
  
// A utility function to create a new Min Heap Node 
struct MinHeapNode* newMinHeapNode(int v, float dist) 
{ 
    struct MinHeapNode* minHeapNode = (struct MinHeapNode*) malloc(sizeof(struct MinHeapNode)); 
    minHeapNode->v = v; 
    minHeapNode->dist = dist; 
    return minHeapNode; 
} 
  
// A utility function to create a Min Heap 
struct MinHeap* createMinHeap(int capacity) 
{ 
    struct MinHeap* minHeap = (struct MinHeap*) malloc(sizeof(struct MinHeap)); 
    minHeap->pos = (int *)malloc(capacity * sizeof(int)); 
    minHeap->size = 0; 
    minHeap->capacity = capacity; 
    minHeap->array = 
         (struct MinHeapNode**) malloc(capacity * sizeof(struct MinHeapNode*)); 
    return minHeap; 
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
        minHeap->array[left]->dist < minHeap->array[smallest]->dist ) 
      smallest = left; 
  
    if (right < minHeap->size && 
        minHeap->array[right]->dist < minHeap->array[smallest]->dist ) 
      smallest = right; 
  
    if (smallest != idx) 
    { 
        // The nodes to be swapped in min heap 
        MinHeapNode *smallestNode = minHeap->array[smallest]; 
        MinHeapNode *idxNode = minHeap->array[idx]; 
  
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
    minHeap->pos[root->v] = minHeap->size-1; 
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
        minHeap->pos[minHeap->array[i]->v] = (i-1)/2; 
        minHeap->pos[minHeap->array[(i-1)/2]->v] = i; 
        swapMinHeapNode(&minHeap->array[i],  &minHeap->array[(i - 1) / 2]); 
  
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

double add_links(vector<vector<node>>& g,vector<int>& path){ //calculates the link delay in a given path in the graph
    double time=0;
    int src = path[0];
    for(int i=1;i<path.size();i++){
        for(auto x:g[path[i-1]]){
            if(x.id == path[i]){
                time += x.link;
            }
        }
    }
    return time;
}

vector<vector<vector<double>>> calc_time(vector<vector<node>>& g,vector<vector<vector<vector<int>>>>& paths){ //calculates the time taken to traverse each of the k shortest path for all src-dest pairs in graph
    vector<vector<vector<double>>> times(paths.size());
    for(int i=0;i<paths.size();i++){
        times[i].resize(paths[i].size());
        for(int j=0;j<paths[i].size();j++){
            times[i][j].resize(paths[i][j].size());
            for(int k=0;k<paths[i][j].size();k++){
                times[i][j][k] = add_links(g,paths[i][j][k]);
            }
        }
    }
    return times;
}

// The main function that calulates distances of shortest paths from src to all 
// vertices. It is a O(ELogV) function 
vector <int> dijkstra(int src,int dest, vector<vector<node>> &graph,int throughput)
{

    // request has source, destination, NF{vector<pair<int, int>>}, throughput, delay 
    int V = graph.size();// Get the number of vertices in graph 
    float dist[V];      // dist values used to pick minimum weight edge in cut 
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
    minHeap->pos[src]   = src; 
    dist[src] = 0; 
    decreaseKey(minHeap, src, dist[src]);

    // Initially size of min heap is equal to V
    minHeap->size = V; 
  
    // In the followin loop, min heap contains all nodes 
    // whose shortest distance is not yet finalized. 
    while (!isEmpty(minHeap)) 
    {
        // Extract the vertex with minimum distance value 
        struct MinHeapNode* minHeapNode = extractMin(minHeap); 
        int u = minHeapNode->v; // Store the extracted vertex number 
  
        // Traverse through all adjacent vertices of u (the extracted 
        // vertex) and update their distance values  
        for(auto pCrawl : graph[u]) 
        {
            int v = pCrawl.id; 
  
            // If shortest distance to v is not finalized yet, and distance to v 
            // through u is less than its previously calculated distance and u-v link throughput is not a bottleneck for us
            if (isInMinHeap(minHeap, v) && dist[u] != FLT_MAX &&  
                                          pCrawl.link + dist[u] < dist[v] && pCrawl.available_bandwidth >= throughput) 
            { 
                paths[v].clear(); // clear the path to the destination node, add the nodes from the node just visited
                for(auto &vertex: paths[u])
                    paths[v].push_back(vertex);
                paths[v].push_back(v);
                dist[v] = dist[u] + pCrawl.link; 
                // //cout<<"delay is "<<pCrawl.delay<<endl;
                // update distance value in min heap also 
                decreaseKey(minHeap, v, dist[v]); 
            }
            // if destination is finalized, we are done!
            else if(v == dest && !isInMinHeap(minHeap, v))
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
    selected_path = paths[dest];

    return selected_path;
}

struct comparePaths{
    vector<vector<node>> g;
    comparePaths(vector<vector<node>> g){
        this->g = g;
    }
    bool operator() (vector<int> &P1,vector<int> &P2){
        return add_links(this->g,P1) <= add_links(this->g,P2);
    }
};

vector<vector<int>> YenKSP(vector<vector<node>> &graph,int s,int d,Request request,int K)
{
    int src = s;
    int dest = d;
    vector<vector<int>> A(K+1);
    int throughput = request.t_arrival_rate;
    A[0] = dijkstra(src,dest,graph,throughput);
    //struct k_shortest temp;
    //temp.path.push_back(A[0]);
    //return temp;
    vector <vector<int>> B;
    for(int k=1;k<=K;k++)
    {
        //cout<<"Im here"<<endl;
        for(int i=0;i<A[k-1].size();i++)
        {
            int spurNode = A[k-1][i];
            //cout<<spurNode<<endl;
            vector <int> rootpath(A[k-1].begin(),A[k-1].begin()+i);
            vector <int> removepath(A[k-1].begin()+i,A[k-1].end());
            //cout<<"Root path"<<endl;
            //print(rootpath);
            //cout<<endl;
            //cout<<"Remove path "<<endl;
            //print(removepath);
            //cout<<endl;
            vector <vector<node>> graph_copy = graph;
            for(int j=0;j<removepath.size()-1;j++)
            {
                int node1=removepath[j];
                int node2=removepath[j+1];
                for(auto a = graph_copy[node1].begin();a!=graph_copy[node1].end();a++)
                {
                    if((*a).id==node2)
                    {
                        graph_copy[node1].erase(a);
                        break;
                    }
                }
                for(auto a = graph_copy[node2].begin();a!=graph_copy[node2].end();a++)
                {
                    if((*a).id==node1)
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
            for(int l=0;l<rootpath.size();l++)
            {
                int node1 = rootpath[l];
                for(auto j = graph_copy[node1].begin();j!=graph_copy[node1].end();j++)
                {
                    int node2 = (*j).id;
                    for(auto a = graph_copy[node2].begin();a!=graph_copy[node2].end();a++)
                    {
                        if((*a).id==node1)
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
            spurPath = dijkstra(spurNode,dest,graph_copy,throughput);
            //print(spurPath);
            vector <int> totalPath ;
            totalPath.insert(totalPath.end(),rootpath.begin(),rootpath.end()); 
            totalPath.insert(totalPath.end(),spurPath.begin(),spurPath.end());

            int flag=0;
            //cout<<"totalPath"<<endl;
            //print(totalPath);
            //cout<<endl;
            if(totalPath.size()!=0){
            if(totalPath[totalPath.size()-1]!=dest)
            flag=1;
            for(int j=0;j<B.size();j++)
            {
                if(B[j]==totalPath)
                flag=1;
            } 
            for(int j=0;j<k;j++)
            {
                if(A[j]==totalPath)
                flag=1;
            }
            }
            if(totalPath.size()==0)
            flag=1;
            if(flag==0)
            B.push_back(totalPath);
        }
        if(B.size()==0){
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
        sort(B.begin(),B.end(),comparePaths(graph));
        A[k]=B[0];
        B.erase(B.begin());
    }
    // cout<<"Size of B is "<<B.size()<<endl;
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
    if(B.size() != 0){
        for(int i=1;i<K;i++){
            //cout<<B[i].size()<<endl;
            A[i] = B[i];
        }
    }
    for(int i=0;i<K;i++)
    {
        if(A[i].size()!=0)
        temp.push_back(A[i]);
        // for(int j=0;j<A[i].size();j++){
        //     cout<<A[i][j]<<"->";
        // }
        // cout<<endl;
    }
    // cout<<"Test:::::::::::::::::::"<<K<<"   "<<temp.size()<<endl;
    B.clear();
    sort(temp.begin(),temp.end(),comparePaths(graph));
    //cout<<"K::::::::::"<<temp.size()<<endl;
    return temp;

}

struct mp_comp{
    bool operator() (const l_id& a,const l_id& b){
        if(a.node_id == b.node_id){
            return a.func < b.func;
        }
        return a.node_id < b.node_id;
    }
};
// void layer_graph(l_id src,vector<int> funcs,l_id dest,map<int,double>& time,map<int,int>& deployed_inst,
// vector<vector<int>>& NF_to_node,map<int,double>& NFs,vector<node_capacity>& n_resource,vector<vector<vector<vector<int>>>>& paths,
// vector<vector<vector<double>>>& time_of_paths){

//     map<l_id,vector<inst_node>> layer_g(mp_comp);
//     queue<l_id> q;
//     int cnt = 1;
//     //forming the multi graph
//     for(int j=0;j<NF_to_node[funcs[0]].size();j++){
//         int id = funcs[0];
//         int node_id = NF_to_node[funcs[0]][j];
//         l_id u_id(cnt,id,node_id);
//         q.push(u_id);
//         vector<bool> vis(time_of_paths[src.node_id][node_id].size(),false);
//         inst_node temp_node(u_id,time_of_paths[src.node_id][node_id],vis);
//         layer_g[src] = temp_node;
//     }

//     // for(int i=1;i<funcs.size();i++){
//     //     queue<l_id> next_q;
//     //     cnt++;
//     //     while(!q.empty()){
//     //         l_id prev_id = q.front();
//     //         q.pop();
//     //         for(int j=0;j<NF_to_node[funcs[i]].size();j++){
//     //             int id = funcs[i];
//     //             int node_id = NF_to_node[funcs[i]][j];
//     //             l_id u_id(cnt,id,node_id);;
//     //             next_q.push(u_id);
//     //             vector<bool> vis(time_of_paths[prev_id.node_id][node_id].size(),false);
//     //             inst_node temp_node(u_id,time_of_paths[prev_id.node_id][node_id],vis);
//     //             layer_g[prev_id].push_back(temp_node);
//     //         }
//     //     }
//     //     q = next_q;
//     // }
//     // // cnt++;
//     // for(int j=0;j<NF_to_node[funcs[funcs.size()-1]].size();j++){
//     //     int id = funcs[funcs.size()-1];
//     //     int node_id = NF_to_node[funcs[funcs.size()-1]][j];
//     //     l_id u_id(cnt,id,node_id);
//     //     dest.level = cnt+1;
//     //     vector<bool> vis(time_of_paths[node_id][dest.node_id].size(),false);
//     //     inst_node temp_node(dest,time_of_paths[node_id][dest.node_id],vis);
//     //     layer_g[u_id].push_back(temp_node);
//     // }

//     map<l_id,vector<inst_node>>::iterator it;
//     for(it=layer_g.begin();it!=layer_g.end();it++){
//         cout<<it->first.func<<"-"<<it->first.node_id<<":\n";
//         for(int j=0;j<it->second.size();j++){
//             inst_node t = it->second[j];
//             for(int i=0;i<t.links.size();i++){
//                 cout<<" "<<t.id.func<<"-"<<t.id.node_id<<"("<<t.links[i]<<")\n";
//             }
//             cout<<endl;
//         }
//     }
// }

vector<int> parse_id(string s){
    vector<int> ids;
    string temp = "";
    for(int i=0;i<s.size();i++){
        if(s[i] == ';'){
            ids.push_back(stoi(temp));
            temp = "";
        }
        else{
            temp += s[i];
        }
    }
    return ids;
}

void layer_graph_2(int src,vector<int> funcs,int dest,map<int,double>& time,map<int,int>& deployed_inst,
vector<vector<int>>& NF_to_node,map<int,double>& NFs,vector<node_capacity>& n_resource,vector<vector<vector<vector<int>>>>& paths,
vector<vector<vector<double>>>& time_of_paths){

    map<string,vector<inst_node>> layer_g;
    queue<string> q;
    int cnt = 1;
    //forming the multi graph
    for(int j=0;j<NF_to_node[funcs[0]].size();j++){
        int id = funcs[0];
        int node_id = NF_to_node[funcs[0]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(id) + ";" + to_string(node_id);
        q.push(u_id);
        string source  = to_string(cnt-1) + ";" + to_string(-1) + ";" + to_string(src);
        vector<bool> vis(time_of_paths[src][node_id].size(),false);
        inst_node temp_node(u_id,time_of_paths[src][node_id],vis);
        layer_g[source].push_back(temp_node);
    }

    for(int i=1;i<funcs.size();i++){
        queue<string> next_q;
        cnt++;
        while(!q.empty()){
            string prev_id = q.front();
            vector<int> ids = parse_id(prev_id);
            q.pop();
            for(int j=0;j<NF_to_node[funcs[i]].size();j++){
                int id = funcs[i];
                int node_id = NF_to_node[funcs[i]][j];
                string u_id;
                u_id = to_string(cnt) + ";" + to_string(id) + ";" + to_string(node_id);
                next_q.push(u_id);
                vector<bool> vis(time_of_paths[ids.back()][node_id].size(),false);
                inst_node temp_node(u_id,time_of_paths[ids.back()][node_id],vis);
                layer_g[prev_id].push_back(temp_node);
            }
        }
        q = next_q;
    }
    // cnt++;
    for(int j=0;j<NF_to_node[funcs[funcs.size()-1]].size();j++){
        int id = funcs[funcs.size()-1];
        int node_id = NF_to_node[funcs[funcs.size()-1]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(id) + ";" + to_string(node_id);
        vector<bool> vis(time_of_paths[node_id][dest].size(),false);
        string destination = to_string(cnt) + ";" + to_string(-1) + ";" + to_string(dest);
        inst_node temp_node(destination,time_of_paths[node_id][dest],vis);
        layer_g[u_id].push_back(temp_node);
    }

    map<string,vector<inst_node>>::iterator it;
    for(it=layer_g.begin();it!=layer_g.end();it++){
        cout<<it->first<<":\n";
        for(int j=0;j<it->second.size();j++){
            inst_node t = it->second[j];
            for(int i=0;i<t.links.size();i++){
                cout<<" "<<t.id<<"("<<t.links[i]<<")\n";
            }
            cout<<endl;
        }
    }
}