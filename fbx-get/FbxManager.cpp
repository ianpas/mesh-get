#include "stdafx.h"
#include "FbxManager.h"

namespace Glorious
{
	std::unique_ptr<FbxManager> FbxManager::Singleton;
	std::once_flag FbxManager::OnceFlag;

	FbxManager & FbxManager::GetSingleton()
	{
		std::call_once(FbxManager::OnceFlag, [&]()
		{
			FbxManager::Singleton.reset(new FbxManager{});
		});

		return *FbxManager::Singleton;
	}

	FbxManager::FbxManager()
	{
		m_FbxManager = ::FbxManager::Create();
		m_FbxManager->SetIOSettings(::FbxIOSettings::Create(m_FbxManager, IOSROOT));
	}

	void FbxManager::ImportMesh(std::string const& mesh_path)
	{
		::FbxImporter* importer = ::FbxImporter::Create(m_FbxManager, "");
		if (!importer->Initialize(mesh_path.c_str(), -1, m_FbxManager->GetIOSettings()))
		{
			throw std::exception{ "Call to FbxImporter::Initialize() failed." };
		}

		m_Scene = FbxScene::Create(m_FbxManager, "myScene");
		importer->Import(m_Scene);
		importer->Destroy();
	}

	Type::Mesh FbxManager::GetMesh() const
	{
		Type::Mesh fbx_mesh{};

		FbxNode* root{ m_Scene->GetRootNode() };
		if (root)
		{
			for (int child_count = 0; child_count < root->GetChildCount(); ++child_count)
			{
				FbxNode* child{ root->GetChild(child_count) };

				if (child->GetNodeAttribute())
				{
					FbxNodeAttribute::EType type{ child->GetNodeAttribute()->GetAttributeType() };

					if (type == FbxNodeAttribute::eMesh)
					{
						::FbxMesh* mesh{ (::FbxMesh*)(child->GetNodeAttribute()) };


						// vertex
						FbxVector4* vertices{ mesh->GetControlPoints() };
						int vertex_amount{ mesh->GetControlPointsCount() };

						for (int k = 0; k < vertex_amount; ++k)
						{
							// due to transform of coordinate
							std::swap(vertices[k].mData[1], vertices[k].mData[2]);
						}

						// index
						int index_amount{ mesh->GetPolygonVertexCount() };
						auto indices{ mesh->GetPolygonVertices() };

						// due to transform of coordinate
						for (int k = 0; k < index_amount; k += 3)
						{
							std::swap(indices[k + 1], indices[k + 2]);

							auto v1_index = vertices[indices[k]];
							auto v2_index = vertices[indices[k + 1]];
							auto v3_index = vertices[indices[k + 2]];

							auto get_vertex = [&](FbxVector4 const& v)
							{
								Type::Vertex vertex{};
								vertex.m_Position.x = static_cast<float>(v.mData[0]);
								vertex.m_Position.y = static_cast<float>(v.mData[1]);
								vertex.m_Position.z = static_cast<float>(v.mData[2]);
								return vertex;
							};

							fbx_mesh.m_Vertices.push_back(get_vertex(v1_index));
							fbx_mesh.m_Vertices.push_back(get_vertex(v2_index));
							fbx_mesh.m_Vertices.push_back(get_vertex(v3_index));
						}


						// normal
						FbxGeometryElementNormal* normal_element{ mesh->GetElementNormal(0) };
						if (normal_element)
						{
							int normal_amount{ mesh->GetPolygonCount() * 3 };

							for (int k = 0; k < normal_amount; k += 3)
							{
								FbxVector4 v1_normal = normal_element->GetDirectArray().GetAt(k);
								FbxVector4 v2_normal = normal_element->GetDirectArray().GetAt(k + 2);
								FbxVector4 v3_normal = normal_element->GetDirectArray().GetAt(k + 1);

								auto get_normal = [&](FbxVector4 const& v)
								{
									return DirectX::XMFLOAT3
									{
										static_cast<float>(v.mData[0]) ,
										static_cast<float>(v.mData[2]) ,//due to transform of coordinate: swap yz
										static_cast<float>(v.mData[1])
									};
								};

								fbx_mesh.m_Vertices[k].m_Normal = get_normal(v1_normal);
								fbx_mesh.m_Vertices[k + 1].m_Normal = get_normal(v2_normal);
								fbx_mesh.m_Vertices[k + 2].m_Normal = get_normal(v3_normal);

							}
						}

						// material
						int material_amount = mesh->GetNode()->GetMaterialCount();
						if (material_amount != 0)
						{
							FbxSurfaceMaterial* material = mesh->GetNode()->GetMaterial(0);// only get a single material now

							if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
							{
								auto ambient = ((FbxSurfacePhong *)material)->Diffuse;
								fbx_mesh.m_Material = Type::Material
								{
									DirectX::XMFLOAT4
									{
										static_cast<float>(ambient.Get()[0]),
										static_cast<float>(ambient.Get()[1]),
										static_cast<float>(ambient.Get()[2]),
										1.f
									}
								};
							}

						}

						// texture
						FbxGeometryElementUV* uv_element{ mesh->GetElementUV(0) };

						if (uv_element)
						{
							for (int i = 0; i < mesh->GetPolygonCount(); ++i)
							{
								for (int j = 0; j < mesh->GetPolygonSize(i); ++j)
								{
									// reorder
									int k{ j };
									if (j == 1)
									{
										k = 2;
									}
									else if (j == 2)
									{
										k = 1;
									}

									int uv_index = mesh->GetTextureUVIndex(i, k);

									FbxVector2 uv = uv_element->GetDirectArray().GetAt(uv_index);

									auto get_uv = [&](FbxVector2 const& v)
									{
										return DirectX::XMFLOAT2
										{
											static_cast<float>(v.mData[0]) ,
											1 - static_cast<float>(v.mData[1])
										};
									};

									fbx_mesh.m_Vertices[i*mesh->GetPolygonSize(i) + j].m_Texture = get_uv(uv);
								}
							}

						}

					}
				}

			}

		}

		return fbx_mesh;
	}


	FbxManager::~FbxManager()
	{
		m_FbxManager->Destroy();
	}
}
