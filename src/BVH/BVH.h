#pragma once

#ifndef BVH_H
#define BVH_H

#include "AABB.h"
#include <vector>
#include <iostream>
#include <algorithm>    // std::sort
#include <cmath>

const int bvh_max_child = 2;


struct bvh_node {
	int idx;
	int primitiveId;
	int p0, p1;
	AABB boundingBox;
	int isLeaf;
	// char p0, p1, p2;
	int left;
	int right;
	int p2;
};



struct bvh_tree {
	std::vector<bvh_node> nodes;
};

inline void build_bvh(bvh_tree & tree, std::vector<AABB> & boundingBoxs) {
	tree.nodes.reserve(boundingBoxs.size() * 2);

}


inline int GetLeftChild(int i) {
	return (i << 1) + 1;
}

inline int GetRightChild(int i) {
	return (i << 1) + 2;
}


inline void build_bvh_simple(bvh_tree & tree, std::vector<AABB> & boundingBoxs) {
	tree.nodes.clear();

	int itemNum = boundingBoxs.size();
	int pow_n = ceil(log2(itemNum))+1;
	int treeMax = pow(2, pow_n);
	int treeMin = pow(2, pow_n-1);


	int treeSize = treeMin + itemNum - 1;
	tree.nodes.reserve(treeSize);



	for(int i = 0; i < treeSize; i++) {
		tree.nodes.push_back({});
	}

	for(int i = treeSize - itemNum; i < treeSize; i++ ){
		bvh_node node;
		node.boundingBox = boundingBoxs[i-(treeSize - itemNum)];
		node.idx = i;
		node.primitiveId = i-(treeSize - itemNum);
		node.isLeaf = true;
		tree.nodes[i] = node;
		// std::cout << "Add leaf " << i << std::endl;
	}

	//std::cout << "Tree size " << treeSize << std::endl;
	//for (int j = pow_n-1; j >= 0 ; --j ) {
	//	int head = j == 0 ? 0 : pow(2, j-1) - 1;
	//	int tail = head << 1;

		// for (int i = head; i <= tail;i++){
		for (int i = treeSize - itemNum - 1; i >= 0 ;i--){
			// std::cout << "Add node " << i << std::endl;
			bvh_node node;
			node.isLeaf = false;
			node.idx = i;
	
			int l = GetLeftChild(i);
			int r = GetRightChild(i);
	
			node.left = l < treeSize ? l : -1;
			node.right = r < treeSize ? r : -1;
			// std::cout << node.left << std::endl;
			// std::cout << node.right << std::endl;
			bvh_node * node_l = nullptr;
			if ( node.left > 0 ) node_l = &tree.nodes[l];
	
			bvh_node * node_r = nullptr;
			if (node.right > 0 ) node_r = &tree.nodes[r];
	
	
			if (node_l != nullptr && node_r != nullptr) {
				node.boundingBox = merge_aabb(node_l->boundingBox, node_r->boundingBox);
			}
			else if (node_l != nullptr){
				node.boundingBox = node_l->boundingBox;
			}
			else if (node_r != nullptr){
				node.boundingBox = node_r->boundingBox;
			}
			else{
				node.boundingBox = aabbZero;
			}
	
			tree.nodes[i] = node;
		}
	// }
}




inline bool compare_axis_z(AABB a, AABB b) {
	return (a.min.x + a.max.x) / 2 < (a.min.x + a.max.x) / 2;
}

inline int find_dim(const std::vector<AABB> & boundingBoxs, int start, int end) {
	AABB bounds = {};
	for(int i = start; i < end ; i++) {
		bounds = merge_aabb(bounds, boundingBoxs[i]);
	}

	return bounds.MaxExtent();
}

struct bvhPrimitiveInfo {
	bvhPrimitiveInfo(){}
	bvhPrimitiveInfo(const AABB & bound, int primitiveIdx) : 
		bound(bound),
		primitiveIdx(primitiveIdx),
		center((bound.min + bound.max) / 2)
	{

	}
	int primitiveIdx;
	AABB bound;
	Vec3 center;
};

struct bvhBuildingNode {
	AABB bound;
	int primitiveIdx;
	bool isLeaf;

	bvhBuildingNode * left;
	bvhBuildingNode * right;
};

constexpr int nBuckets = 12;
struct BucketInfo {
	int count = 0;
	AABB bound;
};		

inline void handleBoundingBox(bvhBuildingNode * node)
{
	if(node == nullptr) return;
	AABB * right = node->right != nullptr ? &node->right->bound : &node->left->bound;
	node->bound = merge_aabb(node->left->bound, *right);
}

