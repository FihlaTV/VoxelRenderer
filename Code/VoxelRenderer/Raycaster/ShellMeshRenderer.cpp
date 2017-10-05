#include "ShellMeshRenderer.h"

#include "VoxelRenderer/Application/RenderingHelpers.h"

#include "swGraphicAPI/Resources/ResourcesFactory.h"

#include "Structures/ShaderCameraBuffer.h"




using namespace DirectX;
using namespace sw::input;



namespace vr
{


//====================================================================================//
//			Helpers	
//====================================================================================//


// ================================ //
//
DirectX::XMFLOAT4X4				Transpose				( DirectX::XMFLOAT4X4 mat )
{
	auto matrix = XMLoadFloat4x4( &mat );
	matrix = DirectX::XMMatrixTranspose( matrix );
	XMStoreFloat4x4( &mat, matrix );
	return mat;
}




//====================================================================================//
//			Rendering	
//====================================================================================//


// ================================ //
//
ShellMeshRenderer::ShellMeshRenderer()
	:	m_height( 0 )
	,	m_width( 0 )
{}

// ================================ //
//
void				ShellMeshRenderer::RenderShellMeshes( const std::vector< ShellMeshPtr >& shellMeshes, CameraActor* camera )
{
	if( m_width != camera->GetWidth() || m_height != camera->GetHeight() )
		ReallocateRenderTarget( (uint16)camera->GetWidth(), (uint16)camera->GetHeight() );

	UpdateCamera( camera );

	RenderingHelper::ClearRenderTargetAndDepth( m_renderer, m_shellMeshTarget.Ptr(), DirectX::XMFLOAT4( 0.0, 0.0, 0.0, 0.0 ), 1.0f );
	RenderingHelper::SetRenderTarget( m_renderer, m_shellMeshTarget.Ptr(), m_rasterizerState.Ptr(), m_blendingState.Ptr(), m_depthStencilState.Ptr() );

	SetShaderStateCommand shaderState;
	RenderingHelper::ClearTextureState( shaderState );

	shaderState.VertexShader = m_vertexShader.Ptr();
	shaderState.PixelShader = m_pixelShader.Ptr();

	m_renderer->SetShaderState( shaderState );

	RenderingHelper::BindBuffer( m_renderer, m_cameraBuffer.Ptr(), 0, (uint8)ShaderType::VertexShader );

	for( auto & shellMesh : shellMeshes )
	{
		DrawCommand drawCommand;
		drawCommand.BaseVertex = 0;
		drawCommand.BufferOffset = 0;
		drawCommand.NumVertices = (uint32)shellMesh->GetNumIndicies();
		drawCommand.Topology = PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		drawCommand.VertexBuffer = shellMesh->GetVertexBuffer();
		drawCommand.IndexBufer = shellMesh->GetIndexBuffer();
		drawCommand.ExtendedIndex = true;
		drawCommand.Layout = m_layout.Ptr();

		m_renderer->Draw( drawCommand );
	}
}

// ================================ //
//
void				ShellMeshRenderer::Render			( OctreePtr octree, RenderTargetObject* svoRenderTarget, CameraActor* camera )
{
	m_blitEffect->Blit( m_renderer, m_shellMeshTarget->GetColorBuffer(), svoRenderTarget );
}

// ================================ //
//
void				ShellMeshRenderer::Init				( IRenderer* renderer, ResourceManager* resourceManager )
{
	m_resourceManager = resourceManager;
	m_renderer = renderer;

	m_blendingState = resourceManager->GetBlendingState( BlendingInfo() );
	m_rasterizerState = resourceManager->GetRasterizerState( RasterizerStateInfo() );
	m_depthStencilState = resourceManager->GetDepthStencilState( DepthStencilInfo() );

	// Layout
	auto animLayoutDesc = ResourcesFactory::CreateInputLayoutDescriptor( L"::AnimationLayout" );
	animLayoutDesc->AddRow( "POSITION", ResourceFormat::RESOURCE_FORMAT_R32G32B32_FLOAT, 0, 0, false, 0 );
	animLayoutDesc->AddRow( "BLENDINDICES", ResourceFormat::RESOURCE_FORMAT_R8G8B8A8_UINT, 0, 12, false, 0 );
	animLayoutDesc->AddRow( "BLENDWEIGHT", ResourceFormat::RESOURCE_FORMAT_R32G32B32A32_FLOAT, 0, 16, false, 0 );
	
	ShaderInputLayout* layout;

	m_vertexShader = resourceManager->LoadVertexShader( L"Shaders/ShellMesh/ShellMeshVS.hlsl", "main", &layout, animLayoutDesc );
	m_pixelShader = resourceManager->LoadPixelShader( L"Shaders/ShellMesh/ShellMeshPS.hlsl", "main" );

	m_layout = layout;

	m_blitEffect = std::unique_ptr< BlitEffect >( new BlitEffect( m_resourceManager ) );
}

// ================================ //
//
void				ShellMeshRenderer::ProcessInput		( const sw::input::MouseState& mouse, const sw::input::KeyboardState& keyboard )
{}


//====================================================================================//
//			Data transfer to GPU	
//====================================================================================//


// ================================ //
//
void				ShellMeshRenderer::UpdateCamera		( CameraActor* camera )
{
	CameraData data = camera->GetCameraData();

	CameraConstants cameraData;
	cameraData.ProjectionMatrix = Transpose( camera->GetProjection() );
	cameraData.ViewMatrix = Transpose( camera->GetView() );
	cameraData.CameraPosition = data.Position;

	if( !m_cameraBuffer )
		m_cameraBuffer = m_resourceManager->CreateConstantsBuffer( L"Camera constant buffer", (uint8*)&cameraData, sizeof( CameraConstants ) );
	else
		RenderingHelper::UpdateBuffer( m_renderer, m_cameraBuffer.Ptr(), cameraData );

}

// ================================ //
//
void				ShellMeshRenderer::ReallocateRenderTarget	( uint16 newWidth, uint16 newHeight )
{
	// Allocate render target for shell mesh rendering result.
	RenderTargetDescriptor descriptor;
	descriptor.TextureWidth = newWidth;
	descriptor.TextureHeight = newHeight;
	descriptor.TextureType = TextureType::TEXTURE_TYPE_TEXTURE2D;
	descriptor.ColorBuffFormat = ResourceFormat::RESOURCE_FORMAT_R8G8B8A8_UNORM;
	descriptor.DepthStencilFormat = DepthStencilFormat::DEPTH_STENCIL_FORMAT_D16_UNORM;
	descriptor.Usage = ResourceUsage::RESOURCE_USAGE_DEFAULT;

	m_shellMeshTarget = m_resourceManager->CreateRenderTarget( L"::ShellMeshTarget", descriptor );
	assert( m_shellMeshTarget );
}

}	// vr


