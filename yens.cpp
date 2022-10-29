#include <bits/stdc++.h>

using namespace std;

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
void minHeapify(struct MinHeap* minHeap, int idx) 
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
int isEmpty(struct MinHeap* minHeap) 
{ 
    return minHeap->size == 0; 
} 
  
// Standard function to extract minimum node from heap 
struct MinHeapNode* extractMin(struct MinHeap* minHeap) 
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
void decreaseKey(struct MinHeap* minHeap, int v, float dist) 
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
bool isInMinHeap(struct MinHeap *minHeap, int v) 
{ 
   if (minHeap->pos[v] < minHeap->size) 
     return true; 
   return false; 
} 
  
// The main function that calulates distances of shortest paths from src to all 
// vertices. It is a O(ELogV) function 
vector <int> dijkstra(int src,int dest, vector<vector<struct LinkInfo>> &graph,int throughput)
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
            int v = pCrawl.node2; 
  
            // If shortest distance to v is not finalized yet, and distance to v 
            // through u is less than its previously calculated distance and u-v link throughput is not a bottleneck for us
            if (isInMinHeap(minHeap, v) && dist[u] != FLT_MAX &&  
                                          pCrawl.delay + dist[u] < dist[v] && pCrawl.available_bandwidth >= throughput) 
            { 
                paths[v].clear(); // clear the path to the destination node, add the nodes from the node just visited
                for(auto &vertex: paths[u])
                    paths[v].push_back(vertex);
                paths[v].push_back(v);
                dist[v] = dist[u] + pCrawl.delay; 
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

bool comparePaths(vector<int> &P1,vector<int> &P2)
{
    int sum1=0;
    int sum2=0;
    for(int i=0;i<P1.size();i++)
    sum1+=1;
    for(int i=0;i<P2.size();i++)
    sum2+=1;
    return sum1<=sum2;
}

struct k_shortest YenKSP(vector<vector<struct LinkInfo>> &graph,struct Request request,int K)
{
    int src = request.source;
    int dest = request.destination;
    vector <int> A[K+1];
    int throughput = request.throughput;
    A[0] = dijkstra(src,dest,graph,throughput);
    // for(int i=0;i<A[0].size();i++)
    // cout<<A[0][i]<<" ";
    
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
            vector <vector<struct LinkInfo>> graph_copy = graph;
            for(int i=0;i<removepath.size()-1;i++)
            {
                int node1=removepath[i];
                int node2=removepath[i+1];
                for(auto a = graph_copy[node1].begin();a!=graph_copy[node1].end();a++)
                {
                    if((*a).node2==node2)
                    {
                        graph_copy[node1].erase(a);
                        break;
                    }
                }
                for(auto a = graph_copy[node2].begin();a!=graph_copy[node2].end();a++)
                {
                    if((*a).node2==node1)
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
            for(int i=0;i<rootpath.size();i++)
            {
                int node1 = rootpath[i];
                for(auto j = graph_copy[node1].begin();j!=graph_copy[node1].end();j++)
                {
                    int node2 = (*j).node2;
                    for(auto a = graph_copy[node2].begin();a!=graph_copy[node2].end();a++)
                    {
                        if((*a).node2==node1)
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
            spurPath = dijkstra(spurNode,dest,graph_copy,0);
            //print(spurPath);
            vector <int> totalPath ;
            totalPath.insert(totalPath.end(),rootpath.begin(),rootpath.end()); 
            totalPath.insert(totalPath.end(),spurPath.begin(),spurPath.end());

            int flag=0;
            //cout<<"totalPath"<<endl;
            //print(totalPath);
            //cout<<endl;
            if(totalPath.size()!=0)
            if(totalPath[totalPath.size()-1]!=dest)
            flag=1;
            for(int i=0;i<B.size();i++)
            {
                if(B[i]==totalPath)
                flag=1;
            } 
            for(int i=0;i<k;i++)
            {
                if(A[i]==totalPath)
                flag=1;
            }
            if(totalPath.size()==0)
            flag=1;
            if(flag==0)
            B.push_back(totalPath);
        }
        if(B.size()==0)
        break;
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
        sort(B.begin(),B.end(),comparePaths);
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
    struct k_shortest temp;
    // for(int i=1;i<K;i++)
    // A[i]=B[i];
    for(int i=0;i<K;i++)
    {
        if(A[i].size()!=0)
        temp.path.push_back(A[i]);
    }
    //B.clear();
    sort(temp.path.begin(),temp.path.end(),comparePaths);
    return temp;

}