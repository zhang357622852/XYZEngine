#include "stdafx.h"
#include "SkinnedMesh.h"

#include "XYZ/Utils/Math/Math.h"
#include "PreviewBone.h"

#include <glm/gtx/transform.hpp>

#include <tpp_interface.hpp>

namespace XYZ {
	namespace Editor {
        namespace Helper {
            static void GetTriangulationPt(const std::vector<tpp::Delaunay::Point>& points, int keyPointIdx, const tpp::Delaunay::Point& sPoint, double& x, double& y)
            {
                if (keyPointIdx >= points.size())
                {
                    x = sPoint[0]; // added Steiner point, it's data copied to sPoint
                    y = sPoint[1];
                }
                else
                {
                    // point from original data
                    x = sPoint[0];
                    y = sPoint[1];
                }
            }
            static glm::vec2 CalculateTexCoord(const glm::vec2& pos, const glm::vec2& size)
            {
                glm::vec2 position = pos + size / 2.0f;
                return glm::vec2(position.x / size.x, position.y / size.y);
            }

        }

        BoneVertex* Submesh::FindVertex(const glm::vec2& pos, float radius)
        {  
            for (auto& vertex : GeneratedVertices)
            {
                if (glm::distance(pos, vertex.Position) < radius)
                    return &vertex;
            }
            return nullptr;
        }
        Triangle* Submesh::FindTriangle(const glm::vec2& pos, float radius)
        {
            for (auto& triangle : Triangles)
            {
                const BoneVertex& first =  GeneratedVertices[triangle.First];
                const BoneVertex& second = GeneratedVertices[triangle.Second];
                const BoneVertex& third =  GeneratedVertices[triangle.Third];
                if (Math::PointInTriangle(pos, first.Position, second.Position, third.Position))
                    return &triangle;
            }
            return nullptr;
        }
        
        SkinnedMesh::SkinnedMesh()
        {
            Colors[TriangleColor] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
            Colors[VertexColor] = glm::vec4(0.7f, 0.4f, 1.0f, 1.0f);
        }

       
        void SkinnedMesh::Triangulate()
		{
            for (Submesh& subMesh : Submeshes)
                triangulateSubmesh(subMesh);
		}
		bool SkinnedMesh::EraseVertexAtPosition(const glm::vec2& pos)
		{
            for (auto& subMesh : Submeshes)
            {
                uint32_t counter = 0;
                for (auto& vertex : subMesh.GeneratedVertices)
                {
                    if (glm::distance(pos, glm::vec2(vertex.Position.x, vertex.Position.y)) < PointRadius)
                    {
                        subMesh.GeneratedVertices.erase(subMesh.GeneratedVertices.begin() + counter);         
                        return true;
                    }
                    counter++;
                }
            }
            return false;
		}
		bool SkinnedMesh::EraseTriangleAtPosition(const glm::vec2& pos)
		{
            for (auto& subMesh : Submeshes)
            {
                uint32_t counter = 0;
                for (auto& triangle : subMesh.Triangles)
                {
                    BoneVertex& first =  subMesh.GeneratedVertices[triangle.First];
                    BoneVertex& second = subMesh.GeneratedVertices[triangle.Second];
                    BoneVertex& third =  subMesh.GeneratedVertices[triangle.Third];
                    if (Math::PointInTriangle(pos, glm::vec2(first.Position.x, first.Position.y), glm::vec2(second.Position.x, second.Position.y), glm::vec2(third.Position.x, third.Position.y)))
                    {
                        subMesh.Triangles.erase(subMesh.Triangles.begin() + counter);
                        eraseEmptyPoints(subMesh);
                        
                        return true;
                    }
                    counter++;
                }
            }
            return false;
		}

