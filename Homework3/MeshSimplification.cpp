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
	//������Ϊ���ƣ���˿���ֱ�������洦��
	std::pair<std::vector<Eigen::Vector3f>, std::vector<Eigen::Vector3i>> simplifyMesh(
		// ÿ��Ԫ��Ϊһ����ά��������ʾÿ���ڵ��λ��
		std::vector<Eigen::Vector3f> vertices, 
		// ÿ��Ԫ�����������ͱ�������ʾһ��������
		std::vector<Eigen::Vector3i> faces,
		// ģ�ͼ򻯱���
		float ratio)
	{
		// ��ǽڵ��Ƿ�ɾ����ʵ������bool�������Ƽ�std::vector<bool>
		std::deque<bool> deleted(vertices.size());		// �趨Ϊstd::deque<bool> ����vector<bool>��ָ������ ��bitset<size>Ҳ��
		
		// ÿ���ڵ㱣������������ı�ţ����㷴���ѯ
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
		// ��ɺ������룬������
		// 1. ����Q����
		// 2. ����ڵ�����ģ�������(std::priority_queue)
		// 3. �����ڵ㣬ֱ���ﵽratioҪ��
		// ע�������ڵ�����faces_of_vertex��deleted
		
		// 1. ����Q����
		// Q = $sum Kp = $sum (p^T * p)
		vector<Eigen::Matrix4f> quadric_of_face(faces.size());
		vector<Eigen::Matrix4f> quadric_of_vertex(vertices.size());
		// ƽ�淽�̣�ax + by + cz + d = 0
		//		  = (a, b, c, d) dot (x, y, z, 1)^T
		//		  = p dot v^T
		// && a^2 + b^2 + c^2 = 0
		for (int i = 0; i < faces.size(); i++) 
		{
			Eigen::Vector3i face = faces[i];
			Eigen::Vector3f Edge1 = vertices[face[0]] - vertices[face[1]];			
			Eigen::Vector3f Edge2 = vertices[face[0]] - vertices[face[2]];	// ��������ߵ�����
			Eigen::Vector3f normal = Edge1.cross(Edge2);					// ��˵õ���ƽ��δ������ķ�����
			normal.normalize();												// ��һ��ʹa^2 + b^2 + c^2 = 1
			float d = -normal.dot(vertices[face[0]]);						// d = -ax0 - by0 - cz0 = -((a, b, c) dot (x0, y0, z0)^T)
			Eigen::Vector4f p(normal[0], normal[1], normal[2], d);			// �õ� p = (a, b, c, d)
			quadric_of_face[i] = p* p.transpose();					// Kp = p^T dot p
		}
		// ��������Χ���������ε����ֵ���
		for (int i = 0; i < vertices.size(); i++)
		{
			Eigen::Matrix4f sum_quadric = Eigen::Matrix4f::Zero();			// ��ʼ��һ��0����
			
			for (int j = 0; j < faces_of_vertex[i].size(); j++)
			{
				sum_quadric += quadric_of_face[faces_of_vertex[i][j]];		// ��vertex��Χ����face��q���
			}
			quadric_of_vertex[i] = sum_quadric;								// �õ�ÿ���ڵ��Q����
		}

		// 2. ����ڵ�����ģ�������
		vector<vector<int>> graph(vertices.size(), vector<int>(vertices.size()));	// �����ڽӾ���
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
					Eigen::Vector4f optimal;										// ���λ��
					Eigen::Matrix4f edge_quadric = quadric_of_vertex[i] + quadric_of_vertex[j];
					Eigen::Matrix4f det = edge_quadric;
					for (int i = 0; i < 3; i++)
						det(3, i) = 0;
					det(3, 3) = 1;
					if (abs(det.determinant()) < EPSILON)
					{
						Eigen::Vector3f tmp = (vertices[i] + vertices[j]) / 2;		// ����޽�ȡ�е�
						optimal = Eigen::Vector4f(tmp[0], tmp[1], tmp[2], 1);
					}
					else
						optimal = det.inverse() * Eigen::Vector4f(0, 0, 0, 1);		// ����н�������λ��
					float cost = optimal.transpose() * edge_quadric * optimal;
					edges.emplace(abs(cost), i, j);									// ��{cost, v1, v2}��Ԫ�����ߵļ���
				}
			}
		}

		// 3. �����ڵ㣬ֱ���ﵽratioҪ�� ����faces_of_vertex��deleted
		int ori_face_cnt = faces.size();
		int eff_face_cnt = faces.size();
		while (eff_face_cnt > ratio * ori_face_cnt)
		{
			// ȡ���Ѷ��ı�
			edge _edge = edges.top();
			float _cost = get<0>(_edge);
			int v1_id = get<1>(_edge);
			int v2_id = get<2>(_edge);
			edges.pop();

			// ���v1��v2�ѱ�ɾ�� �ñ���Ч
			if (deleted[v1_id] || deleted[v2_id]) continue;

			// ���̮����v1-v2�����λ��
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

			// ��������������
			Eigen::Vector3f v = _optimal.block(0, 0, 3, 1);
			int v_id = vertices.size();
			vertices.push_back(v);						// ���µĵ����㼯
			quadric_of_vertex.push_back(_edge_quadric);
			deleted.push_back(false);					// ͬʱ���뽫��q�����deleted����������
			deleted[v1_id] = true;
			deleted[v2_id] = true;

			// ��v1��v2���������v ɾ����v1-v2Ϊ�ߵ���
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
			faces_of_vertex.push_back(faces_of_v);		// v��vertices�е�λ����˶�Ӧ

			// ���¼�����v���ڵ����quadric �õ��ڽӵ�
			vector<int> near_vertices_id;
			for (int face_id : faces_of_v)
			{
				if (faces[face_id][0] != v_id)  near_vertices_id.push_back(faces[face_id][0]);
				if (faces[face_id][1] != v_id)  near_vertices_id.push_back(faces[face_id][1]);
				if (faces[face_id][2] != v_id)  near_vertices_id.push_back(faces[face_id][2]);
			}

			// ȥ��
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

		//ȥ����ɾ������ͽڵ㣺����faces_of_vertex��ɾ�����棬���Ա���-1�Է�ֹ���������
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
						// ��������ﱨ�����˵����֮ǰ����ֵ�������⣬��faces_of_vertexδ���������¡���faces����ʹ���˲���Ҫ��ֵ�ȡ�
						throw "Error! Vertex Index exceeds!";
					}
				}
			}
		}
		vertices.resize(vertex_count);
		faces.resize(face_count);
		// �����µ�ģ��
		return { std::move(vertices), std::move(faces) };
	}
}

