#include "MeshSimplification.h"

#include <iostream>
#include <utility>
#include <algorithm>
#include <queue>
#include <cmath>
#include <tuple>
#include <set>

#define EPSILON 1e-5
using namespace std;
using edge = tuple<float, int, int>;

vector<int> findSameFace(vector<int> a, vector<int> b)
{
	vector<int> res;
	for (vector<int>::iterator iter = a.begin(); iter != a.end(); ++iter)
	{
		if (find(b.begin(), b.end(), *iter) != b.end())
			res.push_back(*iter);
	}
	return res;
}

namespace meshSimplification
{
	//参数均为复制，因此可以直接在上面处理
	std::pair<std::vector<Eigen::Vector3f>, std::vector<Eigen::Vector3i>> simplifyMesh(
		// 每个元素为一个三维向量，表示每个节点的位置
		std::vector<Eigen::Vector3f> vertices, 
		// 每个元素是三个整型变量，表示一个三角形
		std::vector<Eigen::Vector3i> faces,
		// 模型简化比例
		float ratio)
	{
		// 标记节点是否被删除，实际上是bool，但不推荐std::vector<bool>
		std::deque<bool> deleted(vertices.size());		// 设定为std::deque<bool> 避免vector<bool>的指针悬垂 用bitset<size>也行
		
		// 每个节点保存它所含的面的编号，方便反向查询
		std::vector<std::vector<int>> faces_of_vertex(vertices.size());
		for (int i = 0; i < faces.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				faces_of_vertex[faces[i][j]].push_back(i);
			}
		}
		for (int i = 0; i < vertices.size(); i++)
		{
			if (faces_of_vertex[i].size() == 0)
				deleted[i] = true;
		}
		cout << vertices.size() << endl;
		//TODO:
		// 完成后续代码，包括：
		// 1. 计算Q矩阵
		// 2. 计算节点间消耗，并储存(std::priority_queue)
		// 3. 收缩节点，直到达到ratio要求
		// 注意收缩节点后更新faces_of_vertex和deleted
		
		// 1. 计算Q矩阵
		// Q = $sum Kp = $sum (p^T * p)
		vector<Eigen::Matrix4f> quadric_of_face(faces.size());
		vector<Eigen::Matrix4f> quadric_of_vertex(vertices.size());
		// 平面方程：ax + by + cz + d = 0
		//		  = (a, b, c, d) dot (x, y, z, 1)^T
		//		  = p dot v^T
		// && a^2 + b^2 + c^2 = 0
		for (int i = 0; i < faces.size(); i++) 
		{
			Eigen::Vector3i face = faces[i];
			Eigen::Vector3f Edge1 = vertices[face[0]] - vertices[face[1]];			
			Eigen::Vector3f Edge2 = vertices[face[0]] - vertices[face[2]];	// 求得两条边的向量
			Eigen::Vector3f normal = Edge1.cross(Edge2);					// 叉乘得到该平面未经处理的法向量
			normal.normalize();												// 归一化使a^2 + b^2 + c^2 = 1
			float d = -normal.dot(vertices[face[0]]);						// d = -ax0 - by0 - cz0 = -((a, b, c) dot (x0, y0, z0)^T)
			Eigen::Vector4f p(normal[0], normal[1], normal[2], d);			// 得到 p = (a, b, c, d)
			quadric_of_face[i] = p* p.transpose();					// Kp = p^T dot p
		}
		// 将顶点周围所有三角形的误差值相加
		for (int i = 0; i < vertices.size(); i++)
		{
			Eigen::Matrix4f sum_quadric = Eigen::Matrix4f::Zero();			// 初始化一个0矩阵
			
			for (int j = 0; j < faces_of_vertex[i].size(); j++)
			{
				sum_quadric += quadric_of_face[faces_of_vertex[i][j]];		// 将vertex周围所有face的q相加
			}
			quadric_of_vertex[i] = sum_quadric;								// 得到每个节点的Q矩阵
		}

