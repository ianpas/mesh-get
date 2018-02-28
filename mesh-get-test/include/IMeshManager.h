#pragma once
#include "MeshType.h"

namespace Glorious
{
	class IMeshManager
	{
	public:
		virtual ~IMeshManager() = default;

	public:
		virtual void ImportMesh(std::string const& mesh_path) = 0;

		virtual Type::Mesh GetMesh() const = 0;
	};
}