inline void BuildBVH_SAH(int start, int end, std::vector<bvhPrimitiveInfo> & primitives, bvhBuildingNode * parent) {
	// std::cout << "Start : " << start << ". end : " << end << std::endl;
	int primitiveNum = end - start + 1;
	int mid = (start + end) / 2;
	// leaf node
	if (primitiveNum <= 2) {
		auto node = new bvhBuildingNode();
		parent->left = node; 

		node->bound        = primitives[start].bound;
		node->primitiveIdx = primitives[start].primitiveIdx;
		node->isLeaf       = true;
		node->left         = nullptr;
		node->right        = nullptr;
		// std::cout << "Primitive " << node->primitiveIdx << ".  isLeaf " << node->isLeaf << std::endl;
		
		if (primitiveNum > 1) {
			auto node = new bvhBuildingNode();
			parent->right = node;
			node->bound        = primitives[end].bound;
			node->primitiveIdx = primitives[end].primitiveIdx;
			node->isLeaf       = true;
			node->left         = nullptr; 
			node->right        = nullptr;
		}
	} else {
		AABB centroid = {};
		AABB totalBound = {};


		for(int i = start; i <= end; ++i) {
			totalBound = merge_aabb(totalBound, primitives[i].bound);
			centroid.min = min(centroid.min, primitives[i].center);
			centroid.max = max(centroid.max, primitives[i].center);
		}

		// std::cout << "Center surface area " << centroid.SurfaceArea() << std::endl;

		int dim = centroid.MaxExtent();

		std::nth_element(primitives.begin() + start, primitives.begin() + mid, primitives.begin() + end,
			[dim](const bvhPrimitiveInfo & a, const bvhPrimitiveInfo & b){
				return a.center[dim] < b.center[dim];
			}
		);



		BucketInfo buckets[nBuckets] = {};
		for(int i = start; i <= end; i++) {
			Vec3 posNormalized = centroid.Offset(primitives[i].center);
			int bucketIdx = nBuckets * posNormalized[dim];
			if(bucketIdx >= nBuckets) bucketIdx--;
			buckets[bucketIdx].count++;
			buckets[bucketIdx].bound = merge_aabb(buckets[bucketIdx].bound, primitives[i].bound);
		}

		float costs[nBuckets-1] = {};
		for (int i = 0; i < nBuckets-1; ++i){
			AABB b0 = {}, b1 = {};
			int c0 = 0, c1 = 0;
			for(int j = 0; j <= i; ++j) {
				b0 = merge_aabb(b0, buckets[j].bound);
				c0 += buckets[j].count;
			}

			for(int j = i+1; j < nBuckets; ++j) {
				b1 = merge_aabb(b1, buckets[j].bound);
				c1 += buckets[j].count;
			}

			costs[i] = .125f + (c0 * b0.SurfaceArea() + c1 * b1.SurfaceArea()) / totalBound.SurfaceArea();
		}

		float minCost = costs[0];
		int minCostBucket = 0;
		for (int i = 1 ; i < nBuckets-1 ; ++i) {
			if (costs[i] < minCost) {
				minCost = costs[i];
				minCostBucket = i;
			}
		}
		
		bvhPrimitiveInfo * primitiveMid = std::partition(&primitives[start], &primitives[std::min(int(primitives.size()) - 1,end+1)],
			[=] (const bvhPrimitiveInfo & primitiveInfo) {
				int b = nBuckets * centroid.Offset(primitiveInfo.center)[dim];
				if (b == nBuckets) b--;
				return b <= minCostBucket;
			});

		mid = primitiveMid - &primitives[0];
		if (mid == end) mid--;
		//std::cout << "mid " << mid << std::endl;
		//std::cout << "min cost " << minCost << ".  primitives " << primitiveNum << std::endl;

		auto left = new bvhBuildingNode();
		parent->left = left;
		left->primitiveIdx = 0;
		left->isLeaf       = false; 
		left->left         = nullptr; 
		left->right        = nullptr;

		auto right = new bvhBuildingNode();
		parent->right = right;
		right->primitiveIdx = 0;
		right->isLeaf       = false;
		right->left         = nullptr;
		right->right        = nullptr;

		BuildBVH_SAH(start, mid, primitives, parent->left);
		BuildBVH_SAH(std::min(mid+1, end), end, primitives, parent->right);		

		handleBoundingBox(left);
		handleBoundingBox(right);
		//AABB * rightBound = parent->right != nullptr ? &parent->right->bound : &parent->left->bound; 
		//parent->bound = merge_aabb(parent->left->bound, *rightBound);

	}
}

inline void RecursiveBuild(std::vector<bvh_node> & nodes, bvhBuildingNode * buildingNode, int & depth) {
	bool bIsLeaf = buildingNode->isLeaf;

	bvh_node node = {};
	node.idx = nodes.size();
	node.primitiveId = buildingNode->primitiveIdx;
	node.boundingBox = buildingNode->bound;
	node.isLeaf = bIsLeaf;
	node.left = -1;
	node.right = -1;
	nodes.push_back(node);

	//std::cout << "Idx " << node.idx << std::endl;
	//std::cout << "box " << node.boundingBox.min.tostring() <<
	//			 ". " << node.boundingBox.max.tostring() << std::endl;


	depth++;
	int d = depth;
	int d2 = depth;
	if (!bIsLeaf) {
		if (buildingNode->left != nullptr) {
			nodes[node.idx].left = nodes.size();
			RecursiveBuild(nodes, buildingNode->left, d);
			if (d > depth) depth = d;
		}
		if (buildingNode->right != nullptr) {
			nodes[node.idx].right = nodes.size();
			RecursiveBuild(nodes, buildingNode->right, d2);
			if (d2 > depth) depth = d2;
		} 
	}

	delete buildingNode;
}


inline void build_bvh_SAH(bvh_tree & tree, std::vector<AABB> & boundingBoxs) {
	std::vector<bvhPrimitiveInfo> primitives = std::vector<bvhPrimitiveInfo>();
	primitives.reserve(boundingBoxs.size());
	for(int i = 0; i < boundingBoxs.size(); ++i) {
		primitives.push_back({boundingBoxs[i], i});
	}

	tree.nodes.clear();

	bvhBuildingNode * root = new bvhBuildingNode();
	BuildBVH_SAH(0, primitives.size()-1, primitives, root);
	root->bound = merge_aabb(root->left->bound, root->right->bound);
	root->isLeaf = false;
	int depth = 1;
	RecursiveBuild(tree.nodes, root, depth);

	//std::cout << "Done output tree max depth " << depth << std::endl;

}

#endif
