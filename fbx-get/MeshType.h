#pragma once
#include <DirectXMath.h>

namespace Glorious
{
	namespace Type
	{
		class Vertex
		{
		public:
			DirectX::XMFLOAT3 m_Position;
			DirectX::XMFLOAT3 m_Normal;
			DirectX::XMFLOAT2 m_Texture;
		};

		class Material
		{
		public:
			DirectX::XMFLOAT4 m_Diffuse;
		};

		class Mesh
		{
		public:
			std::vector<Vertex>						m_Vertices;
			Material								m_Material;
		};
	}
	
}