        void SkinnedMesh::BuildPreviewVertices(const Tree& hierarchy, const glm::vec2& contextSize, bool preview, bool weight)
        {
            if (preview)
            {
                for (auto& subMesh : Submeshes)
                {
                    uint32_t counter = 0;
                    for (auto& vertex : subMesh.VerticesLocalToBones)
                    {
                        BoneVertex vertexLocalToBone = vertex;
                        getPositionFromBones(vertexLocalToBone, hierarchy);
                        if (weight)
                        {
                            getColorFromBoneWeights(vertexLocalToBone, hierarchy);
                        }
                        BoneVertex& genVertex = subMesh.GeneratedVertices[counter++];
                        PreviewVertices.push_back({
                            vertexLocalToBone.Color,
                            glm::vec3(vertexLocalToBone.Position, 1.0f),
                            Helper::CalculateTexCoord(genVertex.Position, contextSize)
                           });
                    }            
                }
            }
            else
            {
                for (auto& subMesh : Submeshes)
                {
                    for (auto& vertex : subMesh.GeneratedVertices)
                    {
                        BoneVertex vertexLocalToBone = vertex;
                        if (weight)
                        {
                            getColorFromBoneWeights(vertexLocalToBone, hierarchy);
                        }
                        PreviewVertices.push_back({
                            vertexLocalToBone.Color,
                            glm::vec3(vertexLocalToBone.Position, 1.0f),
                            Helper::CalculateTexCoord(vertex.Position, contextSize)
                        });
                    }
                }
            }
        }

