#pragma once


#include "swCommonLib/Common/TypesDefinitions.h"
#include "swCommonLib/System/Path.h"

#include "OctreeNode.h"
#include "VoxelAttributes.h"

#include <vector>


namespace vr
{


class Octree;
DEFINE_PTR_TYPE( Octree );
DEFINE_WPTR_TYPE( Octree );


/**@brief Octree structure.

Octrees can be nested. Order of octrees in m_subtrees array matters. Links between octrees are built by OctreeFarPointer references.
This reference is offset to m_nodes array in different tree. When OctreeFarPointer is larger then size of this octree, we have to
search list of subtrees. Use GetSubtreeTotalSize to determine size of subtree (with all it's children).

Whole subtree can be threatet as flat list of nodes. One Octree object can apear multiple times as subtree but it will be only
once on subtrees list.

OctreeFarPointer should reference only direct children.
*/
class Octree
{
private:

	std::vector< OctreeWPtr	>		m_parent;			///< Parent of this octree. Can be nullptr.
	std::vector< OctreeNode* >		m_parentLink;		///< Node which points to this octree in parent structure.

	Size							m_sizeTotal;		///< Includes size of nested subtrees to faster traversal.
	Size							m_gridSize;			///< Power of 2.
	uint8							m_maxDepth;			///< Maximal levels of octree.

	std::vector< OctreeNode >		m_nodes;			///< This can be OctreeNode, OctreeLeaf, OctreeFarPointer, or VoxelAttributes.
	Size							m_indirectPtrs;		///< Offset to first free indirect pointer.

	std::vector< OctreePtr >		m_subtrees;

protected:
public:
	explicit		Octree	();
	explicit		Octree	( std::vector< OctreeNode >&& data, Size gridSize, Size firstFreeIndirectOffset );
					~Octree	() = default;


	Size			GetSubtreeTotalSize		() { return m_sizeTotal; }
	Size			GetGridSize				() { return m_gridSize; }
	Size			GetFirstFreeIndirect	() { return m_indirectPtrs;  }
	uint8			GetMaxDepth				() { return m_maxDepth;  }


	const std::vector< OctreeNode >&		AccessOctree	() { return m_nodes;  }

	const OctreeNode&		GetNode				( uint32 absolutOffset );
	uint32					GetRootNodeOffset	();
	const BlockDescriptor&	GetBlockDescriptor	() const;


public:
	bool			WriteToFile			( const filesystem::Path& outputFilePath );
	bool			LoadFromFile		( const filesystem::Path& outputFilePath );
};


}	// svo

