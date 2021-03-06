/**
@file DX11Buffer.cpp
@author nieznanysprawiciel
@copyright File is part of graphic engine SWEngine.
*/
#include "swGraphicAPI/DX11API/stdafx.h"

#include "DX11Buffer.h"
#include "DX11Texture.h"
#include "DX11Initializer/DX11ConstantsMapper.h"

#include "swCommonLib/Common/Converters.h"

#include "swCommonLib/Common/MemoryLeaks.h"


RTTR_REGISTRATION
{
	rttr::registration::class_< DX11Buffer >( "DX11Buffer" );
}


// ================================ //
//
DX11Buffer::DX11Buffer( const std::wstring& name, const BufferInfo& descriptor, ComPtr< ID3D11Buffer > buff )
	:	BufferObject( descriptor.ElementSize, descriptor.NumElements ), m_buffer( buff )
	,	m_descriptor( descriptor )
{
	m_descriptor.Name = name;

	if( IsDebugLayerEnabled() )
	{	
		std::string nameStr = Convert::ToString< std::wstring >( name );
		SetDebugName( m_buffer.Get(), nameStr );
	}
}

// ================================ //
//
DX11Buffer::~DX11Buffer()
{}


/**@brief Tworzy bufor wierzcho�k�w, indeks�w lub sta�ych o podanych parametrach.

@param[in] name Buffer name or file path.
@param[in] data Pointer to initialization data. Memory can be released after call.
@param[in] bufferInfo Buffer descriptor.
@return Wska�nik na DX11Buffer w przypadku powodzenia lub nullptr, je�eli co� p�jdzie nie tak.*/
DX11Buffer*		DX11Buffer::CreateFromMemory	( const std::wstring& name, const uint8* data, const BufferInfo& bufferInfo )
{
	if( !ValidateInitData( bufferInfo ) )
		return nullptr;

	ResourceBinding bindFlag;
	if( bufferInfo.BufferType == BufferType::VertexBuffer )
		bindFlag = ResourceBinding::BIND_RESOURCE_VERTEX_BUFFER;
	else if( bufferInfo.BufferType == BufferType::IndexBuffer )
		bindFlag = ResourceBinding::BIND_RESOURCE_INDEX_BUFFER;
	else if( bufferInfo.BufferType == BufferType::ConstantBuffer )
		bindFlag = ResourceBinding::BIND_RESOURCE_CONSTANT_BUFFER;
	else if( bufferInfo.BufferType == BufferType::TextureBuffer )
		bindFlag = ResourceBinding::BIND_RESOURCE_SHADER_RESOURCE;

	// Wype�niamy deskryptor bufora
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory( &bufferDesc, sizeof( bufferDesc ) );
	bufferDesc.Usage = DX11ConstantsMapper::Get( bufferInfo.Usage );
	bufferDesc.BindFlags = DX11ConstantsMapper::Get( bindFlag );
	bufferDesc.ByteWidth = bufferInfo.NumElements * bufferInfo.ElementSize;
	bufferDesc.MiscFlags = bufferInfo.AllowRaw ? D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : 0;

	D3D11_SUBRESOURCE_DATA* initDataPtr = nullptr;
	D3D11_SUBRESOURCE_DATA initData;
	if( data )
	{
		// Je�eli bufor nie istnieje to do funkcji tworz�cej bufor powinni�my poda� nullptr.
		ZeroMemory( &initData, sizeof( initData ) );
		initData.pSysMem = data;
		initDataPtr = &initData;
	}

	HRESULT result;
	ComPtr< ID3D11Buffer > newBuffer;
	result = device->CreateBuffer( &bufferDesc, initDataPtr, &newBuffer );
	if( FAILED( result ) )
		return nullptr;

	DX11Buffer* newBufferObject = new DX11Buffer( name, bufferInfo, newBuffer );
	return newBufferObject;
}


/**@brief Kopiuje pami�� bufora i zwraca w MemoryChunku.

Funkcja zwraca zawarto�� bufora. Pami�� jest kopiowana dwukrotnie.
Najpierw na GPU do tymczasowego bufora, a potem po zmapowaniu na pami�� RAM,
odbywa si� kopiowanie do MemoryChunka.

@todo Nie trzeba by wykonywa� kopiowania na GPU, gdyby bufor by� stworzony z flag�
D3D11_USAGE_STAGING lub D3D11_USAGE_DEFAULT. Trzeba sprawdzi� flagi i robi� kopiowanie tylko, gdy to konieczne.

@attention Funkcja nie nadaje si� do wykonania wielow�tkowego. U�ywa DeviceContextu do kopiowania danych
w zwi�zku z czym wymaga synchronizacji z innymi funkcjami renderuj�cymi.
*/
MemoryChunk		DX11Buffer::CopyData()
{
	// Trzeba stworzy� nowy bufor
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory( &bufferDesc, sizeof( bufferDesc ) );
	bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_STAGING;
	//bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;		// Przy fladze usage::staging nie mo�na bindowa� zasobu do potoku graficznego.
	bufferDesc.ByteWidth = m_elementSize * m_elementCount;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	HRESULT result;
	ID3D11Buffer* newBuffer;
	result = device->CreateBuffer( &bufferDesc, nullptr, &newBuffer );
	if( FAILED( result ) )
		return MemoryChunk();

	// Kopiowanie zawarto�ci mi�dzy buforami
	device_context->CopyResource( newBuffer, m_buffer.Get() );

	D3D11_MAPPED_SUBRESOURCE data;
	result = device_context->Map( newBuffer, 0, D3D11_MAP::D3D11_MAP_READ, 0, &data );
	if( FAILED( result ) )
		return MemoryChunk();

	MemoryChunk memoryChunk;
	memoryChunk.MemoryCopy( (int8*)data.pData, m_elementSize * m_elementCount );

	device_context->Unmap( newBuffer, 0 );
	newBuffer->Release();

	return std::move( memoryChunk );
}

// ================================ //
//
TextureObject*	DX11Buffer::CreateRawShaderViewImpl		() const
{
	if( m_descriptor.AllowRaw )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		viewDesc.Format = DX11ConstantsMapper::Get( ResourceFormat::RESOURCE_FORMAT_R32_TYPELESS );
		viewDesc.BufferEx.FirstElement = 0;
		viewDesc.BufferEx.NumElements = m_descriptor.NumElements;
		viewDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;

		ComPtr< ID3D11ShaderResourceView > view;

		auto result = device->CreateShaderResourceView( m_buffer.Get(), &viewDesc, &view );
		if( FAILED( result ) )
			return nullptr;

		TextureInfo texDesc;
		texDesc.Usage = m_descriptor.Usage;
		texDesc.TextureType = TextureType::TEXTURE_TYPE_BUFFER;
		texDesc.Format = ResourceFormat::RESOURCE_FORMAT_R32_TYPELESS;
		texDesc.TextureHeight = 1;
		texDesc.TextureWidth = m_descriptor.ElementSize * m_descriptor.NumElements / 4;		// Width is number of bytes.
		texDesc.FilePath = m_descriptor.Name;

		return new DX11Texture( std::move( texDesc ), m_buffer, view );
	}

	return nullptr;
}

// ================================ //
//
bool			DX11Buffer::ValidateInitData		( const BufferInfo& descriptor )
{
	if( descriptor.AllowRaw && descriptor.BufferType == BufferType::ConstantBuffer )
		return false;
	return true;
}