		// 2. 计算节点间消耗，并储存
		vector<vector<int>> graph(vertices.size(), vector<int>(vertices.size()));	// 建立邻接矩阵
		for (Eigen::Vector3i face : faces)
		{
			graph[face[0]][face[1]] = graph[face[1]][face[0]] = 1;
			graph[face[0]][face[2]] = graph[face[2]][face[0]] = 1;
			graph[face[1]][face[2]] = graph[face[2]][face[1]] = 1;
		}

		priority_queue<edge, vector<edge>, greater<edge>> edges;
		for (int i = 0; i < vertices.size(); i++) 
		{
			for (int j = i + 1; j < vertices.size(); j++) 
			{
				if (graph[i][j] == 1 && !deleted[i] && !deleted[j])
				{								
					Eigen::Vector4f optimal;										// 最佳位置
					Eigen::Matrix4f edge_quadric = quadric_of_vertex[i] + quadric_of_vertex[j];
					Eigen::Matrix4f det = edge_quadric;
					for (int i = 0; i < 3; i++)
						det(3, i) = 0;
					det(3, 3) = 1;
					if (abs(det.determinant()) < EPSILON)
					{
						Eigen::Vector3f tmp = (vertices[i] + vertices[j]) / 2;		// 如果无解取中点
						optimal = Eigen::Vector4f(tmp[0], tmp[1], tmp[2], 1);
					}
					else
						optimal = det.inverse() * Eigen::Vector4f(0, 0, 0, 1);		// 如果有解算出最佳位置
					float cost = optimal.transpose() * edge_quadric * optimal;
					edges.emplace(abs(cost), i, j);									// 将{cost, v1, v2}的元组加入边的集合
				}
			}
		}

		// 3. 收缩节点，直到达到ratio要求 更新faces_of_vertex和deleted
		int ori_face_cnt = faces.size();
		int eff_face_cnt = faces.size();
		while (eff_face_cnt > ratio * ori_face_cnt)
		{
			// 取出堆顶的边
			edge _edge = edges.top();
			float _cost = get<0>(_edge);
			int v1_id = get<1>(_edge);
			int v2_id = get<2>(_edge);
			edges.pop();

			// 如果v1或v2已被删除 该边无效
			if (deleted[v1_id] || deleted[v2_id]) continue;

			// 获得坍缩边v1-v2的最佳位置
			Eigen::Vector4f _optimal;
			Eigen::Matrix4f _edge_quadric = quadric_of_vertex[v1_id] + quadric_of_vertex[v2_id];
			Eigen::Matrix4f _det = _edge_quadric;
			for (int i = 0; i < 3; i++)
				_det(3, i) = 0;
			_det(3, 3) = 1;
			if (abs(_det.determinant()) < EPSILON)
			{
				Eigen::Vector3f tmp = (vertices[v1_id] + vertices[v2_id]) / 2;
				_optimal = Eigen::Vector4f(tmp[0], tmp[1], tmp[2], 1);
			}
			else
				_optimal = _det.inverse() * Eigen::Vector4f(0, 0, 0, 1);

			// 满足上述条件后
			Eigen::Vector3f v = _optimal.block(0, 0, 3, 1);
			int v_id = vertices.size();
			vertices.push_back(v);						// 将新的点加入点集
			quadric_of_vertex.push_back(_edge_quadric);
			deleted.push_back(false);					// 同时必须将存q矩阵和deleted的数组扩大
			deleted[v1_id] = true;
			deleted[v2_id] = true;

			// 将v1和v2的邻面加入v 删除以v1-v2为边的面
			vector<int> common_faces = findSameFace(faces_of_vertex[v1_id], faces_of_vertex[v2_id]);
			eff_face_cnt -= common_faces.size();
			for (int face_id : common_faces)
			{
				faces[face_id] = Eigen::Vector3i{ -1,-1,-1 };
			}
			vector<int> faces_of_v;
			for (int i = 0; i < faces_of_vertex[v1_id].size(); i++)
			{
				if (faces_of_vertex[v1_id][i] != -1)
				{
					if (faces[faces_of_vertex[v1_id][i]][0] == -1)
						faces_of_vertex[v1_id][i] = -1;
					else
					{
						for (int j = 0; j < 3; j++)
						{
							if (faces[faces_of_vertex[v1_id][i]][j] == v1_id)
								faces[faces_of_vertex[v1_id][i]][j] = v_id;
						}
						faces_of_v.push_back(faces_of_vertex[v1_id][i]);
						faces_of_vertex[v1_id][i] = -1;
					}
				}
			}
			for (int i = 0; i < faces_of_vertex[v2_id].size(); i++)
			{
				if (faces_of_vertex[v2_id][i] != -1)
				{
					if (faces[faces_of_vertex[v2_id][i]][0] == -1)
						faces_of_vertex[v2_id][i] = -1;
					else
					{
						for (int j = 0; j < 3; j++)
						{
							if (faces[faces_of_vertex[v2_id][i]][j] == v2_id)
								faces[faces_of_vertex[v2_id][i]][j] = v_id;
						}
						faces_of_v.push_back(faces_of_vertex[v2_id][i]);
						faces_of_vertex[v2_id][i] = -1;
					}
				}
			}
			faces_of_vertex.push_back(faces_of_v);		// v在vertices中的位置与此对应

			// 重新计算与v相邻的面的quadric 得到邻接点
			vector<int> near_vertices_id;
			for (int face_id : faces_of_v)
			{
				if (faces[face_id][0] != v_id)  near_vertices_id.push_back(faces[face_id][0]);
				if (faces[face_id][1] != v_id)  near_vertices_id.push_back(faces[face_id][1]);
				if (faces[face_id][2] != v_id)  near_vertices_id.push_back(faces[face_id][2]);
			}

			// 去重
			sort(near_vertices_id.begin(), near_vertices_id.end());  
			near_vertices_id.erase(unique(near_vertices_id.begin(), 
			near_vertices_id.end()), near_vertices_id.end());

			for (int vertex_id : near_vertices_id)
			{
				Eigen::Vector4f optimal;
				Eigen::Matrix4f edge_quadric = quadric_of_vertex[v_id] + quadric_of_vertex[vertex_id];
				Eigen::Matrix4f det = edge_quadric;
				for (int i = 0; i < 3; i++)
					det(3, i) = 0;
				det(3, 3) = 1;
				if (abs(det.determinant()) < EPSILON)
				{
					Eigen::Vector3f tmp = (vertices[v_id] + vertices[vertex_id]) / 2;
					optimal = Eigen::Vector4f(tmp[0], tmp[1], tmp[2], 1);
				}
				else
					optimal = det.inverse() * Eigen::Vector4f(0, 0, 0, 1);
				float cost = optimal.transpose() * edge_quadric * optimal;
				edges.emplace(abs(cost), v_id, vertex_id);
			}
		}

