#pragma once

#include "VoxelRenderer/Animation/Skeleton.h"
#include "VoxelRenderer/Animation/Animation.h"

#include "swGraphicAPI/Resources/MeshResources.h"
#include "swGraphicAPI/ResourceManager/ResourceManager.h"

#include "VoxelRenderer/ShellMesh/FbxHelperStructs.h"


namespace vr
{


/**@brief */
class ShellMesh
{
private:

	SkeletonPtr			m_skeleton;
	AnimationPtr		m_animation;		///< Support only one animation at the moment.


	ResourcePtr< BufferObject >		m_shellMesh;			///< VertexBuffer
	ResourcePtr< BufferObject >		m_shellMeshIndex;		///< IndexBuffer
	ResourcePtr< BufferObject >		m_animMatricies;

	Size				m_numVerticies;
	Size				m_numIndiecies;

protected:
public:
	explicit		ShellMesh		( ResourceManager* manager, SkeletonPtr skeleton, AnimationPtr anim, TemporaryMeshInit& meshInitData );
	~ShellMesh	() = default;

	BufferObject*			GetVertexBuffer	()		{ return m_shellMesh.Ptr(); }
	BufferObject*			GetIndexBuffer	()		{ return m_shellMeshIndex.Ptr(); }

	Size					GetNumVerticies	() const { return m_numVerticies; }
	Size					GetNumIndicies	() const { return m_numIndiecies; }
};

DEFINE_PTR_TYPE( ShellMesh );

}