        void SkinnedMesh::InitializeVerticesLocalToBone(const Tree& hierarchy)
        {
            for (auto& subMesh : Submeshes)
            {
                subMesh.VerticesLocalToBones.clear();
                for (auto& vertex : subMesh.GeneratedVertices)
                {
                    BoneVertex vertexLocalToBone = vertex;
                    GetPositionLocalToBone(vertexLocalToBone, hierarchy);
                    subMesh.VerticesLocalToBones.push_back(vertexLocalToBone);
                }
            }
        }
 
       
		bool SkinnedMesh::trianglesHaveIndex(const Submesh& subMesh, uint32_t index)
		{
			for (auto& triangle : subMesh.Triangles)
			{
				if (triangle.First == index || triangle.Second == index || triangle.Third == index)
					return true;
			}
			return false;
		}
		void SkinnedMesh::triangulateSubmesh(Submesh& subMesh)
		{
            if (subMesh.GeneratedVertices.size() < 3)
                return;
            std::vector<tpp::Delaunay::Point> points;

            for (auto& p : subMesh.GeneratedVertices)
            {
                points.push_back({ p.Position.x, p.Position.y });
            }
            tpp::Delaunay generator(points);
            generator.setMinAngle(30.5f);
            generator.setMaxArea(12000.5f);
            generator.Triangulate(true);

            subMesh.OriginalVertices = std::move(subMesh.GeneratedVertices);
            subMesh.Triangles.clear();
            subMesh.GeneratedVertices.clear();
            for (tpp::Delaunay::fIterator fit = generator.fbegin(); fit != generator.fend(); ++fit)
            {
                tpp::Delaunay::Point sp1;
                tpp::Delaunay::Point sp2;
                tpp::Delaunay::Point sp3;

                int keypointIdx1 = generator.Org(fit, &sp1);
                int keypointIdx2 = generator.Dest(fit, &sp2);
                int keypointIdx3 = generator.Apex(fit, &sp3);

                double x = 0.0f, y = 0.0f;
                if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx1))
                {
                    Helper::GetTriangulationPt(points, keypointIdx1, sp1, x, y);
                    if (subMesh.GeneratedVertices.size() <= keypointIdx1)
                        subMesh.GeneratedVertices.resize((size_t)keypointIdx1 + 1);
                    subMesh.GeneratedVertices[keypointIdx1] = { glm::vec2((float)x, (float)y ) };
                }
                if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx2))
                {
                    Helper::GetTriangulationPt(points, keypointIdx2, sp2, x, y);
                    if (subMesh.GeneratedVertices.size() <= keypointIdx2)
                        subMesh.GeneratedVertices.resize((size_t)keypointIdx2 + 1);
                    subMesh.GeneratedVertices[keypointIdx2] = { glm::vec2((float)x, (float)y ) };
                }
                if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx3))
                {
                    Helper::GetTriangulationPt(points, keypointIdx3, sp3, x, y);
                    if (subMesh.GeneratedVertices.size() <= keypointIdx3)
                        subMesh.GeneratedVertices.resize((size_t)keypointIdx3 + 1);
                    subMesh.GeneratedVertices[keypointIdx3] = { glm::vec2((float)x, (float)y ) };
                }
                subMesh.Triangles.push_back({
                    (uint32_t)keypointIdx1,
                    (uint32_t)keypointIdx2,
                    (uint32_t)keypointIdx3
                });
            }
		}
        void SkinnedMesh::eraseEmptyPoints(Submesh& subMesh)
        {      
            std::vector<uint32_t> erasedPoints;
            for (uint32_t i = 0; i < subMesh.GeneratedVertices.size(); ++i)
            {
                if (!trianglesHaveIndex(subMesh, i))
                    erasedPoints.push_back(i);
            }
            for (int32_t i = erasedPoints.size() - 1; i >= 0; --i)
            {
                subMesh.GeneratedVertices.erase(subMesh.GeneratedVertices.begin() + erasedPoints[i]);
                for (auto& triange : subMesh.Triangles)
                {
                    if (triange.First >= erasedPoints[i])
                        triange.First--;
                    if (triange.Second >= erasedPoints[i])
                        triange.Second--;
                    if (triange.Third >= erasedPoints[i])
                        triange.Third--;
                }
            }
        }

        void SkinnedMesh::GetPositionLocalToBone(BoneVertex& vertex, const Tree& hierarchy)
        {
            glm::mat4 boneTransform = glm::mat4(0.0f);
            bool hasBone = false;
            for (uint32_t i = 0; i < BoneData::sc_MaxBonesPerVertex; ++i)
            {
                if (vertex.Data.IDs[i] != -1)
                {
                    const PreviewBone* bone = static_cast<const PreviewBone*>(hierarchy.GetData(vertex.Data.IDs[i]));
                    //boneTransform += glm::translate(glm::vec3(bone->LocalPosition, 0.0f)) * vertex.Data.Weights[i];
                    boneTransform += bone->WorldTransform * vertex.Data.Weights[i];;
                    hasBone = true;
                }
            }
            if (hasBone)
            {
                glm::vec4 localToBone = glm::inverse(boneTransform) * glm::vec4(vertex.Position, 0.0f, 1.0f);
                vertex.Position.x = localToBone.x;
                vertex.Position.y = localToBone.y;
            }
        }
        void SkinnedMesh::getPositionFromBones(BoneVertex& vertex, const Tree& hierarchy)
        {
            bool hasBone = false;
            glm::mat4 boneTransform = glm::mat4(0.0f);
            for (uint32_t i = 0; i < 4; ++i)
            {
                if (vertex.Data.IDs[i] != -1)
                {
                    const PreviewBone* bone = static_cast<const PreviewBone*>(hierarchy.GetData(vertex.Data.IDs[i]));
                    boneTransform += bone->WorldTransform * vertex.Data.Weights[i];
                    hasBone = true;
                }
            }
            if (hasBone)
            {
                glm::vec4 positionFromBone = boneTransform * glm::vec4(vertex.Position, 0.0f, 1.0f);
                vertex.Position.x = positionFromBone.x;
                vertex.Position.y = positionFromBone.y;
            }
        }
        void SkinnedMesh::getColorFromBoneWeights(BoneVertex& vertex, const Tree& hierarchy)
        {
            glm::vec3 color = glm::vec3(0.0f);
            for (uint32_t i = 0; i < 4; ++i)
            {
                if (vertex.Data.IDs[i] != -1)
                {
                    const PreviewBone* bone = static_cast<const PreviewBone*>(hierarchy.GetData(vertex.Data.IDs[i]));
                    color += bone->Color * vertex.Data.Weights[i];
                }
            }
            vertex.Color = color;
        }
        BoneVertex::BoneVertex()
            :
            Position(glm::vec2(0.0f)),
            Color(glm::vec4(0.0f))
        {
        }
        BoneVertex::BoneVertex(const glm::vec2& pos)
            :
            Position(pos),
            Color(glm::vec4(0.0f))
        {
        }
        BoneVertex::BoneVertex(const BoneVertex& other)
            :
            Position(other.Position),
            Color(other.Color)
        {
            memcpy(Data.IDs, other.Data.IDs, BoneData::sc_MaxBonesPerVertex * sizeof(int32_t));
            memcpy(Data.Weights, other.Data.Weights, BoneData::sc_MaxBonesPerVertex * sizeof(float));
        }
}
}