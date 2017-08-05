/*
 * canonical_graph.cpp
 *
 *  Created on: Aug 4, 2017
 *      Author: icuzzq
 */

#include "canonical_graph.hpp"

namespace RStream {

	Canonical_Graph::Canonical_Graph() : number_of_vertices(0), hash_value(0){

	}

//	Canonical_Graph(std::vector<Element_In_Tuple>& tuple, unsigned int num_vertices, unsigned int hash_v)
//		: tuple(tuple), number_of_vertices(num_vertices), hash_value(hash_v) {
//
//	}

	Canonical_Graph::Canonical_Graph(bliss::AbstractGraph* ag, bool is_directed){
		construct_cg(ag, is_directed);
	}

	Canonical_Graph::~Canonical_Graph(){

	}


	int Canonical_Graph::cmp(const Canonical_Graph& other_cg) const {
		//compare the numbers of vertices
		if(get_number_vertices() < other_cg.get_number_vertices()){
			return -1;
		}
		if(get_number_vertices() > other_cg.get_number_vertices()){
			return 1;
		}

		//compare hash value
		if(get_hash() < other_cg.get_hash()){
			return -1;
		}
		if(get_hash() > other_cg.get_hash()){
			return 1;
		}

		//compare edges
		assert(tuple.size() == other_cg.tuple.size());
		for(unsigned int i = 0; i < tuple.size(); ++i){
			const Element_In_Tuple & t1 = tuple[i];
			const Element_In_Tuple & t2 = other_cg.tuple[i];

			int cmp_element = t1.cmp(t2);
			if(cmp_element != 0){
				return cmp_element;
			}
		}

		return 0;
	}



	void Canonical_Graph::construct_cg(bliss::AbstractGraph* ag, bool is_directed){
		assert(!is_directed);
		if(!is_directed){
			number_of_vertices = ag->get_nof_vertices();
			hash_value = ag->get_hash();
//			std::cout << number_of_vertices << ", " << hash_value << std::endl;

			transform_to_tuple(ag);
		}
	}

	void Canonical_Graph::transform_to_tuple(bliss::AbstractGraph* ag){
		bliss::Graph* graph = (bliss::Graph*) ag;
		std::unordered_set<VertexId> set;
		std::unordered_map<VertexId, BYTE> map;
		std::priority_queue<Edge, std::vector<Edge>, EdgeComparator> min_heap;

		std::vector<bliss::Graph::Vertex> vertices = graph->get_vertices_rstream();

		VertexId first_src = init_heapAndset(vertices, min_heap, set);
		assert(first_src != -1);
		push_first_element(first_src, map, vertices);
//		std::cout << "tuple: " << tuple << std::endl;

		while(!min_heap.empty()){
			Edge edge = min_heap.top();
			push_element(edge, map, vertices);
//			std::cout << "tuple: " << tuple << std::endl;

			min_heap.pop();
			add_neighbours(edge, min_heap, vertices, set);
		}
	}


	VertexId Canonical_Graph::init_heapAndset(std::vector<bliss::Graph::Vertex>& vertices, std::priority_queue<Edge, std::vector<Edge>, EdgeComparator>& min_heap, std::unordered_set<VertexId>& set){
		for(unsigned int i = 0; i < vertices.size(); ++i){
			if(!vertices[i].edges.empty()){
				for(auto v: vertices[i].edges){
					min_heap.push(Edge(i, v));
				}
				set.insert(i);
				return i;
			}
		}

		return -1;
	}

	void Canonical_Graph::push_first_element(VertexId first, std::unordered_map<VertexId, BYTE>& map, std::vector<bliss::Graph::Vertex>& vertices){
		map[first] = 0;
		tuple.push_back(Element_In_Tuple(first + 1, (BYTE)0, (BYTE)vertices[first].color, (BYTE)0));
	}

	void Canonical_Graph::push_element(Edge& edge, std::unordered_map<VertexId, BYTE>& map, std::vector<bliss::Graph::Vertex>& vertices){
		assert(edge.src < edge.target);
		if(map.find(edge.src) != map.end()){
			tuple.push_back(Element_In_Tuple(edge.target + 1, (BYTE)0, (BYTE)vertices[edge.target].color, (BYTE)map[edge.src]));
			if(map.find(edge.target) == map.end()){
				unsigned int s = tuple.size() - 1;
				map[edge.target] = s;
			}
		}
		else if(map.find(edge.target) != map.end()){
			tuple.push_back(Element_In_Tuple(edge.src + 1, (BYTE)0, (BYTE)vertices[edge.src].color, (BYTE)map[edge.target]));
			if(map.find(edge.src) == map.end()){
				unsigned int s = tuple.size() - 1;
				map[edge.src] = s;
			}
		}
		else{
			//wrong case
	    	std::cout << "wrong case!!!" << std::endl;
	    	throw std::exception();
		}
	}

	void Canonical_Graph::add_neighbours(Edge& edge, std::priority_queue<Edge, std::vector<Edge>, EdgeComparator>& min_heap, std::vector<bliss::Graph::Vertex>& vertices, std::unordered_set<VertexId>& set){
		add_neighbours(edge.src, min_heap, vertices, set);
		add_neighbours(edge.target, min_heap, vertices, set);
	}

	void Canonical_Graph::add_neighbours(VertexId srcId, std::priority_queue<Edge, std::vector<Edge>, EdgeComparator>& min_heap, std::vector<bliss::Graph::Vertex>& vertices, std::unordered_set<VertexId>& set){
		if(set.find(srcId) == set.end()){
			for(auto v: vertices[srcId].edges){
				VertexId target = v;
				if(set.find(target) == set.end()){
					Edge edge(srcId, target);
					edge.swap();
					min_heap.push(edge);
				}
			}
			set.insert(srcId);
		}
	}


std::ostream & operator<<(std::ostream & strm, const Canonical_Graph& cg){
	strm << "{" << cg.get_tuple_const() << "; " << cg.get_number_vertices() << "; " << cg.get_hash() << "}";
	return strm;
}

}
