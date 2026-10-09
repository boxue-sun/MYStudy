/*********************************************************************************
 * @file		Dijkstra.cpp
 * @brief		Dijkstra belongs to ncs_4layer
 * @details		Dijkstra belongs to ncs_4layer
 * @author		Changxuhui
 * @date		2022/2/15 
 * @copyright	Copyright (c) 2022 Gohigh V2X Division.
 * @verbatim
 *  
 *
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2022/2/15   ChangXuhui       1.0       ————            Create this file
 * 													   
 * @endverbatim
 ********************************************************************************/

#include "Dijkstra.h"

namespace airos {
namespace app {
Dijkstra::Dijkstra()
{
}

Dijkstra::~Dijkstra()
{
}

void Dijkstra::DijkstraCalcu(const MGraph_t &G, const int v, int dist[], int path[])
{
    int i, j, k, min;
    bool final[G.num_vertexes]; // final[w]=1表示"顶点v0"到"顶点w"的最短路径已成功获取。标志位
    for (i = 0; i < G.num_vertexes; ++i)
    {
        final[i] = false;                // 所有顶点均未初始化最短路径
        dist[i] = G.arc[v][i];           // 顶点v0与其它顶点的权值
        if (dist[i] < DIJISTRA_INFINITY) // 顶点i是v的邻接顶点
        {
            path[i] = v; // 暂时将v标记为顶点i的最短路径
        }
        else
        {
            path[i] = -1; // 说明该顶点i与顶点v没有边相连
        }
    }
    dist[v] = 0;     // 起点v到v(自身到自身)的最短路径设置为0
    final[v] = true; // 已经确定v到v的最短路径，将标志位置为1

    // 求v0点到每个顶点的距离
    for (i = 1; i < G.num_vertexes; ++i)
    {
        min = DIJISTRA_INFINITY;
        for (j = 0; j < G.num_vertexes; ++j)
        {
            if (final[j] == false && dist[j] < min)
            {
                k = j;
                min = dist[j];
            }
        }
        final[k] = true; // 将目前找到的最近的顶点设置为true
        // 修正当前的最短距离和路径
        for (j = 0; j < G.num_vertexes; ++j)
        {
            if (final[j] == false && min + G.arc[k][j] < dist[j])
            {
                dist[j] = min + G.arc[k][j];
                path[j] = k;
            }
        }
    }
}

void Dijkstra::DijkstraCalcu(const MGraph_t &G, const int v, std::vector<DijkstraResult_t> &dijkstraResult)
{
    int dist[G.num_vertexes];
    int path[G.num_vertexes];
    DijkstraCalcu(G, v, dist, path);

    // 存放multimap中，按距离排序 (可能有相同的距离因此用multimap)
    std::multimap<int, int> tempDist;
    for (int i = 0; i < G.num_vertexes; ++i)
    {
        if ((dist[i] > 0) && dist[i] < DIJISTRA_INFINITY)
        {
            tempDist.insert({dist[i], i});
        }
    }

    for (auto iter = tempDist.cbegin(); iter != tempDist.cend(); ++iter)
    {
        DijkstraResult_t tempDijkstraResult;
        tempDijkstraResult.seq = iter->second;
        tempDijkstraResult.dist = iter->first;
        int j;
        // 求其他点到起始点v的路径信息
        if (iter->second != v)
        {
            j = iter->second;
            // 搜索到起点v为止，将最路径存放到tempDijkstraResult.path中。
            while (j != v)
            {
                tempDijkstraResult.path.push_back(j);
                j = path[j];
            }
            reverse(tempDijkstraResult.path.begin(), tempDijkstraResult.path.end());
        }
        dijkstraResult.emplace_back(tempDijkstraResult);
    }
}
    

// 从path数组打印最短路径的算法
void Dijkstra::printShortestPath(const MGraph_t &G, const int v, const int dist[], const int path[])
{
    int i, j;
    std::cout << "从顶点" << v << "到其他各顶点的最短路径为: " << std::endl;
    std::vector<int> short_path;
    for (i = 0; i < G.num_vertexes; ++i)
    {
        if (path[i] == -1)
        {
            std::cout << "顶点 " << v << " 到顶点 " << i << " 不存在通路" << std::endl;
            continue;
        }
        // 求其他点到起始点v的路径信息
        if (i != v)
        {
            j = i;
            // 搜索到起点v为止，将最路径存放到short_path中。
            while (j != v)
            {
                short_path.push_back(j);
                j = path[j];
            }
            reverse(short_path.begin(), short_path.end());
            std::cout << "顶点 " << v << " 到顶点 " << i << " 的最短距离为: " << dist[i];
            std::cout << "   路径为:\t" << v;
            for (auto temp_date : short_path)
            {
                std::cout << " --> " << temp_date;
            }
            std::cout << std::endl;
        }
        short_path.clear();
    }
}

} }
