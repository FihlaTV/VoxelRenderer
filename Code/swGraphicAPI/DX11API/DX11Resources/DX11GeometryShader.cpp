/**
@file DX11PixelShader.cpp
@author nieznanysprawiciel
@copyright File is part of graphic engine SWEngine.
*/
#include "swGraphicAPI/DX11API/stdafx.h"

#include "DX11GeometryShader.h"

#include "swCommonLib/Common/MemoryLeaks.h"


//====================================================================================//
//			RTTR registration	
//====================================================================================//

RTTR_REGISTRATION
{
	rttr::registration::class_< DX11GeometryShader >( "DX11GeometryShader" );
}


//====================================================================================//
//			DX11PixelShader	
//====================================================================================//

/**@brief */
DX11GeometryShader::DX11GeometryShader( ID3D11GeometryShader* shader )
	:	m_geometryShader( shader )
{}

/**@brief */
DX11GeometryShader::~DX11GeometryShader()
{
	if( m_geometryShader )
		m_geometryShader->Release();
	m_geometryShader = nullptr;
}

/**@brief */
bool DX11GeometryShader::ReloadFromFile()
{

	return false;
}

/**@brief */
bool DX11GeometryShader::ReloadFromBinFile()
{

	return false;
}

/**@brief */
void DX11GeometryShader::SaveShaderBinFile( const std::wstring& fileName )
{
	assert( false );

}

/**@bref Tworzy obiekt DX11GeometryShader na podstawie pliku.

W przypadku b��d�w kompilacji w trybie debug s� one przekierowane do okna Output.

Na razie obs�uguje tylko nieskompilowane pliki.
@param[in] fileName Nazwa pliku, z kt�rego zostanie wczytany shader.
@param[in] shaderName Nazwa funkcji, kt�ra jest punktem poczatkowym wykonania shadera.
@param[in] shaderModel �a�cuch znak�w opisuj�cy shader model.
@return Zwaraca wska�nik na DX11VertexShader lub nullptr w przypadku b��d�w wczytywania b�d� kompilacji.
*/
DX11GeometryShader* DX11GeometryShader::CreateFromFile( const std::wstring& fileName, const std::string& shaderName, const char* shaderModel )
{
	HRESULT result;
	ID3DBlob* compiledShader;
	ID3D11GeometryShader* geometryShader;
	// Troszk� zamieszania, ale w trybie debug warto wiedzie� co jest nie tak przy kompilacji shadera
#ifdef _DEBUG
	ID3D10Blob* error_blob = nullptr;	// Tu znajdzie si� komunikat o b��dzie
#endif

	// Kompilujemy shader znaleziony w pliku
	D3DX11CompileFromFile( fileName.c_str(), 0, 0, shaderName.c_str(), shaderModel,
#ifdef _DEBUG						   
						   D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION,
#else
						   0,
#endif
						   0, 0, &compiledShader,
#ifdef _DEBUG
						   &error_blob,	// Funkcja wsadzi informacje o b��dzie w to miejsce
#else
						   0,	// W trybie Release nie chcemy dostawa� b��d�w
#endif
						   &result );

	if( FAILED( result ) )
	{
#ifdef _DEBUG
		if( error_blob )
		{
			// B��d zostanie wypisany na standardowe wyj�cie
			OutputDebugStringA( (char*)error_blob->GetBufferPointer() );
			error_blob->Release();
		}
#endif
		return nullptr;
	}

	// Tworzymy obiekt shadera na podstawie tego co sie skompilowa�o
	result = device->CreateGeometryShader( compiledShader->GetBufferPointer(),
										compiledShader->GetBufferSize(),
										nullptr, &geometryShader );

	if( FAILED( result ) )
	{
		compiledShader->Release();
		return nullptr;
	}

	// Tworzymy obiekt nadaj�cy si� do u�ycia w silniku i zwracamy wska�nik na niego
	DX11GeometryShader* newShader = new DX11GeometryShader( geometryShader );
	return newShader;
}

// ================================ //
//
DX11GeometryShader* DX11GeometryShader::CreateFromBinFile( const std::wstring& fileName, const std::string& shaderName, const char* shaderModel)
{
	return nullptr;
}

