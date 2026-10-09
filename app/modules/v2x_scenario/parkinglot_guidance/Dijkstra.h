/*********************************************************************************
 * @file		Dijkstra.h
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

#ifndef PARKING_LOT_GUIDANCE_DIJKSTRA_H
#define PARKING_LOT_GUIDANCE_DIJKSTRA_H

#include <algorithm>
#include <iostream>
#include <vector>
#include <map>

namespace airos {
namespace app {

const int DIJISTRA_MAXVEX = 100;     // 图中最多顶点的数量
const int DIJISTRA_INFINITY = 65535; // 无穷大的数,表示顶点和顶点之间不通

typedef struct MGraph
{
    int vexs[DIJISTRA_MAXVEX];
    int arc[DIJISTRA_MAXVEX][DIJISTRA_MAXVEX];
    int num_vertexes, numEdges;
} MGraph_t;

typedef struct DijkstraResult
{
    std::vector<int> path;
    int dist;
    int seq;
} DijkstraResult_t;

class Dijkstra
{
public:
    Dijkstra();
    virtual ~Dijkstra();
    /**
     * @brief   Dijkstra算法, 求出最短路径
     * @param G     是一个带权有向图。
     * @param v     出发点的节点ID。
     * @param dist[]    是当前求到的从顶点v到顶点j的最短路径长度。
     * @param path[]    存放求到的最短路径,最短路径的上一个节点信息。
     */
    static void DijkstraCalcu(const MGraph_t &G, const int v, int dist[], int path[]);
    
    /**
     * @brief   Dijkstra算法, 求出最短路径
     * @param G     是一个带权有向图。
     * @param v     出发点的节点ID。
     * @param dijkstraResult  存放最短路径的输出结果, vector按照从近到远的距离排序。
     */
    static void DijkstraCalcu(const MGraph_t &G, const int v, std::vector<DijkstraResult_t> &dijkstraResult);
    
    /**
     * @brief   打印出到每个节点的最小距离及路径
     * @param G     是一个带权有向图。
     * @param dist[]    是当前求到的从顶点v到顶点j的最短路径长度。
     * @param path[]    存放求到的最短路径,最短路径的上一个节点信息。
     */
    static void printShortestPath(const MGraph_t &G, const int v, const int dist[], const int path[]);
};
} }

#endif // PARKING_LOT_GUIDANCE_DIJKSTRA_H
