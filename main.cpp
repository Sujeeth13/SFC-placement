#include <bits/stdc++.h>
#include "yens.cpp"

using namespace std;

vector<vector<vector<int>>> bin_FP(vector<vector<vector<int>>> SFC,map<int,double> NFs){
    vector<vector<vector<int>>> temp;

    cout<<"Initial SFC:::::::\n";
    for(int l=0;l<SFC.size();l++){
        for(int j=0;j<SFC[l].size();j++){
            for(int k=0;k<SFC[l][j].size();k++){
                cout<<"("<<SFC[l][j][k]<<")";
            }
            cout<<"-->";
        }
        cout<<endl;
    }

    for(int i=0;i<SFC.size();i++){ //for each Ci
        if(SFC[i].size() == 1){
            temp.push_back(SFC[i]);
            continue;
        }
        double max_process = -1;
        int loc = 0;
        int max_func;
        for(int j=0;j<SFC[i].size();j++){ // for each bx in Ci
            for(int k=0;k<SFC[i][j].size();k++){ //finding the max f_time
                if(NFs[SFC[i][j][k]] > max_process){
                    max_process = NFs[SFC[i][j][k]];
                    max_func = SFC[i][j][k];
                    loc = j;
                }
            }
        }
        if(loc == SFC[i].size()-1)
            return SFC;
        else{
            int temper = SFC[i][SFC[i].size()-1][0];
            SFC[i][SFC[i].size()-1][0] = SFC[i][loc][0];
            SFC[i][loc][0] = temper;
            temp.push_back(SFC[i]);
        }

    }
    cout<<"Final SFC:::::::\n";
    for(int l=0;l<temp.size();l++){
        for(int j=0;j<temp[l].size();j++){
            for(int k=0;k<temp[l][j].size();k++){
                cout<<"("<<temp[l][j][k]<<")";
            }
            cout<<"-->";
        }
        cout<<endl;
    }
    return temp;
}

//SFC = {{{0}},{{1,2},{3}},{{4}}};