		//去除已删除的面和节点：对于faces_of_vertex中删除的面，可以标作-1以防止访问溢出。
		int vertex_count = 0, face_count = 0;
		for (int i = 0; i < vertices.size(); i++)
		{
			if (deleted[i])
			{
				for (int face_id : faces_of_vertex[i])
					if(face_id != -1)
						faces[face_id] = Eigen::Vector3i{ -1,-1,-1 };
			}
			else
			{
				vertices[vertex_count] = vertices[i];
				for (int face_id : faces_of_vertex[i])
					if (face_id != -1)
						for (int j = 0; j < 3; j++)
							if (faces[face_id][j] == i)
								faces[face_id][j] = vertex_count;
				vertex_count++;
			}
		}
		for (int i = 0; i < faces.size(); i++)
		{
			bool is_face_valid = true;
			for (int j = 0; j < 3; j++)
			{
				if (faces[i][j] == -1)
				{
					is_face_valid = false;
					break;
				}
			}
			if (is_face_valid)
			{
				faces[face_count] = faces[i];
				face_count++;
				for (int j = 0; j < 3; j++)
				{
					if (faces[i][j] >= vertex_count)
					{
						// 如果在这里报告错误，说明你之前的数值出现问题，如faces_of_vertex未能正常更新、或faces数组使用了不想要的值等。
						throw "Error! Vertex Index exceeds!";
					}
				}
			}
		}
		vertices.resize(vertex_count);
		faces.resize(face_count);
		// 返回新的模型
		return { std::move(vertices), std::move(faces) };
	}
}

