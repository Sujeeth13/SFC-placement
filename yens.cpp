#include <bits/stdc++.h>

using namespace std;

class node{   //  graph node
    public:
        int id;   // temporary int 
        double link;   // cost 
        double available_bandwidth;     // bandwith
        double init_bw;
        node(int id,double link,double bw){
            this->id = id;
            this->link = link;
            this->available_bandwidth = bw;
            this->init_bw = bw;
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

class Result{
    public:
        double mean_latency;
        double mean_PD;
        Result(){
            this->mean_latency = 0;
            this->mean_PD = 0;
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
ofstream w("test.txt");
ofstream s("status.txt");
double add_links(vector<vector<node>>& g,vector<int>& path,vector<vector<vector<int>>>& SFC,vector<node_capacity>& n_r,int source,int destination){ //calculates the link delay in a given path in the graph
    cout<<"h\n";
    double time=0;
    if(path.size() == 0)
        return time;
    cout<<"he\n";
    w<<"Path Size::::::"<<path.size()<<endl;
    int src = path[0];
    w<<"Path::::::"<<path[0]<<endl;
    if(path.size() > 10000){
        for(int i=0;i<path.size();i++){
            w<<path[i]<<"->";
        }
        w<<endl;
        w<<endl;
        w<<endl;
        w<<"End"<<endl;
    }
    cout<<"hel\n";
    s<<"no\n";
    cout<<"hell\n";
    for(int i=1;i<path.size();i++){
        if(path[i-1] < 0 || path[i-1] > 23){
            cout<<"Errorrrrrr\n";
            s<<"Graph\n";
            for(int j=0;j<g.size();j++){
                for(int k=0;k<g[j].size();k++){
                    s<<j<<"->"<<g[j][k].id<<"("<<g[j][k].link<<")"<<"("<<g[j][k].available_bandwidth<<")"<<endl;
                }
            }

            s<<"SFC\n";
            for(int l=0;l<SFC.size();l++){
                for(int j=0;j<SFC[l].size();j++){
                    for(int k=0;k<SFC[l][j].size();k++){
                        cout<<SFC[l][j][k];
                    }
                    cout<<"-->";
                }
                cout<<endl;
            }

            cout<<"Node to funcs"<<endl;
            for(int l=0;l<g.size();l++){
                cout<<l<<":"<<endl;
                map<int,double>::iterator it;
                for(it=n_r[l].NF_left.begin();it!=n_r[l].NF_left.end();it++){
                    cout<<it->first<<":"<<it->second<<endl;
                }
                cout<<endl;
            }
            cout<<"SRC::::"<<source<<endl;
            cout<<"DEST:::"<<destination<<endl;

        }
        for(auto x:g[path[i-1]]){
            if(x.id == path[i] && path[i-1] == path[i])
                continue;
            if(x.id == path[i]){
                time += x.link;
            }
        }
    }
    s<<"yes\n";
    cout<<"hello\n";
    return time;
}

double add_links(vector<vector<node>>& g,vector<int>& path){ //calculates the link delay in a given path in the graph
    double time=0;
    if(path.size() == 0)
        return 0;
    int src = path[0];
    if(path.size() > 10000){
        for(int i=0;i<path.size();i++){
            w<<path[i]<<"->";
        }
        w<<endl;
        w<<endl;
        w<<endl;
        w<<"End"<<endl;
    }
    s<<"One\n";
    for(int i=1;i<path.size();i++){
        if(path[i-1] < 0 || path[i-1] > 23){
            cout<<"Errorrrrrr\n";
        }
        for(auto x:g[path[i-1]]){
            if(x.id == path[i] && path[i-1] == path[i])
                continue;
            if(x.id == path[i]){
                time += x.link;
            }
        }
    }
    s<<"Two\n";
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
    if(paths[dest].size() != 0)
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
    if(A[0].size() == 0)
    {
        vector<vector<int>> empty;
        return empty;
    }
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
    for(int i=0;i<K;i++)
    {
        if(A[i].size()!=0)
        temp.push_back(A[i]);
        // for(int j=0;j<A[i].size();j++){
        //     cout<<A[i][j]<<"->";
        // }
        // cout<<endl;
    }
    //cout<<"Test:::::::::::::::::::"<<K<<"   "<<temp.size()<<endl;
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

struct info{
    public:
    vector<vector<int>> paths;
    vector<vector<int>> dep;
    info(vector<vector<int>> paths,vector<vector<int>> dep){
        this->paths = paths;
        this->dep = dep;
    }
};

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
    ids.push_back(stoi(temp));
    return ids;
}
int cc =0;
void dfs(string u,string d,int lev,map<string,vector<inst_node>>& layer_g,vector<vector<vector<vector<int>>>>& paths,vector<vector<int>>& dep,vector<int> d_temp,vector<vector<int>>& res,vector<int> temp){

    if (u == d) {
        if(temp.size() == 0)
            return;
        res.push_back(temp);
        dep.push_back(d_temp);
        if(temp.size() > 10000){
            s<<"Error:::::dfs path size"<<u<<" "<<d<<endl;
            map<string,vector<inst_node>>::iterator it;
            for(it=layer_g.begin();it!=layer_g.end();it++){
                s<<it->first<<":\n";
                for(int j=0;j<it->second.size();j++){
                    inst_node t = it->second[j];
                    for(int i=0;i<t.links.size();i++){
                        s<<" "<<t.id<<"("<<t.links[i]<<")\n";
                    }
                    s<<endl;
                }
            }
            exit(0);
        }
        for(int i=0;i<temp.size();i++){
            if(temp[i]<0 || temp[i]>23){
                s<<"Error::::dfs\n";
                map<string,vector<inst_node>>::iterator it;
                for(it=layer_g.begin();it!=layer_g.end();it++){
                    s<<it->first<<":\n";
                    for(int j=0;j<it->second.size();j++){
                        inst_node t = it->second[j];
                        for(int i=0;i<t.links.size();i++){
                            s<<" "<<t.id<<"("<<t.links[i]<<")\n";
                        }
                        s<<endl;
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
        for (i = layer_g[u].begin(); i != layer_g[u].end(); i++){
            for(int l=0;l<i->links.size();l++){
                vector<int> temp2 = temp;
                vector<int> d_temp2 = d_temp;
                vector<int> src_id = parse_id(u);
                vector<int> dest_id = parse_id(i->id);
                d_temp2.push_back(dest_id.back());
                if(temp2.size()!=0 && paths[src_id.back()][dest_id.back()][l].begin()+1 != paths[src_id.back()][dest_id.back()][l].end()){
                    temp2.insert(temp2.end(),paths[src_id.back()][dest_id.back()][l].begin()+1,paths[src_id.back()][dest_id.back()][l].end());
                }
                else
                    temp2.insert(temp2.end(),paths[src_id.back()][dest_id.back()][l].begin(),paths[src_id.back()][dest_id.back()][l].end());
                dfs(i->id,d,lev+1,layer_g,paths,dep,d_temp2,res,temp2);
                cc++;
            }
        }
    }

}

info get_all_paths(string s,string d,map<string,vector<inst_node>>& layer_g,vector<vector<vector<vector<int>>>>& paths,vector<vector<vector<double>>>& time_of_paths){
    vector<vector<int>> res;
    vector<int> temp;
    vector<vector<int>> dep;
    vector<int> d_temp;
    vector<int> ids = parse_id(s);
    int lev = 0;
    cout<<"source::"<<s<<" "<<"Des::"<<d<<endl;
    dfs(s,d,lev,layer_g,paths,dep,d_temp,res,temp);
    cout<<"Size:::::::::::"<<res.size()<<":::"<<cc<<endl;
    info i(res,dep);
    return i;
}

class comp{
    public:
    double time;
    vector<vector<node>> g;
    Request request;
    vector<node_capacity> n_r;
    comp(vector<vector<node>>& g,double time,Request request,vector<node_capacity> n_r){
        this->g = g;
        this->time = time;
        this->request = request;
        this->n_r = n_r;
    }
    bool operator()(pair<vector<int>,vector<int>>& a,pair<vector<int>,vector<int>>& b){
        cout<<"Test1\n";
        double time_a = add_links(g,a.first,request.SFC,n_r,request.src,request.dest);
        cout<<"Test1.5\n";
        double time_b = add_links(g,b.first,request.SFC,n_r,request.src,request.dest);
        cout<<"Test2\n";
        if(abs(time_a-time) == abs(time_b-time)){
            cout<<"Test3\n";
            return (time_a-time) <= (time_b - time);
        }
        cout<<"Test4\n";
        return abs(time_a-time) < abs(time_b-time);
    }
};

void update_BW(vector<vector<node>>& g,vector<int> path,double arrival){
    if(path.size() <= 1)
        return;
    int prev,curr;
    prev = path[0];

    // for(int i=0;i<path.size();i++){
    //     cout<<path[i]<<"->";
    // }
    // cout<<endl;

    for(int i=1;i<path.size();i++){
        curr = path[i];
        int j=0;
        while(j < g[prev].size() && g[prev][j].id != curr)
            j++;
        g[prev][j].available_bandwidth -= arrival;
        j=0;
        while(j < g[curr].size() && g[curr][j].id != prev)
            j++;
        if(j!=g[curr].size())
            g[curr][j].available_bandwidth -= arrival;
        prev = curr;
    }
}

bool is_available_BW(vector<vector<node>>& g,vector<int> path,double arrival){
    if(path.size() <= 1)
        return true;
    int prev,curr;
    prev = path[0];
    for(int i=1;i<path.size();i++){
        curr = path[i];
        int j=0;
        while(j < g[prev].size() && g[prev][j].id != curr)
            j++;
        if(g[prev][j].available_bandwidth < arrival){
            return false;
        }
        prev = curr;
    }
    return true;
}

void update_resource(vector<node_capacity>& n_resource,vector<int> funcs,vector<int> dep,double arrival){
    for(int i=0;i<dep.size()-1;i++){
        n_resource[dep[i]].NF_left[funcs[i]] -= arrival;
    }
}

bool is_available_resource(vector<node_capacity>& n_resource,vector<int> funcs,vector<int> dep,double arrival){
    for(int i=0;i<dep.size()-1;i++){
        if(n_resource[dep[i]].NF_left[funcs[i]] < arrival){
            return false;
        }
    }
    return true;
}

bool layer_graph_2(int src,vector<int> funcs,int dest,vector<vector<node>>& g,map<int,double>& time,map<int,int>& deployed_inst,
vector<vector<int>>& NF_to_node,map<int,double>& NFs,vector<node_capacity>& n_resource,vector<vector<vector<vector<int>>>>& paths,
vector<vector<vector<double>>>& time_of_paths,Request request,Result& result,double diff,double& max_tt){
    cout<<"Fnc3\n";
    double temp_diff = diff;
    for(int i=0;i<funcs.size();i++){
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
    cout<<"FINAL_DIFF::::::"<<diff<<endl;
    map<string,vector<inst_node>> layer_g;
    queue<string> q;
    int cnt = 1;
    //forming the multi graph
    if(NF_to_node[funcs[0]].size() == 0)
        return false;
    for(int j=0;j<NF_to_node[funcs[0]].size();j++){
        int id = funcs[0];
        int node_id = NF_to_node[funcs[0]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(node_id);
        q.push(u_id);
        string source  = to_string(cnt-1) + ";" + to_string(src);
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
            if(NF_to_node[funcs[i]].size() == 0)
                return false;
            for(int j=0;j<NF_to_node[funcs[i]].size();j++){
                int id = funcs[i];
                int node_id = NF_to_node[funcs[i]][j];
                string u_id;
                u_id = to_string(cnt) + ";" + to_string(node_id);
                next_q.push(u_id);
                vector<bool> vis(time_of_paths[ids.back()][node_id].size(),false);
                inst_node temp_node(u_id,time_of_paths[ids.back()][node_id],vis);
                layer_g[prev_id].push_back(temp_node);
            }
        }
        q = next_q;
    }
    // cnt++;
    if(NF_to_node[funcs[funcs.size()-1]].size() == 0)
        return false;
    for(int j=0;j<NF_to_node[funcs[funcs.size()-1]].size();j++){
        int id = funcs[funcs.size()-1];
        int node_id = NF_to_node[funcs[funcs.size()-1]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(node_id);
        vector<bool> vis(time_of_paths[node_id][dest].size(),false);
        string destination = to_string(cnt+1) + ";" + to_string(dest);
        inst_node temp_node(destination,time_of_paths[node_id][dest],vis);
        layer_g[u_id].push_back(temp_node);
    }

    map<string,vector<inst_node>>::iterator it;
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

    string source = to_string(0) + ";" + to_string(src);
    string destination = to_string(cnt+1) + ";" + to_string(dest);
    // vector<vector<int>> layer_paths = get_all_paths(source,destination,layer_g,paths,time_of_paths);
    info i = get_all_paths(source,destination,layer_g,paths,time_of_paths);
    vector<pair<vector<int>,vector<int> > > sorter;
    cout<<"Here1\n";
    cout<<"i.paths:::::"<<i.paths.size()<<endl;
    if(i.paths.size() == 0)
        return false;
    for(int k=0;k<i.paths.size();k++){
        cout<<"counter::;:"<<i.paths[k].size()<<endl;
        sorter.push_back(make_pair(i.paths[k],i.dep[k]));
    }
    // wil have to update the time;;;;;;;;;;;;;;;;;;;;;;;;
    cout<<"Here sub1\n";
    sort(sorter.begin(),sorter.end(),comp(g,diff,request,n_resource));
    cout<<"Here sub2\n";

    vector<vector<int>> layer_paths;
    vector<vector<int>> layer_dep;
    cout<<"Here2\n";
    for(int i=0;i<sorter.size();i++){
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
    cout<<"Here3\n";
    int p;
    for(p=0;p<i.paths.size();p++){
        i.dep[p].pop_back();
        if(is_available_BW(g,i.paths[p],request.t_arrival_rate) && is_available_resource(n_resource,funcs,i.dep[p],request.t_arrival_rate)){
            //do the check for time update
            break;
        }
    }
    if(p == i.paths.size()){
        //somehow drop the request
        return false;
    }
    double tt = add_links(g,i.paths[p]);
    result.mean_PD += abs(tt - diff);
    cout<<"TTT:::::::"<<tt<<endl;
    if(tt > diff){
        if(tt - diff > max_tt)
            max_tt = tt - diff;
    }
    cout<<"Here4\n";
    update_BW(g,i.paths[p],request.t_arrival_rate);
    update_resource(n_resource,funcs,i.dep[p],request.t_arrival_rate);
    cout<<"Here5\n";
    for(int k=0;k<i.dep[p].size();k++){
        deployed_inst[funcs[k]] = i.dep[p][k];
        //cout<<funcs[k]<<":::::::::"<<i.dep[p][k]<<endl;
    }
    return true;
}

bool layer_graph(int src,vector<int> funcs,int dest,vector<vector<node>>& g,map<int,double>& time,map<int,int>& deployed_inst,
vector<vector<int>>& NF_to_node,map<int,double>& NFs,vector<node_capacity>& n_resource,vector<vector<vector<vector<int>>>>& paths,
vector<vector<vector<double>>>& time_of_paths,Request request,Result& result,double& diff,double& max_tt){
    cout<<"layer_DIFF::::::"<<diff<<endl;
    for(int i=0;i<funcs.size();i++){
        diff -= NFs[funcs[i]];
    }
    cout<<"FINAL_DIFF::::::"<<diff<<endl;
    map<string,vector<inst_node>> layer_g;
    queue<string> q;
    int cnt = 1;
    //forming the multi graph
    if(NF_to_node[funcs[0]].size() == 0)
        return false;
    for(int j=0;j<NF_to_node[funcs[0]].size();j++){
        int id = funcs[0];
        int node_id = NF_to_node[funcs[0]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(node_id);
        q.push(u_id);
        string source  = to_string(cnt-1) + ";" + to_string(src);
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
            if(NF_to_node[funcs[i]].size() == 0)
                return false;
            for(int j=0;j<NF_to_node[funcs[i]].size();j++){
                int id = funcs[i];
                int node_id = NF_to_node[funcs[i]][j];
                string u_id;
                u_id = to_string(cnt) + ";" + to_string(node_id);
                next_q.push(u_id);
                vector<bool> vis(time_of_paths[ids.back()][node_id].size(),false);
                inst_node temp_node(u_id,time_of_paths[ids.back()][node_id],vis);
                layer_g[prev_id].push_back(temp_node);
            }
        }
        q = next_q;
    }
    // cnt++;
    if(NF_to_node[funcs[funcs.size()-1]].size()== 0)
        return false;
    for(int j=0;j<NF_to_node[funcs[funcs.size()-1]].size();j++){
        int id = funcs[funcs.size()-1];
        int node_id = NF_to_node[funcs[funcs.size()-1]][j];
        string u_id;
        u_id = to_string(cnt) + ";" + to_string(node_id);
        vector<bool> vis(time_of_paths[node_id][dest].size(),false);
        string destination = to_string(cnt+1) + ";" + to_string(dest);
        inst_node temp_node(destination,time_of_paths[node_id][dest],vis);
        layer_g[u_id].push_back(temp_node);
    }

    map<string,vector<inst_node>>::iterator it;
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

    string source = to_string(0) + ";" + to_string(src);
    string destination = to_string(cnt+1) + ";" + to_string(dest);
    // vector<vector<int>> layer_paths = get_all_paths(source,destination,layer_g,paths,time_of_paths);
    info i = get_all_paths(source,destination,layer_g,paths,time_of_paths);
    vector<pair<vector<int>,vector<int> > > sorter;
    cout<<"Here1\n";
    cout<<"i.paths:::::"<<i.paths.size()<<endl;
    if(i.paths.size() == 0)
        return false;
    for(int k=0;k<i.paths.size();k++){
        sorter.push_back(make_pair(i.paths[k],i.dep[k]));
    }
    // wil have to update the time;;;;;;;;;;;;;;;;;;;;;;;;
    sort(sorter.begin(),sorter.end(),comp(g,0,request,n_resource));

    vector<vector<int>> layer_paths;
    vector<vector<int>> layer_dep;
    cout<<"Here2\n";
    for(int i=0;i<sorter.size();i++){
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
    cout<<"Here3\n";
    int p;
    for(p=0;p<i.paths.size();p++){
        i.dep[p].pop_back();
        if(is_available_BW(g,i.paths[p],request.t_arrival_rate) && is_available_resource(n_resource,funcs,i.dep[p],request.t_arrival_rate)){
            //do the check for time update
            break;
        }
    }
    if(p == i.paths.size()){
        //somehow drop the request
        return false;
    }
    double tt = add_links(g,i.paths[p]);
    result.mean_PD += abs(tt - diff);
    cout<<"TTT:::::::"<<tt<<endl;
    if(tt > diff){
        if(tt - diff > max_tt)
            max_tt = tt - diff;
    }
    cout<<"Here4\n";
    update_BW(g,i.paths[p],request.t_arrival_rate);
    update_resource(n_resource,funcs,i.dep[p],request.t_arrival_rate);
    cout<<"Here5\n";
    for(int k=0;k<i.dep[p].size();k++){
        deployed_inst[funcs[k]] = i.dep[p][k];
        //cout<<funcs[k]<<":::::::::"<<i.dep[p][k]<<endl;
    }
    return true;
}