vector<vector<vector<int>>> bin(vector<vector<vector<int>>> SFC,map<int,double> NFs){
    vector<vector<vector<int>>> temp;

    for(int i=0;i<SFC.size();i++){ //for each Ci
        if(SFC[i].size() == 1){
            temp.push_back(SFC[i]);
            continue;
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
        for(int l=0;l<SFC.size();l++){
            for(int j=0;j<SFC[l].size();j++){
                for(int k=0;k<SFC[l][j].size();k++){
                    if(SFC[l][j].size() > 1){
                        cout<<"It is there\n";
                        exit(0);
                    }
                    cout<<"("<<SFC[l][j][k]<<")";
                }
                cout<<"-->";
            }
            cout<<endl;
        }

        double cap = max_process;
        vector<vector<int>> new_Ci;
        double cap_curr=0;
        vector<int> bin;
        for(int b=0;b<SFC[i].size();b++){
            for(int f=0;f<SFC[i][b].size();f++){
                if(SFC[i][b][f] == max_func)
                    continue;
                else if(cap_curr + NFs[SFC[i][b][f]] <= cap){
                    cap_curr += NFs[SFC[i][b][f]];
                    bin.push_back(SFC[i][b][f]);
                }
                else{
                    cap_curr = 0;
                    new_Ci.push_back(bin);
                    if(bin.size() == 0){
                        cout<<"2222222222222222222\n";
                        map<int,double>::iterator it;
                        for(it = NFs.begin();it!=NFs.end();it++){
                            s<<it->first<<": "<<it->second<<endl;
                        }
                        exit(0);
                    }
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

bool check(vector<vector<vector<vector<int>>>>& paths){
    for(int i=0;i<paths.size();i++){
        for(int j=0;j<paths[i].size();j++){
            for(int k=0;k<paths[i][j].size();k++){
                for(int l=0;l<paths[i][j][k].size();l++){
                    if(paths[i][j][k][l] < 0 || paths[i][j][k][l] > 23)
                        return false;
                }
            }
        }
    }
    return true;
}

//ofstream s("status.txt");

bool SFC_embedding_PD(vector<vector<node>>& g,vector<node_capacity>& n_resource,vector<vector<int>>& NF_to_node,map<int,double>& NFs,Request request,Result& result){
    
    map<int,int> deployed_inst;
    map<int,double> time; //map that contains the reach time to a NF in the chain
    double dest_time;
    double pkt_copy=16,pkt_merge=16;
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
    vector<vector<vector<vector<int>>>> paths(g.size(),vector<vector<vector<int>>>(g.size()));
    //cout<<paths.size()<<paths[0].size()<<endl;
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
                if(k_paths.size() == 0)
                    return false;
                //cout<<"here1\n";
            }
            paths[i][j] = k_paths;
        }
    }

    if(!check(paths)){
        cout<<"error:::::::::PD_paths"<<endl;
        exit(0);
    }

    //cout<<paths.size()<<paths[0].size()<<endl;
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
    for(int i=0;i<SFC.size();i++){
        critical_branch_node[i] = SFC[i][SFC[i].size()-1][SFC[i][SFC[i].size()-1].size()-1];
    }
    critical_branch_node[0] = SFC[0][SFC[0].size()-1][0];
    int inst_0;
    int min_dist = INT_MAX;
    vector<int> min_path;
    // cout<<"Error::::"<<NF_to_node[critical_branch_node[0]].size()<<endl;
    for(int i=0;i<NF_to_node[critical_branch_node[0]].size();i++){
        if(n_resource[NF_to_node[critical_branch_node[0]][i]].NF_left[critical_branch_node[0]] > t_arrival_rate){
            if(time_of_paths[src][NF_to_node[critical_branch_node[0]][i]][0] < min_dist){
                min_dist = time_of_paths[src][NF_to_node[critical_branch_node[0]][i]][0];
                min_path = paths[src][NF_to_node[critical_branch_node[0]][i]][0];
                inst_0 = NF_to_node[critical_branch_node[0]][i];
                // cout<<"All inst::::"<<inst_0<<endl;
            }
        }
    }
    if(min_dist == INT_MAX) // could not find a path
        return false;
    vector<vector<node>> temp_g = g;
    vector<node_capacity> temp_n_resource = n_resource;
    deployed_inst[critical_branch_node[0]] = inst_0;
    time[critical_branch_node[0]] = min_dist/*link delay*/ + NFs[critical_branch_node[0]]/*function delay*/ +(SFC[1].size()-1)*(pkt_copy)/*copy delay*/ ;
    update_BW(temp_g,min_path,request.t_arrival_rate);
    update_resource(temp_n_resource,vector<int>(1,critical_branch_node[0]),vector<int>(1,inst_0),request.t_arrival_rate);
     //call the shortest paths again since network got changed
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
                k_paths = YenKSP(temp_g,i,j,request,K);
                if(k_paths.size() == 0)
                    return false;
                //cout<<"here1\n";
            }
            paths[i][j] = k_paths;
        }
    }
    if(!check(paths)){
        cout<<"error:::::::::PD_paths2"<<endl;
        exit(0);
    }
    time_of_paths = calc_time(temp_g,paths);

    // cout<<"inst:::"<<inst_0<<endl;
    // cout<<"Min dis:::"<<min_dist<<endl;
    // cout<<"F delay:::"<<NFs[critical_branch_node[0]]<<endl;
    // cout<<"Copy delay:::"<<(SFC[1].size()-1)*(pkt_copy)<<endl;
    // cout<<"initial::::"<<time[critical_branch_node[0]]<<endl;

    for(int i=0;i<SFC.size()-1;i++){
        int inst;
        int min_dist = INT_MAX;
        vector<int> min_path;
        int next_branch_node;
        for(int b=0;b<SFC[i+1].size();b++){
            if(SFC[i].size() == 1 && b!=SFC[i+1].size()-1)
                continue;
            next_branch_node = SFC[i+1][b][0];
            min_dist = INT_MAX;
            for(int j=0;j<NF_to_node[next_branch_node].size();j++){
                if(n_resource[NF_to_node[next_branch_node][j]].NF_left[next_branch_node] > t_arrival_rate){
                    if(time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]].size() == 0)
                        continue;
                    if(time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0] < min_dist){
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
            if(min_dist == INT_MAX)
                return false;
            deployed_inst[next_branch_node] = inst;
            time[next_branch_node] = time[critical_branch_node[i]] + min_dist/*link delay*/ + NFs[next_branch_node]/*function delay*/ +(SFC[i].size()-1)*(pkt_merge)/*merge delay*/ ;
            update_BW(temp_g,min_path,request.t_arrival_rate);
            update_resource(temp_n_resource,vector<int>(1,next_branch_node),vector<int>(1,inst),request.t_arrival_rate);
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
                        k_paths = YenKSP(temp_g,i,j,request,K);
                        if(k_paths.size() == 0)
                            return false;
                        //cout<<"here1\n";
                    }
                    paths[i][j] = k_paths;
                }
            }
            if(!check(paths)){
                cout<<"error:::::::::PD_paths3"<<endl;
                exit(0);
            }
            time_of_paths = calc_time(temp_g,paths);

                // cout<<"Min dis:::"<<min_dist<<endl;
                // cout<<"F delay:::"<<NFs[next_branch_node]<<endl;
                // cout<<"Copy delay:::"<<(SFC[i].size()-1)*(pkt_merge)<<endl;
                // cout<<"initial::::"<<time[next_branch_node]<<endl;
            if(i+2 <= SFC.size()-1 && SFC[i+1][b].size() == 1)
                time[next_branch_node] += (SFC[i+2].size()-1)*(pkt_copy);
        }
    }
    if(time_of_paths[deployed_inst[critical_branch_node[SFC.size()-1]]][dest].size()==0)
        return false;
    dest_time = time[critical_branch_node[SFC.size()-1]] + time_of_paths[deployed_inst[critical_branch_node[SFC.size()-1]]][dest][0] + (SFC[SFC.size()-1].size()-1)*(pkt_merge);
    // for(int i=0;i<SFC.size();i++){
    //     for(int j=0;j<SFC[i].size();j++){
    //         for(int k=0;k<SFC[i][j].size();k++){
    //             cout<<i<<","<<j<<","<<k<<":::"<<time[SFC[i][j][k]]<<endl;
    //         }
    //     }
    // }
    cout<<"e2e latency:::::::::::"<<dest_time<<endl;

    // layer graph step......
    for(int i=0;i<SFC.size();i++){
        if(SFC[i].size() == 1)
            continue;
        
        // UPDATE NEEDED!!!!!
        double max_tt = 0;
        double diff = 0;
        if(i == SFC.size()-1){
            diff = dest_time - time[SFC[i-1][SFC[i-1].size()-1][0]];
            diff -= (SFC[i].size()-1)*pkt_merge;
            // cout<<"DIFFFFFFFF:::::::::"<<diff<<endl;
        }
        else{
            diff = time[SFC[i+1][SFC[i+1].size()-1][0]] - time[SFC[i-1][SFC[i-1].size()-1][0]];
            diff -= NFs[SFC[i+1][SFC[i+1].size()-1][0]] + (SFC[i].size()-1)*pkt_merge;
            // cout<<"DIFFFFFFFF:::::::::"<<diff<<endl;
            // cout<<time[SFC[i+1][SFC[i+1].size()-1][0]]<<"\n"<<time[SFC[i-1][SFC[i-1].size()-1][0]]<<"\n"<<NFs[SFC[i+1][SFC[i+1].size()-1][0]]<<"\n"<<(SFC[i].size()-1)*pkt_merge<<endl;
        }
        for(int b=0;b<SFC[i].size()-1;b++){ //checking all branches except critical branch since all function instances have been deployed in the critical branch
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
            if(deployed_inst.find(SFC[i][b][0]) == deployed_inst.end()){
                if(i == 0){
                    cout<<"Fnc1\n";
                    for(int b_n=0;b_n<SFC[i+1].size();b_n++){
                        if(!layer_graph_2(src,SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                            return false;
                    }
                }
                else{
                    if(i == SFC.size()-1){
                        cout<<"CHECKER FOR DEPLOYED::::::"<<deployed_inst[SFC[i-1][0][0]]<<" "<<SFC[i][b].size()<<endl;
                        if(!layer_graph_2(deployed_inst[SFC[i-1][0][0]],SFC[i][b],dest,temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                            return false;
                    }
                    else{
                        cout<<"Fnc2\n";
                        for(int b_n=0;b_n<SFC[i+1].size();b_n++){
                            // cout<<"HHHHHHHHHHHHHHHHHHHHHHHH:"<<b_n<<endl;
                            if(!layer_graph_2(deployed_inst[SFC[i-1][0][0]],SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                                return false;
                            // cout<<"SSSSSSSSSSSSSS::"<<deployed_inst[SFC[i-1][0][0]]<<"DDDDDDDDDDDDDDDD::"<<deployed_inst[SFC[i+1][b_n][0]]<<endl;
                        }
                    }
                }
            }
            else{
                if(i == SFC.size()-1){
                    cout<<"Fnc4\n";
                    if(!layer_graph_2(deployed_inst[SFC[i][b][0]],SFC[i][b],dest,temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                        return false;
                }
                else{
                    cout<<"Fnc5\n";
                    for(int b_n=0;b_n<SFC[i+1].size();b_n++){
                        if(!layer_graph_2(deployed_inst[SFC[i][b][0]],SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                            return false;
                    }
                }
            }
        }
        dest_time += max_tt;
    }
    if(dest_time > request.e2e)
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
    for(int i=0;i<SFC.size();i++){
        for(int j=0;j<SFC[i].size();j++){
            for(int k=0;k<SFC[i][j].size();k++){
                result.nodes_util[deployed_inst[SFC[i][j][k]]]++;
            }
        }
    }

    cout<<"final e2e latency:::::::::::"<<dest_time<<endl;
    result.mean_latency += dest_time;
    return true;
}

bool SFC_embedding(vector<vector<node>>& g,vector<node_capacity>& n_resource,vector<vector<int>>& NF_to_node,map<int,double>& NFs,Request request,Result& result){
    
    map<int,int> deployed_inst;
    map<int,double> time; //map that contains the reach time to a NF in the chain
    double dest_time;
    double pkt_copy=16,pkt_merge=16;
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
    
    int K = 4; //the number of K shortest paths
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
                if(k_paths.size() == 0)
                    return false;
                //cout<<"here1\n";
            }
            paths[i][j] = k_paths;
        }
    }
    if(!check(paths)){
        cout<<"error:::::::::paths"<<endl;
        exit(0);
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
    for(int i=0;i<SFC.size();i++){
        critical_branch_node[i] = SFC[i][SFC[i].size()-1][SFC[i][SFC[i].size()-1].size()-1];
    }
    critical_branch_node[0] = SFC[0][SFC[0].size()-1][0];
    int inst_0;
    int min_dist = INT_MAX;
    vector<int> min_path;
    // cout<<"Error::::"<<NF_to_node[critical_branch_node[0]].size()<<endl;
    for(int i=0;i<NF_to_node[critical_branch_node[0]].size();i++){
        if(n_resource[NF_to_node[critical_branch_node[0]][i]].NF_left[critical_branch_node[0]] > t_arrival_rate){
            if(time_of_paths[src][NF_to_node[critical_branch_node[0]][i]][0] < min_dist){
                min_dist = time_of_paths[src][NF_to_node[critical_branch_node[0]][i]][0];
                min_path = paths[src][NF_to_node[critical_branch_node[0]][i]][0];
                inst_0 = NF_to_node[critical_branch_node[0]][i];
                // cout<<"All inst::::"<<inst_0<<endl;
            }
        }
    }
    if(min_dist == INT_MAX) // could not find a path
        return false;
    vector<vector<node>> temp_g = g;
    vector<node_capacity> temp_n_resource = n_resource;
    deployed_inst[critical_branch_node[0]] = inst_0;
    time[critical_branch_node[0]] = min_dist/*link delay*/ + NFs[critical_branch_node[0]]/*function delay*/ +(SFC[1].size()-1)*(pkt_copy)/*copy delay*/ ;
    update_BW(temp_g,min_path,request.t_arrival_rate);
    update_resource(temp_n_resource,vector<int>(1,critical_branch_node[0]),vector<int>(1,inst_0),request.t_arrival_rate);
     //call the shortest paths again since network got changed
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
                k_paths = YenKSP(temp_g,i,j,request,K);
                if(k_paths.size() == 0)
                    return false;
                //cout<<"here1\n";
            }
            paths[i][j] = k_paths;
        }
    }
    if(!check(paths)){
        cout<<"error:::::::::paths2"<<endl;
        exit(0);
    }
    time_of_paths = calc_time(temp_g,paths);

    // cout<<"inst:::"<<inst_0<<endl;
    // cout<<"Min dis:::"<<min_dist<<endl;
    // cout<<"F delay:::"<<NFs[critical_branch_node[0]]<<endl;
    // cout<<"Copy delay:::"<<(SFC[1].size()-1)*(pkt_copy)<<endl;
    // cout<<"initial::::"<<time[critical_branch_node[0]]<<endl;
    for(int i=0;i<SFC.size()-1;i++){
        int inst;
        int min_dist = INT_MAX;
        vector<int> min_path;
        int next_branch_node;
        for(int b=0;b<SFC[i+1].size();b++){
            if(SFC[i].size() == 1 && b!=SFC[i+1].size()-1)
                continue;
            next_branch_node = SFC[i+1][b][0];
            min_dist = INT_MAX;
            for(int j=0;j<NF_to_node[next_branch_node].size();j++){
                if(n_resource[NF_to_node[next_branch_node][j]].NF_left[next_branch_node] > t_arrival_rate){
                    if(time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]].size() == 0)
                        continue;
                    if(time_of_paths[deployed_inst[critical_branch_node[i]]][NF_to_node[next_branch_node][j]][0] < min_dist){
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
            if(min_dist == INT_MAX)
                return false;
            deployed_inst[next_branch_node] = inst;
            time[next_branch_node] = time[critical_branch_node[i]] + min_dist/*link delay*/ + NFs[next_branch_node]/*function delay*/ +(SFC[i].size()-1)*(pkt_merge)/*merge delay*/ ;
            update_BW(temp_g,min_path,request.t_arrival_rate);
            update_resource(temp_n_resource,vector<int>(1,next_branch_node),vector<int>(1,inst),request.t_arrival_rate);
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
                        k_paths = YenKSP(temp_g,i,j,request,K);
                        //cout<<"here1\n";
                    }
                    paths[i][j] = k_paths;
                }
            }
            if(!check(paths)){
                cout<<"error:::::::::paths3"<<endl;
                exit(0);
            }
            time_of_paths = calc_time(temp_g,paths);

                // cout<<"Min dis:::"<<min_dist<<endl;
                // cout<<"F delay:::"<<NFs[next_branch_node]<<endl;
                // cout<<"Copy delay:::"<<(SFC[i].size()-1)*(pkt_merge)<<endl;
                // cout<<"initial::::"<<time[next_branch_node]<<endl;
            if(i+2 <= SFC.size()-1 && SFC[i+1][b].size() == 1)
                time[next_branch_node] += (SFC[i+2].size()-1)*(pkt_copy);
        }
    }
    if(time_of_paths[deployed_inst[critical_branch_node[SFC.size()-1]]][dest].size()==0)
        return false;
    dest_time = time[critical_branch_node[SFC.size()-1]] + time_of_paths[deployed_inst[critical_branch_node[SFC.size()-1]]][dest][0] + (SFC[SFC.size()-1].size()-1)*(pkt_merge);
    // for(int i=0;i<SFC.size();i++){
    //     for(int j=0;j<SFC[i].size();j++){
    //         for(int k=0;k<SFC[i][j].size();k++){
    //             cout<<i<<","<<j<<","<<k<<":::"<<time[SFC[i][j][k]]<<endl;
    //         }
    //     }
    // }
    cout<<"e2e latency:::::::::::"<<dest_time<<endl;

    // layer graph step......
    for(int i=0;i<SFC.size();i++){
        if(SFC[i].size() == 1)
            continue;
        
        // UPDATE NEEDED!!!!!
        double max_tt = 0;
        double diff = 0;
        if(i == SFC.size()-1){
            diff = dest_time - time[SFC[i-1][SFC[i-1].size()-1][0]];
            diff -= (SFC[i].size()-1)*pkt_merge;
        }
        else{
            diff = time[SFC[i+1][SFC[i+1].size()-1][0]] - time[SFC[i-1][SFC[i-1].size()-1][0]];
            diff -= NFs[SFC[i+1][SFC[i+1].size()-1][0]] + (SFC[i].size()-1)*pkt_merge;
            // cout<<"DIFFFFFFFF:::::::::"<<diff<<endl;
            // cout<<time[SFC[i+1][SFC[i+1].size()-1][0]]<<"\n"<<time[SFC[i-1][SFC[i-1].size()-1][0]]<<"\n"<<NFs[SFC[i+1][SFC[i+1].size()-1][0]]<<"\n"<<(SFC[i].size()-1)*pkt_merge<<endl;
        }
        for(int b=0;b<SFC[i].size()-1;b++){ //checking all branches except critical branch since all function instances have been deployed in the critical branch
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

            if(deployed_inst.find(SFC[i][b][0]) == deployed_inst.end()){
                if(i == 0){
                    cout<<"Fnc1\n";
                    for(int b_n=0;b_n<SFC[i+1].size();b_n++){
                        if(!layer_graph(src,SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                            return false;
                    }
                }
                else{
                    if(i == SFC.size()-1){
                        cout<<"CHECKER FOR DEPLOYED::::::"<<deployed_inst[SFC[i-1][0][0]]<<" "<<SFC[i][b].size()<<endl;
                        if(!layer_graph(deployed_inst[SFC[i-1][0][0]],SFC[i][b],dest,temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                            return false;
                    }
                    else{
                        cout<<"Fnc2\n";
                        for(int b_n=0;b_n<SFC[i+1].size();b_n++){
                            // cout<<"HHHHHHHHHHHHHHHHHHHHHHHH:"<<b_n<<endl;
                            if(!layer_graph(deployed_inst[SFC[i-1][0][0]],SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                                return false;
                            // cout<<"SSSSSSSSSSSSSS::"<<deployed_inst[SFC[i-1][0][0]]<<"DDDDDDDDDDDDDDDD::"<<deployed_inst[SFC[i+1][b_n][0]]<<endl;
                        }
                    }
                }
            }
            else{
                if(i == SFC.size()-1){
                    cout<<"Fnc4\n";
                    if(!layer_graph(deployed_inst[SFC[i][b][0]],SFC[i][b],dest,temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                        return false;
                }
                else{
                    cout<<"Fnc5\n";
                    for(int b_n=0;b_n<SFC[i+1].size();b_n++){
                        if(!layer_graph(deployed_inst[SFC[i][b][0]],SFC[i][b],deployed_inst[SFC[i+1][b_n][0]],temp_g,time,deployed_inst,NF_to_node,NFs,temp_n_resource,paths,time_of_paths,request,result,diff,max_tt))
                            return false;
                    }
                }
            }
        }
        dest_time += max_tt;
    }
    if(dest_time > request.e2e)
        return false;
    g = temp_g;
    n_resource = temp_n_resource;

    // cout<<"Check:::::::::"<<deployed_inst[0]<<endl;

    for(int i=0;i<SFC.size();i++){
        for(int j=0;j<SFC[i].size();j++){
            for(int k=0;k<SFC[i][j].size();k++){
                cout<<i<<","<<j<<","<<k<<":::"<<deployed_inst[SFC[i][j][k]]<<endl;
            }
        }
    }
    for(int i=0;i<SFC.size();i++){
        for(int j=0;j<SFC[i].size();j++){
            for(int k=0;k<SFC[i][j].size();k++){
                result.nodes_util[deployed_inst[SFC[i][j][k]]]++;
            }
        }
    }
    cout<<"final e2e latency:::::::::::"<<dest_time<<endl;
    result.mean_latency += dest_time;
    return true;
}

vector<Request> generate_SFC(int n,int num_of_funcs,int g_low,int g_high){
    vector<Request> requests;
    for(int i=0;i<n;i++){
        Request request;
        int start,end;
        double e2e,arrival;
        int PE_size,SFC_length;
        vector<vector<vector<int>>> SFC;

        std::set<int> numbers;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, num_of_funcs-1);
        std::uniform_int_distribution<> dis_SFC(4, 8);

        // randomly generating the SFC length (3 <= length <= num_of_funcs)
        //SFC_length = dis_SFC(gen);
        SFC_length = 6;
        // cout<<"SFC Length::::::"<<SFC_length<<endl;

        // randomly generating the PE size (2 <= PE_size <= SFC_length-1)
        std::uniform_int_distribution<> dis_PE(2, SFC_length-1);
        PE_size = dis_PE(gen);
        // cout<<"PE Size::::::"<<PE_size<<endl;

        // randomly generating the SFC by forming a non repeating sequence
        while (numbers.size() < SFC_length) {
            numbers.insert(dis(gen));
        }

        vector<int> temp(numbers.begin(),numbers.end());

        std::uniform_int_distribution<> dis_part(1, temp.size()-PE_size);
        int low = dis_part(gen);
        for(int i=0;i<low;i++){
            vector<vector<int>> t(1,vector<int>(1,temp[i]));
            SFC.push_back(t);
        }
        vector<vector<int>> t1;
        for(int i=low;i<low+PE_size;i++){
            t1.push_back(vector<int>(1,temp[i]));
            
        }
        SFC.push_back(t1);
        for(int i=low+PE_size;i<temp.size();i++){
            vector<vector<int>> t2(1,vector<int>(1,temp[i]));
            SFC.push_back(t2);
        }

        std::uniform_real_distribution<> dis_e2e(150,200);
        request.SFC = SFC;
        request.e2e = dis_e2e(gen);
        std::uniform_real_distribution<> dis_arrival(1,1.5);
        request.t_arrival_rate = dis_arrival(gen);
        set<int> nodes;
        std::uniform_int_distribution<> dis_nodes(g_low, g_high);
        while (nodes.size() < 2) {
            nodes.insert(dis_nodes(gen));
        }
        vector<int> temp_nu(nodes.begin(),nodes.end());
        request.src = temp_nu[0];
        request.dest = temp_nu[1];
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
    }
    return requests;
}

double BW_used(vector<vector<node>>& g){
    double bw = 0;
    for(int i=0;i<g.size();i++){
        for(int j=0;j<g[i].size();j++){
            if(i < g[i][j].id)
                bw += (g[i][j].init_bw - g[i][j].available_bandwidth);
        }
    }
    return bw;
}

class comparator{
    public:
    bool operator () (Request& a, Request& b){
        return a.t_arrival_rate < b.t_arrival_rate;
    }
};

int main(){
    ofstream out("output_latency.txt");
    ofstream out1("output_PD.txt");
    ofstream out2("output_BW.txt");
    ofstream out3("output_AR.txt");
    ofstream out4("output_Nodes.txt");
    std::random_device rd;
    std::mt19937 gen(rd());

    int runs = 10;
    int n_of_nodes = 24;

    Result f_result(n_of_nodes);
    Result f_result_1(n_of_nodes);
    Result f_result_2(n_of_nodes);
    double f_AR=0;
    double f_AR_1=0;
    double f_AR_2=0;
    double f_bw=0;
    double f_bw_1=0;
    double f_bw_2=0;

    for(int times=0;times<runs;times++){
        ifstream fin("graph_config.txt");
        ifstream rin("request.txt");

        int N,M;
        int funcs,n_of_requests,n_of_f_instances;

        //rin>>n_of_requests;

        fin>>funcs>>n_of_f_instances;
        fin>>N>>M;
        fin>>n_of_requests;

        vector<vector<node>> g(N); //network topology
        vector<node_capacity> n_resource(N); //tells all the function instances deployed in a node in the topology

        vector<vector<node>> g_1(N); //network topology
        vector<node_capacity> n_resource_1(N); //tells all the function instances deployed in a node in the topology

        vector<vector<node>> g_2(N); //network topology
        vector<node_capacity> n_resource_2(N); //tells all the function instances deployed in a node in the topology

        vector<vector<int>> NF_to_node(funcs); //tells in which node each function is deployed in
        vector<Request> requests_pd;
        vector<Request> requests_1; //contains a batch of SFC requests
        vector<Request> requests_2;
        map<int,double> NFs;

        for(int i=0;i<funcs;i++){
            int f_id;
            double p_time;
            std::uniform_int_distribution<> func_time(1,20);
            //rin>>f_id>>p_time;
            f_id = i;
            p_time = func_time(gen);
            NFs[f_id] = p_time;
        }
        requests_pd = generate_SFC(n_of_requests,funcs,0,23);
        sort(requests_pd.begin(),requests_pd.end(),comparator());
        requests_1 = requests_pd;
        requests_2 = requests_pd;


        // vector<vector<vector<int>>> SFC{{{0}},{{1},{2},{3}},{{4}}};
        // vector<vector<vector<int>>> SFC1{{{0}},{{1},{2}},{{4}}};
        // SFC = bin(SFC,NFs);
        // SFC1 = bin(SFC1,NFs);

        // for(int i=0;i<SFC.size();i++){
        //     for(int j=0;j<SFC[i].size();j++){
        //         for(int k=0;k<SFC[i][j].size();k++){
        //             cout<<SFC[i][j][k]<<"->";
        //         }
        //         cout<<endl;
        //     }
        //     cout<<endl;
        // }
        //SFC = {{{0}},{{1,2},{3}},{{4}}};

        // for(int r=0;r<n_of_requests;r++){
        //     requests_2[r].SFC = bin_FP(requests_2[r].SFC,NFs);
        //     // for(int i=0;i<requests_2[r].SFC.size();i++){
        //     //     for(int j=0;j<requests_2[r].SFC[i].size();j++){
        //     //         for(int k=0;k<requests_2[r].SFC[i][j].size();k++){
        //     //             cout<<"("<<requests_2[r].SFC[i][j][k]<<")";
        //     //         }
        //     //         cout<<"-->";
        //     //     }
        //     // cout<<endl;
        //     // }
        // }
        // exit(0);
        int VMs = 4;
        double VM_cap = 5;
        for(int i=0;i<N;i++){
            std::uniform_int_distribution<> VM(1,VMs);
            int node_vm = VM(gen);
            std::set<int> numbers;
            std::uniform_int_distribution<> node_funcs(0, funcs-1);
            while (numbers.size() < node_vm) {
                numbers.insert(node_funcs(gen));
            }
            vector<int> f_in_node(numbers.begin(),numbers.end());
            for(int j=0;j<node_vm;j++){
                int f_id = f_in_node[j];
                double time;
                int node_id = i;
                //fin>>node_id>>f_id>>time;
                node_id = i;
                f_id = f_in_node[j];
                // node_capacity temp(f_id,time);
                n_resource[node_id].NF_left[f_id] = VM_cap;
                n_resource[node_id].deployed_NF[f_id] = NFs[f_id];

                n_resource_1[node_id].NF_left[f_id] = VM_cap;
                n_resource_1[node_id].deployed_NF[f_id] = NFs[f_id];

                n_resource_2[node_id].NF_left[f_id] = VM_cap;
                n_resource_2[node_id].deployed_NF[f_id] = NFs[f_id];

                NF_to_node[f_id].push_back(node_id);
            }
        }

        // cout<<"Node to funcs"<<endl;
        // for(int i=0;i<N;i++){
        //     cout<<i<<":"<<endl;
        //     map<int,double>::iterator it;
        //     for(it=n_resource[i].deployed_NF.begin();it!=n_resource[i].deployed_NF.end();it++){
        //         cout<<it->first+1<<":"<<it->second<<endl;
        //     }
        //     cout<<endl;
        // }
        // cout<<"Funcs to Node"<<endl;
        // for(int i=0;i<funcs;i++){
        //     cout<<i+1<<":"<<endl;
        //     for(int j=0;j<NF_to_node[i].size();j++){
        //         cout<<NF_to_node[i][j]<<" ";
        //     }
        //     cout<<endl;
        // }

        for(int i=0;i<M;i++){
            int s,d;
            double l,b;
            fin>>s>>d>>l>>b;
            std::uniform_int_distribution<> bw(20,30);
            std::uniform_int_distribution<> link(4,8);
            int temp1 =d ;
            int temp2 = s;
            b = bw(gen);
            l = link(gen);
            node n1(temp1,l,b);
            node n2(temp2,l,b);
            g[s].push_back(n1);
            g[d].push_back(n2);
            g_1[s].push_back(n1);
            g_1[d].push_back(n2);
            g_2[s].push_back(n1);
            g_2[d].push_back(n2);
        }

        // for(int i=0;i<g.size();i++){
        //     for(int j=0;j<g[i].size();j++){
        //         cout<<i<<"->"<<g[i][j].id<<"("<<g[i][j].link<<")"<<"("<<g[i][j].available_bandwidth<<")"<<endl;
        //     }
        // }

        // cout<<"Node to funcs"<<endl;
        // for(int i=0;i<N;i++){
        //     cout<<i<<":"<<endl;
        //     map<int,double>::iterator it;
        //     for(it=n_resource[i].deployed_NF.begin();it!=n_resource[i].deployed_NF.end();it++){
        //         cout<<it->first<<":"<<it->second<<endl;
        //     }
        //     cout<<endl;
        // }

        // Request request(0,6,SFC,270,12);
        // request.SFC = bin(request.SFC,NFs);
        // Request request1(0,6,SFC1,270,12);
        // requests[0] = request;
        // requests[1] = request1;
        
        //do some sort on the requests
        Result result(N);
        bool res;
        double AR = 0;
        for(int i=0;i<n_of_requests;i++){
            requests_pd[i].SFC = bin(requests_pd[i].SFC,NFs);
            res = SFC_embedding_PD(g,n_resource,NF_to_node,NFs,requests_pd[i],result);
            if(res)
                AR++;
        }
        Result result_1(N);
        bool res_1;
        double AR_1 = 0;
        for(int i=0;i<n_of_requests;i++){
            requests_1[i].SFC = bin(requests_1[i].SFC,NFs);
            res_1 = SFC_embedding(g_1,n_resource_1,NF_to_node,NFs,requests_1[i],result_1);
            if(res_1)
                AR_1++;
        }
        Result result_2(N);
        bool res_2;
        double AR_2 = 0;
        for(int i=0;i<n_of_requests;i++){
            requests_2[i].SFC = bin_FP(requests_2[i].SFC,NFs);
            res_2 = SFC_embedding(g_2,n_resource_2,NF_to_node,NFs,requests_2[i],result_2);
            if(res_2)
                AR_2++;
        }
        // SFC_embedding_PD(g,n_resource,NF_to_node,NFs,request,result);
        
        // for(int i=0;i<g.size();i++){
        //     for(int j=0;j<g[i].size();j++){
        //         cout<<i<<"->"<<g[i][j].id<<"("<<g[i][j].available_bandwidth<<")"<<endl;
        //     }
        // }

        // cout<<"Node to funcs"<<endl;
        // for(int i=0;i<N;i++){
        //     cout<<i<<":"<<endl;
        //     map<int,double>::iterator it;
        //     for(it=n_resource[i].NF_left.begin();it!=n_resource[i].NF_left.end();it++){
        //         cout<<it->first+1<<":"<<it->second<<endl;
        //     }
        //     cout<<endl;
        // }

        out3<<AR/n_of_requests<<","<<AR_1/n_of_requests<<","<<AR_2/n_of_requests<<endl;

        f_AR += AR/n_of_requests;
        f_AR_1 += AR_1/n_of_requests;
        f_AR_2 += AR_2/n_of_requests;


        double bw = BW_used(g);
        f_bw += bw;
        double bw_1 = BW_used(g_1);
        f_bw_1 += bw_1;
        double bw_2 = BW_used(g_2);
        f_bw_2 += bw_2;
        //here
        //if(res)
            out<<result.mean_latency/AR<<","<<result_1.mean_latency/AR_1<<","<<result_2.mean_latency/AR_2<<endl;
        f_result.mean_latency += result.mean_latency/AR;
        f_result_1.mean_latency += result_1.mean_latency/AR_1;
        f_result_2.mean_latency += result_2.mean_latency/AR_2;
        //if(res_1)
            out1<<result.mean_PD/AR<<","<<result_1.mean_PD/AR_1<<","<<result_2.mean_PD/AR_2<<endl;
        f_result.mean_PD += result.mean_PD/AR;
        f_result_1.mean_PD += result_1.mean_PD/AR_1;
        f_result_2.mean_PD += result_2.mean_PD/AR_2;

        out2<<bw<<","<<bw_1<<","<<bw_2<<endl;

        for(int i=0;i<result.nodes_util.size();i++){
            out4<<i<<":"<<result.nodes_util[i]/AR<<","<<result_1.nodes_util[i]/AR_1<<","<<result_2.nodes_util[i]/AR_2<<endl;
            f_result.nodes_util[i] += result.nodes_util[i]/AR;
            f_result_1.nodes_util[i] += result_1.nodes_util[i]/AR_1;
            f_result_2.nodes_util[i] += result_2.nodes_util[i]/AR_2;

        }
    }
    cout<<"Final mean latency:"<<f_result.mean_latency/runs<<","<<f_result_1.mean_latency/runs<<","<<f_result_2.mean_latency/runs<<endl;
    cout<<"Final mean PD:"<<f_result.mean_PD/runs<<","<<f_result_1.mean_PD/runs<<","<<f_result_2.mean_PD/runs<<endl;
    cout<<"Final mean bw:"<<f_bw/runs<<","<<f_bw_1/runs<<","<<f_bw_2/runs<<endl;
    cout<<"Final mean AR:"<<f_AR/runs<<","<<f_AR_1/runs<<","<<f_AR_2/runs<<endl;
    cout<<"Node Utilization:"<<endl;
    for(int i=0;i<n_of_nodes;i++){
        cout<<f_result.nodes_util[i]<<","<<f_result_1.nodes_util[i]<<","<<f_result_2.nodes_util[i]<<endl;
    }
    return 0;
}